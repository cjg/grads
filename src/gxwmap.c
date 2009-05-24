/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* Authored by B. Doty */

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
#include <float.h>
#include <gatypes.h>
#include "gx.h"
/* #include "gxmap.h" */

int gagby (char *, int, int);
void gree();
void *galloc(gaint,char *);

static FILE *imap;
static gadouble lomin, lomax, lamin, lamax;
static gadouble lonref;    /* Reference longitude for adjustment */
static gaint adjtyp = 0;  /* Direction adjustment class */

void gxdmap (struct mapopt *mopt) {
gadouble lon[255],lat[255],xx,yy,lnmin,lnmax,ltmin,ltmax,lnfact;
gaint num,i,ipen,rc,type,ilon,ilat,rnum,flag,st1,st2,spos;
gafloat sln1,sln2,slt1,slt2;
gafloat lnsav,ltsav,lndif,ltdif,lntmp,lttmp,llinc,llsum,lldist;
char *fname;
char hdr[3],rec[1530];

  llinc = hypot(mopt->lnmax-mopt->lnmin,mopt->ltmax-mopt->ltmin);
  llinc = llinc/200;
  if (llinc<0.0001) llinc=0.0001;

  /* Open the map data set */

  if (*(mopt->mpdset)=='/' || *(mopt->mpdset)=='\\') {
    imap = fopen(mopt->mpdset,"rb");
    if (imap==NULL) {
      printf ("Open Error on Map Data Set: %s\n",mopt->mpdset);
      return;
    }
  } else {
    fname = gxgnam(mopt->mpdset);
    imap = fopen(fname,"rb");
    if (imap==NULL) {
      imap = fopen(mopt->mpdset,"rb");
      if (imap==NULL) {
        printf ("Open Error on Map Data Set: %s\n",fname);
        gree(fname,"f297");
        return;
      }
    }
    gree(fname,"f298");
  }

  /* Read and process each record */

  rnum = 0;
  while (1) {
    rc = fread(hdr,1,3,imap);
    if (rc!=3) break;
    rnum++;
    i = gagby (hdr,0,1);
    if (i<1 || i>3) {
      printf ("Map file format error: Invalid rec type %i rec num %i\n",i,rnum);
      return;
    }
    if (i==2) {
      st1 = gagby(hdr,1,1);
      st2 = gagby(hdr,2,1);
      fread(rec,1,16,imap);
      spos = gagby(rec,0,4);
      ilon = gagby(rec,4,3);
      sln1 = ((float)ilon)/1e4;
      ilon = gagby(rec,7,3);
      sln2 = ((float)ilon)/1e4;
      ilat = gagby(rec,10,3);
      slt1 = ((float)ilat)/1e4 - 90.0;
      ilat = gagby(rec,13,3);
      slt2 = ((float)ilat)/1e4 - 90.0;
      flag = 0;
      for (i=0; i<256; i++) {
        if (*(mopt->mcol+i)!=-9 && i>=st1 && i<=st2) flag = 1;
      }
      if (flag==0) {
        if (spos==0) {
          fclose(imap);
          return;
        }
        fseek(imap,spos,0);
        continue;
      }
      flag = 0;
      if (sln1>360.0) flag = 1;
      else {
        if (slt2 <= mopt->ltmin || slt1 >= mopt->ltmax) flag = 0;
        else {
          lnfact = 0.0;
          while (sln2+lnfact > mopt->lnmin) lnfact -= 360.0;
          lnfact += 360.0;
          if (sln1+lnfact >= mopt->lnmax) flag = 0;
          else flag = 1;
        }
      }
      if (flag==0) {
        if (spos==0) {
          fclose(imap);
          return;
        }
        fseek(imap,spos,0);
      }
      continue;
    }
    type = gagby(hdr,1,1);
    num = gagby(hdr,2,1);

    /* Read the next record; convert the data points;
       and get the lat/lon bounds for this line segment */

    fread(rec,1,num*6,imap);
    if (*(mopt->mcol+type) == -9) continue;
    if (*(mopt->mcol+type) == -1) {
      gxcolr(mopt->dcol);
      gxstyl(mopt->dstl);
      gxwide(mopt->dthk);
    } else {
      gxcolr(*(mopt->mcol+type));
      gxstyl(*(mopt->mstl+type));
      gxwide(*(mopt->mthk+type));
    }
    lnmin = 9999.9; lnmax = -9999.9; ltmin = 9999.9; ltmax = -9999.9;
    for (i=0; i<num; i++) {
      ilon = gagby(rec,i*6,3);
      ilat = gagby(rec,i*6+3,3);
      lat[i] = ((float)ilat)/1e4 - 90.0;
      lon[i] = ((float)ilon)/1e4;
      if (lat[i]<ltmin) ltmin=lat[i]; if (lat[i]>ltmax) ltmax=lat[i];
      if (lon[i]<lnmin) lnmin=lon[i]; if (lon[i]>lnmax) lnmax=lon[i];
    }

    /* Plot this line segment if it falls within the
       appropriate lat/lon bounds */

    if (ltmax < mopt->ltmin) continue;
    if (ltmin > mopt->ltmax) continue;

    lnfact = 0.0;
    while (lnmax+lnfact > mopt->lnmin) lnfact -= 360.0;
    lnfact += 360.0;

    while (lnmin+lnfact < mopt->lnmax) {
      if (lnmax+lnfact < mopt->lnmin) {
        lnfact += 360.0;
        continue;
      }

      /* Split long lines into shorter segments and limit
         drawing at lat-lon bounds */

      ipen = 3;
      lnsav = lon[0]; ltsav = lat[0];
      for (i=1; i<num; i++) {
        lndif = fabs(lon[i] - lon[i-1]);
        ltdif = fabs(lat[i] - lat[i-1]);
        if (lndif>ltdif) lldist = lndif;
        else lldist = ltdif;
        llsum = llinc;
        lntmp = lnsav; lttmp = ltsav;
        while (llsum<lldist+llinc) {
          if (llsum>=lldist-llinc/4.0) {
            lntmp = lon[i]; lttmp = lat[i];
            llsum += llinc;   /* Insure loop dropout */
          } else {
            if (lndif>ltdif) {
              if (lon[i-1]<lon[i]) {
                lntmp += llinc;
                lttmp += llinc * (lat[i]-lat[i-1])/(lon[i]-lon[i-1]);
              } else {
                lntmp -= llinc;
                lttmp -= llinc * (lat[i]-lat[i-1])/(lon[i]-lon[i-1]);
              }
            } else {
              if (lat[i-1]<lat[i]) {
                lttmp += llinc;
                lntmp += llinc * (lon[i]-lon[i-1])/(lat[i]-lat[i-1]);
              } else {
                lttmp -= llinc;
                lntmp -= llinc * (lon[i]-lon[i-1])/(lat[i]-lat[i-1]);
              }
            }
          }
          if (lntmp+lnfact<mopt->lnmin ||
              lntmp+lnfact>mopt->lnmax ||
              lttmp<mopt->ltmin || lttmp>mopt->ltmax) {
            if (ipen==2) {
              gxconv (lntmp+lnfact,lttmp,&xx,&yy,2);
              gxplot (xx,yy,ipen);
            }
            ipen = 3;
          } else {
            if (ipen==3) {
              gxconv (lnsav+lnfact,ltsav,&xx,&yy,2);
              gxplot (xx,yy,ipen);
            }
            ipen = 2;
            gxconv (lntmp+lnfact,lttmp,&xx,&yy,2);
            gxplot (xx,yy,ipen);
          }
          lnsav = lntmp; ltsav = lttmp;
          llsum += llinc;
        }
      }
      lnfact += 360.0;
    }
  }
  fclose (imap);
}

/* Routine to set up scaling for lat-lon projection.  The aspect
   ratio is *not* maintained.                                   */

int gxscld (struct mapprj *mpj, int xflip, int yflip) {
float x1,x2,y1,y2;

  if (mpj->lnmn>=mpj->lnmx) return(1);
  if (mpj->ltmn>=mpj->ltmx) return(1);
  if (mpj->xmn>=mpj->xmx) return(1);
  if (mpj->ymn>=mpj->ymx) return(1);
  mpj->axmn = mpj->xmn;
  mpj->axmx = mpj->xmx;
  mpj->aymn = mpj->ymn;
  mpj->aymx = mpj->ymx;
  x1 = mpj->lnmn; x2 = mpj->lnmx; y1 = mpj->ltmn; y2 = mpj->ltmx;
  if (xflip) { x1 = mpj->lnmx; x2 = mpj->lnmn; }
  if (yflip) { y1 = mpj->ltmx; y2 = mpj->ltmn; }
  gxscal (mpj->axmn, mpj->axmx, mpj->aymn, mpj->aymx, x1, x2, y1, y2);
  gxproj (NULL);
  adjtyp = 0;
  return (0);
}

/* Routine to set up scaling for lat-lon projection.  Aspect
   ratio of the projection is maintained as a constant, and it
   fills the plotting area as much as possible.                 */

int gxltln (struct mapprj *mpj) {
float lndif,ltdif,aspect,aspect2,xdif,xlo,xhi,ydif,ylo,yhi;

  if (mpj->lnmn>=mpj->lnmx) return(1);
  if (mpj->ltmn>=mpj->ltmx) return(1);
  if (mpj->xmn>=mpj->xmx) return(1);
  if (mpj->ymn>=mpj->ymx) return(1);

  lndif = mpj->lnmx - mpj->lnmn;
  ltdif = mpj->ltmx - mpj->ltmn;
  aspect = 1.2*ltdif/lndif;
  aspect2 = (mpj->ymx - mpj->ymn) / (mpj->xmx - mpj->xmn);
  if (aspect > aspect2) {
    xdif = (mpj->xmx - mpj->xmn) * aspect2/aspect;
    xlo = ((mpj->xmx - mpj->xmn)/2.0)-(xdif*0.5);
    xhi = ((mpj->xmx - mpj->xmn)/2.0)+(xdif*0.5);
    mpj->axmx = mpj->xmn + xhi;
    mpj->axmn = mpj->xmn + xlo;
    mpj->aymn = mpj->ymn;
    mpj->aymx = mpj->ymx;
  } else {
    ydif = (mpj->ymx - mpj->ymn) * aspect/aspect2;
    ylo = ((mpj->ymx - mpj->ymn)/2.0)-(ydif*0.5);
    yhi = ((mpj->ymx - mpj->ymn)/2.0)+(ydif*0.5);
    mpj->aymx = mpj->ymn + yhi;
    mpj->aymn = mpj->ymn + ylo;
    mpj->axmn = mpj->xmn;
    mpj->axmx = mpj->xmx;
  }

  gxscal (mpj->axmn, mpj->axmx, mpj->aymn, mpj->aymx,
          mpj->lnmn, mpj->lnmx, mpj->ltmn, mpj->ltmx);
  gxproj (NULL);
  adjtyp = 0;
  return (0);
}

/* Routine for north polar stereographic.  Projection scaling
   is set along with level 1 linear scaling.   The only difficult
   aspect to this is to set the level 1 linear scaling such that
   the proper aspect ratio is maintained.   */

static float londif;

int gxnste (struct mapprj *mpj) {
gadouble x1,x2,y1,y2,dum,lonave;
gadouble w1,xave,yave;
gadouble lonmn, lonmx, latmn, latmx, xmin, xmax, ymin, ymax;

  lonmn = mpj->lnmn; lonmx = mpj->lnmx;
  latmn = mpj->ltmn; latmx = mpj->ltmx;
  xmin = mpj->xmn; xmax = mpj->xmx;
  ymin = mpj->ymn; ymax = mpj->ymx;

  if ((lonmx-lonmn) > 360.0) {
    return (1);
  }
  if (lonmn<-360.0 || lonmx>360.0) {
    return (1);
  }
  if (latmn<-80.0 || latmx>90.0) {
    return (1);
  }
  if (latmn>=latmx||lonmn>=lonmx||xmin>=xmax||ymin>=ymax) {
    return(1);
  }

  lonave = (lonmx+lonmn)/2.0;            /* Longitude adjustment to put */
  londif = -90.0 - lonave;               /*  central meridian at bottom.*/
  lonref = lonave;

  /* Plotting limits depend on how much of the hemisphere we are
     actually plotting.  */

  if ( (lonmx-lonmn) < 180.0 ) {

     gxnpst ( lonmn, latmn, &x1, &dum );         /* Left side coord  */
     gxnpst ( lonmx, latmn, &x2, &dum );         /* Right side coord */
     gxnpst ( lonmn, latmx, &dum, &y2 );         /* Top coord        */
     gxnpst ( lonave, latmn, &dum, &y1 );        /* Bottom coord     */

  } else {

     gxnpst ( lonave-90.0, latmn, &x1, &dum );   /* Left side coord  */
     gxnpst ( lonave+90.0, latmn, &x2, &dum );   /* Right side coord */
     gxnpst ( lonmn, latmn, &dum, &y2 );         /* Top coord        */
     gxnpst ( lonave, latmn, &dum, &y1 );        /* Bottom coord     */

  }

  /* Set up linear level scaling while maintaining aspect ratio.   */

  if ( ((xmax-xmin)/(ymax-ymin)) > ((x2-x1)/(y2-y1)) ) {
    w1 = 0.5*(ymax-ymin)*(x2-x1)/(y2-y1);
    xave = (xmax+xmin)/2.0;
    gxscal ( xave-w1, xave+w1, ymin, ymax, x1, x2, y1, y2 );
    mpj->axmn = xave-w1;  mpj->axmx = xave+w1;
    mpj->aymn = ymin;  mpj->aymx = ymax;
  } else {
    w1 = 0.5*(xmax-xmin)*(y2-y1)/(x2-x1);
    yave = (ymax+ymin)/2.0;
    gxscal ( xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
    mpj->axmn = xmin;  mpj->axmx = xmax;
    mpj->aymn = yave-w1;  mpj->aymx = yave+w1;
  }

  gxproj (gxnpst);
  gxback (gxnrev);
  adjtyp = 1;
  return (0);

}

void gxnpst (gadouble rlon, gadouble rlat, gadouble *x, gadouble *y) {
gadouble radius,theta;

  radius = tan (0.785315-(0.00872572*rlat));
  theta = (rlon+londif)*0.0174514;
  *x = radius * cos(theta);
  *y = radius * sin(theta);
}

/* Routine for back transform for npst */

void gxnrev (gadouble x, gadouble y, gadouble *rlon, gadouble *rlat) {
gadouble rad,alpha;

  rad = hypot(x,y);
  alpha = 180.0*atan(rad)/pi;
  *rlat = 90.0 - 2.0*alpha;

  if (x==0.0 && y==0.0) *rlon = 0.0;
  else {
    *rlon = (180.0*atan2(y,x)/pi)-londif;
    while (*rlon < lonref-180.0) *rlon += 360.0;
    while (*rlon > lonref+180.0) *rlon -= 360.0;
  }
}

/* Routine for south polar stereographic.  Projection scaling
   is set along with level 1 linear scaling.   The only difficult
   aspect to this is to set the level 1 linear scaling such that
   the proper aspect ratio is maintained.   */

int gxsste (struct mapprj *mpj) {
gadouble x1,x2,y1,y2,dum,lonave;
gadouble w1,xave,yave;
gadouble lonmn, lonmx, latmn, latmx, xmin, xmax, ymin, ymax;

  lonmn = mpj->lnmn; lonmx = mpj->lnmx;
  latmn = mpj->ltmn; latmx = mpj->ltmx;
  xmin = mpj->xmn; xmax = mpj->xmx;
  ymin = mpj->ymn; ymax = mpj->ymx;

  if ((lonmx-lonmn) > 360.0) {
    return (1);
  }
  if (lonmn<-360.0 || lonmx>360.0) {
    return (1);
  }
  if (latmn<-90.0 || latmx>80.0) {
    return (1);
  }
  if (latmn>=latmx||lonmn>=lonmx||xmin>=xmax||ymin>=ymax) {
    return(1);
  }

  lonave = (lonmx+lonmn)/2.0;            /* Longitude adjustment to put */
  londif = -90.0 - lonave;               /*  central meridian at bottom.*/
  lonref = lonave;

  /* Plotting limits depend on how much of the hemisphere we are
     actually plotting.  */

  if ( (lonmx-lonmn) < 180.0 ) {

     gxspst ( lonmn, latmx, &x1, &dum );         /* Left side coord  */
     gxspst ( lonmx, latmx, &x2, &dum );         /* Right side coord */
     gxspst ( lonmn, latmn, &dum, &y1 );         /* Top coord        */
     gxspst ( lonave, latmx, &dum, &y2 );        /* Bottom coord     */

  } else {

     gxspst ( lonave-90.0, latmx, &x1, &dum );   /* Left side coord  */
     gxspst ( lonave+90.0, latmx, &x2, &dum );   /* Right side coord */
     gxspst ( lonmn, latmx, &dum, &y1 );         /* Top coord        */
     gxspst ( lonave, latmx, &dum, &y2 );        /* Bottom coord     */

  }

  /* Set up linear level scaling while maintaining aspect ratio.   */

  if ( ((xmax-xmin)/(ymax-ymin)) > ((x2-x1)/(y2-y1)) ) {
    w1 = 0.5*(ymax-ymin)*(x2-x1)/(y2-y1);
    xave = (xmax+xmin)/2.0;
    gxscal ( xave-w1, xave+w1, ymin, ymax, x1, x2, y1, y2 );
    mpj->axmn = xave-w1;  mpj->axmx = xave+w1;
    mpj->aymn = ymin;  mpj->aymx = ymax;
  } else {
    w1 = 0.5*(xmax-xmin)*(y2-y1)/(x2-x1);
    yave = (ymax+ymin)/2.0;
    gxscal ( xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
    mpj->axmn = xmin;  mpj->axmx = xmax;
    mpj->aymn = yave-w1;  mpj->aymx = yave+w1;
  }
  gxproj (gxspst);
  gxback (gxsrev);
  adjtyp = 2;
  return (0);

}

void gxspst (gadouble rlon, gadouble rlat, gadouble *x, gadouble *y) {
gadouble radius,theta;

  radius = tan(0.785315+(0.00872572*rlat));
  theta = (rlon+londif)*(-0.0174514);
  *x = radius * cos(theta);
  *y = radius * sin(theta);
}

/* Routine for back transform for spst */

void gxsrev (gadouble x, gadouble y, gadouble *rlon, gadouble *rlat) {
gadouble rad,alpha;

  rad = hypot(x,y);
  alpha = 180.0*atan(rad)/pi;
  *rlat = 2.0*alpha - 90.0;

  if (x==0.0 && y==0.0) *rlon = 0.0;
  else {
    *rlon = (-180.0*atan2(y,x)/pi)-londif;
    while (*rlon < lonref-180.0) *rlon += 360.0;
    while (*rlon > lonref+180.0) *rlon -= 360.0;
  }
}

/* Return adjustment angle (in radians) to apply to a wind direction
   to correct for current map projection and position. */

gadouble gxaarw (gadouble lon, gadouble lat) {

  if (adjtyp==0) return(0.0);
  if (adjtyp==1) {
    lon = (lon - lonref)*pi/180.0;
    return (lon);
  }
  if (adjtyp==2) {
    lon = (lonref - lon)*pi/180.0;
    return (lon);
  }
  return (0.0);
}

/*  Set up Robinson Projection */

int gxrobi (struct mapprj *mpj) {
gadouble lonmn, lonmx, latmn, latmx, xmin, xmax, ymin, ymax;
gadouble x1,x2,y1,y2,xd,yd,xave,yave,w1;

  lonmn = mpj->lnmn; lonmx = mpj->lnmx;
  latmn = mpj->ltmn; latmx = mpj->ltmx;
  xmin = mpj->xmn; xmax = mpj->xmx;
  ymin = mpj->ymn; ymax = mpj->ymx;

  /* Check for errors */

  if (lonmn<-180.0 || lonmx>180.0 || latmn<-90.0 || latmx>90.0) {
    return (1);
  }
  if (latmn>=latmx||lonmn>=lonmx||xmin>=xmax||ymin>=ymax) {
    return(1);
  }

  /* Get bounds of the map in linear units */

  gxrobp ( lonmn, latmn, &x1, &y1);           /* Lower Left       */
  gxrobp ( lonmn, latmx, &xd, &y2);           /* Upper Left       */
  if (xd<x1) x1 = xd;
  if (latmn<0.0 && latmx>0.0) {
    gxrobp (lonmn, 0.0, &xd, &yd);            /* Left Middle      */
    if (xd<x1) x1 = xd;
  }
  gxrobp ( lonmx, latmn, &x2, &y1);           /* Lower Right      */
  gxrobp ( lonmx, latmx, &xd, &y2);           /* Upper Right      */
  if (xd>x2) x2 = xd;
  if (latmn<0.0 && latmx>0.0) {
    gxrobp (lonmx, 0.0, &xd, &yd);            /* Right Middle     */
    if (xd>x2) x2 = xd;
  }

  /* Set up linear level scaling while maintaining aspect ratio.   */

  if ( ((xmax-xmin)/(ymax-ymin)) > ((x2-x1)/(y2-y1)) ) {
    w1 = 0.5*(ymax-ymin)*(x2-x1)/(y2-y1);
    xave = (xmax+xmin)/2.0;
    gxscal ( xave-w1, xave+w1, ymin, ymax, x1, x2, y1, y2 );
    mpj->axmn = xave-w1;  mpj->axmx = xave+w1;
    mpj->aymn = ymin;  mpj->aymx = ymax;
  } else {
    w1 = 0.5*(xmax-xmin)*(y2-y1)/(x2-x1);
    yave = (ymax+ymin)/2.0;
    gxscal ( xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
    mpj->axmn = xmin;  mpj->axmx = xmax;
    mpj->aymn = yave-w1;  mpj->aymx = yave+w1;
  }

  gxproj (gxrobp);
  gxback (gxrobb);
  adjtyp = 3;
  return (0);
}

/* Transform routine for Robinson Projection */

gadouble rob1[37] = {-1.349,-1.317,-1.267,-1.206,-1.138,-1.066,-0.991,
    -0.913,-0.833,-0.752,-0.669,-0.586,-0.502,-0.418,-0.334,-0.251,
    -0.167,-0.084,0.000,0.084,0.167,0.251,0.334,0.418,0.502,0.586,
    0.669,0.752,0.833,0.913,0.991,1.066,1.138,1.206,1.267,1.317,1.349};
gadouble rob2[37] = {1.399,1.504,1.633,1.769,1.889,1.997,2.099,
    2.195,2.281,2.356,2.422,2.478,2.532,2.557,2.582,2.602,2.616,
    2.625,2.628,2.625,2.616,2.602,2.582,2.557,2.532,2.478,2.422,
    2.356,2.281,2.195,2.099,1.997,1.889,1.769,1.633,1.504,1.399};

void gxrobp (gadouble rlon, gadouble rlat, gadouble *x, gadouble *y) {
int i;
  rlat = (rlat+90.0)/5.0;
  i = (int)rlat;
  rlat = rlat - (gadouble)i;
  if (i<0) {
    *y = -1.349;
    *x = 1.399*rlon/180.0;
    return;
  }
  if (i>=36) {
    *y = 1.349;
    *x = 1.399*rlon/180.0;
    return;
  }
  *y = rob1[i] + rlat*(rob1[i+1]-rob1[i]);
  *x = rob2[i] + rlat*(rob2[i+1]-rob2[i]);
  *x = *x * rlon/180.0;
  return;
}

/* Back Transform for Robinson Projection */

void gxrobb (gadouble x, gadouble y, gadouble *rlon, gadouble *rlat) {
}
/*------------------------------------------------------------------
     DKRZ appends: Mollweide Projection
     10.08.95   Karin Meier (karin.meier@dkrz.de)
  ------------------------------------------------------------------*/

int gxmoll (struct mapprj *mpj) {
    gadouble lonmn, lonmx, latmn, latmx, xmin, xmax, ymin, ymax;
    gadouble x1,x2,y1,y2,xd,yd,xave,yave,w1;

	lonmn  = mpj->lnmn; lonmx = mpj->lnmx;
	latmn  = mpj->ltmn; latmx = mpj->ltmx;
	xmin   = mpj->xmn;  xmax  = mpj->xmx;
	ymin   = mpj->ymn;  ymax  = mpj->ymx;
	lomin  = lonmn;     lomax = lonmx;
	lamin  = latmn;     lamax = latmx;

/* Check for errors */

  	if (latmn<-90.0 || latmx>90.0) {
    	  return (1);
  	}
  	if (latmn>=latmx||lonmn>=lonmx||xmin>=xmax||ymin>=ymax) {
  	  return(1);
  	}

  /* Get bounds of the map in linear units */

  	gxmollp ( lonmn, latmn, &x1, &y1);           /* Lower Left       */
  	gxmollp ( lonmn, latmx, &xd, &y2);           /* Upper Left       */
  	if (xd<x1) x1 = xd;
  	if (latmn<0.0 && latmx>0.0) {
  	  gxmollp (lonmn, 0.0, &xd, &yd);            /* Left Middle      */
  	  if (xd<x1) x1 = xd;
  	}
  	gxmollp ( lonmx, latmn, &x2, &y1);           /* Lower Right      */
  	gxmollp ( lonmx, latmx, &xd, &y2);           /* Upper Right      */
  	if (xd>x2) x2 = xd;
  	if (latmn<0.0 && latmx>0.0) {
  	  gxmollp (lonmx, 0.0, &xd, &yd);            /* Right Middle     */
  	  if (xd>x2) x2 = xd;
  	}

  /* Set up linear level scaling while maintaining aspect ratio.   */

  	if ( ((xmax-xmin)/(ymax-ymin)) > ((x2-x1)/(y2-y1)) ) {
  	  w1 = 0.5*(ymax-ymin)*(x2-x1)/(y2-y1);
 	  xave = (xmax+xmin)/2.0;
  	  gxscal ( xave-w1, xave+w1, ymin, ymax, x1, x2, y1, y2 );
  	  mpj->axmn = xave-w1;  mpj->axmx = xave+w1;
  	  mpj->aymn = ymin;     mpj->aymx = ymax;
  	} else {
  	  w1 = 0.5*(xmax-xmin)*(y2-y1)/(x2-x1);
	  yave = (ymax+ymin)/2.0;
  	  gxscal ( xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
  	  mpj->axmn = xmin;     mpj->axmx = xmax;
  	  mpj->aymn = yave-w1;  mpj->aymx = yave+w1;
  	}

  	gxproj (gxmollp);
  	gxback (gxmollb);
  	adjtyp = 3;
  	return (0);
}

void gxmollp (gadouble rlon, gadouble rlat, gadouble *x, gadouble *y) {
   gadouble diff, radlat, radlon;

	if (lomin != -180.0) {
	  diff = -180.0 - lomin;
	  rlon = rlon + diff;
	}
	radlat = (pi*rlat)/180.0;
	radlon = (pi*rlon)/180.0;

	*x = cos(radlat);
	*y = sin(radlat)/2.0;
	*x = *x*rlon/180.0;

  	return;
}

/* Back Transform for Mollweide Projection */

void gxmollb (gadouble x, gadouble y, gadouble *rlon, gadouble *rlat) {
}

int gxortg (struct mapprj *mpj) {
    gadouble lonmn, lonmx, latmn, latmx, xmin, xmax, ymin, ymax;
    gadouble x1,x2,y1,y2,xd,yd,xave,yave,w1;

	lonmn  = mpj->lnmn; lonmx = mpj->lnmx;
	latmn  = mpj->ltmn; latmx = mpj->ltmx;
	xmin   = mpj->xmn;  xmax  = mpj->xmx;
	ymin   = mpj->ymn;  ymax  = mpj->ymx;
	lomin  = lonmn;     lomax = lonmx;
	lamin  = latmn;     lamax = latmx;
	lonref = (lonmx+lonmn)/2.0;

  /* Check boundaries */

	if (latmn != -90.0 || latmx != 90.0) {
	   printf("Map Projection Error:  Latitude must be in range -90 90\n");
	   return (1);
	}
	if ((lonmx - lonmn) > 180.0 ) {
	   printf("Map Projection Error:  %.1f - %.1f  > 180.0\n",
		   lonmx, lonmn);
	   return (1);
	}
	if ((lonmx - lonmn) < 180.0) {
	   printf("Map Projection Error:  %.1f - %.1f  < 180.0\n",
		   lonmx, lonmn);
	   return (1);
	}
  	if (latmn>=latmx||lonmn>=lonmx||xmin>=xmax||ymin>=ymax) return(1);

	if (lonmn < -180.0) {
	    mpj->lnmn = lonmn + 360.0;
	    mpj->lnmx = lonmx + 360.0;
	    lonmn  = mpj->lnmn; lonmx = mpj->lnmx;
	}
	if (lonmx > 180.0 ) {
	    mpj->lnmn = lonmn - 360.0;
	    mpj->lnmx = lonmx - 360.0;
	    lonmn  = mpj->lnmn; lonmx = mpj->lnmx;
	}

  /* Get bounds of the map in linear units */

	  gxortgp ( lonmn,  latmn, &x1, &y1);
	  gxortgp ( lonmn,  latmx, &xd, &y2);
	  if (xd<x1) x1 = xd;
	  if (latmn<0.0 && latmx>0.0) {
  	     gxortgp ( lonmn, 0.0, &xd, &yd);
	     if (xd<x1)	x1 = xd;
  	  }
	  gxortgp ( lonmx,  latmn, &x2, &y1);
	  gxortgp ( lonmx,  latmx, &xd, &y2);
	  if(xd>x2) x2 = xd;
	  if (latmn<0.0 && latmx>0.0) {
	     gxortgp ( lonmx, 0.0, &xd, &yd);
	     if (xd>x2) x2 = xd;
	  }

  /* Set up linear level scaling while maintaining aspect ratio.   */

  	if ( ((xmax-xmin)/(ymax-ymin)) > ((x2-x1)/(y2-y1)) ) {
  	  w1 = 0.5*(ymax-ymin)*(x2-x1)/(y2-y1);
 	  xave = (xmax+xmin)/2.0;
  	  gxscal ( xave-w1, xave+w1, ymin, ymax, x1, x2, y1, y2 );
  	  mpj->axmn = xave-w1;  mpj->axmx = xave+w1;
  	  mpj->aymn = ymin;     mpj->aymx = ymax;
  	} else {
  	  w1 = 0.5*(xmax-xmin)*(y2-y1)/(x2-x1);
	  yave = (ymax+ymin)/2.0;
  	  gxscal ( xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
  	  mpj->axmn = xmin;     mpj->axmx = xmax;
  	  mpj->aymn = yave-w1;  mpj->aymx = yave+w1;
  	}

  	gxproj (gxortgp);
  	gxback (gxortgb);
  	adjtyp = 3;
  	return (0);
}

void gxortgp (gadouble rlon, gadouble rlat, gadouble *x, gadouble *y) {

  gadouble radlat, radlon, diff;

  if (lomin != -90.0) {
    diff = -90.0 - lomin;
    rlon = rlon + diff;
  }
  
  radlat = (pi*rlat)/180.0;
  radlon = (pi*rlon)/180.0;
  
  *x = cos(radlat);
  *y = sin(radlat);
  *x = *x * sin(radlon);
  
  return;
}

/* Back Transform for Orthographic Projection */

void gxortgb (gadouble x, gadouble y, gadouble *rlon, gadouble *rlat) {
}

/*------------------------------------------------------------------
     DKRZ appends: Lambert conformal conic Projection
     15.03.96                       Karin Meier (karin.meier@dkrz.de)
  ------------------------------------------------------------------*/
static gadouble  hemi, r;

int gxlamc (struct mapprj *mpj) {
  gadouble  lonmn, lonmx, latmn, latmx, dlat, dlon, dx, dy;
  gadouble  xave,yave, w1, lonave, xmin, xmax, ymin, ymax, x1, x2, y1, y2, xd, yd;

	lonmn  = mpj->lnmn;          lonmx = mpj->lnmx;
	latmn  = mpj->ltmn;          latmx = mpj->ltmx;
	xmin   = mpj->xmn;           xmax  = mpj->xmx;
	ymin   = mpj->ymn;           ymax  = mpj->ymx;
	lomin  = lonmn;              lomax = lonmx;
	lamin  = latmn;              lamax = latmx;
	lonave = (lonmx+lonmn)/2.0;
	dlat   = lamax - lamin;	     dlon  = lomax - lomin;
	dx     = xmax - xmin;	     dy    = ymax - ymin;

  	if ((lonmn>=lonmx)||(latmn>=latmx)||(xmin>=xmax)||(ymin>=ymax)) {
	   return(1);
  	}
	if (((latmn > 0.0) && (latmx < 0.0)) || ((latmn < 0.0) && (latmx >0.0))) {
	   printf("Map Projection Error:  Latitude must be in range -90 0 or 0 90\n");
	   return (1);
	}

/*--- set constant for northern or southern hemisphere  ---*/

	if (latmn >= 0.0) {
	    hemi = 1.0;		/** northern hemisphere **/
	} else {
	    hemi = -1.0;	/** southern hemisphere **/
	}

/*--- reset 90.0/-90.0 degrees to 89.99/-89.99 because of tangent  ---*/

	if (latmn == -90.0) latmn = -89.99;
	if (latmx ==  90.0) latmx =  89.99;

/*--- get viewport coordinates  x1, x2, y1, y2---*/

	gxlamcp (lonmn, latmn, &x1, &y1);
	gxlamcp (lonmn, latmx, &xd, &y2);
	if (xd<x1)  x1=xd;
	if (y2<y1) {
	    yd = y2;  y2 = y1;  y1 = yd;
	}
	if (latmn>=0.0 && latmx>0.0) {
	    gxlamcp (lonmn,0.0,&xd,&yd);
	    if (xd<x1) x1=xd;
	}
	gxlamcp (lonmx, latmn, &x2, &y1);
	gxlamcp (lonmx, latmx, &xd, &y2);
	if (xd>x2)  x2=xd;
	if (y2<y1) {
	    yd = y2;  y2 = y1;  y1 = yd;
	}
	if (latmn<0.0 && latmx<=0.0) {
	    gxlamcp (lonmx,0.0,&xd,&yd);
	    if (xd>x2) x2=xd;
	}

/*--- determining terms for scaling  ---*/

	xave = (xmin+xmax)/2.0;
	yave = (ymin+ymax)/2.0;

 	if ( ((xmax-xmin)/(ymax-ymin)) > ((x2-x1)/(y2-y1)) )
	{
	  if (hemi==-1.0 && 180.0<(lomax-lomin) && (lomax-lomin)<=270.0)
		yave -= 1.5;
	  else if (hemi==1.0 && 180.0<(lomax-lomin) && (lomax-lomin)<=270.0)
		yave += 1.5;
	  else if (hemi==-1.0 && 270.0<=(lomax-lomin) && (lomax-lomin)<=360.0)
		yave -= 1.2;
	  else if (hemi==1.0 && 270.0<=(lomax-lomin) && (lomax-lomin)<=360.0)
		yave += 1.2;
	  else if (hemi==-1.0 && 90.0<(lomax-lomin) && (lomax-lomin)<=180.0)
		yave -= 0.5;
	  else if (hemi==1.0 && 90.0<(lomax-lomin) && (lomax-lomin)<=180.0)
		yave += 1.0;
	  else if (hemi==-1.0 && (lomax-lomin)<=90.0)
		yave += 0.0;
	  else if (hemi==1.0 && (lomax-lomin)<=90.0)
		yave += 1.0;

  	 w1 = 0.5*(ymax-ymin)*(x2-x1)/(y2-y1);
	  if (w1 < 1.0)
		gxscal (xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
	  else if (w1 < 2.0)
		gxscal (xave-0.5*(w1), xave+0.5*w1, yave-w1, yave+w1,
			x1, x2, y1, y2 );
	  else if (w1 < 3.0)
		gxscal (xave-0.5*w1, xave+0.5*w1, yave-w1, yave+w1,
			x1, x2, y1, y2 );
	  else if (w1 > 3.0)
		gxscal (xave-0.75*w1, xave+0.75*w1, yave-0.75*w1,
			yave+0.75*w1, x1, x2, y1, y2 );
	  else
  	  	gxscal (xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
	}
	else
	{
	  if (hemi==-1.0 && 180.0<(lomax-lomin) && (lomax-lomin)<=270.0)
		yave -= 1.0;
	  else if (hemi==1.0 && 180.0<(lomax-lomin) && (lomax-lomin)<=270.0)
		yave += 1.5;
	  else if (hemi==-1.0 && 270.0<=(lomax-lomin) && (lomax-lomin)<=360.0)
		yave -= 1.0;
	  else if (hemi==1.0 && 270.0<=(lomax-lomin) && (lomax-lomin)<=360.0)
		yave += 1.0;
	  else if (hemi==-1.0 && 90.0<(lomax-lomin) && (lomax-lomin)<=180.0)
		yave -= 0.5;
	  else if (hemi==1.0 && 90.0<(lomax-lomin) && (lomax-lomin)<=180.0)
		yave += 1.0;
	  else if (hemi==-1.0 && (lomax-lomin)<=90.0)
		yave += 0.0;
	  else if (hemi==1.0 && (lomax-lomin)<=90.0)
		yave += 1.0;

  	  w1 = 0.5*(xmax-xmin)*(y2-y1)/(x2-x1);
	  if (w1 < 1.0)
		gxscal (xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
	  else if (w1 < 2.0)
		gxscal (xmin+0.5*w1, xmax-0.5*w1, yave-1.25*w1,
			yave+1.25*w1, x1, x2, y1, y2 );
	  else if (w1 < 3.0)
		gxscal (xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
	  else if (w1 > 3.0)
		gxscal (xave-0.5*w1, xave+0.5*w1, yave-0.5*w1,
			yave+0.5*w1, x1, x2, y1, y2 );
	  else
  	  	gxscal (xmin, xmax, yave-w1, yave+w1, x1, x2, y1, y2 );
	}

  	mpj->axmn = xmin;  mpj->axmx = xmax;
  	mpj->aymn = ymin;  mpj->aymx = ymax;

  	gxproj (gxlamcp);
	gxback (gxlamcb);
  	adjtyp = 0;
  	return (0);
}



/*--- transform routine for lambert conformal conic projection  ---*/

void gxlamcp (gadouble rlon, gadouble rlat, gadouble *x, gadouble *y)
{
	gadouble d2r, cone, phis, phin, clon, term1, term2;

	d2r    = pi/180.0;

/*--- standard latitudes:  north - phin;  south - phis  ---*/
	phis   = lamin;
	phin   = lamax;

/*--- reset 90.0/-90.0 degrees to 89.99/-89.99 because of tangent  ---*/
        if(phis == -90.0)  phis = -89.99;
        if(phin ==  90.0)  phin =  89.99;

/*--- calculate the constant of the cone +++ radius, x, y ---*/
/*--- clon -  central meridian;    cone -  cone constant  ---*/
	 clon  = floor((lomax + lomin)/2.0);
	 term1 = tan((45.0-hemi*phis/2.0)*d2r);
	 term2 = tan((45.0-hemi*phin/2.0)*d2r);

         if(phis!=phin)
		cone = (log10(cos(phis*d2r))-log10(cos(phin*d2r)))/
                       (log10(term1)-log10(term2));
	 else
		cone = cos((90.0-hemi*phis)*d2r);

	  r = pow(tan((45.0-hemi*rlat/2.0)*d2r),cone);
	 *x =        r*sin((rlon-clon)*d2r*cone);
	 *y =  -hemi*r*cos((rlon-clon)*d2r*cone);

  	 return;
}



/*--- Back Transform for Lambert conformal Projection ---*/

void gxlamcb (gadouble x, gadouble y, gadouble *rlon, gadouble *rlat) {
}

/* Interpolate lat/lon boundaries, and convert to xy, on 
   behalf of 'draw mappoly' .  For most part, the same
   code as in gxdmap  */

gadouble *gxmpoly(gadouble *xy, gaint cnt, gadouble llinc, gaint *newcnt) {
gadouble ln1, ln2, lt1, lt2, lnsav, ltsav, llsum;
gadouble lndif, ltdif, lldist, lntmp, lttmp, xx, yy; 
gadouble *newxy;
gaint i,j,ip,ncnt;

  /* Determine total 'path' length */

  llsum = 0.0;
  for (i=1; i<cnt; i++) {
    ip = (i-1)*2;
    lndif = fabs(*(xy+ip+2) - *(xy+ip));
    ltdif = fabs(*(xy+ip+3) - *(xy+ip+1));
    if (lndif>ltdif) lldist = lndif;
    else lldist = ltdif;
    llsum += lldist;
  }

  /* Estimate number of output points, and allocate storage for them. */

  ncnt = cnt + llsum/llinc;
  newxy = (gadouble *)galloc(sizeof(gadouble)*ncnt*2,"newxy");
  if (newxy==NULL) return(NULL);  /* caller issues error */

  /* Now interpolate each point, convert to x,y, and put in list */

  j = 0;
  lnsav = *xy; ltsav = *(xy+1);
  for (i=1; i<cnt; i++) {
    ip = (i-1)*2;
    ln1 = *(xy+ip); ln2 = *(xy+ip+2);
    lt1 = *(xy+ip+1); lt2 = *(xy+ip+3);
    lndif = fabs(ln2-ln1);
    ltdif = fabs(lt2-lt1);
    if (lndif>ltdif) lldist = lndif;
    else lldist = ltdif;
    llsum = llinc;
    lntmp = lnsav; lttmp = ltsav;
    while (llsum<lldist+llinc) {
      if (llsum>=lldist-llinc/4.0) {
        lntmp = ln2; lttmp = lt2;
        llsum += llinc;   /* Insure loop dropout */
      } else {
        if (lndif>ltdif) {
          if (ln1<ln2) {
            lntmp += llinc;
            lttmp += llinc * (lt2-lt1)/(ln2-ln1);
          } else {
            lntmp -= llinc;
            lttmp -= llinc * (lt2-lt1)/(ln2-ln1);
          }
        } else {
          if (lt1<lt2) {
            lttmp += llinc;
            lntmp += llinc * (ln2-ln1)/(lt2-lt1);
          } else {
            lttmp -= llinc;
            lntmp -= llinc * (ln2-ln1)/(lt2-lt1);
          }
        }
      }
      gxconv (lntmp,lttmp,&xx,&yy,2);
      *(newxy+j*2) = xx;
      *(newxy+j*2+1) = yy;
      j++;
      if (j>=ncnt) {
        printf ("Logic Error in gxmpoly\n");
        free (newxy);
        return (NULL);
      }
      lnsav = lntmp; ltsav = lttmp;
      llsum += llinc;
    }
  }
  *newcnt = j;
  return (newxy);
}
