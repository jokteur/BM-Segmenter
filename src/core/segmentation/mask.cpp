#include "mask.h"
#include "core/np2cv.h"
#include "pybind11/stl.h"

namespace core {
    namespace segmentation {
        namespace py = pybind11;
        using namespace py::literals;

        void npy_buffer_to_cv(py::object object, cv::Mat &mat) {
            auto buffer = object.cast<py::buffer>();
            py::buffer_info info = buffer.request();
            int rows = info.shape[0];
            int cols = info.shape[1];

            auto array = static_cast<unsigned char *>(info.ptr);
            mat.create(rows, cols, CV_8U);
            memcpy(mat.data, array, sizeof(unsigned char) * rows * cols);
        }

        Mask::Mask(int rows, int cols, bool ones) : rows_(rows), cols_(cols) {
            setDimensions(rows, cols, false, ones);
        }

        void Mask::setDimensions(int rows, int cols, bool no_build, bool ones) {
            rows_ = rows;
            cols_ = cols;
            if (!no_build) {
                if (ones)
                    data_ = cv::Mat::ones(cv::Size(rows, cols), CV_8U);
                else
                    data_ = cv::Mat::zeros(cv::Size(rows, cols), CV_8U);
            }
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

        void Mask::setData(cv::Mat &data) {
            data_ = data;
            rows_ = data.rows;
            cols_ = data.cols;
            is_empty_ = false;
        }

        void Mask::intersect_with(const Mask &other) {
            if (other.rows_ != rows_ || other.cols_ != cols_) {
                return;
            }
            uchar *data_array = data_.data;
            uchar *other_data_array = other.data_.data;

            for (int array_index = 0; array_index < rows_ * cols_; array_index++) {
                if (other_data_array[array_index] == 0) {
                    data_array[array_index] = 0;
                }
            }
        }

        void Mask::union_with(const Mask &other) {
            if (other.rows_ != rows_ || other.cols_ != cols_) {
                return;
            }
            uchar *data_array = data_.data;
            uchar *other_data_array = other.data_.data;

            for (int array_index = 0; array_index < rows_ * cols_; array_index++) {
                if (other_data_array[array_index] == 1) {
                    data_array[array_index] = 1;
                }
            }
        }

        void Mask::difference_with(const Mask &other) {
            if (other.rows_ != rows_ || other.cols_ != cols_) {
                return;
            }
            uchar *data_array = data_.data;
            uchar *other_data_array = other.data_.data;

            for (int array_index = 0; array_index < rows_ * cols_; array_index++) {
                if (other_data_array[array_index] == 1) {
                    data_array[array_index] = 0;
                }
            }
        }


        void Mask::combine_with(const Mask &other) {
            if (other.rows_ != rows_ || other.cols_ != cols_) {
                return;
            }
            uchar *data_array = data_.data;
            uchar *other_data_array = other.data_.data;
            for (int array_index = 0; array_index < rows_ * cols_; array_index++) {
                if (data_array[array_index] == 0 && other_data_array[array_index] == 0) {
                    data_array[array_index] = 0;
                }
                if (data_array[array_index] == 1 && other_data_array[array_index] == 0) {
                    data_array[array_index] = 1;
                }
                if (data_array[array_index] == 0 && other_data_array[array_index] == 1) {
                    data_array[array_index] = 2;
                }
                if (data_array[array_index] == 1 && other_data_array[array_index] == 1) {
                    data_array[array_index] = 3;
                }
            }
        }

        bool Mask::isEqualTo(const Mask &other) {
            if (other.rows_ != rows_ || other.cols_ != cols_) {
                return false;
            }
            uchar *data_array = data_.data;
            uchar *other_data_array = other.data_.data;

            for (int array_index = 0; array_index < rows_ * cols_; array_index++) {
                if (data_array[array_index] != other_data_array[array_index]) {
                    return false;
                }
            }
            return true;
        }

        void Mask::invert() {
            uchar *data_array = data_.data;
            for (int pixel_index = 0; pixel_index < rows_ * cols_; pixel_index++) {
                data_array[pixel_index] = !data_array[pixel_index];
            }
        }

        void Mask::remove_small_objects(int min_object_size) {
            cv::Mat labels;
            cv::Mat stats;
            cv::Mat centroids;

            int regions_count = cv::connectedComponentsWithStats(data_, labels, stats, centroids, 4);

            auto *labels_array = (int *) labels.data;
            unsigned char *mask_array = data_.data;

            for (int region_index = 0; region_index < regions_count; region_index++) {
                if (stats.at<int>(region_index, cv::CC_STAT_AREA) < min_object_size) {
                    for (int pixel_index = 0; pixel_index < cols() * rows(); pixel_index++) {
                        if (labels_array[pixel_index] == region_index) {
                            mask_array[pixel_index] = 0;
                        }
                    }
                }
            }
        }

        void Mask::opening(int size) {
            int diameter = size * 2 + 1;
            auto kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(diameter, diameter));
            cv::morphologyEx(data_, data_, cv::MORPH_OPEN, kernel);
        }

        void Mask::closing(int size) {
            int diameter = size * 2 + 1;
            auto kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(diameter, diameter));
            cv::morphologyEx(data_, data_, cv::MORPH_CLOSE, kernel);
        }

        int Mask::top_row() {
            for (int row_index = 0; row_index < rows_; row_index++)
                for (int col_index = 0; col_index < cols_; col_index++) {
                    if (get_pixel(row_index, col_index) == 1) {
                        return row_index;
                    }
                }
            return rows_;
        }

        int Mask::bottom_row() {
            for (int row_index = rows_ - 1; row_index >= 0; row_index--)
                for (int col_index = 0; col_index < cols_; col_index++) {
                    if (get_pixel(row_index, col_index) == 1) {
                        return row_index;
                    }
                }
            return 0;
        }

        unsigned char Mask::get_pixel(int row_index, int col_index) {
            if (0 <= row_index && row_index < rows_ && 0 <= col_index && col_index < cols_)
                return data_.at<unsigned char>(row_index, col_index);
            else
                return 0;
        }

        void Mask::set_pixel(int row_index, int col_index, unsigned char value) {
            data_.at<unsigned char>(row_index, col_index) = value;
        }

        void Mask::fill(unsigned char value) {
            for (int row_index = 0; row_index < rows_; row_index++)
                for (int col_index = 0; col_index < cols_; col_index++) {
                    set_pixel(row_index, col_index, value);
                }
        }

        cv::Point Mask::find_first_pixel(int first_row, int last_row, int first_col, int last_col) {

            bool top_to_bottom = first_row < last_row;
            bool left_to_right = first_col < last_col;

            for (int row = first_row; row != last_row; top_to_bottom ? row++ : row--) {
                for (int col = first_col; col != last_col; left_to_right ? col++ : col--) {
                    if (get_pixel(row, col)) {
                        return cv::Point(col, row);
                    }
                }
            }
            return cv::Point(0, 0);
        }

        void Mask::convert_to_binary(int value_for_one) {
            uchar *data_array = data_.data;
            for (int pixel_index = 0; pixel_index < rows_ * cols_; pixel_index++) {
                data_array[pixel_index] = data_array[pixel_index] == value_for_one ? 1 : 0;
            }
        }


        int MaskCollection::global_counter_ = 0;

        inline int MaskCollection::get_num_refs() {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
            int count = 0;
            for (auto &pair: ref_counter_)
                count += pair.second;
            return count;
        }

        void MaskCollection::add_one_to_ref(const std::string &id) {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
            set_ref(id);
            ref_counter_[id]++;
        }

        void MaskCollection::remove_one_to_ref(const std::string &id) {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
            set_ref(id);
            ref_counter_[id]--;
            if (ref_counter_[id] < 0) {
                std::cout << "Removed one ref too much" << std::endl;
            }
        }

        void MaskCollection::set_ref(const std::string &id) {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
            if (ref_counter_.find(id) == ref_counter_.end()) {
                ref_counter_[id] = 0;
            }
        }

        MaskCollection::MaskCollection(int rows, int cols, int max_size) : rows_(rows), cols_(cols),
                                                                           max_size_(max_size) {
            it_ = history_.end();
        }

        MaskCollection::MaskCollection(const MaskCollection &other) {
            basename_path_ = other.basename_path_;
            validated_by_ = other.validated_by_;
            rows_ = other.rows_;
            cols_ = other.cols_;
            max_size_ = other.max_size_;
            is_valid_ = other.is_valid_;
            keep_ = other.keep_;
            is_loading_ = other.is_loading_;

            ref_counter_ = other.ref_counter_;
            tmp_ = other.tmp_;
            prediction_ = other.prediction_;
            validated_ = other.validated_;
        }

        MaskCollection::~MaskCollection() {
            history_.clear();
            cancelPendingJobs();
        }

        void MaskCollection::push(const Mask &mask) {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
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

        std::string MaskCollection::loadData(bool immediate, bool clear_history, const std::string &id,
                                             const std::function<void()> &when_finished_fct,
                                             Job::jobPriority priority) {
            //is_valid_ = true;

            {
                std::lock_guard<std::recursive_mutex> guard(ref_mutex_);
                if (is_set_) {
                    if (!id.empty()) {
                        add_one_to_ref(id);
                    }
                    when_finished_fct();
                    return "";
                }
            }
            if (!immediate) {
                cancelPendingJobs(true, id);
            }

            jobFct job = [=](float &progress, bool &abort) -> std::shared_ptr<JobResult> {
                auto result = std::make_shared<JobResult>();

                auto state = PyGILState_Ensure();
                try {
                    std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
                    py::module script = py::module::import("python.scripts.segmentation");

                    auto &dict = script.attr("load_mask_collection")(basename_path_).cast<py::dict>();

                    clearHistory();

                    if (dict.contains("current")) {
                        Mask mask;
                        npy_buffer_to_cv(dict["current"], mask.getData());
                        mask.setNotEmpty();
                        mask.setState(Mask::MASK_EDITED);
                        mask.updateDimensions();
                        push(mask);
                    }
                    if (dict.contains("predicted")) {
                        npy_buffer_to_cv(dict["predicted"], prediction_.getData());
                        prediction_.setState(Mask::MASK_PREDICTION);
                        prediction_.setNotEmpty();
                        prediction_.updateDimensions();
                    }
                    if (dict.contains("validated")) {
                        npy_buffer_to_cv(dict["validated"], validated_.getData());
                        prediction_.setState(Mask::MASK_VALIDATED);
                        validated_.setNotEmpty();
                        validated_.updateDimensions();
                    }

                    auto &users = dict["users"].cast<std::vector<std::string>>();
                    for (auto &user: users) {
                        setValidatedBy(user);
                    }

                    is_valid_ = true;
                }
                catch (const std::exception &e) {
                    py::print(e.what());
                    result->err = e.what();
                }

                PyGILState_Release(state);
                return result;
            };

            jobResultFct when_finished = [=](const std::shared_ptr<JobResult> &result) {
                std::lock_guard<std::recursive_mutex> guard(ref_mutex_);
                is_set_ = true;
                if (!id.empty()) {
                    add_one_to_ref(id);
                }
                when_finished_fct();
            };

            if (immediate) {
                float a = 0.f;
                bool b = true;
                auto &res = job(a, b);
                if (!res->err.empty())
                    std::cout << "Error:" << res->err << std::endl;
                when_finished(res);
                return res->err;
            } else {
                auto &job_ref = JobScheduler::getInstance().addJob("dicom_to_image", job, when_finished, priority);
                pending_jobs_.insert(job_ref->id);
            }
            return "";
        }

        void MaskCollection::unloadData(bool force, const std::string &id) {
            std::lock_guard<std::recursive_mutex> guard(ref_mutex_);

            cancelPendingJobs(true);

            int count = get_num_refs();
            if (count > 0 && !id.empty()) {
                remove_one_to_ref(id);
            }

            if (count == 0 || force) {
                //std::cout << "unload " << std::to_string(global_counter_) << std::endl;
                std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
                prediction_ = Mask();
                validated_ = Mask();
                tmp_ = Mask();
                rows_ = 0;
                cols_ = 0;
                clearHistory();
                is_set_ = false;
                is_valid_ = false;
            }
        }

        Mask MaskCollection::getTmp() {
            Mask ret = tmp_;
            tmp_ = Mask();
            return tmp_;
        }

        void MaskCollection::cancelPendingJobs(bool no_ref_counting, const std::string &id) {
            std::lock_guard<std::recursive_mutex> guard(ref_mutex_);
            int count = get_num_refs();
            if (!no_ref_counting && !id.empty()) {
                remove_one_to_ref(id);
            }
            if (count == 0) {
                for (auto &id: pending_jobs_) {
                    JobScheduler::getInstance().stopJob(id);
                }
                pending_jobs_.clear();
            }
        }

        int MaskCollection::numPendingJobs() {
            return pending_jobs_.size();
        }

        std::string MaskCollection::saveCollection(const std::string &basename) {
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
                auto &current = getCurrent();

                py::module seg = py::module::import("python.scripts.segmentation");
                std::vector<std::string> users;
                for (auto &user: validated_by_) {
                    users.push_back(user);
                }
                seg.attr("save_mask_collection")(users, current.getData(), validated_.getData(), prediction_.getData(),
                                                 basename_path_);
                is_set_ = true;
            }
            catch (const std::exception &e) {
                std::cout << e.what() << std::endl;
                error_msg = e.what();
            }
            PyGILState_Release(state);

            return error_msg;
        }

        MaskCollection MaskCollection::copy() {
            MaskCollection collection(rows_, cols_);
            collection.rows_ = rows_;
            collection.cols_ = cols_;
            for (auto &mask: history_) {
                collection.history_.push_back(mask.copy());
            }
            collection.it_ = collection.history_.end();
            collection.prediction_ = prediction_.copy();
            collection.validated_ = validated_.copy();
            return collection;
        }

        void MaskCollection::clearHistory(bool force) {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
            if (is_valid_) {
                history_.clear();
                it_ = history_.end();
                //is_valid_ = false;
            }
        }

        void MaskCollection::setDimensions(std::shared_ptr<DicomSeries> dicom) {
            if (!dicom->getData().empty()) {
                auto &data = dicom->getData()[0].data;
                if (data.rows > 0 && data.cols > 0) {
                    std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
                    rows_ = data.rows;
                    cols_ = data.cols;
                    is_valid_ = true;
                    return;
                }
            }
            dicom->loadCase(0, false, [this, &dicom](const core::Dicom &dicom_res) {
                std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
                rows_ = dicom_res.data.rows;
                cols_ = dicom_res.data.cols;
                is_valid_ = true;
                dicom->unloadCase(0);
            });
        }

        void MaskCollection::lock() {
            ref_mutex_.lock();
        }

        void MaskCollection::unlock() {
            ref_mutex_.unlock();
        }

        Mask &MaskCollection::undo() {
            if (history_.empty()) {
                return Mask(rows_, cols_);
            }
            if (it_ != ++history_.begin()) {
                it_--;
            }
            return getCurrent();
        }

        Mask &MaskCollection::redo() {
            if (history_.empty()) {
                return Mask(rows_, cols_);
            }
            if (it_ != history_.end()) {
                it_++;
            }
            return getCurrent();
        }

        int MaskCollection::size() {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
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

        Mask &core::segmentation::MaskCollection::getCurrent(bool no_push) {
            std::lock_guard<std::recursive_mutex> lock(ref_mutex_);
            if (history_.empty()) {
                if (no_push) {
                    tmp_ = Mask();
                    return tmp_;
                }
                push_new();
            }
            iterator my_it = it_;
            return *(--my_it);
        }

        void MaskCollection::setBasenamePath(const std::string &basename) {
            basename_path_ = basename;
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

        Mask MaskCollection::getMostAdvancedMask() {
            if (is_validated_) {
                return validated_;
            }
            else if (!getCurrent().empty()) {
                return getCurrent();
            }
            else if (!prediction_.empty()) {
                return prediction_;
            }
            else {
                return Mask();
            }
        }


        Mask huThresholdMask(const cv::Mat &image_matrix, int min_hu, int max_hu, bool ignore_small_objects,
                             int closing_size, int opening_size) {

            Mask threshold_mask(image_matrix.rows, image_matrix.cols);

            auto *image_matrix_array = (short int *) image_matrix.data;
            uchar *threshold_mask_matrix_array = threshold_mask.getData().data;

            for (int pixel_index = 0; pixel_index < image_matrix.cols * image_matrix.rows; pixel_index++) {
                threshold_mask_matrix_array[pixel_index] =
                        min_hu <= image_matrix_array[pixel_index] && image_matrix_array[pixel_index] <= max_hu;
            }

            if (closing_size > 0) {
                threshold_mask.closing(closing_size);
            }

            if (opening_size > 0) {
                threshold_mask.opening(opening_size);
            }

            if (ignore_small_objects) {
                threshold_mask.remove_small_objects(100);
                threshold_mask.invert();
                threshold_mask.remove_small_objects(100);
                threshold_mask.invert();
            }

            return threshold_mask;
        }

        Mask vertebraDistanceMask(const cv::Mat &image_matrix, int vertebra_min_hu, int vertebra_min_distance) {
            Mask vertebra_distance_mask(image_matrix.rows, image_matrix.cols);

            auto *image_matrix_array = (short int *) image_matrix.data;
            uchar *vertebra_distance_mask_matrix_array = vertebra_distance_mask.getData().data;

            for (int pixel_index = 0; pixel_index < image_matrix.cols * image_matrix.rows; pixel_index++) {
                vertebra_distance_mask_matrix_array[pixel_index] = image_matrix_array[pixel_index] <= vertebra_min_hu;
            }

            int erosion_size = vertebra_min_distance * 2;
            auto element = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                     cv::Size(erosion_size, erosion_size));

            cv::erode(vertebra_distance_mask.getData(), vertebra_distance_mask.getData(), element);

            return vertebra_distance_mask;
        }


        void lassoSelectToMask(const std::vector<cv::Point> &pts, Mask &mask, int value) {
            if (!pts.empty() && mask.getData().rows > 0 && mask.getData().cols > 0)
                cv::fillPoly(mask.getData(), pts, value);
        }

        void brushToMask(float radius, ImVec2 position, Mask &mask, int value) {
            if (mask.getData().rows > 0 && mask.getData().cols > 0) {
                cv::Point circle_center(position.x, position.y);
                cv::circle(mask.getData(), circle_center, radius, value, -1);
            }
        }
    }
}