// This file will set all macro defines for ofxImGui.
// You can modify it to quickly suit your needs
// But it's recommended to rather overr-de them using project-scope compiler defines.

//#pragma once

// Ensures they are always set (Only set if not forced by user).

// Flags that you can set :
// #define OFXIMGUI_FORCE_OF_BACKEND --> force-use the simpler OF-based backend
// #define OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP 0|1 --> disable imgui glfw backend modification to allow multiple contexts, for using imgui with multiple ofAppBaseWindows
// #define OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP 0|1 --> Disable modifications to imgui_impl_glfw, disabling support for using ofxImGui in a multi-windowed-ofApp with viewports enabled.

// Grab ofConstants
#include "ofConstants.h"

// OFXIMGUI_DEBUG
#ifndef OFXIMGUI_DEBUG
	// Uncomment to enable debugging by default
	// Prefer defining it in your project macros.
	//#define OFXIMGUI_DEBUG
#endif

// Additional debug flag
#ifdef OFXIMGUI_DEBUG
	// Uncomment to enable by default
	//#define OFXIMGUI_DEBUG_MACROS
#else
	// Force-disable if debug not set
	#undef OFXIMGUI_DEBUG_MACROS
#endif

// Helper for printing debug messages (only when DEBUG is on)
#ifdef OFXIMGUI_DEBUG_MACROS
	#define PRAGMA_MESSAGE(x) _Pragma(#x)
	#define OFXIMGUI_COMPILER_MESSAGE(X) PRAGMA_MESSAGE(message X)
#else
	#define OFXIMGUI_COMPILER_MESSAGE(X)
#endif

// Platform backend selection
#if !defined(OFXIMGUI_FORCE_OF_BACKEND)

	// Vulkan support ?
	#if defined (OF_TARGET_API_VULKAN) && FALSE // tmp disabled, doesn't work either. Maybe rather implement OF backend with vulkan support
		#define OFXIMGUI_LOADED_BACKEND "Vulkan"
		#define OFXIMGUI_BACKEND_VULKAN
		// Warn Vulkan users
		#error "Sorry, there's no tested support for Vulkan yet while it should be very easy to implement"
		// See https://github.com/ocornut/imgui/blob/master/backends/imgui_impl_vulkan.h or ImGui_ImplGlfw_InitForVulkan

	// Global condition for GLES platforms. Todo: Check if there are no false positives ?
	#elif defined(TARGET_OPENGLES) && !defined(TARGET_GLFW_WINDOW) && FALSE  // tmp disabled, doesn't work either. Now we use the OF backend with gles support
		#define OFXIMGUI_LOADED_BACKEND "GLES"
		#define OFXIMGUI_BACKEND_GLES
		#warning "The GLES backend is an ancient relic, proceed with caution."
		//#include "EngineOpenGLES.h"

	// Platforms where GLFW is default don't define TARGET_GLFW_WINDOW, so we have to explictly add them
	#elif defined(TARGET_GLFW_WINDOW) || (!defined(TARGET_OPENGLES) && (defined(TARGET_LINUX) || defined(TARGET_OSX)  || defined(TARGET_WIN32)))
		OFXIMGUI_COMPILER_MESSAGE("ofxImGui is compiling with the GLFW platform backend.")
		//#include "EngineGLFW.h"
		#define OFXIMGUI_LOADED_BACKEND "GLFW"
		#define OFXIMGUI_BACKEND_GLFW
	#endif
#endif

// Default backend : openframeworks
#ifndef OFXIMGUI_LOADED_BACKEND
	OFXIMGUI_COMPILER_MESSAGE("ofxImGui is compiling with the default openFrameworks backend.")
	#define OFXIMGUI_LOADED_BACKEND "OpenFrameworks"
	#define OFXIMGUI_BACKEND_OPENFRAMEWORKS
#endif

// Renderer selection
// - - - - - - - - - -
// GL ES
#if defined(TARGET_OPENGLES)
	#define OFXIMGUI_RENDERER_GLES
	OFXIMGUI_COMPILER_MESSAGE("ofxImGui is compiling with GL ES renderer support.")

	// Specialisations
	// Todo: remove these ? GLES1 is available on all OF gles platforms, so no differenciation is needed
	//#ifdef TARGET_RASPBERRY_PI
	//	#define OFXIMGUI_RENDERER_GLES_1
	//#else
	//	#define OFXIMGUI_RENDERER_GLES_2
	//#endif
// Vulkan renderer ?
#elif defined (OF_TARGET_API_VULKAN)
	#error "Sorry, ofxImGui doesn't support Vulkan yet. Some works have been done in the past, and ImGui supports it, if you have OF+Vulkan, feel free to have a look or ask questions."
// GL SL (default)
#else
	#define OFXIMGUI_RENDERER_GLSL
	OFXIMGUI_COMPILER_MESSAGE("ofxImGui is compiling with GL SL renderer support.")
#endif

// Sanitize backend specific flags
// - - - - - - - - - - - - - - - -
// Sanitize GLFW callback flags which allow interfacing GLFW in different ways.
#ifdef OFXIMGUI_BACKEND_GLFW
	// Wondering what all these settings are ? The easiest way is to add `gui.drawOfxImGuiDebugWindow()` to your ofApp and follow instructions !

	// Retro-compatibility with ±2023 develop versions until commit #9bfb15e
	#if INTERCEPT_GLFW_CALLBACKS == 1
		// Both 3 used to be enabled with this flag
		#ifndef OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS
			#define OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS 1
		#endif
		#ifndef OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP
			#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP 1
		#endif
		#ifndef OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP
			#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP 1
		#endif
	#endif

	// Enabled by default (glfw event binding is more advanced then the ofevents, imgui works beter like this)
	#if !defined(OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS) || OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS != 0
		#define OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS 1
	#endif

	// Bind to GLFW ?
	#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1
		OFXIMGUI_COMPILER_MESSAGE("ofxImGui is binding window events by replacing some OF's GLFW listeners.")
		// Shorthand compiler flag
		#ifdef OFXIMGUI_GLFW_FIX_MULTICONTEXT
			#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP 0
			#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP 1
		#else
			#if !defined(OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP) || OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP != 0
				// You can toggle these lines to change the default value (disabled by default)
				//#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP 1
				#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP 0
			#endif

			// Enable Multi-context support by default
			#if !defined(OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP) || OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP != 0
				// You can toggle these lines to change the default value (enabled by default)
				#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP 1
				//#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP 0
			#endif
		#endif
	// Bind to openFrameworks ?
	#else
		OFXIMGUI_COMPILER_MESSAGE("ofxImGui is binding window events by listening to the OpenFrameworks events.")
		// Sanitize / disable some flags
		#ifdef OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS
			#undef OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS
		#endif
		#define OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS 0
		#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP 0
		#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP 0
	#endif

// OF Backend
#else
	// Disable some GLFW backend related flags
	#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP 0
	#define OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP 0
	#undef OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS
#endif

// Prevent using this flag in code
#undef OFXIMGUI_GLFW_FIX_MULTICONTEXT

// Touch events support ?
// - - - - - - - - - - - - - - - -
// Todo: add explicit compiler flag so devices such as Windows/Linux with TouchScreens can use it too ?
#if defined(TARGET_OF_IOS) || defined(TARGET_ANDROID)
	#define OFXIMGUI_TOUCH_EVENTS
#endif
