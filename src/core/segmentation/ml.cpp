#include "ml.h"

#include <iostream>

namespace core {
	namespace segmentation {
		ML_Model::ML_Model(const std::string& name, int ww, int wc, int input_size)
			: name_(name), ww_(ww), wc_(wc), input_size_(input_size)
		{
			std::cout << "Hello world" << std::endl;
		}
	}
}
