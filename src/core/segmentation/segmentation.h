#pragma once

#include <string>
#include <vector>

#include "core/dicom.h"

namespace core {
	namespace segmentation {
		class Segmentation {
		private:
			std::string name_;
			std::string description_;

			std::vector<std::shared_ptr<DicomSeries>> dicoms_;
		public:
			Segmentation(const std::string& name, const std::string& description);
			Segmentation() = default;

			/**
			 * Sets the name of the segmentation
			*/
			void setName(const std::string& name) { name_ = name; }

			/**
			 * Sets the description of the segmentation
			*/
			void setDescription(const std::string& description) { description_ = description; }
		};
	}
}