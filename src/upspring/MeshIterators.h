class PolyIterator
{
public:
	PolyIterator(MdlObject *o)
	{
		mesh = o->GetPolyMesh();
		pos=0;
	}
	PolyIterator(PolyMesh *m)
	{
		mesh = m;
	}

	Poly* Get() { return mesh ? mesh->poly [pos] : 0; }
	bool End() { return !mesh || (uint)pos >= mesh->poly.size(); }
	Poly* operator->() { return Get(); }
	Poly* operator*() { return Get(); }
	void Next() { pos++; }
	std::vector<Vertex>* verts() { return mesh? &mesh->verts : 0; }
	PolyMesh* Mesh() { return mesh; }

protected:
	PolyIterator(const PolyIterator&) {}

	unsigned int pos;
	PolyMesh *mesh;
};

// The const poly iterator can also iterate non-polymesh geometry (as long as ToPolyMesh() is implemented)
class ConstPolyIterator : public PolyIterator
{
	bool own;
public:
	ConstPolyIterator(MdlObject *o) : PolyIterator(o)
	{ 
		own = false;
		if(!mesh && o->geometry) {
			mesh = o->geometry->ToPolyMesh(); 
			own = true;
		}
	}
	~ConstPolyIterator()
	{
		if (own)
			delete mesh;
	}
};


class VertexIterator
{
public:
	VertexIterator(MdlObject *o)
	{
		mesh = o->GetPolyMesh();
		pos = 0;
	}

	bool End() { return !mesh || (uint)pos >= mesh->verts.size(); }
	void Next() { pos ++; }
	Vertex* Get() { return mesh ? &mesh->verts[pos] : 0; }

	Vertex* operator*() { return Get(); }
	Vertex* operator->() { return Get(); }

protected:
	unsigned int pos;
	PolyMesh *mesh;
};



// NOTE:
//   using typedefs would be cleaner but causes "expected nested-name-specifier"
//   error since MemberContainerT then will get referenced outside the template
//   definition
#define TEMPLATE template<typename ObjIterator, typename ObjT, typename MemberContainerT>
#define ELEM_VAL_VEC std::vector<typename MemberContainerT::value_type >
#define ELEM_PTR_VEC std::vector<typename MemberContainerT::value_type*>

TEMPLATE
ELEM_VAL_VEC
GetElementList(MemberContainerT ObjT::*containerPtr, ObjIterator start, ObjIterator end) {
	ELEM_VAL_VEC vec;

	for (; start != end; ++start) {
		MemberContainerT& mc = (*start)->*containerPtr;
		for (typename MemberContainerT::iterator mi = mc.begin(); mi != mc.end(); ++mi)
			vec.push_back(*mi);
	}
	return vec;
}

TEMPLATE
ELEM_PTR_VEC
GetElementPtrList(MemberContainerT ObjT::*containerPtr, ObjIterator start, ObjIterator end) {
	ELEM_PTR_VEC vec;

	for (; start != end; ++start) {
		MemberContainerT& mc = (*start)->*containerPtr;
		for (typename MemberContainerT::iterator mi = mc.begin(); mi != mc.end(); ++mi)
			vec.push_back(&*mi);
	}
	return vec;
}
