#pragma once

#include "ofMain.h"

#include "ofxImGui.h"
#include "ImHelpers.h"

class myAddonClass : public ofBaseApp
{
public:
    virtual void setup() override;
    virtual void setup( ofxImGui::Gui& _gui );
    virtual void draw() override;
//	virtual void exit() override; // Todo: destroy gui on exit and see what happens ?
	void drawImGui();

private:
    ofxImGui::Gui* gui = nullptr;

	float sizef = 1;
    float speedf = 0.1f;
    ImVec4 color = {1,1,1,1};
    bool showGuiWindow = true;
    bool enableDrawing = true;

    // ofParameters
    ofParameter<ofFloatColor> background{ "Background", ofFloatColor::black };
    ofParameter<ofFloatColor> foreground{ "Foreground", ofFloatColor::crimson };
    ofParameterGroup colors{ "Colors", background, foreground };
    ofParameter<float> rotationOffset{ "Rotation Offset", 180.0f, 0.0f, 360.f };
    ofParameter<float> minSize{ "Min size", 0.f, 0.0f, (float)ofGetHeight() }; // (float) is needed on Windows
};
