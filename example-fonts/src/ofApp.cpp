#include "ofApp.h"
#include "IconsFontAwesome5.h"
#include "ProggyTiny.cpp" // <-- Be sure to include this only 1 time from a cpp file. (do not include from a header file)

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    // The call to setup() is required BEFORE adding fonts.
    bool autoDraw = true;
    ofxImGui::BaseTheme* theme = nullptr;
    ImGuiConfigFlags customFlags = ImGuiConfigFlags_DockingEnable;
    bool restoreGuiState = false;
    bool showImGuiMouseCursor = false;
    gui.setup(theme, autoDraw, customFlags, restoreGuiState, showImGuiMouseCursor);
    
    // Add polish characters
    static const ImWchar polishCharRanges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0100, 0x01FF, // Polish characters
        0,
    };
    static const ImWchar* normalCharRanges = ImGui::GetIO().Fonts->GetGlyphRangesDefault();
    static const ImWchar* myCharRanges = polishCharRanges;
    //myCharRanges = normalCharRanges; // Uncomment to disable polish characters

    // Set font and keep a reference of it for using it later
    // Font files are located within the data folder
    customFont = gui.addFont("Roboto-Medium.ttf",16.f, nullptr, myCharRanges, false);

    // You can also load fonts from memory, optionally compressed
    // It will compile the font within the binary, so you don't have to ship the font file separately. Increases binary size.
    // https://github.com/ocornut/imgui/blob/master/docs/FONTS.md#using-font-data-embedded-in-source-code
    // #include "MyCompressedFont.h"
    // gui.addFontFromMemory((void*)MyCompressedFont, sizeof(MyCompressedFont),16.f, nullptr, myCharRanges);

    // Here we directly interact with the ImGui fonts API instead of ofxImGui
    // (not recommended, but supports some more features such as compression)
    // Fonts from memory are hard-coded in the compiled binary using a C++ source file holding the data.
    auto& io = ImGui::GetIO();
    proggyFont = io.Fonts->AddFontFromMemoryCompressedTTF(&ProggyTiny_compressed_data, ProggyTiny_compressed_size, 10);

    // After manipulating fonts via the ImGui API, you need to trigger ofxImGui
    gui.rebuildFontsTexture();

    // Add fontawesome fonts by merging new glyphs
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f; // Use if you want to make the icon monospaced
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    gui.addFont("fa-regular-400.ttf", 18.f, &config, icon_ranges);

    // For more advanced font loading examples, please refer to
    // https://github.com/ocornut/imgui/blob/master/docs/FONTS.md
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    
    
}
//--------------------------------------------------------------
void ofApp::draw(){
    
    // Start imgui
    gui.begin();

    ImGui::SetNextWindowPos(ImVec2(5,5), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(600-10,600-10), ImGuiCond_Once);
    ImGui::Begin("Font Examples");
    
    ImGui::Spacing();
    ImGui::CollapsingHeader("Default font", ImGuiTreeNodeFlags_Leaf);
    // Uses the default font
    ImGui::Text("Hello, world!");
    ImGui::Spacing();

    // Use 2ndary font
    ImGui::CollapsingHeader("Special characters", ImGuiTreeNodeFlags_Leaf);

    // u8 ensures text is encoded as utf8 from the cpp source code
    ImGui::Text(u8"Witaj świecie !");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(255,255,255,0.5), "<-- one character is not loaded in this font !");

    // Custom font has more caracters, allowing to render them all
    ImGui::PushFont(customFont);
    ImGui::Text(u8"Witaj świecie !");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(255,255,255,0.5), "<-- with another font, it's rendered correctly !");
    ImGui::Text(u8"Some polish characters: ć, ń, ó, ś, ź, ż, ą, ę, ł.");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0,10));

    // Fontawesome
    ImGui::CollapsingHeader("Fontawesome icons", ImGuiTreeNodeFlags_Leaf);
    ImGui::PushFont(customFont);
    ImGui::Text("FontAweome is loaded too ! %s", ICON_FA_THUMBS_UP);
    ImGui::Button( ICON_FA_TRASH_ALT " Trash it !");
    ImGui::SameLine();
    ImGui::Button( ICON_FA_ANGRY " Mad button !");
    ImGui::SameLine();
    ImGui::Button( ICON_FA_BELL " Ring it !");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0,10));

    // ProggyTiny font
    ImGui::PushFont(proggyFont);
    ImGui::CollapsingHeader("ProggyTiny font", ImGuiTreeNodeFlags_Leaf);
    ImGui::Text("This text is rendered with ProggyTiny.");
    ImGui::Text("The font has been loaded from the program binary.");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0,10));

    // Default font
    ImGui::CollapsingHeader("More", ImGuiTreeNodeFlags_Leaf);
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Default font: %s", io.FontDefault==nullptr?"[None]":io.FontDefault->GetDebugName());
    ImGui::Dummy(ImVec2(0,10));

    // More
    ImGui::TextWrapped("For more advanced font loading examples, please refer to : ");
    ImGui::Text("https://github.com/ocornut/imgui/blob/master/docs/FONTS.md");
    ImGui::Dummy(ImVec2(0,10));

    // Show imgui font viewer / debugger
    ImGui::Dummy(ImVec2(0,10));
    ImGui::CollapsingHeader("ImGui Font Debugger", ImGuiTreeNodeFlags_Leaf);
    //ImGui::ShowFontSelector("Default font");
    ImGui::ShowStyleEditor();

    ImGui::End(); // close window
    
    // End imgui
    gui.end();
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}


void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY)
{

}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
