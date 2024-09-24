
# Changelog

_Note: This document is focused on ofxImGui. For detailed ImGui API changes and new features, please refer to the [ImGui changelog](https://github.com/ocornut/imgui/blob/v1.91.0-docking/docs/CHANGELOG.txt)._  
_Tip: For compile-time depreciation hinting on your code, uncomment `IMGUI_DISABLE_OBSOLETE_FUNCTIONS` in `imconfig.h`. If you do this regurarly, it's quite easy to keep up with the occasional ImGui breaking API changes._  

- - - -

# ofxImGui 1.91.0
- Lots of new documentation.
- Fix issue where default font was overwritten loading a first custom font.
- Newly introduced ImGui Features : multi-select, box-select, item flags, links.

- - - -

# ofxImGui 1.82 (develop)
This is a major update for ofxImGui. Prior to this version, DearImGui was bound to openFrameworks using a "custom backend code".
DearImGui has grown considerably over time, but some of the new (optional but nice) features require an advanced backend code, which is now provided by ImGui. For these features to be enabled, DearImGui now needs to be bound to the OS' windowing framework (GLFW). Luckily this is currently the same as most OpenFrameworks projects, and other popular windowing frameworks are also covered in case OpenFrameworks moves away from GLFW.

### Breaking changes
- **GLFW** : If your project uses a windowing system other then GLFW, this newer ofxImGui will not work correctly. This is mainly the case for Emscriptem projects, iOS and some Rpi environments. Consider using an [older ofxImGui release](https://github.com/jvcleave/ofxImGui/tree/330e1425a88de7babd53ceb2fb93f8109b61724c), or try [this untested commit](https://github.com/jvcleave/ofxImGui/tree/05ab1311511523c63c6f22d38ee015942b9ea557) together with `OFXIMGUI_ENABLE_OF_BINDINGS` if you need ImGui 1.79 features.
- **Autodraw** : This setting was already around but worked differently, which lead to confusions. It used to call `gui.draw()` when calling `gui.end()`. Now, when autodraw is enabled, ofxImGui uses a callback on `ofEvents::afterDraw()` to render the gui, drawing the gui always on top of your ofApp. This might affect the render order rendered in your pipeline.
- **imgui_stdlib.h/cpp** : If you were using them in your project, please remove the files, they are now included with ofxImGui.
- **ImHelpers.cpp** : If you are using them, they are not included by default via `#import "ofxImGui.h"`, you now need to include it manually (`#import "ImHelpers.h"`).
- **ImGui API changes** : Refer to the ImGui changelog.

As an upgrade assistant, you might want to enable `OFXIMGUI_DEBUG` while transitioning, it will provide some general warnings on mistakes.

For more exhaustive information on changes, refer to [Daandelange/ofxImGui#1](https://github.com/Daandelange/ofxImGui/issues/1) and [jvcleave/ofxImGui#107](https://github.com/jvcleave/ofxImGui/issues/107).

### New Features
- Load custom fonts
- Set `ImGuiConfigFlags` when setting up ofxImGui. _There were some workarounds for this, but not it's possible natively._
- Share ofxImGui instances within the same ofApp. _By enabling a shared mode, any ofxImGui instance will act as a singleton so multiple instances can seamlessly let eachother know when to render._
- Multi-window support **beta**. _Any feedback is appreciated_.
- Improved example projects.
- Helpers :
  - AddGroup() can now receive treenode flags.
  - New: ofColor and std::string parameters
- Main features introduced by the DearImGui update:
  - Navigation support : Control the GUI with a gamepad or a keyboard !
  - Viewports : The Gui can seamlessly move out of your ofApp !
  - Docking : Awesome layout engine to make your own workspaces !
  - Tables : Display tables !

For more details on how to use them, please refer to the repo's readme and the examples' code.

- - - -