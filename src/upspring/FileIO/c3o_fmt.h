
// Common 3D MdlObject format
#ifndef C3O_FMT_H
#define C3O_FMT_H

#pragma pack(push, 1)

#define C3O_MAGIC_ID (*(unsigned int*)"C3O")
#define C3O_VERSION 1

struct C3O_Header
{
	unsigned int id;
	int version;
	int numChunks;
	int chunkTableOfs;

	int rootObj; // root object chunk
};

// Chunk info
struct C3O_Chunk
{
	unsigned short id;
	unsigned short version;
	int offset;
};

// Chunk IDs
#define C3O_VERTEXTBL	0
#define C3O_POLYGONTBL	1
#define C3O_OBJECT		2
#define C3O_TEXTURE		3 // C3OTEX_* constant saved as char, followed by a zero-term name string

struct C3O_Object
{
	int nameOfs;
	float position[3];  // position in "parent" space
	float scale[3];
	float rotation[3]; // euler rotation (rotation around x axis, y axis and z axis)
	int numChilds;
	int childOfs; // offset to child chunk list
	
	int polyTbl; // polygon table chunk
	int vertexTbl; // vertex table chunk

	int jointInfoChunk; // IK joint chunk, if -1 then it is a fixed joint
};

// Spring type textures
#define C3OTEX_SPRING0	0
#define C3OTEX_SPRING1	1


// IK Joint chunks
#define C3O_HINGEJT		10
#define C3O_UNIVERSALJT	11

struct C3O_HingeJoint
{
	float axis[3];
};

struct C3O_UniversalJoint
{
	float axis1[3];
	float axis2[3];
};

struct C3O_Vertex
{
	float pos[3];
	float normal[3];
	float texcoord[2];
};

#pragma pack(pop)

#endif
