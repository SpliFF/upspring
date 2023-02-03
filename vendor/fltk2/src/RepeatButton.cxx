//
// "$Id: RepeatButton.cxx 4886 2006-03-30 09:55:32Z fabien $"
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

#include <fltk/RepeatButton.h>
#include <fltk/events.h>
using namespace fltk;

/*! \class fltk::RepeatButton
  The callback is done when the user pushes the button down, and then
  after .5 second it is repeated 10 times a second, as long as the
  user is pointing at the button and holding it down.
*/

#define INITIALREPEAT .5f
#define REPEAT .1f

int RepeatButton::handle(int event) {
  bool newval;
  switch (event) {
  case HIDE:
  case DEACTIVATE:
  case RELEASE:
    newval = false; goto J1;
  case PUSH:
  case DRAG:
    newval = event_inside(Rectangle(w(), h()));
  J1:
    if (value(newval)) {
      if (newval) {
	add_timeout(INITIALREPEAT);
	do_callback();
      } else {
	remove_timeout();
      }
    }
    return newval;
  case TIMEOUT:
    repeat_timeout(REPEAT);
    do_callback();
    return 1;
  default:
    return Button::handle(event);
  }
}

//
// End of "$Id: RepeatButton.cxx 4886 2006-03-30 09:55:32Z fabien $".
//
