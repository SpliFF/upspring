//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#ifndef JC_VERTEX_BUFFER_H
#define JC_VERTEX_BUFFER_H


class VertexBuffer
{
public:
	VertexBuffer ();
	~VertexBuffer ();

	void Init(int bytesize);

	void* Bind (); // returns the pointer that should be passed to glVertexPointer
	void Unbind ();

	void* LockData(); // returns a pointer to the data, write-only
	void UnlockData();

	uint GetByteSize() { return size; }
	
	static int TotalSize() { return totalBufferSize; }

protected:
	VertexBuffer(const VertexBuffer&) {} // nocopy

	void *data;
	uint id;
	uint size;
	uint type;

	static int totalBufferSize;
};

class IndexBuffer : public VertexBuffer
{
public:
	IndexBuffer ();
};

#endif
