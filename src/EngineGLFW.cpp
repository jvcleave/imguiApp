#include "ofxImGuiConstants.h"
#include "EngineGLFW.h"

#ifdef OFXIMGUI_BACKEND_GLFW

#include "ofAppGLFWWindow.h"
#include "ofGLProgrammableRenderer.h"
#include "GLFW/glfw3.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_opengl2.h"

#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1 && OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 0 && OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
	#include "backends/imgui_impl_glfw_context_support.h"
#endif


namespace ofxImGui
{
	GLuint EngineGLFW::g_FontTexture = 0;

	//--------------------------------------------------------------
	void EngineGLFW::setup(ofAppBaseWindow* _window, ImGuiContext* _imguiContext, bool autoDraw)
	{
        if (isSetup){
#ifdef OFXIMGUI_DEBUG
            ofLogNotice("EngineGLFW::setup()") << "Ignoring glfw setup, already bound." << std::endl;
#endif
            return;
        }
#ifdef OFXIMGUI_DEBUG
		else ofLogNotice("EngineGLFW::setup()") << ((void*)this) << " Setting up GLFW in oF window " << ofGetWindowPtr() << " // GlfwWindow=" << ofGetWindowPtr()->getWindowContext() << " ["<< ofGetWindowPtr()->getWindowSize() <<"]";

        #if defined(OFXIMGUI_DEBUG) && defined(TARGET_OPENGLES) && defined(TARGET_RASPBERRY_PI)
            // oF 0.11 related warnings
            #if defined(TARGET_RASPBERRY_PI_LEGACY)
                #pragma message "WARNING ! ofxImGui detected that you use the RPI in LEGACY mode, which use an EGL window, which ain't supported by any native imgui backend. Please consider using an ofxImGui version before 1.80."
            #else
                #pragma message "You are using the new Rpi GLFW binding. Please ensure that your raspi-config doesn't use the Legacy video drivers for better performance."
            #endif

            // Warn for GL ES 1 usage (unsupported by imgui)
            if( !ofIsGLProgrammableRenderer() ){
                ofLogWarning(__FUNCTION__) << "ofxImGui has not been tested with GL ES 1.0 and there's no official imgui backend for it.";
            }
        #endif // rpi gles
#endif // debug

		// Store a reference to the current imgui context for event handling
		imguiContext = _imguiContext;

        // Init window
		ofAppBaseWindow* curOfWin = _window;
        GLFWwindow* curWin = (GLFWwindow*)curOfWin->getWindowContext();

		// Check if the undelying window is valid too
		if(curWin == nullptr){
			ofLogError("EngineGLFW::setup()") << "Sorry, for now ofxImGui needs to be setup in a valid window object.";
			return;
		}

		// Maybe we need this in the future ?
		// For now OF ensures it's already the active window so it's useless.
        //glfwMakeContextCurrent(curWin);

        // Todo: check return value from glfw ?
		// no more auto binding as it's required to set the right imgui context before calling the backend ones. See https://github.com/ocornut/imgui/blob/9764adc7bb7a582601dd4dd1c34d4fa17ab5c8e8/backends/imgui_impl_glfw.cpp#L142-L145
#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1 && OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 0
		ImGui_ImplGlfw_InitForOpenGL( curWin, true );
#else
		ImGui_ImplGlfw_InitForOpenGL( curWin, false );
#endif
		//ImGui_ImplGlfw_InitForOpenGL( curWin, !(OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1 && OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1) );

		// Register events manually if needed
		registerCallbacks(curOfWin, curWin);

        // Init renderer
        if( ofIsGLProgrammableRenderer() ){
			const char* glsl_version = getGLVersionFromOF();

#ifdef OFXIMGUI_DEBUG
            int major = ofGetGLRenderer()->getGLVersionMajor();
            int minor = ofGetGLRenderer()->getGLVersionMinor();
    #ifdef TARGET_OPENGLES
			ofLogVerbose("EngineGLFW::setup()") << "ofxImGui loading GLFW with programmable OpenGL ES " << major << "." << minor << " and version string «" << glsl_version << "»";
    #else
			ofLogVerbose("EngineGLFW::setup()") << "ofxImGui loading GLFW with programmable OpenGL " << major << "." << minor << " and version string «" << glsl_version << "»";
    #endif
#endif

            ImGui_ImplOpenGL3_Init(glsl_version); // Called by the function below, but needed with these arguments
			ImGui_ImplOpenGL3_CreateDeviceObjects();
        }
        else
        {
#ifdef OFXIMGUI_DEBUG
    #ifdef TARGET_OPENGLES
			ofLogVerbose("EngineGLFW::setup()") << "ofxImGui loading GLFW with OpenGL ES 1, which is quite experimental. [ofIsGLProgrammableRenderer()=" << (ofIsGLProgrammableRenderer()?"1":"0") << "]";
    #else
			ofLogVerbose("EngineGLFW::setup()") << "ofxImGui loading GLFW with OpenGL2 and ofIsGLProgrammableRenderer()=" << (ofIsGLProgrammableRenderer()?"1":"0");
    #endif
#endif
            ImGui_ImplOpenGL2_Init();
            ImGui_ImplOpenGL2_CreateDeviceObjects();
        }

		isSetup = true;

	} 
    
	//--------------------------------------------------------------
	void EngineGLFW::exit()
	{
		if (!isSetup) return;

		// Ensure context is set, when shutting down from a callback for example.
		if(!setImGuiContext()){
#ifdef OFXIMGUI_DEBUG
			ofLogWarning("EngineGLFW::exit()") << "Couldn't set context on exit... there's probably something wrong. There's unexpected behaviour ahead, good luck !";
#endif
			return;
		}

//#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
//		ofRemoveListener(ofGetCurrentWindow()->events().exit, this, &EngineGLFW::onWindowExit);
//#endif
		unregisterCallbacks();

        if (ofIsGLProgrammableRenderer()){
            //ImGui_ImplOpenGL3_DestroyFontsTexture(); // called by function below
            //ImGui_ImplOpenGL3_DestroyDeviceObjects(); // Called below
            ImGui_ImplOpenGL3_Shutdown(); // called by ImGuiDestroyContext() later ?
        }
        else {
            //ImGui_ImplOpenGL2_DestroyFontsTexture(); // called by function below
            //ImGui_ImplOpenGL2_DestroyDeviceObjects(); // Called below
            ImGui_ImplOpenGL2_Shutdown();
        }
		// Note: Restores OF callbacks; can it happen that of unregistered them already ?
		ImGui_ImplGlfw_Shutdown();

		isSetup = false;
	}

    //--------------------------------------------------------------
    void EngineGLFW::newFrame()
    {
        // Set window ref for later usage
        //ImGuiIO& io = ImGui::GetIO();
        //io.BackendPlatformUserData = (void*) (GLFWwindow*) ofGetWindowPtr()->getWindowContext();
        if (ofIsGLProgrammableRenderer()){
            ImGui_ImplOpenGL3_NewFrame();
        }
        else{
            ImGui_ImplOpenGL2_NewFrame();
        }
        ImGui_ImplGlfw_NewFrame();
    }


	//--------------------------------------------------------------
    void EngineGLFW::render()
	{

        if (ofIsGLProgrammableRenderer()) {
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        else {
            // GLint last_program;
            // glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
            // glUseProgram(0);
            //There are potentially many more states you could need to clear/setup that we can't access from default headers.
            //e.g. glBindBuffer(GL_ARRAY_BUFFER, 0), glDisable(GL_TEXTURE_CUBE_MAP).
            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            //glUseProgram(last_program);
        }

        // Handle multi-viewports
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
            GLFWwindow* backup_current_context = glfwGetCurrentContext(); // Tocheck, maybe not possible on rpi ?

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();

            // Restore context so we can continue to render with oF
            glfwMakeContextCurrent(backup_current_context);
        }
	}

	//--------------------------------------------------------------
    bool EngineGLFW::updateFontsTexture(){
        if (ofIsGLProgrammableRenderer()) {
            return ImGui_ImplOpenGL3_CreateFontsTexture();
        }
        else {
            return ImGui_ImplOpenGL2_CreateFontsTexture();
        }
    }

	//--------------------------------------------------------------
	void EngineGLFW::registerCallbacks(ofAppBaseWindow* ofWindowPtr, GLFWwindow* glfwWindowPtr){
#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1
	#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
		// Use GLFW window user pointer API to retrieve an instance, events are static/non-member
		// Error: OF needs the userpointer, or we'd have to set it back, too messy.
		//glfwSetWindowUserPointer(curWin, (void*)this);

		// Replace callbacks and store original callbacks
		// Note: the set functions return the previously installe callbacks or nullptr if none.
		originalOFCallbacks = {
			glfwSetWindowFocusCallback( glfwWindowPtr, GlfwWindowFocusCallbackGlobal),
			glfwSetCursorEnterCallback( glfwWindowPtr, GlfwCursorEnterCallbackGlobal),
			glfwSetCursorPosCallback(   glfwWindowPtr, GlfwCursorPosCallbackGlobal),
			glfwSetMouseButtonCallback( glfwWindowPtr, GlfwMouseButtonCallbackGlobal),
			glfwSetScrollCallback(      glfwWindowPtr, GlfwScrollCallbackGlobal),
			glfwSetKeyCallback(         glfwWindowPtr, GlfwKeyCallbackGlobal),
			glfwSetCharCallback(        glfwWindowPtr, GlfwCharCallbackGlobal),
			glfwSetMonitorCallback(                    GlfwMonitorCallbackGlobal),
			true
		};

		// Unnecessary (only needed for popped-out windows, when viewports are enabled ?)
		//glfwSetWindowPosCallback(   curWin, ImGui_ImplGlfw_WindowPosCallback);
		//glfwSetWindowSizeCallback(  curWin, ImGui_ImplGlfw_WindowSizeCallback);

		// Register context
		enginesMap.add(glfwWindowPtr, this);

		// Listen to window destruction.
		// fixme: should always register (any backend) ?
		ofAddListener(ofWindowPtr->events().exit, this, &EngineGLFW::onWindowExit);

		// This might give better performance if enabled. To be fully tested.
		//ImGui_ImplGlfw_SetCallbacksChainForAllWindows(true);

		ofLogNotice("EngineGLFW::setup()") << "Replaced the openFrameworks' GLFW event listeners by the ofxImGui ones which will then call the OF and ImGui ones while adding multi-context support.";
	#else
		ofLogNotice("EngineGLFW::setup()") << "Replaced the openFrameworks' GLFW event listeners by the imgui_impl_glfw ones. You will not have multi-window nor multi-context support. This can be enabled by defining OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP=1.";
		#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
		// Register window at 2ndary
		mainGLFWWindow = glfwWindowPtr;
		ImGui_ImplGlfw_ScopedContext::RegisterWindowContext(glfwWindowPtr, this->imguiContext);
		#endif
	#endif
		// - - - - - - - - - - -
		// Bind to openframeworks
		// - - - - - - - - - - -
#else
		// Mouse events
		ofAddListener(ofEvents().mouseMoved,    this, &EngineGLFW::onMouseMoved   );
		ofAddListener(ofEvents().mouseDragged,  this, &EngineGLFW::onMouseDragged );
		ofAddListener(ofEvents().mousePressed,  this, &EngineGLFW::onMouseButton  );
		ofAddListener(ofEvents().mouseReleased, this, &EngineGLFW::onMouseButton  );
		ofAddListener(ofEvents().mouseScrolled, this, &EngineGLFW::onMouseScrolled);

	#ifdef OFXIMGUI_TOUCH_EVENTS
		// TouchEvents
		ofAddListener(ofEvents().touchDoubleTap,this, &EngineGLFW::onTouchInput );
		ofAddListener(ofEvents().touchMoved,    this, &EngineGLFW::onTouchInput );
		ofAddListener(ofEvents().touchDown,     this, &EngineGLFW::onTouchInput );
		ofAddListener(ofEvents().touchUp,       this, &EngineGLFW::onTouchInput );
		ofAddListener(ofEvents().touchCancelled, this, &EngineGLFW::onTouchInput);
	#endif
		// Keyboard avents
		ofAddListener(ofEvents().keyReleased,   this, &EngineGLFW::onKeyEvent );
		ofAddListener(ofEvents().keyPressed,    this, &EngineGLFW::onKeyEvent );
		ofAddListener(ofEvents().charEvent,     this, &EngineGLFW::onCharInput);

		// Window Listeners
		//ofAddListener(ofEvents().windowResized, this, &EngineGLFW::onWindowResized);

	#ifdef OFXIMGUI_TOUCH_EVENTS
		// TouchEvents
		//ofAddListener(ofEvents().touchDoubleTap,this, &EngineGLFW::onDeviceOrientationChanged);
	#endif
		// Additional mouse data
		ofAddListener(ofEvents().mouseEntered, this, &EngineGLFW::onMouseMoved);
		ofAddListener(ofEvents().mouseExited , this, &EngineGLFW::onMouseMoved);

		// ImGui also has io.AddFocusEvent but OF hasn't got them.
#endif
	}

	//--------------------------------------------------------------
	void EngineGLFW::unregisterCallbacks(){
#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1
		// - - - - - - - - - - -
		// Bind to GLFW
		// - - - - - - - - - - -
	#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
		GLFWwindow* glfwWindowPtr = (GLFWwindow*)ofGetWindowPtr()->getWindowContext();
		// Restore OF callbacks
		// Fixme: when OF is exiting, Not sure this is a good idea.
		if(originalOFCallbacks.isValid){
			glfwSetWindowFocusCallback( glfwWindowPtr, originalOFCallbacks.originalCallbackWindowFocus);
			glfwSetCursorEnterCallback( glfwWindowPtr, originalOFCallbacks.originalCallbackCursorEnter);
			glfwSetCursorPosCallback(   glfwWindowPtr, originalOFCallbacks.originalCallbackCursorPos);
			glfwSetMouseButtonCallback( glfwWindowPtr, originalOFCallbacks.originalCallbackMousebutton);
			glfwSetScrollCallback(      glfwWindowPtr, originalOFCallbacks.originalCallbackScroll);
			glfwSetKeyCallback(         glfwWindowPtr, originalOFCallbacks.originalCallbackKey);
			glfwSetCharCallback(        glfwWindowPtr, originalOFCallbacks.originalCallbackChar);
			glfwSetMonitorCallback(                    originalOFCallbacks.originalCallbackMonitor);
			originalOFCallbacks.isValid = false;
		}
		ofRemoveListener(ofGetWindowPtr()->events().exit, this, &EngineGLFW::onWindowExit);
		enginesMap.remove(glfwWindowPtr);
	#else
		#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
		// Unregister window at 2ndary
		if(mainGLFWWindow != nullptr){
			ImGui_ImplGlfw_ScopedContext::RemoveWindowContext(mainGLFWWindow);
			mainGLFWWindow = nullptr;
		}
		#endif
	#endif
		// - - - - - - - - - - -
		// Bind to openframeworks
		// - - - - - - - - - - -
#else
		// Mouse events
		ofRemoveListener(ofEvents().mouseMoved,     this, &EngineGLFW::onMouseMoved   );
		ofRemoveListener(ofEvents().mouseDragged,   this, &EngineGLFW::onMouseDragged );
		ofRemoveListener(ofEvents().mousePressed,   this, &EngineGLFW::onMouseButton  );
		ofRemoveListener(ofEvents().mouseReleased,  this, &EngineGLFW::onMouseButton  );
		ofRemoveListener(ofEvents().mouseScrolled,  this, &EngineGLFW::onMouseScrolled);

	#ifdef OFXIMGUI_TOUCH_EVENTS
		// TouchEvents
		ofRemoveListener(ofEvents().touchDoubleTap, this, &EngineGLFW::onTouchInput );
		ofRemoveListener(ofEvents().touchMoved,     this, &EngineGLFW::onTouchInput );
		ofRemoveListener(ofEvents().touchDown,      this, &EngineGLFW::onTouchInput );
		ofRemoveListener(ofEvents().touchUp,        this, &EngineGLFW::onTouchInput );
		ofRemoveListener(ofEvents().touchCancelled, this, &EngineGLFW::onTouchInput );
	#endif

		// Keyboard avents
		ofRemoveListener(ofEvents().keyPressed,     this, &EngineGLFW::onKeyEvent );
		ofRemoveListener(ofEvents().keyPressed,     this, &EngineGLFW::onKeyEvent );
		ofRemoveListener(ofEvents().charEvent,      this, &EngineGLFW::onCharInput);

		// Window Listeners
		//ofRemoveListener(ofEvents().windowResized,  this, &EngineGLFW::onWindowResized);

		// Additional mouse data
		ofRemoveListener(ofEvents().mouseEntered, this, &EngineGLFW::onMouseMoved);
		ofRemoveListener(ofEvents().mouseExited , this, &EngineGLFW::onMouseMoved);
#endif
	}

	//--------------------------------------------------------------
	void EngineGLFW::onWindowExit(ofEventArgs& event){
		// Todo: Clean this up and check correct behaviour
		std::cout << "Window Exit! [engine]" << (void*)this << " // [window]" << ofGetWindowPtr() << std::endl;
		std::cout << "ofApp = " << (void*)ofGetAppPtr() << " // [window]" << ofGetAppPtr() << std::endl;

		// Set imgui context of this window
		if(!setImGuiContext()) return;

		// Call Destroy platform windows(otherwise pop-out windows remain visible but unresponsive)
		//ImGui_ImplGlfw_Shutdown();
		//ImGui_ImplGlfw_ShutdownPlatformInterface();

		// Todo: Notify global GLFW event listeners to not forward calls anymore ? Windows platforms seem to receuve a focus callback afterwards

		// When using quit, exit() makes pollEvents crash later. Try to call it before.
		//ofGetMainLoop()->pollEvents();

		// Dummy exit ? What if context is shared ?
		//exit();
#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
		enginesMap.remove((GLFWwindow*)ofGetWindowPtr()->getWindowContext());
#endif


		// Set unloaded state
		imguiContext = nullptr;
		isSetup = false;

		// Todo: notify child/slaves about gui destruction ? gui.begin() should return false if the gui has gone ?
		std::cout << "Exit done" << std::endl;
	}

//	//--------------------------------------------------------------
//	// Clipboard functions
//	static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
//	{
//		// Set context
//		//if(!setImGuiContext()) return;

//		return glfwGetClipboardString((GLFWwindow*)user_data);
//	}

//	//--------------------------------------------------------------
//	static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
//	{
//		// Set context
//		//if(!setImGuiContext()) return;
//		glfwSetClipboardString((GLFWwindow*)user_data, text);
//	}

// Bind to GLFW ?
#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1
#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
	//--------------------------------------------------------------
    // The code below is a template from the GLFW backend, almost unmodified, but commented.
    void EngineGLFW::GlfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
		// Call the original callback
		if(originalOFCallbacks.originalCallbackMousebutton) originalOFCallbacks.originalCallbackMousebutton(window, button, action, mods);

		// Set context
		if(!setImGuiContext()) return;

        // Call backend function
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

		// Warning: gotta call newFrame() to compute the active state (1 frame delay?)
		if(!ImGui::GetIO().WantCaptureMouse){
			// todo: call openframeworks callback
        }

        // Restore
		restoreImGuiContext();
    }

	//--------------------------------------------------------------
	void EngineGLFW::GlfwMouseButtonCallbackGlobal(GLFWwindow* window, int button, int action, int mods){
		//EngineGLFW* self = static_cast<EngineGLFW*>(glfwGetWindowUserPointer(window));
		EngineGLFW* self = enginesMap.findData(window);
		if(self) self->GlfwMouseButtonCallback(window, button, action, mods);
#ifdef OFXIMGUI_DEBUG
		else ofLogNotice("EngineGLFW::GlfwMouseButtonCallbackGlobal") << "Missed a callback due to unknown window = " << window;
#endif
	}

	//--------------------------------------------------------------
    void EngineGLFW::GlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
		// Call the original callback
		if(originalOFCallbacks.originalCallbackScroll) originalOFCallbacks.originalCallbackScroll(window, xoffset, yoffset);

		// Set context
		if(!setImGuiContext()) return;

		// Call backend function
		ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

		// Restore
		restoreImGuiContext();
    }

	//--------------------------------------------------------------
	void EngineGLFW::GlfwScrollCallbackGlobal(GLFWwindow* window, double xoffset, double yoffset) {
		EngineGLFW* self = enginesMap.findData(window);
		if(self) self->GlfwScrollCallback(window, xoffset, yoffset);
#ifdef OFXIMGUI_DEBUG
		else ofLogNotice("EngineGLFW::GlfwScrollCallbackGlobal") << "Missed a callback due to unknown window = " << window;
#endif
	}

	//--------------------------------------------------------------
    void EngineGLFW::GlfwKeyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods){
		// Call the original callback
		if(originalOFCallbacks.originalCallbackKey) originalOFCallbacks.originalCallbackKey(window, keycode, scancode, action, mods);

		// Set context
		if(!setImGuiContext()) return;

		// Call backend function
		ImGui_ImplGlfw_KeyCallback(window, keycode, scancode, action, mods);

		// Restore
		restoreImGuiContext();
    }

	//--------------------------------------------------------------
	void EngineGLFW::GlfwKeyCallbackGlobal(GLFWwindow* window, int keycode, int scancode, int action, int mods){
		EngineGLFW* self = enginesMap.findData(window);
		if(self) self->GlfwKeyCallback(window, keycode, scancode, action, mods);
#ifdef OFXIMGUI_DEBUG
		else ofLogNotice("EngineGLFW::GlfwKeyCallbackGlobal") << "Missed a callback due to unknown window = " << window;
#endif
	}

	//--------------------------------------------------------------
    void EngineGLFW::GlfwWindowFocusCallback(GLFWwindow* window, int focused) {
//		if(ofGetMainLoop()->exitEvent)
//			if(ofGetMainLoop()->getCurrentWindow()->events().notifyExit() != nullptr)

		// Call the original callback
		if(originalOFCallbacks.originalCallbackWindowFocus) originalOFCallbacks.originalCallbackWindowFocus(window, focused);

		// Set context
		if(!setImGuiContext()) return;

		// Call backend function
		ImGui_ImplGlfw_WindowFocusCallback(window, focused);

		// Restore
		restoreImGuiContext();
    }

	//--------------------------------------------------------------
	void EngineGLFW::GlfwWindowFocusCallbackGlobal(GLFWwindow* window, int focused) {
		EngineGLFW* self = enginesMap.findData(window);
		if(self) self->GlfwWindowFocusCallback(window, focused);
#ifdef OFXIMGUI_DEBUG
		else ofLogWarning("EngineGLFW::GlfwWindowFocusCallbackGlobal") << "Missed a callback due to unknown window = " << window;
#endif
	}

	//--------------------------------------------------------------
	void EngineGLFW::GlfwCursorPosCallback(GLFWwindow* window, double x, double y) {
		// Call the original callback
		if(originalOFCallbacks.originalCallbackCursorPos) originalOFCallbacks.originalCallbackCursorPos(window, x, y);

		// Set context
		if(!setImGuiContext()) return;

		// Call backend function
		ImGui_ImplGlfw_CursorPosCallback(window, x, y);

		// Restore
		restoreImGuiContext();
    }

	//--------------------------------------------------------------
	void EngineGLFW::GlfwCursorPosCallbackGlobal(GLFWwindow* window, double x, double y) {
		EngineGLFW* self = enginesMap.findData(window);
		if(self) self->GlfwCursorPosCallback(window, x, y);
#ifdef OFXIMGUI_DEBUG
		else ofLogWarning("EngineGLFW::GlfwCursorPosCallbackGlobal") << "Missed a callback due to unknown window = " << window;
#endif
	}

	//--------------------------------------------------------------
    // Workaround: X11 seems to send spurious Leave/Enter events which would make us lose our position,
    // so we back it up and restore on Leave/Enter (see https://github.com/ocornut/imgui/issues/4984)
    void EngineGLFW::GlfwCursorEnterCallback(GLFWwindow* window, int entered) {
		// Call the original callback
		if(originalOFCallbacks.originalCallbackCursorEnter) originalOFCallbacks.originalCallbackCursorEnter(window, entered);

		// Set context
		if(!setImGuiContext()) return;

		// Call backend function
		ImGui_ImplGlfw_CursorEnterCallback(window, entered);

		// Restore
		restoreImGuiContext();
    }

	//--------------------------------------------------------------
	void EngineGLFW::GlfwCursorEnterCallbackGlobal(GLFWwindow* window, int entered) {
		EngineGLFW* self = enginesMap.findData(window);
		if(self) self->GlfwCursorEnterCallback(window, entered);
#ifdef OFXIMGUI_DEBUG
		else ofLogWarning("EngineGLFW::GlfwCursorEnterCallbackGlobal") << "Missed a callback due to unknown window = " << window;
#endif
	}

	//--------------------------------------------------------------
	void EngineGLFW::GlfwCharCallback(GLFWwindow* window, unsigned int c) {

		// Call the original callback
		if(originalOFCallbacks.originalCallbackChar) originalOFCallbacks.originalCallbackChar(window, c);

		// Set context
		if(!setImGuiContext()) return;

		// Call backend function
		ImGui_ImplGlfw_CharCallback(window, c);

		// Restore
		restoreImGuiContext();
    }

	//--------------------------------------------------------------
	void EngineGLFW::GlfwCharCallbackGlobal(GLFWwindow* window, unsigned int c) {
		EngineGLFW* self = enginesMap.findData(window);
		if(self) self->GlfwCharCallback(window, c);
#ifdef OFXIMGUI_DEBUG
		else ofLogWarning("EngineGLFW::GlfwCharCallbackGlobal") << "Missed a callback due to unknown window = " << window;
#endif
	}

	//--------------------------------------------------------------
	void EngineGLFW::GlfwMonitorCallback(GLFWmonitor* mon, int id) {
		// Call the original callback
		if(originalOFCallbacks.originalCallbackMonitor) originalOFCallbacks.originalCallbackMonitor(mon, id);

		// Set context
		if(!setImGuiContext()) return;

		// Call backend function
		ImGui_ImplGlfw_MonitorCallback(mon, id);

		// Restore
		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineGLFW::GlfwMonitorCallbackGlobal(GLFWmonitor* mon, int id) {
		// To prevent storing an extra monitor map, and because windows can change their monitor, simply trigger all engines to resize.
		auto item = enginesMap.getFirst();
#ifdef OFXIMGUI_DEBUG
		if(!item) ofLogWarning("EngineGLFW::GlfwMonitorCallbackGlobal") << "Missed a callback due to no engines  = " << mon;
#endif
		while(item){
			item->data->GlfwMonitorCallbackGlobal(mon, id);
			item = item->getNext();
		}
	}

	LinkedList<GLFWwindow, EngineGLFW*> EngineGLFW::enginesMap = {};

#endif

 // End bind events to OF
#else // if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 0

	//--------------------------------------------------------------
	// Reverse implementation of ofAppGLFWWindow::motion_cb
	void EngineGLFW::onMouseMoved(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		ImGui_ImplGlfw_CursorPosCallback( (GLFWwindow*)ofGetWindowPtr()->getWindowContext(), event.x, event.y );

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineGLFW::onMouseDragged(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		ImGui_ImplGlfw_CursorPosCallback((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), event.x, event.y );

		// Update imgui mouse pos
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddMousePosEvent(static_cast<float>(event.x), static_cast<float>(event.y));

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineGLFW::onMouseScrolled(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		ImGui_ImplGlfw_ScrollCallback((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), event.scrollX, event.scrollY );
		// Update imgui mouse pos
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddMouseWheelEvent( static_cast<float>(event.scrollX), static_cast<float>(event.scrollY) );

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	// Reverse implementation of ofAppGLFWWindow::mouse_cb
	void EngineGLFW::onMouseButton(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		if(event.type == ofMouseEventArgs::Entered || event.type == ofMouseEventArgs::Exited){
			ImGui_ImplGlfw_CursorEnterCallback((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), event.type == ofMouseEventArgs::Entered );
		}
		else if(event.type == ofMouseEventArgs::Moved || event.type == ofMouseEventArgs::Dragged){
			ImGui_ImplGlfw_CursorPosCallback((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), event.x, event.y);
		}
		else {
			int button = GLFW_MOUSE_BUTTON_LEFT;
			switch(event.button){
			case OF_MOUSE_BUTTON_LEFT:
				button = GLFW_MOUSE_BUTTON_LEFT;
				break;
			case OF_MOUSE_BUTTON_RIGHT:
				button = GLFW_MOUSE_BUTTON_RIGHT;
				break;
			case OF_MOUSE_BUTTON_MIDDLE:
				button = GLFW_MOUSE_BUTTON_MIDDLE;
				break;
			}
			int modifiers = 0;
			if(event.modifiers & OF_KEY_SHIFT){
				modifiers |= GLFW_MOD_SHIFT;
			}
			if(event.modifiers & OF_KEY_ALT){
				modifiers |= GLFW_MOD_ALT;
			}
			if(event.modifiers & OF_KEY_CONTROL){
				modifiers |= GLFW_MOD_CONTROL;
			}
			if(event.modifiers & OF_KEY_SUPER){
				modifiers |= GLFW_MOD_SUPER;
			}

			ImGui_ImplGlfw_MouseButtonCallback((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), button, event.type == ofMouseEventArgs::Pressed, modifiers );
		}
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddMouseButtonEvent(event.button, event.type == ofMouseEventArgs::Pressed);

		restoreImGuiContext();
	}
	//--------------------------------------------------------------
	void EngineGLFW::onKeyEvent(ofKeyEventArgs& event)
	{
		// Ignore repeats
		//if(event.isRepeat) return;

		// Set context
		if(!setImGuiContext()) return;

		// This one is a little too ahrd to port, let's behave like EngineOpenFrameworks
		// ImGui_ImplGlfw_KeyCallback((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), ...);

		int key = event.keycode; // Todo: this seems to be window specific ?
		ImGuiIO& io = ImGui::GetIO();

		// Set key modifiers (ensures they are set on time, fixes kb shortcuts)
		io.AddKeyEvent(ImGuiMod_Ctrl,  event.hasModifier(OF_KEY_CONTROL));
		io.AddKeyEvent(ImGuiMod_Shift, event.hasModifier(OF_KEY_SHIFT));
		io.AddKeyEvent(ImGuiMod_Alt,   event.hasModifier(OF_KEY_ALT));
		io.AddKeyEvent(ImGuiMod_Super, event.hasModifier(OF_KEY_SUPER));

		// Since 1.87 : Key events
		ImGuiKey imKey = oFKeyToImGuiKey(event.key);
		//ImGuiKey imgui_key = ImGui_ImplGlfw_KeyToImGuiKey(keycode); // Previous code

		// Fallback by guessing the imguikey from the typed character
		// Note: could create weird behaviour on some special keyboards ?
		// If so: It could be disabled, it doesn't prevent from using ImGui
		// This helps registering key up/down state, which is rarely used in imgui widgets.
//		if(imKey == ImGuiKey_None){
//			// Note: codepoint corresponds to the typed character
//			imKey = keyCodeToImGuiKey( event.codepoint );
//		}

		io.AddKeyEvent(imKey, event.type == ofKeyEventArgs::Pressed );

		// Note: this brings support for pre-1.87 user code using very specific API code.
		// It causes an assert/crash in imgui v1.89.3 when releasing ALT+CMD simultanously (osx+backend_of_native), when IMGUI_DISABLE_OBSOLETE_KEYIO is not defined.
		// As we disabled it, we can support old user code again.
		io.SetKeyEventNativeData(imKey, key, event.scancode); // To support legacy indexing (<1.87 user code)

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineGLFW::onCharInput(uint32_t& _char)
	{
		// Set context
		if(!setImGuiContext()) return;

		ImGui_ImplGlfw_CharCallback((GLFWwindow*)ofGetWindowPtr()->getWindowContext(), _char);
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddInputCharacter((unsigned short)_char);

		restoreImGuiContext();
	}
	//--------------------------------------------------------------
//	void EngineGLFW::onWindowResized(ofResizeEventArgs& window)
//	{
//		// Set context
//		if(!setImGuiContext()) return;

//		//ImGuiIO& io = ImGui::GetIO();
//		//io.DisplaySize = ImVec2((float)window.width, (float)window.height);

//		restoreImGuiContext();
//	}
#endif // End bind events to OF

} // End namespace ofxImGui




#endif
