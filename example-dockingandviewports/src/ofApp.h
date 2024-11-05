#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "imgui_internal.h" // <-- example uses some imgui internals...

class ofApp : public ofBaseApp{

	public:
        ofApp() {}

		void setup() {
            // Setup imgui with the appropriate config flags
            gui.setup(nullptr, false, ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable, true);

            // Uncomment below to perform docking with SHIFT key
            // Gives a better user experience, matter of opinion.
            //ImGui::GetIO().ConfigDockingWithShift=true;

            // Uncomment below to "force" all imgui windows to be standalone
            //ImGui::GetIO().ConfigViewportsNoAutoMerge=true;

            ofSetBackgroundAuto(false);
            ofEnableAlphaBlending();
        }

		void draw() {
            // erase the bg progressively to reveal which ImGui window are within the OF window
            ofPushStyle();
            ofEnableBlendMode(OF_BLENDMODE_SUBTRACT);
            ofFill();
            ofSetColor(200, 200, 200, 2);
            ofSetRectMode(OF_RECTMODE_CENTER);
            ofDrawRectangle(ofGetWidth()*.5f, ofGetHeight()*.5f, ofGetWidth()+20, ofGetHeight()+20);
            ofSetRectMode(OF_RECTMODE_CORNER);
            ofPopStyle();

            ofPushStyle();
            // Example info text
            ofDrawBitmapStringHighlight(
                        "Docking windows lets you easily re-arrange your window space.\n"\
                        "Start dragging a window and drop it on a dockable zone !\n\n"\

                        "Viewports : When you drag a window out of the oF window,\n"\
                        "it seamlessly leaves the oF window context to become standalone.\n"\
                        "if you see onion skins while dragging the window, it's in the same window as oF.",
                        100, 40
                        );
            ofPopStyle();

            // If you need to know the gui viewport, checkout this helper
            // Note: For testing purposes demonstrated before the GUI starts, because this is accessible from anywhere.
            ofSetColor(ofColor::green, 5);
            ofFill();
            ofRectangle vp = gui.getMainWindowViewportRect();
            static const int cSize=10;
            ofDrawCircle(glm::vec2(vp.getTopLeft()), cSize);
            ofDrawCircle(glm::vec2(vp.getBottomLeft()), cSize);
            ofDrawCircle(glm::vec2(vp.getTopRight()), cSize);
            ofDrawCircle(glm::vec2(vp.getBottomRight()), cSize);

            // Start drawing to ImGui (newFrame)
			gui.begin();

            // Make windows transparent, to demonstrate drawing behind them.
            ImGui::PushStyleColor(ImGuiCol_WindowBg , IM_COL32(200,200,200,128)); // This styles the docked windows

            ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_PassthruCentralNode; // Make the docking space transparent
            // Fixes imgui to expected behaviour, having a transparent central node in passthru mode.
            // Alternative: Otherwise add in ImGui::DockSpace() [±line 14505] : if (flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;
            ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0,0,0,0));

            //dockingFlags |= ImGuiDockNodeFlags_NoDockingInCentralNode; // Uncomment to always keep an empty "central node" (a visible oF space)
            //dockingFlags |= ImGuiDockNodeFlags_NoTabBar; // Uncomment to disable creating tabs in the main view

            // Define the ofWindow as a docking space
            // Also draws the docked windows at this stage.
            ImGuiID dockNodeID = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockingFlags);
            ImGui::PopStyleColor(2);

            // Show how to query the imgui docking layout and draw some stuff with it
            // Advanced use, you probably wouldn't need this.
            ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockNodeID);
            if(dockNode){
                ImGuiDockNode* centralNode = ImGui::DockBuilderGetCentralNode(dockNodeID);
                // Verifies if the central node is empty (visible empty space for oF)
                if( centralNode && centralNode->IsEmpty() ){
                    ImRect availableSpace = centralNode->Rect();
                    //availableSpace.Max = availableSpace.Min + ImGui::GetContentRegionAvail();

                    // Submit a red rectangle using the ImGui draw api.
                    // It will be drawn at the same stage as the other Gui elements, by order of submission.
                    ImGui::GetForegroundDrawList()->AddRect(availableSpace.GetTL()+ImVec2(8,8), availableSpace.GetBR()-ImVec2(8,8), IM_COL32(255,50,50,255));

                    ImVec2 viewCenter = availableSpace.GetCenter();
                    // Depending on the viewports flag, the XY is either absolute or relative to the oF window.
                    if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) viewCenter = viewCenter - ImVec2(ofGetWindowPositionX(),ofGetWindowPositionY());

                    // These drawings will be submitted immediately by the renderer pipeline.
                    // So they will be behind the GUI, submitted on `gui.draw()`
                    ofPushStyle();
                    ofSetRectMode(OF_RECTMODE_CENTER);
                    ofSetColor(255,0,0,2);
                    ofNoFill();

                    ofDrawRectangle(
                                viewCenter.x,
                                viewCenter.y,
                                availableSpace.GetSize().x-6,
                                availableSpace.GetSize().y-6
                    );
                    ofNoFill();
                    ofSetColor(255,255,255,30);
                    ofDrawRectangle(
                                (viewCenter.x),
                                (viewCenter.y),
                                (availableSpace.GetSize().x-20)*fmodf(abs(sin(ofGetElapsedTimef())),1.f),
                                (availableSpace.GetSize().y-20)*fmodf(abs(sin(ofGetElapsedTimef())),1.f)
                    );
                    ofSetRectMode(OF_RECTMODE_CORNER);
                    ofPopStyle();
                }
            }

            // Draw a few windows
            static int val0=0, val1=0,val2=0,val3=0,val4=0,val5=0;
            drawWindow("Dockable Window",   val0,   50,     100, ImGuiWindowFlags_None  );
            drawWindow("Window 1",          val1,   300,    100, ImGuiWindowFlags_None  );
            drawWindow("Window 2",          val2,   50,     350, ImGuiWindowFlags_None  );
            drawWindow("Window 3",          val3,   300,    350, ImGuiWindowFlags_None  );
            drawWindow("Window 4",          val4,   550,    100, ImGuiWindowFlags_None  );
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID); // Attach a window to a viewport = prevent popping it out
            drawWindow("Stuck in main window", val5, 550,    350, ImGuiWindowFlags_None  );

            // End our ImGui Frame. Also Renders in autoDraw mode.
            gui.end();

            gui.draw(); // <-- In manual mode, you can choose to render imgui at a given moment in your pipeline

            // Show FPS
            ofDrawBitmapStringHighlight( ofToString(ofGetFrameRate() ), 10, 50, ofColor(0,60), ofColor(255,128)); // <-- This text will be drawn over the layout

            if(!dockNode || !ImGui::DockBuilderGetCentralNode(dockNodeID) || !ImGui::DockBuilderGetCentralNode(dockNodeID)->IsEmpty()){
                static int posX=1, posY=1, velX=1, velY=1;
                ofDrawBitmapStringHighlight("No empty dock space, nowhere to draw for oF !", posX, posY);
                posX+=velX*3;
                posY+=velY*3;
                ofBitmapFont f;
                ofRectangle strSize = f.getBoundingBox("No empty dock space, nowhere to draw for oF !",posX,posY);
                if(posX <= 0) velX=1;
                if(posY <= strSize.height) velY=1;
                if(posX >= ofGetWidth()-strSize.width) velX=-1;
                if(posY >= ofGetHeight()) velY=-1;
            }
		}

    private:

        void drawWindow(const char* _title, int& _value, int _x=300, int _y=300, ImGuiWindowFlags _flags=ImGuiWindowFlags_None ){
            ImGui::SetNextWindowSize(ImVec2(200,200), ImGuiCond_Once);
            ImGui::SetNextWindowPos(ImVec2(_x+ofGetWindowPositionX(),_y+ofGetWindowPositionY()), ImGuiCond_Once);
            ImGui::Begin(_title, NULL, _flags );

            ImGui::InputInt("InputInt", &_value);
            ImGui::SliderInt("SliderInt", &_value, 0, 10);
            ImGui::DragInt("DragInt", &_value);
            ImGui::End();
        }

        ofxImGui::Gui gui;
};
