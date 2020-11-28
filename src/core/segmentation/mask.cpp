#include "mask.h"
#include "core/np2cv.h"
#include "pybind11/stl.h"

namespace core {
	namespace segmentation {
		namespace py = pybind11;
		using namespace py::literals;

		void npy_buffer_to_cv(py::object object, cv::Mat& mat) {
			auto buffer = object.cast<py::buffer>();
			py::buffer_info info = buffer.request();
			int rows = info.shape[0];
			int cols = info.shape[1];

			auto array = static_cast<unsigned char*>(info.ptr);
			mat.create(rows, cols, CV_8U);
			memcpy(mat.data, array, sizeof(unsigned char) * rows * cols);
		}

		MaskCollection::MaskCollection(int rows, int cols, int max_size) : rows_(rows), cols_(cols), max_size_(max_size) {
			it_ = history_.end();
		}

		void MaskCollection::push(const Mask& mask) {
			if (!history_.empty() && it_ != history_.end()) {
				history_.erase(it_, history_.end());
			}
			history_.push_back(mask);

			if (history_.size() > max_size_) {
				history_.pop_front();
			}

			it_ = history_.end();
		}

		void MaskCollection::push_new() {
			Mask mask(cols_, rows_);
			push(mask);
		}

		void MaskCollection::loadData(bool keep, bool immediate, const std::function<void(const Mask&, const Mask&, const Mask&)>& when_finished_fct) {
			//is_valid_ = true;
			keep_ = keep;
			jobId id;

			jobFct job = [=](float& progress, bool& abort) -> std::shared_ptr<JobResult> {
				auto result = std::make_shared<JobResult>();
				auto state = PyGILState_Ensure();
				try {
					py::module script = py::module::import("python.scripts.segmentation");

					auto& dict = script.attr("load_mask_collection")(basename_path_).cast<py::dict>();

					if (dict.contains("current")) {
						Mask mask;
						npy_buffer_to_cv(dict["current"], mask.getData());
						clearHistory();
						mask.updateDimensions();
						push(mask);
					}
					if (dict.contains("predicted")) {
						npy_buffer_to_cv(dict["predicted"], prediction_.getData());
						prediction_.updateDimensions();
					}
					if (dict.contains("validated")) {
						npy_buffer_to_cv(dict["validated"], validated_.getData());
						prediction_.updateDimensions();
					}
						
					auto& users = dict["users"].cast<std::vector<std::string>>();
					for (auto& user : users) {
						setValidatedBy(user);
					}
				}
				catch (const std::exception& e) {
					py::print(e.what());
					result->err = e.what();
				}

				PyGILState_Release(state);
				return result;
			};

			jobResultFct when_finished = [=](const std::shared_ptr<JobResult>& result) {
				when_finished_fct(getCurrent(), prediction_, validated_);
			};
			
			if (immediate) {
				float a = 0.f;
				bool b = true;
				auto& res = job(a, b);
				if (!res->err.empty())
					std::cout << "res" << res->err << std::endl;
				when_finished_fct(getCurrent(), prediction_, validated_);
			}
			else {
				JobScheduler::getInstance().addJob("dicom_to_image", job, when_finished);
			}
		}

		void MaskCollection::unloadData(bool force) {
			if (!keep_ || force) {
				prediction_ = Mask();
				validated_ = Mask();
				history_.clear();
				it_ = history_.end();
				is_valid_ = false;
			}
		}

		MaskCollection MaskCollection::copy() {
			MaskCollection collection(rows_, cols_);
			collection.rows_ = rows_;
			collection.cols_= cols_;
			for (auto& mask : history_) {
				collection.history_.push_back(mask.copy());
			}
			collection.it_ = collection.history_.end();
			collection.prediction_ = prediction_.copy();
			collection.validated_ = validated_.copy();
			return collection;
		}

		void MaskCollection::clearHistory() {
			history_.clear();
			it_ = history_.end();
			is_valid_ = false;
		}

		void MaskCollection::setDimensions(std::shared_ptr<DicomSeries> dicom) {
			if (!dicom->getData().empty()) {
				auto& data = dicom->getData()[0].data;
				if (data.rows > 0 && data.cols > 0) {
					rows_ = data.rows;
					cols_ = data.cols;
					is_valid_ = true;
					return;
				}
			}
			dicom->loadCase(0, false, [this, &dicom](const core::Dicom& dicom_res) {
				rows_ = dicom_res.data.rows;
				cols_ = dicom_res.data.cols;
				is_valid_ = true;
				dicom->cleanData();
			});
		}

		Mask& MaskCollection::undo() {
			if (history_.empty()) {
				return Mask(rows_, cols_);
			}
			if (it_ != ++history_.begin()) {
				it_--;
			}
			return getCurrent();
		}

		Mask& MaskCollection::redo() {
			if (history_.empty()) {
				return Mask(rows_, cols_);
			}
			if (it_ != history_.end()) {
				it_++;
			}
			return getCurrent();
		}

		int MaskCollection::size() {
			return history_.size();
		}

		bool MaskCollection::isCursorBegin() {
			if (history_.empty())
				return true;
			return it_ == ++history_.begin();
		}

		bool MaskCollection::isCursorEnd() {
			if (history_.empty())
				return true;
			return it_ == history_.end();
		}

		Mask& core::segmentation::MaskCollection::getCurrent() {
			if (history_.empty()) {
				push_new();
			}
			iterator my_it = it_;
			return *(--my_it);
		}

		void MaskCollection::setBasenamePath(const std::string& basename) {
			basename_path_ = basename;
		}

		std::string MaskCollection::saveCollection(const std::string& basename) {
			basename_path_ = basename;
			return saveCollection();
		}

		std::string MaskCollection::saveCollection() {
			if (basename_path_.empty()) {
				return "Cannot save mask because basename path is missing";
			}

			std::string error_msg;
			auto state = PyGILState_Ensure();
			try {
				NDArrayConverter::init_numpy();
				py::module np = py::module::import("numpy");
				auto& current = getCurrent();

				py::module seg = py::module::import("python.scripts.segmentation");
				std::vector<std::string> users;
				for (auto& user : validated_by_) {
					users.push_back(user);
					std::cout << "SDFSDF" << user << std::endl;
				}
				seg.attr("save_mask_collection")(users, current.getData(), validated_.getData(), prediction_.getData(), basename_path_);
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
				error_msg = e.what();
			}
			PyGILState_Release(state);

			return error_msg;
		}

		void MaskCollection::setValidatedBy(std::string name) {
			validated_by_.insert(name);
			is_validated_ = true;
		}

		void MaskCollection::removeAllValidatedBy() {
			is_validated_ = false;
			validated_by_.clear();
			validated_ = Mask();
		}

		void MaskCollection::removeValidatedBy(std::string name) {
			if (std::find(validated_by_.begin(), validated_by_.end(), name) != validated_by_.end()) {
				validated_by_.erase(name);
			}
			if (validated_by_.empty()) {
				is_validated_ = false;
				validated_ = Mask();
			}
		}

		Mask::Mask(int rows, int cols) : rows_(rows), cols_(cols) {
			setDimensions(rows, cols);
		}

		void Mask::setDimensions(int rows, int cols, bool no_build) {
			rows_ = rows;
			cols_ = cols;
			if (!no_build)
				data_ = cv::Mat::zeros(cv::Size(rows, cols), CV_8U);
		}

		void Mask::updateDimensions() {
			rows_ = data_.rows;
			cols_ = data_.cols;
		}

		Mask const Mask::copy() const {
			Mask mask(rows_, cols_);
			data_.copyTo(mask.data_);
			mask.rows_ = rows_;
			mask.cols_ = cols_;
			mask.is_empty_ = is_empty_;
			return mask;
		}

		void Mask::setData(cv::Mat& data) {
			data_ = data;
			rows_ = data.rows;
			cols_ = data.cols;
			is_empty_ = false;
		}

		void Mask::intersect_with(const Mask& mat) {
			if (mat.data_.rows != rows_ || mat.data_.cols != cols_) {
				return;
			}
			is_empty_ = false;

			unsigned char* data = data_.data;
			for (int row = 0; row < rows_; ++row) {
				auto p = mat.data_.ptr<unsigned char>(row);

				for (int col = 0; col < cols_; ++col) {
					auto value = (float)*p;
					if (value == 0) {
						*data = 0;
					}
					p++;
					data++;
				}
			}
		}

		void Mask::union_with(const Mask& mat) {
			if (mat.data_.rows != rows_ || mat.data_.cols != cols_) {
				return;
			}
			is_empty_ = false;

			unsigned char* data = data_.data;
			for (int row = 0; row < rows_; ++row) {
				auto p = mat.data_.ptr<unsigned char>(row);

				for (int col = 0; col < cols_; ++col) {
					auto value = (float)*p;
					if (*data || value)
						*data = 1;
					p++;
					data++;
				}
			}
		}

		void Mask::difference_with(const Mask& mat) {
			if (mat.data_.rows != rows_ || mat.data_.cols != cols_) {
				return;
			}
			is_empty_ = false;

			unsigned char* data = data_.data;
			for (int row = 0; row < rows_; ++row) {
				auto p = mat.data_.ptr<unsigned char>(row);

				for (int col = 0; col < cols_; ++col) {
					auto value = (float)*p;
					if (value)
						*data = 0;
					p++;
					data++;
				}
			}
		}


		void buildHuMask(const cv::Mat& hu_mat, Mask& mask, int min, int max) {
			if (hu_mat.rows <= 0 || hu_mat.cols <= 0) {
				mask = Mask(0, 0);
				return;
			}

			auto &mat = mask.getData();
			mat = cv::Mat::zeros(cv::Size(hu_mat.rows, hu_mat.cols), CV_8U);
			mask.setDimensions(hu_mat.rows, hu_mat.cols);

			unsigned char* data = mat.data;

			for (int row = 0; row < hu_mat.rows; ++row) {
				auto p = hu_mat.ptr<short int>(row);

				for (int col = 0; col < hu_mat.cols; ++col) {
					auto value = (float)*p;

					*data = (value > min && value < max) ? 1 : 0;
					p++;
					data++;
				}
			}
		}

		void lassoSelectToMask(const std::vector<cv::Point>& pts, Mask& mask, int value) {
			if (!pts.empty() && mask.getData().rows > 0 && mask.getData().cols > 0)
				cv::fillPoly(mask.getData(), pts, value);
		}

		void brushToMask(float radius, ImVec2 position, Mask& mask, int value) {
			if (mask.getData().rows > 0 && mask.getData().cols > 0) {
				cv::Point circle_center(position.x, position.y);
				cv::circle(mask.getData(), circle_center, radius, value, -1);
			}
		}
	}
}