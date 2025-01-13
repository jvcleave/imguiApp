# ofxImGui

ofxAddon that allows you to use [ImGui](https://github.com/ocornut/imgui) in [openFrameworks](https://github.com/openframeworks/openFrameworks).

### Immediate Mode Gui
Unlike most C++ gui libraries, ImGui uses the [immediate mode](https://en.wikipedia.org/wiki/Immediate_mode_(computer_graphics)) paradigm rather then being OOP (like ofxGui). In other words, every frame you rebuild the whole GUI, executing only the necessary parts, and ImGui is surprisingly fast at that ! This makes the Gui api closer to the OpenGL api as to the inner oF logic (`update()`, `draw()`, `begin()`, `end()`, `push()`, `pop()`, etc.), which makes it very smooth to integrate.  
_(Note: Internally, ImGui is not "ramming" on the CPU as most immediate-mode GUIs do)._

### ImGui Widgets
ImGui provides a lot of gui widgets/components and there's a big community with lots extensions to choose from : you will likely find any widget. In its most basic form, you get a panel with a few control widgets that control your variables, like ofxGui; but ImGui also lets you create a full application GUI layout : a menu, multiple panels, tabs and windows that you can dynamically re-arrange within your ofAppWindow or even popped-out of it. The interactions are very smooth and robust.  
A community member made [this interactive manual](https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html) for the web browser, where you can see what ImGui looks like and the code to reproduce it.  

### ImGui API
ImGui's API is also quite nice : with very few code, you obtain a functional basic GUI component bound to your variable. You can then add arguments or extra api calls to fine-tune it (appearance, layout, interactions, feedback, etc). There's also an internal API (`imgui_internal.h`) which provides access to even more functionality and lets you build your own gui components!  

![Screenshot](images/ofxImGui.gif)

- - - -

### Supported Platforms
ofxImGui should run on the [latest openFrameworks release and it's OS/IDE requirements](https://openframeworks.cc/download/).  
These are typically:

 - MacOs (Xcode + QtCreator + Makefiles)
 - Windows (Visual + VSCode)
 - Raspberry Pi (Makefiles)
 - Linux Desktop (QtCreator + Makefiles)

 For more precise platform compatibility information, please refer to [PlatformSupport.md](./PlatformSupport.md).

- - - -

## Installation

````bash
cd /path/to/of/addons && git clone -b develop https://github.com/jvcleave/ofxImGui.git
````

#### Optional - Upgrade GLFW
You can configure oF to use GLFW 3.4 and ImGui will have an even more polished interface. See [Developers.md](./Developers.md#Improve-ofxImGui-s-backend-bindings).  

#### Keeping ofxImGui up-to-date
To update ofxImGui within your openframeworks installation:

- `cd /path/to/ofxImGui && git pull`
- (*optional but recommended*) After updating: If this ofxImGui version comes with a new ImGui version, add `#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS` in your project defines to make sure you are not using soon-to-be-obsoleted symbols and update them if needed, the compilation errors will guide you trough the changes to make. Specially if you use native ImGui code (as opposed to using the ofxImGui helpers) : if you don't do this regularly, you will have a harder time making your old ImGui code compatible with the newer versions. ImGui has a faster update scheme than OF and we try to update ofxImGui as frequently as possible.

If you need a particular ImGui version, you can stick at the corresponding ofxImGui version, or downgrade it within that latest ofxImGui by grabbing the `imgui` folder from an older ofxImGui version.

- - - -

## Optional - Configure ofxImGui

#### Custom project compilation flags
We try to provide the best default settings for [each platform](./PlatformSupport.md), so you should be good to go.  
If your project is using a special setup, you might need to manually set some configuration options, which are explained in [Configure.md](./Configure.md).

If your project is using **ofxAddons that support ofxImGui**, add the `ofxAddons_ENABLE_IMGUI` define to your project. More info in [Developers.md](./Developers.md#ofxImGui-integration-within-ofxAddons).

#### Debug compilation flags
While interfacing your ofApp with ofxImGui, a good practise is to enable `OFXIMGUI_DEBUG` together with `ofSetLogLevel(OF_LOG_VERBOSE)`, it provides some general warnings on mistakes and logs some important setup steps.

- - - -

## Usage / Code Samples

### Setup
- Add a gui instance to your ofApp : `ofxImGui::Gui gui;`
- Cal `mygui.setup()`, you can pass a few arguments to setup ofxImGui to your needs :  
- `SetupState Gui::setup(BaseTheme* theme_, bool autoDraw_, ImGuiConfigFlags customFlags_, bool _restoreGuiState, bool _showImGuiMouseCursor )`
  *Arguments:**  
   - **theme** : `nullptr` : use default theme (default) / `BaseTheme*` : a pointer to your theme instance.  
   - **autoDraw** : `true` : sets up a listener on `ofApp::afterDraw()` (default) / `false` : allows more precise control over the oF rendering pipeline, you have to call `myGui.draw()` manually )
   - **customFlags** : `ImGuiConfigFlags` : set custom ImGui config flags (default: `ImGuiConfigFlags_None`).
   - **restoreGuiState** : `true` : enabled / `false` : disabled (default).  
   Helper for enabling ImGui's layout saving/restoring feature. Creates `imgui.ini` next to your app binary to save some GUI parameters.
   - **showImGuiMouseCursor** : `true` : use the imgui mouse cursor / `false` : use system mouse cursor (default).
  **Return value:**  
  The setup function returns an `ofxImGui::SetupState` indicating if the setup failed succeeded as a slave or master.  
   - `if( gui->setup(...) & ofxImGui::SetupState::Success ) // I'm either a slave or a master`   
   - `if( gui->setup(...) & ofxImGui::SetupState::Slave   ) // Setup as slave : some requested parameters might have been ignored and there's other code using ofxImGui.`
   - `if( gui->setup(...) & ofxImGui::SetupState::Master  ) // Setup as master : You were the first to create a GUI instance in this ofApp.`   
   - `if(  gui->setup(...) ) // Same as success`  
   - `if( !gui->setup(...) ) // There was an error`

ofxImGui implements DearImGui in such a way that each oF window gets its own imgui context, seamlessly shared between any oF window context. You can also load multiple instances of it in the same ofApp (to use the addon within multiple addons). This feature (shared mode) is automatically enabled when a second `ofxImGui::Gui` instance is created within the same `ofApp` application's platform window context. See example-sharedContext for more info.  
_Note:_ Only the fist call to `gui.setup()` has full control over the settings (master); the next ones setup as slaves.  
_Note:_ Any call to `setup()` has to be done from a valid ofWindow, `ofGetWindowPtr()` is used internally to associate the gui context to that exact ofWindow. Therefore, **you cannot setup ofxImGui before OpenFrameworks' setup() calls**.  

**Advanced setup** : ImGui config flags & ImGui::GetIO()

ofxImGui provides a simple way to interface ImGui, but it's a huge library providing lots of different options to suit your precise needs.  
Most of these advanced options are explained in the `imgui_demo.cpp` source code. Also checkout `example-dockingandviewports` and `example-advanced`.  
Some options are: Docking, viewports, custom fonts, and other configuration flags.

### Draw your GUI
Once the setup is done, there are basically 2 (combinable) ways to use ofxImGui :
  - Via the helpers : The simplest one to get you going. You feed ofxImGui with your `ofParameter` instances and you're almost done. This is also a more robust way too as the ofxImGui code will provide automatic compatibility with future versions. The downside is that you have less control over how the GUI looks and you might want to custimise it further.
  - Via the native ImGui API : For more fine-tuning and customisation, but takes a little more time to write. This provides full access to any of the many ImGui features.  
    _For more experienced ImGui users: You can even write "ImGui extensions" which have access to an additional ImGuiInternal API._

#### Draw a window (optional)
If you don't call any window, ImGui will automatically create a dummy window named "Debug", so it's handy to create them with a name.  
You can create as many windows as you want to keep your GUI organised.
- Create a window : `ImGui::Begin("MyWindow"); /* Draw widgets here... */ ImGui::End();`
- Important: Submit ImGui widgets only when window content is visible : `if(ImGui::Begin("MyWindow")){ /* Draw widgets here... */ } ImGui::End();`  
  (_Anything sumbitted outside will end up in the "Debug" window_)
- You can resume drawing to a window : `ImGui::Begin("MyWindow"); /* Start fill MyWindow */ ImGui::End(); ImGui::Begin("OtherWindow"); /* Fill OtherWindow */ ImGui::End(); ImGui::Begin("MyWindow"); /* Append to MyWindow */ ImGui::End();`

#### Draw Gui Components
In your ofApp::draw(), call `ofAppGui.begin();` and `ofAppGui.end();`. In-between you can submit widgets.

**Important**: Each interactive imgui widget needs to have a unique ID within the ImGui ID stack. This can be a `char*` or a `void*` that has a static memory address that won't move over time.  
For example, using `std::list` is fine: you can use the item as a pointer, but not for `std::vector` as items can be reallocated on the fly.

Some simple components:
- `ImGui::Text("Some static text");`
- `ImGui::Text("Some dynamic text: %s", myString.c_str() ); // std::string ofApp::myString;`
- `ImGui::Checkbox("A checkbox", &myBool); // bool ofApp::myBool;`
- `ImGui::DragInt("An integer", &myInt); // int ofApp::myInt;`
- `if(ImGui::Button("A Button")) myAction();` // void ofApp::myAction();

Checkout the imgui demo window to discover all available widgets !

#### Customise components
There are many ways to change the default widgets, be it by passing extra optional flags or using more API calls. If you're familiar OpenGL API, you won't have a hard time learning the ImGui API.  
Again, have a look at the well documented ImGui Demo code to discover more possibilities. Here are some simple examples.
- `ImGui::BeginDisabled(); /* Some disabled widgets here */ ImGui::EndDisabled();`
- `PushItemWidth(50); /* Some compact widgets here */ PopItemWidth();`
- `ImGui::Text("Push here -->"); ImGui::SameLine(); ImGui::Button("<-- Text there");`
- `ImGui::Indent(); /* Some indented widgets here */ ImGui::Unindent();`
- `ImGui::PushStyleColor(ImGuiCol_Button,{1,0,0,1}); /* Some button here */ ImGui::PopStyleColor();`


#### ofxImGuiHelpers
The helpers are a collection of glue samples for integrating OpenFrameworks objects into ImGui.
Amongst others, it makes porting ofxGui apps to ofxImGui easier as it implements a compatibility layer for ofParameter.  
To use the helpers, you need this : `#include "ImHelpers.h"`.  

For more code samples, check out the examples (below).

- - - -

## Examples

There are several example projects, covering from the simplest use case to more advanced ones : [Examples.md](./Examples.md).

- - - -

## Developer info

Useful developer info and how to get familiar with DearImGui : [Developers.md](./Developers.md).  
Also: information for using ofxImGui within ofxAddons and for ofxImGui developers and contributors.

- - - -

## Community

### ImGui extensions
- Most imgui extensions should be useable by simply including them into your project. Here's [a list of them](https://github.com/ocornut/imgui/wiki/Useful-Extensions) but there are many more around [on github](https://github.com/search?q=+topic%3Aimgui&type=repositories) !

#### ofxImGui extensions
- [moebiussurfing/ofxSurfingImGui](https://github.com/moebiussurfing/ofxSurfingImGui/): An extension to this addon which facilitates advanced integration with ofApps while also making the UI more user-friendly. It embeds a presets system, and embeds many 3rd party ui widgets.
- [ofxImGuizmo](https://github.com/nariakiiwatani/ofxImGuizmo) The ImGuizmo extension shipped as an ofxAddon, for displaying a 3d point handle.

#### ofxAddons providing ofxImGui integration
- [ofxBlend2D](https://github.com/Daandelange/ofxBlend2D)
- [How to add support for my addon](./Developers.md#ofxImGui-integration-within-ofxAddons) ?

- - - -

## Credits

ofxImGui was initiated in 2015 by @jvcleave. Today, he's maintainging it together with @prisonerjohn
 and @daandelange, with the help of [many contributors](https://github.com/jvcleave/ofxImGui/graphs/contributors).

## License

- [ofxImGui is MIT licensed](./License.md).
- [DearImGui is MIT licensed](./libs/imgui/src/LICENSE.txt).
