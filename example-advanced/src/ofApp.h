#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ImHelpers.h"
#include "RandomTheme.h"

void FxTestBed(bool*); // Forward declared

// It is easy to extend ImGui making simple functions that use the ImGui API.
// If you need access to inner ImGui variables (eg: more complex tweaks), it's better to write an ImGui plugin and load that. (see imdrawlist_party.cpp)
// Code from ImGuiDemo.cpp
static void HelpMarker(const char* desc){
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

class ofApp : public ofBaseApp{

    public:
        ofApp() : v(0) {}

        void setup() {
            // Enable viewports, enable state saving
            gui.setup(nullptr, false, ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_DockingEnable, true);
            //ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

            // The backgroundColor is stored as an ImVec4 type but can handle ofColor
            backgroundColor = ofColor(114, 144, 154);

            show_test_window = true;
            show_another_window = false;

            // Load your own ofImage
            imageButtonSource.load("of.png");
            imageButtonID = gui.loadImage(imageButtonSource);

            // Or have the loading done for you if you don't need the ofImage reference
            //imageButtonID = gui.loadImage("of.png");

            // You can also use ofPixels in same way
            ofLoadImage(pixelsButtonSource, "of_upside_down.png");
            pixelsButtonID = gui.loadPixels(pixelsButtonSource);

            // Pass in your own texture reference, and keep it for later usage
            textureSourceID = gui.loadTexture(textureSource, "of_upside_down.png");

            //or just pass a path
            //textureSourceID = gui.loadTexture("of_upside_down.png");

            ofLogVerbose() << "textureSourceID: " << textureSourceID;

            ofDirectory dataDirectory(ofToDataPath("", true));

            files = dataDirectory.getFiles();
            for(size_t i=0; i<files.size(); i++){
                fileNames.push_back(files[i].getFileName());

                // Create texture ID
                GLuint texID = gui.loadPixels(files[i].getFileName());
                fileTextures.push_back(texID);
            }
            imageButtonID = fileTextures.front();

            // It's nice to disable escape quitting your ofApp, as ImGui also listens to escape.
            ofSetEscapeQuitsApp(false);
        }

        void draw() {
            // Precalcs
            #define plotSize 200
            static float realFPS[plotSize]={0};
            static float avgFPS=0;
            avgFPS=0;
            for(int i=0; i<plotSize-1; ++i){
                avgFPS+=realFPS[i];
                realFPS[i]=realFPS[i+1];
            }
            realFPS[plotSize-1]=ofGetFrameRate();
            avgFPS=(avgFPS+realFPS[plotSize-1])/plotSize;

            // BackgroundColor is stored as an ImVec4 type but is converted to ofColor automatically
            ofSetBackgroundColor(backgroundColor);

            // Start drawing to ImGui (newFrame)
            gui.begin();

            // Draw a menu bar
            if(ImGui::BeginMainMenuBar()){

                if(ImGui::BeginMenu( "Options")){

                    ImGui::Checkbox("Show ImGui Metrics", &showMetrics);
                    ImGui::SameLine(); HelpMarker("The ImGui metrics is a gui window allowing you to inspect what is going on and how imgui is intagrated and interfaced with oF.");

                    ImGui::Separator();
                    ImGui::Checkbox("Show ImGui Demo", &showDemo);
                    ImGui::SameLine(); HelpMarker("The ImGui Demo Window demonstrates many gui possibilities and serves as a reference for available gui widgets, together with its source code.");

                    ImGui::Separator();
                    ImGui::Checkbox("Show ofxImGui Debug Window", &showOfxImGuiDebugWindow);
                    ImGui::SameLine(); HelpMarker("Helper window for debugging and optimising your ofxImGui instance within your ofApp.\nIt can also been seen as an interactive tutorial explaining how ofxmGui is setup (for avanced users).");

                    ImGui::Separator();
                    ImGui::Checkbox("Show ImGui FX", &showFX);
                    ImGui::SameLine(); HelpMarker("Demoscene contest !\nDemonstrates :\n - how to make an ImGui plugin\n - How to use the imgui drawlist api");

                    ImGui::Separator();
                    ImGui::Checkbox("Draw lines", &drawLines);
                    ImGui::SameLine(); HelpMarker("An example of how to control your ofApp with ofxImGui.");

                    ImGui::Separator();
                    ImGui::Checkbox("Show Sample Window", &showSampleWindow);
                    ImGui::SameLine(); HelpMarker("Demonstrates some advanced use cases of imgui.");

                    // Submenu
                    ImGui::Separator();
                    if(ImGui::BeginMenu( "More..." )){
                        ImGui::MenuItem( "Something" );
                        ImGui::MenuItem( "Something else" );
                        ImGui::MenuItem( "Something different" );
                        ImGui::EndMenu();
                    }

                    // Exit
                    ImGui::Separator();
                    if(ImGui::MenuItem( "Quit" )){
                        ofExit();
                    }

                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu( "Runtime")){

                    // Full Screen
                    static bool fullScreen = false;
                    if(ImGui::Checkbox("Full screen", &fullScreen)){
                        ofSetFullscreen(fullScreen);
                    }

                    // Vertical Sync
                    static bool vSync = false;
                    if(ImGui::Checkbox("Vertical Sync", &vSync)){
                        ofSetVerticalSync(vSync);
                    }

                    ImGui::Separator();

                    // Resolution changer
                    static int resolution[2];
                    resolution[0]=ofGetWidth();
                    resolution[1]=ofGetHeight();
                    std::string resString = ofToString(resolution[0]).append(" x ").append(ofToString(resolution[1]));
                    if(ImGui::BeginCombo("Resolution", resString.c_str())){
                        if(ImGui::Selectable("800 × 600")){
                            ofSetWindowShape(800,600);
                        }
                        if(ImGui::Selectable("1024 × 768")){
                            ofSetWindowShape(1024, 768);
                        }
                        if(ImGui::Selectable("1366 × 768")){
                            ofSetWindowShape(1366, 768);
                        }
                        if(ImGui::InputInt2("Custom", resolution)){
                            ofSetWindowShape(resolution[0],resolution[1]);
                        }
                        ImGui::EndCombo();
                    }

                    // FPS
                    static int appFPS=30;//ofGetTargetFrameRate();
                    if(ImGui::DragInt("FPS Target", &appFPS, 1.f, 10)){ // Minimum set to 10 because double click is not detected below it
                        ofSetFrameRate(appFPS);
                        appFPS=ofGetTargetFrameRate(); // re-sync value with oF
                    }
                    if(ImGui::IsItemHovered()){
                        ImGui::BeginTooltip();
                        ImGui::Text("(double-click to edit)");
                        ImGui::EndTooltip();
                    }
                    static char avgFPSString[28]={0};
                    sprintf(avgFPSString, "FPS : %.2f average", avgFPS);
                    ImGui::PlotHistogram("FPS Histogram", realFPS, plotSize, 0, avgFPSString, 0, 60);

                    ImGui::EndMenu();
                }
            } // End menu visible

            ImGui::EndMainMenuBar();

            // Start a transparent docking space to allow docked windows within the ofAppWindow
            ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

            // Line drawing logic
            glm::vec2 windowPos;
            // With viewports enabled, coordinates are absolute
            if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)  windowPos -= glm::vec2(ofGetWindowPositionX(), ofGetWindowPositionY());
            glm::vec2 windowSize;
            static bool isWindowDragging = false;
            static bool isWindowFocused = false;

            // Create a new window
            // ImGuiCond_Once => Set on launch, then let user use their size
            ImGui::SetNextWindowPos(ImGui::GetWindowViewport()->Pos + ImVec2(((ofRandomf()+1.f)*.25f)*ofGetWidth(), ((ofRandomf()+1.f)*.25f)*ofGetHeight()), ImGuiCond_Once);
            ImGui::SetNextWindowSize(ImVec2(320,240), ImGuiCond_Once);

            if(ImGui::Begin("Main Window\n")){
                ImGui::Text("Example-Advanced");
                ImGui::TextWrapped(
                    "All windows except this one should restore their size and positions when re-opening this ofApp.\n\n"\
                    "The graphics around this window are pimped by oF. Use the options menu to draw lines instead.\n\n"
                );
                ImGui::Text("Use the options menu to enable different demo components.");
                ImGui::Text("There are some help indicators around.");
                ImGui::SameLine(); HelpMarker("Bravo, you got it !");

                ImGui::Spacing();
                ImGui::SeparatorText("Layout");
#ifdef OFXIMGUI_BACKEND_GLFW
                ImGui::TextWrapped("ImGui windows can be poped-out i using the GLFW");
#elif defined(OFXIMGUI_BACKEND_OPENFRAMEWORKS)
                ImGui::TextWrapped("You're using the custom OpenFrameworks backend. Windows cannot pop-out of the ofAppWindow, but you can still combine and layout imgui windows.");
#else
                ImGui::TextDisabled("Unknown backend = Unknown layouts !");
#endif
                ImGui::TextWrapped("Try dragging around the imgui windows by their title and trop them within other windows to make the layout yours !");

            }
            // Query ImGui window state
            // Note: Helpers help convert ImVec2 to glm::Vec2
            windowPos         = ImGui::GetWindowPos();
            if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)  windowPos -= glm::vec2(ofGetWindowPositionX(), ofGetWindowPositionY()); // with viewports enabled, coordinates change
            windowSize        = ImGui::GetWindowSize();
            isWindowDragging  = ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left);
            isWindowFocused   = ImGui::IsWindowFocused();

            ImGui::End(); // end Main Window

            static float speed1 = 0.072f;
            static float speed2 = 0.103f;
            static float variation1 = 0.0147f;
            static float variation2 = 0.032f;
            static float offset1 = 3.4f;
            static float offset2 = 5.1f;
            static ImVec4 color = ImVec4(1.f, 1.f, 1.f, 1.f);

            // Line Settings window
            if( drawLines ){
                ImGui::Begin("Line Settings", &drawLines );

                if( ImGui::CollapsingHeader("Line settings", ImGuiTreeNodeFlags_DefaultOpen) ){

                    ImGui::TreePush((void*)NULL);

                    ImGui::Dummy(ImVec2(20,5));

                    ImGui::SliderInt("Lines per swarm", &linesPerSide, 1, 1000);
                    ImGui::DragFloat("Line Spacing", &linesSpacing, 0.0001f, 0.001f, 0.5f, "%.4f");
                    ImGui::DragFloat("Line Thickness", &lineThickness, 0.01f, 0.01f, 2.f, "%.2f");
                    ImGui::ColorEdit4("Color", (float*)&color);

                    ImGui::Dummy(ImVec2(20,10));

                    // It's also easy to add extra Gui interactions to fit your needs.
                    ImGui::SliderFloat("Speed 1", &speed1, 0.001f, 1.5f);
                    if(ImGui::IsItemActive() && ofGetKeyPressed(OF_KEY_SHIFT))
                        speed2 = speed1;
                    ImGui::SameLine(); HelpMarker("Hold shift to sync both speeds");
                    ImGui::SliderFloat("Speed 2", &speed2, 0.001f, 1.5f);
                    if(ImGui::IsItemActive() && ofGetKeyPressed(OF_KEY_SHIFT))
                        speed1 = speed2;
                    ImGui::SameLine(); HelpMarker("Hold shift to sync both speeds");

                    ImGui::Separator();
                    ImGui::SliderFloat("Speed Variation 1", &variation1, 0.001f, 1.5f);
                    ImGui::SliderFloat("Speed Variation 2", &variation2, 0.001f, 1.5f);

                    ImGui::Separator();
                    ImGui::SliderFloat("Offset 1", &offset1, 0.001f, 10.f);
                    ImGui::SliderFloat("Offset 2", &offset2, 0.001f, 10.f);

                    ImGui::TreePop();
                }
                // Close the line settings window
                ImGui::End();
            }

            if(showSampleWindow){
                if(ImGui::Begin("Sample Window", &showSampleWindow)){
                    if (ImGui::Button("Another Window"))
                    {
                        //bitwise OR
                        show_another_window ^= 1;
                    }

                    ImGui::Dummy(ImVec2(10,10));
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

                    ImGui::Dummy(ImVec2(10,10));
                    if (ImGui::Button("CUSTOM THEME")){
                        gui.setTheme(new RandomTheme());

                    }
                    ImGui::SameLine();
                    if (ImGui::Button("DEFAULT THEME")){
                        gui.setTheme(new ofxImGui::DefaultTheme());
                    }

                    //ImGui::ShowStyleEditor();
                    //ImGui::ShowFontSelector("label4FontSelector");

                    // Texture loading
                    ImGui::Dummy(ImVec2(10,10));
                    if(!fileNames.empty()){
                        //ofxImGui::VectorListBox allows for the use of a vector<string> as a data source
                        static int currentListBoxIndex = 0;
                        if(ofxImGui::VectorListBox("VectorListBox", &currentListBoxIndex, fileNames)){
                            ofLog() << " VectorListBox FILE PATH: "  << files[currentListBoxIndex].getAbsolutePath();
                            imageButtonID = fileTextures[currentListBoxIndex];
                        }

                        //ofxImGui::VectorCombo allows for the use of a vector<string> as a data source
                        static int currentFileIndex = 0;
                        if(ofxImGui::VectorCombo("VectorCombo", &currentFileIndex, fileNames)){
                            ofLog() << "VectorCombo FILE PATH: "  << files[currentFileIndex].getAbsolutePath();
                            pixelsButtonID = fileTextures[currentFileIndex];
                        }
                    }

                    //GetImTextureID is a static function defined in Helpers.h that accepts ofTexture, ofImage, or GLuint
                    ImGui::Dummy(ImVec2(10,10));
                    if(ImGui::ImageButton("imagebutton", GetImTextureID(imageButtonID), ImVec2(200, 200))){
                           ofLog() << "PRESSED";
                    }
                    //or do it manually
                    ImGui::SameLine();
                    ImGui::Image(GetImTextureID(pixelsButtonID), ImVec2(200, 200));
                    ImGui::Image((ImTextureID)(uintptr_t)textureSourceID, ImVec2(200, 200));
                }
                ImGui::End();
            }


            // Show another window, this time using an explicit ImGui::Begin and ImGui::End
            if (show_another_window){
                //note: ofVec2f and ImVec2f are interchangeable
                ImGui::SetNextWindowSize(ofVec2f(200,100), ImGuiCond_FirstUseEver);
                ImGui::Begin("Another Window", &show_another_window);
                ImGui::Text("Hello");
                ImGui::End();
            }

            if(showMetrics){
                ImGui::SetNextWindowPos(ImVec2(ofGetWindowPositionX()+ofGetWidth()-350,ofGetWindowPositionY()+30), ImGuiCond_FirstUseEver );
                ImGui::ShowMetricsWindow( &showMetrics );
            }

            if(showDemo) ImGui::ShowDemoWindow(&showDemo);

            if(showOfxImGuiDebugWindow) gui.drawOfxImGuiDebugWindow(&showOfxImGuiDebugWindow);

            if(showFX){
                // Window position is only set on the first launch
                ImGui::SetNextWindowPos(ImVec2(ofGetWindowPositionX()+ofGetWidth()-350,ofGetWindowPositionY()+ofGetHeight()-230), ImGuiCond_FirstUseEver );
                FxTestBed(&showFX); // <-- ImGui extension from im_drawlist_party.cpp
            }

            gui.end();

            // Line drawing variables
            glm::vec2 windowTopLeft     = windowPos;
            glm::vec2 windowTopRight    = windowTopLeft   + glm::vec2(windowSize.x,0);
            glm::vec2 windowBottomLeft  = windowTopLeft   + glm::vec2(0, windowSize.y);
            glm::vec2 windowBottomRight = windowTopLeft   + windowSize;

            glm::vec2 ofWindowTopLeft     = glm::vec2(0                 ,0);
            glm::vec2 ofWindowTopRight    = glm::vec2(ofGetWindowWidth(),0);
            glm::vec2 ofWindowBottomLeft  = glm::vec2(0                 ,ofGetWindowHeight());
            glm::vec2 ofWindowBottomRight = glm::vec2(ofGetWindowWidth(),ofGetWindowHeight());

            // Space changes ofWindow to imguiViewport.
            if( !ofGetKeyPressed(' ')){
                ofRectangle viewport = gui.getMainWindowViewportRect();
                ofWindowTopLeft     = viewport.getTopLeft();
                ofWindowTopRight    = viewport.getTopRight();
                ofWindowBottomLeft  = viewport.getBottomLeft();
                ofWindowBottomRight = viewport.getBottomRight();
            }

            if(drawLines){
                ofSetLineWidth(lineThickness);
                ofSetColor(color.x*255.f, color.y*255.f, color.z*255.f, color.w*255.f);
                const static int numSides = 4;
                for(int i=0; i < linesPerSide*numSides; i++){
                    glm::vec2 fromA  = (i<linesPerSide)? ofWindowTopLeft  :((i<linesPerSide*2)? ofWindowTopRight    :((i<linesPerSide*3)? ofWindowBottomRight : ofWindowBottomLeft));
                    glm::vec2 toA    = (i<linesPerSide)? windowTopLeft    :((i<linesPerSide*2)? windowTopRight      :((i<linesPerSide*3)? windowBottomRight   : windowBottomLeft  ));
                    glm::vec2 fromB  = (i<linesPerSide)? ofWindowTopRight :((i<linesPerSide*2)? ofWindowBottomRight :((i<linesPerSide*3)? ofWindowBottomLeft  : ofWindowTopLeft   ));
                    glm::vec2 toB    = (i<linesPerSide)? windowTopRight   :((i<linesPerSide*2)? windowBottomRight   :((i<linesPerSide*3)? windowBottomLeft    : windowTopLeft     ));
                    unsigned int sideNum= (i<linesPerSide)? 0             :((i<linesPerSide*2)? 1                   :((i<linesPerSide*3)? 2                   : 3                 ));
                    int lineNum      = i % linesPerSide;
                    int variantA     = (sideNum+0)%numSides;
                    int variantB     = (sideNum+1)%numSides;
                    ofDrawLine(
                      //                                                     |Speed  | A/B variation                    |       Spacing        | Differentiation
                      ofInterpolateCosine( fromA , toA , ofGetElapsedTimef()*(speed1 + variation1 * variantA * 0.2f )   + lineNum*linesSpacing + ((variantA)%2)*5.3f*offset1),
                      ofInterpolateCosine( fromB,  toB,  ofGetElapsedTimef()*(speed1 + variation1 * variantB * 0.2f )   + lineNum*linesSpacing + ((variantB)%2)*5.3f*offset1)
                    );
                    ofDrawLine(
                      ofInterpolateCosine( fromA , toA , ofGetElapsedTimef()*(speed2 + variation2* variantA * 0.2f )   + lineNum*linesSpacing + ((variantB)%2)*5.3f*offset2),
                      ofInterpolateCosine( fromB,  toB,  ofGetElapsedTimef()*(speed2 + variation2* variantB * 0.2f )   + lineNum*linesSpacing + ((variantA)%2)*5.3f*offset2)
                    );
                }
                ofSetLineWidth(1.f);
            }
            else {
                // Connect Window corners with OF corners
                ofDrawLine(ofWindowTopLeft    , windowTopLeft    );
                ofDrawLine(ofWindowTopRight   , windowTopRight   );
                ofDrawLine(ofWindowBottomRight, windowBottomRight);
                ofDrawLine(ofWindowBottomLeft , windowBottomLeft );

                // Draw a rectangle behind the window
                ofFill();
                if(isWindowDragging) ofSetColor(255,0,0,126);
                else ofSetColor(255,isWindowFocused?80:40);
                ofDrawRectRounded(windowTopLeft-glm::vec2(10), windowSize.x+20, windowSize.y+20, 10);

                // Draw Circles on corners
                ofSetColor(255,255);
                ofNoFill();
                ofDrawCircle(windowTopLeft    , glm::mod( ofGetElapsedTimef()*10.f     , 10.f) );
                ofDrawCircle(windowTopRight   , glm::mod( ofGetElapsedTimef()*10.f+5.0f, 10.f) );
                ofDrawCircle(windowBottomLeft , glm::mod( ofGetElapsedTimef()*10.f+2.5f, 10.f) );
                ofDrawCircle(windowBottomRight, glm::mod( ofGetElapsedTimef()*10.f+7.5f, 10.f) );
            }

            gui.draw();

        }

        //--------------------------------------------------------------
        void keyPressed(int key){

            ofLogVerbose(__FUNCTION__) << key;
            switch (key) {

                case 's':
                {
                    break;
                }
            }
        }

        //--------------------------------------------------------------
        void keyReleased(int key){
            ofLogVerbose(__FUNCTION__) << key;
        }

        void mouseScrolled(int x, int y, float scrollX, float scrollY){
            ofLogVerbose(__FUNCTION__) << "x: " << x << " y: " << y;
        }
        //--------------------------------------------------------------
        void mouseMoved(int, int){}

        //--------------------------------------------------------------
        void mouseDragged(int , int , int){}

        //--------------------------------------------------------------
        void mousePressed(int, int, int){}

        //--------------------------------------------------------------
        void mouseReleased(int, int, int){}

        //--------------------------------------------------------------
        void windowResized(int, int){}

        //--------------------------------------------------------------
        void gotMessage(ofMessage){}

        //--------------------------------------------------------------
        void dragEvent(ofDragInfo){}

    private:
        ofxImGui::Gui gui;

        // Variables exposed to ImGui
        bool showMetrics      = true;
        bool showDemo         = false;
        bool showFX           = false;
        bool showSampleWindow = true;
        bool showOfxImGuiDebugWindow = false;

        float v = 0;

        int   linesPerSide  = 15;
        float linesSpacing  = 0.02f;
        float lineThickness = 0.8f;
        bool  drawLines     = false;

        ImVec4 backgroundColor;

        bool   show_test_window;
        bool   show_another_window;

        ofImage   imageButtonSource;
        GLuint    imageButtonID;
        ofPixels  pixelsButtonSource;
        GLuint    pixelsButtonID;
        ofTexture textureSource;
        GLuint    textureSourceID;

        std::vector<std::string> fileNames;
        std::vector<ofFile> files;
        std::vector<GLuint> fileTextures;
};
