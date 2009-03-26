//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef EDITOR_FLTK_H
#define EDITOR_FLTK_H

#ifdef _MSC_VER
#pragma warning(disable: 4312 4311)
#endif 

#include <fltk/Window.h>
#include <fltk/PackedGroup.h>
#include <fltk/MenuBar.h>
#include <fltk/PopupMenu.h>
#include <fltk/Item.h>
#include <fltk/Divider.h>
#include <fltk/TiledGroup.h>
#include <fltk/TabGroup.h>
#include <fltk/Group.h>
#include <fltk/Output.h>
#include <fltk/FileInput.h>
#include <fltk/BarGroup.h>
#include <fltk/ask.h>
#include <fltk/events.h>
// KLOOTNOTE: "font.h" needs to be "Font.h" on Linux
#include <fltk/Font.h>
#include <fltk/filename.h>
#include <fltk/ScrollGroup.h>
// #include <fltk/file_chooser.h>
// #include <fltk/FileChooser.h>


// Try to undo the effects of evil windoze headers
#ifdef _WINDOWS_
#undef CreateDialog
#undef LoadImage
#undef FreeModule
#endif

#endif
