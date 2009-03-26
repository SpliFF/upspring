
#include "EditorIncl.h"
#include "EditorDef.h"

#include "Model.h"
#include "MeshIterators.h"
#include "CurvedSurface.h"
#include "Util.h"

// ------------------------------------------------------------------------------------------------
// Rotator
// ------------------------------------------------------------------------------------------------

Rotator::Rotator() {
	eulerInterp = false;
}

Vector3 Rotator::GetEuler() 
{/*
	Matrix m;
	q.makematrix(&m);
	return m.calcEulerYXZ();*/
	return euler;
}

void Rotator::SetEuler(Vector3 euler)
{
/*	Matrix m;
	m.eulerYXZ(euler);
	m.makequat(&q);
	q.normalize();*/
	this->euler = euler;
}

void Rotator::SetQuat(Quaternion q)
{
	Matrix m;
	q.makematrix(&m);
	euler = m.calcEulerYXZ();
}

Quaternion Rotator::GetQuat()
{
	Matrix m;
	m.eulerYXZ(euler);
	Quaternion q;
	m.makequat(&q);
	return q;
}

void Rotator::AddEulerAbsolute(const Vector3& rot)
{
	/*
	Matrix m;
	m.eulerYXZ(rot);

	Quaternion tq;
	m.makequat(&tq);

	q *= tq;
	q.normalize();*/

	euler += rot;
}


void Rotator::AddEulerRelative(const Vector3& rot)
{
/*	Matrix m;
	m.eulerYXZ(rot);

	Quaternion tq;
	m.makequat(&tq);

	q = tq * q;
	q.normalize();*/

	euler += rot;
}

void Rotator::ToMatrix(Matrix& o) 
{
//	q.makematrix(&o);
	o.eulerYXZ(euler);
}

void Rotator::FromMatrix(const Matrix&r )
{
	euler = r.calcEulerYXZ();
//	r.makequat(&q);
//	q.normalize();
}


// ------------------------------------------------------------------------------------------------
// MdlObject
// ------------------------------------------------------------------------------------------------

// Is pos contained by this object?
float MdlObject::Selector::Score (Vector3 &pos, float camdis)
{
	// it it close to the center?
	Vector3 center;
	Matrix transform;
	obj->GetFullTransform(transform);
	transform.apply(&Vector3(), &center);
	float best=(pos-center).length();
	// it it close to a polygon?
	for (PolyIterator pi(obj);!pi.End();pi.Next()) {
		pi->selector->mesh = pi.Mesh();
		float polyscore=pi->selector->Score(pos, camdis);
		if (polyscore < best) best=polyscore;
	}
	return best;
}
void MdlObject::Selector::Toggle (Vector3 &pos, bool bSel) { 
	obj->isSelected = bSel; 
}
bool MdlObject::Selector::IsSelected () { 
	return obj->isSelected; 
}


MdlObject::MdlObject ()
{
	selector = new Selector (this);
	isSelected=false;
	isOpen=true;
	parent=0;
	scale.set(1,1,1);
	bTexturesLoaded=false;
	csurfobj = 0;
	geometry = 0;

	InitAnimationInfo ();
}


MdlObject::~MdlObject()
{ 
	delete geometry;

	for(int a=0;a<childs.size();a++)
		if (childs[a]) delete childs[a];
	childs.clear();

	delete selector;
	delete csurfobj;
}

PolyMesh* MdlObject::GetPolyMesh()
{
	return dynamic_cast<PolyMesh*> (geometry);
}

PolyMesh* MdlObject::GetOrCreatePolyMesh()
{
	if (!dynamic_cast<PolyMesh*>(geometry)) 
	{
		delete geometry;
		geometry = new PolyMesh;
	}
	return (PolyMesh*)geometry;
}

void MdlObject::InvalidateRenderData ()
{
	if (geometry)
		geometry->InvalidateRenderData();
}

void MdlObject::RemoveChild(MdlObject *o)
{
	childs.erase(find(childs.begin(),childs.end(),o));
	o->parent = 0;
}

void MdlObject::AddChild(MdlObject *o)
{
	if (o->parent)
		o->parent->RemoveChild(o);

	childs.push_back(o);
	o->parent = this;
}

bool MdlObject::IsEmpty ()
{
	for (int a=0;a<childs.size();a++)
		if (!childs[a]->IsEmpty())
			return false;

	PolyMesh *pm = GetPolyMesh();
	return pm ? pm->poly.empty() : true;
}

void MdlObject::Dump (int r)
{
	for (int a=0;a<r;a++)
		logger.Print ("  ");
	logger.Trace (NL_Debug, "MdlObject \'%s\'\n", name.c_str());

	for (int a=0;a<childs.size();a++)
		childs[a]->Dump(r+1);
}


void MdlObject::GetTransform(Matrix& mat)
{
	Matrix scaling;
	scaling.scale(scale);

	Matrix rotationMatrix;
	rotation.ToMatrix(rotationMatrix);

	mat = scaling * rotationMatrix;
	mat.t(0) = position.x;
	mat.t(1) = position.y;
	mat.t(2) = position.z;
}

void MdlObject::GetFullTransform(Matrix& tr)
{
	GetTransform (tr);

	if (parent) {
		Matrix parentTransform;
		parent->GetFullTransform(parentTransform);

		tr = parentTransform * tr;
	}
}

void MdlObject::SetPropertiesFromMatrix(Matrix& transform)
{
	position.x = transform.t(0);
	position.y = transform.t(1);
	position.z = transform.t(2);

	// extract scale and create a rotation matrix
	Vector3 cx,cy,cz; // columns
	transform.getcx(cx);
	transform.getcy(cy);
	transform.getcz(cz);

	scale.x = cx.length();
	scale.y = cy.length();
	scale.z = cz.length();

	Matrix rotationMatrix;
	rotationMatrix.identity();
	rotationMatrix.setcx (cx / scale.x);
	rotationMatrix.setcy (cy / scale.y);
	rotationMatrix.setcz (cz / scale.z);
	rotation.FromMatrix(rotationMatrix);
}

void MdlObject::Load3DOTextures (TextureHandler *th)
{
	if (!bTexturesLoaded) {
		for (PolyIterator p(this); !p.End(); p.Next())
		{
			if (!p->texture && !p->texname.empty()) {
				p->texture = th->GetTexture (p->texname.c_str());
				if (p->texture) 
					p->texture->VideoInit();
				else
					p->texname.clear();
			}
		}
		bTexturesLoaded=true;
	}

	for (int a=0;a<childs.size();a++)
		childs[a]->Load3DOTextures (th);
}

void MdlObject::FlipPolygons()
{
	ApplyPolyMeshOperationR(&PolyMesh::FlipPolygons);
}


// apply parent-space transform without modifying any transformation properties
void MdlObject::ApplyParentSpaceTransform(const Matrix& psTransform)
{
	/*
	A = object transformation matrix to parent space
	T = given parent-space transform matrix
	p = original position
	p' = new position

	the new position, when transformed,
	needs to be equal to the transformed old position with scale/mirror applied to it:

	A*p' = SAp
	p' = (A^-1)SAp
	*/

	Matrix transform, inv;

	GetTransform(transform);
	transform.inverse(inv);

	Matrix result = inv * psTransform;
	result *= transform;

	TransformVertices(result);

	// transform childs objects
	for (int a=0;a<childs.size();a++)
		childs[a]->ApplyParentSpaceTransform(result);
}

void MdlObject::ApplyTransform (bool removeRotation, bool removeScaling, bool removePosition)
{
	Matrix mat;
	mat.identity();
	if (removeScaling) {
		// child vertices have to be transformed to do mirroring properly
		if (scale.x < 0.0f || scale.y < 0.0f || scale.z < 0.0f) {
			Vector3 mirror;
			bool flip = false;
			for (int a=0;a<3;a++) {
				if (scale[a] < 0.0f) {
					mirror[a] = -1.0f;
					scale[a] = -scale[a];
					flip = !flip;
				} else mirror[a] = 1.0f;
			}
			Matrix mirrorMatrix;
			mirrorMatrix.scale(mirror);

			TransformVertices(mirrorMatrix);
			for (int a=0;a<childs.size();a++)
				childs[a]->ApplyParentSpaceTransform(mirrorMatrix);

			if (flip)
				FlipPolygons();
		}

		Matrix scaling;
		scaling.scale(scale);
		scale.set(1,1,1);
		mat = scaling;
	}
	if (removeRotation) {
		Matrix rotationMatrix;
		rotation.ToMatrix(rotationMatrix);
		mat *= rotationMatrix;
		rotation = Rotator();
	}
	
	if (removePosition) {
		mat.t(0) = position.x;
		mat.t(1) = position.y;
		mat.t(2) = position.z;
		position=Vector3();
	}
	Transform(mat);
}

void MdlObject::NormalizeNormals ()
{
	for (VertexIterator v(this);!v.End();v.Next())
		v->normal.normalize();

	for (int a=0;a<childs.size();a++)
		childs[a]->NormalizeNormals ();

	InvalidateRenderData();
}

void MdlObject::TransformVertices (const Matrix& transform)
{
	if (geometry)
		geometry->Transform(transform);
	InvalidateRenderData();
}




void MdlObject::Transform (const Matrix& transform)
{
	TransformVertices (transform);
	
	for (vector<MdlObject*>::iterator i=childs.begin();i!=childs.end();++i) {
		Matrix subObjTr;
		(*i)->GetTransform (subObjTr);
		subObjTr *= transform;
		(*i)->SetPropertiesFromMatrix(subObjTr);
	}
}

MdlObject* MdlObject::Clone()
{
	MdlObject *cp = new MdlObject;

	if (geometry)
		cp->geometry = geometry->Clone();

	for (int a=0;a<childs.size();a++) {
		MdlObject *ch = childs[a]->Clone();
		cp->childs.push_back(ch);
		ch->parent = cp;
	}

	cp->position=position;
	cp->rotation=rotation;
	cp->scale=scale;
	cp->name=name;
	cp->isSelected=isSelected;

	// clone animInfo
	animInfo.CopyTo(cp->animInfo);

	return cp;
}

bool MdlObject::HasSelectedParent ()
{
	MdlObject *c = parent;
	while (c) {
		if (c->isSelected) 
			return true;
		c = c->parent;
	}
	return false;
}

void MdlObject::ApproximateOffset()
{
	Vector3 mid;
	int c=0;
	for (VertexIterator v(this);!v.End();v.Next()) 
	{
		mid += v->pos;
		c++;
	}
	if (c)
		mid/=(float)c;

	position += mid;
	for (VertexIterator v(this);!v.End();v.Next())
		v->pos -= mid;
}

void MdlObject::UnlinkFromParent ()
{
	if (parent) {
		parent->childs.erase (find(parent->childs.begin(),parent->childs.end(),this));
		parent = 0;
	}
}

void MdlObject::LinkToParent(MdlObject *p)
{
	if (parent) UnlinkFromParent();
	p->childs.push_back(this);
	parent = p;
}

vector<MdlObject*> MdlObject::GetChildObjects()
{
	vector<MdlObject*> objects;

	for (uint a=0;a<childs.size();a++)  {
		if (!childs[a]->childs.empty ()) {
			vector <MdlObject *> sub = childs[a]->GetChildObjects();
			objects.insert (objects.end(), sub.begin(), sub.end());
		}
		objects.push_back (childs[a]);
	}

	return objects;
}

void MdlObject::FullMerge ()
{
	vector <MdlObject *> ch=childs;
	for (int a=0;a<ch.size();a++) {
		ch[a]->FullMerge ();
		MergeChild (ch[a]);
	}
}


void MdlObject::MergeChild (MdlObject *ch)
{
	ch->ApplyTransform(true,true,true);
	PolyMesh* pm = ch->GetPolyMesh();
	if (pm)
		pm->MoveGeometry(GetOrCreatePolyMesh());

	// move the childs
	for (int a=0;a<ch->childs.size();a++) ch->childs[a]->parent = this;
	childs.insert (childs.end(), ch->childs.begin(),ch->childs.end());
	ch->childs.clear();

	// delete the child
	childs.erase (find(childs.begin(),childs.end(),ch));
	delete ch;
}

void MdlObject::MoveOrigin(Vector3 move)
{
	// Calculate inverse
	Matrix tr;
	GetTransform(tr);

	Matrix inv;
	if (tr.inverse(inv)) // Origin-move only works for objects with inverse transform (scale can't contain zero)
	{
		// move the object
		position += move;

		Matrix mat;
		mat.translation(-move);
		mat = tr * mat;
		mat *= inv;

		Transform(mat);
	}
}

void MdlObject::UpdateAnimation(float time)
{
	animInfo.Evaluate (this, time);

	for (int a=0;a<childs.size();a++)
		childs[a]->UpdateAnimation (time);
}

void MdlObject::InitAnimationInfo ()
{
	animInfo.AddProperty (AnimController::GetStructController (AnimController::GetFloatController(), 
		Vector3::StaticClass()), "position", (int)&((MdlObject*)0)->position);
/*	animInfo.AddProperty (AnimController::GetQuaternionController(),
		"rotation", (int)&((MdlObject*)0)->rotation.q);*/
	animInfo.AddProperty (AnimController::GetStructController (AnimController::GetFloatController(), 
		Vector3::StaticClass()), "rotation", (int)&((MdlObject*)0)->rotation.euler);
//	animInfo.AddProperty (AnimController::GetStructController (AnimController::GetEulerAngleController(), 
//		Vector3::StaticClass()), "rotation", (int)&((MdlObject*)0)->rotation);
	animInfo.AddProperty (AnimController::GetStructController (AnimController::GetFloatController(), 
		Vector3::StaticClass()), "scale", (int)&((MdlObject*)0)->scale);
}

