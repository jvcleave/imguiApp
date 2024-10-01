// This file is here to bring support for obj-c ARC which was introduced in of_v0.12.0.
// ImGui doesn't support compiling with ARC features and OF enabled it by default now.
// It also fixes prior OF versions when eventually compiled with ARC support enabled.
//
// We need to tell the ARC meccanism either to `(__bridge void*)` or `CFBridgingRetain(...)`
// ofAppGLFWWindow.cpp uses `__bridge` so I blindly choose that solution. Maybe it's wrong.
// See also issue #134 : https://github.com/jvcleave/ofxImGui/issues/134

#pragma once

// Hacky but simple :)
// Injects ARC support by defining glfwGetCocoaWindow something else
#if defined(__OBJC__) && __has_feature(objc_arc)
#   define glfwGetCocoaWindow (__bridge void*) glfwGetCocoaWindow
//     Alternatives
//#    define glfwGetCocoaWindow(X) (__bridge void*) glfwGetCocoaWindow(X)
#else
#   undef glfwGetCocoaWindow
//     Alternatives
//#    define glfwGetCocoaWindow(X) glfwGetCocoaWindow(X)
#endif
