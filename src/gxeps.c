
/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/***********************************************************
 * GXEPS a grads metafile to PostScript converter
 * Copyright (c) 1999 - 2001 Matthias Munnich
 *
 * I regard gxeps as freeware as long as proper credit is given.
 * As I often read lengthy copyright notices I adjusted
 * one which seemed fitting. Here it goes:
 * * Permission has been granted to copy and distribute gxeps in any 
 * context, including a commercial application, provided that this notice 
 * is present in user-accessible supporting documentation.
 * 
 * This does not affect your ownership of the derived work itself, and 
 * the intent
 * is to assure proper credit for the authors of gxeps, not to interfere
 * with your productive use of gxeps. If you have questions, ask. 
 * "Derived works" includes all programs that utilize gxeps.
 * Credit must be given in user-accessible documentation.
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation.  This software is provided "as is" without express or 
 * implied warranty.
 * 
 * Matthias Munnich mailto:munnich@atmos.ucla.edu
 * ==================================================
 *
 *   $Log: gxeps.c,v $
 *   Revision 1.11  2009/01/05 12:48:24  jma
 *   changed
 *   #include <config.h>
 *   to
 *   #include "config.h"
 *
 *   Revision 1.10  2008/11/07 19:03:55  jma
 *   removed all the #ifdefs for PRINT_EPS
 *   this is now permanently enabled so you can
 *   create postcript files using 'print' without 'enable print'
 *
 *   Revision 1.9  2008/01/23 13:04:49  jma
 *   removed files, no more support of USEIMG
 *
 *   Revision 1.8  2007/09/13 13:58:42  jma
 *   First and incomplete stab at double precision data handling and undef mask.
 *   Compiles with few warnings on OSX, checked in now for testing on linux.
 *
 *   Revision 1.7  2007/05/31 15:45:28  jma
 *   Changes to the configure system -- updated for dods 3.4 as the default. 
 *   Some other minor changes to src files to add #include statements to remove compiler warnings.
 *
 *   Revision 1.6  2004/02/27 14:42:11  administrator
 *   Changes to RGB color settings in gxps.c and gxeps.c to match the colors in
 *   gxX.c, gxhpng.c, and the documentation. This should make image and postscript
 *   ouput look just like the graphical display.
 *
 *   Revision 1.5  2003/11/10 21:27:39  jma
 *   Fix to gxeps.c to add graylines (-G option) which fixes hardcopy output from basemap.gs
 *
 *   Revision 1.4  2003/04/04 01:52:30  joew
 *   Mod from Matthias Munnich:
 *
 *   -DMM_NEW_PROMPT
 *       - The grads prompt is changed to "ga[command number]> ", e.g., "ga[17]> ".
 *
 *   -DMM_READLINE Enhanced READLINE.
 *   Command line enhancements:
 *       - New "history" or "his" command:
 *         "his[tory] [-sqx] [first] [last] [filename]" writes commands "first" to "last"
 *         in the command history buffer to file "filename".
 *         "his[tory] -r filename" reads commands from file "filename" into the command
 *         history buffer.
 *
 *          Options:
 *           -s  Write history in script syntax (quoted).
 *           -x  Write history unquoted for "exec" command.
 *           -q  Write quitely.
 *           -r  Read history from file.
 *
 *         For files with extension ".gs" the history is written quoted (option "-s").
 *         Examples:
 *            "ga[17]> his" list the current command history.
 *            "ga[17]> his 17 27 foo.gs" writes quoted commands 17 through 27 to file
 *            foo.gs so that ga[17]> foo will reproduce the current state.
 *
 *       - New startup option "-H [history_log_file]":
 *         At startup the last 256 commands in history_log_file are read into the command
 *         history buffer.  When GrADS is terminated the new history is written to
 *         history_log_file.
 *         The default history_log_file is $HOME/.grads.log. (This may need adjustment
 *         for Windows).
 *
 *       - New "repeat" or "r" command: r[epeat] [start] stop
 *         Examples:
 *         "ga[17]> r 21"  repeats command number 21. "ga[17]> r 20 30" repeats commands
 *         20 to 30 skipping r commands to avoid infinite loops.
 *
 *   Revision 1.3  2003/04/03 19:46:27  joew
 *   Mod from Matthias Munnich: improved EPS output
 *
 *   Revision 1.6  2001/12/17 23:37:15  munnich
 *   Symbol fonts are now always drawn. To this end the string information
 *   where moved to command -15 (begin string).
 *
 *   Revision 1.5  2001/12/07 00:40:42  munnich
 *   Fixed bugs with gxheps.
 *
 *   Revision 1.4  2001/12/04 05:37:53  munnich
 *   Incorporated gxeps into the print command. If printing is
 *   not yet "enabled" the print command now uses gxeps code
 *   to directly produce EPS files. Most of the standard gxeps
 *   options are valid. Help is given via ga-> print -h or any
 *   invalid option.
 *   Affected files: Makefile, gauser.c, gx.h, gxmeta.c
 *
 *   Also changed the map drawing to use polygons if clip is hard.
 *   Affected: gxwmap.c
 *   RCS file: /long/munnich/CVSROOT/grads/cola/src/Makefile.in,v
 *   RCS file: /long/munnich/CVSROOT/grads/cola/src/gauser.c,v
 *   RCS file: /long/munnich/CVSROOT/grads/cola/src/gx.h,v
 *   cvs diff: gxeps.c is a new entry, no comparison available
 *   RCS file: /long/munnich/CVSROOT/grads/cola/src/gxmeta.c,v
 *   RCS file: /long/munnich/CVSROOT/grads/cola/src/gxwmap.c,v
 *
 *   Revision 0.94  2001/12/02 06:22:59  munnich
 *   gxeps.c
 *
 *   Revision 0.93  2001/12/02 06:20:55  munnich
 *   Fixed Filled rectangle bug for RSCALE != 2
 *
 *   Revision 0.92  2001/11/24 04:48:25  munnich
 *   Added clipboundary to bounding box computation.
 *   Added meta file draw line command (-13) to search for Polygons.
 *   Took care that the gsave/grestore pairs in clipping match at the end.
 *
 *   Revision 0.91  2001/11/22 02:17:23  munnich
 *   Back to 1000dpi resolution due to visible roundoff errors.
 *   Added IsoLatin1 support for PostScript level 2.
 *   Made PS Level 2 default.
 *
 *   Revision 0.90  2001/11/21 07:03:28  munnich
 *   Fixed a hardware clipping bug.
 *
 *   Revision 0.88  2001/04/19 01:55:35  munnich
 *   Fixed a bug in rotated strings.-- 
 *   of grads as well.
 *
 *   Revision 0.87  2001/03/08 21:57:23  munnich
 *   Introduced hardware clipping, line styles, polygon and line drawing.
 *   Use hardware clipping to cut contour lines for labels.
 *   Right now smoothing of contours is disabled.
 *   To be done:
 *   1. Enable smoothing of contours.
 *   2. Make hardware clipping and line style a setting
 *      which can be changed by something like
 *      set clip hard/soft
 *      set style hard/soft
 *   3. Disgard overlapping lables (an old "bug").
 *
 *   Revision 0.86  2001/03/08 21:21:22  munnich
 *   Added support for new metafile commands:
 *   (*) -13    draw a polygon
 *   (*) -14    draw a line
 *   (*) -17    set line style (dashes)
 *   (*) -18    set/change clipping region
 *
 *   Revision 0.85  2001/02/01 21:23:45  munnich
 *   1. Another change to the memory reallocation for large polygons.
 *   Now it should always works.
 *   2. New macro PSFONTS to enable PostScript font support.
 *   3. Input can now come from stdin by using option -i -.
 *
 *   Revision 0.84  2001/01/27 06:57:04  munnich
 *   Fixed coordinate line thicknes bug.
 *
 *   Revision 0.83  2000/10/13 16:36:01  munnich
 *   Another bug fix related user defined colors.
 *
 *   Revision 0.82  2000/10/12 22:33:52  munnich
 *   Fixed a bug related to user defined colors and black&white mode.
 *
 *   Revision 0.81  2000/09/20 22:51:42  munnich
 *   Fixed rotated strings
 *
 *   Revision 0.80  2000/09/15 19:19:24  munnich
 *   Implemented "-g" option for true 8.5'' x 11'' inch output size.
 *
 *   Revision 0.79  2000/09/05 01:33:13  munnich
 *   Back to old scaling for letter paper.
 *
 *   Revision 0.78  2000/08/25 20:54:30  munnich
 *   Back to version 0.76. V0.77 was broken.
 *
 *   Revision 0.76  2000/06/02 23:45:10  munnich
 *   *** empty log message ***
 *
 *   Revision 0.75  2000/06/02 23:23:52  munnich
 *   Fixed Orientation comment.
 *
 *   Revision 0.74  2000/04/03 17:28:54  munnich
 *   Fixed 2 bugs:
 *   (i) gsave was not bracketed with a grestore when
 *   neither options -L nor -n were used.
 *   (ii) Undefined colors are now set to gray (no. 15).
 *
 *   There is also a rudimentary font support in gxeps.c.
 *   This only comes into affect when a new metafile format is
 *   introduced.  Everthing should be backward compatible.
 *
 *   Revision 0.73  1998/11/30 10:13:57  m211033
 *   Fixed a 2 bug: Lineswidth and colors can know
 *   change within a polygon. This bug only occurred with
 *   colored streamline plots.
 *
 *   Changed the line breaking method for polygons.
 *
 *   Revision 0.72  1997/12/08 14:16:15  m211033
 *   Added the -R option which suppresses the rotation
 *   for landscape metafiles.
 *
 *   Revision 0.71  1997/11/05 17:26:41  m211033
 *   Added a compile option NEVER_CTL_D which disables
 *   the -d option for DEFAULT_CTL_D 0, i.e. no ^D is added in
 *   any case.
 *
 *   Revision 0.70  1997/04/15 15:14:37  m211033
 *   Added a BORDER macro that controls the
 *   added white space (in points) around the graph
 *   for the bounding box comment.
 *
 * Revision 0.69  1997/04/09  14:38:39  m211033
 * Black background now as large as the bounding box.
 * BoundingBox corrected for black background.
 *
 *   Revision 0.68  1996/11/22 14:10:45  m211033
 *   BoundingBox takes filled rectangulars into account.
 *
 * Revision 0.67  96/11/19  16:46:19  m211033
 * Two bugs were fixed:
 * (i) with DEFAULT_CTL_D 1 the -d option didn't work.
 * (ii) Color change now leads to a flush of stored lines
 *     (Before sometimes a line was drawn in a invisible color.)
 * 
 * Revision 0.66  96/10/23  12:50:26  m211033
 * Fixed the data type of file position.
 * 
 * Revision 0.65  1996/06/26  15:20:29  m211033
 * Inserted a gsave to match the grestore.
 * This fixes an annotation bug.
 * Resolution is reduce to 500dpi since Version 0.64.
 *
 * Revision 0.64  1996/05/15  10:17:00  m211033
 * The cut of filled line did not work. No the old method
 * is used untill drawline and fillline is rewritten for
 * the use of reversepath.
 * Filline now also looks for colinear lines.
 *
 * Revision 0.63  1996/04/26  19:32:57  m211033
 * Got rid off the error prone reallocation for saved point.
 *
 * Revision 0.62  96/04/26  19:08:25  m211033
 * Fixed another bug with multible plots in one metafile.
 * 
 * Revision 0.61  1996/04/26  17:16:57  m211033
 * Now it tries to find straight lines in polygons.
 *
 * Revision 0.6  96/04/26  16:15:45  m211033
 * Tries to find dashed lines to make the EPS output
 * more compact. Unnecessary return(0) in while(1) eliminated.
 * 
 *
 ***********************************************************/
          /* default PS-level */
#define PS_LEVEL 2
          /* set this to 1 for default a4 paper */
#define DEFAULTA4 0
     /* default add control-D at end-of-file (some printer may need this) */
#define DEFAULT_CTL_D 0
   /* if NEVER_CTL_D is defined ^D will never be added (disables -d Option) */
#define NEVER_CTL_D
#define BORDER 10

/* PS font support */
#define PSFONTS 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define max(A, B) ((A) > (B) ? (A) : (B))
#define min(A, B) ((A) < (B) ? (A) : (B))


/* Some scales for letter paper */
/* RSCALE: resolution scale */
#define RSCALE 1
/* #define LETSCALE 0.1314 */
#define LETSCALE 0.0657 * RSCALE
#define LETXOFF 30
#define LETYOFF 45
/* GrADS scaling 8.'' x 11'' */
#define GASCALE 0.144 
/* Some scales for a4 paper */
#define A4SCALE 0.1134 
#define A4XOFF 71
#define A4YOFF 109
/* default file extensions */
#define IN_EXT "gm"
#define OUT_EXT "eps"

#define PNMAX 512
/* maximum string size */
#define STRSIZ 300
#ifdef OLD__GNUC__
#define fpos_t long
#endif

/* is also defined in gxhpng.c */
struct point {short x; short y;};
struct point bblow, bbhigh;     /* BoundingBox */
struct point cliplow, cliphigh; /* Clipping Box */
struct options {                /* command line options (flags) */
  short pscale, color, ctl_d, comment, pslevel,
    tstamp, reverse, rotate, verbose, label, font,
    history, graylines;
};
FILE *infile, *outfile;
fpos_t bboxpos,pagespos,pagepos,blackpos; /* output file position of boundingbox and page # comments */
void boundbox(struct point* pnt); /* compute bounding box */
void printbbox(short opscale,short landscape,struct options *o);  /* print bounding box to PS-file */
int print_options(char *argv[]); /* print command line options */
void drawline(struct point pnts[],short *pcnt,
	      short lcolor,short *colold,short *lastfill,short flush); /* draw a polygon */
int SaveLine(struct point pnts[],struct point dline[]);
void PrintDashLine(struct point dline[]);
short CheckDashLine(struct point pnts[],struct point dline[]);
void SetDline(struct point pnts[],struct point dline[]);
short CheckExpandDashLine(struct point pnts[],struct point dline[]);
int parse_arg(int argc,char *argv[],struct options *o,char **fin, char **fout); /* parse command line */
int openfiles(char **fin, char **fout, short verbose); /* Open files */
void fillline(struct point pnts[],short pcnt); /* fill a polygon */
void initepsf(short landscape,int argc,char *argv[],char *fout,struct options *o); /* write PS-file header */
void init_newpage(short landscape,char *fout,struct options *o); /* New page header */
void setjust(short *opts, short *justx, short *justy);
int gxheps(char*);
void getwrd (char *, char *, int);
int gxeps(int , char **);

#ifndef GXHEPS
#define RDSTREAM(var,num)  fread (var , sizeof(short), num, infile);
#define MAIN main
#else
#define MAIN gxeps
#define RDSTREAM(var,num)  var=poi; poi+=num;
char *nxtwrd (char *); 

/* --------------------- Wrapper for gxeps -------------------------- */
int gxheps(char *com) {
  char *args,**argv; 
  int argc=0;
  char *cmd,*ch;
  int i,rc;

  cmd=com;

  while( ((cmd=nxtwrd(cmd)) != NULL)) {
    argc++; 
  }

  argc+=1;

  argv=(char **) malloc(argc*sizeof(char **));
  args=(char *) malloc( (strlen(com)+5) *sizeof(char));
  for(i=0;i<argc;i++) argv[i]=NULL;

  /* copy words in com into args and assign pointers to each word */
  cmd=com;
  /* copy command (print) */
  i=0;
  ch=args;
  getwrd(ch,cmd,256);
  argv[i]=ch;
  ch+=strlen(ch)+1; 
  
  /* copy gxeps options */ 
  while(++i<argc) {
    cmd=nxtwrd(cmd);
    getwrd(ch,cmd,256); 
    argv[i]=ch;
    ch+=strlen(ch)+1;
  }

#ifdef DEBUG_EPS
  printf(" GXHEPS:\n");
  printf("argc = %d\n",argc);
  for(i=0;i<argc;i++)
      printf("argv[%d]= >%s<\n",i,argv[i]);
  printf("Calling gxeps...\n");
#endif
  rc=gxeps(argc, argv);
  rc=0;
  free(argv);
  free(args);
  return (rc);
}

#endif

/* --------------------------------------------------------------------------------- */
int MAIN (int argc, char *argv[])  {
   short cmd,coldef[100],lcolor=0, colold=-1, wold=-1, landscape=0, npnmax=1, lastfill=0;
   short oldstyle=0;
   struct point *pnts, *pnts2;            /* points of a polygon */
   struct options o;                      /* command line options */
   int nodraw=0;                          /* skip draw commands (for string drawing) */
   int pnmax=PNMAX;                        /* maximum length of a polygon */
   register int  i,j, ii,jj;
   short pcnt,pcntSave,fcnt=0,filflg=0;
   float red,blue,green;
   char *fin=NULL,*fout=NULL;              /* file names */
   char str[STRSIZ+50]; /* I hope nobody writes longer lines */
   short justx,justy; /* justification in % */
#ifndef GXHEPS
   fpos_t infile_pos;
   char string[STRSIZ]; /* I hope nobody writes longer lines */
   short opts[8];
#else
   char *string;
   short *opts;
  int cnt,flag,fflag,ib;
  short *poi,*pend;
#endif
   o.pscale=DEFAULTA4;                                           /* default options */
#ifdef NEVER_CTL_D
   o.ctl_d=0;
#else
   o.ctl_d=DEFAULT_CTL_D;
#endif
#ifdef PSFONTS
   o.font=1;
#else
   o.font=0;
#endif
   o.reverse=o.verbose=o.comment=o.tstamp=o.label=0;
   o.history=1;
   o.rotate=o.color=1;
   o.pslevel=PS_LEVEL;
   o.graylines=0;
   landscape=0;
   bblow.x=1200;bblow.y=1200;
   bbhigh.x=0; bbhigh.y=0;
   pnts=(struct point *) malloc(pnmax*sizeof(struct point)); /* initial maximal polygon size */

  for(i=0;i<16;i++) coldef[i]=1;          /* Initialize defined color flag*/
  for(i=16;i<100;i++) coldef[i]=0;

  if(parse_arg(argc,argv,&o,&fin,&fout))
    return(1);     /* Parse command line arguments */
  if(openfiles(&fin,&fout,o.verbose))
    return(1);                  /* open files */

  /* Translate the file */

#ifdef GXHEPS  
  /*  Set up pointers into current meta buffer list */

  if (dbmode && pntf==0) {
    lens2[pnt2-1] = hpnt-hbuff;
    cnt = pnt2; flag = 1;
  } else {
    lens[pnt-1] = hpnt-hbuff;
    cnt = pnt; flag = 0;
  }
  fflag = 0;

    /* Initialization (cmd ==-1) not in buffer. we do it here */

    landscape= (yrsize<xrsize);  /* ysize < xsize : landscape */
    if(o.rotate && landscape) o.rotate=1; /* no -R option and */
    else o.rotate=0; 
    
    initepsf(landscape,argc,argv,fout,&o);
    colold=-1;
    init_newpage(landscape,fout,&o);
    pcnt = 0;

  for (ib=0; ib<cnt; ib++) {
    if (flag) {
      poi = bufs2[ib];
      pend = poi + lens2[ib];
    } else { 
      poi = bufs[ib];
      pend = poi + lens[ib];
    } 

    while (poi<pend) {
    cmd= *poi; poi++;
#else 
      /* gxeps utility */
  fcnt = 1;
  while (1) {
    fread (&cmd , sizeof(short), 1, infile);
#endif

    if (cmd==-11) {				/* Draw to */
      /*      fread (opts, sizeof(short), 2, infile); */
      RDSTREAM(opts,2)
      if(nodraw) {
	pnts[pcnt+1].x=opts[0]/RSCALE;
	pnts[pcnt+1].y=opts[1]/RSCALE;
	boundbox(&pnts[pcnt+1]);
	if (lcolor != colold)  {
	  fprintf(outfile,"c%i ",lcolor);
	  colold=lcolor;
	}
	continue;
      }
      pnts[++pcnt].x=opts[0]/RSCALE;
      pnts[pcnt].y=opts[1]/RSCALE;
      boundbox(&pnts[pcnt]);
      if (pcnt%(pnmax/4)==(pnmax/4)-1){
        if (!filflg) {   /* flush pnts to keep vector counts small */
	  drawline(pnts,&pcnt,lcolor,&colold,&lastfill,-1);
          pnts[0].x=opts[0]/RSCALE;
	  pnts[0].y=opts[1]/RSCALE;
        } else if(pcnt==pnmax-1) { /* increase pnmax to hold more points */
	  pnmax =PNMAX* ++npnmax;
	  pnts2=(struct point *) malloc(pnmax*sizeof(struct point)); /* new memory array */
	  memcpy(pnts2,pnts,pnmax*sizeof(struct point));    /* copy saved pnts to new array */
	  free(pnts);
	  pnts=pnts2;
	  if(o.verbose)
	    printf("     Memory allocation for polygons increased to %i points.\n",pnmax);
        }
      }
    }
    else if (cmd==-10){				/* Move to */
      /*      fread ( opts, sizeof(short), 2, infile); */
      RDSTREAM(opts,2)
      if (pcnt) {
	if( (pnts[pcnt].x==opts[0]/RSCALE) && (pnts[pcnt].y==opts[1]/RSCALE) ) {
	  continue;
	}
	drawline(pnts,&pcnt,lcolor,&colold,&lastfill,0);
      }
      pnts[pcnt].x=opts[0]/RSCALE;
      pnts[pcnt].y=opts[1]/RSCALE;
      boundbox(&pnts[pcnt]);
      if(nodraw) continue;
    }
    else if (cmd==-4) {				/* Set line width */
      if (pcnt) {
	pcntSave=pcnt;
	drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);
	pnts[pcnt].x=pnts[pcntSave].x;
	pnts[pcnt].y=pnts[pcntSave].y;
      } else
	drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);
      /*       fread (opts, sizeof(short), 2, infile); */
      RDSTREAM(opts,2)
      i = opts[0];
      if (i>12) i=12;
      else if (i<1) i=1;
      if(--i!=wold){
	fprintf(outfile,"w%i ",i);
	wold=i;
      }
    }
    else if (cmd==-3) {				/* Set color */
      if (pcnt) {
	pcntSave=pcnt;
	drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);
	pnts[pcnt].x=pnts[pcntSave].x;
	pnts[pcnt].y=pnts[pcntSave].y;
      }
      /*      fread (opts, sizeof(short), 1, infile); */
      RDSTREAM(opts,1)
      lcolor = opts[0];
      /********      printf("New lcol#=%i\n",lcolor); */
      if (lcolor<0) lcolor=0;
      if (lcolor>99) lcolor=99;
      if (!(coldef[lcolor])) lcolor=15;
      /*      if (nodraw) fprintf(outfile,"c%i ",lcolor); */
      fprintf(outfile,"c%i ",lcolor);
    }
    else if (cmd==-7){				/* Start fill */
      /*       fread (opts, sizeof(short), 1, infile); */
      RDSTREAM(opts,1)
      filflg = 1;
      /* if(o.verbose) fprintf(outfile, "%%Fflag\n"); */
    }
    else if (cmd==-8){				/* End fill */
      if (pcnt) { 
	  if (lcolor != colold || lastfill==0)  {
	  fprintf(outfile,"g%i ",lcolor);
	  colold=lcolor;
	  }
        fillline(pnts,pcnt); pcnt=0;
        filflg = 0;
	lastfill=1;
      }
    }
    else if (cmd==-6){				/* Rectangle fill */
      if (pcnt) {drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);}
      /*       fread (opts, sizeof(short), 4, infile); */
      RDSTREAM(opts,4)
      if (lcolor != colold || lastfill==0)  {
	fprintf(outfile,"g%i ",lcolor);
	colold=lcolor;
      }
      for(i=0;i<4;i++) opts[i]/=RSCALE;
      pnts[0].x=opts[0];
      pnts[0].y=opts[2];
      pnts[1].x=opts[1];
      pnts[1].y=opts[3];
      boundbox(&pnts[0]);
      boundbox(&pnts[1]);
      fprintf(outfile,"%i %i %i %i B\n",
	      opts[0],opts[2],opts[1]-opts[0],opts[3]-opts[2]);
      lastfill=1;
    }
    else if (cmd==-9) {				/* End of plotting */
      /*  fsetpos(outfile,&pagepos); */
	fprintf (outfile,"%%%%Trailer\n%%%%EOF\n");
	/*  if(!o.reverse) printbbox(o.pscale,landscape); */
	/*  if(o.verbose) printf("BBox without offset: %i %i %i %i \n",
                           bblow.x, bblow.y, bbhigh.x, bbhigh.y); */
      if (o.ctl_d) fprintf (outfile,"%c%c",(char)4,'\n');
      if(o.verbose) printf ("     Number of pages = %i\n",fcnt);
#ifdef GXHEPS
      fsetpos(outfile,&pagespos);
      printf("WARNING: command -9 in plot buffer.\n");
#endif
      fprintf(outfile,"%i",fcnt);
      fclose (outfile);
      return(0);
    }
    else if (cmd==-1) {				/* Start of plotting */
      /*       fread (opts, sizeof(short), 2, infile); */
      RDSTREAM(opts,2);
      landscape= (opts[1]<opts[0]);  /* ysize < xsize : landscape */
      if(o.rotate && landscape) o.rotate=1; /* no -R option and */
      else o.rotate=0; 
      
      initepsf(landscape,argc,argv,fout,&o);
      colold=-1;
      init_newpage(landscape,fout,&o);
      pcnt = 0;
    }
    else if (cmd==-2) {				/* New Page */
      if (pcnt) drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);
      fprintf(outfile,"C0\n");
#ifdef DEBUG_EPS
     fprintf(outfile,
     " %i %i moveto %i %i lineto %i %i lineto %i %i lineto closepath stroke\n ",
     bblow.x,bblow.y,bblow.x, bbhigh.y,bbhigh.x,bbhigh.y,bbhigh.x,bblow.y);
#endif
      fprintf (outfile,"showpage\n");
      printbbox(o.pscale,landscape,&o);
#ifndef GXHEPS
      fgetpos(infile,&infile_pos);
      /*       fread (&cmd , sizeof(short), 1, infile);  */
      RDSTREAM(&cmd,1)
      fsetpos(infile,&infile_pos);
#endif
      if (cmd != -9) {  /* if next command is not end_of_plotting */
      ++fcnt;
      fprintf (outfile,"\n%%%%Page: %i %i\n",fcnt,fcnt); /* print %%Page: for the last page */
#ifndef GXHEPS
         fprintf(outfile,"%%%%BoundingBox: ");
         fgetpos(outfile, &bboxpos);
         fprintf(outfile,"                  \n");
#else
         fprintf(outfile,"%%%%BoundingBox: (atend)\n");
#endif
         bblow.x=1200;bblow.y=1200;
         bbhigh.x=0; bbhigh.y=0;
	 colold=-1;
         init_newpage(landscape,fout,&o);
      }
    }
    else if (cmd==-5){				/* Define new color */
      /*       fread (opts, sizeof(short), 4, infile); */
      RDSTREAM(opts,4);
      i = opts[0];
      if (i>15 && i<100) {
          green = ((float)(opts[2]))/255.0;
          if (green<0.0) green=0.0;
          if (green>1.0) green=1.0;
          red = ((float)opts[1])/255.0;
          if (red<0.0) red=0.0;
          if (red>1.0) red=1.0;
          blue = ((float)opts[3])/255.0;
          if (blue<0.0) blue=0.0;
          if (blue>1.0) blue=1.0;
          fprintf(outfile,"blackwhite \n");
	  /* (b/w: mapped into grey scale via green intensity) */
	  if(o.graylines)
	    fprintf(outfile,"  {/g%i {%.5g g} bdef /c%i {%.5g g} bdef}\n",i,1.0-green,i,1.0-green);
	  else
	    fprintf(outfile,"  {/g%i {%.5g g} bdef /c%i {g1} bdef}\n",i,1.0-green,i);
	  fprintf(outfile,"  {/c%i {%.5g %.5g %.5g c} bdef\n",i,red,green,blue);
          fprintf(outfile,"/g%i {c%i} def}\n ifelse\n",i,i);
	  coldef[i]=1;
      }
    }
      /*  -13 draw a polygon.  It has 2*siz+1 args. */
      else if (cmd==-13) {
	/* 	fread (opts, sizeof(short), 1, infile); */
	RDSTREAM(opts,1)
	i=opts[0]/2; /* i: # points in polygon */
	/* 	fprintf(outfile,"%% polygon of %d points, opts[0]=%d\n",i,opts[0]); */
	pnts2=(struct point *) malloc(i*sizeof(struct point));
#ifndef GXHEPS
	fread (pnts2, sizeof(struct point), i, infile);
#else
        pnts2=(struct point *) poi; poi+=2*i;
#endif
	jj=0;
	for(ii=i-1;ii>0; ii--){
	  jj+=fprintf(outfile,"%d %d ",(pnts2[ii].x-pnts2[ii-1].x)/RSCALE,
		      (pnts2[ii].y-pnts2[ii-1].y)/RSCALE);
	  if(jj>70) {
	    jj=0;
	    fprintf(outfile,"\n");
	  }
	}
	fprintf(outfile,"%d %d%s%d S\n",pnts2->x/RSCALE,pnts2->y/RSCALE,(jj>70) ? "\n" : " ", --i);
	/*	if(jj>70) fprintf(outfile,"\n");
		fprintf(outfile,"%d S\n",--i); */
      }
  

      /* -14 is draw a line  It has four float args. */ 
 
    else if (cmd==-14) {
      /*      fread (opts, sizeof(short), 4, infile); */
      RDSTREAM(opts,4)
      if(pcnt) { /* Check if we can add this secment to the stored polygon..*/
	if( (pnts[pcnt].x==opts[0]/RSCALE) && (pnts[pcnt].y==opts[1]/RSCALE) ){
	  pnts[++pcnt].x=opts[2]/RSCALE;
	  pnts[pcnt].y=opts[3]/RSCALE;
	  boundbox(&pnts[pcnt]);
	  if (pcnt%(pnmax/4)==(pnmax/4)-1){
	    drawline(pnts,&pcnt,lcolor,&colold,&lastfill,-1);
	    pnts[0].x=opts[2]/RSCALE;
	    pnts[0].y=opts[3]/RSCALE;
	  }
	} else 	if( (pnts[pcnt].x==opts[2]/RSCALE) && (pnts[pcnt].y==opts[3]/RSCALE) ){
	  pnts[++pcnt].x=opts[0]/RSCALE;
	  pnts[pcnt].y=opts[1]/RSCALE;
	  boundbox(&pnts[pcnt]);
	  if (pcnt%(pnmax/4)==(pnmax/4)-1){
	    drawline(pnts,&pcnt,lcolor,&colold,&lastfill,-1);
	    pnts[0].x=opts[0]/RSCALE;
	    pnts[0].y=opts[1]/RSCALE;
    }
	} else {
	  drawline(pnts,&pcnt,lcolor,&colold,&lastfill,0);
	}
      } else {
	pnts[pcnt].x=opts[0]/RSCALE;
	pnts[pcnt].y=opts[1]/RSCALE;
	boundbox(&pnts[pcnt]);
	pnts[++pcnt].x=opts[2]/RSCALE;
	pnts[pcnt].y=opts[3]/RSCALE;
	boundbox(&pnts[pcnt]);
/*       fprintf(outfile," %d %d %d %d L\n", */
/* 	      (opts[2]-opts[0])/RSCALE,(opts[3]-opts[1])/RSCALE,opts[0]/RSCALE,opts[1]/RSCALE); */
/*       boundbox((struct point *)&opts[0]); */
/*       boundbox((struct point *)&opts[2]); */
      }
    }
    else if (cmd==-15) {		/* Begin  Char. String drawing */
      /*  if (pcnt) drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1); */
      /*       fread (opts, sizeof(short), 8, infile); */
      RDSTREAM(opts,8)
                  /* opts[0]=len,   opts[1]=x, opts[2]=y, opts[3]=height,
                   * opts[4]=width, opts[5]=angle, opts[6]=font number
                   * opts[7]=justification
                   */
      if(opts[0]>=STRSIZ) {
	printf("String too long. Recompile with bigger STRSIZ.\n");
	return(1);
      }
#ifndef GXHEPS
      fread(string,sizeof(short),(opts[0])/sizeof(short)+2,infile);
#else
      string=(char *) poi; 
      poi+= (int) (opts[0])/sizeof(short)+2;
#endif
      i=j=0;
      do { /* quote parentheses */
	if(string[i]=='(' || string[i]==')' ) {
	  str[j++]='\\';
	  str[j++]=string[i++];
	} else
	  str[j++]=string[i++];
      }  while(string[i]!='\0');
      str[j]='\0';
      
      if((opts[6] != 3) && o.font) nodraw=1;
      if(!nodraw) fprintf(outfile,"%% String: ");
      setjust(opts,&justx,&justy);
      if(opts[7] == 1) {
      fprintf(outfile,"(%s) %d %d %d %d %d %d LS\n",
                  str,opts[6],opts[4]*85/50/RSCALE,opts[3]*75/50/RSCALE,
	         (int)((float)opts[5]/64.),opts[1]/RSCALE,opts[2]/RSCALE);
	
      } else {

		fprintf(outfile,"(%s) %d %d %d %d %d %d %d %d JS\n",
			str,opts[6],opts[4]*85/50/RSCALE,opts[3]*75/50/RSCALE,
			(int)((float)opts[5]/64.),opts[1]/RSCALE,opts[2]/RSCALE,justx,justy);
	      
      }
      if(!nodraw) fprintf(outfile,"%% String begin{\n");
    }
    else if (cmd==-16) {		/* End string  drawing */
      if (pcnt) drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);
      if(nodraw) {
      nodraw=0;
      } else {
	fprintf(outfile,"\n%%} String end\n");
      }
    }

      /* -17 indicates linestyle.  One arg; style number.  */
  
      else if (cmd==-17) {
	/* 	fread (opts, sizeof(short), 1, infile); */
	RDSTREAM(opts,1)
	if(oldstyle != opts[0]) {
	  if (pcnt) {drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);} 
	  if(opts[0]>0 && opts[0]<9)
	    fprintf(outfile,"s%d ",opts[0]);
	  else
	    fprintf(outfile,"s1 %%s%d\n",opts[0]);
	  oldstyle=opts[0];
	}
    }

      /* -18 Change clipping region: has 5 args  */
  
      else if (cmd==-18) {
	/* 	fread (opts, sizeof(short), 5, infile); */
	RDSTREAM(opts,5)

	if(opts[0]==0) {
	  fprintf(outfile,"C0\n");
	  cliplow.x=0;cliplow.y=0;cliphigh.x=11000;cliphigh.y=11000;
	}
	else if(opts[0]==1) { /* clip exterior of rectangle */
	  fprintf(outfile,"%d %d %d %d C1\n",opts[1]/RSCALE,opts[3]/RSCALE,
		  (opts[2]-opts[1])/RSCALE,(opts[4]-opts[3])/RSCALE);
	  cliplow.x=opts[1]/RSCALE;
	  cliplow.y=opts[3]/RSCALE;
	  cliphigh.x=opts[2]/RSCALE;
	  cliphigh.y=opts[4]/RSCALE;
 	}
	else if(opts[0]==2) /* add interior of rectangle to clipping*/
	  fprintf(outfile,"%d %d %d %d C2\n",opts[1]/RSCALE,opts[4]/RSCALE,
		  (opts[2]-opts[1])/RSCALE,(opts[3]-opts[4])/RSCALE);
      }

    else if (cmd==-20) {		/* Draw button -- ignore */
      /*       fread (opts, sizeof(short), 1, infile); */
      RDSTREAM(opts,1)
    }
    else {
       printf ("   Fatal error: Invalid command \"%i\" found in metafile\"%s\".\n",cmd,fin);
       printf ("   Is \"%s\" really a GrADS (v1.5 or higher) metafile?\n",fin);
      return(1);
    }
  }
#ifdef GXHEPS
    }
    /* Commands -2 (new page) and -9 (end plot) not in buffer *
     *  We need to do it now */
    
    /* cmd == -2  New page  + -9 end of plotting*/
    if (pcnt) drawline(pnts,&pcnt,lcolor,&colold,&lastfill,1);
    fprintf (outfile,"showpage\n");
    fprintf (outfile,"%%%%Trailer\n");
    fprintf(outfile,"%%%%BoundingBox: ");
    printbbox(o.pscale,landscape,&o);
    fprintf (outfile,"\n%%%%EOF\n");
    fprintf(outfile,"\n");
    if (o.ctl_d) fprintf (outfile,"%c%c",(char)4,'\n');
    fclose (outfile);
    if(fout != NULL) printf("  EPS file written to %s\n",fout);
    return(0);
#endif
}
/* --------------------------------------------------------------------------------- */

void initepsf(short landscape,int argc,char *argv[],char *fout,struct options *o){
       char *gr[16] =  {                      /* default greys */
                  "1", "0", "0.16", "0.46", "0.7", "0.58", "0.1", "0.34",
                  "0.22", "0.82", "0.4", "0.64", "0.28", "0.52", "0.76", "0.5"
             };
       char * col[16] = {          /* colors */
		"1.00 1.00 1.00",  /* white */
		"0.00 0.00 0.00",  /* black */
		"0.98 0.24 0.24",  /* red */
		"0.00 0.86 0.00",  /* green */
		"0.12 0.24 1.00",  /* blue */
		"0.00 0.78 0.78",  /* cyan */
		"0.94 0.00 0.51",  /* magenta */
		"0.90 0.86 0.19",  /* yellow */
		"0.94 0.51 0.16",  /* orange */
		"0.63 0.00 0.78",  /* purple */
		"0.63 0.90 0.19",  /* yellow/green */
		"0.00 0.63 1.00",  /* med. blue */
		"0.90 0.69 0.18",  /* dark yellow */
		"0.00 0.82 0.55",  /* aqua */
		"0.51 0.00 0.86",  /* dark purple */
		"0.67 0.67 0.67"   /* grey */
};
      char *widths[12] = {                      /* widths */
	 "4 w","7 w","10 w","14 w","17 w","20 w",
                "24 w","27 w","31 w","34 w","38 w","41 w"
	/*   "1 w","3 w","5 w","7 w","9 w","10 w",
	     "12 w","14 w","15 w","17 w","19 w","21 w"  RSCALE=2 widths */
             };
      int i;
      char buf[256];
      time_t thetime=time(NULL);
      if(o->pslevel>1) fprintf(outfile, "%%!PS-Adobe-2.0 EPSF-1.2\n");
      else             fprintf(outfile, "%%!PS-Adobe-1.0 EPSF-1.2\n");
#ifndef GXHEPS
      fprintf(outfile,"%%%%BoundingBox: ");
      fgetpos(outfile, &bboxpos);
      fprintf(outfile,"                    \n");
#else
         fprintf(outfile,"%%%%BoundingBox: (atend)\n");
#endif
      if(getenv("USER"))
	fprintf(outfile,"%%%%For: %s\n",getenv("USER"));
      fprintf(outfile,
              "%%%%Creator: gxeps $Revision: 1.11 $\n"
              "%%%%Title: %s\n"
              "%%%%CreationDate: %s"
              "%%%%Pages: ",fout,ctime(&thetime));
#ifndef GXHEPS
      fgetpos(outfile, &pagespos);
#endif
      if(o->pscale !=2) {
	fprintf(outfile,"    \n%%%%PageOrder: Ascend\n%%%%PaperSize: ");
	if(o->pscale==1) fprintf(outfile, "A4\n");
	else      fprintf(outfile, "Letter\n");
      }
      if(landscape) fprintf(outfile, "%%%%PageOrientation: Landscape\n");
      else      fprintf(outfile, "%%%%PageOrientation: Portrait\n");
      /*       fprintf(outfile,"%%%%DocumentFonts: "); */
      /*      if(o->tstamp||o->label) fprintf(outfile,"Helvetica"); */
      fprintf(outfile,"\n%%GxepsCommandLine: ");
      for(i=0;i<argc;i++) fprintf(outfile," %s",argv[i]);
      fprintf(outfile,"\n");
      if(getenv("HOST"))
	fprintf(outfile,"%%Host: %s\n",getenv("HOST"));
      if(getenv("PWD"))
	fprintf(outfile,"%%PWD: %s\n",getenv("PWD"));
      if(o->comment) {			/* get user comments */
         fprintf(outfile,"\n%%BeginUserComments\n");
         printf("%s%s","Comments to plot=?",
               " (Period (.) on a single line to finish)\n");
        while(1) {
          fgets(buf,130,stdin);
          if(buf[0]=='.' && buf[1]=='\n') break;
          fprintf(outfile,"%% %s",buf);
        }
         fprintf(outfile,"%%EndUserComments\n");
      }
#ifdef GXHEPS
#if READLINE == 1
     if(o->history) {
       fprintf(outfile,"%%BeginGradsCommandHistory\n");
       fclose(outfile);
       strcpy(buf,"his -q ");
       gahistory("his",strncat(buf,fout,255), NULL);
       outfile=fopen(fout,"a");
       fprintf(outfile,"%%EndGradsCommandHistory\n");
      }
#endif
#endif
      fprintf(outfile,"%%%%EndComments\n");
      fprintf(outfile,"%s",
                      "\n%%BeginProlog\n");
      fprintf(outfile,"%%Change the following (values: true, false) to get color or b/w output\n");
      fprintf(outfile,"/blackwhite {%s} def\n",
	      (o->color) ? "false" : "true");
      fprintf(outfile,"%s",
	              "/clipOn {false} def\n"
	              "/bdef {bind def} bind def\n"
	              "/edef {exch def} bind def\n"
                      "/in {72 mul} bdef\n"
                      "/cm {in 2.54 div} bdef\n"
                      "/c {setrgbcolor} bdef\n"
                      "/g {setgray} bdef\n"
                      "/w {setlinewidth} bdef\n"
                      "/L {moveto rlineto stroke} bdef\n"
                      "/D {5 -1 roll 0 setdash L []0 setdash} bdef %% dashed line\n"
                      "/P {% Lay out a polygon\n"
                      "    % usage: x_n y_n x_n-1 y_n-1 ... x_0 y_0 n P\n"
                      "    3 1 roll moveto {rlineto} repeat} bdef\n"
                      "/F {P fill} bdef %Fill a polygon\n"
                      "/S {P stroke} bdef %Draw a polygon\n"
                      "/gxfonts {%Usage: <font#> gxfonts\n"
                      "    [/Helvetica /Times-Roman /Times-Italic /Symbol /Helvetica-Bold /Times-Bold] \n"
                      "     exch get findfont} bdef\n"
                      "/sfont {%Usage: <font#> <width> <height> sfont\n"
                      "     /szy edef /szx edef gxfonts [szx 0 0 szy 0 0 ] makefont setfont} def\n"
                      "/LS % Left alligned string\n" 
                      "{ % Usage: <string> <font#> <width> <height> <angle> <pos_x> <pos_y>\n"
                      "   gsave translate rotate sfont 0 0 moveto show grestore\n"
                      "} def\n"
                      "/RS % Right alligned string\n" 
                      "{ % Usage: <string> <font#> <width> <height> <angle> <pos_x> <pos_y>\n"
                      "   gsave translate rotate sfont 0 0 moveto dup stringwith pop \n"
                      "   0 sub rmoveto show grestore\n"
                      "} def\n"
		      "/JS % General justified string \n"
		      "{ %Usage: <string> <font#> <width> <height> "
		      "<angle> <pos_x> <pos_y> <justx> <justy> JS\n"
		      " % (Note: justx, justy in percent: e.g., justx=50 => x-centered)\n"
		      "   /jy edef /jx edef gsave translate rotate sfont 0 0 moveto dup stringwidth\n"
		      "   jy mul 0.01 neg mul exch jx mul 0.01 mul neg exch rmoveto show grestore\n} def\n"
		      );
if(o->pslevel>1){
         fprintf(outfile,"%s",
                      "/B {rectfill} bdef %Fill a box, usage: xlow ylow width height B\n");
         fprintf(outfile,"%s",
                      "/iso_reencode { % Usage: <Font> iso_reencode\n"
                      "  dup /isoname exch def findfont\n"
                      "  dup length dict begin\n"
                      "  { 1 index /FID ne\n"
                      "      {def}\n"
                      "      {pop pop}\n"
                      "      ifelse\n"
                      "  } forall\n"
                      "  /Encoding ISOLatin1Encoding def\n"
                      "  currentdict end\n"
                      "  isoname exch definefont pop\n"
		      "} def\n"
		      "[/Helvetica /Times-Roman /Times-Italic /Helvetica-Bold /Times-Bold]\n"
                      "   {iso_reencode} forall\n");
      }
      else { 
         fprintf(outfile,"%s",
                      "/B { %Fill a box, usage: xlow ylow width height B\n"
                      "     /height exch def /width exch def moveto width 0 rlineto\n"
                      "     0 height rlineto width neg 0 rlineto closepath fill} bdef \n");
      }
         fprintf(outfile,"%s",
                      "/BL { %layout a box, usage: xlow ylow width height BL\n"
                      "     /height exch def /width exch def moveto width 0 rlineto\n"
                      "     0 height rlineto width neg 0 rlineto closepath} bdef \n");
         fprintf(outfile,"%s","/G {gsave} bdef /R {grestore} bdef\n");
/*          fprintf(outfile,"%s", */
/*                       "/C { %Clip a box, usage: xlow ylow width height C\n" */
/*                       "  clippath BL  clip newpath} bdef \n"); */
         fprintf(outfile,"%s", 
                      "/C0 { %Reset clip path\n"  
                      "     clipOn {grestore /clipOn {false} def} if} def\n"
                      "/C1 { %Reset clipping to a box. Usage: xlow ylow width height C\n" 
                      "      clipOn {grestore} if gsave /clipOn {true} def\n"
		      "      BL eoclip newpath } bdef\n" 
                      "/C2 {%Add clip inside a box to current clipping.  Usage: xlow ylow width height C \n"
                      "      clipOn not {gsave /clipOn {true} def} if\n"
                      "     clippath BL eoclip newpath } bdef\n");


      fprintf(outfile,"\n%%Define line widths\n");
      for (i=0;i<12;i++) fprintf(outfile,"/w%i {%s} bdef\n",i,widths[i]);

      fprintf(outfile,"\n%%Define dashes\n");
      fprintf(outfile,"/s0 {[] 0 setdash} bdef\n");
      fprintf(outfile,"/s1 {[] 0 setdash} bdef\n");
      fprintf(outfile,"/s2 {[125 60] 0 setdash} bdef\n");
      fprintf(outfile,"/s3 {[60 60] 0 setdash} bdef\n");
      fprintf(outfile,"/s4 {[125 60 60 60] 0 setdash} bdef\n");
      fprintf(outfile,"/s5 {[10 40] 0 setdash} bdef\n");
      fprintf(outfile,"/s6 {[75 60] 0 setdash} bdef\n");
      fprintf(outfile,"/s7 {[125 40 15 15 15 40] 0 setdash} bdef\n");
      fprintf(outfile,"/s8 {[15 80] 0 setdash} bdef\n");
      

      fprintf(outfile,"\n%%Define colors:\n");
      if (o->reverse) {                       /* switch black and white colors */
          fprintf(outfile,"/c0 {%s g} bdef %%black and white reversed\n",gr[1]);
          fprintf(outfile,"/c1 {%s g} bdef\n",gr[0]);
      } else {
          for (i=0;i<2;i++) fprintf(outfile,"/c%i {%s g} bdef\n",i,gr[i]);
      }
      for (i=2;i<16;i++) fprintf(outfile,"/c%i {%s c} bdef\n",i,col[i]);

      fprintf(outfile,"\n%%Define grays:\n");
      if (o->reverse) {                       /* switch black and white */
          fprintf(outfile,"/g0 {%s g} bdef %%black and white reversed\n",gr[1]);
          fprintf(outfile,"/g1 {%s g} bdef\n",gr[0]);
      } else {
          for (i=0;i<2;i++) fprintf(outfile,"/g%i {%s g} bdef\n",i,gr[i]);
      }
      for (i=2;i<16;i++) fprintf(outfile,"/g%i {%s g} bdef\n",i,gr[i]);
      fprintf(outfile,"\nblackwhite\n");
      fprintf(outfile,"{ %%Black&white output: re-define line colors black, fill colors gray... \n  ");
      for (i=1;i<16;i++) fprintf(outfile,"/c%i {g1} def%s",i,(((i+1)%5)==0) ? "\n  " : " ");
      fprintf(outfile,"}\n{ %%Color output: undefine fill-colors... \n  ");
      for (i=0;i<16;i++) fprintf(outfile,"/g%i {c%i} def%s",i,i,(((i+1)%5)==0) ? "\n  " : " ");
      fprintf(outfile,"\n} ifelse\n");
      

      fprintf(outfile,"%%%%EndProlog\n\n%%%%BeginSetup\n");
      fprintf(outfile,"1 setlinecap 1 setlinejoin\n%%%%EndSetup\n");
#ifndef GXHEPS
      fgetpos(outfile,&pagepos);
#endif
      fprintf(outfile,"%%%%Page: 1 1\n");
      return;
}
/* --------------------------------------------------------------------------------- */

void init_newpage(short landscape,char *fout,struct options *o){
      int i;
      char buf[130];
      time_t thetime=time(NULL);
      struct tm *ltime=localtime(&thetime);
#ifndef GXHEPS
      if (o->reverse) {
        fgetpos(outfile, &blackpos);
        fprintf(outfile,"                              %% black background\n");
      }
#endif
      if(o->tstamp||o->label) fprintf(outfile,"gsave\n");
      if(o->pscale==1) { /* A4 paper */
        if (o->rotate) {
	  fprintf(outfile,"21 cm 0 translate 90 rotate\n"); /* landscape mode */
          fprintf(outfile,"%i %i translate %%move origin slightly in\n",A4YOFF, A4XOFF);
	} else {
          fprintf(outfile,"%i %i translate %%move origin slightly in\n",A4XOFF, A4YOFF);
	}
	fprintf(outfile,"%f %f scale  %% Units: 1000 = 2 cm\n",A4SCALE, A4SCALE);
      } else if (o->pscale==0){ /* Letter paper */
        if (o->rotate) {
	  fprintf(outfile,"8.5 in 0 translate 90 rotate %% landscape\n");
          fprintf(outfile,"%i %i translate %%move origin slightly in\n",LETYOFF,LETXOFF);
	} else {
          fprintf(outfile,"%i %i translate %%move origin slightly in\n",LETXOFF,LETYOFF);
	}
	fprintf(outfile,"%f %f scale  %%Units: 1000 = .9125 inch\n",LETSCALE, LETSCALE);
      } else if (o->pscale==2){ /* GrADS 8.5''x 11' scaling */
        if (o->rotate) 
	  fprintf(outfile,"8.5 in 0 translate 90 rotate %% landscape\n");
	fprintf(outfile,"%f %f scale  %%Units: 1000 = .9125 inch\n",GASCALE, GASCALE);
      }
      if(landscape)
	fprintf(outfile,"0 0 %d %d BL clip newpath %% Initial clipping.\n",
			    11000/RSCALE, 8500/RSCALE);
      else
	fprintf(outfile,"0 0 %d %d BL clip newpath %% Initial clipping.\n",
			    8500/RSCALE, 11000/RSCALE);
	
      fprintf(outfile,"c1\n");
      if(o->tstamp||o->label) {
	fprintf(outfile,"matrix currentmatrix %% store CTM\n");
        fprintf(outfile,"grestore\nc1 /Helvetica findfont 8 scalefont setfont\n");
	if(o->pscale) fprintf(outfile,"1 cm 28.5 cm translate 0 0 moveto\n");
	else     fprintf(outfile,"0.3 in 0.5 in translate 0 0 moveto\n");
	if(o->tstamp) {
          strftime(buf,130,"%c",ltime);
	  fprintf(outfile,"(%s,  %s) show %%file and time stamp\n", fout,buf);
	}
        if(o->label) {			/* get user label */
          fprintf(outfile,"\n\n%%BeginUserLabel\n");
          printf("   %s%s","Label to print on the plot=?"," (Period (.) on a single line to finish)\n");
          while(1) {
            fgets(buf,130,stdin);
            if(buf[0]=='.' && buf[1]=='\n') break;
            fprintf(outfile,"0 -7 translate 0 0 moveto (");
	    for (i=0;i<strlen(buf)-1;i++){
	      if(buf[i]=='(' || buf[i]==')'|| buf[i]=='\\') fprintf(outfile,"%c",'\\'); 
              fprintf(outfile,"%c",buf[i]);
            }
	    fprintf(outfile,") show\n");
          }
          fprintf(outfile,"%%EndUserLabel\n");
        }
        fprintf(outfile,"setmatrix %% restore CTM\n");
      }
      return;
}
/* --------------------------------------------------------------------------------- */

void drawline(struct point pnts[],short *pcnt,
	      short lcolor,short *colold,short *lastfill,short flush){
   register int i,im,j,colum;
   static short LastLine=0;
   static short firstcall=1;
   static struct point *dline;
   long dx,dy,dx1,dy1;

   /* flush=1; mm debugging */

   if(firstcall) {
      dline=(struct point *) malloc(4*sizeof(struct point)); 
      firstcall=0;
   }
   if(*pcnt==1) {
      if (LastLine==2){ /* there is a saved dashed line */
	     if ((LastLine=CheckExpandDashLine(pnts,dline))==2) {
	        dline[1].x=pnts[1].x; dline[1].y=pnts[1].y;
	     } else { /*  Dashed Line  not expandable => print dashed line */
	        PrintDashLine(dline);
		LastLine=SaveLine(pnts,dline);
	     }
      } else if (LastLine==1) { /* there is a saved non-dashed line */
            if((LastLine=CheckDashLine(pnts,dline))==2) {                 
	        SetDline(pnts,dline);
            } else { /* Not New LastLine */
	        PrintDashLine(dline);
		LastLine=SaveLine(pnts,dline);
	    }
      } else { /* no last line saved */
         if (lcolor != (*colold) || (*lastfill) ) {
             fprintf(outfile,"c%i ",lcolor);
             *lastfill=0;
             *colold=lcolor;
         }
	    LastLine=SaveLine(pnts,dline);
      }
      if(flush){ 
   if ( lcolor != (*colold) || (*lastfill) )  {
     fprintf(outfile,"c%i ",lcolor);
     *lastfill=0;
     *colold=lcolor;
   }
	 PrintDashLine(dline);
	 LastLine=0;
      }
   } else { 
        if(LastLine) PrintDashLine(dline);
	if(*pcnt>1){
           if ( lcolor != (*colold) || (*lastfill) ) {
                fprintf(outfile,"c%i ",lcolor);
                *lastfill=0;
                *colold=lcolor;
           }
	   dx=(long)(pnts[*pcnt].x- pnts[(*pcnt)-1].x);
	   dy=(long)(pnts[*pcnt].y- pnts[(*pcnt)-1].y);
	   j=0;
	   colum=1;
           for(i=(*pcnt)-1;i>0;i--) {
              im=i-1;
	      dx1=(long)(pnts[i].x-pnts[im].x);
	      dy1=(long)(pnts[i].y-pnts[im].y);
	      if (((dx==0&&dy==0) || (dx*dy1==dx1*dy)) && !((dx==0&&dx1!=0) || (dy==0&&dy1!=0)) ) {
		 dx+=dx1;
		 dy+=dy1;
		 (*pcnt)--;
	      } else {
		             /* if old and new line directions disagree */
		 colum+=fprintf(outfile,"%ld %ld",dx,dy);
                 if (colum>=70) {
		     fprintf(outfile,"%c",'\n');
		     colum=1;
		 } else 
		     colum+=fprintf(outfile,"%c",' ');
		 dx=dx1;
		 dy=dy1;
	      }
           }
	   fprintf(outfile,"%ld %ld",dx,dy);
                 if (colum>=60) {
		     fprintf(outfile,"%c",'\n');
		     colum=1;
		 } else 
		     colum+=fprintf(outfile,"%c",' ');
           colum+=fprintf(outfile,"%i %i ",pnts[0].x,pnts[0].y);
	   if(flush==-1) fprintf(outfile,"%i P\n",*pcnt);
	   else
	     if(*pcnt==1) fprintf(outfile,"L\n");
	     else fprintf(outfile,"%i S\n",*pcnt);
	}
	LastLine=0;
   } 
   *pcnt=0;
}
/* --------------------------------------------------------------------------------- */
int SaveLine(struct point pnts[],struct point dline[]){
  dline[0].x=pnts[0].x;
  dline[0].y=pnts[0].y;
  dline[1].x=pnts[1].x;
  dline[1].y=pnts[1].y;
  dline[2].x=pnts[0].x;
  dline[2].y=pnts[0].y;
  dline[3].x=pnts[0].x;
  dline[3].y=pnts[0].y;
  return 1;
}
/* --------------------------------------------------------------------------------- */
void PrintDashLine(struct point dline[]){
  short SolidLine,dxsolid,dysolid,dxskip,dyskip;
  dxsolid=dline[2].x-dline[0].x;
  dysolid=dline[2].y-dline[0].y;
  dxskip=dline[3].x-dline[2].x;
  dyskip=dline[3].y-dline[2].y;
  SolidLine= ((dxsolid==0)&&(dysolid==0)&&(dxskip==0)&&(dyskip==0));
  if(!SolidLine) fprintf(outfile,"[%i %i] ",
	      (short) (sqrt((float) dxsolid*dxsolid+(float) dysolid*dysolid)+0.5),
	      (short) (sqrt((float) dxskip*dxskip+(float) dyskip*dyskip)+0.5));
  fprintf(outfile,"%i %i %i %i %c\n",dline[1].x-dline[0].x,dline[1].y-dline[0].y,
	  dline[0].x,dline[0].y,SolidLine ? 'L' : 'D');
  return;
}
/* --------------------------------------------------------------------------------- */
short CheckDashLine(struct point pnts[],struct point dline[]){
  short dx,dy;
  dx=pnts[1].x-pnts[0].x;
  if (dx!=dline[1].x-dline[0].x) return 1; /* delta x disagree */
  dy=pnts[1].y-pnts[0].y;
  if (dy!=dline[1].y-dline[0].y) return 1; /* delta y disagree */
  if((int) dx *(pnts[0].y-dline[1].y) !=(int) (pnts[0].x-dline[1].x)*dy)
      return 1;  /* line directions disagree */
  else return 2;
}
/* --------------------------------------------------------------------------------- */
short CheckExpandDashLine(struct point pnts[],struct point dline[]){
  if( (dline[3].x-dline[2].x!=pnts[0].x-dline[1].x) ||
      (dline[3].y-dline[2].y!=pnts[0].y-dline[1].y) ||
      (dline[2].x-dline[0].x!=pnts[1].x-pnts[0].x)  ||
      (dline[2].y-dline[0].y!=pnts[1].y-pnts[0].y)    )return 1;
  else return 2;
}
/* --------------------------------------------------------------------------------- */
void SetDline(struct point pnts[],struct point dline[]){
  dline[2].x=dline[1].x;
  dline[2].y=dline[1].y;
  dline[3].x=pnts[0].x;
  dline[3].y=pnts[0].y;
  dline[1].x=pnts[1].x;
  dline[1].y=pnts[1].y;
  return;
  }
/* --------------------------------------------------------------------------------- */
void fillline(struct point pnts[],short pcnt){
   register int i,im;
   /* check if the polygon is a rectangel: */
   if(pcnt==4 &&  pnts[0].x==pnts[1].x && pnts[2].x==pnts[3].x &&
	     pnts[1].y==pnts[2].y && pnts[0].y==pnts[3].y) {
      fprintf(outfile,"%i %i %i %i B\n",
	      pnts[0].x,pnts[0].y,pnts[2].x-pnts[0].x,pnts[2].y-pnts[0].y);
   } else if(pcnt==4 && pnts[0].y==pnts[1].y && pnts[2].y==pnts[3].y &&
	     pnts[1].x==pnts[2].x && pnts[0].x==pnts[3].x) {
      fprintf(outfile,"%i %i %i %i B\n",
	      pnts[0].x,pnts[0].y,pnts[2].x-pnts[0].x,pnts[2].y-pnts[0].y);
   } else {
     long dx,dy,dx1,dy1;
     int j,colum;
	   dx=(long)(pnts[pcnt].x- pnts[(pcnt)-1].x);
	   dy=(long)(pnts[pcnt].y- pnts[(pcnt)-1].y);
	   j=0;
	   colum=1;
           for(i=(pcnt)-1;i>0;i--){
              im=i-1;
	      dx1=(long)(pnts[i].x-pnts[im].x);
	      dy1=(long)(pnts[i].y-pnts[im].y);
	      if (((dx1==0&&dy1==0) ||
		   (dx*dy1==dx1*dy)) && !((dx==0&&dx1!=0)||(dy==0&&dy1!=0)) ) {
		 dx+=dx1;
		 dy+=dy1;
		 (pcnt)--;
	      } else {      /* if old and new line directions disagree */
		 colum+=fprintf(outfile,"%ld %ld",dx,dy);
                 /* (j++%10==9) ? fprintf(outfile,"%c",'\n') : fprintf(outfile,"%c",' '); */
                 if (colum>=70) {
		     fprintf(outfile,"%c",'\n');
		     colum=1;
		 } else 
		     colum+=fprintf(outfile,"%c",' ');
		 dx=dx1;
		 dy=dy1;
	      }
           }
	   if(colum>=60)
	       fprintf(outfile,"%ld %ld\n%i %i %i F\n",dx,dy,pnts[0].x,pnts[0].y,pcnt);
	   else
               fprintf(outfile,"%ld %ld %i %i %i F\n",dx,dy,pnts[0].x,pnts[0].y,pcnt);
   }
}
/* --------------------------------------------------------------------------------- */

void boundbox(struct point* pnts){  /* compute bounding box */
  /*  Does not work because of clipping to the outside 
   *  For now, let's not take clipping into account
   *  *** TO BE FIXED ***
  if((cliplow.x<bblow.x) && (pnts->x<bblow.x)) bblow.x=max(pnts->x,cliplow.x);
  if((cliplow.y<bblow.y) && (pnts->y<bblow.y)) bblow.y=max(pnts->y,cliplow.y);
  if((cliphigh.x>bbhigh.x) && (pnts->x>bbhigh.x)) bbhigh.x=min(pnts->x,cliphigh.x);
  if((cliphigh.y>bbhigh.y) && (pnts->y>bbhigh.y)) bbhigh.y=min(pnts->y,cliphigh.y);
  */
     if(pnts->x<bblow.x) bblow.x=pnts->x;
     if(pnts->y<bblow.y) bblow.y=pnts->y;
    if(pnts->x>bbhigh.x) bbhigh.x=pnts->x;
     if(pnts->y>bbhigh.y) bbhigh.y=pnts->y;
     return;
}
/* --------------------------------------------------------------------------------- */
void printbbox(short pscale,short landscape,struct options *o){
#ifndef GXHEPS
     fsetpos(outfile, &bboxpos);
#endif
     if(pscale) {
        bblow.x= ( (float) bblow.x) *A4SCALE;
        bbhigh.x= ((float) bbhigh.x)*A4SCALE;
        bblow.y= ((float) bblow.y)*A4SCALE;
        bbhigh.y= ((float) bbhigh.y)*A4SCALE;
        if(o->rotate==0) {
          fprintf(outfile,"%i %i %i %i",
               bblow.x+A4XOFF-BORDER,bblow.y+A4YOFF-BORDER,bbhigh.x+A4XOFF+BORDER,
		  bbhigh.y+A4YOFF+BORDER);
#ifndef GXHEPS
	  if(o->reverse) {
            fsetpos(outfile, &blackpos);
            fprintf(outfile,"0 g %i %i %i %i B 1 g ",
               bblow.x+A4XOFF-BORDER,bblow.y+A4YOFF-BORDER,bbhigh.x-bblow.x+2*BORDER,
		    bbhigh.y-bblow.y+2*BORDER);
	  }
#endif
        } else {
           fprintf(outfile,"%i %i %i %i",
              595-bbhigh.y-A4XOFF-BORDER,bblow.x+A4YOFF-BORDER,612-bblow.y-A4XOFF+BORDER,
		   bbhigh.x+A4YOFF+BORDER);
#ifndef GXHEPS
	  if(o->reverse) {
            fsetpos(outfile, &blackpos);
            fprintf(outfile,"0 g %i %i %i %i B 1 g ",
		  595-bbhigh.y-A4XOFF-BORDER,bblow.x+A4YOFF-BORDER,bbhigh.y-bblow.y+2*BORDER,
		    bbhigh.x-bblow.x+2*BORDER); 
	  }
#endif
	}
     } else {
        bblow.x= ((float) bblow.x)*LETSCALE;
        bbhigh.x= ((float) bbhigh.x)*LETSCALE;
        bblow.y= ((float) bblow.y)*LETSCALE;
        bbhigh.y= ((float) bbhigh.y)*LETSCALE;
        if(o->rotate==0){
           fprintf(outfile,"%i %i %i %i",
               bblow.x+LETXOFF-BORDER,bblow.y+LETYOFF-BORDER,bbhigh.x+LETXOFF+BORDER,
		   bbhigh.y+LETYOFF+BORDER);
#ifndef GXHEPS
	  if(o->reverse) {
            fsetpos(outfile, &blackpos);
            fprintf(outfile,"0 g %i %i %i %i B 1 g ",
               bblow.x+LETXOFF-BORDER,bblow.y+LETYOFF-BORDER,bbhigh.x-bblow.x+2*BORDER,
		    bbhigh.y-bblow.y+2*BORDER);
	  }
#endif
	} else {
           fprintf(outfile,"%i %i %i %i",
              612-bbhigh.y-LETXOFF-BORDER,bblow.x+LETYOFF-BORDER,612-bblow.y-LETXOFF+BORDER,
		   bbhigh.x+LETYOFF+BORDER);
#ifndef GXHEPS
	  if(o->reverse) {
            fsetpos(outfile, &blackpos);
            fprintf(outfile,"0 g %i %i %i %i B 1 g ",
		      612-bbhigh.y-LETXOFF-BORDER,bblow.x+LETYOFF-BORDER,
		    bbhigh.y-bblow.y+2*BORDER,bbhigh.x-bblow.x+2*BORDER); 
	  }
#endif
	}
     }
#ifndef GXEPS       
     fseek(outfile,0L,2); 
#endif
}
/* --------------------------------------------------------------------------------- */


int parse_arg(int argc,char *argv[],struct options *o,char **fin, char **fout){
     register int i,j;
     i = 1;
#ifndef GXHEPS
     if(argc==1) return(print_options(argv));
#endif

     for (i=1;i<argc;i++) {

     if (*(argv[i])=='-') {  /* parse options */
       j = 0;
       while (*(argv[i]+(++j))) {
         if      (*(argv[i]+j)=='a') o->pscale = 1;
         else if (*(argv[i]+j)=='1') o->pslevel = 1;
         else if (*(argv[i]+j)=='2') o->pslevel = 2;
	 else if (*(argv[i]+j)=='b') o->color = 0;
         else if (*(argv[i]+j)=='c') o->color = 1;
#ifdef NEVER_CTL_D
         else if (*(argv[i]+j)=='d') ;
#else
         else if (*(argv[i]+j)=='d') o->ctl_d = !o->ctl_d;
#endif
#ifdef PSFONTS
         else if (*(argv[i]+j)=='f') o->font =0;
#endif
         else if (*(argv[i]+j)=='g') o->pscale = 2;
	 else if (*(argv[i]+j)=='G') o->graylines = 1;
	 else if (*(argv[i]+j)=='h') return (print_options(argv));
	 else if (*(argv[i]+j)=='H') o->history = 0;
         else if (*(argv[i]+j)=='i') {*fin = argv[++i];break;}
         else if (*(argv[i]+j)=='l') o->pscale = 0;
         else if (*(argv[i]+j)=='L') o->label = 1;
         else if (*(argv[i]+j)=='n') o->comment = 1;
         else if (*(argv[i]+j)=='o') {*fout = argv[++i];break;}
         else if (*(argv[i]+j)=='r') o->reverse = 1;
         else if (*(argv[i]+j)=='R') o->rotate = 0;
         else if (*(argv[i]+j)=='s') o->tstamp = 1;
         else if (*(argv[i]+j)=='v') o->verbose = 1;
         else { 
	   fprintf(stderr,"Unknown option: %s\n\n",argv[i]);
	   print_options(argv);
	     return(1);
	 }
       }
     }
    else  /* No command line "-" */
#ifdef GXHEPS
	 *fout = argv[i];
#else
      *fin=argv[i];
#endif
  }
#ifdef GXHEPS
     if(*fout==NULL)
       *fout="grads.eps";
#endif     
     return(0);
}
/* --------------------------------------------------------------------------------- */

int print_options(char *argv[]){
#ifndef GXHEPS	       
	       /* printf("   This is gxeps $Revision: 1.11 $\n"); */
	       printf("   This is gxeps Version: GRADS_VERSION\n");
#endif
#ifdef NEVER_CTL_D
       fprintf(stderr,"   %s%s%s","Usage: ",argv[0],
#ifndef GXHEPS	       
	   " [-abcfghiLlnRrsv -i <in_file>[."  IN_EXT
#else
	   " [-abcfghiLlnRrsv"
#endif 
           "] -o <out_file>] [<in_file>[."IN_EXT"]].\n");
#else
       fprintf(stderr,"%s%s%s","Usage: ",argv[0],
	   " [-acdifhLlnRrsv -i <in_file>[."  IN_EXT
           "] -o <out_file>] [<in_file>[."IN_EXT"]].\n");
#endif
       fprintf(stderr,"Options:\n");
       fprintf(stderr,"     -1   PostScript Level 1 output.\n");
       fprintf(stderr,"     -2   PostScript Level 2 output  (default).\n");
       fprintf(stderr,"     -a   A4 paper.\n");
       fprintf(stderr,"     -b   Black & white output.\n");
       fprintf(stderr,"     -c   Color output (default).\n");
#ifndef NEVER_CTL_D
       if(DEFAULT_CTL_D)  fprintf(stderr,"     -d   Do not add Control-D at end.\n");
       else fprintf(stderr,"     -d   Add Control-D at end.\n");
#endif
#ifdef PSFONTS
       fprintf(stderr,"     -f   Do not use PostScript fonts.\n");
#endif
       fprintf(stderr,"     -g   True 8.5'' x 11'' output (GrADS scaling).\n");
       fprintf(stderr,"     -G   Use gray lines in black & white mode (default: all black).\n");
#ifdef GXHEPS
       fprintf(stderr,"     -H   Do not include GrADS command history in output.\n");
#endif
       fprintf(stderr,"     -h   Help.\n");
#ifndef GXHEPS	       
       fprintf(stderr,"     -i   <in_file>[."IN_EXT"], '-' = stdin.\n");
#endif
       fprintf(stderr,"     -l   letter paper\n");
       fprintf(stderr,"     -L   Ask for a label to be printed on the plot.\n");
       fprintf(stderr,"     -n   Ask for a note to include in postscript file header.\n");
       fprintf(stderr,"     -o   <out_file> (default: basename(in_file)."OUT_EXT", '-' = stdout).\n");
#ifndef GXHEPS	       
       fprintf(stderr,"     -R   Do not rotate landscape metafiles.\n");
#endif
       fprintf(stderr,"     -r   Black background.\n");
       fprintf(stderr,"     -s   Add a file & time stamp.\n");
       fprintf(stderr,"     -v   Verbose.\n\n");
       fprintf(stderr,"For more information visit http://www.bol.ucla.edu/~munnich/grads/gxeps.html.\n");
     return(8);
     }
/* --------------------------------------------------------------------------------- */

int openfiles(char **fin, char **fout,short verbose){ /* Open files */

#ifndef GXHEPS
  int i;
  if (*fin==NULL) {
    *fin = (char *) malloc(sizeof(char)*150);
     fgets(*fin,150,stdin);
     printf("   Read infile = %s\n",*fin);
  }
  if(strcmp(*fin,"-")==0) infile=stdin;
  else infile = fopen(*fin ,"rb");
  if (infile == NULL) {
    *fin=strcat(*fin,"."IN_EXT);
    infile = fopen(*fin,"rb");
    if (infile == NULL) {
      (*fin)[strlen(*fin)-3]='\0';
      printf ("Input file %s[."IN_EXT"] not found.\n",*fin);
      return(1);
    }
  }
  /* setvbuf(infile,NULL,_IOFBF,(size_t) 524288L); */
  if (*fout==NULL) {
    *fout = (char *) malloc(sizeof(char)*150);
    strcpy(*fout,*fin);
    for (i=strlen(*fout)-1;i>=0;i--) {
      if((*fout)[i]=='.') {strcpy((*fout)+i+1,OUT_EXT);
	break;
      }
      if(i==0){strcpy((*fout)+strlen(*fout),"."OUT_EXT);}
    }
  }
#endif
  if(strcmp(*fout,"-")==0) outfile=stdout;
  else outfile = fopen(*fout,"w");
  if (outfile==NULL) {
    printf ("Error opening output file %s \n",*fout);
    return(1);
  }
  if(verbose) {
    printf("\n   Gxeps $Revision: 1.11 $\n");
#ifndef GXHEPS
    printf("     Input file  = %s\n",*fin);
#endif
    if(strcmp(*fout,"-")==0) printf("output to stdout\n");
    else printf("     Output file = %s\n",*fout);
  }
  /* setvbuf(outfile,NULL,_IOFBF,(size_t) 524288L); */
  return(0);
}
/* --------------------------------------------------------------------------------- */
void setjust(short *opts, short *justx, short *justy)
{	/* 
	 * Set justification
	 * for just>9 an addition border of size height is
	 * added. This is mainly used for axis labels who initial
	 * position touched the axis.
	 */
	short just,wid;
	struct point pnt;

	just=opts[7];
	/*  Remember:
	 *  xpos=opts[1];
	 *  ypos=opts[2];
	 *  height=opts[3].
	 */
	if(just==2) {
		*justx=50;
		*justy=0;
	} else if (just==3) {
		*justx=100;
		*justy=0;
	} else if (just==4) {
		*justx=0;
		*justy=50;
	} else if (just==5) {
		*justx=50;
		*justy=50;
	} else if (just==6) {
		if(opts[5]/64==90) {
			*justx=50;
			*justy=100;
		} else {
			*justx=100;
			*justy=50;
		}
	} else if (just==7) {
		*justx=0;
		*justy=100;
	} else if (just==8) {
		*justx=50;
		*justy=100;
	} else if (just==9) {
		*justx=100;
		*justy=100;
	} else if (just==11) { /* title, x-axis label top */
		*justx=50;
		*justy=100;
		opts[2]+=opts[3];
	} else if (just==12) { /* y-axis label left */
		*justx=100;
		*justy=50;
		opts[1]-=opts[3];
		opts[2]-=opts[3]/2;
	} else if (just==13) { /* yaxis label right */
		*justx=0;
		*justy=50;
		opts[1]+=opts[3]; 
		opts[2]-=opts[3]/2;
	} else if (just==14) { /* title or x-axis label */
		*justx=50;
		*justy=100;
		opts[2]-=2*opts[3];
	}
	/* Guess new bounding box
	 * 
	 * We dont know the exact string width 
	 * and guess it to be height * strlen * 3/4
	 */

	wid=opts[3]*opts[0]*3/4; 
	/* lower left */
	pnt.x=opts[1]-wid * *justx/100;
	pnt.y=opts[2]-opts[3]**justy/100;
	boundbox(&pnt);
	/* lower right */
	pnt.x=opts[1]+wid*(100 - *justx)/100;
	boundbox(&pnt);
	/* upper right */
	pnt.y=opts[2]+opts[3]*(100 - *justy)/100;
	boundbox(&pnt);
	/* upper left */
	pnt.x=opts[1]-wid * *justx/100;
	boundbox(&pnt);
  return;
}
/* --------------------------------------------------------------------------------- */
/*
/Helvetica findfont
dup length dict begin
  { 1 index /FID ne
      {def}
      {pop pop}
    ifelse
  }
/Encoding ISOLatin1Encoding def
  currentdict
end
/Helvetica-ISOLatin1 exch definefont pop

/iso_reencode { 
  dup findfont
  dup length dict begin
  { 1 index /FID ne
      {def}
      {pop pop}
      ifelse
  } forall
  /Encoding ISOLatin1Encoding def
  currentdict end
  exch definefont pop 
} def
 */
/* --------------------------------------------------------------------------------- */
