#include "mask.h"
#include "python/py_api.h"

namespace core {
	namespace segmentation {
		namespace py = pybind11;

		MaskCollection::MaskCollection(int rows, int cols) : rows_(rows), cols_(cols) {
			it_ = history_.end();
		}

		void MaskCollection::push(const Mask& mask) {
			if (it_ != history_.end()) {
				history_.erase(it_, history_.end());
			}
			history_.push_back(mask);
			it_ = history_.end();
		}

		void MaskCollection::push_new() {
			Mask mask(cols_, rows_);
			push(mask);
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

		void MaskCollection::setDimensions(std::shared_ptr<DicomSeries> dicom) {
			dicom->loadCase(0, false, [this, &dicom](const core::Dicom& dicom_res) {
				rows_ = dicom_res.data.rows;
				cols_ = dicom_res.data.cols;
				dicom->cleanData();
			});
		}

		Mask& MaskCollection::undo() {
			if (it_ != ++history_.begin()) {
				it_--;
			}
			return getCurrent();
		}

		Mask& MaskCollection::redo() {
			if (it_ != history_.end()) {
				it_++;
			}
			return getCurrent();
		}

		Mask& core::segmentation::MaskCollection::getCurrent() {
			if (history_.empty()) {
				return Mask(rows_, cols_);
			}
			Mask& mask = *(it_--);
			it_++;
			return mask;
		}

		void MaskCollection::setBasenamePath(const std::string& basename) {
			basename_path_ = basename;
		}

		std::string MaskCollection::saveCollection(const std::string& basename) {
			basename_path_ = basename;
			saveCollection();
			return "";
		}

		std::string MaskCollection::saveCollection() {
			if (basename_path_.empty()) {
				return "Cannot save mask because basename path is missing";
			}
			std::cout << "Saving " << basename_path_ << std::endl;
			return "";
		}

		std::string MaskCollection::loadCollection(const std::string& basename) {
			basename_path_ = basename;
			loadCollection();
			return "";
		}

		std::string MaskCollection::loadCollection() {
			if (basename_path_.empty()) {
				return "Cannot load mask because basename path is missing";
			}			
			return "";
		}

		Mask::Mask(int rows, int cols) : rows_(rows), cols_(cols) {
			setDimensions(rows, cols);
		}

		void Mask::setDimensions(int rows, int cols) {
			rows_ = rows;
			cols_ = cols;
			data_ = cv::Mat::zeros(cv::Size(rows, cols), CV_8U);
		}

		Mask Mask::copy() {
			Mask mask(rows_, cols_);
			data_.copyTo(mask.data_);
			mask.rows_ = rows_;
			mask.cols_ = cols_;
			mask.filename_ = filename_;
			return mask;
		}

		void Mask::setData(cv::Mat& data) {
			data_ = data;
		}

		void Mask::intersect_with(const Mask& mat) {
			if (mat.data_.rows != rows_ || mat.data_.cols != cols_) {
				return;
			}

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

		void brushToMask(float radius, ImVec2 position, Mask& mask, int value) {
			cv::Point circle_center(position.x, position.y);
			cv::circle(mask.getData(), circle_center, radius, value, -1);
		}
	}
}