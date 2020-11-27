#pragma once

#include <string>
#include <list>
#include <memory>

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"

#include "core/dicom.h"

namespace core {
	namespace segmentation {
		struct Validator {
			std::string name;
		};

		/**
		 * Little container class around cv::Mat to store and manipulate masks 
		 * 
		*/
		class Mask {
		public:
			enum mask_info {MASK_EDITED, MASK_VALIDATED, MASK_PREDICTION};
		private:
			int rows_ = 0;
			int cols_ = 0;
			std::string filename_;
			cv::Mat data_;

			mask_info state_ = MASK_EDITED;

			bool is_empty_ = true;

			//void load_from_file(const std::string& filename);
		public:
			Mask(int rows, int cols);

			Mask() = default;

			/**
			 * Sets the dimensions of the mask
			 * Will reset any data that was previously present in the object
			*/
			void setDimensions(int rows, int cols);
			//Mask(const std::string& filename);

			/**
			* Copy constructors
			*/
			//void operator=(const Mask& other);
			//Mask(const Mask& other);

			Mask const copy() const;

			bool empty() { return is_empty_; }

			/**
			 * Returns the opencv matrix that represent the mask 
			*/
			cv::Mat& getData() { return data_; }

			void setData(cv::Mat& data);

			void setState(mask_info state) { state_ = state; }

			//void saveToFile(const std::string& filename);

			void intersect_with(const Mask& mat);
			void union_with(const Mask& mat);
			void difference_with(const Mask& mat);

			/**
			 * Returns the width of the mask
			*/
			int width() { return rows_; }

			/**
			 * Returns the width of the mask
			*/
			int height() { return cols_; }
		};

		/**
		* A mask collection allows to store the history of a segmentation,
		* along with the ML prediction and the validated mask
		*/
		class MaskCollection {
		private:
			using iterator = std::list<Mask>::iterator;

			iterator it_;
			std::list<Mask> history_;
			
			iterator current_;

			std::string basename_path_;
			std::set<std::string> validated_by_;

			int rows_ = 0;
			int cols_ = 0;
			int max_size_ = 40;

			bool is_valid_ = false;
			bool is_validated_ = false;
			bool keep_ = false;

			Mask prediction_;
			Mask validated_;
		public:
			MaskCollection() = default;
			MaskCollection(int rows, int cols, int max_size = 40);
			//MaskCollection(const std::string& filename);

			void push(const Mask& mask);
			void push_new();

			void loadData(bool keep = false);
			void unloadData(bool force = false);
			bool isValid() { return is_valid_; }

			MaskCollection copy();

			void setDimensions(int rows, int cols) { rows_ = rows; cols_ = cols; }
			void setDimensions(std::shared_ptr<DicomSeries> dicom);

			/**
			 * Goes back into the history of the mask collection
			*/
			Mask& undo();

			/**
			 * Goes in the future of the history of the mask collection
			*/
			Mask& redo();

			/**
			 * Returns the size of the history
			*/
			int size();

			bool isCursorBegin();
			bool isCursorEnd();

			/**
			 * Returns the current mask in the list
			 * If there is no mask in the collection, returns an empty mask
			*/
			Mask& getCurrent();

			void setBasenamePath(const std::string& basename);

			std::string saveCollection(const std::string& basename);
			std::string saveCollection();

			std::string loadCollection(const std::string& basename);
			std::string loadCollection();

			bool getIsValidated() { return is_validated_; }
			std::set<std::string> getValidatedBy() { return validated_by_; }

			void setValidated(Mask& mask) { validated_ = mask.copy(); }
			void setValidatedBy(std::string name);
			void removeAllValidatedBy();
			void removeValidatedBy(std::string name);

			void setPrediction(const Mask& mask) { prediction_ = prediction_; }
		};

		void buildHuMask(const cv::Mat& hu_mat, Mask& mask, int min, int max);

		void lassoSelectToMask(const std::vector<cv::Point>& pts, Mask& mask, int value = 1);

		//void boxSelectToMask(const ImVec2& top_left, const ImVec2& bottom_right, Mask& mask, int value = 1);
		void brushToMask(float brush_size, ImVec2 position, Mask& mask, int value = 1);
	}
}