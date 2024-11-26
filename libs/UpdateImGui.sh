#!/bin/sh
#!/usr/bin/gsed -f

# Run this script to easily update ImGui
# Note: Only tested on Macos, should run on Linux.
# Copyright 2023 Daan de Lange

# ------------------
# CONFIG
# Sha of latest version release (in docking branch!)
# How to find it : Omar publishes a version commit, for example "Version 1.89.3" which is the non-docking version
# Right after that, in the docking branch, there's a commit "Merge branch 'master' into docking ".
# That's the one you wish. The docs/CHANGELOG.txt should NOT state "In Progress" (ie: "VERSION 1.89.3 (In Progress)"")
# Update 2024 : Docking branch versions are now tagged, just grab the commit you wish from https://github.com/ocornut/imgui/tags

VERSION_1_89_2=d822c65 # 1.89.2
VERSION_1_89_3=1921967 # 1.89.3
VERSION_1_89_4=9e30fb0 # 1.89.4
VERSION_1_89_5_WIP=6b3435a # 1.89.5 WIP - Fixes 2ndary viewport modifier keys including Copy&Paste (see commit 91577c7f51)
VERSION_1_89_8=f8704cd0 # 1.89.8
VERSION_1_89_9=1d8e48c1 # 1.89.9
VERSION_1_90_0=ce0d0ac8 # 1.90.0
VERSION_1_90_1=2dc85e6e # 1.90.1
VERSION_1_90_9=3369cbd2 # 1.90.9
VERSION_1_91_0=139e99ca # 1.91.0
VERSION_1_91_5=368123a # 1.91.5
VERSION_SHA=$VERSION_1_91_5
# ------------------

echo "\nHello,"
echo "This script automatically updates ImGui within ofxImGui from a manually selected (fixed) version."
echo "Using commit $VERSION_SHA. [edit this script to change the version]"

echo "\nFinding ImGui..."

# Clone fresh repo
if [[ ! -d ./imgui_git ]]; then
	echo "Cloning ocornut/imgui..."
	git clone https://github.com/ocornut/imgui.git ./imgui_git -b docking
else
	echo "Found ImGui repo in /libs/imgui_git"
fi;

# Pick the right version commit
echo "Checking out latest commit"
if [[ -d ./imgui_git ]]; then
	cd ./imgui_git
	git fetch
	#git checkout -b docking
	# Alternative restoring all missing files AND reverings any change
	git reset --hard $VERSION_SHA --

	cd ../
else
	echo "Problem finding the imgui directory, exiting now."
	exit 1
fi;

# Copy the files we need
echo "\nCopying files..."
# Backend
echo "Copying backend files..."
mkdir -p ./imgui_git/backends
cp ./imgui_git/backends/imgui_impl_glfw.cpp         ./imgui/backends/imgui_impl_glfw.cpp
cp ./imgui_git/backends/imgui_impl_glfw.h           ./imgui/backends/imgui_impl_glfw.h
cp ./imgui_git/backends/imgui_impl_opengl2.cpp      ./imgui/backends/imgui_impl_opengl2.cpp
cp ./imgui_git/backends/imgui_impl_opengl2.h        ./imgui/backends/imgui_impl_opengl2.h
cp ./imgui_git/backends/imgui_impl_opengl3.cpp      ./imgui/backends/imgui_impl_opengl3.cpp
cp ./imgui_git/backends/imgui_impl_opengl3.h        ./imgui/backends/imgui_impl_opengl3.h
cp ./imgui_git/backends/imgui_impl_opengl3_loader.h ./imgui/backends/imgui_impl_opengl3_loader.h
#cp ./imgui_git/backends/imgui_impl_vulkan.cpp       ./imgui/backends/imgui_impl_vulkan.cpp
#cp ./imgui_git/backends/imgui_impl_vulkan.h         ./imgui/backends/imgui_impl_vulkan.h

# Source files
echo "Copying source files..."
mkdir -p ./imgui_git/src
cp ./imgui_git/imgui_demo.cpp		./imgui/src/imgui_demo.cpp
cp ./imgui_git/imgui_draw.cpp		./imgui/src/imgui_draw.cpp
cp ./imgui_git/imgui_internal.h		./imgui/src/imgui_internal.h
cp ./imgui_git/imgui_tables.cpp		./imgui/src/imgui_tables.cpp
cp ./imgui_git/imgui_widgets.cpp	./imgui/src/imgui_widgets.cpp
cp ./imgui_git/imgui.cpp			./imgui/src/imgui.cpp
cp ./imgui_git/imgui.h				./imgui/src/imgui.h
cp ./imgui_git/imstb_rectpack.h		./imgui/src/imstb_rectpack.h
cp ./imgui_git/imstb_textedit.h		./imgui/src/imstb_textedit.h
cp ./imgui_git/imstb_truetype.h		./imgui/src/imstb_truetype.h
cp ./imgui_git/LICENSE.txt			./imgui/src/LICENSE.txt

# Copy some docs
echo "Copying some docs..."
mkdir -p ./imgui_git/docs
#cp ./imgui_git/docs/ ./imgui/docs
mv ./imgui_git/docs/BACKENDS.md		./imgui/docs/BACKENDS.md
mv ./imgui_git/docs/CHANGELOG.txt	./imgui/docs/CHANGELOG.txt
mv ./imgui_git/docs/README.md		./imgui/docs/README.md

#copy extras
echo "Copying extras..."
mkdir -p ./imgui_git/extras
cp ./imgui_git/misc/cpp/imgui_stdlib.cpp ./imgui/extras/imgui_stdlib.cpp
cp ./imgui_git/misc/cpp/imgui_stdlib.h ./imgui/extras/imgui_stdlib.h

# Remove git repo // cleanup
#echo "Cleaning up git repo..."
#rm -rf ./imgui_git

# Apply patches
echo "\nApplying patches..."

# OSX sed is a pain in the ass. Use gnu-sed on osx.
if [ `uname -s` = "Darwin" ] ; then
	if ! [ -x "$(command -v gsed)" ] ; then
		echo "GSED is not installed, installing..."
		brew install gnu-sed

		# Todo: Brew might not be installed. Ask user to install gnu-sed and abort.
	fi;
fi;

echo "\nApplying GLFW multi context patch with obj-c ARC support..."
# Option 1 : Patch file ?
#git apply --check Glfw_MultiContext_Support.diff
patch --dry-run -s ./imgui/backends/imgui_impl_glfw.cpp ./Glfw_MultiContext_Support.diff > /dev/null 2>&1
if [ $? -eq 0 ]; then
	#git apply Glfw_MultiContext_Support.diff
	echo "Applying a GIT diff as it seems compatible..."
	patch ./imgui/backends/imgui_impl_glfw.cpp ./Glfw_MultiContext_Support.diff

# option 2 : Sed search/replace
else
	echo "The Git diff is not compatible anymore, trying to apply a custom GNU sed patch that might be less vulnerable to breaking changes."
	echo "This has only been tested on OSX."

	# todo: On linux use sed instead of gsed ? Tested on OSX only.

	echo "Injecting warning..."
	# Inform about changes
	gsed -i '1s/^/\/\/ Warning!\n\/\/ This file has been modified for ofxImGui to support context switching.\n\/\/ This is not the original one. Changes are indicated.\n\/\/----\n\n/' ./imgui/backends/imgui_impl_glfw.cpp
	
	echo "Applying multi-context patch..."
	# Add include
	gsed -i '/^#include "imgui_impl_glfw\.h"$/a #include "backends\/imgui_impl_glfw_context_support.h" \/\/ CUSTOM OFXIMGUI ADDED LINE' ./imgui/backends/imgui_impl_glfw.cpp
	# Add scoped context on glfw window callbacks
	gsed -i -e '/^.*void ImGui_ImplGlfw_.*Callback.*(GLFWwindow\* window.*)$/{n; s/$/\n\tImGui_ImplGlfw_ScopedContext sc\(window\)\; \/\/ CUSTOM OFXIMGUI ADDED LINE\n/}' ./imgui/backends/imgui_impl_glfw.cpp
	# Register Windows
	gsed -i '/^static void ImGui_ImplGlfw_CreateWindow(/,/^}$/s/^}$/\tImGui_ImplGlfw_RegisterWindowContext(vd->Window, ImGui::GetCurrentContext())\; \/\/ CUSTOM OFXIMGUI ADDED LINE\n}/' ./imgui/backends/imgui_impl_glfw.cpp
	# Forget windows
	gsed -i '/^static void ImGui_ImplGlfw_DestroyWindow(/,/^}$/s/vd->Window \= nullptr;$/ImGui_ImplGlfw_RemoveWindowContext(vd->Window)\; \/\/ CUSTOM OFXIMGUI ADDED LINE\n\t\tvd->Window \= nullptr\;/' ./imgui/backends/imgui_impl_glfw.cpp
	# Initialise in the context (needed for secondary viewports)
	gsed -i '/^static bool ImGui_ImplGlfw_Init(/{n; s/$/\n\tImGui_ImplGlfw_ScopedContext sc\(window\)\; \/\/ CUSTOM OFXIMGUI ADDED LINE\n/}' ./imgui/backends/imgui_impl_glfw.cpp

	# Modify imgui's GLFW autoconfig due to oF 0.11.0 and 0.11.2 (not 0.11.1) having a pre-3.3.0 glfw library on osx platform.
	# This causes the native backend to fail linking, the of-version hasn't got glfwGetMonitorWorkarea which was added only in the final version
	# before: #define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3300) // 3.3+ glfwGetMonitorWorkarea
	# after : #define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3301) // 3.3+ glfwGetMonitorWorkarea
	gsed -i '/^#define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3300) \/\/ 3.3+ glfwGetMonitorWorkarea$/i #if defined(TARGET_OSX) || defined(TARGET_WIN32) \/\/ BEGIN CUSTOM OFXIMGUI LINES\n#define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3301) \/\/ 3.3+ glfwGetMonitorWorkarea\n#else' ./imgui/backends/imgui_impl_glfw.cpp
	gsed -i '/^#define GLFW_HAS_MONITOR_WORK_AREA      (GLFW_VERSION_COMBINED >= 3300) \/\/ 3.3+ glfwGetMonitorWorkarea$/a #endif \/\/ END CUSTOM OFXIMGUI LINES' ./imgui/backends/imgui_impl_glfw.cpp

	# Obj-c ARC fixes // of_v0.12.0+
	# Inject the hack by adding this include below the GLFW `GLFW_HAS_GETERROR` define.
	# `#include "backends/imgui_impl_glfw_arc_support.h" // CUSTOM OFXIMGUI ADDED LINE`
	echo "Applying ARC patch..."
	gsed -i '/^#define GLFW_HAS_GETERROR/{n; s/$/\n#include "backends\/imgui_impl_glfw_arc_support.h" \/\/ CUSTOM OFXIMGUI ADDED LINE\n/}' ./imgui/backends/imgui_impl_glfw.cpp
	
	# Generate a new diff :
	# With timestamps included
	# diff -u ./imgui_git/backends/imgui_impl_glfw.cpp ./imgui/backends/imgui_impl_glfw.cpp > ./Glfw_MultiContext_Support_New.diff
	# Without timestamps
	git diff --no-index ./imgui_git/backends/imgui_impl_glfw.cpp ./imgui/backends/imgui_impl_glfw.cpp > ./Glfw_MultiContext_Support_new.diff
fi;

# Inject GLES1 compatibility in imgui_implopengl2
echo "\nApplying OpenGL2 patch for compatibility with GLES1..."

# Test the diff file
patch --dry-run -s ./imgui/backends/imgui_impl_opengl2.cpp ./Glfw_GLES1_Support.diff > /dev/null 2>&1
if [ $? -eq 0 ]; then
	#git apply Glfw_MultiContext_Support.diff
	echo "Applying a GIT diff as it seems compatible..."
	patch ./imgui/backends/imgui_impl_opengl2.cpp ./Glfw_GLES1_Support.diff
else
	echo "The Git diff is not compatible anymore, trying to apply a custom GNU sed patch that might be less vulnerable to breaking changes."
	echo "This has only been tested on OSX."

	echo "Injecting warning..."
	# Inform about changes
	gsed -i '1s/^/\/\/ Warning!\n\/\/ This file has been modified for ofxImGui to support GLES1.\n\/\/ This is not the original one. Changes are indicated.\n\/\/----\n\n/' ./imgui/backends/imgui_impl_opengl2.cpp

	echo "Applying GLES1 patches..."
	# Include GLES1 compatibility hack in imgui_implopengl2
	gsed -i '/^\/\/ Include OpenGL header/i \\/\/ --- BEGIN CUSTOM MODIFICATION\n#include "ofxImGuiConstants\.h"\n#if defined(OFXIMGUI_RENDERER_GLES)\n#include "gles1CompatibilityHacks\.h"\n#else// --- END CUSTOM MODIFICATION\n' ./imgui/backends/imgui_impl_opengl2.cpp

	# Modify the OpenGL loader condition
	gsed -i '/^#include <GL\/gl\.h>$/{n; s/$/\n#endif \/\/ CUSTOM OFXIMGUI ADDED LINE/}' ./imgui/backends/imgui_impl_opengl2.cpp
	#gsed -i '/^\/\/ Include OpenGL header/{n; s/^#if/#elif/; s/$/ \/\/ CUSTOM OFXIMGUI MODIFIED LINE/}' ./imgui/backends/imgui_impl_opengl2.cpp

	# Generate a new diff :
	git diff --no-index ./imgui_git/backends/imgui_impl_opengl2.cpp ./imgui/backends/imgui_impl_opengl2.cpp > ./Glfw_GLES1_Support_new.diff
fi;

# Done !
echo "\nDone !"
echo "ImGui has been updated, don't forget to check the breaking changes and verify if the examples work!\n"
