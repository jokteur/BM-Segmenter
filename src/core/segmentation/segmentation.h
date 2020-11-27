#pragma once

#include <string>
#include <vector>
#include <map>

#include "events.h"

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
			ImVec4 color_ = { 1.f, 0.f, 0.f, 0.5f };

			std::map<std::shared_ptr<DicomSeries>, MaskCollection> segmentations_;
		public:
			Segmentation(const std::string& name, const std::string& description, ImVec4 color = { 1.f, 0.f, 0.f, 0.5f });
			Segmentation() = default;

			void setFilename(const std::string& filename) { filename_ = filename; }
			void setStrippedName(const std::string& name) { stripped_name_ = name; }

			std::string getName() { return name_; }
			std::string getFilename() { return filename_; }
			std::string getStrippedName() { return stripped_name_; }
			std::string getDescription() { return description_; }

			std::map<std::shared_ptr<DicomSeries>, MaskCollection>& getMasks() { return segmentations_; }

			void setMaskColor(const ImVec4 color) { color_ = color; }
			void setMaskColor(const std::vector<float> color);
			void setMaskColor(const float color[4]);

			ImVec4& getMaskColor() { return color_; }

			/**
			 * Unload from memory all masks that may be present
			*/
			void clear();

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

		class ReloadSegmentationEvent : public Event {
		public:
			explicit ReloadSegmentationEvent() : Event("segmentation/reload") {
			}
		};

		class SelectSegmentationEvent : public Event {
		private:
			std::shared_ptr<Segmentation> seg_ = nullptr;
		public:
			SelectSegmentationEvent(std::shared_ptr<Segmentation> seg) : seg_(seg), Event("segmentation/select") {}

			std::shared_ptr<Segmentation> getSegmentation() { return seg_; }
		};
	}
}