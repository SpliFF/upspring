items as of January 31, 2010  [SpliFF]
	* Build on 64-bit platforms
	* Show all supported types as default filetype in load/save selection.
	* Try to preserve/scale normals and UV on scaling and rotation
	* Fix any issues with objects "jumping around".
	* Allow Lua metadata file to be exported without saving model
	* Batch convert option for files, directories, archives (sdz/sd7) and maps
	* Command-line operations for batch convert without GUI.
	* Fix texture handling and display. Fix common problems automatically unless disabled in prefs.
	* Create / replace parent object for all childs called 'base' at the top of the tree. You can rename this if you require.
	* Check UpSpring builds on all platforms that can build Spring
	* Fix not saving file extension

	OBJ
	* Import/Export named objects. Hierarchies may still not be possible but your objects won't automatically merge either.
	
	3DS
	* Auto rotate on import unless UpSpring preferences say otherwise
	* Maintain hierarchy if possible (may not be)
	
	Lua
	* Look for <modelname>.lua in objects3d and import values and hierarchy if found
	* Use Lua as 'god' for all values if that value is set.
	* Disable any automatic rotations / fixes if Lua defines these.
	* Option to raw import without Lua data (act like the Lua isn't there)
	* Attempt to preserve Lua comments and source order/formatting.
	* Allow seperate import of Lua (ie, to use a shared Lua or Lua stored outside objects3d/)


items as of March 29, 2009  [Kloot]
	build the NativeFileChooser widget automatically [done]
	use the fltk2-config shellscript for linker flags?
	eliminate all compiler warnings

items as of March 25, 2009 [Kloot]
	set up a build-system (scons, cmake, autotools?)
	set up and test cross-compilation system
	add all needed third-party headers to distr. [done]
	remove the weird mix of FLTK versions [done]
	fix the scaling of normals on resize, etc.
