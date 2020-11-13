#include "dataset.h"

core::dataset::Group::Group(const std::string& name) : name_(name) {

}

void core::dataset::Group::addDicom(std::shared_ptr<DicomSeries> dicom) {
	dicoms_.insert(dicom);
}

void core::dataset::Group::removeDicom(std::shared_ptr<DicomSeries> dicom) {
	dicoms_.erase(dicom);
}

core::dataset::Group& core::dataset::Dataset::createGroup(const std::string& name) {
	groups_.emplace_back(Group(name));
	return *(--groups_.end());
}
