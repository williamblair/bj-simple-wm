/* header file for
 * reparenting windows when they send a map request 
 * (want to be displayed on the screen) */

#include <X11/Xlib.h>

#ifndef REPARENT_H_INCLUDED
#define REPARENT_H_INCLUDED

#define BORDER_WIDTH 2  // border size of the parent window
#define TITLE_HEIGHT 20 // size of the title bar

// args - Display, Window to be reparented, Boolean if the window was 
// created before we started the window manager
Bool reparent_window(Display *d, Window child, Bool before_wm);

#endif
