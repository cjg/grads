/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Note that a GrADS metafile is composed of messages, which themselves
   are composed of from one to five short integers.  In each case, 
   the first integer contains the message type, which also indicates
   how many arguments (and what type of arguments) follow.  Note that
   if the argument is in plot units, it needs to be divided by 1000
   to get floating point units.  */

#include <stdio.h>
#include <math.h>
#include "grads.h"
#include "gx.h"

struct gamfcmn mfcmn;

int help=0;
void command_line_help(void) ;

char buff[132];
int pnt;
FILE *infile;

int nxtcmd (char *, char *);
void xyccc (short, short, float *, float *);
int fflag;

/*mf 980115 -- 
  increase buffer from 1000 to 10000 - this is dynamically allocated in gxmeta
  gxtran was seg faulting for plots with BIG polygon files
mf*/

double xybuf[10000];

#if defined(XLIBEMU32)
GRXMain (int argc, char *argv[])  {
#else
int main (int argc, char *argv[])  {
#endif

short opts[4];
int cmd,i,j,iflag,rflag,aflag,gflag,wflag,rc;
int cont,xyc,lcolor,fflag,fcnt;
float xlo,xhi,ylo,yhi,xpos,ypos,xsiz,ysiz;
char in[100],*ifi;


  /* Parse command line arguments */

  i = 1;
  iflag = 0; rflag = 0; aflag = 0; gflag = 0; wflag = 0; xyc = 0;
  ifi = NULL;
  while (i<argc) {
    if (*(argv[i])=='-' && *(argv[i]+1)=='h' && *(argv[i]+2)=='e' && *(argv[i]+3)=='l' && *(argv[i]+4)=='p' ) {
      command_line_help();
      return(0);
    } else if (*(argv[i])=='-') {
      j = 1;
      while (*(argv[i]+j)) {
        if (*(argv[i]+j)=='i') iflag = 1;
        else if (*(argv[i]+j)=='i') iflag = 1;
        else if (*(argv[i]+j)=='r') rflag = 1;
        else if (*(argv[i]+j)=='a') aflag = 1;
        else if (*(argv[i]+j)=='g') gflag = 1;
        else printf ("Unknown flag %c; ignored.\n",*(argv[i]+j));
        j++;
      }
    } else {
      if (iflag) {ifi = argv[i]; iflag = 0;}
      else if (gflag) { gxdgeo(argv[i]); wflag = 1; gflag = 0; }
      else printf ("Unknown argument: %s.  Ignored.\n",argv[i]);
    }
    i++;
  }

  if (ifi==NULL) {
    command_line_help();
    nxtcmd (in,"Enter input file: ");
    ifi = in;
  }

  infile = fopen(ifi,"rb");
  if (infile == NULL) {
    printf ("Cannot open input file: %s\n",ifi);
    return 1;
  }

  pnt = 0;
  fcnt = 0;
  fflag = 0;

  cont = 1;
  while (cont) {
    
    /* Read the 1st integer, which is the message type */

    rc = fread (opts , sizeof(short), 1, infile);
    if (rc != 1) {
      printf ("Error on fread of message type\n");
      return 1;
    }
    cmd = opts[0];

    /* -9 indicates end of file.  Terminate graphics output.
       It is followed by 0 arguments */

    if (cmd==-9) {
      cont = 0;
      if (aflag) gxfrme(0);
      gxend();
    }

    /*  -1 indicates start of file.  It is followed by two arguments,
        which are the X and Y size of the virtual device in plotter
        units */ 

    else if (cmd==-1) {
      fread ((char *)opts, sizeof(short), 2, infile);
      xyccc (opts[0],opts[1],&xsiz,&ysiz);
      gxstrt (xsiz,ysiz,0,1000000);  /* default buffer size */
      if (rflag) gxdbck(1);          
      if (wflag) gxfrme(1);
      else gxfrme(0);
      if (aflag) gxfrme(2);
    }
  
    /* -2 indicates new frame.  It has no arguments */

    else if (cmd==-2) {
      gxfrme(9);
      if (aflag) gxfrme(2);
      else gxfrme(0);
    }

    /* -3 indicates new color.  It has one argument, a color number.
       Colors 0 to 15 are predefined; colors 16 through 99 are 
       allowed to be user defined (colors 100 through 255 are 
       reserved for image use, a feature under development). */
  
    else if (cmd==-3) {
      fread ((char *)opts, sizeof(short), 1, infile);
      lcolor = opts[0];
      gxcolr(lcolor);
    }

    /*  -4 indicates new line thickness.  It has one argument, 
        where 1 indicates the thinnest line, 10 indicates a fairly
        thick line.  Postscript Line thicknesses (used by gxps):
  
                    " 0.001 w",
                    " 0.006 w",
                    " 0.009 w",
                    " 0.012 w",
                    " 0.015 w",
                    " 0.018 w",
                    " 0.021 w",
                    " 0.024 w",
                    " 0.027 w",
                    " 0.030 w",
                    " 0.033 w",
                    " 0.036 w"    

         Line weights greater than 12 are just set to 0.036.  */
 
    else if (cmd==-4) {
      fread ((char *)opts, sizeof(short), 2, infile);
      i = opts[0];
      gxwide(i);
    }

    /*  -5 defines a new color, in rgb.  it is followed by  four
        args:  color number (16 to 99), then red, green, blue, 
        each ranging from 0 to 255 */

    else if (cmd==-5){
      fread ((char *)opts, sizeof(short), 4, infile);
      i = opts[0];
      gxacol (i,opts[1],opts[2],opts[3]);
    }

    /*  -6 is for a filled rectangle.  It is followed by four args, 
        each a plotting coordinate (divide by 1000 to get floating
        point arg).  The coords are:  xlo, xhi, ylo, yhi */
 
    else if (cmd==-6){
      fread ((char *)opts, sizeof(short), 4, infile);
      xyccc (opts[0],opts[2],&xlo,&ylo);
      xyccc (opts[1],opts[3],&xhi,&yhi);
      gxrecf(xlo,xhi,ylo,yhi);
    }

    /* -7 indicates the start of a polygon fill.  It is followed
       by the polygon outline, provided as move and draw instructions,
       which are followed by the -8 command.  This command has
       one arg, the length. */

    else if (cmd==-7){
      fread ((char *)opts, sizeof(short), 1, infile);
      fflag = 1;
      xyc = 0;
    }

    /*  -8 is to terminate polygon fill.  It has no args */

    else if (cmd==-8){
      gxfill (xybuf,xyc);
      fflag = 0;
    }

    /* -10 is a move to instruction.  It is followed by two args, 
       the X and Y position to move to, in plotting units.  If this
       is between a -7 and a -8 command, then it is part of the 
       polygon outline to be filled */

    else if (cmd==-10){
      fread ((char *)opts, sizeof(short), 2, infile);
      xyccc (opts[0],opts[1],&xpos,&ypos);
      if (fflag) {
        xybuf[xyc*2] = xpos;
        xybuf[xyc*2+1] = ypos;
        xyc++;
      } else gxplot(xpos,ypos,3);
    }

    /*  -11 is draw to.  It is followed by two instructions.  
        If between a -7 and -8 instruction, it is part of a polygon
        to be filled. */ 
        
    else if (cmd==-11){
      fread ((char *)opts, sizeof(short), 2, infile);
      xyccc (opts[0],opts[1],&xpos,&ypos);
      if (fflag) {
        xybuf[xyc*2] = xpos;
        xybuf[xyc*2+1] = ypos;
        xyc++;
      } else {
	gxplot(xpos,ypos,2);
      }
    }

    else if (cmd==-20) {       /* Draw button -- ignore */
      fread ((char *)opts, sizeof(short), 1, infile);
    }

    /* Any other command would be invalid */

    else {
      printf ("Invalid command found %i \n",cmd);
      return 1;
    }
  }
  return 0;
}

#if !defined(XLIBEMU)
int nxtcmd (char *cmd, char *prompt) {
int past,cnt;

  printf ("%s ",prompt);
  past = 0;
  cnt = 0;
  while (1) {
    *cmd = getchar();
    if (*cmd == EOF) return (-1);
    if (*cmd == '\n') {
      *cmd = '\0';
      return (cnt);
    }
    if (past || *cmd != ' ') {
      cmd++; cnt++; past = 1;
    }
  }
}
#endif

void xyccc (short ix, short iy, float *x, float *y) {

  *x = ((float)ix)/1000.0;
  *y = ((float)iy)/1000.0;
}


void command_line_help(void) {
/*--- 
  output command line options 
---*/

printf("gxtran for GrADS Version " GRADS_VERSION "\n\n");
printf("Display a GrADS meta file (from the \"print\" command in grads)\n\n");
printf("Command line options: \n\n");
printf("          -help   Just this help\n");
printf("          -i      input GrADS meta file\n");
printf("          -o      output postscript file\n");
printf("          -r      reverse background (typically black) default is white background\n");
printf("          -g LLLLxHHHH+(-)XXXX+(-)YYYY  set size of graphics window (like X windows)\n");
printf("                LLLL -- length of box in pixels (x side)\n");
printf("                HHHH -- height of box in pixels (y side)\n");
printf("       + or -   XXXX -- starting pixel point in x (0,0 is upper lefthand corner)\n");
printf("       + or -   YYYY -- starting pixel point in y\n");
printf("          -a      \"animate\" by displaying all images as fast as possible\n");
printf("   Example:\n\n");
printf("   gxtran -r -i myplot.gm\n\n");
printf("   displays the plot(s) in the GrADS meta file mplot.gm \n\n");

}

void gaprnt (int i, char *ch) {
  printf ("%s",ch);
}
