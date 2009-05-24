/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* Authored by B. Doty */

/* Plot character strings using the Hershey fonts */

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
#include <math.h>
#include <gatypes.h>
#include "gx.h"

static char *fch[10];     /* Pointers to font data once it is read in */
static gaint *foff[10];   /* Pointers to character offsets */
static gaint *flen[10];   /* Pointers to character lengths */
static gaint dfont;       /* Default font */
void gree();

/* Initialize */

void gxchii (void) {
gaint i;
  for (i=0; i<10; i++) fch[i] = NULL;
  dfont = 0;
}

/* Change default font */

void gxchdf (gaint df) {
  if (df<0 || df>9) return;
  dfont = df;
}

/* Plot character string */

void gxchpl (char *chrs, int len, gadouble x, gadouble y, gadouble height, gadouble width, gadouble angle) {
gadouble xc,yc,xscl,yscl,xs,ys,w,d,ang,rx,ry,yoff;
gaint i,fn,ic,jc,cnt,ipen,supsub;
char *cdat;

  xscl = width/21.0;
  yscl = height/22.0;
  fn = dfont;
  angle = angle * 3.1416/180.0;
  supsub = 0;
  while (*chrs!='\0' && len>0) {
    while (*chrs=='`') {
      if (*(chrs+1)>='0' && *(chrs+1)<='9') {
        fn = (gaint)*(chrs+1) - 48;
        chrs+=2;
        len-=2;
      } else if (*(chrs+1)=='a') {
        supsub = 1;
        chrs+=2; len-=2;
      } else if (*(chrs+1)=='b') {
        supsub = 2;
        chrs+=2; len-=2;
      } else if (*(chrs+1)=='n') {
        supsub = 0;
        chrs+=2; len-=2;
      } else break;
    }
    if (*chrs!='\0' && len>0) {
      if (angle==0.0) {           /* Fast path for ang=0 */
        cdat = gxchgc ( (gaint)*(chrs), fn, &cnt);
        if (cdat==NULL) return;
        ic = (gaint)(*(cdat+3)) - 82;
        jc = (gaint)(*(cdat+4)) - 82;
        if (supsub) {
          xs = xscl*0.45; ys = yscl*0.45;
          if (supsub==1) yoff = height*0.35;
          else yoff = -1.0*height*0.42;
        } else {
          xs = xscl; ys = yscl; yoff = 0.0;
        }
        w = (gadouble)(jc-ic) * xs * 1.05;
        xc= x + w/2.0;
        yc = y + height*0.42 + yoff;
        cdat += 5;
        ipen = 3;
        for (i=1; i<cnt; i++) {
          ic = (gaint)*cdat;
          jc = (gaint)*(cdat+1);
          if (ic==32) ipen = 3;
          else {
            ic = ic - 82;
            jc = jc - 82;
            rx = xc + ((gadouble)ic)*xs;
            ry = yc - ((gadouble)jc)*ys;
            if (ipen==3) gxmove (rx,ry);
            else gxdraw (rx,ry);
            ipen = 2;
          }
          cdat += 2;
        }
        x = x + w;
        chrs++; len--;
      } else {
        cdat = gxchgc ( (gaint)*(chrs), fn, &cnt);
        if (cdat==NULL) return;
        ic = (gaint)(*(cdat+3)) - 82;
        jc = (gaint)(*(cdat+4)) - 82;
        if (supsub) {
          xs = xscl*0.45; ys = yscl*0.45;
          if (supsub==1) yoff = height*0.35;
          else yoff = -1.0*height*0.42;
        } else {
          xs = xscl; ys = yscl; yoff = 0.0;
        }
        w = (gadouble)(jc-ic) * xs * 1.05;
        d = hypot(w/2.0,height*0.42+yoff);
        ang = atan2(height*0.42+yoff,w/2.0)+angle;
        xc = x + d*cos(ang);
        yc = y + d*sin(ang);
        cdat += 5;
        ipen = 3;
        for (i=1; i<cnt; i++) {
          ic = (gaint)*cdat;
          jc = (gaint)*(cdat+1);
          if (ic==32) ipen = 3;
          else {
            ic = ic - 82;
            jc = jc - 82;
            rx = ((gadouble)ic)*xs;
            ry = -1.0*((gadouble)jc)*ys;
            if (rx==0.0 && ry==0.0) {
              d = 0.0; ang = 0.0;
            } else {
              d = hypot(rx,ry);
              ang = atan2(ry,rx)+angle;
            }
            rx = xc + d*cos(ang);
            ry = yc + d*sin(ang);
            if (ipen==3) gxmove (rx,ry);
            else gxdraw (rx,ry);
            ipen = 2;
          }
          cdat += 2;
        }
        x = x + w*cos(angle);
        y = y + w*sin(angle);
        chrs++; len--;
      }
    }
  }
}

/* Figure out the length of a character string, without
   plotting it */

gaint gxchln (char *chrs, gaint len, gadouble width, gadouble *w) {
int fn,ic,jc,supsub;
gadouble xscl,xs;
char *cdat;

  *w = 0.0;
  xscl = width/21.0;
  fn = dfont;
  supsub = 0;
  while (*chrs!='\0' && len>0) {
    while (*chrs=='`') {
      if (*(chrs+1)>='0' && *(chrs+1)<='9') {
        fn = (gaint)*(chrs+1) - 48;
        chrs+=2;
        len-=2;
      } else if (*(chrs+1)=='a') {
        supsub = 1;
        chrs+=2; len-=2;
      } else if (*(chrs+1)=='b') {
        supsub = 2;
        chrs+=2; len-=2;
      } else if (*(chrs+1)=='n') {
        supsub = 0;
        chrs+=2; len-=2;
      } else break;
    }
    cdat = gxchgc ((gaint)*(chrs), fn, &ic);
    if (cdat==NULL) return(1);
    ic = (gaint)(*(cdat+3)) - 82;
    jc = (gaint)(*(cdat+4)) - 82;
    if (supsub) xs = xscl*0.45;
    else xs = xscl;
    *w = *w + (gadouble)(jc-ic) * xs * 1.05;
    chrs++; len--;
  }
  return 0;
}

/* Get location and length of particular character info
   for particular font */

char * gxchgc (int ch, int fn, int *cnt) {
int *clen, *coff, rc;
char *fdat;

  if (fch[fn]==NULL) {
    rc = gxchrd (fn);
    if (rc) return(NULL);
  }
  clen = flen[fn];
  coff = foff[fn];
  fdat = fch[fn];
  if (ch<32 || ch>127) ch=32;
  ch = ch - 32;
  *cnt = *(clen+ch);
  return (fdat + *(coff+ch));
}

/* Read in a font file */

int gxchrd (int fn) {
FILE *ifile;
gaint i,j,rc,tlen,*coff,*clen,flag;
char buff[20],*fname,*fdat;

  sprintf (buff,"font%i.dat",fn);

  ifile = NULL;
  fname = gxgnam(buff);
  if (fname!=NULL) ifile = fopen(fname,"rb");
  if (ifile==NULL) {
    ifile = fopen(buff,"rb");
    if (ifile==NULL) {
      printf ("Error opening stroke character data set \n");
      if (fname!=NULL) {
        printf ("  Data set names = %s ; %s\n",fname,buff);
        gree (fname,"f295");
      }
      return(1);
    }
  }
  gree(fname,"f296");

  fseek(ifile,0L,2);
  tlen = ftell(ifile);
  fseek(ifile,0L,0);

  fdat = (char *)malloc(tlen+1);
  if (fdat==NULL) {
    printf ("Error reading font data:  Memory allocation error\n");
    return(1);
  }
  coff = (gaint *)malloc(sizeof(gaint)*95);
  if (coff==NULL) {
    printf ("Error reading font data:  Memory allocation error\n");
    free (fdat);
    return(1);
  }
  clen = (gaint *)malloc(sizeof(gaint)*95);
  if (clen==NULL) {
    printf ("Error reading font data:  Memory allocation error\n");
    free (fdat);
    return(1);
  }

  rc = fread(fdat,1,tlen,ifile);
  if (rc!=tlen) {
    printf ("Error reading font data: I/O Error\n");
    return(1);
  }
  *(fdat+tlen) = '\0';

  /* Determine the locations of the start of each character */

  i = 0;
  j = 1;
  *(coff) = 0;
  flag = 0;
  while (*(fdat+i)) {
    if (*(fdat+i)<' ') {
      flag = 1;
    } else {
      if (flag) {
        *(coff+j) = i;
        j++;
      }
      flag = 0;
    }
    i++;
  }

  /* Determine the count on each character */

  for (i=0; i<95; i++) {
    for (j=0; j<3; j++) buff[j] = *(fdat+*(coff+i)+j);
    buff[3] = '\0';
    sscanf (buff,"%i",&rc);
    *(clen+i) = rc;
  }

  flen[fn] = clen;
  foff[fn] = coff;
  fch[fn] = fdat;

  return(0);
}
