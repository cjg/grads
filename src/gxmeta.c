/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* Routines related to hardcopy (metafile) output. */

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

static gaint bopt=BUFOPT;                   /* Buffering option   */
static long buffsz;                         /* metafile buffer size, formerly known as HBUFSZ */
static gaint bflag;                         /* Buffering disabled */
static gaint hflag;                         /* File output enabled*/
static gaint herr;                          /* Fatal error        */
static gaint hferr;                         /* File I/O error     */
static gaint hpflg;                         /* User printed frame */
static short *hbuff, *hpnt, *hend;          /* Current buffer ptrs*/
static FILE *hfile;                         /* meta file pointer  */
static gaint hpos;                          /* Frame pos in metafi*/
static short *bufs[250];                    /* Buffer stuff       */
static short *bufs2[250];                   /* Enough for 50MB    */
static gaint lens[250],lens2[250];
static gaint pnt,pnt2,pntf;     
static gaint dbmode;                        /* Double Buffer mode */
static gadouble xrsize,yrsize;

void xycnv (short, short, gadouble *, gadouble *);

/* Specify on startup what sort of buffering we want.  
   We can do memory buffering (the default on UNIX machines) 
   or file buffering, done only when print is enabled */

void gxhopt (gaint flag) {
  bopt = flag;
}

/* Initialize any buffering, etc. when GrADS starts up */

void gxhnew (gadouble xsiz, gadouble ysiz, gaint hbufsz) {
  buffsz = hbufsz;
  xrsize = xsiz;
  yrsize = ysiz;
  hflag = 0; 
  herr = 0;
  hferr = 0;
  hpflg = 0;
  dbmode = 0;
  if (bopt) bflag = 1;
  else {
    hbuff = (short *)malloc(sizeof(short)*buffsz);
    if (hbuff==NULL) {
      printf ("Unable to allocate memory for metafile operations.\n");
      herr = 1;
      bufs[0] = NULL;
    } else {
      bflag = 0;
      pnt = 1;
      bufs[0] = hbuff;
      hpnt = hbuff;
      hend = hbuff+(buffsz-10L);
    }
  }
  pnt2 = 0;
}

/* Enable hardcopy (metafile) output.   */

gaint gxhbgn (char *fname) {
gaint xx,rc;
short bb[3];

  if (hferr) {
    printf ("Metafile error state is positive.\n");
    printf ("Disable Print before attempting an Enable Print.\n");
    return (0);
  }

  /* Open metafile if not already open.  */

  if (hflag) {
    printf ("Metafile already open\n");
    return(0);
  }

  hfile = fopen(fname,"wb");
  if (hfile==NULL) {
    printf ("Error opening meta file %s \n", fname);
    return (1);
  }

  /* Allocate the memory buffer if we are doing file buffering.  */

  if (bopt) {
    hbuff = (short *)malloc(sizeof(short)*buffsz);
    if (hbuff==NULL) {
      printf ("Unable to allocate memory for metafile operations.\n");
      fclose(hfile);
      return (1);
    }
    hpnt = hbuff;
    hend = hbuff+(buffsz-10L);
    bflag = 0;
  }

  /* Write physical page size to output metafile */

  bb[0] = -1;
  xx = (gaint)(xrsize*1000.0+0.5);
  if (xx<0) xx=0;
  if (xx>32760) xx=32760;
  bb[1] = xx;
  xx = (gaint)(yrsize*1000.0+0.5);
  if (xx<0) xx=0;
  if (xx>32760) xx=32760;
  bb[2] = xx;
  rc = gxhwri(bb, 3);
  if (rc) return (1);
  hpos = ftell(hfile);
  herr = 0;
  hpflg = 0;
  hferr = 0;
  hflag = 1;
  if (bopt) {
    printf ("File buffering enabled. ");
    printf ("Issue CLEAR command to start buffering.\n");
  }
  return (0);
}

/* Metafile output, command with 0 args */

void hout0 (gaint cmd) {
  if (bflag||herr) return;
  *hpnt = cmd;
  hpnt++;
  if (hpnt>hend) hfull();
}

/* Metafile output, command plus one integer argument */

void hout1 (gaint cmd, gaint opt) {
  if (bflag||herr) return;
  *hpnt = cmd;
  hpnt++;
  *hpnt = opt;
  hpnt++;
  if (hpnt>hend) hfull();
}

/* Metafile output, command plus two double args */

void hout2 (gaint cmd, gadouble x, gadouble y) {
gaint xx,yy;
  if (bflag||herr) return;
  *hpnt = cmd;
  hpnt++;
  xx = (gaint)(x*1000.0+0.5);
  if (xx<0) xx=0;
  if (xx>32760) xx=32760;
  *hpnt = xx;
  hpnt++;
  yy = (gaint)(y*1000.0+0.5);
  if (yy<0) yy=0;
  if (yy>32760) yy=32760;
  *hpnt = yy;
  hpnt++;
  if (hpnt>hend) hfull();
}

/* Metafile output, command plus two integer args */

void hout2i (gaint cmd, gaint i1, gaint i2) {

  if (bflag||herr) return;
  *hpnt = cmd;
  hpnt++;
  *hpnt = i1;
  hpnt++;
  *hpnt = i2;
  hpnt++;
  if (hpnt>hend) hfull();
}

/* Metafile output, command plus three integer args */

void hout3i (gaint cmd, gaint i1, gaint i2, gaint i3) {

  if (bflag||herr) return;
  *hpnt = cmd;
  hpnt++;
  *hpnt = i1;
  hpnt++;
  *hpnt = i2;
  hpnt++;
  *hpnt = i3;
  hpnt++;
  if (hpnt>hend) hfull();
}

/* Metafile output, command plus four integer args */

void hout4i (gaint cmd, gaint i1, gaint i2, gaint i3, gaint i4) {

  if (bflag||herr) return;
  *hpnt = cmd;
  hpnt++;
  *hpnt = i1;
  hpnt++;
  *hpnt = i2;
  hpnt++;
  *hpnt = i3;
  hpnt++;
  *hpnt = i4;
  hpnt++;
  if (hpnt>hend) hfull();
}

/* Metafile output, command plus four double args */

void hout4 (gaint cmd, gadouble xl, gadouble xh, gadouble yl, gadouble yh) {
gaint vv;
  if (bflag||herr) return;
  *hpnt = cmd;
  hpnt++;
  vv = (gaint)(xl*1000.0+0.5);
  if (vv<0) vv=0;
  if (vv>32760) vv=32760;
  *hpnt = vv;
  hpnt++;
  vv = (gaint)(xh*1000.0+0.5);
  if (vv<0) vv=0;
  if (vv>32760) vv=32760;
  *hpnt = vv;
  hpnt++;
  vv = (gaint)(yl*1000.0+0.5);
  if (vv<0) vv=0;
  if (vv>32760) vv=32760;
  *hpnt = vv;
  hpnt++;
  vv = (gaint)(yh*1000.0+0.5);
  if (vv<0) vv=0;
  if (vv>32760) vv=32760;
  *hpnt = vv;
  hpnt++;
  if (hpnt>hend) hfull();
}

/* Handle situation where memory buffer is full.  Either output
   to the temporary file buffer, or disable additional input
   into the memory buffer. */

void hfull (void) {
gaint len,rc;

  if (bopt) {
    len = hpnt - hbuff;
    rc = gxhwri(hbuff, len);
    if (rc) return;
    else hpnt = hbuff;
  } else {
    if (dbmode && pntf==1) {
      lens2[pnt2-1] = hpnt-hbuff;
      if (pnt2>249) {
        printf ("Out of buffer space\n");
        herr=1;
        return;
      }
      hbuff = (short *)malloc(sizeof(short)*buffsz);
      if (hbuff==NULL) {
        printf ("Memory allocation error for metafile buffers.\n");
        herr = 1;
      } else {
        bufs2[pnt2] = hbuff;
        hpnt = hbuff;
        hend = hbuff+(buffsz-10L);
        pnt2++;
      } 
    } else {
      if (pnt>249) {
        printf ("Out of buffer space\n");
        herr=1;
        return;
      }
      lens[pnt-1] = hpnt-hbuff;
      hbuff = (short *)malloc(sizeof(short)*buffsz);
      if (hbuff==NULL) {
        printf ("Memory allocation error for metafile buffers.\n");
        herr = 1;
      } else {
        bufs[pnt] = hbuff;
        hpnt = hbuff;
        hend = hbuff+(buffsz-10L);
        pnt++;
      } 
    }
  }
}

/* Output the current frame to the metafile.   Empty the metafile
   buffer, and set things so that the frame will be marked on a
   frame action.

   If no metafile is open, write an EPS file by calling gxheps.
*/

void gxhprt (char *cmd) {
gaint len,i,rc;

  if (bopt && dbmode) {
    printf ("Cannot print while in double-buffer mode ");
    printf ("when using file buffering\n");
    return;
  }
#ifndef STNDALN
  if(!hflag) { /* No metafile open write a EPS file using gxeps */
    gxheps(cmd);
    return;
  }
#endif
  if (herr||hferr) {
    printf ("Error status on print metafile is positive.\n");
    printf ("Cannot print current frame.\n");
    return;
  }
  if (!hflag) {
    printf ("Metafile not currently open\n");
    return;
  }
  if (bopt) {
    len = hpnt - hbuff;
    rc = gxhwri(hbuff,len);
    if (rc) return;
    else hpnt = hbuff;
  } else {
    if (dbmode && pntf==0) {
      i = 0;
      while (i<pnt2) {
        rc = gxhwri(bufs2[i],lens2[i]);
        if (rc) return;
        i++;
      }
    } else {
      if (!dbmode) lens[pnt-1] = hpnt-hbuff;
      i = 0;
      while (i<pnt) {
        rc = gxhwri(bufs[i],lens[i]);
        if (rc) return;
        i++;
      }
    }
  }
  hpflg = 1;
  hpos = ftell(hfile);
  return;
}

/* Write to output metafile */ 

gaint gxhwri (void *buf, gaint len) {
/* kk 020624 --- s */
/*  if (len>0) fwrite (buf, sizeof(short), len, hfile); */
  if (len>0) fwrite (buf, sizeof(short)*len, 1, hfile);
/* kk 020624 --- e */
  if (ferror(hfile)) {
    printf ("I/O Error writing to print metafile.\n");
    fclose(hfile);
    hferr = 1;
    hflag = 0;
    hpflg = 0;
    if (bopt) {
      free(hbuff);
      herr = 1;
      bflag = 1;
    }
    return (1);
  }
  return (0);
}

/* Close the metafile output file without further output */

void gxhend (void) {
short bb[2];
gaint rc;
  if (hferr) {
    printf ("Resetting print metafile error status.\n");
    hferr = 0;
    if (bopt) herr = 0;
    return;
  }
  if (!hflag) {
    printf ("No hardcopy metafile open\n");
    return;
  }
  fseek(hfile,hpos,0L);
  if (hpflg) {
    bb[0] = -2;
    bb[1] = -9;
    rc = gxhwri(bb,2);
  } else {
    bb[0] = -9;
    rc = gxhwri(bb,1);
  }
  if (!rc) {
    fclose(hfile);
    printf ("Hardcopy output file is closed \n");
    hflag = 0;
    if (bopt) {
      free(hbuff);
      bflag = 1;
    }
  }
}

/* User has issued a clear.  Mark an end of frame if needed in
   the output file; free buffers if we are in memory buffering
   mode */

void gxhfrm (gaint iact) {
short bb;
gaint rc,i;

  if (hflag) {
    fseek(hfile,hpos,0L);
    if (hpflg) {
      bb = -2;
      rc = gxhwri(&bb, 1);
      hpflg = 0;
      if (!rc) hpos = ftell(hfile);
    }
  }
  if (iact==2 && dbmode==0) {
    dbmode = 1;
    if (bopt==0) {
      hbuff = (short *)malloc(sizeof(short)*buffsz);
      if (hbuff==NULL) {
        printf ("Memory allocation error for metafile buffers.\n");
        herr = 1;
        pnt2 = 0;
        bufs2[0] = NULL;
      } else {
        pnt2 = 1;
        pntf = 1;
        bufs2[0] = hbuff;
      } 
    }
  }
  if (iact!=2 && dbmode==1) {
    dbmode = 0;
    if (bopt==0) {
      for (i=0; i<pnt2; i++) free(bufs2[i]);
      pnt2 = 0;
    }
  }
  if (bopt==0) {
    if (dbmode) {
      if (pntf==0) {
        lens[pnt-1] = hpnt-hbuff;
        for (i=1; i<pnt2; i++) free(bufs2[i]);   
        pntf = 1;
        pnt2 = 1;
        hbuff = bufs2[0];
        hpnt = hbuff;
        hend = hbuff+(buffsz-10L);
      } else {
        lens2[pnt2-1] = hpnt-hbuff;
        for (i=1; i<pnt; i++) free(bufs[i]);   
        pntf = 0;
        pnt = 1;
        hbuff = bufs[0];
        hpnt = hbuff;
        hend = hbuff+(buffsz-10L);
      }
    } else {
      for (i=1; i<pnt; i++) {
        free(bufs[i]);
      }
      pnt = 1;
      hbuff = bufs[0];
      hpnt = hbuff;
      hend = hbuff+(buffsz-10L);
    }
    if (herr && bufs[0] && !dbmode) herr = 0;
  } else hpnt = hbuff;
}

/* Redraw based on contents of current buffers */

void gxhdrw (gaint dbflg) {
short *poi,*pend;
gaint cmd, i, cnt, flag, ii, siz;
gaint lcolor,fflag,xyc=0;
gadouble xlo,xhi,ylo,yhi,xpos,ypos,*xybuf=NULL;

  if (dbflg && !dbmode) {
    printf ("Logic error 0 in Redraw.  Contact Developer.\n");
    return;
  }

  if (dbmode && pntf==1) {
    lens2[pnt2-1] = hpnt-hbuff;
    cnt = pnt2; flag = 1;
    if (dbflg) {cnt = pnt; flag = 0;}
  } else {
    lens[pnt-1] = hpnt-hbuff;
    cnt = pnt; flag = 0;
    if (dbflg) {cnt = pnt2; flag = 1;}
  }

  gxsfrm ();
  fflag = 0;

  for (ii=0; ii<cnt; ii++) {
    if (flag) {
      poi = bufs2[ii];
      pend = poi + lens2[ii];
    } else { 
      poi = bufs[ii];
      pend = poi + lens[ii];
    } 
    
    while (poi<pend) {

      /* Get message type */

      cmd = *poi; 

      /* Handle various message types */
      /* -9 is end of file.  Should not happen. */

      if (cmd==-9) {
        printf ("Logic Error 4 in Redraw.  Notify Developer\n");
        return;
      }

      /*  -1 indicates start of file.  Should not ocurr. */

      else if (cmd==-1) {
        printf ("Logic Error 8 in Redraw.  Notify Developer\n");
        return;
      }
  
      /* -2 indicates new frame.  Also should not ocurr */

      else if (cmd==-2) {
        printf ("Logic Error 12 in Redraw.  Notify Developer\n");
        return;
      }

      /* -3 indicates new color.  One arg; color number.  */
  
      else if (cmd==-3) {
        lcolor = *(poi+1);
        gxdcol (lcolor);
        poi += 2;
      }

      /* -4 indicates new line thickness.  It has two arguments */
 
      else if (cmd==-4) {
        i = *(poi+2);
        gxdwid(i);
        poi += 3;
      }

      /*  -5 defines a new color, in rgb.  It has four int args */

      else if (cmd==-5){
        gxdacl ((gaint)*(poi+1),(gaint)*(poi+2),(gaint)*(poi+3),(gaint)*(poi+4));
        poi += 5;
      }

      /* -6 is for a filled rectangle.  It has four args. */ 
 
      else if (cmd==-6){
        xycnv (*(poi+1),*(poi+3),&xlo,&ylo);
        xycnv (*(poi+2),*(poi+4),&xhi,&yhi);
        gxdrec(xlo,xhi,ylo,yhi);
        poi += 5;
      }

      /* -7 indicates the start of a polygon fill.  It has one arg. */

      else if (cmd==-7){
        siz = *(poi+1);
        xybuf = (gadouble *)malloc(sizeof(gadouble)*siz*2);
        if (xybuf==NULL) {
          printf ("Memory allocation error: Redraw\n");
          return;
        }
        fflag = 1;
        xyc = 0;
        poi += 2;
      }

      /* -8 is to terminate polygon fill.  It has no args */

      else if (cmd==-8){
        gxdfil (xybuf,xyc);
        if (xybuf==NULL) {
          printf ("Logic Error 16 in Redraw.  Notify Developer\n");
          return;
        } else free (xybuf);
        fflag = 0;
        poi += 1;
      }

      /* -10 is a move to instruction.  It has two double args */ 

      else if (cmd==-10){
        xycnv (*(poi+1),*(poi+2),&xpos,&ypos);
        if (fflag) {
          *(xybuf+xyc*2) = xpos;
          *(xybuf+xyc*2+1) = ypos;
          xyc++;
        } else gxdmov(xpos,ypos);
        poi += 3;
      }

      /*  -11 is draw to.  It has two double args. */  
        
      else if (cmd==-11){
        xycnv (*(poi+1),*(poi+2),&xpos,&ypos);
        if (fflag) {
          xybuf[xyc*2] = xpos;
          xybuf[xyc*2+1] = ypos;
          xyc++;
        } else gxddrw(xpos,ypos);
        poi += 3;
      }

      /* -12 indicates new fill pattern.  It has three arguments. */
 
      else if (cmd==-12) {
        gxdptn ((gaint)*(poi+1),(gaint)*(poi+2),(gaint)*(poi+3));
        poi += 4;
      }

      /* -20 is a draw widget.  We will redraw it in current state. */

      else if (cmd==-20) {
        gxdpbn ((gaint)*(poi+1),NULL,1,0,-1);
        poi += 2;
      }

      /* Any other command would be invalid */

      else {
        printf ("Logic Error 20 in Redraw.  Notify Developer\n");
        return;
      }
    }
  }
}

void xycnv (short ix, short iy, gadouble *x, gadouble *y) {

  *x = ((gadouble)ix)/1000.0;
  *y = ((gadouble)iy)/1000.0;
}

#if GXPNG==1
#include "gxhpng.c"
#endif

#ifndef STNDALN
#  define GXHEPS
#  include "gxeps.c"
#endif
