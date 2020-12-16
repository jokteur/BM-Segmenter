#include "dicom.h"
#include "dataset/dicom_to_image.h"

#include <algorithm>

namespace core {

    int DicomSeries::num_loaded_ = 0;

    DicomSeries::DicomSeries(file_format format) {
        format_ = format;
    }

    DicomSeries::DicomSeries(std::vector<std::string> paths, const std::string& id, file_format format) : images_path_(paths), id_(id), format_(format) {
        init();
    }

    void DicomSeries::setPaths(const std::vector<std::string>& paths) {
        images_path_ = paths;
        init();
    }

    void DicomSeries::setId(const std::string& id) {
        id_ = id;
        id_pair_ = parse_dicom_id(id);
    }

    void DicomSeries::init() {
        cancelPendingJobs();
        data_.clear();
        id_pair_ = parse_dicom_id(id_);
        for (auto& _ : images_path_) {
            data_.emplace_back(Dicom());
        }
    }


    DicomSeries::~DicomSeries() {
        forceClean();
    }

    void DicomSeries::free_memory(int index) {
        if (data_[index].is_set) {
            data_[index].data = cv::Mat();
            data_[index].is_set = false;
            num_loaded_--;
        }
    }
    void DicomSeries::cancelPendingJobs() {
        for (auto& id : pending_jobs_)
            JobScheduler::getInstance().stopJob(id);
        pending_jobs_.clear();
    }

    void DicomSeries::loadAll(const std::function<void(const Dicom&)>& when_finished_fct) {
        for (int i = 0; i < data_.size(); i++) {
            load_case(i, false, true, when_finished_fct);
        }
        load_all_ = true;
    }

    jobId DicomSeries::loadCase(float percentage, bool force_replace, const std::function<void(const Dicom&)>& when_finished_fct) {
        if (percentage >= 0.f && percentage <= 1.f) {
            return load_case((int)((float)(data_.size() - 1) * percentage), force_replace, false, when_finished_fct);
        }
        return 0;
    }

    jobId DicomSeries::loadCase(int index, bool force_replace, const std::function<void(const Dicom&)>& when_finished_fct) {
        return load_case(index, force_replace, false, when_finished_fct);
    }

    jobId DicomSeries::load_case(int index, bool force_replace, bool keep_previous, const std::function<void(const Dicom&)>& when_finished_fct) {
        if (index >= 0 && index < data_.size()) {
            if (!load_all_) {
                cancelPendingJobs();
            }

            // Unload previous case if it was loaded
            if (selected_index_ != index && !keep_previous)
                unloadCase(selected_index_);

            data_[index].error_message.clear();
            if (!force_replace && data_[index].is_set) {
                add_one_to_ref(index);
                when_finished_fct(data_[index]);
                return 0;
            }

            jobResultFct when_finished = [=](const std::shared_ptr<JobResult>& result) {
                auto dicom_result = std::dynamic_pointer_cast<dataset::DicomResult>(result);
                if (dicom_result->success) {
                    if (!data_[index].is_set) {
                        auto& dicom = dicom_result->image;
                        cv::Rect ROI(0, 0, dicom.data.rows, dicom.data.cols);
                        if (crop_x_.x != crop_x_.y && crop_y_.x != crop_y_.y) {
                            ROI = {
                                    (int)((float)dicom.data.rows * crop_x_.x / 100.f),
                                    (int)((float)dicom.data.cols * crop_y_.x / 100.f),
                                    (int)((float)dicom.data.rows * (crop_x_.y - crop_x_.x) / 100.f),
                                    (int)((float)dicom.data.rows * (crop_y_.y - crop_y_.x) / 100.f)
                            };
                        }
                        data_[index].data = dicom.data(ROI);
                        data_[index].is_set = true;
                        selected_index_ = index;
                        add_one_to_ref(index); // Add one reference to this index, only if the job is finished
                        num_loaded_++;
                    }
                }
                else {
                    data_[index].error_message = dicom_result->error_msg;
                }
                when_finished_fct(data_[index]);
                pending_jobs_.erase(result->id);
            };

            jobId id;
            if (format_ == F_NP) {
                auto job = dataset::npy_to_matrix(images_path_[index], when_finished);
                id = job->id;
                pending_jobs_.insert(job->id);
            }
            else {
                auto job = dataset::dicom_to_matrix(images_path_[index], when_finished);
                pending_jobs_.insert(job->id);
                id = job->id;
            }
            return id;
        }
        return 0;
    }

    void DicomSeries::unloadCase(int index) {
        if (index == -1) {
            index = selected_index_;
        }
        set_ref(index);
        if (ref_counter_[index] > 0) {
            remove_one_to_ref(index);
            if (ref_counter_[index] == 0)
                free_memory(index);
        }
    }

    void DicomSeries::unloadAll(bool keep_current) {
        load_all_ = false;
        for (int i = 0; i < data_.size(); i++) {
            unloadCase(i);
        }
    }

    void DicomSeries::forceClean() {
        load_all_ = false;
        cancelPendingJobs();
        for (int i = 0; i < data_.size(); i++) {
            ref_counter_[i] = 1;
            free_memory(i);
        }
    }

    int DicomSeries::rows() {
        if (data_.empty())
            return 0;
        return data_[0].data.rows;
    }

    int DicomSeries::cols() {
        if (data_.empty())
            return 0;
        return data_[0].data.cols;
    }

    Dicom& DicomSeries::getCurrentDicom() {
        return data_[selected_index_];
    }

    void DicomSeries::setCrops(ImVec2 crop_x, ImVec2 crop_y, bool no_reload) {
        crop_x_ = crop_x;
        crop_y_ = crop_y;
        if (!no_reload)
            reload();
    }

    void DicomSeries::setCropX(ImVec2 crop_x, bool no_reload) {
        crop_x_ = crop_x;
        if (!no_reload)
            reload();
    }

    void DicomSeries::setCropY(ImVec2 crop_y, bool no_reload) {
        crop_y_ = crop_y;
        if (!no_reload)
            reload();
    }

    bool DicomSeries::isReady() {
        return pending_jobs_.empty();
    }

    void DicomSeries::reload() {
        if (load_all_) {
            loadAll();
        }
        else {
            loadCase(selected_index_, true);
        }
    }

    void DicomSeries::set_ref(int idx) {
        if (ref_counter_.find(idx) == ref_counter_.end()) {
            ref_counter_[idx] = 0;
        }
    }

    void DicomSeries::add_one_to_ref(int idx) {
        if (ref_counter_.find(idx) != ref_counter_.end()) {
            ref_counter_[idx]++;
        }
        else {
            ref_counter_[idx] = 1;
        }
    }

    void DicomSeries::remove_one_to_ref(int idx) {
        if (ref_counter_.find(idx) != ref_counter_.end()) {
            ref_counter_[idx]--;
            if (ref_counter_[idx] < 0) {
                std::cout << "Removed one ref too much" << std::endl;
            }
        }
        else {
            std::cout << "Removing without ref existing" << std::endl;
        }
    }

    std::pair<std::string, std::string> parse_dicom_id(const std::string& id) {
        int pos = id.find("___");

        if (pos == -1) {
            return std::make_pair<std::string, std::string>(std::string(id), "");
        }
        else {
            return std::make_pair<std::string, std::string>(id.substr(0, pos), id.substr(pos + 3));
        }
    }

    inline int first_non_numeric(const std::string& str) {
        int i = 0;
        for (char chr : str) {
            if (chr < 48 || chr > 57)
                return i;
            i++;
        }
        return i;
    }

    bool OrderDicom::operator() (const std::shared_ptr<DicomSeries>& dicom1, const std::shared_ptr<DicomSeries>& dicom2) const {
        std::string id1 = dicom1->getId();
        std::string id2 = dicom2->getId();

        std::string name1 = parse_dicom_id(id1).first;
        std::string name2 = parse_dicom_id(id2).first;

        int i = 0;
        std::string number1 = name1.substr(0, first_non_numeric(name1));
        std::string number2 = name2.substr(0, first_non_numeric(name2));

        if (number1.size() == number2.size()) {
            return id1 < id2;
        }
        else {
            return number1.size() < number2.size();
        }
    }

}