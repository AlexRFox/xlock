/*
 * xlock.c by Alex Willisson, 2011
 *
 * Derived from Ian Jackson's xtrlock (X Transparent Lock),
 * simplified, collapsed into a single file and modified to have a
 * password inputted as the first argument instead of using the user's
 * password
 *
 * The majority of the coding is Ian Jackson's, I only coded the input
 * parsing and picked which parts of Ian Jackson's code I wanted to
 * keep
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

Display *display;
Window window;

#define lock_width 28
#define lock_height 40
#define lock_x_hot 14
#define lock_y_hot 21
static char lock_bits[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xf8, 0xff, 0x7f, 0x00, 0xe0, 0xff,
	0x3f, 0x00, 0xc0, 0xff, 0x1f, 0x00, 0x80, 0xff, 0x0f, 0xfc, 0x03, 0xff,
	0x0f, 0xfe, 0x07, 0xff, 0x0f, 0xff, 0x0f, 0xff, 0x07, 0xff, 0x0f, 0xfe,
	0x87, 0xff, 0x1f, 0xfe, 0x87, 0xff, 0x1f, 0xfe, 0x87, 0xff, 0x1f, 0xfe,
	0x87, 0xff, 0x1f, 0xfe, 0x87, 0xff, 0x1f, 0xfe, 0x87, 0xff, 0x1f, 0xfe,
	0x87, 0xff, 0x1f, 0xfe, 0x87, 0xff, 0x1f, 0xfe, 0x87, 0xff, 0x1f, 0xfe,
	0x87, 0xff, 0x1f, 0xfe, 0x01, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00, 0xf8,
	0x01, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00, 0xf8, 0x01, 0xf0, 0x00, 0xf8,
	0x01, 0xf8, 0x01, 0xf8, 0x01, 0xf8, 0x01, 0xf8, 0x01, 0xf8, 0x01, 0xf8,
	0x01, 0xf8, 0x01, 0xf8, 0x01, 0xf0, 0x00, 0xf8, 0x01, 0x60, 0x00, 0xf8,
	0x01, 0x60, 0x00, 0xf8, 0x01, 0x60, 0x00, 0xf8, 0x01, 0x60, 0x00, 0xf8,
	0x01, 0x60, 0x00, 0xf8, 0x01, 0x60, 0x00, 0xf8, 0x01, 0x00, 0x00, 0xf8,
	0x01, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x00, 0xf8,
	0xff, 0xff, 0xff, 0xff};

#define mask_width 28
#define mask_height 40
#define mask_x_hot 14
#define mask_y_hot 21
static char mask_bits[] = {
   0x00, 0xfe, 0x07, 0x00, 0x80, 0xff, 0x1f, 0x00, 0xc0, 0xff, 0x3f, 0x00,
   0xe0, 0xff, 0x7f, 0x00, 0xf0, 0xff, 0xff, 0x00, 0xf8, 0xff, 0xff, 0x01,
   0xf8, 0x03, 0xfc, 0x01, 0xf8, 0x01, 0xf8, 0x01, 0xfc, 0x01, 0xf8, 0x03,
   0xfc, 0x00, 0xf0, 0x03, 0xfc, 0x00, 0xf0, 0x03, 0xfc, 0x00, 0xf0, 0x03,
   0xfc, 0x00, 0xf0, 0x03, 0xfc, 0x00, 0xf0, 0x03, 0xfc, 0x00, 0xf0, 0x03,
   0xfc, 0x00, 0xf0, 0x03, 0xfc, 0x00, 0xf0, 0x03, 0xfc, 0x00, 0xf0, 0x03,
   0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f,
   0xff, 0xff, 0xff, 0x0f};

#define MAXGOODWILL 150000
#define GOODWILLPORTION 0.3
#define TIMEOUTPERATTEMPT 30000

int
main (int argc, char **argv) {
	XSetWindowAttributes attrib;
	Pixmap csr_source,csr_mask;
	int ret, clen, rlen;
	XColor csr_fg, csr_bg, dummy;
	Cursor cursor;
	XEvent ev;
	double timeout, goodwill;
	char cbuf[10], rbuf[128], *pw;
	KeySym ks;

	if (argc > 1) {
		pw = malloc (sizeof (argv[1]));
		pw = argv[1];
		printf ("X locked started with password: %s\n", pw);
		printf ("Type the password to unlock X\n");
	} else {
		printf ("usage: xlock password-to-unlock\n");
	}

	timeout = 0;
	rlen = 0;
	goodwill = 150000;

	display = XOpenDisplay (0);

	if (display == NULL) {
		fprintf (stderr,"xlock: cannot open display\n");
		exit (1);
	}

	attrib.override_redirect= True;
	window= XCreateWindow(display,DefaultRootWindow(display),
			      0,0,1,1,0,CopyFromParent,InputOnly,CopyFromParent,
			      CWOverrideRedirect,&attrib);
                        
	XSelectInput(display,window,KeyPressMask|KeyReleaseMask);

	csr_source= XCreateBitmapFromData(display,window,lock_bits,lock_width,lock_height);
	csr_mask= XCreateBitmapFromData(display,window,mask_bits,mask_width,mask_height);

	ret = XAllocNamedColor(display,
			       DefaultColormap(display, DefaultScreen(display)),
			       "steelblue3",
			       &dummy, &csr_bg);
	if (ret==0)
		XAllocNamedColor(display,
				 DefaultColormap(display, DefaultScreen(display)),
				 "black",
				 &dummy, &csr_bg);

	ret = XAllocNamedColor(display,
			       DefaultColormap(display,DefaultScreen(display)),
			       "grey25",
			       &dummy, &csr_fg);
	if (ret==0)
		XAllocNamedColor(display,
				 DefaultColormap(display, DefaultScreen(display)),
				 "white",
				 &dummy, &csr_bg);



	cursor= XCreatePixmapCursor(display,csr_source,csr_mask,&csr_fg,&csr_bg,
				    lock_x_hot,lock_y_hot);

	XMapWindow(display,window);
	if (XGrabKeyboard(display,window,False,GrabModeAsync,GrabModeAsync,
			  CurrentTime)!=GrabSuccess) {
		exit(1);
	}
	if (XGrabPointer(display,window,False,(KeyPressMask|KeyReleaseMask)&0,
			 GrabModeAsync,GrabModeAsync,None,
			 cursor,CurrentTime)!=GrabSuccess) {
		XUngrabKeyboard(display,CurrentTime);
		exit(1);
	}

	for (;;) {
		XNextEvent(display,&ev);
		switch (ev.type) {
		case KeyPress:
			if (ev.xkey.time < timeout) { XBell(display,0); break; }
			clen= XLookupString(&ev.xkey,cbuf,9,&ks,0);
			switch (ks) {
			case XK_Escape: case XK_Clear:
				rlen=0; break;
			case XK_Delete: case XK_BackSpace:
				if (rlen>0) rlen--;
				break;
			case XK_Linefeed: case XK_Return:
				if (rlen==0) break;
				rbuf[rlen]=0;
				if (strcmp (rbuf, pw) >= 0) goto loop_x;
				XBell(display,0);
				rlen= 0;
				if (timeout) {
					goodwill+= ev.xkey.time - timeout;
					if (goodwill > MAXGOODWILL) {
						goodwill= MAXGOODWILL;
					}
				}
				timeout= -goodwill*GOODWILLPORTION;
				goodwill+= timeout;
				timeout+= ev.xkey.time + TIMEOUTPERATTEMPT;
				break;
			default:
				if (clen != 1) break;
				/* allow space for the trailing \0 */
				if (rlen < (sizeof(rbuf) - 1)){
					rbuf[rlen]=cbuf[0];
					rlen++;
				}
				break;
			}
			break;
		default:
			break;
		}
	}
loop_x:
	exit(0);
}
