#pragma once

#include <string>

#include <torch/torch.h>

namespace core {
	namespace segmentation {
		class ML_Model {
		private:
			std::string name_;
			int ww_ = 400;
			int wc_ = 40;

			int input_size_ = 256;
		public:
			ML_Model(const std::string& name, int ww, int wc, int input_size);
		};
	}
}