project(fltk2)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-Wno-unused-parameter -Wno-unused-label -fno-strict-aliasing -Wno-deprecated -Wall -DUSE_IK -Wint-to-pointer-cast")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-Wdelete-non-virtual-dtor -Wno-unused-parameter -Wno-unused-label -fno-strict-aliasing -Wno-deprecated -Wall -DUSE_IK -fpermissive -Wint-to-pointer-cast")

set(FLTK_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/Fl_Gl_Choice.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/Fl_Gl_Overlay.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/Fl_Gl_Window.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/gl_draw.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGL/gl_start.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/CodeEditor.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/Enumeration.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/FluidType.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/Fluid_Image.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/Fluid_Plugins.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/FunctionType.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/GroupType.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/MenuType.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/WidgetClassType.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/WidgetType.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/WindowType.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/about_panel.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/align_widget.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/alignment_panel.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/code.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/coding_style.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/coding_style_func.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/factory.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/file.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/fluid.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/fluid_img.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/fluid_menus.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/function_panel.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/image_file_panel.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/template_panel.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/undo.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/fluid/widget_panel.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/Fl_Chart.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/Fl_Positioner.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/colbrowser.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/fl_load_browser.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/forms.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/forms_bitmap.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/forms_compatability.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/forms_free.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/forms_fselect.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/forms_pixmap.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/forms/forms_timer.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/glut/fractals.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/glut/glpuzzle.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/glut/glut_compatability.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/glut/glut_font.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/FileIcon2.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/Fl_Guess_Image.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/HelpDialog.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/fl_jpeg.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/fl_png.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/images_core.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/pnmImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/images/xpmFileImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dump_compose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scandir.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vsnprintf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/fromdos.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/trackball.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Adjuster.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlignGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AnsiWidget.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BarGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Browser.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Browser_load.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Button.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CheckButton.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Choice.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Clock.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Color.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ComboBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Cursor.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CycleButton.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Dial.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DiamondBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/EngravedLabel.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileBrowser.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileChooser.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileChooser2.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileIcon.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileInput.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/Fl_Menu_Item.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FloatInput.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Font.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GSave.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Group.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HelpView.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HighlightButton.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Image.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Input.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputBrowser.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InvisibleWidget.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Item.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LightButton.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Menu.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MenuBar.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MenuWindow.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Menu_add.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Menu_global.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Menu_popup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MultiImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NumericInput.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Output.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OvalBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PackedGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlasticBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PopupMenu.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Preferences.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ProgressBar.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RadioButton.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RepeatButton.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ReturnButton.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RoundBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RoundedBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ScrollGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Scrollbar.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ShadowBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ShapedWindow.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SharedImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ShortcutAssignment.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Slider.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StatusBarGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StringList.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Style.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/StyleSet.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Symbol.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemMenuBar.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TabGroup2.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextBuffer.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextDisplay.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TextEditor.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ThumbWheel.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TiledGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TiledImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Tooltip.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UpBox.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Valuator.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ValueInput.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ValueOutput.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ValueSlider.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Widget.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WidgetAssociation.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Widget_draw.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window_fullscreen.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window_hotspot.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Window_iconize.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WizardGroup.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/add_idle.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/addarc.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/addcurve.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/args.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/bmpImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clip.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/color_chooser.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/colormap.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/compose.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/default_glyph.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dnd.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/drawtext.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/error.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/event_key_state.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/file_chooser.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filename_absolute.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filename_ext.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filename_isdir.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filename_list.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filename_match.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/filename_name.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fillrect.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fltk_theme.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gifImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/key_name.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/list_fonts.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/load_plugin.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lock.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/message.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/numericsort.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/overlay_rect.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/own_colormap.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/path.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/readimage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/run.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scrollrect.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/setcolor.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/setdisplay.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/setvisual.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/show_colormap.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/xbmImage.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/xpmImage.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/Alternative.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/KDE.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/KDE_colors.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/classic.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/essai.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/motif.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/none.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/scheme.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/windows.cxx
    # ${CMAKE_CURRENT_SOURCE_DIR}/themes/xft.cxx
)

if (UNIX AND NOT APPLE)
    # set(FLTK_SOURCES
    #     ${FLTK_SOURCES}
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/Font.cxx
    #     # ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/Font_xft.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/Font_xlfd.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/Image.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/dnd.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/event_key_state.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/find_overlay_visual.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/fltk_theme.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/list_fonts.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/list_fonts_xlfd.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/lock.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/readimage.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/run.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/setcolor.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/x11/xutf8.cxx
    # )
elseif (APPLE)
    set(FLTK_SOURCES 
        ${FLTK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dlload_osx.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/Font.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/Image.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/clip.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/dnd.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/event_key_state.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/list_fonts.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/mactoutf8.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/readimage.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/run.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/setcolor.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/osx/utf8tomac.cxx
    )
elseif (WIN32)
    set(FLTK_SOURCES 
        ${FLTK_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/scandir.c
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Font.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/Image.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/WinMain.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/dnd.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/event_key_state.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/fltk_theme.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/list_fonts.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/lock.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/readimage.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/run.cxx
    #     ${CMAKE_CURRENT_SOURCE_DIR}/src/win32/setcolor.cxx
    )
endif ()

add_library(fltk2 STATIC ${FLTK_SOURCES})
target_include_directories(fltk2
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_directories(fltk2 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

if (WIN32)
target_compile_definitions(fltk2 PUBLIC HAVE_SCANDIR=0 HAVE_SCANDIR_POSIX=0 HAVE_DIRENT_H=1)
    target_link_libraries(fltk2 wsock32 ws2_32 msimg32 pthread)
elseif (UNIX)
    target_compile_definitions(fltk2 PUBLIC HAVE_SCANDIR=1 HAVE_SCANDIR_POSIX=1 HAVE_DIRENT_H=1)

    find_package(X11 COMPONENTS Xft Xinerama Xcursor Xi Xext)

    target_link_libraries (fltk2 
        PUBLIC ${X11_LIBRARIES} ${X11_Xft_LIB} ${X11_Xinerama_LIB} ${X11_Xcursor_LIB} ${X11_Xi_LIB}
    )
endif()