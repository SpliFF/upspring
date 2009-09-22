//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "EditorUI.h"
#include "CfgParser.h"
#include "MeshIterators.h"


#include <GL/glew.h>
#include <GL/gl.h>

#define CH_R 1
#define CH_G 2
#define CH_B 3
#define CH_RGB 4
#define CH_A 5

// ------------------------------------------------------------------------------------------------
// UVViewWindow - Subclasses ViewWindow to view model UV coordinates
// ------------------------------------------------------------------------------------------------

UVViewWindow::UVViewWindow (int X,int Y,int W,int H,const char *l) :
	ViewWindow (X,Y,W,H,l)
{
	textureIndex=0;
	channel = CH_RGB;

	hasTexCombine = false;
}


void UVViewWindow::SetupProjectionMatrix ()
{}

bool UVViewWindow::SetupChannelMask ()
{
	Model *mdl = editor->GetMdl ();

	if (!mdl->HasTex(textureIndex)) 
		return false;

	if (!hasTexCombine) {
		glEnable (GL_TEXTURE_2D);
		glBindTexture (GL_TEXTURE_2D, mdl->TextureID(textureIndex));
		return true;
	}

	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	if (channel == CH_A) {
		glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_ALPHA);
		glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	} else {
		float maskcol[4] ={ 0.0f,0.0f,0.0f,0.0f };
		switch (channel) {
			case CH_R: maskcol[0]=1.0f; break;
			case CH_G: maskcol[1]=1.0f; break;
			case CH_B: maskcol[2]=1.0f; break;
			case CH_RGB: maskcol[0]=maskcol[1]=maskcol[2]=1.0f; break;
		}
		glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, maskcol);
		glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE);
		glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_CONSTANT_ARB);
		glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);
	}
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, mdl->TextureID(textureIndex));
	return true;
}

void UVViewWindow::DisableChannelMask ()
{
	glDisable (GL_TEXTURE_2D);
	if (hasTexCombine) {
		glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi (GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_PREVIOUS);
		glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	}
}

void UVViewWindow::DrawScene ()
{
	Model *mdl = editor->GetMdl ();
	glDisable (GL_CULL_FACE);
	glDisable (GL_DEPTH_TEST);
	glColor3ub (255,255,255);

	const float border=0.1f;
	const float scale=2.0f*(1.0f-border);

	glPushMatrix ();
	glScalef (scale, scale, 1.0f);
	glTranslatef (-0.5f,-0.5f,1.0f);

	if (SetupChannelMask ())
	{
		// draw texture on background
		glBegin (GL_QUADS);
			glTexCoord2f(-border, -border);
			glVertex2f  (-border, -border);
			glTexCoord2f (1.0f+border, -border);
			glVertex2f   (1.0f+border, -border);
			glTexCoord2f (1.0f+border,  1.0f+border);
			glVertex2f   (1.0f+border,  1.0f+border);
			glTexCoord2f(-border,  1.0f+border);
			glVertex2f  (-border,  1.0f+border);
		glEnd ();

		DisableChannelMask ();
	}

	// draw (0,0)-(1,1) texture box
	glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
	glBegin (GL_QUADS);
		glVertex2f (0.0f, 0.0f);
		glVertex2f (1.0f, 0.0f);
		glVertex2f (1.0f, 1.0f);
		glVertex2f (0.0f, 1.0f);
	glEnd ();

	// draw model vertices
	vector <MdlObject*> objs = mdl->GetObjectList ();
	for (int a=0;a<objs.size();a++) {
		MdlObject *obj = objs[a];
		for (PolyIterator pi(obj);!pi.End();pi.Next()) {
			if (!pi.verts())
				continue;

			glBegin(GL_POLYGON);
			for (int v=0;v<pi->verts.size();v++) {
				Vertex &vrt = (* pi.verts()) [pi->verts[v]];
				glVertex2f (vrt.tc[0].x, vrt.tc[0].y);
			}
			glEnd ();
		}
	}
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

	glPopMatrix ();
}

void UVViewWindow::GetViewMatrix (Matrix &m)
{
	m.identity ();
}

int UVViewWindow::handle (int msg)
{
	int r = ViewWindow::handle (msg);

	if (msg == fltk::PUSH) {
		if (fltk::event_button () == 3) {
			ShowPopupMenu ();
			return -1;
		}
	}

	return r;
}

template<int chan> static void SetChannelCB(fltk::Widget *w, void *data)
{
	UVViewWindow *wnd = (UVViewWindow *)data;
	wnd->channel = chan;
	wnd->redraw();
}

template<int texture> static void SetTextureIndexCB(fltk::Widget* w, void *data)
{
	UVViewWindow *wnd = (UVViewWindow *)data;
	wnd->textureIndex = texture;
	wnd->redraw();
}

void UVViewWindow::ShowPopupMenu()
{
	fltk::Menu *p = new fltk::Menu (0,0,30,20);

	setvalue(p->add("Texture 1",0,&SetTextureIndexCB<0>, this),textureIndex==0);
	setvalue(p->add("Texture 2",0,&SetTextureIndexCB<1>, this),textureIndex==1);

	if (hasTexCombine) {
		setvalue(p->add("Channel/R",0,&SetChannelCB<CH_R>, this),channel==CH_R);
		setvalue(p->add("Channel/G",0,&SetChannelCB<CH_G>, this),channel==CH_G);
		setvalue(p->add("Channel/B",0,&SetChannelCB<CH_B>, this),channel==CH_B);
		setvalue(p->add("Channel/RGB",0,&SetChannelCB<CH_RGB>, this),channel==CH_RGB);
		setvalue(p->add("Channel/A",0,&SetChannelCB<CH_A>, this),channel==CH_A);
	}

	p->popup(fltk::Rectangle(click.x,click.y,30,20), "View Settings");
	delete p;
}


// ------------------------------------------------------------------------------------------------
// MappingUI implementation
// ------------------------------------------------------------------------------------------------

MappingUI::MappingUI (IEditor *callback) : callback(callback)
{
	CreateUI ();
	view->editor=callback;
}

MappingUI::~MappingUI ()
{
	delete window;
}

void MappingUI::Show ()
{
	view->hasTexCombine=GLEW_ARB_texture_env_combine;

	window->set_non_modal();
	window->show ();
}

void MappingUI::flipUVs ()
{
	Model* mdl = callback->GetMdl ();
	vector<MdlObject*> obj = mdl->GetObjectList ();

	for (int a=0;a<obj.size();a++)
	{
		MdlObject *o = obj[a];

		for (VertexIterator vi(o); !vi.End(); vi.Next())
			vi->tc[0].y = 1.0f - vi->tc[0].y;
	}
}

void MappingUI::mirrorUVs()
{
	Model* mdl = callback->GetMdl ();
	vector<MdlObject*> obj = mdl->GetObjectList ();

	for (int a=0;a<obj.size();a++)
	{
		MdlObject *o = obj[a];

		for (VertexIterator vi(o); !vi.End(); vi.Next())
			vi->tc[0].x = 1.0f - vi->tc[0].x;
	}
}

