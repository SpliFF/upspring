CC     = g++
CFLAGS = -Wall -Wextra -g -DUSE_IK -O2
LFLAGS = \
	-s                                         \
	-lz -lIL -lILU -lILUT -lGLEW -lGL          \
	-lfltk2_gl -lfltk2                         \
	-lfltk_images -lfltk                       \
	-llua                                      \
	-lX11 -lXft -lXinerama -l3ds -lboost_regex \
	-z muldefs

MKDIR  = mkdir
TARGET = UpSpring

## <fltk1.1.7/FL/>
## <fltk2/fltk/>
FLTK1_SRCDIR  = ../fltk1.1.7/
FLTK2_SRCDIR  = ../fltk2/
LUAJIT_SRCDIR = ../LuaJIT-1.1.2/
LIB3DS_SRCDIR = ../lib3ds-1.2/

## SRC_BASE_DIR = ./upspring
SRC_BASE_DIR = ./
OBJ_BASE_DIR = ../obj
BIN_BASE_DIR = ../bin

CREG_SRC_DIR      = $(SRC_BASE_DIR)/creg
CREG_OBJ_DIR      = $(OBJ_BASE_DIR)/creg
FILEIO_SRC_DIR    = $(SRC_BASE_DIR)/FileIO
FILEIO_OBJ_DIR    = $(OBJ_BASE_DIR)/FileIO
FLCHOOSER_SRC_DIR = $(SRC_BASE_DIR)/Fl_Native_File_Chooser
FLCHOOSER_OBJ_DIR = $(OBJ_BASE_DIR)/Fl_Native_File_Chooser
MATH_SRC_DIR      = $(SRC_BASE_DIR)/math
MATH_OBJ_DIR      = $(OBJ_BASE_DIR)/math
SWIG_SRC_DIR      = $(SRC_BASE_DIR)/swig
SWIG_OBJ_DIR      = $(OBJ_BASE_DIR)/swig

IFLAGS_FLTK1 = -I$(SRC_BASE_DIR) -I$(LUAJIT_SRCDIR) -I$(LUAJIT_SRCDIR)/src  -I$(LIB3DS_SRCDIR) -I$(FLTK1_SRCDIR)
IFLAGS_FLTK2 = -I$(SRC_BASE_DIR) -I$(LUAJIT_SRCDIR) -I$(LUAJIT_SRCDIR)/src  -I$(LIB3DS_SRCDIR) -I$(FLTK2_SRCDIR) -I$(FLTK2_SRCDIR)/fltk/compat/



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
	$(FLCHOOSER_OBJ_DIR)/Fl_Native_File_Chooser.o

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

OBJXCTS =            \
	$(CREG_OBS)      \
	$(FILEIO_OBS)    \
	$(FLCHOOSER_OBS) \
	$(MATH_OBS)      \
	$(SWIG_OBS)      \
	$(UPSPRING_OBS)
OBJECTS = $(UPSPRING_OBS)

objects: $(OBJECTS)



$(CREG_OBJ_DIR)/%.o: $(CREG_SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

$(FILEIO_OBJ_DIR)/%.o: $(FILEIO_SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

$(FLCHOOSER_OBJ_DIR)/%.o: $(FLCHOOSER_SRC_DIR)/%.cxx
	$(CC) $(CFLAGS) $(IFLAGS_FLTK1)   -I$(FLCHOOSER_SRC_DIR)   -o $@    -c $<

$(MATH_OBJ_DIR)/%.o: $(MATH_SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

$(SWIG_OBJ_DIR)/%.o: $(SWIG_SRC_DIR)/%.cxx
	$(CC) $(CFLAGS) $(IFLAGS_FLTK2)   -o $@    -c $<

## works: g++ -Wall -Wextra -g -o ../obj/Animation.o    -c Animation.cpp    -I ./   -I ../LuaJIT-1.1.2/src/
## works: g++ -Wall -Wextra -g -o ../obj/Animation.o    -c Animation.cpp    -I ./   -I"../LuaJIT-1.1.2/src/"
## fails: g++ -Wall -Wextra -g -DUSE_IK -O2 "-I././ -I"../LuaJIT-1.1.2/src/"  -I"../lib3ds-1.2/" -I"../fltk2/" -I"../fltk2/"/fltk/compat/"   -I.//Fl_Native_File_Chooser   -o ../obj/Animation.o    -c Animation.cpp
## fails: g++ -Wall -Wextra -g -DUSE_IK -O2 "-I./  -I"../LuaJIT-1.1.2/" -I"../LuaJIT-1.1.2/"/src  -I"../lib3ds-1.2/" -I"../fltk2/" -I"../fltk2/"/fltk/compat/"   -I.//Fl_Native_File_Chooser   -o ../obj/Animation.o    -c Animation.cpp
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
	$(CC) $(LFLAGS)   -o $(BIN_BASE_DIR)/$(TARGET)    $(OBJECTS)

all:
	make dirs
	make objects
	make target

clean:
	rm -f -r $(OBJ_BASE_DIR)
	rm -f -r $(BIN_BASE_DIR)
