#include "EngineOpenFrameworks.h"

// Custom backend include, only for getting keycodes, improving keyboard input
#if defined (TARGET_GLFW_WINDOW)
#include "ofAppGLFWWindow.h"
//#include "backends/imgui_impl_glfw.h"
#include "GLFW/glfw3.h" // needed by rpi
#endif

// Renderer includes
#include "ofGLProgrammableRenderer.h"
#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_opengl2.h"

#include "imgui.h"


namespace ofxImGui
{

	// Global storage for the latest clipboard string, ensures static storage
	std::string EngineOpenFrameworks::g_ClipboardText = "";

	//--------------------------------------------------------------
	void EngineOpenFrameworks::setup(ofAppBaseWindow* _window, ImGuiContext* _imguiContext, bool autoDraw)
	{
		// Store a reference to the current imgui context for event handling
		//imguiContext = ImGui::GetCurrentContext();

		ImGuiIO& io = ImGui::GetIO();

		// Todo: this feature could be enabled and mapped to OF. Seems rarely used.
		// io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // Not yet supported

		// Can we change mouse cursors ? Nope, not supported out of the box by OF...
		// io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

		// Store a reference to the current imgui context for event handling
		imguiContext = _imguiContext;

		//io.AddKeyEvent();
		io.DisplaySize = ImVec2( _window->getWindowSize() );//ofGetWindowSize() );
		io.DeltaTime = 1.0f / 60.0f; // start with non-null time
		io.WantCaptureMouse = true;

#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
		// Here we disable the code below to let users compile a project without throwing ofxImGui related warnings
		// Devs note: this functionality still needs to be updated before it depreciates !
		#pragma message "Notice: you are compiling with IMGUI_DISABLE_OBSOLETE_FUNCTIONS enabled, clipboard functions have been disabled (for EngineOpenFrameworks only)."
#else
		// Clipboard management
		io.SetClipboardTextFn = &EngineOpenFrameworks::setClipboardString;
		io.GetClipboardTextFn = &EngineOpenFrameworks::getClipboardString;
#endif

		// Backend name
		io.BackendPlatformName = "imgui_impl_ofximgui_native_of";
		io.BackendRendererName = "imgui_impl_opengl3";

	   // OpenGL setup
		if (ofIsGLProgrammableRenderer()){
			// Todo: Maybe better to replace with const getGLVersionFromOF(); ?
			// To be tested on multiple platforms.
#if defined( TARGET_OPENGLES )
			ImGui_ImplOpenGL3_Init( "#version 100" ); // Todo: we might need to pass another or no version string here
#else
			ImGui_ImplOpenGL3_Init();
#endif
		} else {
#if defined( TARGET_OPENGLES )
			ImGui_ImplOpenGL2_Init(); // Todo: we might need to pass a version string here
#else
			ImGui_ImplOpenGL2_Init(); // Todo: we might need to pass a version string here
#endif
		}
		// Create initial fonts texture
		updateFontsTexture();

		// Event listeners
		registerListeners();

		// We're done :)
		isSetup = true;
	} 

	//--------------------------------------------------------------
	void EngineOpenFrameworks::exit()
	{
		if (!isSetup) return;

		// Event listeners
		unregisterListeners();

		// Clear GPU data
		if (ofIsGLProgrammableRenderer()){
			ImGui_ImplOpenGL3_Shutdown();
		} else {
			ImGui_ImplOpenGL2_Shutdown();
		}

		//ImGui::DestroyContext();
		ImGuiIO& io = ImGui::GetIO();
		io.BackendPlatformName = nullptr;
		imguiContext = nullptr;
		isSetup = false;
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::newFrame()
	{
		// Need to set context here too ?

		// Set delta time
		ImGuiIO& io = ImGui::GetIO();
		float elapsed = ofGetLastFrameTime();
		io.DeltaTime = elapsed > 0.0 ? elapsed : (1.f/60.f); // Note: never be 0 !

		// Prepare graphics
		if (ofIsGLProgrammableRenderer()){
			ImGui_ImplOpenGL3_NewFrame();
		} else {
			ImGui_ImplOpenGL2_NewFrame();
		}
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::render()
	{
		// Need to set context here too ?

		// Flush imgui pipeline
		ImGui::Render();

		// Draw !
		if (ofIsGLProgrammableRenderer()){
			ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
		} else {
			ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
		}

	}

	//--------------------------------------------------------------
	bool EngineOpenFrameworks::updateFontsTexture(){
		// Set context ?

		// The renderer does it for us :)
		if (ofIsGLProgrammableRenderer()) {
			return ImGui_ImplOpenGL3_CreateFontsTexture();
		}
		else {
			return ImGui_ImplOpenGL2_CreateFontsTexture();
		}
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::onMouseMoved(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		// Update imgui mouse pos
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(static_cast<float>(event.x), static_cast<float>(event.y));

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::onMouseDragged(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		// Update imgui mouse pos
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(static_cast<float>(event.x), static_cast<float>(event.y));

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::onMouseScrolled(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		// Update imgui mouse pos
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent( static_cast<float>(event.scrollX), static_cast<float>(event.scrollY) );

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::onMouseButton(ofMouseEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;

		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseButtonEvent(event.button, event.type == ofMouseEventArgs::Pressed);

		restoreImGuiContext();
	}

#ifdef OFXIMGUI_TOUCH_EVENTS
	//--------------------------------------------------------------
	void EngineOpenFrameworks::onTouchInput(ofTouchEventArgs& event)
	{
		// Set context
		if(!setImGuiContext()) return;
		
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent(event.x, event.y);
		static bool isDown; isDown = (event.type == ofTouchEventArgs::down || event.type == ofTouchEventArgs::doubleTap);
		switch(event.type){
			case ofTouchEventArgs::down :
			case ofTouchEventArgs::cancel :
			case ofTouchEventArgs::up :
			//case ofTouchEventArgs::doubleTap :
				io.AddMouseButtonEvent(ImGuiMouseButton_Left, isDown);
				break;
			default:
				break;
		}
		
		restoreImGuiContext();
	}
#endif

	//--------------------------------------------------------------
	void EngineOpenFrameworks::onKeyEvent(ofKeyEventArgs& event)
	{
		// Ignore repeats
		if(event.isRepeat) return;

		// Set context
		if(!setImGuiContext()) return;

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
		if(imKey == ImGuiKey_None){
			// Note: codepoint corresponds to the typed character
			imKey = keyCodeToImGuiKey( event.codepoint );
		}

		io.AddKeyEvent(imKey, event.type == ofKeyEventArgs::Pressed );

		// Note: this brings support for pre-1.87 user code using very specific API code.
		// It causes an assert/crash in imgui v1.89.3 when releasing ALT+CMD simultanously (osx+backend_of_native), when IMGUI_DISABLE_OBSOLETE_KEYIO is not defined.
		// As we disabled it, we can support old user code again.
		io.SetKeyEventNativeData(imKey, key, event.scancode); // To support legacy indexing (<1.87 user code)

		// Note: Not anymore, we do this in the charInput callback now.
		// io.AddInputCharacter((unsigned short)event.codepoint);

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::onCharInput(uint32_t& _char)
	{
		// Set context
		if(!setImGuiContext()) return;

		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter((unsigned short)_char);

		restoreImGuiContext();
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::onWindowResized(ofResizeEventArgs& window)
	{
		// Set context
		if(!setImGuiContext()) return;

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)window.width, (float)window.height);

		restoreImGuiContext();
	}

	// Constexpr ascii table math, which will be optimised by the compiler. Small overhead on debug.
//	inline constexpr char constexprCharToLower(const char c) {
//		return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
//	}
//	inline constexpr char constexprCharToUpper(const char c) {
//		//return c + (c - 'a' + 'A');
//		return (c >= 'a' && c <= 'z') ? c + (c - 'a' + 'A') : c;
//	}

	//--------------------------------------------------------------
	// Utility to guess the key from the keycode, which is more universal amongst platforms
	ImGuiKey EngineOpenFrameworks::keyCodeToImGuiKey(int keyCode)
	{
#define DOUBLE_QUOTE(x) #x
#define SINGLE_QUOTE(x) DOUBLE_QUOTE(x)[0]
//#define LOWERTOUPPER(x) ((SINGLE_QUOTE(x) - 'a') + 'A')
#define UPPERTOLOWER(x) (SINGLE_QUOTE(x) + ('a' - 'A'))
#define MAKE_VARNAME(before, var) before##var
#define MAP_LETTER(upper) case SINGLE_QUOTE(upper): case UPPERTOLOWER(upper): return MAKE_VARNAME(ImGuiKey_,upper);
#define MAP_NUMBER(number) case (char)(number): return MAKE_VARNAME(ImGuiKey_,number);
#define MAP_SIGN(sign, signname) case sign: return MAKE_VARNAME(ImGuiKey_,signname);

//#define MAP_KEY(lower, upper) case lower: case upper: return ImGuiKey_##upper;
		switch (keyCode)
		{
			// a-z and A-Z
			MAP_LETTER(A)
			MAP_LETTER(B)
			MAP_LETTER(C)
			MAP_LETTER(D)
			MAP_LETTER(E)
			MAP_LETTER(F)
			MAP_LETTER(G)
			MAP_LETTER(H)
			MAP_LETTER(I)
			MAP_LETTER(J)
			MAP_LETTER(K)
			MAP_LETTER(L)
			MAP_LETTER(M)
			MAP_LETTER(N)
			MAP_LETTER(O)
			MAP_LETTER(P)
			MAP_LETTER(Q)
			MAP_LETTER(R)
			MAP_LETTER(S)
			MAP_LETTER(T)
			MAP_LETTER(U)
			MAP_LETTER(V)
			MAP_LETTER(W)
			MAP_LETTER(X)
			MAP_LETTER(Y)
			MAP_LETTER(Z)
			// 0-9
			MAP_NUMBER(0)
			MAP_NUMBER(1)
			MAP_NUMBER(2)
			MAP_NUMBER(3)
			MAP_NUMBER(4)
			MAP_NUMBER(5)
			MAP_NUMBER(6)
			MAP_NUMBER(7)
			MAP_NUMBER(8)
			MAP_NUMBER(9)
			// Some signs (todo: add more?)
			MAP_SIGN('.', Period)
			MAP_SIGN(',', Comma)
			MAP_SIGN('-', Minus)
			MAP_SIGN('\'',Apostrophe)
			MAP_SIGN('/', Slash)
			MAP_SIGN(';', Semicolon)
			MAP_SIGN('=', Equal)
			MAP_SIGN('[', LeftBracket)
			MAP_SIGN('\\',Backslash)
			MAP_SIGN(']', RightBracket)
			MAP_SIGN(' ', Space)
		}
		return ImGuiKey_None;
	}

	//--------------------------------------------------------------
	const char* EngineOpenFrameworks::getClipboardString(void * userData)
	{
		g_ClipboardText = ofGetWindowPtr()->getClipboardString();
		return g_ClipboardText.c_str();
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::setClipboardString(void * userData, const char * text)
	{
		g_ClipboardText = ofToString(text);
		ofGetWindowPtr()->setClipboardString(g_ClipboardText);
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::registerListeners(){
		// Mouse events
		ofAddListener(ofEvents().mouseMoved,    this, &EngineOpenFrameworks::onMouseMoved   );
		ofAddListener(ofEvents().mouseDragged,  this, &EngineOpenFrameworks::onMouseDragged );
		ofAddListener(ofEvents().mousePressed,  this, &EngineOpenFrameworks::onMouseButton  );
		ofAddListener(ofEvents().mouseReleased, this, &EngineOpenFrameworks::onMouseButton  );
		ofAddListener(ofEvents().mouseScrolled, this, &EngineOpenFrameworks::onMouseScrolled);
		
#ifdef OFXIMGUI_TOUCH_EVENTS
		// TouchEvents
//		ofAddListener(ofEvents().touchDoubleTap,this, &EngineOpenFrameworks::onTouchDoubleTap   );
//		ofAddListener(ofEvents().touchMoved,    this, &EngineOpenFrameworks::onTouchMoved );
//		ofAddListener(ofEvents().touchDown,     this, &EngineOpenFrameworks::onTouchDown  );
//		ofAddListener(ofEvents().touchUp,       this, &EngineOpenFrameworks::onTouchUp  );
//		ofAddListener(ofEvents().touchCancelled, this, &EngineOpenFrameworks::onTouchCancelled);
		ofAddListener(ofEvents().touchDoubleTap,this, &EngineOpenFrameworks::onTouchInput );
		ofAddListener(ofEvents().touchMoved,    this, &EngineOpenFrameworks::onTouchInput );
		ofAddListener(ofEvents().touchDown,     this, &EngineOpenFrameworks::onTouchInput );
		ofAddListener(ofEvents().touchUp,       this, &EngineOpenFrameworks::onTouchInput );
		ofAddListener(ofEvents().touchCancelled, this, &EngineOpenFrameworks::onTouchInput);
#endif
		// Keyboard avents
		ofAddListener(ofEvents().keyReleased,   this, &EngineOpenFrameworks::onKeyEvent );
		ofAddListener(ofEvents().keyPressed,    this, &EngineOpenFrameworks::onKeyEvent );
		ofAddListener(ofEvents().charEvent,     this, &EngineOpenFrameworks::onCharInput);

		// Window Listeners
		ofAddListener(ofEvents().windowResized, this, &EngineOpenFrameworks::onWindowResized);
		//ofAddListener(ofEvents().windowMoved, this, &EngineOpenFrameworks::onWindowResized); // Seems unnecssary
#ifdef OFXIMGUI_TOUCH_EVENTS
		// TouchEvents
		//ofAddListener(ofEvents().touchDoubleTap,this, &EngineOpenFrameworks::onDeviceOrientationChanged);
#endif
		// Additional mouse data
		ofAddListener(ofEvents().mouseEntered, this, &EngineOpenFrameworks::onMouseMoved);
		ofAddListener(ofEvents().mouseExited , this, &EngineOpenFrameworks::onMouseMoved);

		// ImGui also has io.AddFocusEvent but OF hasn't got them.
	}

	//--------------------------------------------------------------
	void EngineOpenFrameworks::unregisterListeners(){
		// Mouse events
		ofRemoveListener(ofEvents().mouseMoved,     this, &EngineOpenFrameworks::onMouseMoved   );
		ofRemoveListener(ofEvents().mouseDragged,   this, &EngineOpenFrameworks::onMouseDragged );
		ofRemoveListener(ofEvents().mousePressed,   this, &EngineOpenFrameworks::onMouseButton  );
		ofRemoveListener(ofEvents().mouseReleased,  this, &EngineOpenFrameworks::onMouseButton  );
		ofRemoveListener(ofEvents().mouseScrolled,  this, &EngineOpenFrameworks::onMouseScrolled);
		
#ifdef OFXIMGUI_TOUCH_EVENTS
		// TouchEvents
//		ofRemoveListener(ofEvents().touchDoubleTap,this, &EngineOpenFrameworks::onTouchDoubleTap   );
//		ofRemoveListener(ofEvents().touchMoved,    this, &EngineOpenFrameworks::onTouchMoved );
//		ofRemoveListener(ofEvents().touchDown,     this, &EngineOpenFrameworks::onTouchDown  );
//		ofRemoveListener(ofEvents().touchUp,       this, &EngineOpenFrameworks::onTouchUp  );
//		ofRemoveListener(ofEvents().touchCancelled, this, &EngineOpenFrameworks::onTouchCancelled);
		ofRemoveListener(ofEvents().touchDoubleTap,this, &EngineOpenFrameworks::onTouchInput );
		ofRemoveListener(ofEvents().touchMoved,    this, &EngineOpenFrameworks::onTouchInput );
		ofRemoveListener(ofEvents().touchDown,     this, &EngineOpenFrameworks::onTouchInput );
		ofRemoveListener(ofEvents().touchUp,       this, &EngineOpenFrameworks::onTouchInput );
		ofRemoveListener(ofEvents().touchCancelled, this, &EngineOpenFrameworks::onTouchInput );
#endif

		// Keyboard avents
		ofRemoveListener(ofEvents().keyReleased,    this, &EngineOpenFrameworks::onKeyEvent );
		ofRemoveListener(ofEvents().keyPressed,     this, &EngineOpenFrameworks::onKeyEvent );
		ofRemoveListener(ofEvents().charEvent,      this, &EngineOpenFrameworks::onCharInput);

		// Window Listeners
		ofRemoveListener(ofEvents().windowResized,  this, &EngineOpenFrameworks::onWindowResized);

		// Additional mouse data
		ofRemoveListener(ofEvents().mouseEntered, this, &EngineOpenFrameworks::onMouseMoved);
		ofRemoveListener(ofEvents().mouseExited , this, &EngineOpenFrameworks::onMouseMoved);
	}
}

