/* simplest possible window manager for x11 
 *
 * William Blair
 * 04/19/17
 * */

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdio.h>
#include <stdlib.h>

#define MIN_WIDTH  10 // the minimum width and height of each window
#define MIN_HEIGHT 10

#define TASKBAR_HEIGHT 30 // the height of the taskbar

int frames_index=0;
Window frames[10]; // make me a linked list later!

/* testing!!! */
Bool reparent_window(Display *d, Window child, Bool before_wm)
{
	XWindowAttributes a; // get info about the child window to create
	                     // its border
	                     
	const int border_width = 2; // border size of the parent window
	const int title_height = 20; // size of the title bar
	
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
	                             a.width+(border_width),           // window width
	                             a.height+title_height,              // window height
	                             border_width,                       // border size
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
	                border_width,             // int x - x position in new parent window
	                title_height);            // int y - y position in new parent window
	
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

int main(int argc, char *argv[])
{
	Display            *d = NULL;   // the main X Display to open
	XWindowAttributes   a;          // properties of the main window
	XButtonEvent        start;      // saves the cursor's position at the beginning of a move event
	XEvent              e;          // hold's X server events
	Bool                run = True; // False when the program is done running

	Window              testWindow; // testing creation of a taskBar or something
	
	//Window              parentWindow; // testing reparenting
	//Window              childWindow;

	/* open the main display and error check 
	 * args - char *display_name
	 * if display is NULL, defaults to the DISPLAY
	 * environment variable */	
	d = XOpenDisplay(NULL);
	if( !d ){
		fprintf(stderr, "Failed to open X display!\n");
		return -1;
	}
	printf("Opened Display!\n");
	
	/* test the window reparenting */
	//reparent_window(d, parentWindow, childWindow);

	/* tell X we want to utilize Substructure Redirection, for 
	 * reparenting created windows */
	XSelectInput( d,                                                     // display
	              RootWindow(d, DefaultScreen(d)),                      // window to select input on
	              SubstructureRedirectMask | SubstructureNotifyMask );   // which masks to use
	XSync(d, False); // clear all events
	
	XGrabServer(d); // prevents windows from changing under us
	
	/* frame windows created before manager started */
	Window r, p, *c; // root return, parent return, *children return
	unsigned int numChildren, i; // number of children of the root windows
	XQueryTree( d,                                     // Display *d
	            RootWindow(d, DefaultScreen(d)),       // Window w
	            &r,                                    // Window *root_return
	            &p,                                    // Window *parent
	            &c,                                    // Window **children_return
	            &numChildren );                        // unsigned int *nchildren_return

	for(i=0; i<numChildren; i++){
		printf("Reparenting window number %d\n", i);
		reparent_window(d, *(c+i), True);
	}

	/* Select which types of key press/mouse press events we want to receieve */

	/* grab F1 */
	XGrabKey(d,                                            // display pointer - x server connection
	         XKeysymToKeycode(d, XStringToKeysym("F1")),   // int keycode - which key is pressed to trigger handling
			 Mod1Mask,                                     // unsigned int modifiers - specific keymass, can also send AnyModifier for all types
			 DefaultRootWindow(d),                         // Window grab_window - which window these keypresses apply to - in this case the root window
			 True,                                         // Bool owner_events - wether these keyboard events are also reported like normal to the system
			 GrabModeAsync,                                // int pointer_mode - specifies further processing of pointer events - either GrabModeSync or GrabModeAsync
			 GrabModeAsync);                               // int keyboard_mode - specifies further processing of keyboard events - either GrabModeSync or GrabModeAsync

	/* grab left mouse click in specific situations */
	XGrabButton(d,                                                        // display pointer
	            1,                                                        // unsigned int button - which button is pressed, or AnyButton
				Mod1Mask,                                                 // unsigned int modifiers - set of keymasks or AnyModifier
				DefaultRootWindow(d),                                     // Window grab_window - which window these button presses are handled in - in this case the root window
				True,                                                     // Bool owner_events - wether the mouse press is reported like normal to the system
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask,  // unsigned int event_mask - which pointer events are reported to the client
				GrabModeAsync,                                            // int pointer_mode - specifies further processing of events - either GrabModeSync or GrabModeAsync
				GrabModeAsync,                                            // int pointer_mode - specifies further processing of events - either GrabModeSync or GrabModeAsync
				None,                                                     // Window confine_to - specifies to window to confine the cursor in or None
				None);                                                    // Cursor cursor - specifies the cursor to be displayed or None

	/* grab right mouse click in specific situations */
	XGrabButton(d,                                                        // display pointer
	            3,                                                        // unsigned int button - which button is pressed, or AnyButton
				Mod1Mask,                                                 // unsigned int modifiers - set of keymasks or AnyModifier
				DefaultRootWindow(d),                                     // Window grab_window - which window these button presses are handled in - in this case the root window
				True,                                                     // Bool owner_events - wether the mouse press is reported like normal to the system
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask,  // unsigned int event_mask - which pointer events are reported to the client
				GrabModeAsync,                                            // int pointer_mode - specifies further processing of events - either GrabModeSync or GrabModeAsync
				GrabModeAsync,                                            // int pointer_mode - specifies further processing of events - either GrabModeSync or GrabModeAsync
				None,                                                     // Window confine_to - specifies to window to confine the cursor in or None
				None);                                                    // Cursor cursor - specifies the cursor to be displayed or None
				
	/* grab right mouse click on root window */
	XGrabButton(d,
	            3,
	            0,                      // trying with no masks... edit - Works!\n
	            DefaultRootWindow(d),
	            True,
	            ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
	            GrabModeAsync,
	            GrabModeAsync,
	            None,
	            None);

	/* Create the taskbar/test window - won't be reparented (i think) */
	int width  = DisplayWidth( d, DefaultScreen(d));
	int height = DisplayHeight(d, DefaultScreen(d));
	testWindow = XCreateSimpleWindow(d,                                  // Display *d
	                                 RootWindow(d, DefaultScreen(d)),    // Display *parent
									 0,                                  // x coord
									 height - TASKBAR_HEIGHT,            // y coord
									 width,                              // window width
									 TASKBAR_HEIGHT,                     // window height
									 1,                                  // border size
									 BlackPixel(d, DefaultScreen(d)),    // border
									 WhitePixel(d, DefaultScreen(d)));   // background
	/* make the window visible */
	XMapWindow(d, testWindow);

	/* makes it so that there is no child event window of 
	 * the current event window */
	start.subwindow = None;

	XUngrabServer(d);

	/* the main event loop */
	do
	{
		/* get the next XEvent 
		 * can also use XPending(), ConnectionNumber() with 
		 * select() or poll() */
		XNextEvent(d, &e);

		/* raise the selected window on a keypress */
		if( e.type == KeyPress &&                      // if the event type is a keypress event
		    e.xkey.subwindow != None )                 // because we're in control of the root window, the subwindow will be an application window (I think)
		{
			printf("Keypress Event!\n");
			XRaiseWindow(d, e.xkey.subwindow);         // raise the window the active event window; which, again, is a subwindow from root
		}

		/* handle mouse events */
		else if( e.type == ButtonPress &&             // if the event type is a mouse press
		         e.xbutton.subwindow != None )        // if the mouse press happened on a window (subwindow of root)
		{
			printf("Selecting a subwindow from buttonpress event!\n");

			/* Save current attributes of the active window and mouse position,
			 * that way once movement or something happens we know where to start from */
			XGetWindowAttributes(d, e.xbutton.subwindow, &a);
			start = e.xbutton;

			/* make sure the window isn't below any other windows
			 * when it is active - this is the definition of
			 * a 'raised' window, and is what the keypress event
			 * above does */
			XRaiseWindow(d, e.xbutton.subwindow);
		}
		
		else if(e.type == ButtonPress &&
		        e.xbutton.subwindow == None){
			printf("No subwindow!\n");
		}

		/* handle mouse movement 
		 * X11 automatically only sends motion events if a button is also
		 * being pressed */
		else if(e.type == MotionNotify &&       // if the event was a motion event
				start.subwindow != None )       // from above, if we had clicked on a window
		{
			/* notes below taken from tinywm that this is based off of 
			 *
			 * could compress motion notify events here with
			 * while(XCheckTypedEventd, MotionNotify, &e());
			 *
			 * if there are 10 waiting, it makes since to only handle the
			 * most recent one
			 *
			 * if the window is large or things are slow to begin with,
			 * not doing this can result in drag lag
			 *
			 * also, for desktop switching, you might want to 
			 * compress EnterNotify events so you don't get 'focus flicker'
			 * as windows shuffle around beneath the pointer
			 * */

			/* calculate the x and y difference between the original mouse
			 * position and the new mouse position */
			int dX = e.xbutton.x_root - start.x_root;
			int dY = e.xbutton.y_root - start.y_root;
		
			/* the new position of the window is equal
			 * to its old position plus the difference of the mouse
			 * cursor */
			int newX = a.x + dX;
			int newY = a.y + dY;

			/* if it was a left mouse click, move the window by
			 * applying the mouse movement difference to the
			 * x and y coordinates of the window */
			if( start.button == 1 ) {

				printf("Movement Event!\n");
				printf("Ax Dx Width = %d\n", a.x + dX + a.width);
			
				/* if the farthest right of the window would be
				 * offscreen, force it to stay onscreen*/
				if( newX + a.width > XDisplayWidth(d, DefaultScreen(d))) {
					newX = XDisplayWidth(d, DefaultScreen(d)) - a.width;
				}
				/* if the farthest left of the window would be
				 * offscreen, force it to stay onscreen */
				else if( newX <= 0 ) newX = 1;

				/* same as above but for y coordinates */
				if( newY + a.height > XDisplayHeight(d, DefaultScreen(d))) {
					newY = XDisplayHeight(d, DefaultScreen(d)) - a.height;
				}
				else if( newY <= 0 ) newY = 1;

				/* apply the new coordinates */
				XMoveResizeWindow(d,                    // display pointer
				                  start.subwindow,      // which window to move
								  newX,                 // x coord 
								  newY,                 // y coord
								  a.width,              // width of the window
								  a.height);            // height of the window

			}

			/* if it was a right mouse click, resize the window 
			 * by applying the mouse movement difference to the width
			 * and height of the window */
			else if( start.button == 3){

				printf("Resize Event!\n");

				int newWidth = a.width + dX;   // the updated width and height of the window
				int newHeight = a.height + dY;

				if( newWidth  <= 0 ) newWidth  = MIN_WIDTH; // make sure the difference doesn't result in a window with no size
				if( newHeight <= 0 ) newHeight = MIN_HEIGHT;

				/* make sure the window doesn't move offscreen */
				if( a.x + newWidth > XDisplayWidth(d, DefaultScreen(d)) ) {
					newWidth = XDisplayWidth(d, DefaultScreen(d)) - a.x;
				}
				if( a.y + newHeight > XDisplayHeight(d, DefaultScreen(d))) {
					newHeight = XDisplayHeight(d, DefaultScreen(d)) - a.y;
				}

				XMoveResizeWindow(d,                    // display pointer
			    	              start.subwindow,      // which window to move
								  a.x,                  // x coord 
								  a.y,                  // y coord
								  newWidth,             // width of the window
								  newHeight);           // height of the window

			}
		}
		
		else if(e.type == CreateNotify){
			printf("Create notify event!\n");
		}
		/* TODO - destroy frame when the window is destroyed */
		/* resize window if necessary*/
		else if (e.type == ConfigureRequest){
			XWindowChanges changes;
			  changes.x = e.xconfigurerequest.x;
			  changes.y = e.xconfigurerequest.y;
			  changes.width = e.xconfigurerequest.width;
			  changes.height = e.xconfigurerequest.height;
			  changes.border_width = e.xconfigurerequest.border_width;
			  changes.sibling = e.xconfigurerequest.above;
			  changes.stack_mode = e.xconfigurerequest.detail;
			  /*if (clients_.count(e.window)) {
				const Window frame = clients_[e.window];
				XConfigureWindow(display_, frame, e.value_mask, &changes);
				LOG(INFO) << "Resize [" << frame << "] to " << Size<int>(e.width, e.height);
			  }*/
			  XConfigureWindow(d, e.xconfigurerequest.window, e.xconfigurerequest.value_mask, &changes);
			  //LOG(INFO) << "Resize " << e.window << " to " << Size<int>(e.width, e.height);
			  printf("Resize: (%d, %d), X,Y: (%d,%d)\n", e.xconfigurerequest.width, e.xconfigurerequest.height, e.xconfigurerequest.x, e.xconfigurerequest.y);
		}
		
		else if(e.type == MapRequest){
			printf("Map request!\n");
			reparent_window(d, e.xmaprequest.window, False);
			XMapWindow(d, e.xmaprequest.window);
		}

		else if(e.type == UnmapNotify){
			printf("Unmap notify event!\n");
			
			/* if im understanding xlib.pdf right, 'event' is the 
			 * unmapped window or its parent, depending if
			 * StructureNotify or SubstructureNotify was selected */
			printf("Before unmap window!\n");
			XUnmapWindow(d, e.xunmap.event);
			XDestroyWindow(d, e.xunmap.event);
			printf("After unmap window!\n");

			// free allocated memory for children
			//if(c) XFree(c);
		}

		/* when the window is destroyed, destroy the parent */
		else if(e.type == DestroyNotify){
			printf("Destroy notify event!\n");

		}

		/* if we relase the mouse or keyboard keys,
		 * make sure there is no active window for events */
		else if( e.type == ButtonRelease ){
			printf("Button Release Event!\n");
			start.subwindow = None;
		}
		
		printf("Window X = %d\n", a.x);
		printf("Window Y = %d\n", a.y);
		printf("Window W = %d\n", a.width);
		printf("Window H = %d\n", a.height);

		printf("Display Width  = %d\n", XDisplayWidth(d,DefaultScreen(d)));
		printf("Display Height = %d\n", XDisplayHeight(d,DefaultScreen(d)));


	} while(1);

	/* disconnect from the display */
	if(d) XCloseDisplay(d);

	return 0;
}

