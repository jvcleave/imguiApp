#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "IconsFontAwesome5.h"

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    
    ofxImGui::Gui gui;
    ImFont* customFont = nullptr;
    ImFont* proggyFont = nullptr;

};
