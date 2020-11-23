#include "mask.h"

namespace core {
	namespace segmentation {

		MaskCollection::MaskCollection(int width, int height) : width_(width), height_(height) {
			it_ = history_.end();
		}

		void MaskCollection::push(const Mask& mask) {
			if (it_ != history_.end()) {
				history_.erase(it_, history_.end());
			}
			history_.push_back(mask);
			it_ = history_.end();
		}

		void MaskCollection::push_new() {
			Mask mask(width_, height_);
			push(mask);
		}

		MaskCollection MaskCollection::copy() {
			MaskCollection collection(width_, height_);
			collection.width_ = width_;
			collection.height_ = height_;
			for (auto& mask : history_) {
				collection.history_.push_back(mask.copy());
			}
			collection.it_ = collection.history_.end();
			collection.prediction_ = prediction_.copy();
			collection.validated_ = validated_.copy();
			return collection;
		}

		Mask& MaskCollection::undo() {
			if (it_ != ++history_.begin()) {
				it_--;
			}
			return getCurrent();
		}

		Mask& MaskCollection::redo() {
			if (it_ != history_.end()) {
				it_++;
			}
			return getCurrent();
		}

		Mask& core::segmentation::MaskCollection::getCurrent() {
			if (history_.empty()) {
				return Mask(width_, height_);
			}
			Mask& mask = *(it_--);
			it_++;
			return mask;
		}

		Mask::Mask(int width, int height) : width_(width), height_(height) {
			data_ = cv::Mat(width, height, CV_8U, char(0));
		}

		Mask Mask::copy() {
			Mask mask(width_, height_);
			data_.copyTo(mask.data_);
			mask.width_ = width_;
			mask.height_ = height_;
			mask.filename_ = filename_;
			return mask;
		}

		void Mask::setData(cv::Mat& data) {
			data_ = data;
		}

	}
}