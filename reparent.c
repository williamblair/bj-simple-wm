/* implementation file for
 * reparenting windows when they send a map request 
 * (want to be displayed on the screen) */

#include "reparent.h"
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

// counter and Windows to be used as window borders
int frames_index=0;
Window frames[10];

/* creates a frame for the input child window and reparents
 * it to the created frame */
Bool reparent_window(Display *d, Window child, Bool before_wm)
{
	XWindowAttributes a; // get info about the child window to create
	                     // its border
	  
	/* moved as defines in reparent.h; used to resize windows */                   
	//const int border_width = 2; // border size of the parent window
	//const int title_height = 20; // size of the title bar
	
	/* get child information */
	XGetWindowAttributes(d, child, &a);
	
	/* exit if we have too many windows cuz no linked list yet */
	if(frames_index >= 10){
		fprintf(stderr, "MORE THAN 10 Windows AAAAHHH!\n");
		exit(0);
	}
	
	/* create the border window */
	frames[frames_index] = XCreateSimpleWindow(d,                                  // Display *d
	                             RootWindow(d, DefaultScreen(d)),    // Display *parent
	                             0,                                // x coord
	                             0,                                // y coord
	                             a.width+(BORDER_WIDTH),           // window width
	                             a.height+TITLE_HEIGHT,              // window height
	                             BORDER_WIDTH,                       // border size
	                             WhitePixel(d, DefaultScreen(d)),    // border
	                             BlackPixel(d, DefaultScreen(d)));   // background
	
	/* select events on the frame */
	XSelectInput( d, 
	              frames[frames_index], 
	              SubstructureRedirectMask | SubstructureNotifyMask );
	              
	/* restores the child if we crash somehow */
	XAddToSaveSet(d, child);
	
	/* assuming last thing needed to do */
	XReparentWindow(d,                        // Display *d 
	                child,                    // Window w
	                frames[frames_index],     // Window parent
	                //BORDER_WIDTH-(BORDER_WIDTH/2),             // int x - x position in new parent window
	                0,             // int x - x position in new parent window
	                TITLE_HEIGHT);            // int y - y position in new parent window
	
	  // 9. Grab universal window management actions on client window.
  //   a. Move windows with alt + left button.
  XGrabButton(
      d,
      Button1,
      Mod1Mask,
      child,
      False,
      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
      GrabModeAsync,
      GrabModeAsync,
      None,
      None);
  //   b. Resize windows with alt + right button.
  XGrabButton(
      d,
      Button3,
      Mod1Mask,
      child,
      False,
      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
      GrabModeAsync,
      GrabModeAsync,
      None,
      None);
  //   c. Kill windows with alt + f4.
  XGrabKey(
      d,
      XKeysymToKeycode(d, XK_F4),
      Mod1Mask,
      child,
      False,
      GrabModeAsync,
      GrabModeAsync);
  //   d. Switch windows with alt + tab.
  XGrabKey(
      d,
      XKeysymToKeycode(d, XK_Tab),
      Mod1Mask,
      child,
      False,
      GrabModeAsync,
      GrabModeAsync);
	
	/* map the parent window */
	XMapWindow(d, frames[frames_index]);
	frames_index++;
	return True;
}
