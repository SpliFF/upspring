#include "EditorIncl.h"
#include "EditorDef.h"

#include "Model.h"
#include "Util.h"


// ------------------------------------------------------------------------------------------------
// Polygon
// ------------------------------------------------------------------------------------------------

float Poly::Selector::Score(Vector3 &pos, float camdis)
{
	assert (mesh);
	const vector<Vertex>& v=mesh->verts;
	Plane plane;

	Vector3 vrt[3];
	for (int a=0;a<3;a++) 
		transform.apply(&v[poly->verts[a]].pos, &vrt[a]);
    
	plane.MakePlane (vrt[0],vrt[1],vrt[2]);
	float dis = plane.Dis (&pos);
	return fabs (dis);
}
void Poly::Selector::Toggle (Vector3 &pos, bool bSel) {
	poly->isSelected = bSel;
}
bool Poly::Selector::IsSelected () { 
	return poly->isSelected; 
}

Poly::Poly() {
	selector=new Selector (this);
	isSelected=false;
	texture = 0;
	color.set(1,1,1);
	taColor=-1;
	isCurved = false;
}

Poly::~Poly() {
	SAFE_DELETE(selector);
}

Poly* Poly::Clone()
{
	Poly *pl = new Poly;
	pl->verts=verts;
	pl->texname=texname;
	pl->color= color;
	pl->taColor = taColor;
	pl->texture = texture;
	pl->isCurved = isCurved;
	return pl;
}

void Poly::Flip()
{
	vector<int> nv;
	nv.resize(verts.size());
	for (int a=0;a<verts.size();a++)
		nv[verts.size()-a-1]=verts[(a+2)%verts.size()];
	verts=nv;
}

Plane Poly::CalcPlane (const vector<Vertex>& vrt) {
	Plane plane;
	plane.MakePlane (vrt[verts[0]].pos,vrt[verts[1]].pos,vrt[verts[2]].pos);
	return plane;
}

void Poly::RotateVerts ()
{
	vector<int> n(verts.size());
	for (int a=0;a<verts.size();a++)
		n[a]=verts[(a+1)%n.size()];
	verts=n;
}



// ------------------------------------------------------------------------------------------------
// PolyMesh
// ------------------------------------------------------------------------------------------------


PolyMesh::~PolyMesh()
{
	for(int a=0;a<poly.size();a++) 
		if (poly[a]) delete poly[a]; 
	poly.clear();
}

// Special case... polymesh drawing is done in the ModelDrawer
void PolyMesh::Draw(ModelDrawer* drawer, Model *mdl, MdlObject *o)
{}

PolyMesh* PolyMesh::ToPolyMesh()
{
	return (PolyMesh*)Clone();
}

Geometry* PolyMesh::Clone()
{
	PolyMesh *cp = new PolyMesh;

	cp->verts=verts;

	for (int a=0;a<poly.size();a++) 
		cp->poly.push_back(poly[a]->Clone());

	return cp;
}

void PolyMesh::Transform(const Matrix& transform)
{
	Matrix normalTransform, invTransform;
	transform.inverse(invTransform);
	invTransform.transpose(&normalTransform);

	// transform and add the child vertices to the parent vertices list
	for (int a=0;a<verts.size();a++) 
	{
		Vertex&v = verts[a];
		Vector3 tpos, tnormal;
		
		transform.apply (&v.pos, &tpos);
		v.pos = tpos;
		normalTransform.apply (&v.normal, &tnormal);
		v.normal = tnormal;
	}
}


bool PolyMesh::IsEqualVertexTC (Vertex& a,Vertex& b)
{		 
	return a.pos.epsilon_compare (&b.pos, 0.001f) && 
		a.tc[0].x == b.tc[0].x && a.tc[0].y == b.tc[0].y;
}

bool PolyMesh::IsEqualVertexTCNormal (Vertex& a, Vertex& b)
{
	return a.pos.epsilon_compare (&b.pos, 0.001f) && 
		a.tc[0].x == b.tc[0].x && a.tc[0].y == b.tc[0].y &&
		a.normal.epsilon_compare (&b.normal, 0.01f);
}



void PolyMesh::OptimizeVertices (PolyMesh::IsEqualVertexCB cb)
{
	vector <int> old2new;
	vector <int> usage;
	vector <Vertex> nv;

	old2new.resize(verts.size());
	usage.resize (verts.size());
	fill(usage.begin(),usage.end(),0);

	for (int a=0;a<poly.size();a++) 
	{
		Poly *pl=poly[a];
		for (int b=0;b<pl->verts.size();b++) 
			usage[pl->verts[b]]++;
	}

	for (int a=0;a<verts.size();a++) {
		bool matched=false;

		if (!usage[a])
			continue;

		for (int b=0;b<nv.size();b++) {
			Vertex *va = &verts[a];
			Vertex *vb = &nv[b];

			if (cb(*va, *vb)) {
				matched=true;
				old2new[a] = b;
				break;
			}
		}

		if (!matched) {
			old2new[a] = nv.size();
			nv.push_back (verts[a]);
		}
	}

	verts = nv;

	// map the poly vertex-indices to the new set of vertices
	for (int a=0;a<poly.size();a++) 
	{
		Poly *pl=poly[a];
		for (int b=0;b<pl->verts.size();b++) 
			pl->verts[b] = old2new[pl->verts[b]];
	}
}

void PolyMesh::Optimize (PolyMesh::IsEqualVertexCB cb)
{
	OptimizeVertices(cb);

	// remove double linked vertices
	vector<Poly*> npl;
	for (int a=0;a<poly.size();a++) {
		Poly *pl=poly[a];

		bool finished;
		do {
			finished=true;
			for (int i=0,j=(int)pl->verts.size()-1;i<pl->verts.size();j=i++) 
				if (pl->verts[i] == pl->verts[j]) {
					pl->verts.erase (pl->verts.begin()+i);
					finished=false;
					break;
				}
		} while (!finished);

		if (pl->verts.size()>=3)
			npl.push_back(pl);
		else
			delete pl;
	}
	poly=npl;
}


void PolyMesh::CalculateRadius(float& radius, const Matrix &tr, const Vector3& mid)
{
	for(int v=0;v<verts.size();v++)
	{
		Vector3 tpos;
		tr.apply(&verts[v].pos,&tpos);
		float r= (tpos-mid).length();
		if (radius < r) radius=r;
	}
}


vector<Triangle> PolyMesh::MakeTris ()
{
	vector<Triangle> tris;

	for (int a=0;a<poly.size();a++) {
		Poly *p = poly[a];
		for (int b=2;b<p->verts.size();b++)
		{
			Triangle t;

			t.vrt[0] = p->verts[0];
			t.vrt[1] = p->verts[b-1];
			t.vrt[2] = p->verts[b];

			tris.push_back (t);
		}
	}
	return tris;
}


void GenerateUniqueVectors(const std::vector<Vertex>& verts, 
						   std::vector<Vector3>& vertPos, 
						   std::vector<int>& old2new)
{
	old2new.resize(verts.size());

	for (int a=0;a<verts.size();a++) {
		bool matched=false;

		for (int b=0;b<vertPos.size();b++) 
			if (vertPos[b] == verts[a].pos) {
				old2new[a] = b;
				matched=true;
				break;
			}
		if (!matched) {
			old2new[a] = vertPos.size();
			vertPos.push_back (verts[a].pos);
		}
	}
}

	
struct FaceVert {
	std::vector <int> adjacentFaces;
};

void PolyMesh::CalculateNormals2(float maxSmoothAngle)
{
	float ang_c = cosf (M_PI * maxSmoothAngle / 180.0f);
	vector<Vector3> vertPos;
	vector<int> old2new;
	GenerateUniqueVectors(verts, vertPos, old2new);

	vector<vector<int> > new2old;
	new2old.resize(vertPos.size());
	for (int a=0;a<old2new.size();a++)
		new2old[old2new[a]].push_back(a);

	// Calculate planes
	std::vector <Plane> polyPlanes;
	polyPlanes.resize (poly.size());
	for (int a=0;a<poly.size();a++)
		polyPlanes[a] = poly[a]->CalcPlane (verts);

	// Determine which faces are using which unique vertex
	std::vector <FaceVert> faceVerts; // one per unique vertex
	faceVerts.resize (old2new.size ());

	for (int a=0;a<poly.size();a++) {
		Poly *pl = poly[a];
		for (int v=0;v<pl->verts.size();v++)
			faceVerts[old2new[pl->verts[v]]].adjacentFaces.push_back (a);
	}

	// Calculate normals
	int cnorm = 0;
	for (int a=0;a<poly.size();a++) cnorm += poly[a]->verts.size();
	std::vector <Vector3> normals;
	normals.resize (cnorm);

	cnorm = 0;
	for (int a=0;a<poly.size();a++) {
		Poly *pl = poly[a];
		for (int v=0;v<pl->verts.size();v++) 
		{
			FaceVert& fv = faceVerts[old2new[pl->verts[v]]];
			std::vector<Vector3> vnormals;
			vnormals.push_back(polyPlanes[a].GetVector());
			for (int adj = 0; adj < fv.adjacentFaces.size(); adj ++)
			{
				// Same poly?
				if (fv.adjacentFaces [adj] == a)
					continue;

				Plane adjPlane = polyPlanes[fv.adjacentFaces[adj]];

				// Spring 3DO style smoothing
				float dot = adjPlane.GetVector ().dot (polyPlanes[a].GetVector());
			//	logger.Print("Dot: %f\n",dot);
				if (dot < ang_c) 
					continue;

				// see if the normal is unique for this vertex
				bool isUnique = true;
				for (int norm = 0; norm < vnormals.size(); norm ++) 
					if (vnormals[norm] == adjPlane.GetVector ()) 
					{
						isUnique = false;
						break;
					}
					
				if (isUnique)
					vnormals.push_back (adjPlane.GetVector());
			}
			Vector3 normal;
			for (int n=0;n<vnormals.size();n++)
				normal += vnormals[n];

			if (normal.length () > 0.0f)
				normal.normalize ();
			normals [cnorm ++] = normal;
		}
	}

	// Create a new set of vertices with the calculated normals
	std::vector <Vertex> newVertices;
	newVertices.reserve(poly.size()*4); // approximate
	cnorm = 0;
	for (int a=0;a<poly.size();a++) {
		Poly *pl = poly[a];
		for (int v=0;v<pl->verts.size();v++) {
			FaceVert &fv = faceVerts[old2new[pl->verts[v]]];
			Vertex nv = verts[pl->verts[v]];
			nv.normal = normals[cnorm++];
			newVertices.push_back (nv);
			pl->verts [v] = newVertices.size () - 1;
		}
	}

	// Optimize
	verts = newVertices;
	Optimize(&PolyMesh::IsEqualVertexTCNormal);
}

// In short, the reason for the complexity of this function is:
//  - creates a list of vertices where every vertex has a unique position (UV ignored)
//  - doesn't allow the same poly normal to be added to the same vertex twice
void PolyMesh::CalculateNormals()
{
	vector<Vector3> vertPos;
	vector<int> old2new;
	GenerateUniqueVectors(verts, vertPos, old2new);
	
	vector<vector<int> > new2old;
	new2old.resize(vertPos.size());
	for (int a=0;a<old2new.size();a++)
		new2old[old2new[a]].push_back(a);

	vector<std::vector<Vector3> > normals;
	normals.resize(vertPos.size());

	for (int a=0;a<poly.size();a++) {
		Poly *pl = poly[a];
		Plane plane;
		
		plane.MakePlane(
			vertPos[old2new[pl->verts[0]]],
			vertPos[old2new[pl->verts[1]]],
			vertPos[old2new[pl->verts[2]]]);

		Vector3 plnorm = plane.GetVector();
		for (int b=0;b<pl->verts.size();b++) {
			vector<Vector3>& norms = normals[old2new[pl->verts[b]]];
			int c;
			for (c=0;c<norms.size();c++)
				if (norms[c] == plnorm) break;

			if (c == norms.size())
				norms.push_back(plnorm);
		}
	}

	for (int a=0;a<normals.size();a++) {
		Vector3 sum;
		vector<Vector3>& vn = normals[a];
		for(int b=0;b<vn.size();b++)
			sum+=vn[b];

		if (sum.length()>0.0f)
			sum.normalize ();

		vector<int>& vlist=new2old[a];
		for(int b=0;b<vlist.size();b++)
			verts[vlist[b]].normal = sum;
	}
}

void PolyMesh::FlipPolygons()
{
	for (int a=0;a<poly.size();a++)
		poly[a]->Flip();
}


void PolyMesh::MoveGeometry (PolyMesh *dst)
{
	// offset the vertex indices and move polygons
	for (int a=0;a<poly.size();a++)
	{
		Poly *pl = poly[a];

		for (int b=0;b<pl->verts.size();b++) 
			pl->verts [b] += (int)dst->verts.size();
	}
	dst->poly.insert (dst->poly.end(), poly.begin(),poly.end());
	poly.clear ();

	// insert the child vertices
	dst->verts.insert (dst->verts.end(),verts.begin(),verts.end());
	verts.clear ();

	InvalidateRenderData();
}
