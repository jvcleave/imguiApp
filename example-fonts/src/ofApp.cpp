#include "ofApp.h"

#include "imgui_stdlib.h" // For string input widgets

// Important !
// Be sure to include each font file only 1 time from a cpp file. (do not include from a header file)
#include "IconsFontAwesome5.h"
#include "ProggyTiny.cpp"

// Fixme for c++20 : `u8"stringLiteral"` will not compile anymore.
// Will become : `(const char*) u8"stringLiteral`

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
    // Font files are located within the data folder.
    // Note: Takes ownership. Use for reference only. ImGui handles font ownership (unless explicitly requested by the user)
    customFont = gui.addFont("Roboto-Medium.ttf", 16.f, nullptr, myCharRanges, false);

    // Add fontawesome fonts
    // Demonstrates merging additional characters into an existing font
    // (to prevent having to change the ImGui font when displaying additional characters)
    // Note: MergeMode adds the additional glyphs into the last used/loaded font : customFont.
    useBigFaIcons = true; // Enable to demonstrate making the font bigger (optional)
    ImFontConfig faConfig;
    faConfig.MergeMode = true; // Merge within the previously used ImFont
    faConfig.GlyphMinAdvanceX = 13.0f; // To make the icon monospaced (optional)
    float faHeight = faConfig.MergeMode ? 16.f : 20.f; // It's advised to use the same height in merge mode !
    // Make the fonts bigger
    if(useBigFaIcons){
        faHeight += 8;
        // In MergeMode, it's advised to use the same height as the original font
        // If enabled with a different height, we can use GlyphOffset.y to center the glyphs vertically.
        if(faConfig.MergeMode){
            faConfig.GlyphOffset.y = 8/2; // Center icons vertically
        }
    }
    // Inject glyphs
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    gui.addFont(FONT_ICON_FILE_NAME_FAR, faHeight, &faConfig, icon_ranges); // FONT_ICON_FILE_NAME_FAR = "fa-regular-400.ttf"

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

    // After manipulating fonts via the ImGui API, you need to rebuild the font texture
    gui.rebuildFontsTexture();

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
    ImGui::TextWrapped("The glyphs are merged into another one, allowing mix text and " ICON_FA_IMAGES " without changing font.");
    ImGui::Button( ICON_FA_TRASH_ALT " Trash it !");
    ImGui::SameLine();
    ImGui::Button( ICON_FA_ANGRY " Mad button !");
    ImGui::SameLine();
    ImGui::Button( ICON_FA_BELL " Ring it !");
    ImGui::PopFont();
    if(useBigFaIcons){
        ImGui::TextDisabled("Icons are loaded bigger on purpose.");
    }
    ImGui::TextDisabled("Note: the embedded icon set is NOT complete.");
    ImGui::Dummy(ImVec2(0,10));

    // ProggyTiny font
    ImGui::PushFont(proggyFont);
    ImGui::CollapsingHeader("ProggyTiny font", ImGuiTreeNodeFlags_Leaf);
    ImGui::Text("This text is rendered with ProggyTiny.");
    ImGui::Text("The font has been loaded from the program binary.");
    ImGui::PopFont();
    ImGui::Dummy(ImVec2(0,10));

    // Here we test if the string literal is passed correctly, helps debugging
    ImGui::CollapsingHeader("ImGui character debugger helper", ImGuiTreeNodeFlags_Leaf);
    ImGui::TextDisabled("Helps debugging incorrectly rendered characters.");
    if(ImGui::TreeNode("Static string literal")){
        ImGui::DebugTextEncoding("Witaj świecie !"); // incorrect on some platforms (for demo)
        //ImGui::DebugTextEncoding(u8"Witaj świecie !"); // correct
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Dynamic text")){
        static std::string customString = "0-éàî-°C";
        ImGui::InputText("Custom string", &customString);
        ImGui::DebugTextEncoding(customString.c_str());
        ImGui::TreePop();
    }
    ImGui::Dummy(ImVec2(0,10));

    // Default font
    ImGui::CollapsingHeader("Default font", ImGuiTreeNodeFlags_Leaf);
    ImGuiIO& io = ImGui::GetIO();
    // Refer to imgui_internal GetDefaultFont()
    ImGui::Text("Default font: %s", io.FontDefault==nullptr?"[None]":io.FontDefault->GetDebugName());
    if(io.FontDefault==nullptr){
        ImGui::SameLine();
        ImGui::TextDisabled(" (uses first font)");
    }
    ImGui::Dummy(ImVec2(0,10));

    // More
    ImGui::SeparatorText("More");
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
