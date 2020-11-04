#include "extract_view_from_dicom.h"


std::shared_ptr<Job> core::dataset::extract_view(const std::vector<cv::Mat>& matrices, jobResultFct result_fct, float position, bool horizontal) {

    jobFct job = [=](float &progress, bool &abort) -> std::shared_ptr<JobResult> {
        auto view_result = std::make_shared<DicomViewResult>();

        // Don't want to reconstruct images with less than 5 slices
        if (matrices.size() < 5)
            return view_result;

        if (position < 0.f || position > 1.f)
            return view_result;

        int pos;
        if (horizontal) {
            view_result->data = cv::Mat(matrices.size(), matrices[0].cols, CV_16S);
            pos = (int)((float)matrices[0].rows * position);
        }
        else {
            view_result->data = cv::Mat(matrices[0].rows, matrices.size(), CV_16S);
            pos = (int)((float)matrices[0].cols * position);
        }
        auto& view = view_result->data;

        int i = 0;
        for (const auto& mat : matrices) {
            if (horizontal) {
                cv::Mat tmp = mat.row(pos);
                std::copy(tmp.begin<short>(), tmp.end<short>(), view.row(i).begin<short>());
            }
            else {
                cv::Mat tmp = mat.col(pos);
                std::copy(tmp.begin<short>(), tmp.end<short>(), view.col(i).begin<short>());
            }
            i++;
        }
        if (!horizontal)
            view = view.t();

        return view_result;
    };
    return JobScheduler::getInstance().addJob("build_dicom_view", job, result_fct);
}