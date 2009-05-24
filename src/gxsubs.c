/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* Authored by B. Doty */

/*  Low level graphics interface, providing scaling, line styles,
    clipping, character drawing, metafile output, etc.         */

#ifdef HAVE_CONFIG_H
#include "config.h"

/* If autoconfed, only include malloc.h when it's presen */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#else /* undef HAVE_CONFIG_H */

#include <malloc.h>

#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gatypes.h>
#include "gx.h"

#ifdef XLIBEMU
#include "pcx11e.h"
#endif

void *galloc(gaint,char *);
void gree();

/* The following variables are local to this file, and are used by
   all the routines in the file.    */

static char *datad = "/usr/local/lib/grads";
static gadouble xsize, ysize;                /* Virtual size       */
static gadouble rxsize, rysize;              /* Real size          */
static gaint lwflg;                          /* Reduce lw due vpage*/
static gadouble clminx,clmaxx,clminy,clmaxy; /* Clipping region    */
static gaint cflag;                          /* Clipping flag      */
static gaint mflag;                          /* mask flag          */
static gadouble dash[8];                     /* Linestyle pattern  */
static gaint dnum,lstyle;                    /* Current linestyle  */
static gaint color;                          /* Current color      */
static gaint lwide;                          /* Current linewidth  */
static gadouble oldx,oldy;                   /* Previous position  */
static gaint bufmod;                         /* Buffering mode     */
static gadouble xsave,ysave,alen,slen;       /* Linestyle constants*/
static gaint jpen,dpnt;
static gaint bcol;                           /* Background color   */
static gaint intflg;                         /* Batch flag         */
static gaint reds[100],grns[100],blus[100];  /* Save defined color info */
static void (*fconv) (gadouble, gadouble, gadouble *, gadouble *);
                                             /* fconv points to proj rnt  */
static void (*gconv) (gadouble, gadouble, gadouble *, gadouble *);
                                             /* gconv points to grid rnt  */
static void (*bconv) (gadouble, gadouble, gadouble *, gadouble *);
                                             /* gconv points to grid rnt  */
static char *mask;                           /* pointer to mask array */
static gaint maskflg;                        /* mask flag; -999 no mask yet,
                                                0 no mask used, 1 mask values set, -888 error  */
static gaint masksize;                       /* Size of mask array */
static gaint maskx;                          /* Size of a row in the array */

/* For STNDALN, routines included are gxgnam and gxgsym */
#ifndef STNDALN

/* Initialize graphics output  */
/* batch flag = 1, batch mode only (no graphics output) */

void gxstrt (gadouble xmx, gadouble ymx, gaint batch, gaint hbufsz) {
  gaint ii;

  printf ("GX Package Initialization: Size = %g %g \n",xmx,ymx);
  if (batch) printf ("Running in Batch mode\n");
  intflg = !batch;
  if (intflg) {
    gxdbgn (xmx, ymx);
    gxdcol (1);                             /* Initial device color    */
    gxdwid (1);                             /* Initial line width      */
  } else {
    gxdbat ();
  }
  rxsize = xmx;  rysize = ymx;
  clminx=0; clmaxx=xmx;                     /* Set clipping area       */
  clminy=0; clmaxy=ymx;
  xsave=0.0; ysave=0.0; lstyle=0; lwide = 1;
  oldx=0.0; oldy=0.0;
  fconv=NULL;                               /* No projection set up    */
  gconv=NULL;                               /* No grid scaling set up  */
  bconv=NULL;                               /* No back transform       */
  gxscal (0.0,xmx,0.0,ymx,0.0,xmx,0.0,ymx); /* Linear scaling=inches*/
  gxvpag (xmx,ymx,0.0,xmx,0.0,ymx);         /* Virtual page scaling */
  gxchii();                                 /* Init character plotting */
  bufmod=0;
  bcol = 0;                                 /* Background is black     */
  for (ii=0; ii<100; ii++) reds[ii]=-999;
  gxhnew(rxsize,rysize,hbufsz);             /* Init hardcopy buffering */
  color = 1;
  mask = NULL; maskflg = -999;              /* Don't allocate mask until first use */
}

/* Terminate graphics output */

void gxend (void){                     /* Return screen to normal */
  gxhend();
  if (mask) free(mask);
  if (intflg) gxdend();
  printf ("GX package terminated \n");
}

/* Frame action.  Values for action are:
      0 -- new frame (clear display), wait before clearing.
      1 -- new frame, no wait.
      2 -- New frame in double buffer mode.  If not supported
           has same result as action=1.  Usage involves multiple
           calls with action=2 to obtain an animation effect.  
      7 -- new frame, but just clear graphics.  Do not clear  
           event queue; redraw buttons. 
      8 -- clear only the event queue.
      9 -- clear only the X request buffer */

void gxfrme (gaint action) {
gaint scol,i;

  if (action>7) { 
    if (intflg) gxdfrm(action);
    return;
  }
  gxmaskclear();
  if (intflg) {
#ifdef XLIBEMU
    if (action==0) {
        { char cmd[512];
          gets(cmd);
        }
    }
#else
    if (action==0) getchar();              /* Wait if requested    */
#endif
    if (action!=2&&bufmod) {
      gxdsgl ();
      bufmod=0;
    }
    if (action==2&&(!bufmod)) {
      gxddbl ();
      bufmod=1;
    }
    if (bufmod) gxdswp ();
    gxdfrm (action);
  }

  gxhfrm (action);                         /* Reset meta buffer */

  for (i=16; i<100; i++) {
    if (reds[i]>-1) hout4i(-5,i,reds[i],grns[i],blus[i]);
  }
  if (bcol>0) {
    scol = color;
    gxcolr(bcol);
    color = scol;
    gxrecf (0.0, rxsize, 0.0, rysize);
    if (intflg) gxdfrm (9);
  }

}

/* Perform new frame stuff for redraw.  This primarily involves
   the background color. */

void gxsfrm (void) {
  if (bcol>0) {
    gxdcol(bcol);
    gxdrec (0.0, rxsize, 0.0, rysize);
    gxdcol(color);
  }
}

/* Set color.  Colors are: 0 - black;  1 - white
                           2 - red;    3 - green;    4 - blue
                           5 - cyan;   6 - magenta   7 - yellow
                           8 - orange; 9 - purple;  10 - lt. green
                          11 - m.blue 12 - d.yellow 13 - aqua
			  14 - d.purple 15  - gray
   Other colors may be available but are defined by the device
   driver.   */

void gxcolr (gaint clr){                 /* Set color     */
  if (clr<0) clr=0;
  if (clr>99) clr=99;
  hout1(-3,clr);
  if (intflg) gxdcol (clr);
  color = clr;
}

/* Set and query background color */

void gxbckg (gaint col) {
  bcol = col;
}

gaint gxqbck (void) {
  return(bcol);
}

gaint gxacol (gaint clr, gaint red, gaint green, gaint blue ) {
gaint rtn;
  rtn=1;
  hout4i(-5,clr,red,green,blue);
  if (intflg) rtn = gxdacl (clr, red, green, blue);
  if (clr>15 && clr<100) {
    reds[clr] = red;
    grns[clr] = green;
    blus[clr] = blue;
  }
  return rtn;
}


/* Set line weight */

void gxwide (gaint wid){                 /* Set width     */
gaint hwid;
  hwid = wid;
  if (lwflg) hwid = (wid+1)/2;
  hout2i(-4,hwid,wid);
  if (intflg) gxdwid (wid);
  lwide = wid;
}


/* Move to x, y with 'clipping'.  Clipping is implmented
   corsely, where any move or draw point that is outside the
   clip region is not plotted.                          */

void gxmove (gadouble x, gadouble y) {        /* Move to x,y   */
  mflag = 0;
  oldx = x;
  oldy = y;
  if ( x<clminx || x>clmaxx || y<clminy || y>clmaxy ) {
    cflag=1;
    return;
  }
  cflag=0;
  gxvcon(x,y,&x,&y);
  hout2(-10,x,y);
  if (intflg) gxdmov (x,y);
}

/* Draw to x, y with clipping */

void gxdraw (gadouble x, gadouble y){        /* Draw to x,y   */
gadouble xnew,ynew;
gaint pos;
  if ( x<clminx || x>clmaxx || y<clminy || y>clmaxy ) {
    if (!cflag) {
      bdterp (oldx,oldy,x,y,&xnew,&ynew);
      gxvcon(xnew,ynew,&xnew,&ynew);
      hout2(-11,xnew,ynew);
      if (intflg) gxddrw (xnew,ynew);
      cflag=1;
    }
    oldx = x; oldy = y;
    return;
  }
  if (cflag) {
    bdterp (oldx,oldy,x,y,&xnew,&ynew);
    cflag=0;
    gxvcon(xnew,ynew,&xnew,&ynew);
    hout2(-10,xnew,ynew);
    if (intflg) gxdmov (xnew,ynew);
  }
  oldx = x; oldy = y;
  gxvcon(x,y,&x,&y);
  if (maskflg>0) pos = ((gaint)(y*100.0))*maskx + (gaint)(x*100.0);
  if (maskflg>0 && pos>0 && pos<masksize && *(mask+pos)=='1') {
    hout2(-10,x,y);
    if (intflg) gxdmov (x,y);
    mflag = 1;
    return;
  }
  if (mflag) {
    hout2(-10,x,y);
    if (intflg) gxdmov (x,y);
    mflag = 0;
    return;
  }
  hout2(-11,x,y);
  if (intflg) gxddrw (x, y);
}

/* Set software linestyle */

void gxstyl (gaint style) {              /* Set line style  */
  if (style==-9) style=1;
  lstyle=style;
  if (style==2) {
    dnum=1;
    dash[0]=0.25;
    dash[1]=0.1;  }
  else if (style==3) {
    dnum=1;
    dash[0]=0.03;
    dash[1]=0.03;   }
  else if (style==4) {
    dnum=3;
    dash[0]=0.25;
    dash[1]=0.1;
    dash[2]=0.1;
    dash[3]=0.1;   }
  else if (style==5) {
    dnum=1;
    dash[0]=0.01;
    dash[1]=0.08;  }
  else if (style==6) {
    dnum=3;
    dash[0]=0.15;
    dash[1]=0.08;
    dash[2]=0.01; ;
    dash[3]=0.08;   }
  else if (style==7) {
    dnum=5;
    dash[0]=0.15;
    dash[1]=0.08;
    dash[2]=0.01;
    dash[3]=0.08;
    dash[4]=0.01;
    dash[5]=0.08;  }
  else lstyle=0;
  slen=dash[0]; jpen=2; dpnt=0;
}

/* Move and draw with linestyles and clipping */

void gxplot (gadouble x, gadouble y, gaint ipen ) {    /* Move or draw  */
gadouble x1,y1;

  if (lstyle<2) {
     if (ipen==2) gxdraw (x,y);
     else gxmove (x,y);
     xsave=x; ysave=y;
     return;
  }
  if (ipen==3) {
    slen=dash[0];
    dpnt=0;
    jpen=2;
    xsave=x;
    ysave=y;
    gxmove (x,y);
    return;
  }
  alen=hypot ((x-xsave),(y-ysave));
  if (alen<0.001) return;
  while (alen>slen) {
    x1=xsave+(x-xsave)*(slen/alen);
    y1=ysave+(y-ysave)*(slen/alen);
    if (jpen==2) gxdraw (x1,y1);
            else gxmove (x1,y1);
    dpnt+=1;
    if (dpnt>dnum) dpnt=0;
    slen=slen+dash[dpnt];
    jpen+=1;
    if (jpen>3) jpen=2;
  }
  slen=slen-alen;
  xsave=x;
  ysave=y;
  if (jpen==2) gxdraw (x,y);
          else gxmove (x,y);
  if (slen<0.001) {
    dpnt+=1;
    if (dpnt>dnum) dpnt=0;
    slen=dash[dpnt];
    jpen+=1;
    if (jpen>3) jpen=2;
  }
}

/* Specify software clip region.  */

void gxclip (gadouble xmin, gadouble xmax, gadouble ymin, gadouble ymax) {
  clminx = xmin;
  clmaxx = xmax;
  clminy = ymin;
  clmaxy = ymax;
  if (clminx<0.0) clminx = 0.0;
  if (clmaxx>xsize) clmaxx = xsize;
  if (clminy<0.0) clminy = 0.0;
  if (clmaxy>ysize) clmaxy = ysize;
}

/* Constants for linear scaling */

static gadouble xm,xb,ym,yb;

/* Specify low level linear scaling (scaling level 1) */

void gxscal (gadouble xmin, gadouble xmax, gadouble ymin, gadouble ymax,
             gadouble smin, gadouble smax, gadouble tmin, gadouble tmax){
  xm=(xmax-xmin)/(smax-smin);
  xb=xmin-(xm*smin);
  ym=(ymax-ymin)/(tmax-tmin);
  yb=ymin-(ym*tmin);
}

/* Constants for virtual page scaling */

static gadouble vxm,vxb,vym,vyb;

/* Specify virtual page scaling */

void gxvpag (gadouble xmax, gadouble ymax,
             gadouble smin, gadouble smax, gadouble tmin, gadouble tmax){
gadouble xmin, ymin;
  xmin = 0.0;
  ymin = 0.0;
  xsize = xmax;
  ysize = ymax;
  if (smin<0.0) smin=0.0;
  if (smax>rxsize) smax = rxsize;
  if (tmin<0.0) tmin=0.0;
  if (tmax>rysize) tmax = rysize;
  clminx = 0.0;
  clmaxx = xmax;
  clminy = 0.0;
  clmaxy = ymax;
  if ((smax-smin)/rxsize < 0.6 || (tmax-tmin)/rysize < 0.6) lwflg = 1;
  else lwflg = 0;
  vxm=(smax-smin)/(xmax-xmin);
  vxb=smin-(vxm*xmin);
  vym=(tmax-tmin)/(ymax-ymin);
  vyb=tmin-(vym*ymin);
}

/* Do virtual page scaling conversion */

void gxvcon (gadouble s, gadouble t, gadouble *x, gadouble *y) {
  *x = s*vxm+vxb;
  *y = t*vym+vyb;
}
 
void gxppvp (gadouble x, gadouble y, gadouble *s, gadouble *t) {
  *s = (x-vxb)/vxm;
  *t = (y-vyb)/vym;
}


/* Specify projection-level scaling, typically used for map
   projections.  The address of the routine to perform the scaling
   is provided.  This is scaling level 2, and is the level that
   mapping is done. */

void gxproj ( void (*fproj) (gadouble s, gadouble t, gadouble *x, gadouble *y)){

  fconv=fproj;
}

/* Specify grid level scaling, typically used to convert a grid
   to lat-lon values that can be input to the projection or linear
   level scaling.  The address of a routine is provided to perform
   the possibly non-linear scaling.  This is scaling level 3, and
   is the level that contouring is done.  */

void gxgrid ( void (*fproj) (gadouble s, gadouble t, gadouble *x, gadouble *y)){

  gconv=fproj;
}

/* Convert coordinates at a particular level to level 0 coordinates
   (hardware coords, 'inches').  The level of the input coordinates
   is provided.  User projection and grid scaling routines are called
   as needed.  */

void gxconv (gadouble s, gadouble t, gadouble *x, gadouble *y, gaint level) { 

  if (level>2 && gconv!=NULL) (*gconv)(s,t,&s,&t);
  if (level>1 && fconv!=0) (*fconv)(s,t,&s,&t);
  if (level>0) {
    s=s*xm+xb;
    t=t*ym+yb;
  }
  *x=s;
  *y=t;
}

/* Convert from level 0 coordinates (inches) to level 2 world
   coordinates.  The back transform is done via conversion
   linearly from level 0 to level 1, then calling the back
   transform map routine, if available, to do level 1 to level
   2 transform.  */

void gxxy2w (gadouble x, gadouble y, gadouble *s, gadouble *t) { 

  /* Do level 0 to level 1 */
  if (xm==0.0 || ym==0.0) {
    *s = -999.9;
    *t = -999.9;
    return;
  }
  *s = (x-xb)/xm;
  *t = (y-yb)/ym;

  /* Do level 1 to level 2 */
  if (bconv!=NULL) (*bconv)(*s,*t,s,t);
}

/* Allow caller to specify a routine to do the back transform from
   level 1 to level 2 coordinates. */
void gxback ( void (*fproj) (gadouble s, gadouble t, gadouble *x, gadouble *y)){

  bconv=fproj;
}


/* Convert from grid coordinates to map coordinates (level 3 to level 2) */
void gxgrmp (gadouble s, gadouble t, gadouble *x, gadouble *y) {

  if (gconv!=NULL) (*gconv)(s,t,&s,&t);
  *x = s;
  *y = t;
}

/* Convert an array of higher level coordinates to level 0 coordinates.
   The conversion is done 'in place' and the input coordinates are
   lost.  This routine performs the same function as coord except is
   somewhat more efficient for many coordinate transforms.         */

void gxcord (gadouble *coords, gaint num, gaint level) {
gaint i;
gadouble *xy;

  if (level>2 && gconv!=NULL) {
    xy=coords;
    for (i=0; i<num; i++) {
      (*gconv) (*xy,*(xy+1),xy,xy+1);
      xy+=2;
    }
  }

  if (level>1 && fconv!=NULL) {
    xy=coords;
    for (i=0; i<num; i++) {
      (*fconv) (*xy,*(xy+1),xy,xy+1);
      xy+=2;
    }
  }

  if (level>0) {
    xy=coords;
    for (i=0; i<num; i++) {
      *xy = *xy*xm+xb;
      xy++;
      *xy = *xy*ym+yb;
      xy++;
    }
  }
}

/* Delete level 3 or level 2 and level 3 scaling.  
   Level 1 scaling cannot be deleted.  */

void gxrset (gaint level) {

  if (level > 2) gconv=NULL;
  if (level > 1) { fconv=NULL; bconv=NULL; }
}

/* Plot a color filled rectangle.  */

void gxrecf (gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
gadouble x;

  if (xlo>xhi) {
    x = xlo;
    xlo = xhi;
    xhi = x;
  }
  if (ylo>yhi) {
    x = ylo;
    ylo = yhi;
    yhi = x;
  }
  if (xhi<=clminx || xlo>=clmaxx || yhi<=clminy || ylo>=clmaxy) return;
  if (xlo<clminx) xlo = clminx;
  if (xhi>clmaxx) xhi = clmaxx;
  if (ylo<clminy) ylo = clminy;
  if (yhi>clmaxy) yhi = clmaxy;
  gxvcon (xlo,ylo,&xlo,&ylo);
  gxvcon (xhi,yhi,&xhi,&yhi);
  hout4(-6,xlo,xhi,ylo,yhi);
  if (intflg) gxdrec (xlo, xhi, ylo, yhi);
}

/* Define fill pattern for rectangles and polygons. */

void gxptrn (gaint typ, gaint den, gaint ang) {
  hout3i(-12,typ,den,ang);
  if (intflg) gxdptn (typ, den, ang);
}

/* query color */

gaint gxqclr (void) {
  return (color);
}

/* query non-default color rgb values*/

void gxqrgb (gaint clr, gaint *r, gaint *g, gaint *b) {
  if (clr>15 && clr<100) {
    *r = reds[clr];
    *g = grns[clr];
    *b = blus[clr];
  } 
  return;
}

/* query style */

gaint gxqstl (void) {
  return (lstyle);
}

/* Draw markers 1-5. */

void gxmark (gaint mtype, gadouble x, gadouble y, gadouble siz ) {
gadouble xy[80],siz2;
gaint i,ii,cnt;

  siz2 = siz/2.0;
  if (mtype==1) {                      /* cross hair */
    gxmove (x,y-siz2);
    gxdraw (x,y+siz2);
    gxmove (x-siz2,y);
    gxdraw (x+siz2,y);
    return;
  }
  if (mtype==2 || mtype==3 || mtype==10 || mtype==11) { /* circles */
    if (siz<0.1) ii = 30;
    else if (siz<0.3) ii = 15;
    else ii = 10;
    if (mtype>3) ii = 15;
    cnt = 0;
    for (i=60; i<415; i+=ii) {
      xy[cnt*2]   = x + siz2*cos((gadouble)(i)*pi/180.0);
      xy[cnt*2+1] = y + siz2*sin((gadouble)(i)*pi/180.0);
      cnt++;
    }
    xy[cnt*2]   = xy[0];
    xy[cnt*2+1] = xy[1];
    cnt++;
    if (mtype==2) {                  /* Open circle */
      gxmove(xy[0],xy[1]);
      for (i=1; i<cnt; i++) gxdraw (xy[i*2],xy[i*2+1]);
    } else if (mtype==3) {           /* Filled circle */
      gxfill (xy,cnt);
    } else if (mtype==10) {          /* Scattered fill */
      gxmove(xy[6],xy[7]);
      for (i=4; i<14; i++) gxdraw (xy[i*2],xy[i*2+1]);
      gxmove(xy[30],xy[31]);
      for (i=16; i<25; i++) gxdraw (xy[i*2],xy[i*2+1]);
      gxdraw (xy[0],xy[1]);
      for (i=8; i<14; i++) xy[i] = xy[i+18];
      xy[14] = xy[2]; xy[15] = xy[3];
      gxfill (xy+2,7);
    } else if (mtype==11) {          /* Broken fill */
      xy[0]  = x + siz2*cos(68.0*pi/180.0);
      xy[1]  = y + siz2*sin(68.0*pi/180.0);
      xy[8]  = x + siz2*cos(112.0*pi/180.0);
      xy[9]  = y + siz2*sin(112.0*pi/180.0);
      xy[24] = x + siz2*cos(248.0*pi/180.0);
      xy[25] = y + siz2*sin(248.0*pi/180.0);
      xy[32] = x + siz2*cos(292.0*pi/180.0);
      xy[33] = y + siz2*sin(292.0*pi/180.0);
      gxmove(xy[0],xy[1]);
      for (i=1; i<5; i++) gxdraw (xy[i*2],xy[i*2+1]);
      gxmove(xy[24],xy[25]);
      for (i=13; i<17; i++) gxdraw (xy[i*2],xy[i*2+1]);
      xy[26] = xy[8]; xy[27] = xy[9];
      gxfill (xy+8,10);
      xy[50] = xy[0]; xy[51] = xy[1];
      gxfill (xy+32,10);
    }
    return;
  }
  if (mtype==4 || mtype==5) {          /* Draw sqaures */
    xy[0] = x-siz2; xy[1] = y+siz2;
    xy[2] = x+siz2; xy[3] = y+siz2;
    xy[4] = x+siz2; xy[5] = y-siz2;
    xy[6] = x-siz2; xy[7] = y-siz2;
    xy[8] = xy[0]; xy[9] = xy[1];
    if (mtype==4) {
      gxmove (xy[0],xy[1]);
      for (i=1; i<5; i++) gxdraw (xy[i*2],xy[i*2+1]);
    } else {
      gxfill (xy,5);
    }
    return;
  }
  if (mtype==6) {                      /* ex marks the spot */
    gxmove (x-siz2*0.71,y-siz2*0.71);
    gxdraw (x+siz2*0.71,y+siz2*0.71);
    gxmove (x-siz2*0.71,y+siz2*0.71);
    gxdraw (x+siz2*0.71,y-siz2*0.71);
    return;
  }
  if (mtype==7) {                      /* Open diamond */
    gxmove (x-siz2*0.75,y);
    gxdraw (x,y+siz2*1.1);
    gxdraw (x+siz2*0.75,y);
    gxdraw (x,y-siz2*1.1);
    gxdraw (x-siz2*0.75,y);
    return;
  }
  if (mtype==8 || mtype==9) {          /* Triangles */
    xy[0] = x; xy[1] = y+siz2;
    xy[2] = x+siz2*0.88; xy[3] = y-siz2*0.6;
    xy[4] = x-siz2*0.88; xy[5] = y-siz2*0.6;
    xy[6] = x; xy[7] = y+siz2;
    if (mtype==8) {
      gxmove (xy[0],xy[1]);
      for (i=1; i<4; i++) gxdraw (xy[i*2],xy[i*2+1]);
    } else {
      gxfill (xy,4);
    }
    return;
  }
}

/* Plot centered title.  Only supports angle of 0 and 90 */

void gxtitl (char *chrs, gadouble x, gadouble y, gadouble height,
             gadouble width, gadouble angle) {
gadouble xx,yy;
gaint len,i;

  i = 0;
  len = 0;
  while (*(chrs+i)) {
    if (*(chrs+i)!=' ') len=i+1;
    i++;
  }
  if (len==0) return;

  xx = x; yy = y;
  if (angle > 45.0) {
    yy = y - 0.5*width*(gadouble)len;
  } else {
    xx = x - 0.5*width*(gadouble)len;
  }
  gxchpl (chrs, len, xx, yy, height, width, angle);
}

/* Do polygon fill.  It is assumed the bulk of the work will be done
   in hardware.  We do perform clipping at this level, and
   actually do the work to clip at the clipping boundry.       */

void gxfill (gadouble *xy, gaint num) {
gadouble *r, *out, *buff, x, y, xybuff[40];
gaint i,flag,onum,aflag;

  if (num<3) return;

  /* Do clipping.    */

  aflag = 0;
  if (num<10) buff = xybuff;
  else {
    buff = (gadouble *)malloc(sizeof(gadouble)*num*4);
    if (buff==NULL) {
      printf("Memory allocation error in gxfill.  Can't fill contour\n");
      return;
    }
    aflag = 1;
  }

  r = xy;
  out = buff;
  onum = 0;
  flag = 0;
  if (*r<clminx || *r>clmaxx || *(r+1)<clminy || *(r+1)>clmaxy) flag=1;
  for (i=0; i<num; i++) {
    if (*r<clminx || *r>clmaxx || *(r+1)<clminy || *(r+1)>clmaxy) {
      if (!flag) {
        bdterp (*(r-2), *(r-1), *r, *(r+1), &x, &y);
        *out = x;
        *(out+1) = y;
        onum++;
        out+=2;
      }
      *out = *r;
      *(out+1) = *(r+1);
      if (*r<clminx) *out = clminx;
      if (*r>clmaxx) *out = clmaxx;
      if (*(r+1)<clminy) *(out+1) = clminy;
      if (*(r+1)>clmaxy) *(out+1) = clmaxy;
      onum++;
      out+=2;
      flag = 1;
    } else {
      if (flag) {
        bdterp (*(r-2), *(r-1), *r, *(r+1), &x, &y);
        *out = x;
        *(out+1) = y;
        onum++;
        out+=2;
      }
      *out = *r;
      *(out+1) = *(r+1);
      onum++;
      out+=2;
      flag = 0;
    }
    r+=2;
  }

  r = buff;
  for (i=0; i<onum; i++) {
    gxvcon (*r,*(r+1),r,r+1);
    r+=2;
  }

  /* Output to meta buffer if requested.   */

  hout1(-7,onum);
  r = buff;
  hout2(-10,*r,*(r+1));
  r+=2;
  for (i=1; i<onum; i++) {
    hout2(-11,*r,*(r+1));
    r+=2;
  }
  hout0(-8);

  /* Output to hardware */

  if (intflg) gxdfil (buff, onum);
  if (aflag) free(buff);
}

/* Perform edge interpolation for clipping  */

void bdterp (gadouble x1, gadouble y1, gadouble x2, gadouble y2,
             gadouble *x, gadouble *y) {

  if (x1<clminx || x2<clminx || x1>clmaxx || x2>clmaxx) {
    *x = clminx;
    if (x1>clmaxx || x2>clmaxx) *x = clmaxx;
    *y = y1 - ((y1-y2)*(x1-*x)/(x1-x2));
    if (*y<clminy || *y>clmaxy) goto sideh;
    return;
  }

  sideh:

  if (y1<clminy || y2<clminy || y1>clmaxy || y2>clmaxy) {
    *y = clminy;
    if (y1>clmaxy || y2>clmaxy) *y = clmaxy;
    *x = x1 - ((x1-x2)*(y1-*y)/(y1-y2));
    return;
  }
}

void gxbutn (gaint bnum, struct gbtn *pbn) {
  hout1(-20,bnum);
  gxdpbn(bnum, pbn, 0, 0, -1);
}

/* Set mask for a rectangular area */

void gxmaskrec (gadouble xlo, gadouble xhi, gadouble ylo, gadouble yhi) {
gaint siz,i,j,pos,ilo,ihi,jlo,jhi,jj;

  if (maskflg == -888) return;

  if (mask==NULL) {                     /* If not allocated yet, now's the time */
    siz = (gaint)(rxsize*rysize*10000.0); 
    mask = (char *)malloc(siz);
    if (mask==NULL) {
      printf ("Error allocating mask array memory\n");
      printf ("Execution continues with no mask\n");
      maskflg = -888;
      return;
    }
    masksize = siz;
    maskx = (gaint)(rxsize*100.0);
    gxmaskclear();
  } 
  
  ilo = (gaint)(xlo*100.0);
  ihi = (gaint)(xhi*100.0);
  jlo = (gaint)(ylo*100.0);
  jhi = (gaint)(yhi*100.0);
  if (ilo<0) ilo = 0;
  if (ihi<0) ihi = 0;
  if (ilo>maskx) ilo = maskx;
  if (ihi>maskx) ihi = maskx;
  for (j=jlo; j<=jhi; j++) {
    jj = j*maskx;
    for (i=ilo; i<=ihi; i++) {
      pos = jj+i;
      if (pos>=0 || pos<masksize) *(mask+pos) = '1';
    }
  }
  maskflg = 1;
}

/* Set mask to unset state */

void gxmaskclear(void) {
gaint i;   
  if (maskflg > 0)  {
    for (i=0; i<masksize; i++) *(mask+i) = '0';
    maskflg = 0;
  }
}

#endif  /* matches #ifndef STNDALN */


/* Query env symbol */

char *gxgsym(char *ch) {
  return (getenv(ch));
}

/* Construct full file path name from env symbol or default */

char *gxgnam(char *ch) {
char *fname, *ddir;
gaint len,i,j;

  /* calc partial length of output string */
  len = 0;
  i = 0;
  while (*(ch+i)) { i++; len++;}

  /* Query the env symbol */
  ddir = gxgsym("GADDIR");

  /* calc the total length of the output string */
  if (ddir==NULL) {
    i = 0;
    while (*(datad+i)) { i++; len++;}
  } else {
    i = 0;
    while (*(ddir+i)) { i++; len++;}
  }

  /* Allocate memory for the output */
  fname = (char *)galloc(len+15,"fname");
  if (fname==NULL) {
    printf ("Memory allocation error in data set open\n");
    return (NULL);
  }

  /* fill in the directory depending on the value of the env var */
  if (ddir==NULL) {
    i = 0;
    while (*(datad+i)) {
      *(fname+i) = *(datad+i);
      i++;
    }
  } else if (*ddir=='.') {
    i = 0;
  } else {
    i = 0;
    while (*(ddir+i)) {
      *(fname+i) = *(ddir+i);
      i++;
    }
  }

  /* Insure a slash between dir name and file name */
  if (i!=0 && *(fname+i-1)!='/') {
    *(fname+i) = '/';
    i++;
  }

  /* fill in the file name */
  j = 0;
  while (*(ch+j)) {
    *(fname+i) = *(ch+j);
    i++; j++;
  }
  *(fname+i) = '\0';

  return (fname);
}

