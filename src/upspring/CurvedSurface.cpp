#include "EditorIncl.h"
#include "EditorDef.h"

#include "Model.h"
#include "CurvedSurface.h"
#include "DebugTrace.h"

#include <GL/glew.h>
#include <GL/gl.h>

using namespace csurf;

Object::Object()
{
}

Object::~Object()
{
	for (unsigned int a=0;a<edges.size();a++)
		delete edges[a];
	for (unsigned int a=0;a<faces.size();a++)
		delete faces[a];
}

void Object::GenerateFromPolyMesh(PolyMesh *o)
{
	std::vector<int> old2new;
	std::vector<Vector3> vertPos;

	GenerateUniqueVectors(o->verts, vertPos, old2new);

	vertices.resize(o->verts.size());
	copy(o->verts.begin(),o->verts.end(),vertices.begin());

	std::map<Poly*, Face*> poly2face;

	// maps edge to first vertex index
	std::map<int, std::vector<Edge*> > edgeMap;

	// simple definition: intersecting edges are edges with the same vertex pair
	// use o->poly, because the edges from non-curved polygons are needed as well
	for (uint a=0;a<o->poly.size();a++) {
		Poly* pl = o->poly[a];

		Face *f = new Face;
		poly2face[pl] = f;
		faces.push_back(f);
		f->plane = pl->CalcPlane(o->verts);

		for (uint e=0;e<pl->verts.size();e++) {
			Edge* edge = new Edge;

			edge->meshVerts[0] = pl->verts[e];
			edge->meshVerts[1] = pl->verts[(e+1)%pl->verts.size()];
			for(int x=0;x<2;x++) edge->pos [x] = old2new[edge->meshVerts[x]];
			edge->face = f;
			edge->dir = o->verts[edge->meshVerts[1]].pos - o->verts[edge->meshVerts[0]].pos;

			edges.push_back(edge);
			f->edges.push_back(edge);

			edgeMap[edge->pos[0]].push_back (edge);
		}
	}

	for (uint a=0;a<edges.size();a++) {
		Edge *edge = edges[a];

		// parallel edge with same direction
		std::vector<Edge*>& v = edgeMap[edge->pos[0]];
		for (uint b=0;b<v.size();b++) {
			if (v[b] != edge && v[b]->pos[1] == edge->pos[1]) {
				d_trace("Matching parallel edge (%d, %d) with (%d, %d)\n", v[b]->pos[0], v[b]->pos[1], edge->pos[0], edge->pos[1]);
				edge->intersecting.push_back(v[b]);
			}
		}
		// opposite direction
		std::vector<Edge*>& w = edgeMap[edge->pos[1]];
		for (uint b=0;b<w.size();b++) {
			if (w[b]->pos[1] == edge->pos[0] && w[b] != edge) {
				d_trace("Matching opposite edge (%d, %d) with (%d, %d)\n", w[b]->pos[0], w[b]->pos[1], edge->pos[0], edge->pos[1]);
				edge->intersecting.push_back(w[b]);
			}
		}
	}
	edgeMap.clear();

	// calculate edge normals
	for (uint a=0;a<edges.size();a++) {
		Edge* e = edges[a];

		e->normal = e->face->plane.GetVector();
		for (uint b=0;b<e->intersecting.size();b++)
			e->normal += e->intersecting[b]->face->plane.GetVector();

		e->normal.normalize();
	}

	std::vector<Vector3> tmpvrt;

	const int steps=10;

	int numCurvedPoly = 0;
	for (std::map<Poly*,Face*>::iterator pit=poly2face.begin();pit!=poly2face.end();++pit)
		if (pit->first->verts.size() == 4) numCurvedPoly ++;

	indexBuffer.Init(sizeof(uint) * 3 * 2 * (steps-1) * (steps-1) * numCurvedPoly);
	vertexBuffer.Init(sizeof(Vector3) * steps * steps * numCurvedPoly);

	uint *indexData = (uint*)indexBuffer.LockData();
	Vector3* vertexData = (Vector3*)vertexBuffer.LockData();
	Vector3* curPos = vertexData;
	uint* curIndex = indexData;

	uint vertexOffset = 0;

	for (std::map<Poly*,Face*>::iterator pit=poly2face.begin();pit!=poly2face.end();++pit)
	{
		Poly* pl = pit->first;
		Face* face = pit->second;

		if (pl->verts.size() == 4) {
			const float step = 1.0f / (float)(steps-1);

			/*
			Edge* Xedge = face->edges[0];
			Edge* Yedges[2] = { face->edges[1], face->edges[3] };
			*/

//			const Vector3& start = vertices[face->edges[0]->meshVerts[0]].pos;
			const Vector3& leftEdge = face->edges[3]->dir;
			const Vector3& rightEdge = face->edges[1]->dir;

			for (int yp=0;yp<steps;yp++) {
				float y = yp * step;
				Vector3 rowStart = vertices[face->edges[0]->meshVerts[0]].pos - leftEdge * y;
				Vector3 rowEnd = vertices[face->edges[0]->meshVerts[1]].pos + rightEdge * y;

				Vector3 row = rowEnd-rowStart;

				for (int xp=0;xp<steps;xp++) {
					float x = xp * step;
					*(curPos++) = rowStart + row * x + face->plane.GetVector() * 0.2f;
				}
			}

			for (int y=1;y<steps;y++)
				for (int x=1;x<steps;x++)
				{
					*(curIndex++) = vertexOffset + (y-1)*steps + (x-1);
					*(curIndex++) = vertexOffset + (y-1)*steps + x;
					*(curIndex++) = vertexOffset + y*steps + x;

					*(curIndex++) = vertexOffset + (y-1)*steps + (x-1);
					*(curIndex++) = vertexOffset + y*steps + x;
					*(curIndex++) = vertexOffset + y*steps + (x-1);
				}

			vertexOffset += steps * steps;
		}
	}
	d_trace("Numtris: %d, NumVerts: %d, VertexOffset=%d\n", curIndex - indexData, curPos - vertexData, vertexOffset);
	indexBuffer.UnlockData ();
	vertexBuffer.UnlockData ();
}

void Object::DrawBuffers()
{
	Vector3 *vbuf = (Vector3 *)vertexBuffer.Bind();
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer (3, GL_FLOAT, sizeof(Vector3), vbuf);

	glDrawElements (GL_TRIANGLES, indexBuffer.GetByteSize()/sizeof(uint), GL_UNSIGNED_INT, indexBuffer.Bind());
	indexBuffer.Unbind ();

	glDisableClientState(GL_VERTEX_ARRAY);
	vertexBuffer.Unbind();
}

void Object::Draw()
{
	glColor3ub(255,255,255);
	DrawBuffers();

	glColor3ub(255,255,255);
	glBegin(GL_LINES);
	for (unsigned int a=0;a<edges.size();a++)
	{
		Edge* edge = edges[a];
		
		Vector3 ep[2];
		for (int x=0;x<2;x++)
			ep[x] = Vector3(vertices[edge->meshVerts[x]].pos + edge->normal * 0.5f);
		
		Vector3 mid = (ep[0] + ep[1]) * 0.5f;

		glVertex3fv(ep[0].getf());
		glVertex3fv(ep[1].getf());

		for (unsigned int l=0;l<edge->intersecting.size();l++) {
			Edge *ie = edge->intersecting[l];

			Vector3 iep[2];
			for (int x=0;x<2;x++)
				iep[x] = Vector3(vertices[ie->meshVerts[x]].pos + ie->normal * 0.5f);
			
			Vector3 iemid = (iep[0] + iep[1]) * 0.5f;

			glColor3ub(255, 255, 0);
			glVertex3fv(mid.getf());
			glColor3ub(255, 120, 0);
			glVertex3fv(iemid.getf());
		}
		glColor3ub(255,255,255);
	}
	glEnd();
}

