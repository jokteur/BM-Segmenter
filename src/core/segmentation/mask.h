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

			void load_from_file(const std::string& filename);
		public:
			Mask(int width, int height);
			Mask(const std::string& filename);

			/**
			* Copy constructors
			*/
			//void operator=(const Mask& other);
			//Mask(const Mask& other);

			Mask copy();

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
			using iterator = std::list<Mask>::iterator;

			iterator it_;
			std::list<Mask> history_;
			
			iterator current_;

			int width_ = 0;
			int height_ = 0;

			Mask prediction_;
			Mask validated_;
		public:
			MaskCollection() = default;
			MaskCollection(int width, int height);
			MaskCollection(const std::string& filename);

			void push(const Mask& mask);
			void push_new();

			MaskCollection copy();

			/**
			 * Goes back into the history of the mask collection
			*/
			Mask& undo();

			/**
			 * Goes in the future of the history of the mask collection
			*/
			Mask& redo();

			/**
			 * Returns the current mask in the list
			 * If there is no mask in the collection, returns an empty mask
			*/
			Mask& getCurrent();

			std::string saveCollection(const std::string& basename);

			std::string loadCollection(const std::string& basename);

			void setValidated(const Mask& mask) { validated_ = validated_; }
			void setPrediction(const Mask& mask) { prediction_ = prediction_; }
		};
	}
}