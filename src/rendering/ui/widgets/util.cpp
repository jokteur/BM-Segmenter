#include "util.h"

void Rendering::Widgets::Selectable::build(std::vector<std::string> options) {
	options_ = options;
}

Rendering::Widgets::Selectable::Selectable(const std::vector<std::string>& options, std::function<void(std::string)> on_select) {
	setOptions(options, on_select);
}

Rendering::Widgets::Selectable::Selectable(const std::vector<std::string>& options, std::function<void(int)> on_select) {
	setOptions(options, on_select);
}

void Rendering::Widgets::Selectable::setOptions(const std::vector<std::string>& options, std::function<void(std::string)> on_select) {
	str_fct_ = on_select;
	idx_ = 0;
	which_ = 0;
	build(options);
}

void Rendering::Widgets::Selectable::setOptions(const std::set<std::string>& options, std::function<void(std::string)> on_select) {
	std::vector<std::string> opts;
	for (auto& option : options) {
		opts.push_back(option);
	}
	setOptions(opts, on_select);
}

void Rendering::Widgets::Selectable::setOptions(std::vector<std::string> options, std::function<void(int)> on_select) {
	idx_fct_ = on_select;
	idx_ = 0;
	which_ = 1;
	build(options);
}

void Rendering::Widgets::Selectable::setOptions(std::set<std::string> options, std::function<void(int)> on_select) {
	std::vector<std::string> opts;
	for (auto& option : options) {
		opts.push_back(option);
	}
	setOptions(opts, on_select);
}

void Rendering::Widgets::Selectable::setIdx(int idx) {
	if (idx > 0 && idx < options_.size()) {
		idx_ = idx;
	}
}

void Rendering::Widgets::Selectable::ImGuiDraw(const std::string& label, float size) {
	if (!options_.empty()) {
		const char* combo_label = options_[idx_].c_str();

		if (size != 0.f) {
			ImGui::PushItemWidth(size);
		}
		if (ImGui::BeginCombo(label.c_str(), combo_label)) {
			int n = 0;
			for (int n = 0; n < options_.size(); n++) {
				const bool is_selected = (idx_ == n);
				if (ImGui::Selectable(options_[n].c_str(), is_selected))
					idx_ = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		if (idx_ != prev_idx_) {
			prev_idx_ = idx_;
			if (which_ == 0) {
				str_fct_(options_[idx_]);
			}
			else {
				idx_fct_(idx_);
			}
		}
	}
}
