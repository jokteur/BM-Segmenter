#pragma once

#include "state.h"
#include <tempo.h>

#define SCALED_PX(X) m_ui_state->scaling * X
#define ALIGN_RIGHT(X) \
float pos_x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - SCALED_PX(X); \
ImGui::SetCursorPosX(pos_x)