#pragma once

#include <thread>
#include <string>
#include <vector>

namespace core {
	class LoadQueue {
	private:
	public:
		LoadQueue() = default;

		/**
		 * @return instance of the Singleton of the EventQueue
		 */
		static LoadQueue& getInstance() {
			static LoadQueue instance;
			return instance;
		}
	};

	class DicomLoader {
	private:
	public:

	}; 

	class MaskLoader {
	private:
	public:
	};
}