#include "segmentation.h"

core::segmentation::Segmentation::Segmentation(const std::string& name, const std::string& description)
	: name_(name), description_(description)
{
}

void core::segmentation::Segmentation::addDicom(std::shared_ptr<DicomSeries> dicom) {
	if (segmentations_.find(dicom) == segmentations_.end()) {
		if (dicom->size() > 0) {
			segmentations_[dicom] = MaskCollection();
		}
	}
}

//void core::segmentation::Segmentation::addDicom(std::shared_ptr<DicomSeries> dicom, MaskCollection masks) {
//	segmentations_[dicom] = masks.copy();
//}

void core::segmentation::Segmentation::removeDicom(std::shared_ptr<DicomSeries> dicom) {
	if (segmentations_.find(dicom) != segmentations_.end())
		segmentations_.erase(dicom);
}
