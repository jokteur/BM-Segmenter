#include "dicom.h"
#include "dataset/dicom_to_image.h"

core::DicomSeries::DicomSeries(file_format format) {
    format_ = format;
}

core::DicomSeries::DicomSeries(std::vector<std::string> paths, const std::string& id, file_format format) : images_path_(paths), id_(id), format_(format) {
    init();
}

void core::DicomSeries::setPaths(const std::vector<std::string> &paths) {
    images_path_ = paths;
    init();
}

void core::DicomSeries::init() {
    cancelPendingJobs();
    data_.clear();
    for (auto &_ : images_path_) {
        data_.emplace_back(Dicom());
    }
}


core::DicomSeries::~DicomSeries() {
    cancelPendingJobs();
    unloadData();
}

void core::DicomSeries::free_memory(int index) {
    if (data_[index].is_set) {
        data_[index].data = cv::Mat();
        data_[index].is_set = false;
    }
}
void core::DicomSeries::cancelPendingJobs() {
    for (auto& id : pending_jobs_)
        JobScheduler::getInstance().stopJob(id);
    pending_jobs_.clear();
}

void core::DicomSeries::loadAll(bool force_load) {
    if (!load_all_ || force_load) {
        load_all_ = true;
        for (int i = 0; i < data_.size(); i++) {
            loadCase(i, false);
        }
    }
}

void core::DicomSeries::loadCase(float percentage, bool force_replace, const std::function<void(const Dicom&)>& when_finished_fct) {
    if (percentage >= 0.f && percentage <= 1.f) {
        loadCase((int) ((float) (data_.size() - 1) * percentage), force_replace, when_finished_fct);
    }
}

void core::DicomSeries::loadCase(int index, bool force_replace, const std::function<void(const Dicom&)>& when_finished_fct) {
    if (index >= 0 && index < data_.size()) {
        if (!load_all_) {
            cancelPendingJobs();
        }
        //std::cout << "Select " << index << std::endl;
        data_[index].error_message.clear();
        if (!force_replace && data_[index].is_set) {
            when_finished_fct(data_[index]);
            return;
        }

        jobResultFct when_finished = [=] (const std::shared_ptr<JobResult> &result) {
            auto dicom_result = std::dynamic_pointer_cast<::core::dataset::DicomResult>(result);
            if (dicom_result->success) {
                auto& dicom = dicom_result->image;
                cv::Rect ROI(0, 0, dicom.data.rows, dicom.data.cols);
                if (crop_x_.x != crop_x_.y && crop_y_.x != crop_y_.y) {
                    ROI = {
                            (int)((float)dicom.data.rows*crop_x_.x/100.f),
                            (int)((float)dicom.data.cols*crop_y_.x/100.f),
                            (int)((float)dicom.data.rows*(crop_x_.y - crop_x_.x)/100.f),
                            (int)((float)dicom.data.rows*(crop_y_.y - crop_y_.x)/100.f)
                    };
                }
                data_[index].data = dicom.data(ROI);
                data_[index].is_set = true;
            }
            else {
                data_[index].error_message = dicom_result->error_msg;
            }
            selected_index_ = index;
            when_finished_fct(data_[index]);
            pending_jobs_.erase(result->id);
        };

        if (format_ == F_NP) {
            auto job = dataset::npy_to_matrix(images_path_[index], when_finished);
            pending_jobs_.insert(job->id);
        }
        else {
            auto job = dataset::dicom_to_matrix(images_path_[index], when_finished);
            pending_jobs_.insert(job->id);
        }
    }
}

void core::DicomSeries::unloadData(bool keep_current) {
    load_all_ = false;
    for(int i = 0;i < data_.size();i++) {
        if (keep_current && selected_index_ == i)
            continue;
        free_memory(i);
    }
}

void core::DicomSeries::cleanData() {
    if (!load_all_) {
        unloadData();
    }
}

core::Dicom &core::DicomSeries::getCurrentDicom() {
    return data_[selected_index_];
}

void core::DicomSeries::setCrops(ImVec2 crop_x, ImVec2 crop_y, bool no_reload) {
    crop_x_ = crop_x;
    crop_y_ = crop_y;
    if (!no_reload)
        reload();
}

void core::DicomSeries::setCropX(ImVec2 crop_x, bool no_reload) {
    crop_x_ = crop_x;
    if (!no_reload)
        reload();
}

void core::DicomSeries::setCropY(ImVec2 crop_y, bool no_reload) {
    crop_y_ = crop_y;
    if (!no_reload)
        reload();
}

bool core::DicomSeries::isReady() {
    return pending_jobs_.empty();
}

void core::DicomSeries::reload() {
    if (load_all_) {
        loadAll();
    }
    else {
        loadCase(selected_index_, true);
    }
}

void core::DicomSeries::eraseCurrent() {
    if (!load_all_) {
        free_memory(selected_index_);
    }
}

std::pair<std::string, std::string> core::parse_dicom_id(const std::string& id) {
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

bool core::OrderDicom::operator() (const std::shared_ptr<DicomSeries>& dicom1, const std::shared_ptr<DicomSeries>& dicom2) const {
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

