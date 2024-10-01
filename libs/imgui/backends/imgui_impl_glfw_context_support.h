// Fixes for enabling multi-context support for the GLFW backend
// See https://github.com/ocornut/imgui/pull/3934#issuecomment-873213161
// See https://github.com/ocornut/imgui/issues/5439

// Multi-context is temporary in ofxImGui until ImGui eventually implements "additional host viewports"
// Creating and managing multiple contexts is temporary and not recommended by Omar.

// Glfw events are a bit messed; this ensures that the right windows events call upon the right imgui contexts. (see PR#3934 )
// This file aims to reduce changes in the GLFW backend while injecting context switching before imgui callbacks are triggered.

#pragma once

#include "ofxImGuiConstants.h" // To get the definition of OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP
#if defined(OFXIMGUI_BACKEND_GLFW) && (OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1)

#include "backends/imgui_impl_glfw.h"

#include "LinkedList.hpp"

// Fwd declarations
struct ImGui_ImplGlfw_Data;

namespace ofxImGui {
	class Gui;
}

// A scoped struct to set the correct context and restores it when destroyed
// Reduces
struct ImGui_ImplGlfw_ScopedContext {
		friend class ofxImGui::Gui;
	public:
		inline ImGui_ImplGlfw_ScopedContext(GLFWwindow* window): prevContext(ImGui::GetCurrentContext()){
			ImGuiContext* context = Contexts.findData(window);
			if(context){
				ImGui::SetCurrentContext(context);
			}
		}
		~ImGui_ImplGlfw_ScopedContext(){
			if(prevContext != nullptr){
				ImGui::SetCurrentContext(prevContext);
			}
		}
		static inline void RegisterWindowContext(GLFWwindow* window, ImGuiContext* context){
			Contexts.add( window, context );
		}
		static inline void RemoveWindowContext(GLFWwindow* window){
			Contexts.remove(window);
		}

	private:
		ImGuiContext* const prevContext;
		// Contains all standalone viewport windows.
		static LinkedList<GLFWwindow, ImGuiContext*> Contexts;
};

// Very tmp not to modify imgui_glfw again
inline void ImGui_ImplGlfw_RegisterWindowContext(GLFWwindow* window, ImGuiContext* context){
	ImGui_ImplGlfw_ScopedContext::RegisterWindowContext(window, context);
}
inline void ImGui_ImplGlfw_RemoveWindowContext(GLFWwindow* window){
	ImGui_ImplGlfw_ScopedContext::RemoveWindowContext(window);
}

// Todo: set context for :
// - ImGui_ImplGlfw_UpdateKeyModifiers ?
// - ImGui_ImplGlfw_MonitorCallback ?

#else
// Define classes, compiler should strip all in optimisation steps as it's just dummy code.
class GLFWwindow;
class ImGuiContext;
inline void ImGui_ImplGlfw_RemoveWindowContext(GLFWwindow* window){}
inline void ImGui_ImplGlfw_RegisterWindowContext(GLFWwindow* window, ImGuiContext* context){}
struct ImGui_ImplGlfw_ScopedContext {
		ImGui_ImplGlfw_ScopedContext(GLFWwindow* window){}
};
#endif
