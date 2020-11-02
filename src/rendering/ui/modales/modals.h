#pragma once

#include <string>
#include <functional>
#include <utility>
#include <vector>
#include <iostream>

#include <GLFW/glfw3.h>
#include "imgui.h"

#include "keyboard_shortcuts.h"
#include "rendering/drawables.h"

namespace Rendering {
    using modal_fct = std::function<void (bool&, bool&, bool&)>;

    /**
     * Struct to help draw a Modal
     */
    struct Modal {
        std::string title;
        modal_fct draw_fct;
        int flags = 0;
        bool show = false;
        bool enter = false;
        bool escape = false;

        // Modals can be stacked inside other modals
        Modal* modal = nullptr;

        Shortcut escape_shortcut = {
                {GLFW_KEY_ESCAPE},
                "escape",
                "",
                [this] {
                    escape = true;
                }
        };
        Shortcut enter_shortcut = {
                {GLFW_KEY_ENTER},
                "enter",
                "",
                [this] {
                    enter = true;
                }
        };
        Shortcut enter_kp_shortcut = {
                {GLFW_KEY_KP_ENTER},
                "enter",
                "",
                [this] {
                    enter = true;
                }
        };

        void ImGuiDraw(GLFWwindow *window) {
            if(show) {
                ImGui::OpenPopup(title.c_str());
            }
            if (ImGui::BeginPopupModal(title.c_str(), &show, flags)) {
                // Want that all parent shortcuts are ignored -> flush them
                KeyboardShortCut::flushTempShortcuts();
                draw_fct(show, enter, escape);
                KeyboardShortCut::addTempShortcut(escape_shortcut);
                KeyboardShortCut::addTempShortcut(enter_shortcut);
                KeyboardShortCut::addTempShortcut(enter_kp_shortcut);
                if(modal != nullptr) {
                    modal->ImGuiDraw(window);
                }
                ImGui::EndPopup();
            }
        }
    };

    /**
     * This class helps to manage modal (also modals inside modals)
     * This is a Singleton, to be called from anywhere
     * The class that draws the modal is called ModalsDrawable
     */
    class Modals {
    private:
        Modal modal_;
        std::vector<Modal*> stacked_modals_;

        void free_memory() {
            // Erase all previously stacked modals
            for(auto &it : stacked_modals_) {
                delete it;
            }
            stacked_modals_.clear();
        }

        Modals() = default;
    public:
        /**
         * Copy constructors stay empty, because of the Singleton
         */
        Modals(Modals const &) = delete;
        void operator=(Modals const &) = delete;

        /**
         * @return instance of the Singleton of the Job Scheduler
         */
        static Modals& getInstance () {
            static Modals instance;
            return instance;
        }

        /**
         * Returns true if any modal is already active
         * @return
         */
        bool isActive() const {
            return modal_.show;
        }

        /**
         * Sets the current modal to be displayed
         * @param title title of the modal
         * @param draw_fct function to be drawn in the modal
         * @param flags ImGui flags for the modal
         */
        void setModal(std::string title, modal_fct draw_fct, int flags = 0) {
            free_memory();

            modal_.title = std::move(title);
            modal_.draw_fct = std::move(draw_fct);
            modal_.show = true;
            modal_.modal = nullptr;
            modal_.enter = false;
            modal_.escape = false;
            modal_.flags = flags;
        }

        /**
         * Stacks a modal inside the currently displayed modal
         * @param title title of the modal
         * @param draw_fct function to be drawn in the modal
         * @param flags ImGui flags for the modal
         */
        void stackModal(const std::string& title, const modal_fct& draw_fct, int flags = 0) {
            // Means that no modal is currently showing
            if (!modal_.show) {
                modal_.show = true;
                modal_.flags = flags;
                modal_.title = title;
                modal_.draw_fct = draw_fct;
                modal_.modal = nullptr;
            }
            else {
                Modal *tmp_modal = modal_.modal;
                while (tmp_modal != nullptr)
                    tmp_modal = tmp_modal->modal;
                auto modal = new Modal;
                modal->title = title;
                modal->draw_fct = draw_fct;
                modal->flags = flags;
                modal->show = true;

                modal_.modal = modal;
                stacked_modals_.push_back(modal);
            }
        }

        void ImGuiDraw(GLFWwindow *window) {
            modal_.ImGuiDraw(window);
        }

        ~Modals() {
            free_memory();
        }
    };

    class ModalsDrawable : public AbstractLayout {
    public:
        ModalsDrawable() = default;

        void ImGuiDraw(GLFWwindow *window, Rect &dimensions) override {
            Modals::getInstance().ImGuiDraw(window);
        }
    };
}