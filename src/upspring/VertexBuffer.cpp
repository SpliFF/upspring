
//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"
#include "VertexBuffer.h"

#include <GL/glew.h>
#include <GL/gl.h>

int VertexBuffer::totalBufferSize = 0;

VertexBuffer::VertexBuffer() 
{
	id = 0;
	data = 0;
	size = 0;
	type = GL_ARRAY_BUFFER_ARB;
}

void VertexBuffer::Init (int bytesize)
{
	if (GLEW_ARB_vertex_buffer_object) {
		data=0;
		glGenBuffersARB(1,&id);
	} else {
		data=new char[bytesize];
	}
	size=bytesize;
	totalBufferSize+=size;
}

VertexBuffer::~VertexBuffer ()
{
	if (id) {
		glDeleteBuffersARB(1,&id);
		id=0;
	} else {
		SAFE_DELETE_ARRAY(data);
	}
	totalBufferSize-=size;
}

void* VertexBuffer::LockData ()
{
	if (id) { 
		glBindBufferARB(type, id);
		glBufferDataARB(type, size, 0, GL_STATIC_DRAW_ARB);
		return glMapBufferARB(type, GL_WRITE_ONLY);
	} else
		return data;
}

void VertexBuffer::UnlockData ()
{
	if (id) 
		glUnmapBufferARB(type);
}

void* VertexBuffer::Bind ()
{
	if (id) {
		glBindBufferARB(type, id);
		return 0;
	}
	else return data;
}


void VertexBuffer::Unbind ()
{
	if (id)
		glBindBufferARB(type, 0);
}


IndexBuffer::IndexBuffer ()
{
	type = GL_ELEMENT_ARRAY_BUFFER_ARB;
}
