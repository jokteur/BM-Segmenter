#pragma once

#include <string>
#include <vector>
#include <map>

#include "core/dicom.h"
#include "core/segmentation/mask.h"

namespace core {
	namespace segmentation {
		class Segmentation {
		private:
			std::string name_;
			std::string stripped_name_;
			std::string description_;
			std::string filename_;

			std::map<std::shared_ptr<DicomSeries>, MaskCollection> segmentations_;
		public:
			Segmentation(const std::string& name, const std::string& description);
			Segmentation() = default;

			void setFilename(const std::string& filename) { filename_ = filename; }
			void setStrippedName(const std::string& name) { stripped_name_ = name; }

			std::string getName() { return name_; }
			std::string getFilename() { return filename_; }
			std::string getStrippedName() { return stripped_name_; }
			std::string getDescription() { return description_; }

			std::map<std::shared_ptr<DicomSeries>, MaskCollection>& getMasks() { return segmentations_; }

			/**
			 * Sets the name of the segmentation
			*/
			void setName(const std::string& name) { name_ = name; }

			/**
			 * Sets the description of the segmentation
			*/
			void setDescription(const std::string& description) { description_ = description; }

			//std::string saveMask(std::shared_ptr<DicomSeries> dicom);

			std::string getMaskBasename(std::shared_ptr<DicomSeries> dicom);

			void addDicom(std::shared_ptr<DicomSeries> dicom);

			//void addDicom(std::shared_ptr<DicomSeries> dicom, MaskCollection masks);

			void removeDicom(std::shared_ptr<DicomSeries> dicom);
		};
	}
}