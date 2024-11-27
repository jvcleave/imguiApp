#include "DefaultTheme.h"

#include "ofColor.h"
#include "imgui.h"

// from imgui_internals.h
static inline ImVec4 ImLerp(const ImVec4& a, const ImVec4& b, float t)          { return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t); }
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs)            { return ImVec4(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w); }


namespace ofxImGui
{
    
    
    void DefaultTheme::setup()
    {
        ofColor col_main_text = ofColor::fromHex(0xdbede2);
        ofColor col_main_head = ofColor::fromHex(0xd12d49);
        ofColor col_main_area = ofColor::fromHex(0x333844);
        ofColor col_win_popup = ofColor::fromHex(0x77c4d3);
        ofColor col_win_backg = ofColor::fromHex(0x21232b);
        
        ImGuiStyle* style = &ImGui::GetStyle();

        // Prefer to use the original imgui order for easier syncing new style variables
        // Commented ones can be customized

        // LAYOUT
        style->Alpha                       = 1.0f;  // Global alpha applies to everything in Dear ImGui.
        style->WindowPadding               = ImVec2(8, 8);  // Padding within a window.
        style->WindowRounding              = 0.0f;  // Radius of window corners rounding. Set to 0.0f to have rectangular windows. Large values tend to lead to variety of artifacts and are not recommended.
        style->WindowBorderSize            = 0.0f;  // Thickness of border around windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        style->WindowMinSize               = ImVec2(160, 65);  // Minimum window size. This is a global setting. If you want to constraint individual windows, use SetNextWindowSizeConstraints().
        //style->WindowTitleAlign            = ;  // Alignment for title bar text. Defaults to (0.0f,0.5f) for left-aligned,vertically centered.
        //style->WindowMenuButtonPosition    = ;  // Side of the collapsing/docking button in the title bar (None/Left/Right). Defaults to ImGuiDir_Left.
        //style->ChildRounding               = ;  // Radius of child window corners rounding. Set to 0.0f to have rectangular windows.
        style->ChildBorderSize             = 0.0f;  // Thickness of border around child windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        style->PopupRounding               = 0.0f;  // Radius of popup window corners rounding. (Note that tooltip windows use WindowRounding)
        style->PopupBorderSize             = 0.0f;  // Thickness of border around popup/tooltip windows. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        style->FramePadding                = ImVec2(4, 2);  // Padding within a framed rectangle (used by most widgets).
        style->FrameRounding               = 0.0f;  // Radius of frame corners rounding. Set to 0.0f to have rectangular frame (used by most widgets).
        style->FrameBorderSize             = 0.0f;  // Thickness of border around frames. Generally set to 0.0f or 1.0f. (Other values are not well tested and more CPU/GPU costly).
        style->ItemSpacing                 = ImVec2(6, 2);  // Horizontal and vertical spacing between widgets/lines.
        style->ItemInnerSpacing            = ImVec2(2, 4);  // Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label).
        //style->CellPadding                 = ;  // Padding within a table cell
        //style->TouchExtraPadding           = ;  // Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
        style->IndentSpacing               = 6.0f;  // Horizontal indentation when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
        style->ColumnsMinSpacing           = 50.0f;  // Minimum horizontal spacing between two columns. Preferably > (FramePadding.x + 1).
        style->ScrollbarSize               = 14.0f;  // Width of the vertical scrollbar, Height of the horizontal scrollbar.
        style->ScrollbarRounding           = 0.0f;  // Radius of grab corners for scrollbar.
        style->GrabMinSize                 = 12.0f;  // Minimum width/height of a grab box for slider/scrollbar.
        style->GrabRounding                = 0.0f;  // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
        //style->LogSliderDeadzone           = ;  // The size in pixels of the dead-zone around zero on logarithmic sliders that cross zero.
        style->TabRounding                 = 0.0f;  // Radius of upper corners of a tab. Set to 0.0f to have rectangular tabs.
        style->TabBorderSize               = 0.0f;  // Thickness of border around tabs.
        //style->TabMinWidthForCloseButton   = ;  // Minimum width for close button to appears on an unselected tab when hovered. Set to 0.0f to always show when hovering, set to FLT_MAX to never show close button unless selected.
        //style->ColorButtonPosition         = ;  // Side of the color button in the ColorEdit4 widget (left/right). Defaults to ImGuiDir_Right.
        //style->ButtonTextAlign             = ;  // Alignment of button text when button is larger than text. Defaults to (0.5f, 0.5f) (centered).
        //style->SelectableTextAlign         = ;  // Alignment of selectable text. Defaults to (0.0f, 0.0f) (top-left aligned). It's generally important to keep this left-aligned if you want to lay multiple items on a same line.
        //style->DisplayWindowPadding        = ;  // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
        //style->DisplaySafeAreaPadding      = ;  // If you cannot see the edges of your screen (e.g. on a TV) increase the safe area padding. Apply to popups/tooltips as well regular windows. NB: Prefer configuring your TV sets correctly!
        //style->MouseCursorScale            = ;  // Scale software rendered mouse cursor (when io.MouseDrawCursor is enabled). We apply per-monitor DPI scaling over this scale. May be removed later.
        //style->AntiAliasedLines            = ;  // Enable anti-aliased lines/borders. Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
        //style->AntiAliasedLinesUseTex      = ;  // Enable anti-aliased lines/borders using textures where possible. Require backend to render with bilinear filtering. Latched at the beginning of the frame (copied to ImDrawList).
        //style->AntiAliasedFill             = ;  // Enable anti-aliased edges around filled shapes (rounded rectangles, circles, etc.). Disable if you are really tight on CPU/GPU. Latched at the beginning of the frame (copied to ImDrawList).
        //style->CurveTessellationTol        = ;  // Tessellation tolerance when using PathBezierCurveTo() without a specific number of segments. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.
        //style->CircleTessellationMaxError  = ;  // Maximum error (in pixels) allowed when using AddCircle()/AddCircleFilled() or drawing rounded corner rectangles with no explicit segment count specified. Decrease for higher quality but more geometry.

        // COLORS
        style->Colors[ImGuiCol_Text]                    = ImVec4(col_main_text, 1.00f);
        style->Colors[ImGuiCol_TextDisabled]            = ImVec4(col_main_text, 0.58f);
        style->Colors[ImGuiCol_WindowBg]                = ImVec4(col_win_backg, 0.70f);
        style->Colors[ImGuiCol_ChildBg]                 = ImVec4(col_main_area, 0.58f);
        style->Colors[ImGuiCol_PopupBg]                 = ImVec4(col_win_backg, 0.92f);
        style->Colors[ImGuiCol_Border]                  = ImVec4(col_win_backg, 0.00f);
        style->Colors[ImGuiCol_BorderShadow]            = ImVec4(col_win_backg, 0.00f);
        style->Colors[ImGuiCol_FrameBg]                 = ImVec4(col_main_area, 1.00f);
        style->Colors[ImGuiCol_FrameBgHovered]          = ImVec4(col_main_head, 0.78f);
        style->Colors[ImGuiCol_FrameBgActive]           = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_TitleBg]                 = ImVec4(col_main_area, 1.00f);
        style->Colors[ImGuiCol_TitleBgCollapsed]        = ImVec4(col_main_area, 0.75f);
        style->Colors[ImGuiCol_TitleBgActive]           = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_MenuBarBg]               = ImVec4(col_main_area, 0.47f);
        style->Colors[ImGuiCol_ScrollbarBg]             = ImVec4(col_main_area, 1.00f);
        style->Colors[ImGuiCol_ScrollbarGrab]           = ImVec4(col_win_popup, 0.21f);
        style->Colors[ImGuiCol_ScrollbarGrabHovered]    = ImVec4(col_main_head, 0.78f);
        style->Colors[ImGuiCol_ScrollbarGrabActive]     = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_CheckMark]               = ImVec4(col_main_head, 0.80f);
        style->Colors[ImGuiCol_SliderGrab]              = ImVec4(col_win_popup, 0.14f);
        style->Colors[ImGuiCol_SliderGrabActive]        = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_Button]                  = ImVec4(col_win_popup, 0.14f);
        style->Colors[ImGuiCol_ButtonHovered]           = ImVec4(col_main_head, 0.86f);
        style->Colors[ImGuiCol_ButtonActive]            = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_Header]                  = ImVec4(col_main_head, 0.76f);
        style->Colors[ImGuiCol_HeaderHovered]           = ImVec4(col_main_head, 0.86f);
        style->Colors[ImGuiCol_HeaderActive]            = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_Separator]               = ImVec4(col_win_popup, 0.32f);
        style->Colors[ImGuiCol_SeparatorHovered]        = ImVec4(col_main_head, 0.78f);
        style->Colors[ImGuiCol_SeparatorActive]         = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_ResizeGrip]              = ImVec4(col_win_popup, 0.04f);
        style->Colors[ImGuiCol_ResizeGripHovered]       = ImVec4(col_main_head, 0.78f);
        style->Colors[ImGuiCol_ResizeGripActive]        = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_Tab]                     = ImVec4(col_main_area, 0.9f); // ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f)
		style->Colors[ImGuiCol_TabHovered]              = ImVec4(col_main_head, 0.5f);
		style->Colors[ImGuiCol_TabSelected]             = ImVec4(col_main_head, 0.7f);
        style->Colors[ImGuiCol_TabDimmed]               = ImLerp(style->Colors[ImGuiCol_Tab],       style->Colors[ImGuiCol_TitleBg], 0.80f);
        style->Colors[ImGuiCol_TabDimmedSelected]       = ImLerp(style->Colors[ImGuiCol_TabSelected], style->Colors[ImGuiCol_TitleBg], 0.40f);
        style->Colors[ImGuiCol_DockingPreview]          = style->Colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
        style->Colors[ImGuiCol_DockingEmptyBg]          = ImVec4(col_win_backg, 0.8f);
        style->Colors[ImGuiCol_PlotLines]               = ImVec4(col_main_text, 0.63f);
        style->Colors[ImGuiCol_PlotLinesHovered]        = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_PlotHistogram]           = ImVec4(col_main_text, 0.63f);
        style->Colors[ImGuiCol_PlotHistogramHovered]    = ImVec4(col_main_head, 1.00f);
        style->Colors[ImGuiCol_TableHeaderBg]           = ImVec4(col_main_area, 0.76f);
        style->Colors[ImGuiCol_TableBorderStrong]       = ImVec4(col_main_area, 1.f);
        style->Colors[ImGuiCol_TableBorderLight]        = ImVec4(col_main_area, 1.f);
        style->Colors[ImGuiCol_TableRowBg]              = ImVec4(col_main_area, 0.f);
        style->Colors[ImGuiCol_TableRowBgAlt]           = ImVec4(col_main_area, 0.06f);
        style->Colors[ImGuiCol_TextSelectedBg]          = ImVec4(col_main_head, 0.43f);
        style->Colors[ImGuiCol_DragDropTarget]          = ImVec4(col_main_head, 0.8f);
        style->Colors[ImGuiCol_NavCursor]               = ImVec4(col_main_head, 1.f);
        style->Colors[ImGuiCol_NavWindowingHighlight]   = ImVec4(col_main_head, 0.f);
        style->Colors[ImGuiCol_NavWindowingDimBg]       = ImVec4(col_main_area, 0.73f);
        style->Colors[ImGuiCol_ModalWindowDimBg]        = ImVec4(col_main_area, 0.73f);
    }
}

