#include "Gui.h"

#include "ofAppRunner.h"
#include "ofAppGLFWWindow.h"

#include "ofxImGuiConstants.h"
#include "imgui_internal.h" // <-- advanced docking features from imgui internals...

//#include "imgui.h"
//#include "backends/imgui_impl_glfw.h"

// For apps that compile without GLFW
#ifdef OFXIMGUI_DEBUG
//	#ifndef GLFW_VERSION_MAJOR
//		#define GLFW_VERSION_MAJOR 0
//	#endif
//	#ifndef GLFW_VERSION_MINOR
//		#define GLFW_VERSION_MINOR 0
//	#endif
//	#ifndef GLFW_VERSION_REVISION
//		#define GLFW_VERSION_REVISION 0
//	#endif

// Some includes to debug GLFW
#ifdef OFXIMGUI_BACKEND_GLFW
		//#include "backends/imgui_impl_glfw.h"
		#include <GLFW/glfw3.h> // For getting macro-defined versions
		#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
			#include "backends/imgui_impl_glfw_context_support.h"
		#endif
#endif
#endif

//#ifdef OFXIMGUI_BACKEND_GLFW
//#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
//#include "backends/imgui_impl_glfw_context_support.h"
//#endif
//#endif


// Todo: Texture/Pixel loading functions: Stop using ofDisableArbTex and use appropriate textureSettings instead.

namespace ofxImGui
{
	std::ostream& operator<<(std::ostream& os, const SetupState& _state)
	{
		os << std::bitset<sizeof(SetupState)*8>(_state).to_string() << " (" << ((unsigned int)_state) << ")";
		return os;
	}

	//--------------------------------------------------------------
	Gui::Gui() : context(nullptr)
	{
        // This performs an ImGui version check (asserts on fail)
        // Probably of interest for DLL code that needs to draw to ImGui.
        IMGUI_CHECKVERSION();

        // Note: ofGetWindowPtr() is not correct at this point in multi-window setups.
        // Cannot call ImGui::CreateContext here
    }

	//--------------------------------------------------------------
	Gui::~Gui()
	{
		exit();
	}

	// Retro-compatibility with older setup functions
	SetupState Gui::setup(BaseTheme* theme_, bool autoDraw_, ImGuiConfigFlags customFlags_, bool _restoreGuiState, bool _showImGuiMouseCursor )
	{
		auto window = ofGetCurrentWindow();
		IM_ASSERT( window && window != nullptr && "You must call setup() from within an active ofAppBaseWindow, or provide a pointer to it." );
		return setup( window, theme_, autoDraw_, customFlags_, _restoreGuiState, _showImGuiMouseCursor);
	}

	//--------------------------------------------------------------
    // Must be called from the oF window that's gonna display the gui.
    // Creates 1 imgui context per oF window (glfw context).
    // Links the context handle to the correct window's imguicontext.
    // Todo: an optional ofAppBaseWindow* to specify a manual context instead of using the current active ofApp window.
	// Returns a state indcating if the setup happened as slave or master
	SetupState Gui::setup(std::shared_ptr<ofAppBaseWindow>& _ofWindow, BaseTheme* theme_, bool autoDraw_, ImGuiConfigFlags customFlags_, bool _restoreGuiState, bool _showImGuiMouseCursor )
	{
#ifdef OFXIMGUI_DEBUG
		ofLogVerbose("Gui::setup()") << "Setting up ofxImGui [" << this << "] in window " << _ofWindow.get();
#endif
        // Instance already setup ?
		if( context!=nullptr ){
#ifdef OFXIMGUI_DEBUG
            ofLogWarning("Gui::setup()") << "This Gui instance is already setup, you are calling it twice ! (ignoring this 2nd call)";
#endif
			return isContextOwned ? SetupState::Master : SetupState::Slave;
        }

        // Curwindow cannot be null
		if(_ofWindow==nullptr){
			ofLogError("Gui::setup()") << "The provided ofAppBaseWindow pointer is null, cannot continue setup !";
			return SetupState::Error;
        }

		// Grab existing ImGui Context
		//ImGuiContext* existingImGuiContext = ImGui::GetCurrentContext(); // Null on first call ever

		// Does this ofBaseWindow already have an ofxImGuiContext ?
		ofxImGuiContext* existingOfWindowContext = nullptr;
		auto foundWindowContext = imguiContexts.find( _ofWindow.get() );
		if( foundWindowContext != imguiContexts.end() ) existingOfWindowContext = &foundWindowContext->second ;

		if( existingOfWindowContext != nullptr ){
			existingOfWindowContext->slaveCount++; // tells master that the context became shared

			// Keep reference of master context
			context = existingOfWindowContext;
			isContextOwned = false;

#ifdef OFXIMGUI_DEBUG
			ofLogNotice("Gui::setup()") << "Context " << context << "/" << context->imguiContext << " already exists in window " << _ofWindow.get() << ", using the existing context as a shared one.";
			if(autoDraw_) ofLogWarning("Gui::setup()") << "You requested to enable Autodraw, but this is a Slave setting. Not enabling autoDraw.";
#endif
        }
        // Create a unique context for this window
        else {
			//context = new ofxImGuiContext(_ofWindow);
//			std::pair<ofAppBaseWindow*, ofxImGuiContext> myPair(std::piecewise_construct, std::forward_as_tuple(_ofWindow.get()), std::forward_as_tuple(_ofWindow));
			//std::pair<ofAppBaseWindow*, ofxImGuiContext> myPair(std::piecewise_construct, std::forward_as_tuple(_ofWindow.get()), std::forward_as_tuple(_ofWindow));

			// Register
			//imguiContexts.emplace(_ofWindow.get(), _ofWindow);
			//imguiContexts[_ofWindow.get()] = ofxImGuiContext(_ofWindow);
			auto createdContext = imguiContexts.emplace(std::piecewise_construct, std::forward_as_tuple(_ofWindow.get()), std::forward_as_tuple(_ofWindow));
			if(!createdContext.second){ // Failed creating ?
				ofLogError("Gui::setup()") << "Couldn't create a context for this window !";
				return SetupState::Error;
			}
			context = &createdContext.first->second;
			context->slaveCount ++; // Master counts as a slave too ;)
			isContextOwned = true;

			// Enable autodraw
			if( autoDraw_ && _ofWindow!=nullptr ){
				context->autoDraw = true;
				autoDrawListener = _ofWindow->events().draw.newListener( this, &Gui::afterDraw, OF_EVENT_ORDER_AFTER_APP );
            }

#ifdef OFXIMGUI_DEBUG
			ofLogVerbose("Gui::setup()") << "Created context "<< context << " in window " << _ofWindow.get() << " ["<< ofGetWindowPtr()->getWindowSize() <<"]";
#endif
        }

		// Set context and grab IO
		ImGui::SetCurrentContext(context->imguiContext);
        ImGuiIO& io = ImGui::GetIO();

		// Dummy call that will crash if the io is invalid --> ie for easier debugging
		(void)io;

        // Note : In chaining mode, additional flags can still be set.
        io.ConfigFlags |= customFlags_;

		// Already-setup window --> slaves exit early
		if( !isContextOwned ) {
#ifdef OFXIMGUI_DEBUG
            ofLogVerbose("Gui::setup()") << "Context is not owned, skipping some settings and not binding the engine again.";
#endif
			return SetupState::Slave;
        }

		// Master may set more settings
		// Mouse cursor drawing (disabled by default, oF uses the system mouse)
		io.MouseDrawCursor = _showImGuiMouseCursor;

		// Handle gui state saving
		if(_restoreGuiState == false)
			io.IniFilename = nullptr;

		// Start engines
		this->context->engine.setup( _ofWindow.get(), context->imguiContext, context->autoDraw);

		// Load a default font
		// Fixme: make this optional ? (= less GPU memory).
		// For now we ensure a default font is always available, for the ease of use.
		ImFont* defaultFont = nullptr;
		if(io.Fonts->Fonts.size()==0){
		   defaultFont = io.Fonts->AddFontDefault();

            // Set as default
            if(defaultFont){
                rebuildFontsTexture(); // Fixme: could be an optional call to improve loading speeds (only needs to be called once after loading all fonts).

                // Ensure the default font is marked default (can be user-overridden later)
                if(!io.FontDefault){
                    //io.FontDefault = defaultFont;
                    setDefaultFont(defaultFont);
                }
            }
        }

		// Load a theme
		if (theme_)
		{
			setTheme(theme_);
		}
		else
		{
			DefaultTheme* defaultTheme = new DefaultTheme();
			setTheme((BaseTheme*)defaultTheme);
		}

		return SetupState::Master;
	}

	//--------------------------------------------------------------
	void Gui::exit()
	{
        // Exit engine
		if(isContextOwned){

			// Unregister the afterDraw() callback (if used)
			autoDrawListener.unsubscribe();

			ImGui::SetCurrentContext(context->imguiContext);
			this->context->engine.exit();

			// Theme
			if (theme)
			{
				delete theme;
				theme = nullptr;
			}
        }

		// Textures (slaves can have some too)
		for (size_t i = 0; i < loadedTextures.size(); i++)
		{
            if(loadedTextures[i])
            {
                delete loadedTextures[i];
                loadedTextures[i] = NULL;
            }
		}
		loadedTextures.clear();

        // Destroy context
		if(isContextOwned){
			// What if slaves exit() after the master ?
			// Todo: maybe leave it alive ? Transfer ownership to slave ?
			if(context!=nullptr){

				// Unregister
//				ofAppBaseWindow* myOfWindow = imguiContexts.find(context);
//				if(myOfWindow!=nullptr){
//					imguiContexts.remove(myOfWindow);
//				}
//				else {
//					ofLogWarning("Gui::exit()") << "Master could not unregister the context !";
//				}

#ifdef OFXIMGUI_DEBUG
//			ofLogNotice("Gui::exit()") << "Destroyed master context " << context << " that was bound to window " << myOfWindow << " (together with ImGuiContext" << context->imguiContext << ").";
#endif

				// Destroy
				ImGui::DestroyContext(context->imguiContext);

				// Set context to null so that slaves can know the context is gone.
				context->imguiContext = nullptr;

				isContextOwned = false;
				//delete context;
			}
        }
		else {
#ifdef OFXIMGUI_DEBUG
			ofLogNotice("Gui::exit()") << "Destroyed slave context " << context << ".";
#endif

			// Slaves set their context to nullptr without destroying anything
			context = nullptr;
		}
		
	}

    //--------------------------------------------------------------
	// In some rare cases you might wish to enable this manually. (in most cases it's automatic)
//    void Gui::setSharedMode(bool _sharedMode) {
//		if(!_sharedMode || !context || !context->imguiContext) return;
//		context->slaveCount++;
//    }
	// Returns true if the context is setup in shared mode
	// (when multiple Gui instances share the same context).
    bool Gui::isInSharedMode() const {
		if( !context ) return false;
		return context->isShared();
    }

    bool Gui::isMaster() const {
        return isContextOwned;
    }

    //--------------------------------------------------------------
    bool Gui::setDefaultFont(int indexAtlasFont) {
        if(context==nullptr){
            ofLogWarning("Gui::setDefaultFont()") << "You must load fonts after gui.setup() ! (ignoring this call)";
            return false;
        }

		ImGui::SetCurrentContext(context->imguiContext);
        ImGuiIO& io = ImGui::GetIO();
        if( indexAtlasFont < 0 ){
            io.FontDefault = NULL; // default value, uses 0
        }
        else if (indexAtlasFont < io.Fonts->Fonts.size()) {
            io.FontDefault = io.Fonts->Fonts[indexAtlasFont];
            return true;
        }
        else {
            io.FontDefault = io.Fonts->Fonts[0];
        }
        return false;
    }

    bool Gui::setDefaultFont(ImFont* _atlasFont){
        if(context==nullptr){
            ofLogWarning("Gui::setDefaultFont") << "You must load fonts after gui.setup() ! (ignoring this call)";
            return false;
        }

		ImGui::SetCurrentContext(context->imguiContext);
        ImGuiIO& io = ImGui::GetIO();

		// Don't override default font with nullptr
		if( _atlasFont != nullptr ){
			// Loop existing fonts to ensure setting an available font
            for(int i=0; i<io.Fonts->Fonts.size(); ++i){
                if(io.Fonts->Fonts[i] == _atlasFont){
                    // Set font
                    io.FontDefault = _atlasFont;
					return true;
                }
            }
        }
        return false;
    }

	//--------------------------------------------------------------
	ImFont* Gui::addFont(const std::string & fontPath, float fontSize, const ImFontConfig* _fontConfig, const ImWchar* _glyphRanges, bool _setAsDefaultFont ) {

		if(context==nullptr){
			ofLogWarning("Gui::addFont()") << "You must load fonts after gui.setup() ! (ignoring this call)";
			return nullptr;
		}

		// ImFontConfig structure allows you to configure oversampling.
		// By default OversampleH = 3 and OversampleV = 1 which will make your font texture data 3 times larger
		// than necessary, so you may reduce that to 1.

		ImGui::SetCurrentContext(context->imguiContext);
		ImGuiIO& io = ImGui::GetIO();
        std::string filePath = ofFilePath::getAbsolutePath(fontPath);


        ImFont* font = io.Fonts->AddFontFromFileTTF(filePath.c_str(), fontSize, _fontConfig, _glyphRanges);

		if (font != nullptr){
			 // Fixme: could be an optional call to improve loading speeds (only needs to be called once after loading all fonts).
			 rebuildFontsTexture();

			if(_setAsDefaultFont) setDefaultFont(font);

			return font;
		}
		else {
			return nullptr;
		}
	}
	//--------------------------------------------------------------
	ImFont* Gui::addFontFromMemory(void* fontData, int fontDataSize, float fontSize, const ImFontConfig* _fontConfig, const ImWchar* _glyphRanges, bool _setAsDefaultFont ) {

		if(context==nullptr){
		  ofLogWarning("Gui::addFontFromMemory()") << "You must load fonts after gui.setup() ! (ignoring this call)";
		  return nullptr;
		}

		// ImFontConfig structure allows you to configure oversampling.
		// By default OversampleH = 3 and OversampleV = 1 which will make your font texture data 3 times larger
		// than necessary, so you may reduce that to 1.

		ImGui::SetCurrentContext(context->imguiContext);
		ImGuiIO& io = ImGui::GetIO();


		ImFont* font = io.Fonts->AddFontFromMemoryTTF( fontData, fontDataSize, fontSize, _fontConfig, _glyphRanges);

		if (font != nullptr){
			// Fixme: could be an optional call to improve loading speeds (only needs to be called once after loading all fonts).
			rebuildFontsTexture();
			if(_setAsDefaultFont) setDefaultFont(font);
			return font;
		}
		else {
			return nullptr;
		}
	}

	//--------------------------------------------------------------
	bool Gui::rebuildFontsTexture(){
		if(context==nullptr){
		  ofLogWarning("Gui::rebuildFontsTexture()") << "You must build fonts after gui.setup() ! (ignoring this call)";
		  return false;
		}

		ImGui::SetCurrentContext(context->imguiContext);
		ImGuiIO& io = ImGui::GetIO();

		if (io.Fonts->Fonts.size() > 0) {
			io.Fonts->Build();
			return context->engine.updateFontsTexture();
		}
		return false;
	}

	//--------------------------------------------------------------
	void Gui::setTheme(BaseTheme* theme_)
	{
		// Handle ownership of the previous theme (we do take ownership)
		if (theme)
		{
			delete theme;
			theme = nullptr;
		}
		theme = theme_;

		ImGui::SetCurrentContext(context->imguiContext);
		theme->setup();

        // Source : https://github.com/yumataesu/ofxImGui_v3/blob/23ff3e02ae3b99cb3db449b950c2f3e34424fbc8/src/ofxImGui.cpp#L12-L18
        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
		if(
				// Viewports are enabled
				(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				// Platform&Renderer backends both support viewports
				&& (ImGui::GetIO().BackendFlags & (ImGuiBackendFlags_PlatformHasViewports))
				&& (ImGui::GetIO().BackendFlags & (ImGuiBackendFlags_RendererHasViewports))
		){
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
	}



	//--------------------------------------------------------------
	GLuint Gui::loadPixels(ofPixels& pixels)
	{
		return context->engine.loadTextureImage2D(pixels.getData(), pixels.getWidth(), pixels.getHeight());
	}

	//--------------------------------------------------------------
	GLuint Gui::loadPixels(const std::string& imagePath)
	{
		ofPixels pixels;
		ofLoadImage(pixels, imagePath);
		return loadPixels(pixels);
	}

	//--------------------------------------------------------------
	GLuint Gui::loadImage(ofImage& image)
	{
		return loadPixels(image.getPixels());
	}

	//--------------------------------------------------------------
	GLuint Gui::loadImage(const std::string& imagePath)
	{
		return loadPixels(imagePath);
	}

	//--------------------------------------------------------------
	GLuint Gui::loadTexture(const std::string& imagePath)
	{
		ofDisableArbTex();
		ofTexture* texture = new ofTexture();
		ofLoadImage(*texture, imagePath);
		ofEnableArbTex();
		loadedTextures.push_back(texture);
		return texture->getTextureData().textureID;
	}

	//--------------------------------------------------------------
	GLuint Gui::loadTexture(ofTexture& texture, const std::string& imagePath)
	{
		bool isUsingArb = ofGetUsingArbTex();
		if (isUsingArb)
		{
			ofDisableArbTex();
		}
		ofLoadImage(texture, imagePath);
		if (isUsingArb)
		{
			ofEnableArbTex();
		}
		return texture.getTextureData().textureID;
	}

	//--------------------------------------------------------------
	void Gui::begin()
	{
        if(context==nullptr){
#ifdef OFXIMGUI_DEBUG
            ofLogWarning("Gui::begin()") << "Context is not set. Please call setup() first !";
#endif
            return;
        }

        // Ignore 2nd call to begin(), to allow chaining
		if( context->isRenderingFrame == true ){
            // Already began context
			if(!context->isShared()){
				// Notify misuse if shared context is off
				static bool userWasWarned = false;
				if(!userWasWarned){
					ofLogWarning("Gui::begin()") << "The frame already began, and the context is not shared, you're probably not orchestrating begin/end calls correctly, or you forgot to call gui.draw() in manual draw mode !";
					userWasWarned = true;
				}
			}
            return;
        }

		ImGui::SetCurrentContext(context->imguiContext);

        // Help people loading fonts incorrectly
        ImGuiIO& io = ImGui::GetIO();
        IM_ASSERT( io.Fonts->IsBuilt() );

        //std::cout << "New Frame in context " << context << " in window " << ofGetWindowPtr() << " (" << ofGetWindowPtr()->getWindowSize().x << ")" << std::endl;
		context->engine.newFrame();
        ImGui::NewFrame();

		context->isRenderingFrame = true;
	}

	//--------------------------------------------------------------
	void Gui::end()
    {
        if(context==nullptr){
#ifdef OFXIMGUI_DEBUG
            ofLogWarning("Gui::end()") << "Context is not set. Please call setup() first !";
#endif
            return;
        }

		// Let context open in shared mode. (automatically called in the native render function)
		if( context->isShared()==true ){
#ifdef OFXIMGUI_DEBUG
			if( !context->isRenderingFrame ){
				ofLogWarning("Gui::end()") << "The Gui already rendered, or you forgot to call Gui::Begin() ! Please ensure you render the gui after other instances have rendered.";
            }
#endif
            // Todo: if user doesn't gui.draw() (autodraw=0), ImGui::EndFrame() is not called, causing a "forgot to BeginFrame()" assert
            return;
        }

        // Update cached variables
        updateDockingVp();
        // Update height
        ImGuiWindow* menuWin = ImGui::FindWindowByID(ImGui::GetIDWithSeed("##MainMenuBar", nullptr, 0));
        menuHeight = (menuWin && !menuWin->Hidden && menuWin->Active) ? ImGui::GetFrameHeight() : 0;

        // Only render in autodraw mode.
		if(context->autoDraw){
            // Protection : don't endFrame() twice.
			if( !context->isRenderingFrame ){
                return;
            }

            // End submitting to ImGui
			//context->engine.endFrame();
            ImGui::EndFrame();

            //render(); // Now called after ofApp::draw() using a callback.
        }
        // Otherwise end the frame. User chooses when to render (later) using Gui::draw()
        // Note: You cannot resume using ImGui::NewFrame() without flushing the pipeline.
        else {
            // Protection : don't endFrame() twice.
			if( !context->isRenderingFrame ){
                return;
            }

			//context->engine.endFrame(); // (Does nothing...)
            ImGui::EndFrame();
        }
    }

    //--------------------------------------------------------------
    // Note: render() is private, never called by the end user.
    void Gui::render(){
        if( context==nullptr ) return;

		ImGui::SetCurrentContext(context->imguiContext);
        ImGui::Render();
		context->engine.render();
		context->isRenderingFrame = false;
    }

	//--------------------------------------------------------------
    void Gui::draw()
	{
		if(!context) return;

        // Just for clarity, we require draw() not to be used in autoDraw mode.
		if( context->autoDraw ){
#ifdef OFXIMGUI_DEBUG
            ofLogWarning("ofxImGui::Gui::draw()") << "Please don't call draw() in autoDraw mode.";
#endif
            return;
        }
		else if(context->isRenderingFrame==true){
            render();
		}
	}

	//--------------------------------------------------------------
    void Gui::afterDraw( ofEventArgs& ){

        // This function is registered after ofApp::draw() to honor autodraw in shared context mode.
		if(context && context->isRenderingFrame ){
            // Autodraw renders here if sharedMode is on
			if( context->autoDraw && context->isShared()) render();
            // Render if manual render was forgotten (do we want this to happen??)
			else if( !context->isShared() ) render();
        };
    }

	//--------------------------------------------------------------
	void Gui::drawOfxImGuiDebugWindow(bool* open) const {
		// Only provide this functions with debug flags on
#ifdef OFXIMGUI_DEBUG
		if( ImGui::Begin("ofxImGui Debug Window", open) ){

			if(ImGui::BeginTabBar("DebugTabs")){

				// Doctor
				if (ImGui::BeginTabItem("Doctor"))
				{
					ImGui::Dummy({10,10});
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
					ImGui::TextWrapped("The doctor cares about the health of your ofxImGui configuration.");
					ImGui::TextWrapped("As ofxImGui offers various configuration settings that come with their pros and cons, this view provides some hints for configuring it to your needs.");
					ImGui::PopStyleColor();

					// GLFW
#if defined(OFXIMGUI_BACKEND_GLFW)
					ImGui::Dummy({10,10});
					ImGui::SeparatorText("GLFW Backend");
					ImGui::TextWrapped("You are using the more featured GLFW backend.");
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
					ImGui::TextWrapped("This backend interfaces ImGui directly to the underlying GLFW window that your ofApp runs.");
					ImGui::TextWrapped("It features all of ImGui's features, including viewports, those ImGui windows that seamlessly pop-out of your ofAppWindow.");
					ImGui::PopStyleColor();
					// Backend differences
					ImGui::Dummy({10,10});
					ImGui::Text("Limitations compared to the OpenFrameworks backend:");
					ImGui::Bullet(); ImGui::TextWrapped("As there are more features, the implementation has a lot more bindings (and even some hacks), which makes it harder to maintain, and it's a bit more likely to break with future imgui updates. (on the other side, it\'s using the native backend, changes are documented and most oftenly easy to fix).");
					ImGui::Bullet(); ImGui::TextWrapped("The OpenFrameworks backend can always be used as an alternative, which ensures a fallback.");

					// GLFW Checks
					ImGui::Dummy({10,10});
					ImGui::Text("Tips:");
					// Event binding
	#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1
					ImGui::Bullet(); ImGui::TextWrapped("The GLFW backend has to replace the openFrameworks GLFWWindow events so that ofxImGui can intercept them, forwarding them afterwards.\nThis introduces a tiny extra delay and it 'alters' of-internals (without interfering with them).\nAlternatively you may set `OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS` to rather listen to oF events, but some information is lost and the user experience will be reduced. ");
	#else
					ImGui::Bullet(); ImGui::TextWrapped("You are using `OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS` which binds events to openFrameworks rather then GLFW.\nSome input information is lost and the user experience will be reduced. ");
	#endif
					// Outdated GLFW ?
	#if GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR <= 3 && GLFW_VERSION_MINOR <= 3
					ImGui::Bullet(); ImGui::TextWrapped("Your GLFW version (%i.%i.%i) is quite old, you could update it to get a smoother user interface experience.\nRefer to Developers.md.", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_MINOR);
	#endif
	#ifdef TARGET_LINUX
					ImGui::Bullet(); ImGui::TextWrapped("On Linux, moving pop-out windows (viewports enabled) with the mouse is quite shaky!\nThis also happens in the standalone imgui glfw opengl example so it's an ImGui bug.");
	#endif
					// OSX pre-3.3.0 GLFW Warning
	#if defined(TARGET_OSX) && OF_VERSION_MAJOR == 0 && OF_VERSION_MINOR == 11 && ( OF_VERSION_PATCH == 2 || OF_VERSION_PATCH == 0 )
					ImGui::Dummy({10,10});
					ImGui::Bullet(); ImGui::TextWrapped("On OSX and OF 0.11.0 and OF 0.11.2, GLFW ships as a pre-3.3.0 release, breaking native ImGui compatibility.\nBut we got you covered by disabling some advanced ImGui features !\nAlternatively you can update GLFW (refer to Developers.md).");
	#endif
	#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 0
					ImGui::TextWrapped("You are using an unmodified imgui_backend_glfw : you can't use multiple ofAppBaseWindows together with viewports enabled.");
	#endif

#elif defined(OFXIMGUI_BACKEND_OPENFRAMEWORKS)
					ImGui::Dummy({10,10});
					ImGui::SeparatorText("Native Openframeworks Backend");
					ImGui::TextWrapped("You are using the fallback Openframeworks backend.");
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
					ImGui::TextWrapped("This backend interfaces the minimum required interactions to use ImGui with ofEvents, so interactions are a little less user-friendly and some advanced ImGui features are missing.\nMeanwhile, it's also less likely to break on future ImGui releases, which ensures a more-stable-over-time fallback for the fully featured GLFW backend.");
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
					//ImGui::SeparatorText("Backend Usage Tips");
					ImGui::Text("Limitations compared to the GLFW backend:");
					ImGui::Bullet(); ImGui::TextWrapped("You cannot use viewport features (pop-out windows) and all ImGui windows are constrained to fit in your ofAppWindow.\nThis \"custom backend feature\" will be quite hard to port and maintain.");
					ImGui::Bullet(); ImGui::TextWrapped("The (native) mouse cursor will always be a simple pointer, unless you use ImGui's software-rendered cursor (and ensure to disable the native oF mouse cursor).");
					ImGui::Bullet(); ImGui::TextWrapped("There's no gamepad support (useful for mouseless gui navigation in some situations, keyboard navigation should still work).\n(it could be implemented by binding an optional ofxAddon)");
					ImGui::Bullet(); ImGui::TextWrapped("There's no touchscreen support.\n(but it's probably easy to implement in EngineOpenFrameworks.cpp)");
					ImGui::Bullet(); ImGui::TextWrapped("Text input and keyboard shortcuts are more limited due to how OF forwards user input.");
					ImGui::Bullet(); ImGui::TextWrapped("(Unexhaustive list, to be continued...)");

					ImGui::Dummy({10,10});
					ImGui::Text("Other hints:");
	#ifdef TARGET_GLFW_WINDOW
					ImGui::Bullet(); ImGui::TextWrapped("Your ofApp uses a GLFW windows, you could switch to the GFW backend.");
	#else
					ImGui::Bullet(); ImGui::TextWrapped("If your ofApp can use GLFW windows, you could switch to the GFW backend.");
	#endif
#else
					ImGui::Bullet(); ImGui::TextWrapped("There's no custom information for your backend.");
#endif // End backend switch

					// Runtime checks (prevent imgui config mistakes)
					ImGui::Dummy({10,10});
					ImGui::SeparatorText("Runtime Checks");
					// Viewports support check
					if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
#if defined(OFXIMGUI_BACKEND_GLFW)
	#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 0 && OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 0
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,128,20,200));
						ImGui::Bullet(); ImGui::TextWrapped("Warning: Viewports are enabled, but not OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP.\nViewport windows will only work in one of your ofApp windows (if using multiple windows).");
						ImGui::PopStyleColor();
	#elif OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 0
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,128,20,200));
						ImGui::Bullet(); ImGui::TextWrapped("Warning: Viewports are enabled, but not OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP.\nViewport windows will only work in one of your ofApp windows (if using multiple windows).");
						ImGui::PopStyleColor();
						if(imguiContexts.size() > 1){
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,20,20,200));
							ImGui::Bullet(); ImGui::TextWrapped("You are using multiple ofAppBaseWindows !");
							ImGui::PopStyleColor();
						}
	#else
						ImGui::Bullet(); ImGui::TextWrapped("You requested to enable viewports which are supported. :) ");
	#endif
#else
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,20,20,200));
						ImGui::Bullet(); ImGui::TextWrapped("You requested to enable viewports but it's not implemented in the backend that you're using (%s).", OFXIMGUI_LOADED_BACKEND);
						ImGui::PopStyleColor();
#endif
//#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 0
//						// Todo: fix this msg, it might be incorrect
//						ImGui::TextWrapped("You are using multiple ofAppBaseWindows together with viewports enabled, which won't work because you requested to use an unmodified imgui_backend_glfw.");
//#endif
					}
					// Slaves initialisation settings limitation
					if(context->slaveCount > 1){
						ImGui::Dummy({10,10});
						ImGui::Bullet(); ImGui::TextWrapped("Notice: Your ofApp creates multiple ofxImGui instances.\nJust note that in such scenarios, only the first call to `gui.setup()` has full control over initialisation settings, the next ones will setup as slaves (if they use the same ofAppBaseWindow), limiting the control over some initialisation settings.");
					}
					// ofApps with multiple ofAppBaseWindows
					if(imguiContexts.size() > 1){
						ImGui::Dummy({10,10});
						ImGui::Bullet(); ImGui::TextWrapped("Notice: Your ofApp runs ofxImGui in multiple ofAppBaseWindows.\nWhile it's not recommended to use a gui in both windows for GPU applications, sometimes you can't do otherwise.");
#if defined(OFXIMGUI_BACKEND_GLFW)
	#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1 || OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
						ImGui::Bullet(); ImGui::TextWrapped("Running ofxImGui in multiple ofWindows is supported in your configuration.");
	#else
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,128,20,200));
						ImGui::Bullet(); ImGui::TextWrapped("Running ofxImGui in multiple ofWindows is not supported in your configuration.");
						ImGui::PopStyleColor();
						ImGui::Bullet(); ImGui::TextWrapped("Enable either `OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP=1` or `OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP=1`.");
	#endif
#elif defined(OFXIMGUI_BACKEND_OPENFRAMEWORKS)
						ImGui::Bullet(); ImGui::TextWrapped("Running ofxImGui in multiple ofWindows is supported in your configuration.");
#else
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,128,20,200));
						ImGui::Bullet(); ImGui::TextWrapped("Support for running ofxImGui in multiple ofWindows is unknown for this backend.");
						ImGui::PopStyleColor();
#endif
					}
					// Input Checks
					ImGui::Dummy({10,10});
#ifdef OFXIMGUI_BACKEND_OPENFRAMEWORKS
					ImGui::Bullet(); ImGui::TextWrapped("Your OF event handling is untouched by ofxImGui.");
#elif defined(OFXIMGUI_BACKEND_GLFW)
	#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 0
					ImGui::Bullet(); ImGui::TextWrapped("Custom event bindings to ImGui IO:\nimgui_impl_glfw callbacks are not used to fetch user input, which might behave a little differently then the native imgui platform backend.");
	#else
					ImGui::Bullet(); ImGui::TextWrapped("ImGui input is parsed from glfw events by the native imgui_impl_glfw callbacks. Your user experience will be at its best !");
	#endif
#endif // End Event handling

					// Event Handling Checks
					ImGui::Dummy({10,10});
#ifdef OFXIMGUI_BACKEND_OPENFRAMEWORKS
					ImGui::Bullet(); ImGui::TextWrapped("Your OF event handling is untouched by ofxImGui.");
#elif defined(OFXIMGUI_BACKEND_GLFW)
	#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 0
					ImGui::Bullet(); ImGui::TextWrapped("Event Propagation: GLFW --> OpenFrameworks --> ofxImGui --> ImGui");
					ImGui::Bullet(); ImGui::TextWrapped("ofxImGui tries to do its best at converting ofEvents to glfwEvents, but this is experimental. ofxImGui might loose some non-crucial UX event data.");
	#else
		#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1 && OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
					ImGui::Bullet(); ImGui::TextWrapped("Event Propagation: GLFW --> ofxImGui --> ImGui --> OpenFrameworks");
					ImGui::Bullet(); ImGui::TextWrapped("While both work together, you could gain some computing power by disabling OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP.");
		#elif OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
					ImGui::Bullet(); ImGui::TextWrapped("Event Propagation: GLFW --> ImGui --> OpenFrameworks");
		#elif OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
					ImGui::Bullet(); ImGui::TextWrapped("Event Propagation: GLFW --> ofxImGui --> ( ImGui + OpenFrameworks )");
		#else
					ImGui::Bullet(); ImGui::TextWrapped("Event Propagation: GLFW --> ImGui --> OpenFrameworks");
		#endif
	#endif
#endif // End Event handling

					ImGui::EndTabItem(); // End doctor tab
				}

				// STATE tab
				if (ImGui::BeginTabItem("State"))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
					ImGui::Dummy({10,10});
					ImGui::TextWrapped("Here is some useful information about this ofxImGui instance's state.");
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
					ImGui::SeparatorText("Gui instance");
					ImGui::Text("Gui instance: %p (ofxImGui::Gui)", (void*)this);
					ImGui::Text("Owns context: %s", isContextOwned?"YES":"NO");
					ImGui::Indent();
					if( isContextOwned ){
						ImGui::TextWrapped("This instance is setup as a MASTER.");
						ImGui::TextWrapped("It has full control over all settings.");
					}
					else {
						ImGui::TextWrapped("This instance is setup as a SLAVE.");
						ImGui::TextWrapped("Some initial setup settings might have been ignored.");
					}
					ImGui::Unindent();

					ImGui::Dummy({10,10});
					ImGui::SeparatorText("Context");
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
					ImGui::TextWrapped("ImGui needs a context instance to work correctly.");
					ImGui::TextWrapped("ofxImGui provides automatic context handling, setting up one context per ofAppWindow.");
					ImGui::TextWrapped("Note that it's generally recommended to use only one GUI window, and that setting up multiple contexts is not encouraged by the ImGui community.");
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
					ImGui::Text("ofxImGuiContext : %p (isSetup=%s)", (void*)context, (bool)context?"1":"0");
					ImGui::Text("ImGui context   : %p", (void*)context->imguiContext);
					ImGui::Text("Bound OF window : %p", (void*)context->ofWindow.get());
					ImGui::Text("Autodraw        : %s", context->autoDraw?"enabled":"disabled");
					ImGui::Text("Slave count     : %u (%s)", context->slaveCount, context->isShared()?"shared mode":"not shared");
					ImGui::Text("Engine          : %p (isSetup=%s)", &context->engine, context->engine.isSetup?"1":"0");

					ImGui::EndTabItem();
				}

				// Backend TAB
				if (ImGui::BeginTabItem("oF Windows"))
				{
					ImGui::Dummy({10,10});
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
					ImGui::TextWrapped("ofxImGui assigns one context per ofAppBaseWindow instance.");
					ImGui::Dummy({10,10});
					ImGui::TextWrapped("This tab gives an overview of all ofxImGuiContexts bound to ofWindows.");
					ImGui::TextWrapped("They can be owned by any ofxImGui::Gui instance.");
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
					ImGui::SeparatorText("Windows");
					if(ImGui::BeginTable("ofxImGui bound OF windows.", 4)){
						ImGui::TableSetupColumn("ofxAppBaseWindow*");
						ImGui::TableSetupColumn("ofxImGuiContext*");
						ImGui::TableSetupColumn("ImGuiContext*");
						ImGui::TableSetupColumn("Slaves");
						ImGui::TableHeadersRow();

						//for(auto* windowEntry = imguiContexts.getFirst(); windowEntry != nullptr; windowEntry=windowEntry->getNext()){
						//for(const std::pair<std::string, int>& windowEntry : imguiContexts){
						for(auto windowEntry = imguiContexts.begin(); windowEntry!=imguiContexts.end(); windowEntry++){
							ImGui::TableNextRow();

							ImGui::TableNextColumn();
							ImGui::Text("%p", (void*)windowEntry->first);
							ImGui::TableNextColumn();
							ImGui::Text("%p", (void*)&windowEntry->second);
							ImGui::TableNextColumn();
							ImGui::Text("%p", (void*)windowEntry->second.imguiContext);
							ImGui::TableNextColumn();
							ImGui::Text("%u", windowEntry->second.slaveCount);
						}

						ImGui::EndTable();
					}

					ImGui::Dummy({10,10});
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
					ImGui::TextWrapped("Slaves count relate to attached gui instances, be they master or slave.");
					ImGui::PopStyleColor();

					ImGui::EndTabItem();
				}

				// Backend TAB
				if (ImGui::BeginTabItem("ofxImGuiBackend"))
				{
					ImGui::Dummy({10,10});
					if(ImGui::TreeNode("What are backends ?")){
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("The ofxImGui engine interfaces ImGui with an ofAppBaseWindow.");
						ImGui::Dummy({10,10});
						ImGui::TextWrapped("There are currently 2 implementations :");
						ImGui::Bullet(); ImGui::TextWrapped("OpenFrameworks : The simplest one which binds ImGui to native OF events in a simple way, ensured to work in any ofApp.");
						ImGui::Bullet(); ImGui::TextWrapped("GLFW : A fully featured ImGui binding for ofGLFWWindow-based-ofApps (mainly MacOsX/Linux/Windows).\nIt binds the native ImGui Glfw backend to your ofWindows.");
						ImGui::Bullet(); ImGui::TextWrapped("In both cases, rendering is done by loading the native ImGui shaders.");
						ImGui::PopStyleColor();
						ImGui::TreePop();
					}

					auto& io = ImGui::GetIO();

					// Backend info
					ImGui::Dummy({10,10});
					ImGui::SeparatorText("Used Backend");
					ImGui::Text("Loaded backend   : %s", OFXIMGUI_LOADED_BACKEND);
					ImGui::Text("Backend instance : %p", (void*)&context->engine);
					ImGui::Text("Backend.isSetup  : %s", context->engine.isSetup?"1":"0");
					ImGui::Text("Renderer         : %s", io.BackendRendererName);
					ImGui::Text("Platform         : %s", io.BackendPlatformName);
					ImGui::Text("ImGui version    : %s (%i)", IMGUI_VERSION, IMGUI_VERSION_NUM);

					// OF environment
					ImGui::Dummy({10,10});
					ImGui::SeparatorText("openFrameworks Environment");
					ImGui::Text("OF version         : %i.%i.%i (%s)", OF_VERSION_MAJOR, OF_VERSION_MINOR, OF_VERSION_PATCH, OF_VERSION_PRE_RELEASE );
					static int minor; static int major;
					major = ofGetGLRenderer()->getGLVersionMajor();
					minor = ofGetGLRenderer()->getGLVersionMinor();
					static bool isProgrammable = ofIsGLProgrammableRenderer();
#if defined(TARGET_OPENGLES) || defined(TARGET_LINUX_ARM) || defined(TARGET_IOS) || defined(TARGET_EMSCRIPTEM) || defined(TARGET_ANDROID)
					ImGui::Text("OFRenderer OpenGL  : %d.%d (%s)", major, minor, isProgrammable?"programmable":"fixed pipeline");
#else
					ImGui::Text("OFRenderer OpenGL  : %d.%d (#version %s) (%s)", major, minor, ofGLSLVersionFromGL(major, minor).c_str(), isProgrammable?"programmable":"fixed pipeline");
#endif
#ifdef TARGET_OPENGLES
					ImGui::Text("OF TARGET_OPENGLES : YES");
#else
					ImGui::Text("OF TARGET_OPENGLES : NO");
#endif
					// OpenGL environment
					ImGui::Dummy({10,10});
					ImGui::SeparatorText("OpenGL Environment");
					ImGui::Text( "Vendor            : %s", glGetString(GL_VENDOR) );
					ImGui::Text( "GPU               : %s", glGetString(GL_RENDERER) );
					ImGui::Text( "OpenGL version    : %s", glGetString(GL_VERSION) ); // Version + eventual vendor info

// Note: Separated versions are only available in OpenGL 3+ (glsl 1.3+) (null on GLSL 1.1 and 1.2 at least) :
#if defined(GL_MAJOR_VERSION) && defined(GL_MINOR_VERSION)
					static int glMajor = 0; if(glMajor==0) glGetIntegerv(GL_MAJOR_VERSION, &glMajor);
					static int glMinor = 0; if(glMinor==0) glGetIntegerv(GL_MINOR_VERSION, &glMinor);
					if(glMajor != 0 || glMinor != 0){
						ImGui::Text( "GLSL version max  : %s (%i.%i) (#version %s)", glGetString(GL_SHADING_LANGUAGE_VERSION), glMajor, glMinor, ofGLSLVersionFromGL(glMajor, glMinor).c_str() );
					}
					else {
						ImGui::Text( "GLSL version max  : %s", glGetString(GL_SHADING_LANGUAGE_VERSION) );
					}
#endif
#ifdef GL_NUM_SHADING_LANGUAGE_VERSIONS
					// List of Supported GL SL versions
					static int numSupportedShadingLanguages = 0; glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &numSupportedShadingLanguages);
					/* If zero, The implementation doesn't support querying profiles. ( probably GL < 4.3) */
					if(numSupportedShadingLanguages>0){
						ImGui::Text("Supported profiles: %i", numSupportedShadingLanguages);
						for(int i = 0; i<numSupportedShadingLanguages; i++){
							ImGui::BulletText("%s", glGetStringi(GL_SHADING_LANGUAGE_VERSION, i));
						}
					}
					else {
						ImGui::Text("Supported profiles: [Unavailable]");
					}
#else
					ImGui::Text("Supported profiles: [Undefined]");
#endif
					ImGui::Text("OpenGL Context    :");
// Missing defines, they are not set in all contexts... this prevents compile issues, ignoring the settings
#ifndef GL_CONTEXT_COMPATIBILITY_PROFILE_BIT
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0
#endif
#ifndef GL_CONTEXT_CORE_PROFILE_BIT
#define GL_CONTEXT_CORE_PROFILE_BIT 0
#endif
#ifndef GL_CONTEXT_FLAG_DEBUG_BIT
#define GL_CONTEXT_FLAG_DEBUG_BIT 0
#endif
#ifndef GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB 0
#endif
#ifndef GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0
#endif
#ifndef GL_CONTEXT_FLAG_NO_ERROR_BIT
#define GL_CONTEXT_FLAG_NO_ERROR_BIT 0
#endif
#ifndef GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0
#endif
#ifdef GL_CONTEXT_PROFILE_MASK
					ImGui::SameLine();
					ImGui::BeginGroup();
					// Core / Compatibbility mode
					static int  glContextProfileMask = 0;
					if(glContextProfileMask == 0)
						glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &glContextProfileMask);
					static bool isCoreProfile          = glContextProfileMask & GL_CONTEXT_CORE_PROFILE_BIT;
					static bool isCompatibilityProfile = glContextProfileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT;
					if(isCoreProfile)          ImGui::BulletText("Core Profile");
					if(isCompatibilityProfile) ImGui::BulletText("Compatibility Profile");
#endif
#ifdef GL_CONTEXT_FLAGS
					// Context flags
					static int glContextFlags = 0;
					if(glContextFlags==0)
						glGetIntegerv(GL_CONTEXT_FLAGS, &glContextFlags);
					static bool isContextForwardCompatible  = glContextFlags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT;
					static bool isContextDebugMode          = glContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT;
					static bool isContextRobustMemoryAxxArb = glContextFlags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB;
					static bool isContextRobustMemoryAxx    = glContextFlags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT;
					static bool isContextReportsErrors      = glContextFlags & GL_CONTEXT_FLAG_NO_ERROR_BIT;
					if(isContextForwardCompatible)   ImGui::BulletText("Forward Compatible Profile");
					if(isContextDebugMode)           ImGui::BulletText("Debug Mode");
					if(isContextRobustMemoryAxxArb)  ImGui::BulletText("Robust memory access on ARB textures");
					if(isContextRobustMemoryAxx)     ImGui::BulletText("Robust memory access");
					if(isContextReportsErrors)       ImGui::BulletText("OpenGL does not report errors");
					ImGui::EndGroup();
#endif
					// GL Extensions
					std::vector<std::string> glExtensions = ofGLSupportedExtensions();
					static int numExtensions = glExtensions.size();
					if(ImGui::TreeNode("GL Extensions", "GL Extensions (%2i):", numExtensions)){
						for(const auto& extension : glExtensions){
							ImGui::BulletText("%s", extension.c_str());
						}
						ImGui::TreePop();
					}
#ifndef OFXIMGUI_RENDERER_GLES
					// Check OpenGL support for GL ES :
					static bool glHasES2Compatibility = std::find(glExtensions.cbegin(), glExtensions.cend(), "GL_ARB_ES2_compatibility") != glExtensions.cend();
					ImGui::BulletText("GL ES 2 compatibile : %s", glHasES2Compatibility?"YES":"NO");
					static bool glHasES3Compatibility = std::find(glExtensions.cbegin(), glExtensions.cend(), "GL_ARB_ES3_compatibility") != glExtensions.cend();
					ImGui::BulletText("GL ES 3 compatibile : %s", glHasES3Compatibility?"YES":"NO");
#endif

					ImGui::Dummy({10,10});
					if( ImGui::CollapsingHeader("Global Backend Details") ){

						// Some IO data
						ImGui::Dummy({10,10});
						ImGui::SeparatorText("Global Internals");
						ImGui::Text("FPS          : %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
						ImGui::Text("Delta time   : %.2f (sec)", io.DeltaTime);
						ImGui::Text("Display size : %.0f x %.0f (px)", io.DisplaySize.x, io.DisplaySize.y);
						ImGui::Text("Display scale: %.3f x %.3f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
						ImGui::Text("Ini filename : %s", io.IniFilename);
						ImGui::Text("Loaded Fonts : %i", io.Fonts->Fonts.size());
						for(auto& font : io.Fonts->Fonts){
							ImGui::BulletText("%s", font->ConfigData->Name);
                            if(font == io.FontDefault){
                                ImGui::SameLine();
                                ImGui::TextDisabled("Default");
                            }
						}
						ImGui::TextWrapped("");

						// Backend Flags
						ImGui::Dummy({10,10});
						ImGui::SeparatorText("Backend Capabilities");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("Backends can support advanced ImGui features.");
						ImGui::PopStyleColor();
						ImGuiBackendFlags backend_flags = io.BackendFlags;
						ImGui::CheckboxFlags("HasGamepad",             &backend_flags, ImGuiBackendFlags_HasGamepad);
						ImGui::CheckboxFlags("HasMouseCursors",        &backend_flags, ImGuiBackendFlags_HasMouseCursors);
						ImGui::CheckboxFlags("HasSetMousePos",         &backend_flags, ImGuiBackendFlags_HasSetMousePos);
						ImGui::CheckboxFlags("PlatformHasViewports",   &backend_flags, ImGuiBackendFlags_PlatformHasViewports);
						ImGui::CheckboxFlags("HasMouseHoveredViewport",&backend_flags, ImGuiBackendFlags_HasMouseHoveredViewport);
						ImGui::CheckboxFlags("RendererHasVtxOffset",   &backend_flags, ImGuiBackendFlags_RendererHasVtxOffset);
						ImGui::CheckboxFlags("RendererHasViewports",   &backend_flags, ImGuiBackendFlags_RendererHasViewports);

						// Parts from imgui_demo.cpp
						ImGui::Dummy({10,10});
						ImGui::SeparatorText("ImGui::io.ConfigFlags");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("These are the most important settings, for more fine-tuning, refer to ImGui docs.");
						ImGui::TextWrapped("You can set them like : ImGui::GetIO().ConfigFlags |= myFlag");
						ImGui::PopStyleColor();
						ImGui::CheckboxFlags("NavEnableKeyboard",    &io.ConfigFlags, ImGuiConfigFlags_NavEnableKeyboard);
						ImGui::CheckboxFlags("NavEnableGamepad",     &io.ConfigFlags, ImGuiConfigFlags_NavEnableGamepad);
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS // soon to be depreciated ! Safe to remove when reade. See io.ConfigNavMoveSetMousePos
						ImGui::CheckboxFlags("NavEnableSetMousePos", &io.ConfigFlags, ImGuiConfigFlags_NavEnableSetMousePos);
#endif
						ImGui::CheckboxFlags("NoMouse",              &io.ConfigFlags, ImGuiConfigFlags_NoMouse);
						ImGui::CheckboxFlags("DockingEnable",        &io.ConfigFlags, ImGuiConfigFlags_DockingEnable);
						ImGui::CheckboxFlags("ViewportsEnable",      &io.ConfigFlags, ImGuiConfigFlags_ViewportsEnable);

						// Widget settings
						ImGui::Dummy({10,10});
						ImGui::SeparatorText("ImGui Widget Settings");
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("You can change these to adjust ImGui input behaviour.");
						ImGui::PopStyleColor();
						ImGui::Checkbox("io.MouseDrawCursor", &io.MouseDrawCursor);
						ImGui::Checkbox("io.ConfigInputTextCursorBlink", &io.ConfigInputTextCursorBlink);
						ImGui::Checkbox("io.ConfigWindowsResizeFromEdges", &io.ConfigWindowsResizeFromEdges);
						ImGui::Checkbox("io.ConfigNavMoveSetMousePos", &io.ConfigNavMoveSetMousePos);

						// User Input
						ImGui::Dummy({10,10});
						ImGui::SeparatorText("ImGui Input");
						ImGui::Text("Keys down  :");
						for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) {
							if(!ImGui::IsKeyDown(key)) continue;
							ImGui::SameLine();
							ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key);
						}
						ImGui::Text("Mouse pos  : (%g, %g)", io.MousePos.x, io.MousePos.y);
						ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
						ImGui::Text("MouseWheel : %f", io.MouseWheel);
						ImGui::Text("Mouse down :");
						for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++){
							if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
						}
					}

					ImGui::Dummy({10,10});
					if( ImGui::CollapsingHeader("Backend Specific Information", ImGuiTreeNodeFlags_DefaultOpen) ){

					ImGui::SeparatorText(OFXIMGUI_LOADED_BACKEND " Backend Information");
#if defined(OFXIMGUI_BACKEND_GLFW)
					ImGui::TextWrapped("GLFW version : %i.%i.%i", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION );

					ImGui::Dummy({10,10});
					ImGui::SeparatorText("GLFW Callbacks");
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_ChildBg)); // Disable hover on treenode
					if(ImGui::TreeNode("Explanations##glfw_callbacks")){
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("GLFW window events can only be bound to one listener; imgui_backend_glfw allows either to bind to GLFW directly or to call its event listeners manually.\n");
						ImGui::TextWrapped("This can be toggled with the OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS macro define. (default = 1)");
						ImGui::Bullet(); ImGui::TextWrapped("0 = ofxImGui listens to ofEvents and forwards them to the ImGui backend (after converting them).\nEvent propagation: GLFW -> OpenFrameworks -> ofxImGui -> ImGui");
						ImGui::Bullet(); ImGui::TextWrapped("1 = Replace GLFW callbacks by those of ImGui or ofxImGui then forward them to OpenFrameworks.\nEvent propagation: GLFW -> (ImGui | ofxImGui) -> OpenFrameworks");
						ImGui::PopStyleColor();
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
					ImGui::TextWrapped("OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS = %i", OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS);
#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 1
					ImGui::Bullet(); ImGui::TextWrapped("You are replacing the OpenFrameworks GLFW callbacks. They are intercepted by either ofxImGui or ImGui, then forwarded to OpenFrameworks.");
#else
					ImGui::Bullet(); ImGui::TextWrapped("The original OpenFrameworks GLFW window callbacks left intact. ofxImGui listens to ofEvents, converts them back to GlfwEvents then forwards them to ImGui.");
#endif

					ImGui::Dummy({10,10});
					ImGui::SeparatorText("MultiContext Support");
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_ChildBg)); // Disable hover on treenode
					if(ImGui::TreeNode("Explanations##multicontext_support")){
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("The native GLFW ImGui backend doesn't support multiple contexts, which is needed for running ofxImGui in multi-window ofApps.\nThere are two compilation options to enable support for it.");
						ImGui::Bullet(); ImGui::TextWrapped("OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP   = 0 | 1 (default: 0)\n  - Asks ofxImGui to intercept GLFW calls rather then ImGui directly.\n  - This enables multi-context support only for ofWindows, but not for viewport (standalone) windows.\n  - As a side effect, this also causes OpenFrameworks' GLFW callbacks to be called a bit earlier too.");
						ImGui::Bullet(); ImGui::TextWrapped("OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP = 0 | 1 (default: 1)\n  - Applies some changes to imgui_impl_glfw to manage a window->context map and set the right context prior to calling API calls.\n  - This enables support for multiple contexts");
						ImGui::Bullet(); ImGui::TextWrapped("None, one or both can be enabled simultanously.");

						ImGui::PopStyleColor();
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
					ImGui::TextWrapped("OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP   = %i", OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP);
					ImGui::TextWrapped("OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP = %i", OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP);

					ImGui::Dummy({10,10});
					ImGui::Bullet();
#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
					ImGui::TextWrapped("You have full multicontext support.");
#elif OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
					ImGui::TextWrapped("You have multicontext support within your OpenFrameworks windows, but not in standalone viewport windows.");
#else
					ImGui::TextWrapped("You have no multicontext support; you can only use ofxImGui within one of your OpenFrameworks applications.");
#endif

					ImGui::Dummy({10,10});
					ImGui::SeparatorText("ofxImGui Multicontext fix");
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_ChildBg)); // Disable hover on treenode
					if(ImGui::TreeNode("Explanations##ofximgui_multicontext_support")){
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("0 = XXX.");
						ImGui::PopStyleColor();
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
	#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
					ImGui::TextWrapped("The openFrameworks listeners (on GLFW Windows) are replaced by those of ofxImGui, which then forwards them to OpenFrameworks and ImGui by adding multi-context support.");

					ImGui::Dummy({10,10});
					ImGui::TextWrapped("ofxImGui engines handling GLFW windows :");
					if(ImGui::BeginTable("Bound engines table", 2)){
						ImGui::TableSetupColumn("GlfwWindow*");
						ImGui::TableSetupColumn("EngineGlfw*");
						ImGui::TableHeadersRow();
						for(auto* engineEntry = context->engine.enginesMap.getFirst(); engineEntry != nullptr; engineEntry=engineEntry->getNext()){
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImGui::Text("%p", engineEntry->key);
							ImGui::TableNextColumn();
							ImGui::Text("%p", engineEntry->data);
						}
						ImGui::EndTable();
					}
	#else
					ImGui::TextWrapped("ofxImGui doesn't handle any GLFW events.\nImGui replaces the OpenFrameworks ones while ensuring to call the OF ones afterwards.");
	#endif

					ImGui::Dummy({10,10});
					ImGui::SeparatorText("GLFW Backend modification");
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_ChildBg)); // Disable hover on treenode
					if(ImGui::TreeNode("Explanations##impl_glfw_multicontext_support")){
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("If there are multiple contexts, the native GLFW backend needs to be able to switch context on GLFW callbacks.");
						ImGui::TextWrapped("For this to work (including viewport support), we use a modified GLFW backend so that it supports multiple contexts with pop-out windows.");
						ImGui::TextWrapped("If you don't need ofxImGui in multiple windows, you may toggle this off by setting the OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP macro define to 0.");
						ImGui::PopStyleColor();
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
	#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
					ImGui::TextWrapped("You are using a modified imgui_backend_glfw so that you can use multiple ofAppBaseWindows together with viewports enabled.");

					ImGui::Dummy({10,10});
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_ChildBg)); // Disable hover on treenode
					if(ImGui::TreeNode("Viewport windows handled by ofxImGui for the native imgui_backend_glfw:")){
						if(ImGui::BeginTable("Bound Engines Table", 2)){
							ImGui::TableSetupColumn("GlfwWindow*");
							ImGui::TableSetupColumn("ImGuiContext*");
							ImGui::TableHeadersRow();
							for(auto* engineEntry = ImGui_ImplGlfw_ScopedContext::Contexts.getFirst(); engineEntry != nullptr; engineEntry=engineEntry->getNext()){
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								ImGui::Text("%p", engineEntry->key);
								ImGui::TableNextColumn();
								ImGui::Text("%p", engineEntry->data);
								if(ImGui::IsItemHovered()){
									ImGui::SameLine();
									ImGui::BeginTooltip();
									ImGui::Text("%s", context->imguiContext==engineEntry->data?"Current context":"Other instance's context");
									ImGui::EndTooltip();
								}
							}
							ImGui::EndTable();
						}
						ImGui::TreePop();
					}
					ImGui::PopStyleColor();
	#else // if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 0
					ImGui::TextWrapped("You are using an unmodified imgui_backend_glfw : you can't use multiple ofAppBaseWindows together with viewports enabled.");
	#endif
					ImGui::Dummy({10,10});
					ImGui::SeparatorText("GLFW event propagation");
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_ChildBg)); // Disable hover on treenode
					if(ImGui::TreeNode("Explanations##glfw_event_propagation")){
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200,200,200,200));
						ImGui::TextWrapped("Depending on how you configure ofxImGui, the event propagation chain is bound differently.\nIn some rare cases, this might interfere with the events you're listening to.");
						ImGui::TreePop();
						ImGui::PopStyleColor();
					}
					ImGui::PopStyleColor();

					ImGui::Dummy({10,10});
					ImGui::TextWrapped("Here's how you are currently bound :");
	#if OFXIMGUI_GLFW_EVENTS_REPLACE_OF_CALLBACKS == 0
					ImGui::Bullet(); ImGui::TextWrapped("GLFW --> OpenFrameworks --> ofxImGui --> ImGui");
					ImGui::Bullet(); ImGui::TextWrapped("ImGui input is bound to ofEvents and directly send to ImGuiIO, bypassing the need for imgui_impl_glfw callbacks (custom backend implementation).\nEvent conversion is needed and might loose some non-crucial UX event data.");
	#else
		#if OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1 && OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
					ImGui::Bullet(); ImGui::TextWrapped("Primary viewports (ofAppBaseWindows) :\nGLFW --> ofxImGui --> ImGui --> OpenFrameworks");
					ImGui::Bullet(); ImGui::TextWrapped("Secondary viewports (pop-out windows):\nGLFW --> ImGui");
		#elif OFXIMGUI_GLFW_FIX_MULTICONTEXT_SECONDARY_VP == 1
					ImGui::Bullet(); ImGui::TextWrapped("Primary viewports (ofAppBaseWindows) :\nGLFW --> ImGui --> OpenFrameworks");
					ImGui::Bullet(); ImGui::TextWrapped("Secondary viewports (pop-out windows):\nGLFW --> ImGui");
		#elif OFXIMGUI_GLFW_FIX_MULTICONTEXT_PRIMARY_VP == 1
					ImGui::Bullet(); ImGui::TextWrapped("Primary viewports (ofAppBaseWindows) :\nGLFW --> ofxImGui --> ( ImGui + OpenFrameworks )");
					ImGui::Bullet(); ImGui::TextWrapped("Secondary viewports (pop-out windows):\nGLFW --> ImGui");
		#else
					ImGui::Bullet(); ImGui::TextWrapped("GLFW --> ImGui --> OpenFrameworks");
		#endif
					ImGui::Bullet(); ImGui::TextWrapped("ImGui input is parsed from glfw events by the native imgui_impl_glfw callbacks.");
	#endif // End Callbacks overview

#endif // Backends switch
					} // Backend Specific Information collapsable

					ImGui::Dummy({10,10});

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			} // End tabs
		}
		// Close window space
		ImGui::End();

#endif // OFXIMGUI_DEBUG
	}

	void Gui::updateDockingVp(){
		static ofRectangle dockingVpCached = ofGetWindowRect();

        // Only when docking is enabled
        if(bool(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)){
            // Note: same as defalut ID returned by ImGui::DockSpaceOverViewport(0, NULL, dockingFlags);
            // A bit complicated to reverse, we have to grab the hardcoded IDs

            // Like ImGui::DockSpaceOverViewport
            char label[32];
            ImFormatString(label, IM_ARRAYSIZE(label), "WindowOverViewport_%08X", ImGui::GetMainViewport()->ID);

            // Get docking host window
            ImGuiWindow* window = ImGui::FindWindowByName(label);
            if(window){
                // Like ImGui::DockSpaceOverViewport
                static const std::string dockSpaceId = "DockSpace";
                ImGuiID dockNodeID = ImGui::GetIDWithSeed(&*dockSpaceId.cbegin(), &*dockSpaceId.cend(), window->ID);

                ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockNodeID);

                // Only use dockspace if currently visible
                if(dockNode && dockNode->IsVisible){
                    ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockNodeID);

                    // Verifies if the central node is empty (visible empty space for oF)
                    if( centralNode && centralNode->IsEmpty() ){
                        ImRect availableSpace = centralNode->Rect();

                        // Detect change ?
                        if(
                            dockingVpCached.x != availableSpace.Min.x ||
                            dockingVpCached.y != availableSpace.Min.y ||
                            dockingVpCached.width != availableSpace.GetWidth() ||
                            dockingVpCached.height != availableSpace.GetHeight()
                        ){
                            // Update viewport
                            dockingVpCached.x = availableSpace.Min.x;
                            dockingVpCached.y = availableSpace.Min.y;
                            dockingVpCached.width = availableSpace.GetWidth();
                            dockingVpCached.height = availableSpace.GetHeight();
                        }

                        dockingViewport = dockingVpCached;

                        // Normalise data according to the different ImGui coordinate spaces
                        // Depending on the viewports flag, the XY is either absolute or relative to the oF window.
                        if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ){
                            dockingViewport.position.x -= ofGetWindowPositionX();
                            dockingViewport.position.y -= ofGetWindowPositionY();
                        }

                        // Success (don't set default below)
                        return;
                    }
                }
            }
        }

		// Set dockingVP to ofWindow when docking unactive
		dockingViewport = ofGetWindowRect();
	}


	ofRectangle Gui::getMainWindowViewportRect(bool returnScreenCoords, bool removeMenuBar, bool removeDockingAreas) const {

		// Handle menubar
		ofRectangle rect = ofGetWindowRect();

		// Subtract menubar
		if(removeMenuBar){
			rect.y += menuHeight;
			rect.height -= menuHeight;
		}

		// When docking is active, it already excludes the menubar
		if(removeDockingAreas && ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable){
			rect = dockingViewport;
		}

		// Make them absolute ?
		if(returnScreenCoords){
			rect.position.x += ofGetWindowPositionX();
			rect.position.y += ofGetWindowPositionY();
		}

		return rect;
	};

	int Gui::getMenuHeight() const {
		return menuHeight;
	}

	ofRectangle Gui::getDockingViewport() const {
		return dockingViewport;
	}

    // Initialise statics
	//LinkedList<ofAppBaseWindow, ofxImGuiContext> Gui::imguiContexts = {};
	std::unordered_map<ofAppBaseWindow*, ofxImGuiContext> Gui::imguiContexts = {};
}

