// This file is here to bring support for obj-c ARC which was introduced in of_v0.12.0.
// ImGui doesn't support compiling with ARC features and OF enabled it by default now.
// It also fixes prior OF versions when eventually compiled with ARC support enabled.
//
// We need to tell the ARC meccanism either to `(__bridge void*)` or `CFBridgingRetain(...)`
// ofAppGLFWWindow.cpp uses `__bridge` so I blindly choose that solution. Maybe it's wrong.
// See also issue #134 : https://github.com/jvcleave/ofxImGui/issues/134

#pragma once

// Required for non-Clang compiler support
// Thanks to @GitBruno : https://github.com/PlaymodesStudio/ofxImGuiSimple/issues/3#issuecomment-2586343914
#ifndef __has_feature         // Optional of course
  #define __has_feature(x) 0  // Compatibility with non-clang compilers
#endif
#ifndef __has_extension
  #define __has_extension __has_feature // Compatibility with pre-3.0 compilers
#endif

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
