#include "animation_util.h"

void Rendering::push_animation() {
	EventQueue::getInstance().post(Event_ptr(new Event("push_animation")));
	glfwPostEmptyEvent();
}