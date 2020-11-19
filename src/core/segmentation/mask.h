#pragma once

#include <string>
#include <list>
#include <memory>

#include "opencv2/opencv.hpp"

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
			enum {MASK_EDITED, MASK_VALIDATED, MASK_PREDICTION};
		private:
			int width_ = 0;
			int height_ = 0;
			std::string filename_;
			cv::Mat data_;

			load_from_file(const std::string& filename);
		public:
			Mask(int width, int height);
			Mask(const std::string& filename);

			/**
			* Copy constructors
			*/
			operator=(const Mask& other);
			Mask(const Mask& other);

			Mask() = default;

			/**
			 * Returns the opencv matrix that represent the mask 
			*/
			cv::Mat& getData() { return data_; }

			void setData(cv::Mat& data);

			void saveToFile(const std::string& filename);

			/**
			 * Returns the width of the mask
			*/
			int width() { return width_; }

			/**
			 * Returns the width of the mask
			*/
			int height() { return height_; }
		};

		/**
		* A mask collection allows to store the history of a segmentation,
		* along with the ML prediction and the validated mask
		*/
		class MaskCollection {
		private:
			using iterator = std::list<Student>::iterator it;

			std::list<Mask> history_;
			
			iterator current_;

			Mask prediction_;
			Mask validated_;
		public:
			MaskCollection() = default;

			void push(const Mask& mask);
			void undo();
			void redo();

			Mask& getCurrent() { return *current_; }

			void setValidated(const Mask& mask);
			void setPrediction(const Mask& mask);
		};
	}
}