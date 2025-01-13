# Developpers

This readme contains some useful information for development with or on ofxImGui.

## ofxImGui branches

| Branch   | Description |
| ---------|-------------|
| master   | Up-to-date with the latest oF, quite stable. |
| develop  | Active development branch before releasing to the master |
| OF**     | Master equivalent, blocked at the most recent update compatible with oF 0.**. |

The master branch may not be tested on all platforms. See [Releases](https://github.com/jvcleave/ofxImGui/releases/) for more extensively tested versions.

- - - -

## Coding with ofxImGui

### ImGui coding style
ImGui uses a lot of assertions to warn you about API call mistakes. They will make your app crash when something is bad, providing a textual indication about what could be wrong.
Compile-time asserts will provide some static hints. For more insight, attach your favourite debugger and check the callstack when a runtime assert is triggered.

### ImGui documentation
In ImGui, everything is documented in the source code. Resolve a symbol in your IDE and read the comments around the definition. [A documentation is also on its way, covering some big topics](https://github.com/ocornut/imgui/tree/master/docs).  
One important concept to understand is the powerful ImGui ID Stack, reading [its FAQ primer](https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-about-the-id-stack-system) can prevent a lot of mistakes.

### Native ImGui components
There are plenty of ImGui widgets. Use `example-demo` to discover them, together with the demo source code to check how to use them in your project. There's also a useful [interactive demo](https://pthom.github.io/imgui_manual_online/) that runs in your browser.

### ImGui extensions
ImGui has a huge community and is growing fast. There are a lot of plugins available (gui widgets). You can easily add some to your project, they should normally load into ofxImGui. See `example-advanced` and `example-imguizmo`.

### ofxImGui compilation flags
- `#define OFXIMGUI_DEBUG` : Set to print some information on how ofxImGui runs. Also gives some hints if you're implementing it wrong.

- - - -

## Updating ImGui

DearImGui has a fast update scheme and changes frequently. ofxImGui rather tries to follow the slower openFrameworks update scheme.  
Here are some instructions for updating DearImGui within ofxImGui:
- Download the updated files and put them in `ofxImGui/libs`: (choose one)
    1. Manually:
      - Go to the [official ImGui](https://github.com/ocornut/imgui/tree/docking/) repo and get the `glfw` and `opengl 2+3` header and source files from and into the `backends` folders. Beware that we're using the `docking` branch of imgui, until it gets merged in the master.
      - Similarly, put the `*.h` and `*.cpp` files from the `imgui root` go into `ofxImGui/libs/imgui/src`.
    2. Automatically:
      - Edit the commit you want in `libs/UpdateImGui.sh`.
      - Run: `cd ofxImGui/libs && ./UpdateImGui.sh`.
- Manually compare `ofxImGui/src/imconfig.h` with the new `ofxImGui/libs/imgui/src/imconfig.h`, merge new changes if needed, then delete `ofxImGui/libs/imgui/src/imconfig.h`.
- Apply platform specific hacks listed below.
- After updating: Check ofxImGui's source code for detecting obsolete API functions usage using `IMGUI_DISABLE_OBSOLETE_FUNCTIONS`.

### Applying platform specific hacks
After updating imgui, it's required to do some changes.

- 1. *For multiwindow support with viewports enabled* : Enable support for multiple contexts in the glfw backend.  
In `imgui_impl_gflw.cpp` : Add context registering, switching and restoring.  
````cpp
// Below #include "imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw_context_support.h" // CUSTOM ADDED LINE FOR OFXIMGUI

// Within all GLFW callback functions taking `window` as 1st arg, add this to the begin :
ImGui_ImplGlfw_ScopedContext sc(window); // CUSTOM ADDED LINE FOR OFXIMGUI

// At the end of ImGui_ImplGlfw_CreateWindow :
ImGui_ImplGlfw_RegisterWindowContext(vd->Window, ImGui::GetCurrentContext()); // CUSTOM ADDED LINE FOR OFXIMGUI

// In ImGui_ImplGlfw_DestroyWindow, right before `if (vd->WindowOwned)` :
ImGui_ImplGlfw_RemoveWindowContext(vd->Window); // CUSTOM ADDED LINE FOR OFXIMGUI
````  
Switching contexts is possible since these issues, but still we are in "isolation mode". Right now, ImGui doesn't offer a way to register additional host viewports from a native system window, but they plan to support it in the future. So now we create an ImGui context per ofAppWindow.  
ImGui doesn't plan to support this but the creator himself [started a template for this](https://github.com/ocornut/imgui_club/blob/main/imgui_multicontext_compositor/), ideally we [would move to support that](https://github.com/jvcleave/ofxImGui/issues/130).  
Related issues:
 - [Multiple GLFW contexts : glfw event binding](https://github.com/ocornut/imgui/issues/5439)
 - [Does GLFW implementation only handle one GLFWindow?](https://discourse.dearimgui.org/t/does-glfw-implementation-only-handle-one-glfwindow/305)
 - [Add support for multiple GLFW contexts](https://github.com/ocornut/imgui/pull/3934)
 - [Multiple host viewports](https://github.com/ocornut/imgui/issues/3012)
 - [Correct use of ImGui_ImplGlfw_NewFrame with multiple ImGui contexts, and g_Time](https://github.com/ocornut/imgui/issues/2526)
 - [Nesting multiple imgui contexts (glfw+opengl3)](https://github.com/ocornut/imgui/issues/2004)

- 2. *Add GL ES 1 support so that it compiles on Rpis :*  in `imgui_impl_opengl2.cpp`
````cpp
// --- CUSTOM MODIFICATION
// Rpi dirty fix : Add support for GLES 1.1, used by the imgui fixed pipeline.
#elif defined(TARGET_RASPBERRY_PI) && defined(TARGET_OPENGLES) // && defined(IMGUI_IMPL_OPENGL_ES1)
#include "gles1CompatibilityHacks.h"
// --- END CUSTOM MODIFICATION
````
- 3. *GLFW compatibility* :  
  Between oF 0.11.0 and oF 0.12.1, GLFW versions have been changing a lot. Refer to [./Configure.md#Glfw-version].
  *Note:* **oF 0.11.0 uses GLFW pre-3.3.0**; this causes the imgui glfw backend to use an unavailable function. Until oF's GLFW library gets updated (0.12.0), `imgui_impl_glfw.cpp` will need to be modified in order to work with ofxImGui.
  - The raspberry pi might have an old GLFW implementation, please add these lines in `imgui_impl_glfw.cpp` to ensure cross platform compatibility.  
  ````cpp
// Custom modification : Add support for older GLFW versions (<3.2) (on Rpi Stretch for example)
#include "glfwCompatibilityHacks.h"
  ````
  - [Change `3300` to `3310`](https://github.com/ocornut/imgui/blob/dd4ca70b0d612038edadcf37bf601c0f21206d28/backends/imgui_impl_glfw.cpp#L62). This change disables some optional imgui features, related to viewport behaviour, and available mouse cursors.  

- - - -

# ofxImGui integration within ofxAddons
For ofxAddon developers, we provide 2 ways of integrating ofxImGui within your addon.

#### 1. Provide custom ofxImGui widgets and methods
The best way for supporting ofxImGui within your addon is to write custom widgets that end-users can call within their own ofApp's GUI layout.   
A fully integrated example can be found in [ofxBlend2D](https://github.com/Daandelange/ofxBlend2D/blob/263f1910bd7a269a1d4dcd001360dfd8520442bf/src/ofxBlend2D.h).

_Note to addon maintainers:_ We encourage addon maintainers to accept community pull requests for ofxImGui widgets. We are aware that this creates a small maintenance overhaul but your ofxAddon is really the easiest place to provide support for ofxImGui features that control it. The extra code is always disabled by default and enabled on a per project opt-in basis. And it will help other ofxImGui users that use your addon.

In your ofxAddon:  
- Add macro conditions to your addon using `ofxAddons_ENABLE_IMGUI` to enable ofxImGui integration. Please use this exact name so that a project can turn on ImGui features for all addons at once. [Sample](https://github.com/Daandelange/ofxBlend2D/blob/263f1910bd7a269a1d4dcd001360dfd8520442bf/src/ofxBlend2D.h#L36-L41).
- Add `#include "ofxImGui.h"` where you use ImGui code. [Sample](https://github.com/Daandelange/ofxBlend2D/blob/263f1910bd7a269a1d4dcd001360dfd8520442bf/src/ofxBlend2D.cpp#L10-L13).
- Add `myAddonInstance::drawImGui(){}` to your methods. [Sample](https://github.com/Daandelange/ofxBlend2D/blob/263f1910bd7a269a1d4dcd001360dfd8520442bf/src/ofxBlend2D.h#L99-L101).
- Add `ImGuiEx::MyAddonGuiItem(const char* label, MyOfxAddonVarType&)`. [Sample](https://github.com/Daandelange/ofxBlend2D/blob/263f1910bd7a269a1d4dcd001360dfd8520442bf/src/ofxBlend2D.h#L171-L193).

In an ofApp project :
- Add `ofxAddons_ENABLE_IMGUI` to your project defines.
- Use `MyAddonInstance.drawImGui();` on your addon instance to draw a full GUI.
- Use `ImGuiEx::MyAddonGuiItem("MyAddonItem", &myAddonVar);` to draw a specific widget.

#### 2. Use ofxImGui within your addon
If your plugin needs to setup a GUI context and draw GUI widgets, your API calls might conflict with the ones from other addons and/or the ofxApp code. For example, what happens when `gui.setup()` is called twice ? When `gui.begin()` is called twice per frame ? ofxImGui provides a solution for this, please refer to `example-sharedcontext` to share a gui instance with other unknown usercode using a master/slave setup system.

While supported, this method is not recommended. If possible, a better scenario would be to let the final ofApp setup the GUI without your ofxAddon submitting any ofxImGui calls directly. Please refer to method 1 above for directions.

- - - -

# ofxAddons with custom ofxImGui widgets.

_A note about third party ofxAddons that provide ofxImGui widgets:_  
Some ofxAddon authors provide custom ofxImGui widgets, facilitating integration with ImGui. These are optional but can save you a lot of time.  

 - Sometimes, the widgets are supported by the addon maintainer.
 - Other ofxAddons have a community-managed widgets. The author is accepting (well formatted) pull requests but not actively maintaining the ofxImGui part.  
   For such repos : **Please fill any ofxImGui related issue in the ofxImGui repo.**

#### How to enable custom ofxImGui widgets from third party ofxAddons ?

There's a global compilation flag (`ofxAddons_ENABLE_IMGUI`) that has to be enabled in your ofApp project's compilation settings.  
When an ofApp project enables this setting, all supported addons will compile with ofxImGui support.  
Without this flag, all ofxImGui related code stays off as well as their respective dependencies.  
Once enabled, you can use the widgets that your included ofxAddons provide.  

Depending on your IDE, here are some directions:
 - Makefiles / VSCode / VSCodium : In `config.make` : `PROJECT_DEFINES = ofxAddons_ENABLE_IMGUI`.
 - XCode : In your project's "Build Settings" : Add in "Other C Flags" : `-DofxAddons_ENABLE_IMGUI`. 
 - Qt-Creator : In your `.qbs` : `of.defines: ['ofxAddons_ENABLE_IMGUI']`.
 - Other IDE : Try to find a "project defines" section in your project settings, or a "c / c++ flags" section and add `ofxAddons_ENABLE_IMGUI` to it.

- - - -

# ofxImGui implementation
This section aims to provide some development choices that have been made for the ofxImGui implementation.

ImGui mainly needs a backend to handle all platform specificities, and there are 2 options for us :
 - Write a **custom backend**: In its minimal version, this works for a simple GUI, but is very tedious to implement and maintain for supporting all advanced imgui features such as docking, viewports, etc. This is how @Jvcleave started this addon such as in the `legacy` branch.
 - Bind to the underlying GLFW window manager. As OF is already bound to this, we replace the 
ofxImGui mainly uses the 2nd and newer method, but the legacy method is still an alternative so all the code is kept.

- - - -

# Contributions

If you have some time to improve ofxImGui, please do and submit a pull request. There are things to be improved for any programer skill level. This list is non-exhaustive.  
Please keep in mind that our philosophy is to stick with the slower oF release cycle rather then the fast Dear Imgui one.

#### Future directions and contribution suggestions:
 - There's some work needed on to make the Helpers more useful and to document their features. The ofParameter bindings could be more deeply integrated (ranged values...) and flexible (ex: `ofParameter<int>` could both be mapped to `SliderInt()`, `DragInt()`, `InputInt`). Improving the Helpers example could help. Also, there's been discussions about a layout engine to position windows next to eachother. Other ideas include saving more states, such as collapsed sections.
 - There is also some work to be done about event handling/bubbling. Currently, mouse and keyboard interactions are both send to oF and ofxImGui. In oF-event-space, how can one know if ImGui is using the mouse/keyboard data or not ? (`example-advanced` could be extended with some demo code on this subject, or even some helper functions to facilitate this).  
- RaspberryPi support will probably [evolve soon](https://github.com/openframeworks/openFrameworks/issues/6713). If using GLFW, imgui support will be good. But it'be great to be able ofxImGui from the console (without X11 desktop environment = more CPU & RAM available) will be a great addition. If oF 0.12 gets, a custom backend will need to be written, maybe letting the user choose between `1` a simpler console-compatible backend or `2` a GLFW-bound fully-featured backend.
- Testing different platforms, specially rpi and emscripten.
- Improving the documentation and the examples.
- Work on combined use of OpenFrameworks input listeners and imgui input listeners. Can there be some options like event bubbling (interception, propagation, etc. ) ?
- Also checkout the [currently open issues](https://github.com/jvcleave/ofxImGui/issues).
