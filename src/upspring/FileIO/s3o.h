//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------
#ifndef s3oH
#define s3oH

struct S3OPiece{
	uint name;		//offset in file to char* name of this piece
	uint numChilds;		//number of sub pieces this piece has
	uint childs;		//file offset to table of dwords containing offsets to child pieces
	uint numVertices;	//number of vertices in this piece
	uint vertices;		//file offset to vertices in this piece
	uint vertexType;	//0 for now
	uint primitiveType;	//type of primitives for this piece, 0=triangles,1 triangle strips,2=quads
	uint vertexTableSize;	//number of indexes in vertice table
	uint vertexTable;	//file offset to vertice table, vertice table is made up of dwords indicating vertices for this piece, to indicate end of a triangle strip use 0xffffffff
	uint collisionData;	//offset in file to collision data, must be 0 for now (no collision data)
	float xoffset;		//offset from parent piece
	float yoffset;
	float zoffset;
};

struct S3OVertex{
	float xpos;		//position of vertex relative piece origin
	float ypos;
	float zpos;
	float xnormal;		//normal of vertex relative piece rotation
	float ynormal;
	float znormal;
	float texu;		//texture offset for vertex
	float texv;
};

struct S3OHeader{
	char magic[12];		//"Spring unit\0"
	uint version;		//0 for this version
	float radius;		//radius of collision sphere
	float height;		//height of whole object
	float midx;		//these give the offset from origin(which is supposed to lay in the ground plane) to the middle of the unit collision sphere
	float midy;
	float midz;
	uint rootPiece;		//offset in file to root piece
	uint collisionData;	//offset in file to collision data, must be 0 for now (no collision data)
	uint texture1;		//offset in file to char* filename of first texture
	uint texture2;		//offset in file to char* filename of second texture
};

#endif