/* File: pcx11e.c
 *
 * Implements PC/X11e GrADS console and command line interface.
 * Redirects stdout and stderr into a X console window.
 * It relies on "gvwm", the GrADS Virtual Window Manager, a slight
 * costumization of "fvwm" which replaces "system()" with "gacmd()".
 *
 * Based on console.c from the Xlibemu distribution.
 * Modified for PC/X11e GrADS by Arlindo da Silva.
 *
 * REVISION HISTORY:
 *
 * 15Mar1997   da Silva   First crack (GrADS v1.6b9)
 * 28Dec1997   da Silva   Adapted for DJGPP v2 & GrADS 1.7b6;
 *                        notice that XLookupString() is broken.    
 * 28Jan1998   da Silva   Fixed gets() bug affecting pull command.
 * 01Feb1998   da Silva   Allowed any resize; made scroll-up
 *                        of 1 line at a time; introduced ^L
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"

/* If autoconfed, only include malloc.h when it's presen */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#else /* undef HAVE_CONFIG_H */

#include <malloc.h>

#endif /* HAVE_CONFIG_H */


#ifdef XLIBEMU

/*--------- These are GrADS specific --------*/
#include "grads.h"
#include "gx.h"
extern struct gacmn gcmn;
extern char x11_initialized;
       int  got_grads_cmd=0;
       char grads_cmd[1024];
static int  gptr=0;
/* ------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "history.h"

static char my_stdin_iobuf[BUFSIZ];
static char my_stdout_iobuf[BUFSIZ];
static char my_stderr_iobuf[BUFSIZ];

static char *program = "GrADS Prompt";
static Display *condpy = NULL;
static int screen = 0;
static Window win;
static XSizeHints hints;
static GC gc_normal;
static GC gc_cursor;
static int nrows, ncols;
static int row = -1, col = -1;
static XFontStruct *font;
static unsigned int font_width, font_height, font_ascent;
static unsigned int width, height;

#define MAX_NCOLS 100
#define MAX_NROWS 40
static char *text;
static char *cursor = " ";
static int  plen=0;        /* prompt length */

static char *my_stdout_name = NULL;
static FILE *my_stderr = NULL;
static FILE *my_stdout = NULL;
static long my_stderr_pos = 0l;
static long my_stdout_pos = 0l;

static char *my_stdin_name = NULL;
static FILE *my_stdin = NULL;
static long my_stdin_pos = 0L;
static char my_stdin_buf[512] = "";

static void
Init (int argc, char **argv)
{
  int background, foreground;
  long event_mask;
  int dw, dh;

  condpy = XOpenDisplay ("");
  if (condpy == NULL)
    {
      fprintf (stderr, "Cannot open display\n");
      exit (1);
    }
  screen = DefaultScreen (condpy);
  dw = DisplayWidth(condpy, screen);
  dh = DisplayHeight(condpy, screen);

  /* cope with PC low res */
  if ( dw < 800 ) {
       font = XLoadQueryFont (condpy, "xm6x10b");
  } else {
       font = XLoadQueryFont (condpy, "8x14");
  }

  ncols = 80;
#if defined(STNDALN)
  nrows = 10;
#else
  nrows = 24;
#endif

  text = (char *) calloc (MAX_NROWS * MAX_NCOLS, 1);

  font_width = font->max_bounds.width;
  font_ascent = font->max_bounds.ascent;
  font_height = font->max_bounds.ascent + font->max_bounds.descent;
  width = ncols * font_width;
  height = nrows * font_height;

  background = WhitePixel (condpy, screen);
  foreground = BlackPixel (condpy, screen);

  hints.flags = 0;
  hints.x = DisplayWidth (condpy, screen) - width - 20;
  hints.y = DisplayHeight (condpy, screen) - height - 40; 
  hints.flags |= PPosition;
  hints.width = width;
  hints.height = height;
  hints.flags |= PSize;
  hints.min_width = font_width * 20;
  hints.min_height = font_height * 5;
  hints.flags |= PMinSize;
  hints.max_width = font_width * MAX_NCOLS;
  hints.max_height = font_height * MAX_NROWS;
  hints.flags |= PMaxSize;
  hints.width_inc = font_width;
  hints.height_inc = font_height;
  hints.flags |= PResizeInc;

  win = XCreateSimpleWindow (condpy,
			     DefaultRootWindow (condpy),
			     hints.x, hints.y,
			     hints.width, hints.height, 4,
			     foreground, background);
  event_mask = EnterWindowMask 
    |LeaveWindowMask 
    |ButtonPressMask
    |KeyPressMask
    |StructureNotifyMask
    |ExposureMask;

  XSelectInput (condpy, win, event_mask);

  gc_normal = XCreateGC (condpy, (Drawable) win, 0, 0);
  XSetFont (condpy, gc_normal, font->fid);
  XSetBackground (condpy, gc_normal, background);
  XSetForeground (condpy, gc_normal, foreground);

  gc_cursor = XCreateGC (condpy, (Drawable) win, 0, 0);
  XSetFont (condpy, gc_cursor, font->fid);
  XSetBackground (condpy, gc_cursor, foreground);
  XSetForeground (condpy, gc_cursor, background);

  XSetStandardProperties (condpy, win,
			  program, program,
			  None,
			  argv, argc, &hints);
  XMapWindow (condpy, win);

  
}

static void
Resize(int new_cols, int new_rows)
{
  int i, nc, nr;
  char *new_text;

  if (new_cols == ncols && new_rows == nrows)
    return;

  new_text = (char *) calloc (new_cols * new_rows + 1, 1);
  if (new_text == NULL)
  return;

  // nc = (ncols > new_cols) ? new_cols : ncols;
  // nr = (nrows > new_rows) ? new_rows : nrows;

  nc = new_cols;
  nr = new_rows;

  for (i = 0; i < nr; i++)
    {
      memcpy (new_text + (i * new_cols), text + (i * ncols), nc);
    }
  free (text);
  text = new_text;
  ncols = nc;
  nrows = nr;
}


static void
Redraw()
{
  int i, length, y, x;
  char *p;

  for(i=0;i<MAX_NROWS*MAX_NCOLS;i++) {
      if(!isprint(text[i])) {
         text[i] = 0;
         break;
      }
  }


  y = 0;
  p = text;
  for (i = 0; i < nrows; i++)
    {
      for (length = 0;
	   length < ncols && p[length] != 0;
	   length++);
      if (length > 0)
	{
	  XDrawImageString (condpy, win, gc_normal,
			    0,  y + font_ascent - 1,
			    p, length);
	}

      if (length < ncols) 
	{
	  x = length * font_width;
	  XClearArea (condpy, win,
		      x, y-1,
		      width - x, font_height+1,
		      0);
	}

        if (row == i) {
	    XDrawImageString ( condpy, win, gc_cursor,
			      font_width * col, y + font_ascent-1,
			      cursor, 1);
	  }

      y += font_height;
      p += ncols;
    }
}

static void
RedrawRow()
{
  int length, y, x;
  char *p;

  y = row*font_height;
  p = text + ncols*row;

  /* Draw the row proper */
  for (length = 0;
       length < ncols && p[length] != 0;
       length++);
  if (length > 0)
     {
	  XDrawImageString (condpy, win, gc_normal,
			    0,  y + font_ascent - 1,
			    p, length);
     }

     /* clean up to EOL */
     if (length < ncols) 
	{
	  x = length * font_width;
	  XClearArea (condpy, win,
		      x, y-1,
		      width - x, font_height+1,
		      0);
	}

     /* Draw the cursor */
     XDrawImageString ( condpy, win, gc_cursor,
	  	        font_width * col, y + font_ascent-1,
			cursor, 1);

}



static void 
ScrollUp (int lines)
{
  if (lines < 0)
    return;

  if (lines >= nrows) {
    memset (text, 0, ncols * nrows);
    row = 0;
    col = 0;
  }
  else {
    char *src, *dst;
    
    dst = text;
    src = text + ncols * lines;
    memcpy (dst, src, ncols * (nrows - lines));
    dst += ncols * (nrows - lines);
    memset (dst, 0, ncols * lines);
    row -= lines;
  }
}

static void
PutChar (char c)
{
  char *p = text + ncols * row + col;
  int i;

  switch (c) {
  case '':
    col--;
    if (col < plen)  
      col = plen;
    break;
  case '':
    col++;
    if (col >= ncols)
      col = ncols - 1;
    break;
  case '':
    row++;
    if (row >= nrows)
      row = nrows - 1;
    break;
  case '':
    row--;
    if (row < 0)
      row = 0;
    break;
  case '':
    {
      char *dst = text + ncols * row + col;
      memcpy (dst, dst + 1, ncols - col - 1);
      // dst[ncols - col - 1] = ' ';
    }
    break;
  case '':
    memset (text, ' ', ncols * nrows );
    text[0]='\0';
    Redraw();    
    row = 0;
    col = 4;
    strcpy(text,"ga> ");
    break;
  case '\n':
    // if (col < (ncols - 1)) *p = 0;
    row++;
    if (row >= nrows)
      {
	// ScrollUp (nrows / 2);
	ScrollUp (1);
      }
  case '\r':
    col = 0;
    break;
  case '\t':
    do PutChar (' '); while ((col % 8) != 0);
    break;
  case '\b':
    col--;
    if (col < plen) {
        col = plen;
    }
    else {
      char *dst = text + ncols * row + col;
      memcpy (dst, dst + 1, ncols - col - 1);
      dst[ncols - col - 1] = ' ';
    }
    break;
  default:
    *p = c;
    col++;
    if (col >= ncols)
      {
	col = 0;
	row ++;
	if (row >= nrows)
	  {
	    // ScrollUp (nrows / 2);
	    ScrollUp (1);
	  }
      }
    break;
  }
}

static void
PutString (char *string)
{
  char c;

  while ((c = *string++) != 0)
    PutChar (c);
}

static void
ProcessInput()
{
  int got_something = 0;
  long pos;
  char buf[512];

  if (my_stdout) {
    fgetpos (stdout, &pos);
    if (pos > my_stdout_pos) {
      fsetpos (my_stdout, &my_stdout_pos);
      while (fgets (buf, sizeof(buf)-1, my_stdout))
	{
	  buf[sizeof(buf)-1] = 0;
	  PutString (buf);
	  got_something = 1;
	}
      fgetpos (my_stdout, &my_stdout_pos);
      fsetpos (stdout, &pos);
    }
  }
  if (my_stderr) {
    fgetpos (stderr, &pos);
    if (pos > my_stderr_pos) {
      fsetpos (my_stderr, &my_stderr_pos);
      while (fgets (buf, sizeof(buf)-1, my_stderr))
	{
	  buf[sizeof(buf)-1] = 0;
	  PutString (buf);
	  got_something = 1;
	}
      fgetpos (my_stderr, &my_stderr_pos);
      fsetpos (stderr, &pos);
    }
  }
  if (got_something)
    Redraw ();
}

static void
ProcessOutput(char *string)
{
  long pos;

  if (my_stdin) {
    fgetpos (stdin, &pos);
    fsetpos (my_stdin, &my_stdin_pos);
    fputs (string, my_stdin);

      if (strchr (string, '\n'))
      fflush (my_stdin);  

    fgetpos (my_stdin, &my_stdin_pos);
    fsetpos (stdin, &pos);
  }
}


static void
ProcessEvent()
{
  char intext[10];
  KeySym keysym;
  XComposeStatus cs;
  int i, n, count;
  XEvent ev;
  HIST_ENTRY *hist;  /* History entry */
  char is_hist;

  XNextEvent (condpy, &ev);
    
  is_hist = 0;
  switch (ev.type) {

  case Expose:
    if (ev.xexpose.count == 0)
      Redraw ();
    break;
  case ConfigureNotify:
    width = ev.xconfigure.width;
    height = ev.xconfigure.height;
    Resize (width / font_width, height / font_height);
    break;

  case KeyPress:
//    count = XLookupString (&ev.xkey, intext, 10, &keysym, &cs);
//    intext[count] = 0;
    count = 1;
    intext[0] = ev.xkey.keycode;
    intext[1] = 0;
    if (count == 1) {
      switch (intext[0]) {
      case '\r':
	intext[0] = '\n';
        break;
#if !defined(STNDALN)
      case '':
        hist = previous_history();
        is_hist = 1;
        break;
      case '':
        hist = next_history();
        is_hist = 1;
        break;
#endif
      }
    }

#if !defined(STNDALN)
    /* Process history */
    if ( is_hist ) {
       if(hist) {
          char *dst = text + ncols * row + 4;
          memset(dst,' ',col-3);                /* clean to EOL */
          col = 4;
          PutString(hist->line);
          PutChar (0);
          RedrawRow();
       }
       break;
    }
#endif

#ifdef OBSOLETE               /* as XLookupString is broken */
    else switch (keysym) {
    case XK_Return:
    case XK_Linefeed:
      intext[0] = '\n';
      intext[1] = 0;
      break;
    case XK_Tab:
      intext[0] = '\t';
      intext[1] = 0;
      break;
    case XK_BackSpace:
      intext[0] = '\b';
      intext[1] = 0;
      break;
    case XK_Delete:
      break;
    case XK_Left:
      break;
    case XK_Right:
      break;
    case XK_Down:
      break;
    case XK_Up:
      break;
    }
#endif /* OBSOLETE */

    if (intext[0] ) {
      if (intext[0] == '\n')
      {
          char *dst = text + ncols * row + plen; 
          strncpy(grads_cmd,dst,1024);
          got_grads_cmd = 1;    /* signal nxtcmd() that we got a command */
      }
      PutChar (intext[0]);
      RedrawRow();
      strcat (my_stdin_buf, intext);
      if ( intext[0] == '\n' ) {
	ProcessOutput (my_stdin_buf);
	my_stdin_buf[0] = 0;
      }
    }
    break;

  }
}

static void
Cleanup ()
{
  if (my_stderr) fclose (my_stderr);
  if (my_stdout) fclose (my_stdout);
  if (my_stdout_name)
    unlink (my_stdout_name);

  if (my_stdin) fclose (my_stdin);
  if (my_stdin_name)
    unlink (my_stdin_name);
}

static void
Redirect ()
{
  if (isatty (fileno(stdout)))
    {
      my_stdout_name = tempnam ("c:/", "cout");

      (void) freopen (my_stdout_name, "wb", stdout);
      (void) freopen (my_stdout_name, "wb", stderr);
    }
  my_stdout = fdopen (fileno(stdout), "r");
  my_stderr = fdopen (fileno(stderr), "r");

  /*ams No buffering ams*/
  /* setbuf(my_stdout, NULL );
  setbuf(my_stderr, NULL );
  setbuf(stdout, NULL );
  setbuf(stderr, NULL ); */

  setbuf(my_stdout, my_stdout_iobuf ); /* my own buffers */
  setbuf(my_stderr, my_stderr_iobuf );

  if (isatty (fileno(stdin)))
    {
      my_stdin_name = tempnam ("c:/", "cinp");
      (void) freopen (my_stdin_name, "rb", stdin);

      my_stdin = fdopen (fileno(stdin), "wb");

      setbuf(my_stdin, my_stdin_iobuf );

    }

  atexit (Cleanup);
}


static int
main_console (int argc, char *argv[])
{
  if (condpy == NULL)
    {
      Init (argc, argv);
      Redirect ();
    }
  ProcessInput ();
  /*ams  while (XPending (condpy)) ProcessEvent(); ams*/ 
  if (XPending (condpy)) ProcessEvent(); 
  return 1;
}

int
console()
{
  int argc;
  char *argv[2];
  argc = 1;
  argv[0] = "GrADS Prompt";
  argv[1] = NULL;

  fvwm();
  gxdeve(0);
  return main_console(argc, argv);
}


/* Retrieves the next command from the user.  Leading blanks
   are NOT stripped.  The number of characters entered before the
   CR is returned. Input is gotten from the Xlib queue by console() */

int nxtcmd (char *cmd, char *prompt) 
{
  int i, len, past=0, cnt=0;

  printf ("%s ",prompt);  /* this is actually redirected by console() */ 
  plen = strlen(prompt)+1;

  past = cnt = 0;
  if ( x11_initialized ) {
   got_grads_cmd = 0;           /* fresh as new */
   while (1) {
      console();        /* console manager: handles stdio */
      if(got_grads_cmd) {
        strcpy(cmd,grads_cmd);
	got_grads_cmd = 0;             /* fresh as new */
        len = strlen(cmd);
#if !defined(STNDALN)
        if(len>0) {
           add_history(cmd);                  /* add to the history list */
           history_set_pos(history_length);
        }
#endif
	return(len);
      }
    }

  } else {   /* do as in regular grads */

    while (1) {
      *cmd = getchar();
      if (*cmd == EOF) {
        return(-1);
      }
      if (*cmd == '\n') {
	cmd++;
	*cmd = '\0';
	return(cnt);  
      }
      if (past || *cmd != ' ') {
	cmd++; cnt++; past = 1;
      }
    }
  }
}

/* printf() replacement */

int
printf (const char *format, ...)
{
  char c, buffer[2048];
  va_list ap;
  va_start (ap, format);
  buffer[0] = 0;
  vsprintf (buffer, format, ap);
  va_end (ap);
  if ( x11_initialized ) {
       PutString (buffer);  
       Redraw(); 
       return(0);
  } else {
       fprintf(stdout,"%s",buffer);
       return(0);
  }
}

/* gets() replacement */

char *
gets ( char *str )
{

  if ( x11_initialized ) {
    plen=0;       /* no prompt */
    while (1) {
      console();                      /* console manager: handles stdio */
      if(got_grads_cmd) {
        strcpy(str,grads_cmd);
	got_grads_cmd = 0;           /* fresh as new */
	return(str);
      }
    }
  } else {
        return(fgets(str,256,stdin));
	       }
}

/* Wrapper around system(): need to restart event queue */

int
System ( const char *action )
{

  int rc;

   if ( x11_initialized ) {
      XSync(condpy,1);           /* syncronizes X event queue */
      _WQueueUnInit();        /* turn off event queue */
      rc = system(action);    /* go for it */
      _WQueueInit();          /* restart event queue */
      _WMouseDisplayCursor();
      XSync(condpy,1);           /* syncronizes X event queue */
  } else {
      rc = system(action);    /* go for it */
  }

  return(rc);

}

/* This functions handles Exec calls from the windows manager */

int
Grads_system ( char *action )
{
#ifndef STNDALN
  printf("%s\n",action);
  return(gacmd(action,&gcmn,0));
#else
  printf("Error: cannot execute %s from gxtran\n", action);
  return(0);         /* disable this function for gxtran */
#endif
}

#endif /* XLIBEMU */




