CC     = g++
CFLAGS = -Wall -Wextra -g -DUSE_IK -O2   -Wno-unused-parameter -Wno-unused-label
LFLAGS = \
	-lz -lIL -lILU -lILUT -lGLEW -lGL          \
	-lfltk2_gl -lfltk2 -lfltk2_images          \
	-llua                                      \
	-lX11 -lXft -lXinerama -l3ds -lboost_regex

MKDIR  = mkdir
TARGET = UpSpring

FLTK2_SRCDIR  = ../fltk2/
LUAJIT_SRCDIR = ../LuaJIT-1.1.2/
LIB3DS_SRCDIR = ../lib3ds-1.2/

SRC_BASE_DIR = ./
OBJ_BASE_DIR = ../obj
BIN_BASE_DIR = ../bin

CREG_SRC_DIR      = $(SRC_BASE_DIR)/creg
CREG_OBJ_DIR      = $(OBJ_BASE_DIR)/creg
FILEIO_SRC_DIR    = $(SRC_BASE_DIR)/FileIO
FILEIO_OBJ_DIR    = $(OBJ_BASE_DIR)/FileIO
FLCHOOSER_SRC_DIR = $(SRC_BASE_DIR)/FLNativeFileChooser
FLCHOOSER_OBJ_DIR = $(OBJ_BASE_DIR)/FLNativeFileChooser
MATH_SRC_DIR      = $(SRC_BASE_DIR)/math
MATH_OBJ_DIR      = $(OBJ_BASE_DIR)/math
SWIG_SRC_DIR      = $(SRC_BASE_DIR)/swig
SWIG_OBJ_DIR      = $(OBJ_BASE_DIR)/swig

IFLAGS_FLTK2 = -I$(SRC_BASE_DIR) -I$(LUAJIT_SRCDIR) -I$(LUAJIT_SRCDIR)/src  -I$(LIB3DS_SRCDIR) -I$(FLTK2_SRCDIR)



CREG_OBS = \
	$(CREG_OBJ_DIR)/creg.o       \
	$(CREG_OBJ_DIR)/Serializer.o \
	$(CREG_OBJ_DIR)/VarTypes.o

FILEIO_OBS = \
	$(FILEIO_OBJ_DIR)/3DO.o          \
	$(FILEIO_OBJ_DIR)/3DS.o          \
	$(FILEIO_OBJ_DIR)/c3o.o          \
	$(FILEIO_OBJ_DIR)/MD5.o          \
	$(FILEIO_OBJ_DIR)/OgreModel.o    \
	$(FILEIO_OBJ_DIR)/S3O.o          \
	$(FILEIO_OBJ_DIR)/WavefrontOBJ.o

FLCHOOSER_OBS = \
	$(FLCHOOSER_OBJ_DIR)/common.o $(FLCHOOSER_OBJ_DIR)/Fl_Native_File_Chooser.o

MATH_OBS = \
	$(MATH_OBJ_DIR)/Mathlib.o

SWIG_OBS = \
	$(SWIG_OBJ_DIR)/upspring_wrap.o

UPSPRING_OBS = \
	$(OBJ_BASE_DIR)/Animation.o       \
	$(OBJ_BASE_DIR)/AnimationUI.o     \
	$(OBJ_BASE_DIR)/AnimTrackEditor.o \
	$(OBJ_BASE_DIR)/BackupManager.o   \
	$(OBJ_BASE_DIR)/BackupViewerUI.o  \
	$(OBJ_BASE_DIR)/CfgParser.o       \
	$(OBJ_BASE_DIR)/CurvedSurface.o   \
	$(OBJ_BASE_DIR)/DebugTrace.o      \
	$(OBJ_BASE_DIR)/Editor.o          \
	$(OBJ_BASE_DIR)/EditorUI.o        \
	$(OBJ_BASE_DIR)/FileDialog.o      \
	$(OBJ_BASE_DIR)/FileSearch.o      \
	$(OBJ_BASE_DIR)/IK.o              \
	$(OBJ_BASE_DIR)/IK_UI.o           \
	$(OBJ_BASE_DIR)/Image.o           \
	$(OBJ_BASE_DIR)/MdlObject.o       \
	$(OBJ_BASE_DIR)/Model.o           \
	$(OBJ_BASE_DIR)/ModelDrawer.o     \
	$(OBJ_BASE_DIR)/nv_dds.o          \
	$(OBJ_BASE_DIR)/ObjectView.o      \
	$(OBJ_BASE_DIR)/pch.o             \
	$(OBJ_BASE_DIR)/PolyMesh.o        \
	$(OBJ_BASE_DIR)/RotatorUI.o       \
	$(OBJ_BASE_DIR)/TexBuilderUI.o    \
	$(OBJ_BASE_DIR)/TexGroupUI.o      \
	$(OBJ_BASE_DIR)/TextureBrowser.o  \
	$(OBJ_BASE_DIR)/Texture.o         \
	$(OBJ_BASE_DIR)/Timeline.o        \
	$(OBJ_BASE_DIR)/Tools.o           \
	$(OBJ_BASE_DIR)/Util.o            \
	$(OBJ_BASE_DIR)/UVMappingUI.o     \
	$(OBJ_BASE_DIR)/VertexBuffer.o    \
	$(OBJ_BASE_DIR)/View.o            \
	$(OBJ_BASE_DIR)/ZipFile.o

OBJECTS =            \
	$(CREG_OBS)      \
	$(FILEIO_OBS)    \
	$(FLCHOOSER_OBS) \
	$(MATH_OBS)      \
	$(SWIG_OBS)      \
	$(UPSPRING_OBS)

all:
	make dirs
	make objects
	make target

objects: $(OBJECTS)



$(CREG_OBJ_DIR)/%.o: $(CREG_SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

$(FILEIO_OBJ_DIR)/%.o: $(FILEIO_SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

$(FLCHOOSER_OBJ_DIR)/%.o: $(FLCHOOSER_SRC_DIR)/%.cxx
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -I$(FLCHOOSER_SRC_DIR)   -o $@    -c $<

$(MATH_OBJ_DIR)/%.o: $(MATH_SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

$(SWIG_OBJ_DIR)/%.o: $(SWIG_SRC_DIR)/%.cxx
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

$(OBJ_BASE_DIR)/%.o: $(SRC_BASE_DIR)/%.cpp
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -I$(FLCHOOSER_SRC_DIR)   -o $@    -c $<



dirs:
	if [ ! -d $(OBJ_BASE_DIR) ];      then $(MKDIR) $(OBJ_BASE_DIR);      fi
	if [ ! -d $(BIN_BASE_DIR) ];      then $(MKDIR) $(BIN_BASE_DIR);      fi
	if [ ! -d $(CREG_OBJ_DIR) ];      then $(MKDIR) $(CREG_OBJ_DIR);      fi
	if [ ! -d $(FILEIO_OBJ_DIR) ];    then $(MKDIR) $(FILEIO_OBJ_DIR);    fi
	if [ ! -d $(FLCHOOSER_OBJ_DIR) ]; then $(MKDIR) $(FLCHOOSER_OBJ_DIR); fi
	if [ ! -d $(MATH_OBJ_DIR) ];      then $(MKDIR) $(MATH_OBJ_DIR);      fi
	if [ ! -d $(SWIG_OBJ_DIR) ];      then $(MKDIR) $(SWIG_OBJ_DIR);      fi

target:
	## FIXME: the extra linker argument should not be needed
	$(CC)   -o $(BIN_BASE_DIR)/$(TARGET)   $(OBJECTS) -lfltk2_images $(LFLAGS)

clean:
	rm -f -r $(OBJ_BASE_DIR)
	rm -f -r $(BIN_BASE_DIR)
