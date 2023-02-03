#ifndef CURVED_SURFACE_H
#define CURVED_SURFACE_H

#include "VertexBuffer.h"

// Curved surfaces using cubic hermite splines

struct MdlObject;
class PolyMesh;
struct Vertex;
struct Poly;

namespace csurf {

	struct Edge;

	class Face
	{
	public:
		Face(){}
		~Face() {}

		std::vector<Edge*> edges;
		Plane plane;
	};

	struct Edge
	{
		int pos[2],  // indices into unique position list
			meshVerts[2]; // indices into vertex list
		Vector3 normal; // the edge normal
		Vector3 dir; // v1-v0
		Face *face;
		std::vector<Edge*> intersecting;
	};

	class Object
	{
	public:
		Object();
		~Object();

		std::vector<Edge*> edges;
		std::vector<Face*> faces;
		std::vector<Vertex> vertices;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		void GenerateFromPolyMesh(PolyMesh *o);
		void Draw();
		void DrawBuffers();
	};

};

#endif

