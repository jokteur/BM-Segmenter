#pragma once

#include "first_include.h"
#include "events.h"

namespace Rendering {
	void push_animation() {
		EventQueue::getInstance().post(Event_ptr(new Event("push_animation")));
		glfwPostEmptyEvent();
	}
}