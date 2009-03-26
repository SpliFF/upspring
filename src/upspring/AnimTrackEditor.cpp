
//-----------------------------------------------------------------------
//  Upspring model editor
//  Copyright 2005 Jelmer Cnossen
//  This code is released under GPL license, see LICENSE.HTML for info.
//-----------------------------------------------------------------------

#include "EditorIncl.h"
#include "EditorDef.h"

#include "CfgParser.h"
#include "AnimationUI.h"

#include <set>
#include <iterator>

#include <fltk/draw.h>


AnimTrackEditorUI::AnimTrackEditorUI(IEditor *editor, TimelineUI *tl)
{
	callback = editor;
	timeline = tl;

	CreateUI();

	trackView->trackUI = this;
	browserList.ui = this;
	propBrowser->list(&browserList);
}

AnimTrackEditorUI::~AnimTrackEditorUI()
{
	delete window;
}

void AnimTrackEditorUI::Show()
{
	window->set_non_modal();
	window->show();
}

void AnimTrackEditorUI::Hide()
{
	window->hide();
}

void AnimTrackEditorUI::toggleMoveView()
{
}

void AnimTrackEditorUI::cmdAutoFitView()
{
	float maxY = -100000.0f;
	float minY = -maxY;

	for (list<AnimTrackEditorUI::AnimObject>::iterator oi = objects.begin(); oi != objects.end(); ++oi)
	{
		for (int p = 0; p < oi->props.size(); p ++) {
			if (!oi->props [p].display) continue;

			float step = (trackView->maxTime-trackView->minTime)/trackView->w();
			int x = 0, lastkey=-1;
			for (float time=trackView->minTime;x<trackView->w();time+=step,x++)
			{
				float y = oi->props[p].EvaluateY(time, lastkey);
				if (minY > y) minY = y;
				if (maxY < y) maxY = y;
			}
		}
	}
	float d = maxY-minY;
	if (!d) d = 1.0f;
	d *= 0.1f;
	trackView->minY = minY - d;
	trackView->maxY = maxY + d;
	trackView->redraw();
}

void AnimTrackEditorUI::cmdAutoFitTime()
{
	float minTime=10000;
	float maxTime=-10000;
	bool keys=false;
	for (list<AnimTrackEditorUI::AnimObject>::iterator oi = objects.begin(); oi != objects.end(); ++oi)
	{
		for (int p = 0; p < oi->props.size(); p ++) {
			if (!oi->props [p].display) continue;

			AnimProperty *prop = oi->props[p].prop;
			int nk=prop->NumKeys();
			if (nk>0) {
				keys=true;
				if (minTime > prop->GetKeyTime (0)) minTime = prop->GetKeyTime(0);
				if (maxTime < prop->GetKeyTime (nk-1)) maxTime = prop->GetKeyTime(nk-1);
			}
		}
	}
	if (keys) {
		float d = maxTime-minTime;
		if (!d) d = 1.0f;
		d *= 0.1f;
		trackView->minTime = minTime - d;
		trackView->maxTime = maxTime + d;
		trackView->redraw();
	}
}

void AnimTrackEditorUI::cmdDeleteKeys()
{
}

void AnimTrackEditorUI::Update()
{
	if(!window->visible())
		return;

	UpdateBrowser ();
	UpdateKeySel ();
	window->redraw();
}

// update the property browser with a new list of properties
void AnimTrackEditorUI::UpdateBrowser()
{
	Model *mdl = callback->GetMdl();

	if (chkLockObjects->value()) {
		vector<MdlObject*> obj = mdl->GetObjectList ();
		// look for non-existant objects that are still in the browser
		list<AnimObject>::iterator ai=objects.begin();
		while (ai != objects.end()) {
			list<AnimObject>::iterator i = ai++;
			if (find(obj.begin(),obj.end(),i->obj) == obj.end()) 
				objects.erase (i);
		}
	} else {
		vector<MdlObject*> selObj = mdl->GetSelectedObjects ();

		// look for objects in the view that shouldn't be there because of the new selection
		list<AnimObject>::iterator ai=objects.begin();
		while (ai != objects.end()) {
			list<AnimObject>::iterator i = ai++;
			if (find(selObj.begin(),selObj.end(),i->obj) == selObj.end()) 
				objects.erase (i);
		}

		// look for objects that aren't currently in the view and add them
		for (vector<MdlObject*>::iterator i=selObj.begin();i!=selObj.end();++i) {
			list<AnimObject>::iterator co = objects.begin();
			for (; co != objects.end(); ++co)
				if (co->obj == *i) break;
			if (co == objects.end())
				AddObject (*i);
		}
	}
	propBrowser->layout();
}


void AnimTrackEditorUI::AddObject (MdlObject *o)
{
	objects.push_back (AnimObject());
	objects.back().obj = o;

	// add properties from the object
	AnimationInfo& ai = o->animInfo;
	for (vector<AnimProperty*>::iterator ap=ai.properties.begin();ap!=ai.properties.end();++ap)
	{
		vector<Property>& props = objects.back().props;

		if ((*ap)->controller->GetNumMembers () > 0)  {
			for (int a = 0; a < (*ap)->controller->GetNumMembers (); a++) {
				props.push_back (Property());
				props.back().prop = *ap;
				props.back().display = false;
				props.back().member = a;
			}
		} else {
			props.push_back (Property());
			props.back().prop = *ap;
			props.back().display = false;
			props.back().member = -1;
		}
	}
}

void AnimTrackEditorUI::UpdateKeySel ()
{
	for (list<AnimTrackEditorUI::AnimObject>::iterator oi = objects.begin(); oi != objects.end(); ++oi)
	{
		for (int i = 0; i < oi->props.size(); i++) {
			Property *p = &oi->props[i];
			if (!p->display) continue;

			size_t nk = (size_t)p->prop->NumKeys();
			if (nk != p->keySel.size()) {
				p->keySel.resize(nk);
				fill(p->keySel.begin(),p->keySel.end(),false);
				trackView->redraw();
			}
		}
	}
}
// ------------------------------------------------------------------------------------------------
// AnimObject::Property evaluation
// ------------------------------------------------------------------------------------------------

float AnimTrackEditorUI::Property::EvaluateY (float time, int& lastkey)
{
	static vector<char> valbuf;

	int neededBufSize = prop->controller->GetSize();
	if (neededBufSize > valbuf.size())
		valbuf.resize(neededBufSize);

	// get the right member out of the value
	pair<AnimController*,void*> memctl = prop->controller->GetMemberCtl (member, &valbuf.front());

	// only evaluate when it is actually usable
	if (memctl.first->CanConvertToFloat())
	{
		prop->Evaluate (time, &valbuf.front(), &lastkey);
		return memctl.first->ToFloat(memctl.second);
	}

	return 0.0f;
}


// ------------------------------------------------------------------------------------------------
// List
// ------------------------------------------------------------------------------------------------

AnimTrackEditorUI::List::List()
{}

int AnimTrackEditorUI::List::children (const fltk::Menu *, const int *indexes, int level) {
	if (level == 0) {
		// object level
		return (int)ui->objects.size();
	} else if (level == 1) {
		// property level
		AnimObject& obj = *element_at(ui->objects.begin(),ui->objects.end(),*indexes);
		return (int)obj.props.size ();
	} else return -1;
}


fltk::Widget *AnimTrackEditorUI::List::child (const fltk::Menu *, const int *indexes, int level)
{
	AnimObject &o = *element_at(ui->objects.begin(),ui->objects.end(),indexes[0]);
	item.textfont(fltk::HELVETICA);
	item.ui = ui;
	if (level == 0) {
		item.user_data(&o);
		item.label (o.obj->name.c_str());

		// set open/closed state
		item.set_flag (fltk::OPENED, o.open && o.props.size());

		// set selection
		item.set_flag (fltk::SELECTED, o.selected);
	}
	else if (level == 1) {
		Property& p = o.props[indexes[1]];
		item.user_data(&p);

		itemLabel = p.prop->name;
		if (p.member >= 0) {
			itemLabel += '.';
			itemLabel += p.prop->controller->GetMemberName (p.member);
		}
		item.label (itemLabel.c_str());
		if (p.display) item.textfont (fltk::HELVETICA_BOLD);

		// set selection
		if (p.selected) item.set_flag(fltk::SELECTED);
		else item.clear_flag(fltk::SELECTED);

		item.callback(item_callback);
	}
	item.w(0);
	return &item;
}

void AnimTrackEditorUI::List::flags_changed (const fltk::Menu *, fltk::Widget *w)
{
	BrowserItem *bi = (BrowserItem *)w->user_data();
	bi->open = w->flags() & fltk::OPENED;
	bi->selected = w->flags() & fltk::SELECTED;
}

void AnimTrackEditorUI::List::item_callback (fltk::Widget *w, void *user_data)
{
	BrowserItem *bi = (BrowserItem *)user_data;
	if (bi->IsProperty () && fltk::event_clicks () == 1) {
		Property* p = (Property *)user_data;

		p->display = !p->display;
		((Item*)w)->ui->UpdateKeySel ();
		((Item*)w)->ui->window->redraw();
	}
}

// -----------------------------------------------------------------------------
// Animation Track View - displays keyframes for the selected object properties
// -----------------------------------------------------------------------------

AnimKeyTrackView::AnimKeyTrackView(int X,int Y,int W,int H, const char*label) 
	:  Group (X,Y,W,H,label)
{
	minY = 0.0f;
	maxY = 360.0f;
	minTime = 0.0f;
	maxTime = 5.0f;
	curTime = 0.0f;
	movingView=movingKeys=zooming=false;

	trackUI = 0;
}

void AnimKeyTrackView::draw()
{
	fltk::Group::draw();
	if (w()<1) return;

	assert (trackUI);

	fltk::push_clip(Rectangle(w(),h()));
	
	fltk::setcolor(fltk::GRAY40);
	fltk::fillrect(fltk::Rectangle(w(),h()));
	
	char buffer[128];
	sprintf (buffer, "Time=[%3.2f,%3.2f], Y=[%4.4f,%4.4f]",minTime,maxTime,minY,maxY);
	fltk::setcolor (fltk::GRAY80);
	fltk::drawtext(buffer,0.0f,15.0f);

	curTime = trackUI->callback->GetTime();

	fltk::setcolor(fltk::WHITE);

	float step = (maxTime-minTime)/w();
	for (list<AnimTrackEditorUI::AnimObject>::iterator oi = trackUI->objects.begin(); oi != trackUI->objects.end(); ++oi)
	{
		for (int i = 0; i < oi->props.size();i ++) {
			AnimTrackEditorUI::Property *p = &oi->props[i];
			if (!p->display) continue;

			int x = 1, lastkey=-1, key=-1;
			float firstY = p->EvaluateY(minTime, lastkey);
			int prevY = h()*(1.0f-(firstY-minY)/(maxY-minY));

			for (float time=minTime+step;x<w();time+=step,x++)
			{
				float y = p->EvaluateY(time, lastkey);
				int cury = h()*(1.0f-(y-minY)/(maxY-minY));
				fltk::drawline(x,prevY,x+1,cury);
				if (key != lastkey) {
					fltk::setcolor(p->keySel[lastkey] ? fltk::GREEN : fltk::BLUE);
					fltk::drawline(x+1,cury-10,x+1,cury+10);
					fltk::setcolor(fltk::WHITE);
					key = lastkey;
				}
				prevY = cury;
			}
		}
	}

	fltk::setcolor(fltk::YELLOW);
	int x=(curTime-minTime)/step;
	fltk::drawline(x, 0, x, h());

	fltk::pop_clip();
}

void AnimKeyTrackView::SelectKeys (int mx, int my)
{
	float time = mx*(maxTime-minTime)/w() + minTime; // time at selection pos
	float y = my*(maxY-minY)/h() + minY;

	// Make sure key selection arrays have the correct size
	trackUI->UpdateKeySel ();

	float epsilon=0.5f;

	for (list<AnimTrackEditorUI::AnimObject>::iterator oi = trackUI->objects.begin(); oi != trackUI->objects.end(); ++oi)
	{
		for (int i = 0; i < oi->props.size();i ++) {
			AnimTrackEditorUI::Property *p = &oi->props[i];
			if (!p->display) continue;

            for (int a=0;a<p->prop->NumKeys();a++) {
				float kt = p->prop->GetKeyTime (a);
				if (kt < time+epsilon && kt > time-epsilon)
				{
					int lastkey=-1;
					float ky = p->EvaluateY (kt, lastkey);
					if (ky < y+epsilon && ky > y-epsilon) {
						p->keySel [a] = true;
						redraw();
					}
				}
			}		
		}
	}
}

int AnimKeyTrackView::handle(int event)
{
	if (event == fltk::PUSH) {
		clickx = xpos = fltk::event_x();
		clicky = ypos = fltk::event_y();
		
		if (fltk::event_button() == 2)
			movingView=true;
		if (fltk::event_button() == 1)
			movingKeys=true;
		if (fltk::event_button() == 3)
			zooming=true;
		return 1;
	}
	else if(event == fltk::LEAVE)
		movingView=movingKeys=zooming=false;
	else if(event == fltk::RELEASE) {
		if (fltk::event_x () == clickx && 
			fltk::event_y () == clicky)
		{
			SelectKeys (clickx,clicky);
		}
		movingView=movingKeys=zooming=false;
	} else if(event == fltk::ENTER)
		return 1;
	else if (event == fltk::DRAG)
	{
		int dx = fltk::event_x()-xpos;
		int dy = fltk::event_y()-ypos;
		xpos = fltk::event_x();
		ypos = fltk::event_y();
		
		if (movingView) {
			float ts = dx*(maxTime-minTime)/w();
			minTime -= ts;
			maxTime -= ts;
			float ys = dy*(maxY-minY)/h();
			minY += ys;
			maxY += ys;
			redraw();
		}
		if (movingKeys) {
		}
		if (zooming) {
			float xs = 1.0f + dx * 0.01f;
			float ys = 1.0f - dy * 0.01f;

			float midTime = (minTime+maxTime)*0.5f;
			minTime = midTime + (minTime-midTime) * xs;
			maxTime = midTime + (maxTime-midTime) * xs;

			float midY = (minY+maxY)*0.5f;
			minY = midY + (minY-midY) * ys;
			maxY = midY + (maxY-midY) * ys;
			redraw();
		}
	}
	return fltk::Group::handle(event);
}



