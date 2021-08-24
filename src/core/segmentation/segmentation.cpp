#include "segmentation.h"
#include "python/py_api.h"
#include "pybind11/pybind11.h"

namespace core {
	namespace segmentation {
		namespace py = pybind11;

		Segmentation::Segmentation(const std::string& name, const std::string& description, ImVec4 color)
			: name_(name), description_(description), color_(color)
		{
		}

		Segmentation::~Segmentation() {
		}

		std::shared_ptr<MaskCollection> Segmentation::getMask(std::shared_ptr<DicomSeries> dicom) {
			addDicom(dicom);
			return segmentations_[dicom];
		}

		void Segmentation::setMaskColor(const std::vector<float> color) {
			color_.x = color[0];
			color_.y = color[1];
			color_.z = color[2];
			color_.w = color[3];
		}

		void Segmentation::setMaskColor(const float color[4]) {
			color_.x = color[0];
			color_.y = color[1];
			color_.z = color[2];
			color_.w = color[3];
		}

		void Segmentation::clear() {
			segmentations_.clear();
		}

		std::string Segmentation::getMaskBasename(std::shared_ptr<DicomSeries> dicom) {
			std::string id = dicom->getId();
			std::string name;
			auto state = PyGILState_Ensure();
			try {
				auto script = py::module::import("python.scripts.segmentation");
				name = script.attr("get_mask_path")(id, filename_, stripped_name_).cast<std::string>();
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
				name = "";
			}
			PyGILState_Release(state);
			return name;
		}

		void Segmentation::addDicom(std::shared_ptr<DicomSeries> dicom) {
			if (segmentations_.find(dicom) == segmentations_.end()) {
				if (dicom->size() > 0) {
					segmentations_[dicom] = std::make_shared<MaskCollection>();
					segmentations_[dicom]->setBasenamePath(getMaskBasename(dicom));
				}
			}
		}

		//void Segmentation::addDicom(std::shared_ptr<DicomSeries> dicom, MaskCollection masks) {
		//	segmentations_[dicom] = masks.copy();
		//}

		void Segmentation::removeDicom(std::shared_ptr<DicomSeries> dicom) {
			if (segmentations_.find(dicom) != segmentations_.end())
				segmentations_.erase(dicom);
		}

	}
}