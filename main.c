/* simplest possible window manager for x11 
 *
 * William Blair
 * 04/19/17
 * */

#include <X11/Xlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	Display            *d = NULL;   // the main X Display to open
	XWindowAttributes   a;          // properties of the main window
	XButtonEvent        start;      // saves the cursor's position at the beginning of a move event
	XEvent              e;          // hold's X server events
	Bool                run = True; // False when the program is done running

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

	/* makes it so that there is no child event window of 
	 * the current event window */
	start.subwindow = None;

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
			
			/* if it was a left mouse click, move the window by
			 * applying the mouse movement difference to the
			 * x and y coordinates of the window */
			if( start.button == 1 ) {

				printf("Movement Event!\n");

				XMoveResizeWindow(d,                    // display pointer
			    	              start.subwindow,      // which window to move
								  a.x + dX,             // x coord 
								  a.y + dY,             // y coord
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

				if( newWidth  == 0 ) newWidth  = 1; // make sure the difference doesn't result in a window with no size
				if( newHeight == 0 ) newHeight = 1;

				XMoveResizeWindow(d,                    // display pointer
			    	              start.subwindow,      // which window to move
								  a.x,                  // x coord 
								  a.y,                  // y coord
								  newWidth,             // width of the window
								  newHeight);           // height of the window

			}
		}

		/* if we relase the mouse or keyboard keys,
		 * make sure there is no active window for events */
		else if( e.type == ButtonRelease ){
			printf("Button Release Event!\n");
			start.subwindow = None;
		}

	} while(1);

	/* disconnect from the display */
	if(d) XCloseDisplay(d);

	return 0;
}

