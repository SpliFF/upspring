March 28, 2009 [Kloot]
	quick compilation guide:
		src/fltk1.1.7      configure && make
		src/fltk2          configure && make
		src/lib3ds         configure && make
		src/LuaJIT-1.1.2   make linux

		put the compiled libraries (*.a) in /usr/local/lib/ (or
		modify src/upspring/makefile to look for them elsewhere)
		and then simply cd src/upspring/ && make all

		for now, all temporary object files are created in src/obj/
		and the binary is placed in src/bin/ after linking, but do
		not run UpSpring from that location or it won't be able to
		find certain resources (button textures, etc)

March 25, 2009 [Kloot]
	created a github repository for the 1.54 EA source

	added a short todo-list for the immediate future

	patched Image.cpp with JC's file-locking fix

	moved data.ups, palette.pal, skybox.dds, views.cfg to
	their own directory data/ (and adjusted the source path
	strings accordingly)



original readme:
	Upspring Model Editor (freeware)
	version 1.54 (Euler Angles release)
	---------------------
	By Jelmer Cnossen

	This is a unit model editor that can be used to put TA spring S3O and 3DO units together.
	It supports the 3DO (original Total Annihilation) format, the S3O format, and 3DS format and Wavefront OBJ format.

	Program info
	------------
	Rendering: OpenGL (www.opengl.org)
	Image loading: DevIL (openil.sf.net)
	GUI: FLTK (www.fltk.org)
	zlib: ZIP archive loading
	lib3ds: 3DS file loading/saving
	GLEW: OpenGL extension loading (glew.sf.net)

	Known problems
	--------------
	Redrawing problems with adding/removing views, resize the views a bit to get them correct again.

	Thanks to
	---------
	-Maestro for testing, writing a spring manual(!) and creating the application icon
	-Weaver for testing and creating tool icons
	-The guy that made the armpw model who's name I forgot :O, if you object to this being included as a sample let me know.
	-zwzsg, who helped a lot with debugging and made the TA texture pack
	-Rattle for creating tool icons, and fixing the BOSExporter lua script
	-The rest of the spring mod community for testing... :)

	Keys (should be obvious though)
	-------------------------------

	In viewports:
	Alt   = Force camera tool

	When object tab is enabled:
	Copy   = Ctrl-C
	Cut    = Ctrl-X
	Paste  = Ctrl-V
	Delete = Delete selected polygons or selected objects, depending on which tool is active.

	There are many other keys avaiable, most menu items have a key selected 
	but you can see those next to the menu item name.


	Changes for 1.54
	----------------
	- Texture image handling rewrite, this fixes the reduced
	  image quality of textures in previous versions.
	- Added new mode of normal calculation, which allows a maximum smoothing angle. 

	Changes for 1.53
	----------------
	- Added 3DO to S3O converter. 
	  Colored faces do not work well yet, and DevIL reduces image quality,
	  but it might still be useful if you want to retexture an old 3DO model.

	Changes for 1.52
	----------------
	- Fixed OBJ normal loading (No idea how it got messed up .. )
	- Fixed some crashbugs related to empty objects.

	Changes for 1.51
	----------------
	- Fixed rotating objects in viewport (It switched angles to NaN)

	Changes for 1.5
	----------------
	- Fixed wrong lighting in X-direction for exported S3O models
	- Undo/redo support + History viewer (Called Undo buffer viewer)
	- Lighting unlockable from camera
	- Fixed weird lighting when using untextured solid rendering
	- Fixed "add empty"
	- Fixed and improved "Set S3O texture dir"
	- 3DS smoothing groups are used when 3DS files are imported.

	Changes for 1.44
	----------------
	- Rewritten the sloppy S3O texture handling. Hopefully this will result in less crashes.
	- 3DS support rewritten using lib3ds
	- A lot of internal refactoring of geometry code


	Changes for 1.43
	----------------
	- Objects can be mirrored by entering negative numbers for scale
	- BVH import script added

	Changes for 1.42
	------------------
	- Fixed UV importing
	- Fixed OBJ export, it now exports the full model instead of only the root object
	- Fixed object replacing


	Changes for 1.41
	-----------------
	- Manual-entering method added for moving the origin

	Changes for 1.4
	-----------------
	- Basic animation support, 
	- Lua scripting
	- Origin-move tool
	- Texture names shouldn't be changed for 3DO models, but it still has to be tested
	- 3DO texture rotate
	- Object swapping, can swap any pair of objects including parent/child
	- Rewritten rotation code, using quaternions
	- Rotator dialog, supports relative rotation and absolute rotations
	- Fixed transformation errors in saving

	Changes for 1.3
	-----------------
	- 3DO colors are not messed up anymore. 
	- You can flip and mirror the UVs in the UV viewer window.
	- Black texture bug fixed
	- S3O texture behavior improved: 
		- names of textures will be written in S3O even if they are missing
		- A texture loading directory can be set (You would usually set this to something like c:\spring\unittextures)
	- S3O texture rendering can be changed, so you can view the textures seperately and without teamcolor applied.
	- S3O texture builder, you can convert an RGB texture + grayscale texture into a single RGBA texture.

original source-code readme:
	This is upspring code released under GPL license.
	If you make any modifications or port it or something, I'd appreciate if you send me
	an update for it so I can possibly include it in the standard upspring release.

	All libraries that I used are portable to linux (little endian only), so in general
	it shouldn't be a lot of effort to port it, only small compiler differences.

	Jelmer Cnossen
