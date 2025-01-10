#pragma once

#include "ofImage.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include "ofEvents.h"
#include "ofAppBaseWindow.h"
#include "ofRectangle.h"

#include <map>
#include <bitset>

#include "ofxImGuiConstants.h"
#include "DefaultTheme.h"
//#include "LinkedList.hpp"
#include <unordered_map>
#include "imgui.h" // for ImFont*

#if defined(OFXIMGUI_BACKEND_OPENFRAMEWORKS)
	#include "EngineOpenFrameworks.h"
#elif defined (OFXIMGUI_BACKEND_VULKAN)
	#include "EngineVk.h"
#elif defined(OFXIMGUI_BACKEND_GLES)
	#include "EngineOpenGLES.h"
#elif defined(OFXIMGUI_BACKEND_GLFW)
	#include "EngineGLFW.h"
#else
	#include "EngineOpenFrameworks.h"
	//#error "Sorry, ofxImGui is incorrectly configured, cannot proceed !"
#endif

//#include "imgui_internal.h" // for ImGuiContext*

// Notes on context handling in ImGui
// - Ideally, we should create 1 imgui context per ofApp and one host viewports per ofAppBaseWindow, which actually is not possible.
//   Related issues:
//     - Use of viewports with existing windows : https://github.com/ocornut/imgui/issues/4298
//     - Multiple host viewports (solution: 1 context per window) : https://github.com/ocornut/imgui/issues/3012
// - Currently implemented, the easiest way to achieve this is by providing one ImGui context per ofWindow, letting instances draw to it.
//   The only drawback of this is that ImGui windows (or other draggables) cannot be moved to the other gui.
//   This should not be an issue in most ofApps. Generally in GPU apps it's recommended to keep the GUI to one application window.

// Todo: We could provide a way to intercept/filter OF mouse clicks when they are used by ImGui ?

// Todo: count slave instances so only master exits when no more slaves ?

// Todo: Maybe we could move setup() to main.cpp, or bind it to an ofxImGuiWindow
// This could clarify the setup process by explicitly giving is the window handle.


namespace ofxImGui {
	class Gui;
}
// A context pointer with some more info. One ofxImGuiContext per ofAppWindow.
struct ofxImGuiContext {
		friend class ofxImGui::Gui;
		friend struct std::pair<ofAppBaseWindow*, ofxImGuiContext>;
	public:
		explicit ofxImGuiContext(std::shared_ptr<ofAppBaseWindow>& _window) :
			imguiContext(ImGui::CreateContext()),
			ofWindow(_window),
			autoDraw(false),
			slaveCount(0),
			isRenderingFrame(false){}
	public:
		// Prevent making copies
		ofxImGuiContext( const ofxImGuiContext& ) = delete;
		ofxImGuiContext& operator=( const ofxImGuiContext& ) = delete;

		// Allow move contructor & move assign
		explicit ofxImGuiContext(ofxImGuiContext&& other) noexcept : imguiContext{other.imguiContext}{
			other.imguiContext = nullptr;
			//other. = false;
		}
//		ofxImGuiContext& operator=(ofxImGuiContext&& other) noexcept {

//		}

		~ofxImGuiContext(){
			if(imguiContext) {
				ImGui::DestroyContext(imguiContext);
			}
		}

		ImGuiContext* imguiContext = nullptr;
		std::shared_ptr<ofAppBaseWindow> ofWindow;
		bool autoDraw;

		inline bool isShared() const {
			return slaveCount > 1;
		}

		// Note: Explicit, not to mess with compilers casting it to anything !
		explicit operator bool() const {
			return imguiContext != nullptr;
		}
		bool operator !() const {
			return !(bool)this;//imguiContext == nullptr;
		}
	protected:
		unsigned int slaveCount;
		bool isRenderingFrame;

#if defined (OFXIMGUI_BACKEND_OPENFRAMEWORKS)
		ofxImGui::EngineOpenFrameworks engine;
#elif defined (OFXIMGUI_BACKEND_VULKAN)
		ofxImGui::EngineVk engine;
#elif defined(OFXIMGUI_BACKEND_OPENGLES)
		ofxImGui::EngineOpenGLES engine;
#elif defined(OFXIMGUI_BACKEND_GLFW)
		ofxImGui::EngineGLFW engine;
#else
		ofxImGui::EngineOpenFrameworks engine;
#endif
};

namespace ofxImGui
{
	enum SetupState : unsigned char {
		Error = 0, // Keep to 0 so that it evaluates to false ?
		Slave = 1 << 1,
		Master = 1 << 2,
		// Success flag
		Success = Slave | Master, // Use like: if(mState & Success)
	};
	std::ostream& operator<<(std::ostream& os, const SetupState& _state);

	class Gui
	{
		// to provide access to window->context map
		friend class BaseEngine;
		friend class EngineGLFW;
		friend class EngineOpenFrameworks;
	public:
		Gui();
		~Gui();

		// Prevent making copies, prevents making mistakes messing up the gui instances and contexts
		Gui( const Gui& ) = delete;
		Gui& operator=( const Gui& ) = delete;

		SetupState setup(BaseTheme* theme = nullptr, bool autoDraw = true, ImGuiConfigFlags customFlags_=ImGuiConfigFlags_None, bool _restoreGuiState = false, bool _showImGuiMouseCursor = false );
		SetupState setup(std::shared_ptr<ofAppBaseWindow>& _ofWindow, BaseTheme* theme = nullptr, bool autoDraw = true, ImGuiConfigFlags customFlags_=ImGuiConfigFlags_None, bool _restoreGuiState = false, bool _showImGuiMouseCursor = false );
		void exit();

        // Todo: remove these ? Adapt them ?
		//void setSharedMode(bool _sharedMode=true);
        bool isInSharedMode() const;
        bool isMaster() const;

		void begin();
		void end();

		void draw();

        bool setDefaultFont(int indexAtlasFont);
        bool setDefaultFont(ImFont* _atlasFont);
        ImFont* addFont(const std::string & fontPath, float fontSize = 13.0f, const ImFontConfig* _fontConfig = nullptr, const ImWchar* _glyphRanges = nullptr, bool _setAsDefaultFont=false );
        ImFont* addFontFromMemory(void* fontData, int fontDataSize, float fontSize = 13.0f, const ImFontConfig* _fontConfig = nullptr, const ImWchar* _glyphRanges = nullptr, bool _setAsDefaultFont=false );
        bool rebuildFontsTexture();

		void setTheme(BaseTheme* theme);

		GLuint loadImage(ofImage& image);
		GLuint loadImage(const std::string& imagePath);

		GLuint loadPixels(const std::string& imagePath);
		GLuint loadPixels(ofPixels& pixels);

		GLuint loadTexture(const std::string& imagePath);
		GLuint loadTexture(ofTexture& texture, const std::string& imagePath);
		// todo: updateFontTexture ?

        void afterDraw(ofEventArgs& _args); // Listener

		// Helper window to debug ofxImGui specific stuff, and provide some hints on your setup.
		void drawOfxImGuiDebugWindow(bool* open=nullptr) const;

		// Helper to retrieve the current gui-free zone within the ofAppWindow.
		// Basically returns windowRect - MenuBarSpace - SideDocks
		ofRectangle getMainWindowViewportRect(bool returnScreenCoords=false, bool removeMenuBar=true, bool removeDockingAreas=true) const;
		int getMenuHeight() const;
		ofRectangle getDockingViewport() const;

    private:
        void render();
		static void initialiseForWindow();

//#if defined (OFXIMGUI_FORCE_OF_BACKEND)
//        EngineOpenFrameworks engine;
//#elif defined (OF_TARGET_API_VULKAN)
//        EngineVk engine;
////#elif defined(TARGET_OPENGLES) && !defined(TARGET_GLFW_WINDOW)
////        EngineOpenGLES engine;
//#else
//        EngineGLFW engine;
//#endif
		ofEventListener autoDrawListener;

		BaseTheme* theme=nullptr; // Todo: move this into ofxImguiContext ?

		std::vector<ofTexture*> loadedTextures;

		ofRectangle dockingViewport;
		int menuHeight;

		void updateDockingVp();

        // Static context instance. All Gui instances share the same context.
        // If you're dealing with dynamic libraries, you might need to pass this over to another ImGui instance.
		ofxImGuiContext* context = nullptr; // Short-hand value, same as stored in the map, faster access
		bool isContextOwned = false; // Copy of context, set when it needs destruction

		//static LinkedList<ofAppBaseWindow*, ofxImGuiContext> imguiContexts; // Window/MasterContext map
		static std::unordered_map<ofAppBaseWindow*, ofxImGuiContext> imguiContexts; // Window/MasterContext map
	};
}
