/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* Authored by B. Doty and Jennifer Adams */

#ifdef HAVE_CONFIG_H
#include "config.h"

/* If autoconfed, only include malloc.h when it's presen */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#else /* undef HAVE_CONFIG_H */

#include <malloc.h>

#endif /* HAVE_CONFIG_H */

#include "grads.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if USENETCDF == 1
#include <netcdf.h>
#endif
#if USEHDF ==1
#include <mfhdf.h>
#endif

/* global struct for warning level setting */
extern struct gamfcmn mfcmn;

gaint garead (off_t, gaint, gadouble *, char *);
off_t gafcorlf (gaint, gaint, gaint, gaint, gaint);

/* Global pointers for this file */
static struct gafile *pfi;
static struct gagrid *pgr;
static struct gavar *pvr;
static struct gaindx *pindx;
static gaint timerr;
static gaint msgflg=1;
static char pout[256]; 

/* For STNDALN, routines included are gaopfn, gaopnc, and gaophdf */
#ifndef STNDALN

/* GRIB I/O caching.  GRIB data is chached, as well as the bit
   maps, if present.  Sometimes the expanded bit map is cached. */
static char *cache;             /* I/O cache for GRIB */
static unsigned char *bcache;   /* Bit map cache */
static gaint cflag=0;           /* cache flag */
static gaint bcflag=0;          /* Bit cache flag */
static gaint bpsav = -999;      /* Bit cache pointer */
static gaint bssav = -999;      /* Bit cache size */
static gaint *bpcach;           /* expanded bit map cache */

/* Station data I/O caching.  We will cache fairly small I/O
   requests that fit within the specified size buffer.  If the buffer
   gets overfilled, we just forget the caching.  */
static gaint scflg = 0;         /* Anything cached? */
static gaint scuca = 0;         /* Can use cache for this request */
static gaint scerr = 0;         /* Buffer full? */
static gaint scok;              /* Ok to fill buffer */
static gaint scpnt;             /* Current cache offset */
static gaint scseq;             /* File sequence of last request */
static struct gastn scstn;      /* Previous request */
static char *scbuf=NULL;        /* Cache */


#if GRIB2
/* GRIB2 I/O cache */
static struct gag2indx *g2indx;
static struct g2anchor *anchor;
static gaint debug=0;
#define MAXG2CACHE 500100100
#endif

/* Size of cache */
#define SCNUM 50000


/* Routine resets flag to allow warning in regards to interpolation */
void gaiomg () {
  msgflg = 1;
}

/* Routine to obtain a grid.  The addresses of the gagrid
   structure is passed to this routine.  The storage for the
   grid is obtained and the grid is filled with data.                 */

gaint gaggrd (struct gagrid *pgrid) {
  gadouble *gr;
  char *gru;
  gaint x,i,id,jd,d[5],dx[5];
  gaint incr,rc,dflag;
  gaint size,ssz,usz;

  if (cflag) gree(cache,"f105");
  cache = NULL;
  cflag = 0;
  if (bcflag) {
    gree(bcache,"f106");
    gree(bpcach,"f107");
  }
  bcache = NULL;
  bpcach = NULL;
  bcflag = 0;
  bssav = -999;
  bpsav = -999;

  pgr = pgrid;
  pvr = pgr->pvar;
  pfi = pgr->pfile;
  timerr = 0;
  if (pfi->idxflg==1) pindx  = pfi->pindx;
#if GRIB2
  if (pfi->idxflg==2) g2indx = pfi->g2indx;
#endif
  if (pfi->ppflag && msgflg) {
    gaprnt (3,"Notice:  Automatic Grid Interpolation Taking Place\n");
    msgflg = 0;
  }

  if (pfi->type==4) {
    rc = gagdef();
    return (rc);
  }

  /* Check dimensions we were given */
  if (pgr->idim < -1 || pgr->idim > 4 ||
      pgr->jdim < -1 || pgr->jdim > 4 ||
      (pgr->idim == -1 && pgr->jdim!=-1)) {
    sprintf (pout,"Internal logic check 16:  %i %i  \n", pgr->idim, pgr->jdim);
    gaprnt (0,pout);
    return (16);
  }

  /* Calc sizes and get storage for the grid */
  id = pgr->idim;
  jd = pgr->jdim;
  if (id > -1)  pgr->isiz = pgr->dimmax[id] - pgr->dimmin[id] + 1;
  else pgr->isiz = 1;
  if (jd > -1)  pgr->jsiz = pgr->dimmax[jd] - pgr->dimmin[jd] + 1;
  else pgr->jsiz = 1;
  size = pgr->isiz*pgr->jsiz;
  if (size>1) {
    /* this is for the grid */
    ssz  = size * sizeof(gadouble);
    gr = (gadouble *)galloc(ssz,"gr");
    if (gr==NULL) {
      gaprnt (0,"Memory Allocation Error:  grid storage \n");
      return (1);
    }
    pgr->grid = gr;
    /* this is for the undef mask */
    usz = size * sizeof(char);
    gru = (char *)galloc(usz,"gru");
    if (gru==NULL) {
      gaprnt (0,"Memory Allocation Error:  grid storage \n");
      return (1);
    }
    pgr->umask = gru;
  } 
  else {
    pgr->grid = &(pgr->rmin);
    gr = pgr->grid;
    pgr->umask = &(pgr->umin);  
    gru = pgr->umask;
  }

  /* Handle predefined variable */
  if (pvr->levels<-900) {
    rc = gagpre();
    return (rc);
  }

  /* set minimum and maximum grid indices */
  for (i=0; i<5; i++) {
    d[i]  = pgr->dimmin[i]; 
    dx[i] = pfi->dnum[i];
  }
  dx[2] = pvr->levels;
  if (dx[2]==0) {        
    if (id==2 || jd==2) goto nozdat;
    dx[2] = 1;
    d[2] = 1;
  }

  incr = pgr->isiz;

  /* If X does not vary, make sure the X coordinate is normalized.    */
  if (id!=0 && pfi->wrap) {
    x=pgr->dimmin[0];
    while (x<1) x=x+dx[0];
    while (x>dx[0]) x=x-dx[0];
    pgr->dimmin[0]=x;
    pgr->dimmax[0]=x;
    d[0] = x;
  }

  /* If any of the non-varying dimensions are out of bounds of the
     file dimension limits, then we have a grid of missing data.
     Check for this.                                                  */
  for (i=0; i<5; i++) {
    if (id!=i && jd!=i &&
       (d[i]<1 || d[i]>dx[i]) ) goto nodat;
  }

  /* Break out point for reading 2D netcdf grids (OPeNDAP or special case NetCDF) */
  /* JMA still need to optimize handling of OPeNDAP pre-projected grids */
  if ((!strncmp(pgrid->pfile->name,"http://",7)) ||                   /* OPeNDAP */
      ((pfi->ncflg==1) && (pfi->ppflag==0) && (pfi->tmplat==0))) {    /* non-templated, non-pdef, NetCDF */
    rc = gancgrid(gr,gru,id,jd);
    return (rc);
  }

  /* Handle case where X varies. */
  dflag = 0;
  if ( id == 0 ) {
    if (jd<0) jd = 1;      
    for (d[jd]=pgr->dimmin[jd]; d[jd]<=pgr->dimmax[jd]; d[jd]++) {
      if (d[jd]<1 || d[jd]>dx[jd]) {
        for (i=0; i<incr; i++) {
	  *(gr+i) = pgr->undef;
	  *(gru+i) = 0;
	}
      } 
      else {
        rc = gagrow(gr, gru, d);
        if (rc > 0 ) return (1);
        if (rc==0) dflag=1;
      }
      gr += incr;
      gru += incr;
    }
    if (!dflag) goto nodatmsg;
    return (0);
  }

  /* Handle cases where X does not vary. Read each point in the grid seperately. */
  if (jd<0) {
    if (id<0) { id=0; jd=1; }
    else jd=0;
  }
  for (d[jd]=pgr->dimmin[jd]; d[jd]<=pgr->dimmax[jd]; d[jd]++) {
    if (d[jd]<1 || d[jd]>dx[jd]) {
      for (i=0; i<incr; i++,gr++,gru++) {
	*gr = pgr->undef;
	*gru = 0;
      }
    } else {
      for (d[id]=pgr->dimmin[id]; d[id]<=pgr->dimmax[id]; d[id]++) {
        if (d[id]<1 || d[id]>dx[id]) {
	  *gr = pgr->undef;
	  *gru = 0;
	}
        else {
          rc = garrow (d[0], d[1], d[2], d[3], d[4], 1, gr, gru);  
          if (rc != 0 ) return (1);
          dflag=1;
        }
        gr++; gru++;
      }
    }
  }
  if (!dflag) goto nodatmsg;
  return (0);

nozdat:
  if(mfcmn.warnflg>0) {
    gaprnt (1,"Data Request Warning:  Varying Z dimension environment...\n");
    gaprnt (1,"  but the requested variable has no Z dimension\n");
    gaprnt (2,"  Entire grid contents are set to missing data \n");
  }
  for (i=0; i<size; i++,gr++,gru++) {
    *gr = pgr->undef;  
    *gru = 0;
  }
  return (-1);

nodat:
  for (i=0; i<size; i++,gr++,gru++) {
    *gr = pgr->undef;  
    *gru = 0;
  }

nodatmsg:
  if(mfcmn.warnflg>0) {
    gaprnt (1,"Data Request Warning:  Request beyond file limits\n");
    gaprnt (2,"  Entire grid contents are set to missing data \n");
    sprintf (pout,"  Dimension ranges are:  X = %i %i  Y = %i %i ",
	     d[0],dx[0],d[1],dx[1]);
    gaprnt (2,pout);
    sprintf (pout," Z = %i %i  T = %i %i  E = %i %i\n",
	     d[2],dx[2],d[3],dx[3],d[4],dx[4]);
    gaprnt (2,pout);
  }
  return (-1);

}


/* gagrow gets a row of data from the file.  The row of data can
   be 'wrapped' if the x direction of the grid spans the globe.       */

gaint gagrow (gadouble *gr, char *gru, gaint *d) {
gaint rc,i,x,j;
gaint y,z,t,e;

  y = *(d+1);
  z = *(d+2);
  t = *(d+3);
  e = *(d+4);

  /* If the needed data is within the bounds of the file dimensions
     then read the data directly.                                     */
  if (pgr->dimmin[0] >= 1 && pgr->dimmax[0] <= pfi->dnum[0]) {
    rc = garrow (pgr->dimmin[0], y, z, t, e, (pgr->dimmax[0]-pgr->dimmin[0]+1), gr, gru);
    if (rc != 0 ) return (1);
    return (0);
  }

  /* If the file does not wrap, then read the data directly, if possible.  
     If the requested data lies outside the file's bounds,
     fill in with missing data where appropriate.                   */
  if (!pfi->wrap) {
    if ( pgr->dimmin[0]>=1 && pgr->dimmax[0]<=pfi->dnum[0] ) {
      rc = garrow (pgr->dimmin[0], y, z, t, e, (pgr->dimmax[0]-pgr->dimmin[0]+1), gr, gru);
      if (rc != 0 ) return (1);
      return (0);
    }
    for (i=0; i<pgr->isiz; i++) {
      *(gr+i) = pgr->undef;
      *(gru+i) = 0;
    }
    if (pgr->dimmin[0]<1 && pgr->dimmax[0]<1 ) return (-1);
    if (pgr->dimmin[0]>pfi->dnum[0] &&
        pgr->dimmax[0]>pfi->dnum[0] ) return (-1);
    i = 1 - pgr->dimmin[0];
    if (i>0) { gr+=i; gru+=i; }
    i = 1;
    if (pgr->dimmin[0]>1) i = pgr->dimmin[0];
    j = pgr->dimmax[0];
    if (j > pfi->dnum[0]) j = pfi->dnum[0];
    j = 1 + (j - i);
    rc = garrow (i, y, z, t, e, j, gr, gru);
    if (rc != 0 ) return (1);
    return (0);
  }

  /* When the file wraps, we read the entire row into the row buffer, and
     copy the values as needed into locations in the requested row.    */
  rc = garrow (1, y, z, t, e, pfi->dnum[0], pfi->rbuf, pfi->ubuf);
  if (rc != 0 ) return (1);
  for (x=pgr->dimmin[0];x<=pgr->dimmax[0];x++) {
    i=x;
    while (i<1) i = i + pfi->dnum[0];
    while (i>pfi->dnum[0]) i = i-(pfi->dnum[0]);    /* Best way??? */
    *gr = *((pfi->rbuf)+i-1);
    *gru = *((pfi->ubuf)+i-1);
    gr++; gru++;
  }
  return (0);
}


/*  Basic read of a row of data elements -- a row is always
    in the X direction, which for grads binary is the fastest
    varying dimension */

gaint garrow (gaint x, gaint y, gaint z, gaint t, gaint e, gaint len, gadouble *gr, char *gru) {
  gaint rc,i=0,tt,ee,oflg;
  off_t fposlf;

  tt = t;
  if (pfi->tmplat) {
    tt = gaopfn(t,e,&ee,&oflg,pfi);
    if (tt==-99999) return(1);
    if (tt==-88888) {
      for (i=0; i<len; i++) {
	*(gr+i) = pfi->undef;
	*(gru+i) = 0;
      }
      return (0);
    }
    if (oflg) bpsav = -999;  /* Force new bit map cache if new file opened */
  }
  else {
    ee = e;  /* set relative ensemble number to e for non-templated data sets */
  }
  
  /* Preprojected (pdef) */
  if (pfi->ppflag) {                             
    if (pfi->idxflg)  
      rc = gaprow (x, y, z, t,  e, tt, len, gr, gru);  /* Grib uses e to read index file */
    else 
      rc = gaprow (x, y, z, t, ee, tt, len, gr, gru);  /* All other data types use ee */
    return (rc);
  }
  
#if USENETCDF == 1
  /* netcdf */
  if (pfi->ncflg==1) {              
    rc = gancsetup();
    if (rc) return(rc);
    rc = gancrow (x, y, z, tt, ee, len, gr, gru);
    return(rc);
  }
#endif
#if USEHDF == 1
  /* HDF-SDS */
  if (pfi->ncflg==2) {              
    rc = gahrow(x, y, z, tt, ee, len, gr, gru);
    return(rc);
  }
#endif
  
  /* Indexed (grib) */
  if (pfi->idxflg) {               
    rc = gairow (x, y, z, t, e, i, len, gr, gru);  
    return (rc);
  }

  /* if none of the above... binary */
  fposlf = gafcorlf (x, y, z, tt, ee);         
  rc = garead (fposlf, len, gr, gru);
  return (rc);
}
 
 
off_t gafcorlf (gaint x, gaint y, gaint z, gaint t, gaint e) {
off_t pos;
off_t ltmpz,ltmpy,ltmpt,ltmpe;
off_t yy,zz;
off_t levs;
off_t xl, yl, zl, tl, el;

  xl = x;
  yl = y;
  zl = z;
  tl = t;
  el = e;

  levs=(off_t)pvr->levels;
  if(levs == 0) levs=1;
  if (pfi->tlpflg) {
    tl = tl + (off_t)pfi->tlpst;
    if (tl > (off_t)pfi->dnum[3]) tl = tl - (off_t)pfi->dnum[3];
  }

  if (pfi->yrflg) {
    yy = (off_t)pfi->dnum[1] - yl;
  }
  else {
    yy = yl - 1;
  }

  if (pfi->zrflg) {
    if (levs==0) {
      zz = 0;
    }
    else {
      zz = levs - zl;
    }
  } 
  else {
    zz = zl - 1;
  }
  if (pvr->var_t) {
    ltmpe = (el-1)*(off_t)((pfi->dnum[3])*(pfi->vnum)*levs*(pfi->gsiz)); 
    ltmpt = (tl-1)*(off_t)(pfi->gsiz)*levs;
    ltmpz = zz*(off_t)(pfi->gsiz);
    ltmpy = yy*(off_t)(pfi->dnum[0]);
  } else {
    ltmpe=(el-1)*(pfi->dnum[3]*(off_t)pfi->tsiz);
    ltmpt=(tl-1)*((off_t)pfi->tsiz);
    ltmpz=zz*(off_t)(pfi->gsiz); 
    ltmpy=yy*((off_t)pfi->dnum[0]);
  }
  pos = 
    ltmpe + 
      ltmpt +
        (off_t)pvr->offset +
          ltmpz +
            ltmpy +
              (xl-1); 

  if (pfi->xyhdr) pos = pos + (off_t)pfi->xyhdr;
  return (pos);
}

gaint garead (off_t fpos, gaint len, gadouble *gr, char *gru) {
gafloat *fgr;
off_t ffpos;
gaint rc,i;
gaint cnt,ival,*ig;
char *ch1,*ch2,*ch3,*ch4,cc1,cc2;
unsigned char *uch1,*uch2,ucc1,ucc2;
unsigned char *igr;
unsigned char *cgr;

  if (pvr->dfrm == 1) {
    ffpos = fpos*(off_t)sizeof(char) + (off_t)pfi->fhdr;
  } 
  else if (pvr->dfrm == 2 || pvr->dfrm == -2 ) {
    ffpos = fpos*2ll + (off_t)pfi->fhdr;
  } 
  else if (pvr->dfrm == 4) {
    ffpos = fpos*(off_t)sizeof(gaint) + (off_t)pfi->fhdr;
  } 
  else {
    ffpos = fpos*(off_t)sizeof(gafloat) + (off_t)pfi->fhdr;
  }
  rc = fseeko(pfi->infile, ffpos, 0);
  if (rc!=0) {
    gaprnt (0,"Low Level I/O Error:  Seek error on data file \n");
    sprintf (pout,"  Data file name = %s \n",pfi->name);
    gaprnt (0,pout);
    sprintf (pout,"%d rc=%d fpos=%ld pfi->fhdr=%ld\n",__LINE__,rc,fpos,pfi->fhdr);
    gaprnt (0,pout);
    sprintf (pout,"  Error occurred when seeking to byte %ld \n",ffpos);
    gaprnt (0,pout);
    return (1);
  }

  if (pvr->dfrm == 1) {
    igr = (unsigned char *)galloc(len*sizeof(char),"igr");
    if (igr==NULL) {
      gaprnt (0,"Memory Allocation Error:  char grid storage \n");
      return (1);
    }
    rc = fread (igr, 1, len, pfi->infile);               /* read 1-byte data */
    if (rc<len) {
      gree(igr,"f108a");
      goto readerr;
    }
    for(i=0;i<len;i++) *(gr+i) = (gadouble)(*(igr+i));   /* convert to gadouble */
    gree(igr,"f108");
  } 
  else if (pvr->dfrm == 2 || pvr->dfrm == -2 ) {
    cgr = (unsigned char *)galloc(len*2,"cgr");
    if (cgr==NULL) {
      gaprnt (0,"Memory Allocation Error:  integer*2 storage \n");
      return (1);
    }
    rc = fread (cgr, 2, len, pfi->infile);               /* read 2-byte data */
    if (rc<len) {
      gree(cgr,"f109a");
      goto readerr;
    }
    cnt=0;
    if (pfi->bswap) {                                    /* byte swap if necessary */
      uch1 = cgr;
      uch2 = uch1+1;
      for (i=0; i<len; i++) {
	ucc1 = *uch1;
	ucc2 = *uch2;
	*uch1 = ucc2;
	*uch2 = ucc1;
	uch1+=2; uch2+=2;
      }
    }
    if (pvr->dfrm == -2) {
      for(i=0;i<len;i++) {
	ival=(gaint)(*(cgr+cnt)*256) + (gaint)((*(cgr+cnt+1))) - 65536*(*(cgr+cnt)>>7);
	*(gr+i) = (gadouble)ival;                       /* convert to gadouble */
	cnt+=2;
      }
    } else {
      for(i=0;i<len;i++) {
	ival=(gaint)(*(cgr+cnt)*256) + (gaint)((*(cgr+cnt+1)));
	*(gr+i) = (gadouble)ival;                       /* convert to gadouble */
	cnt+=2;
      }
    }
    gree(cgr,"f109");
  } 
  else if (pvr->dfrm == 4) {
    ig = (gaint *)galloc(len*sizeof(gaint),"ig");
    if (ig==NULL) {
      gaprnt (0,"Memory Allocation Error:  integer*4 storage \n");
      return (1);
    }
    rc = fread (ig, sizeof(gaint), len, pfi->infile);      /* read 4-byte integers */
    if (rc<len) {
      gree(ig,"f110a");
      goto readerr;
    }
    if (pfi->bswap) {                                    /* byte swap if necessary */
      ch1 = (char *)ig;
      ch2 = ch1+1;
      ch3 = ch2+1;
      ch4 = ch3+1;
      for(i=0;i<len;i++) {
	cc1 = *ch1;
	cc2 = *ch2;
	*ch1 = *ch4;
	*ch2 = *ch3;
	*ch3 = cc2;
	*ch4 = cc1;
	ch1+=4; ch2+=4; ch3+=4; ch4+=4;
      }
    }
    for(i=0;i<len;i++) *(gr+i) = (gadouble)(*(ig+i));     /* convert to gadouble */
    gree(ig,"f110");

  } 
  else {
    /* standard direct access read */
    /* here we use 4 instead of sizeof(gafloat) to read float data */
    fgr = (gafloat *)galloc(len*4,"fgr");
    rc = fread (fgr, 4, len, pfi->infile); 
    if (rc<len) {
      gree(fgr,"f113a");
      goto readerr;
    }
    if (pfi->bswap) {                                     /* byte swap if necessary */
      ch1 = (char *)fgr;
      ch2 = ch1+1;
      ch3 = ch2+1;
      ch4 = ch3+1;
      for (i=0; i<len; i++) {
	cc1 = *ch1;
	cc2 = *ch2;
	*ch1 = *ch4;
	*ch2 = *ch3;
	*ch3 = cc2;
	*ch4 = cc1;
	ch1+=4; ch2+=4; ch3+=4; ch4+=4;
      }
    }
    for(i=0;i<len;i++) *(gr+i) = (gadouble)(*(fgr+i));    /* convert to doubles */
    gree(fgr,"f113");
  }

  /* Test if grid value is within EPSILON of the missing data value.
     If yes, set undef mask to 0. The undef mask is 1 for valid grid values */
  for (i=0;i<len;i++) {
    if (*gr >= pfi->ulow && *gr <= pfi->uhi) {
      *gru = 0;
    }
    else {
      *gru = 1;
    }
    gr++; gru++;
  }
  
  return (0);

readerr:
  gaprnt (0,"Low Level I/O Error:  Read error on data file \n");
  sprintf (pout,"  Data file name = %s \n",pfi->name);
  gaprnt (0,pout);
  sprintf (pout,"  Error reading %i bytes at location %ld \n",len, ffpos);
  gaprnt (0,pout);
  return (1);

}

/* Handle a station data request */

gaint gagstn (struct gastn *stn) {
  struct rpthdr ghdr, *hdr;
  gaint i,j,rc,tim,fnum,rtot,rdw,nsiz;
  gaint selflg,oflg,dummy;
  off_t fpos=0;
  gaint sizhdrf,sizhdrd,sizf,sizd,idum;
  gadouble lnmin,lnmax,ltmin,ltmax,hlon;
  char ch1[16],ch2[16],*ch;

  stn->rpt = NULL;
  for (i=0; i<BLKNUM; i++) {
    stn->blks[i] = NULL;
  }

  if (stn->pfi->bufrflg) {         /* bufrstn */
    rc = getbufr(stn);             /* bufrstn */
    return (rc);                   /* bufrstn */
  }

#if USEGADAP
  if (stn->pfi->dhandle > -999) {  /* dodstn */
    rc = dapget(stn);              /* dodstn */
    return (rc);                   /* dodstn */
  }
#endif

  /* Determine cache situation */
  if (scbuf==NULL && !scerr) {
    scbuf = (char *)galloc(SCNUM,"scbuf");
    if (scbuf==NULL) {
      gaprnt (0,"Memory allocation error:  Stn data cache buffer\n");
      gaprnt (0,"  Station data cache disabled\n");
      scerr = 1;
    }
    scflg = 0;
  }

  scuca = 0;
  scpnt = 0;
  if (!scerr) scok = 1;
  if (scflg && stn->pfi->fseq != -999 && scseq == stn->pfi->fseq &&
      scstn.pfi==stn->pfi && scstn.idim==stn->idim &&
      scstn.jdim==stn->jdim && scstn.tmin==stn->tmin &&
      scstn.tmax==stn->tmax && scstn.rflag==stn->rflag &&
      scstn.sflag==stn->sflag) {
    rc = 1;
    if (stn->rflag && scstn.radius!=stn->radius) rc = 0;
    if (stn->sflag) {
      for (i=0; i<8; i++) if (scstn.stid[i]!=stn->stid[i]) rc=0;
    } else {
      if (scstn.dmin[0]!=stn->dmin[0]) rc = 0;
      if (scstn.dmin[1]!=stn->dmin[1]) rc = 0;
      if (scstn.dmax[0]!=stn->dmax[0]) rc = 0;
      if (scstn.dmax[1]!=stn->dmax[1]) rc = 0;
    }
    if (rc) {
      scuca = 1;
      scok = 0;
    }
  }

  pvr = stn->pvar;
  pfi = stn->pfi;
  hdr = &ghdr;

  lnmin = stn->dmin[0]; lnmax = stn->dmax[0];
  ltmin = stn->dmin[1]; ltmax = stn->dmax[1];
  if (stn->rflag) {
    lnmin = lnmin - stn->radius;
    lnmax = lnmax + stn->radius;
    ltmin = ltmin - stn->radius;
    ltmax = ltmax + stn->radius;
  }
  stn->rnum = 0;

  /* set size of the file and data hdr */
  sizhdrf = sizeof(struct rpthdr);
  sizhdrd = sizeof(struct rpthdr);

  /* Loop through times looking for appropriate stations */
  for (tim=stn->tmin; tim<=stn->tmax; tim++) {
    if (tim<1) continue;
    if (tim > pfi->dnum[3]) break;

    if (!scuca && pfi->tmplat) {
      rc = gaopfn(tim,1,&dummy,&oflg,pfi);
      if (rc==-99999) goto err;
      if (rc==-88888) {
        if (scok) {
          hdr->nlev = 0;
          gacstn((char *)hdr, NULL, 0, sizhdrd);
        }
        continue;
      }
    }

    /* Loop through stations for this time looking for valid reports */
    if (!scuca) {
      fpos = *(pfi->tstrt+tim-1);
      rc = gasstn(fpos);
      if (rc) goto err;
    }

    while (1) {
      /* get the header */
      if (scuca) {                             /* from the cache */
	gagcst (sizhdrd, (char *)hdr);
      } else {                                 /* from the file */
        if (pfi->seqflg) {
          rc = garstn(4,(char *)(&rdw),fpos);  
          if (rc) goto err;
          if (pfi->bswap) gabswp((gafloat *)(&rdw),1);
        }
        rc = garstn (sizhdrf, (char *)hdr, fpos);
        if (rc) goto err;
        if (pfi->bswap) gahswp(hdr);
      }

      if (hdr->nlev==0) break;   /* END OF DATA CHECK */

      /* Left justify the station id if there are leading blanks */

      j = 0;
      while (j<7 && hdr->id[0]==' ') {
        for (i=0; i<7; i++) hdr->id[i] = hdr->id[i+1];
        hdr->id[7] = ' ';
        j++;
      }

      /* Determine if we want to read the data portion of this report */
      selflg = 1;
      if (stn->sflag) {
        getwrd (ch1,hdr->id,8);
        lowcas(ch1);
        getwrd(ch2,stn->stid,8);
        if (!cmpwrd(ch1,ch2)) selflg = 0;
      } else {
        hlon = (gadouble)hdr->lon;
        if (hlon<lnmin) hlon+=360.0;
        if (hlon>lnmax) hlon-=360.0;
        if (hlon<lnmin || hlon>lnmax ||
            hdr->lat<ltmin || hdr->lat>ltmax ) selflg=0;
        if (selflg && stn->rflag &&
            hypot(hlon-stn->dmin[0], hdr->lat-stn->dmin[1])>stn->radius) {
          selflg = 0;
        }
      }

      /* Determine size of the data portion of this report */
      if (hdr->flag) {
	fnum = (hdr->nlev-1) * (pfi->lvnum+1) + pfi->ivnum;
      } else {
	fnum =  hdr->nlev * (pfi->lvnum+1);
      }

      /* calc size of floating point data section in the FILE not the machine */
      sizd=fnum*sizeof(gafloat);
      sizf=fnum*sizeof(gafloat);

      /* Read the data portion of this report, byteswap it if needed,
	 and set exact missing data values if specified.*/
      if (selflg) {
        if (scuca) {                                  /* from the cache */
	  gagcst (sizd, (char *)pfi->sbuf);
        } else {                                       /* from the file */
          if (pfi->seqflg) {
            ch = (char *)(pfi->sbuf);
            nsiz = rdw - sizhdrf;
            if (nsiz>0) {
              rc = garstn(nsiz,ch,fpos);
              if (rc) goto err;
              ch += nsiz;
            }
            rtot = rdw;
            nsiz = sizf + sizhdrf;
            while (rtot<=nsiz) {
              fpos = fpos + rdw + 8;
              rc = gasstn(fpos);
              if (rc) goto err;
              if (rtot==nsiz) break;
              rc = garstn(4,(char *)(&rdw),fpos); 
              if (rc) goto err;
              if (pfi->bswap) gabswp((gafloat *)(&rdw),1);
              rtot +=rdw;
              if (rtot>nsiz) break;
              rc = garstn(rdw,ch,fpos);

              if (rc) goto err;
	      idum=rdw*2;
              ch += idum;
            }
            if (rtot>nsiz) {
              gaprnt (0,"Low Level I/O Error:  Sequential read error\n");
              gaprnt (0,"  Record size exceeds report size\n");
              sprintf (pout,"  Data file name = %s \n",pfi->name);
              gaprnt (0,pout);
              goto err;
            }

	  /* normal read -- NON sequential */
          } else {
            rc = garstn (sizf, (char *)pfi->sbuf, fpos+sizhdrf);
            fpos = fpos + sizf + sizhdrf;
            if (rc) goto err;
          }
          if (pfi->bswap) gabswp(pfi->sbuf,fnum);
        }
	/* set undef mask for data read from file or the cache */
	for (i=0; i<fnum; i++) {
	  if ((*(pfi->sbuf+i) >= pfi->ulow) && (*(pfi->sbuf+i) <= pfi->uhi)) {
	    *(pfi->sbuf+i) = pfi->undef;
	    *(pfi->ubuf+i) = 0;
	  }
	  else {
	    *(pfi->ubuf+i) = 1;
	  }
	}

        /* Check the data portion for any matches.  */
        rc = gaglvs (tim,hdr,stn);
        if (rc) goto err;

        /* Cache this report if appropriate */
        if (scok) gacstn((char *)hdr,(char *)pfi->sbuf,sizd,sizhdrd);

      /* Skip the data portion of this report.*/
      } else {
        if (scuca) {
          gaprnt (0,"Logic Error 8 in gaio\n");
          goto err;
        }
        if (pfi->seqflg) {
          rtot = rdw;
          sizf += sizhdrf;
          while (rtot<=sizf) {
            fpos = fpos + rdw + 8;
            rc = gasstn(fpos);
            if (rc) goto err;
            if (rtot==sizf) break;
            rc = garstn(4,(char *)(&rdw),fpos);  
            if (rc) goto err;
            if (pfi->bswap) gabswp((float *)(&rdw),1);
            rtot +=rdw;
          }
          if (rtot>sizf) {
            gaprnt (0,"Low Level I/O Error:  Sequential read error\n");
            gaprnt (0,"  Record size exceeds report size\n");
            sprintf (pout,"  Data file name = %s \n",pfi->name);
            gaprnt (0,pout);
            goto err;
          }
        } else {
          fpos = fpos + sizf + sizhdrf;
          rc = gasstn(fpos);
          if (rc) goto err;
        }
      }  /* END OF if (scuca) -- use the cache or not */
    }    /* END OF  while (1) */

    if (scok) {
      hdr->nlev = 0;
      gacstn((char *)hdr, NULL, 0,sizhdrd);
    }
  }
  if (scok) {
    scflg = 1;
    scstn = *stn;
    scseq = stn->pfi->fseq;
  } else scflg = 0;
  if (scuca) scflg = 1;
  return (0);

err:
  for (i=0; i<BLKNUM; i++) {
    if (stn->blks[i] != NULL) free (stn->blks[i]);
  }
  return (1);
}

/* Select appropriate variable and levels from report, and chain
   them off the stn block.  */

gaint gaglvs (gaint tim, struct rpthdr *hdr, struct gastn *stn) {
struct garpt *rpt;
gafloat *vals,*pval;
gaint i,k,voff,mlev;
char *uvals,*upval;

  vals  = pfi->sbuf;
  uvals = pfi->ubuf;
  voff = pvr->offset;
  if (pvr->levels==0) {
    if (hdr->flag) {
      pval  =  vals+voff;
      upval = uvals+voff;
      rpt = gaarpt (stn);
      if (rpt==NULL) return(1);
      rpt->lat = (gadouble)hdr->lat;
      rpt->lon = (gadouble)hdr->lon;
      rpt->lev = -9.99e33;
      rpt->tim = (gadouble)(tim + hdr->t);
      rpt->val = (gadouble)(*pval);
      rpt->umask = *upval;
      for (k=0; k<8; k++) *(rpt->stid+k) = *(hdr->id+k);
      stn->rnum++;
    }
  } else {
    if (hdr->flag) {
      vals  =  vals + pfi->ivnum;
      uvals = uvals + pfi->ivnum;
    }
    mlev = hdr->nlev;
    if (hdr->flag) mlev--;
    for (i=0; i<mlev; i++) {
      pval = vals+(i*(pfi->lvnum+1));
      upval = uvals+(i*(pfi->lvnum+1));
      if (dequal(stn->dmax[2],stn->dmin[2],1e-08)==0) {
	if (fabs(*pval-stn->dmin[2])>0.01) continue;  /* JMA check with bdoty  */
      } else {
        if (*pval<stn->dmax[2] || *pval>stn->dmin[2]) continue;
      }
      rpt = gaarpt (stn);
      if (rpt==NULL) return(1);
      rpt->lat = (gadouble)hdr->lat;
      rpt->lon = (gadouble)hdr->lon;
      rpt->lev = (gadouble)(*pval);
      rpt->tim = (gadouble)(tim + hdr->t);
      rpt->val = (gadouble)(*(pval+voff+1)); 
      rpt->umask = *upval;
      for (k=0; k<8; k++) *(rpt->stid+k) = *(hdr->id+k);
      stn->rnum++;
    }
  }
  return (0);
}

/* Allocate a rpt structure, return pointer to allocated buffer.
   On the first request, stn->rpt should be set to NULL. */

struct garpt *gaarpt (struct gastn *stn) {
struct garpt *rpt;
gaint i;

  /* First time through, define the static variables. */

  if (stn->rpt == NULL) {
    stn->prev = &(stn->rpt);
    for (i=0; i<BLKNUM; i++) {
      stn->blks[i] = NULL;
    }
    stn->rptcnt = RPTNUM;    /* Force new block allocation */
    stn->blkcnt = -1;
  }

  stn->rptcnt++;
  rpt = stn->crpt;

  if (stn->rptcnt>=RPTNUM) {
    stn->blkcnt++;
    if (stn->blkcnt==BLKNUM) {
      printf ("Out of memory blocks to allocate \n");
      return(NULL);
    }
    rpt = (struct garpt *)galloc(sizeof(struct garpt)*(RPTNUM+2),"rpt");
    if (rpt==NULL) {
      printf ("Couldn't allocate memory for stn block \n");
      return(NULL);
    }
    stn->blks[stn->blkcnt] = rpt;
    stn->rptcnt = 0;
  } else rpt++;

  *(stn->prev) = rpt;
  stn->prev = &(rpt->rpt);
  rpt->rpt = NULL;
  stn->crpt = rpt;
  return(rpt);
}

void gacstn (char *hdr, char *rdat, gaint siz, gaint sizhdr) {
gaint i;
  if (scpnt+sizhdr*2+siz+10 > SCNUM) {
    scok = 0;
  } else {
    for (i=0; i<sizhdr; i++) *(scbuf+scpnt+i) = *(hdr+i);
    scpnt += sizhdr;
    if (siz>0) {
      for (i=0; i<siz; i++) *(scbuf+scpnt+i) = *(rdat+i);
      scpnt += siz;
    }
  }
}

/* Return info from the station data cache */

void gagcst (gaint siz, char *ch) {
gaint i;
  for (i=0; i<siz; i++) *(ch+i) = *(scbuf+scpnt+i);
  scpnt += siz;
}

/* Seek to specified location in a station data file */

gaint gasstn (off_t fpos) {
gaint rc;

  rc = fseeko(pfi->infile, fpos, 0);
  if (rc!=0) {
    gaprnt (0,"Low Level I/O Error:  Seek error on data file \n");
    sprintf (pout,"  Data file name = %s \n",pfi->name);
    gaprnt (0,pout);
    sprintf (pout,"%d  rc=%d pos=%ld pfi->fhdr =%ld \n",__LINE__,rc,fpos,pfi->fhdr);
    gaprnt (0,pout);
    sprintf (pout,"  Error occurred when seeking to byte %ld \n",fpos);
    gaprnt (0,pout);
    return (1);
  }
  return (0);
}

/* Read specified amount of data from a station data file */

gaint garstn (gaint siz, char *val, off_t fpos) {

gaint rc;

  rc = fread (val, siz, 1, pfi->infile);
  if (rc<1) {
    gaprnt (0,"Low Level I/O Error:  Read error on data file \n");
    sprintf (pout,"  Data file name = %s \n",pfi->name);
    gaprnt (0,pout);
    sprintf (pout,"  Error reading %i bytes at location %ld \n", siz, fpos);
    gaprnt (0,pout);
    return (1);
  }
  return (0);
}

/*  Obtain user requested grid from defined variable */

gaint gagdef (void) {
gaint id, jd, i, flag;
gaint ys,zs,ts,es,siz,pos;
gaint d[5],d1min=0,d1max=0,d2min=0,d2max=0,xt,yt;
gadouble *v;
char *vmask;

  /* If a dimension is a fixed dimension in the defined
     variable, it must be a fixed dimension in the output
     grid.  */

  id = pgr->idim;
  jd = pgr->jdim;
  if (jd>-1) {
    if (pfi->dnum[jd]==1) {
      jd = -1;
      pgr->jdim = -1;
      pgr->jsiz = -1;
    }
  }
  if (id>-1) {
    if (pfi->dnum[id]==1) {
      id = jd;
      pgr->idim = pgr->jdim;
      pgr->isiz = pgr->jsiz;
      pgr->igrab = pgr->jgrab;
      pgr->iabgr = pgr->jabgr;
      pgr->ivals = pgr->jvals;
      pgr->iavals = pgr->javals;
      pgr->ilinr = pgr->jlinr;
      jd = -1;
      pgr->jdim = -1;
      pgr->jsiz = 1;
    }
  }

  /* Set up constants for array subscripting */

  ys = pfi->dnum[0];
  zs = ys * pfi->dnum[1];
  ts = zs * pfi->dnum[2];
  es = ts * pfi->dnum[3];

  /* Set up dimension ranges */

  for (i=0; i<5; i++) d[i] = pgr->dimmin[i] - pfi->dimoff[i] - 1;
  for (i=0; i<5; i++) if (pfi->dnum[i]==1) d[i] = 0;
  if (id>-1) {
    d1min = d[id];
    d1max = pgr->dimmax[id] - pfi->dimoff[id] - 1;
  }
  if (jd>-1) {
    d2min = d[jd];
    d2max = pgr->dimmax[jd] - pfi->dimoff[jd] - 1;
  }

  /* Get storage for output grid */

  pgr->isiz = 1;
  pgr->jsiz = 1;
  if (id>-1) pgr->isiz = 1 + d1max - d1min;
  if (jd>-1) pgr->jsiz = 1 + d2max - d2min;
  siz = pgr->isiz * pgr->jsiz;
  if (siz>1) {
    pgr->grid = (gadouble *)galloc(sizeof(gadouble)*siz,"pgrg");
    pgr->umask = (char *)galloc(sizeof(char)*siz,"pgrgu");
    if (pgr->grid==NULL) {
      gaprnt (0,"Memory Allocation Error: Grid Request\n");
      return (2);
    }
    if (pgr->umask==NULL) {
      gaprnt (0,"Memory Allocation Error: Grid Request\n");
      return (2);
    }
  } else {
    pgr->grid = &(pgr->rmin);
    pgr->umask = &(pgr->umin); 
  }

  /* Normalize time coordinate if not varying */
  /* JMA: This does not handle leap years properly!!!!  Gotta fix this someday */

  if (pfi->climo && id!=3 && jd!=3) clicyc(d+3);

  /* Check for entirely undefined grid */

  flag = 0;
  for (i=0; i<5; i++) {
    if (i!=id && i!=jd && (d[i]<0 || d[i]>=pfi->dnum[i])) flag = 1;
  }
  if (flag) {
    for (i=0; i<siz; i++) {
      *(pgr->grid+i) = pfi->undef;
      *(pgr->umask+i) = 0;
    }
    return (0);
  }

  /* Move appropriate grid values */

  if (id==-1 && jd==-1) {
    pos = d[0] + d[1]*ys + d[2]*zs + d[3]*ts + d[4]*es;
    pgr->rmin = *(pfi->rbuf+pos);
    pgr->umin = *(pfi->ubuf+pos);
    return (0);
  }

  v = pgr->grid;
  vmask = pgr->umask;

  if (jd==-1) {
    for (xt=d1min; xt<=d1max; xt++) {
      d[id] = xt;
      if (id==3 && pfi->climo) clicyc(d+3);
      if (d[id]<0 || d[id]>=pfi->dnum[id]) {
	*v = pfi->undef;
	*vmask = 0;
      }
      else {
        pos = d[0] + d[1]*ys + d[2]*zs + d[3]*ts + d[4]*es;
        *v     = *(pfi->rbuf+pos);
	*vmask = *(pfi->ubuf+pos);
      }
      v++;vmask++;
    }
    return (0);
  }

  for (yt=d2min; yt<=d2max; yt++) {
    d[jd] = yt;
    if (jd==3 && pfi->climo) clicyc(d+3);
    for (d[id]=d1min; d[id]<=d1max; d[id]++) {
      if (d[jd]<0 || d[jd]>=pfi->dnum[jd] ||
          d[id]<0 || d[id]>=pfi->dnum[id]) {
	*v = pfi->undef;
	*vmask = 0;
      }
      else {
        pos = d[0] + d[1]*ys + d[2]*zs + d[3]*ts + d[4]*es;
        *v     = *(pfi->rbuf+pos);
	*vmask = *(pfi->ubuf+pos);
      }
      v++;vmask++;
    }
  }
  return(0);
}

void clicyc (gaint *ti) {
  if (pfi->climo>0) {
    while (*ti>pfi->cysiz-1) *ti = *ti - pfi->cysiz;
    while (*ti<0) *ti = *ti + pfi->cysiz;
  }
}

/* Fill in grid for predefined variable */

gaint gagpre (void) {
gadouble (*conv)(gadouble *, gadouble);
gaint d[5],id,jd,i,dim;
gadouble *gr,*vals,t;
char *gru;

  id = pgr->idim;
  jd = pgr->jdim;
  for (i=0; i<5; i++) d[i] = pgr->dimmin[i];

  dim = pvr->offset;
  conv = pfi->gr2ab[dim];
  vals = pfi->grvals[dim];

  gr = pgr->grid;
  gru = pgr->umask;

  if (id>-1 && jd>-1) {
    for (d[jd]=pgr->dimmin[jd]; d[jd]<=pgr->dimmax[jd]; d[jd]++) {
      for (d[id]=pgr->dimmin[id]; d[id]<=pgr->dimmax[id]; d[id]++) {
        t = (gadouble)(d[dim]);
        *gr = conv(vals, t);
	*gru = 1;
        gr++;gru++;
      }
    }
  } else if (id>-1) {
    for (d[id]=pgr->dimmin[id]; d[id]<=pgr->dimmax[id]; d[id]++) {
      t = (gadouble)(d[dim]);
      *gr = conv(vals, t);
      *gru = 1;
      gr++;gru++;
    }
  } else {
    t = (gadouble)(d[dim]);
    *gr = conv(vals, t);
    *gru = 1;
  }
  return (0);
}


/* Read index data, in this case GRIB type data.
   Currently assumes no pole point, and only one record
   per grid.  */

gaint gairow (gaint x, gaint y, gaint z, gaint t, gaint e, gaint offset, gaint len, 
	      gadouble *gr, char *gru) {
 gaint irec,ioff,bstrt,bend,blen,cstrt,cend,clen,rc;
 gaint ival,i,yy,bpos,boff,siz,gtyp,xsiz,ysiz;
 off_t fpos;
 float dsf,bsf,ref;
#if GRIB2
 struct g2buff *buff=NULL;
 g2int seek,ifld;
 gaint g2off,ng2elems=2;
#endif

 if (pfi->idxflg==0) return (1); 

 /* code that grib1 and grib2 data sets can share */
 if (pfi->ppflag) {
   xsiz = pfi->ppisiz; 
   ysiz = pfi->ppjsiz;
 }
 else {
   xsiz = pfi->dnum[0]; 
   ysiz = pfi->dnum[1];
 }
 

 /* GRIB1 */
 if (pfi->idxflg==1) {
   /* Figure out position and length of the I/O */
   gtyp = *(pindx->hipnt+3);
   irec = (e-1)*pfi->dnum[3]*pfi->trecs + (t-1)*pfi->trecs + pvr->recoff + z - 1;
   if (gtyp==29) {
     xsiz = 145;
     irec = irec*6;
     if (y<37) y--;
     else { irec+=3; y-=37; }
     yy = y;
   } else {
     irec = irec*3;
     if (pfi->yrflg) yy = ysiz - y;
     else yy = y-1;
   }
   if (pfi->ppflag) 
     ioff = offset;               
   else 
     ioff = yy*xsiz + x - 1;      
   boff = ioff;
   blen = *(pindx->intpnt + irec + 2);
   if (blen<0) {
     for (i=0; i<len; i++) *(gr+i) = pfi->undef;
     for (i=0; i<len; i++) *(gru+i) = 0;
     return (0);
   }
   bpos = *(pindx->intpnt + irec + 1);
   dsf = *(pindx->fltpnt+irec);
   bsf = *(pindx->fltpnt+irec+1);
   ref = *(pindx->fltpnt+irec+2);
   if (bpos>-900 && bpos!=bpsav) {
     bpsav = bpos;
     siz = 2+(xsiz*ysiz)/8;
     if (siz>bssav) {
       if (bcflag) {
	 gree(bcache,"f114");
	 gree(bpcach,"f115");
       }
       bcache = (unsigned char *)galloc(siz,"bcache");
       bpcach = (gaint *)galloc(sizeof(gaint)*(xsiz*ysiz+1),"bpcach");
       if (bcache==NULL||bpcach==NULL) {
	 gaprnt(0,"Memory Allocation Error During GRIB I/O\n");
	 return (1);
       }
       bssav = siz;
       bcflag = 1;
     }
     rc = fseeko(pfi->infile, bpos, 0);
     rc = fread(bcache,1,siz,pfi->infile);
     if (rc!=siz) {
       gaprnt(0,"GRIB I/O Error: Bit Map I/O\n");
       return(1);
     }
     boff=1;
     for (i=0; i<xsiz*ysiz; i++) {
       if (gagbb(bcache,i,1)) {
	 *(bpcach+i) = boff;
	 boff++;
       } else {
	 *(bpcach+i) = -1*boff;
       }
     }
     *(bpcach+xsiz*ysiz) = boff;  /* Provide an ending offset */
   }
   if (bpos>-900) {
     boff = *(bpcach+ioff);
     if (boff<0) boff = -1*boff;
     boff--;
     bstrt = blen * boff;
     boff = *(bpcach+ioff+len);
     if (boff<0) boff = -1*boff;
     boff--;
     bend = blen * boff - 1;
   } else {
     bstrt = blen * boff;
     bend = bstrt + blen*len;
   }
   cstrt = bstrt/8;
   cend = bend/8;
   clen = cend-cstrt+2;
   fpos = *(pindx->intpnt+irec);
   rc = gaird(fpos,cstrt,clen,xsiz,ysiz,blen);
   if (rc) return(rc);
   bstrt = bstrt - cstrt*8;
   for (i=0; i<len; i++) {
     if (bpos>-900 && *(bpcach+ioff+i)<0) {
       *(gr+i) = pfi->undef;
       *(gru+i) = 0;
     }
     else {
       ival = gagbb(pfi->pbuf,bstrt,blen);
       *(gr+i) = ( ref + (gadouble)ival * bsf )/dsf;
       *(gru+i) = 1;
       bstrt += blen;
     }
   }
   return (0);
 }

#if GRIB2
 /* GRIB 2 */
 if (pfi->idxflg==2) {
   
  /* figure out which record to retrieve from index file */
  irec = (e-1)*pfi->dnum[3]*pfi->trecs + (t-1)*pfi->trecs + pvr->recoff + z - 1;
  irec = irec * ng2elems;
  if (irec > pfi->g2indx->g2intnum) {
    sprintf(pout,"GRIB2 I/O error: irec=%d is greater than g2intnum=%d\n",irec,pfi->g2indx->g2intnum);
    gaprnt(0,pout);
    return(1);
  }

  /* get file position offset and field number from grib2map file */
  seek = *(pfi->g2indx->g2intpnt+irec+0);
  ifld = *(pfi->g2indx->g2intpnt+irec+1);
  if (debug) printf("gairow debug: seek,ifld = %d %d\n",seek, ifld);
  if (seek<-900 && ifld<-900) {                    /* grid is missing */
    for (i=0; i<len; i++) *(gru+i) = 0;
    return(0);
  }
  buff = g2check(z,t,e);                           /* check if grid is already in the cache */
  if (buff==NULL) buff = g2read(seek,ifld,z,t,e);  /* if not in cache, read the grid */
  if (buff==NULL) return(1);                       /* give up */

  /* copy required data from grid */
  if (pfi->ppflag) {
    g2off = offset;                    
  }
  else {
    g2off = (y-1)*xsiz + x - 1;        
  }
  for (i=0; i<len; i++) {
    if (buff->mask[g2off+i]==1) {  
      *(gr+i)  = (gadouble) buff->fld[g2off+i];
      *(gru+i) = 1;
    }
    else {
      *(gru+i) = 0;
    }
  }
  return(0);    
 }
#endif
 return(0);
}


#if GRIB2
/* Routine to read a grib2 message.
   New blocks are added to the end of the chain.
   If the cache is too large, the first block in the chain is released. */
struct g2buff * g2read (g2int seek, g2int ifld, gaint z, gaint t, gaint e) {
struct g2buff *newbuff,*buff1,*buff2,*lastbuff;
gaint i,x,y,rc,unpack,expand,ierr,newbuffsize,buff1size,roff,field;
gaint sbit1,sbit2,sbit3,sbit4,gdt,soct,nx,ny,missflg,diag,cpbm=0,numdp;
gaint flag,A,pos,yfac1,yfac2,yadd1,yadd2,xfac1,xfac2,xadd1,xadd2;
long  lskip=0,lgrib=-999;
unsigned char  *cgrib,*mycgrib;
size_t  lengrib;
gafloat *fld,miss1,miss2;
char *mask;
unsigned char *sect,*s1,*s2,*s3,*s4,*s5,*s6,*s7;
gribfield *gfld;

  diag=0; 
  /* move the file pointer and get length of record */
  seekgb(pfi->infile,seek,32000,&lskip,&lgrib);
  if (lgrib == -999) {
    gaprnt(0,"GRIB2 I/O error: seekgb failed completely\n");
  } 
  else if (lgrib == 0) {
    gaprnt(0,"GRIB2 I/O error: message not found\n");
    if (debug) {
      if (pfi->tmplat) 
	printf("g2read debug: seekgb returned 0. file=%s iseek=%d\n",pfi->tempname,seek);
      else
	printf("g2read debug: seekgb returned 0. file=%s iseek=%d\n",pfi->name,seek);
    }
    return(NULL);     
  }
  /* allocate memory for the record */
  cgrib=(unsigned char *)galloc(lgrib,"cgrib");
  if (cgrib == NULL) {
    gaprnt(0,"GRIB2 I/O error: unable to allocate memory for cgrib \n");
    return(NULL);
  }
  mycgrib = cgrib;
  /* move file pointer past stuff to skip at beginning of record */
  rc=fseek(pfi->infile,lskip,SEEK_SET);
  if (rc) {
    gaprnt(0,"GRIB2 I/O error: fseek failed \n");
    goto g2err;
  }
  /* read the grib record */
  lengrib=fread(cgrib,sizeof(unsigned char),lgrib,pfi->infile);
  if (lengrib < lgrib) {
    gaprnt(0,"GRIB2 I/O Error: unable to read record \n");
    goto g2err;
  }

  /* we need to extract certain octets from the grib record, so set pointers to each section */
  s1 = s2 = s3 = s4 = s5 = s6 = s7 = NULL;
  /* Section 0, always 16 octets long */
  roff = 16;                         
  /* Section 1, the Identification Section */
  sect = mycgrib+roff;  
  i = gagby(sect,4,1);
  if (i==1) {
    s1 = mycgrib+roff;  
    if (diag) printf("Sec1   %d\n",gagby(s1,0,4));
    roff += gagby(s1,0,4);     
  } else {
    sprintf (pout,"GRIB2 I/O Error: reading header, section 1 expected, found %i\n",i);
    gaprnt (0,pout);
    goto g2err;
  }
  field=1;
  while (field<=ifld) {
    /* Section 2, the Local Use Section */
    sect = mycgrib + roff;
    i = gagby(sect,4,1);
    if (i==2) {
      s2 = mycgrib + roff;
      if (diag) printf("Sec2.%d %d\n",field,gagby(s2,0,4));
      roff += gagby(s2,0,4);
    } else {
      if (diag) printf("Sec2.%d not present\n",field);
    }

    /* Section 3, the Grid Definition Section */
    sect = mycgrib + roff;
    i = gagby(sect,4,1);
    if (i==3) {
      s3 = mycgrib + roff;
      if (diag) printf("Sec3.%d %d\n",field,gagby(s3,0,4));
      roff += gagby(s3,0,4);  
    } 
    else if (field==1) {
      sprintf (pout,"GRIB2 I/O Error: reading header, section 3 expected, found %i\n",i);
      gaprnt (0,pout);
      goto g2err;
    } 
    else if (field>1) {
      if (diag) printf("Sec3.%d not present\n",field);
    }
    /* Section 4, the Product Definition Section */
    sect = mycgrib + roff;
    i = gagby(sect,4,1);
    if (i==4) {
      s4 = mycgrib + roff;
      if (diag) printf("Sec4.%d %d\n",field,gagby(s4,0,4));
      roff += gagby(s4,0,4);
    }
    else if (field==1) {
      sprintf (pout,"GRIB2 I/O Error: reading header, section 4 expected, found %i\n",i);
      gaprnt (0,pout);
      goto g2err;
    }
    else if (field>1) {
      if (diag) printf("Sec4.%d not present\n",field);
    }
    /* Section 5, the Data Representation Section */
    sect = mycgrib + roff;
    i = gagby(sect,4,1);
    if (i==5) {
      s5 = mycgrib + roff;
      if (diag) printf("Sec5.%d %d\n",field,gagby(s5,0,4));
      roff += gagby(s5,0,4);
    }
    else if (field==1) {
      sprintf (pout,"GRIB2 I/O Error: reading header, section 5 expected, found %i\n",i);
      gaprnt (0,pout);
      goto g2err;
    }
    else if (field>1) {
      if (diag) printf("Sec5.%d not present\n",field);
    }
    /* Section 6, the Bit Map Section*/
    sect = mycgrib + roff;
    i = gagby(sect,4,1);
    if (i==6) {
      s6 = mycgrib + roff;
      if (diag) printf("Sec6.%d %d\n",field,gagby(s6,0,4));
      roff += gagby(s6,0,4);
    }
    else {
      if (diag) printf("Sec6.%d not present\n",field);
    }
    
    /* Section 7, the Data Section */
    sect = mycgrib+roff;
    i = gagby(sect,4,1);
    if (i==7) {
      s7 = mycgrib + roff;
      if (diag) printf("Sec7.%d %d\n",field,gagby(s7,0,4));
      roff += gagby(s7,0,4);
    }
    else if (field==1) {
      sprintf (pout,"GRIB2 I/O Error: reading header, section 7 expected, found %i\n",i);
      gaprnt(0,pout);
      goto g2err;
    }
    else if (field>1) {
      if (diag) printf("Sec7.%d not present\n",field);
    }
    field++;
  }

  /* get the scanning mode */
  gdt = gagby(s3,12,2);
  switch (gdt) {
    case  0:
    case  1:
    case  2:
    case  3: soct = 71; break;
    case 10: soct = 59; break;
    case 20:
    case 30:
    case 31: soct = 64; break;
    case 40:
    case 41:
    case 42: soct = 71; break;
    case 90: soct = 63; break;
    case 204: soct = 71; break; 
    default: 
      sprintf (pout,"g2read error: Grid Definition Template %d not handled\n",gdt);
      gaprnt (0,pout);
      goto g2err;
  };
  sbit1 = gagbb(s3+soct,0,1);
  sbit2 = gagbb(s3+soct,1,1);
  sbit3 = gagbb(s3+soct,2,1);
  sbit4 = gagbb(s3+soct,3,1);
  numdp = gagby(s3,6,4); /* number of data points */
  nx = gagby(s3,30,4);   /* these seem to be in the same location */
  ny = gagby(s3,34,4);   /* for for all grid definition templates */

  /* Use the g2clib routine g2_getfld to unpack the desired field in the record */
  unpack=1;
  expand=1;
  ierr=g2_getfld(cgrib,ifld,unpack,expand,&gfld);
  if (ierr) {
    sprintf (pout,"GRIB2 I/O Error: g2_getfld failed, ierr=%d\n",ierr);
    gaprnt (0,pout);
    goto g2err;
  }
  /* set up a new block to add to the grib2 cache */
  newbuff = NULL;
  if ((newbuff = (struct g2buff *)galloc(sizeof(struct g2buff),"g2buff1"))==NULL) {
    gaprnt (0,"GRIB2 I/O error: unable to allocate memory for cache buffer\n");
    goto g2err;
  }
  fld=NULL;
  if ((fld = (gafloat *)galloc(gfld->ngrdpts*sizeof(gafloat),"g2fld1"))==NULL) {
    gaprnt (0,"GRIB2 I/O error: unable to allocate memory for cache grid \n");
    gree(newbuff,"f253");
    goto g2err;
  }
  mask=NULL;
  if ((mask = (char *)galloc(gfld->ngrdpts*sizeof(char),"g2mask1"))==NULL) {
    gaprnt (0,"GRIB2 I/O error: unable to allocate memory for cache undef mask \n");
    gree(fld,"f255");
    gree(newbuff,"f256");
    goto g2err;
  }
  /* populate the new cache block  */
  newbuff->fld  = fld;
  newbuff->mask = mask;
  newbuff->prev = NULL;
  newbuff->next = NULL;
  newbuff->fseq = pfi->fseq;
  newbuff->z    = z;
  newbuff->t    = t;
  newbuff->e    = e;
  newbuff->size = gfld->ngrdpts;
  for (i=0;i<16;i++) *(newbuff->abbrv+i)=*(pvr->abbrv+i);
  /* size = structure + data + undef mask */
  newbuffsize = sizeof(struct g2buff) + newbuff->size*(sizeof(gafloat) + sizeof(char));

  /* create the undef mask */
  if (gfld->ibmap!=0 && gfld->ibmap!=254 && gfld->ibmap!=255) {
    sprintf (pout,"GRIB2 I/O Error: Predefined bitmap applies (ibmap=%ld) \n",gfld->ibmap);
    gaprnt (0,pout);
    goto g2err1;
  }
  else if (gfld->ibmap==255) {  
    /* bitmap doesn't apply -- all data are good */
    for (i=0;i<newbuff->size;i++) *(newbuff->mask+i) = 1;
    cpbm = 0;
  }
  else if (gfld->ibmap==0 || gfld->ibmap==254) {       
    /* copy the bit map */
    cpbm = 1;
  } 
  /* complex packing with spatial diff uses special missing value management */
  missflg = miss1 = miss2 = 0;
  if (gfld->idrtnum==2 || gfld->idrtnum==3) {
    if (gfld->idrtmpl[6] == 1) {
      /* primary missing value substitute */
      missflg=1;
      miss1 = ieee2flt(&s5[23]);
      miss2 = miss1;
    }
    else if (gfld->idrtmpl[6] == 2) {
    /* primary and secondary missing value substitutes */
      missflg=1;
      miss1 = ieee2flt(&s5[23]);
      miss2 = ieee2flt(&s5[27]);
    }
  }

  /* use scanning mode bits to set up coefficients for proper placement in rows/columns */
  if (sbit3!=0) {
    printf("Contact GrADS developers and tell them you have grib2 data written out columnwise\n");
    goto g2err1;
  }
  if (sbit2==1) {  /* columns scan south to north (normal) */
    yfac1 = 1;
    yfac2 = 1;
    yadd1 = 0;
    yadd2 = 0;
  } else {
    yfac1 = -1;
    yfac2 = -1;
    yadd1 = ny - 1;
    yadd2 = ny - 1;
  }
  if (sbit4==0) {   /* all rows scan in the same direction */
    if (sbit1==0) { /* 1st row scans west to east (normal) */
      xfac1 = +1;
      xfac2 = +1;
      xadd1 = 0;
      xadd2 = 0;
    }
    else {
      xfac1 = -1; 
      xfac2 = -1;
      xadd1 = nx - 1;
      xadd2 = nx - 1;
    }
  }
  else {             /* rows scan in alternating directions */
    if (sbit1==0) {  /* 1st row scans west to east (normal) */
      xfac1 = +1; 
      xfac2 = -1;
      xadd1 = 0;
      xadd2 = nx - 1;
    }
    else {
      xfac1 = -1; 
      xfac2 = +1;
      xadd1 = nx - 1;
      xadd2 = 0;
    }
  }

  if (nx==-1) {
    /* copy each element in the grib2 field and mask grids as a 1D array */
    for (i=0; i<numdp; i++) {
      if (cpbm) 
	(gfld->bmap[i]==1) ? (*(newbuff->mask+i) = 1) : (*(newbuff->mask+i) = 0) ;
      if (missflg && (gfld->fld[i]==miss1 || gfld->fld[i]==miss2)) 
	*(newbuff->mask+i) = 0;
      else 
	*(newbuff->fld+i) = gfld->fld[i];
    }
  }
  else {
    /* copy each element in the grib2 field and mask grids 
       into the right row/column in the cache grids */
    flag=1;
    pos=0;
    for (y=0; y<ny; y++) {
      for (x=0; x<nx; x++) {
	if (flag) {
	  A = (yadd1+(yfac1*y))*nx + (xadd1+(xfac1*x));
	  if (cpbm) {
	    (gfld->bmap[pos]==1) ? (*(newbuff->mask+A) = 1) : (*(newbuff->mask+A) = 0) ;
	  }
	  if (missflg && (gfld->fld[pos]==miss1 || gfld->fld[pos]==miss2)) 
	    *(newbuff->mask+A) = 0;
	  else
	    *(newbuff->fld+A) = gfld->fld[pos];
	} 
	else {
	  A = (yadd2+(y*yfac2))*nx + (xadd2+(xfac2*x));
	  if (cpbm) {
	    (gfld->bmap[pos]==1) ? (*(newbuff->mask+A) = 1) : (*(newbuff->mask+A) = 0) ;
	  }
	  if (missflg && (gfld->fld[pos]==miss1 || gfld->fld[pos]==miss2)) 
	    *(newbuff->mask+A) = 0;
	  else
	    *(newbuff->fld+A) = gfld->fld[pos];
	}
	pos++;
      }
      flag = flag==0 ? 1 : 0 ;
    }
  }

  /* set up or adjust the anchor's pointers */
  if (anchor==NULL) {
    /* make sure cache will be big enough for the new block */
    if (newbuffsize > MAXG2CACHE) {
      gaprnt(0,"GRIB2 I/O error: size of cache (MAXG2CACHE) is too small\n");
      goto g2err1;
    }
    /* allocate space for a new anchor and initialize */
    if ((anchor = (struct g2anchor *)galloc(sizeof(struct g2anchor),"anchor"))==NULL) {
      gaprnt(0,"GRIB2 I/O error: unable to allocate memory for cache anchor \n");
      goto g2err1;
    }
    anchor->start = newbuff;
    anchor->end   = newbuff;
    anchor->total = newbuffsize;
  } 
  else {
    /* make sure there's room in the cache buffer for the new block */
    while ((newbuffsize + anchor->total) > MAXG2CACHE) {
      /* point to the first block in the chain and get its size */
      buff1 = anchor->start;
      buff1size = sizeof(struct g2buff) + buff1->size*(sizeof(gafloat) + sizeof(char));
      if (buff1->next==NULL) {   
	/* the first block was the only block */
	anchor->start = NULL; 
	anchor->end   = NULL; 
	anchor->total = 0;
      }
      else {                    
      /* move the start of the chain from 1st to 2nd block  */
	buff2 = buff1->next;  
	buff2->prev = NULL;     
	anchor->start = buff2;     
	/* adjust total size of cache */
	anchor->total = anchor->total - buff1size;
      }
      /* release memory from 1st block */
      gree(buff1->fld,"f262");
      gree(buff1->mask,"f263");
      gree(buff1,"f264");
    }
    /* now add the new block onto the end of the chain */
    if (anchor->end==NULL) {
      /* no blocks are hanging off anchor */
      newbuff->prev = NULL;
      newbuff->next = NULL;
      anchor->start = newbuff;
      anchor->end   = newbuff;
      anchor->total = newbuffsize;

    } 
    else {
      lastbuff = anchor->end;
      lastbuff->next = newbuff;
      newbuff->prev = lastbuff;
      newbuff->next = NULL;
      anchor->end = newbuff;
      anchor->total = anchor->total + newbuffsize;
    }
  }
  
  /* release memory */
  (void)g2_free(gfld);
  gree(cgrib,"f262");
  return (anchor->end);

g2err1:
  gree(newbuff->fld,"f257");
  gree(newbuff->mask,"f258");
  gree(newbuff,"f259");
  goto g2err;
g2err:
  gree(cgrib,"f261");
  return(NULL);
}

/* Routine to check if a requested grid already exists in the grib2 cache.
   Blocks are checked starting from the end of the chain because 
   the most recently read grid is most likely to be the one we need 
*/
struct g2buff * g2check (gaint z, gaint t, gaint e) {
struct g2buff *buff;
gaint size; 

  if (anchor!=NULL) {
    buff = anchor->end; 
    if (pfi->ppflag) {
      size = pfi->ppisiz * pfi->ppjsiz;
    }
    else {
      size = pfi->dnum[0]*pfi->dnum[1];
    }
    while (buff != NULL) {
      if ((buff->fseq == pfi->fseq) &&              
	  (buff->z == z) &&
	  (buff->t == t) &&
	  (buff->e == e) &&
	  (cmpwrd(buff->abbrv,pvr->abbrv)) &&
	  (buff->size == size)) {
	/* found grid in the cache */
	return(buff);
      }
      else {
	/* next block */
	buff = buff->prev;
      }
    }
    /* grid not found in cache */
    return(NULL);
  }
  else {
    /* no existing cache */
    return (NULL);
  }
}

/* Routine to clear the grib2 cache. */
void g2clear (void) {
struct g2buff *buff1,*buff2;

 if (anchor!=NULL) {
   /* release the first block in chain until there's only one block left */
   while (anchor->end->prev != NULL) {
     /* move the start of the chain from 1st to 2nd block  */
     buff1 = anchor->start;
     buff2 = buff1->next;
     buff2->prev = NULL;
     anchor->start = buff2;
     /* release memory from 1st block */
     gree(buff1->fld,"f270a");
     gree(buff1->mask,"f271a");
     gree(buff1,"f272a");
   }
   /* free the last block */
   buff1 = anchor->start;
   gree(buff1->fld,"f270");
   gree(buff1->mask,"f271");
   gree(buff1,"f272");
   gree(anchor,"f273");
   anchor = NULL;
 }
}
#endif  /* matches #if GRIB2 */

gaint gaird (off_t fpos, gaint cstrt, gaint clen, gaint xsiz, gaint ysiz, gaint blen) {
gaint rc,siz,i;
  if (pfi->ppflag && pgr->idim==0 && pgr->jdim==1) {
    if (!cflag) {
      cflag = 1;
      siz = 5 + xsiz*ysiz*blen/8;  /* qqq  Warning:  siz calc does not */
                                   /* qqq  take into account bms!!! */
      cache = (char *)galloc(siz,"cache");
      if (cache==NULL) {
        gaprnt(0,"GRIB Memory Allocation Error\n");
        return (1);
      }
      rc = fseeko(pfi->infile, fpos, 0);
      rc = fread(cache,sizeof(char),siz,pfi->infile);
      if (rc==0) {
        sprintf (pout,"GRIB I/O Error reading %i bytes at %ld\n",siz,fpos);
        gaprnt (0,pout);
        gaprnt (0,"  File name is: ");
        if (pfi->tempname) gaprnt(0,pfi->tempname);
        else gaprnt(0,pfi->name);
        gaprnt (0,"\n");
        return (1);
      }
    }
    if (cache==NULL) return(1);
    for (i=0; i<clen; i++) {
      *(pfi->pbuf+i) = *(cache+cstrt+i);
    }
  } else {
    rc = fseeko(pfi->infile, fpos+cstrt, 0);
    rc = fread (pfi->pbuf, sizeof(char), clen, pfi->infile);
    if (rc==0) {
      sprintf (pout,"GRIB I/O Error reading %i bytes at %ld\n",clen,fpos+cstrt);
      gaprnt (0,pout);
      gaprnt (0,"  File name is: ");
      if (pfi->tempname) gaprnt(0,pfi->tempname);
      else gaprnt(0,pfi->name);
      gaprnt (0,"\n");
      return(1);
    }
  }
  return(0);
}

/* Read in a row of data from a pre-projected grid data set.
   This involves doing interpolation to the lat-lon
   grid */

gaint gaprow (gaint x, gaint y, gaint z, gaint t, gaint e, gaint tt, 
	      gaint len, gadouble *gr, char *gru) {
gadouble p[4],dx,dy,g1,g2;
gadouble vals[9],wts[9],sum,wt;
char umask[9];
gaint ioffs[9],cnt,ig0,goflg;
gaint rc,i,j,ig,ioff,ncig=0,ncjg=0;
off_t pos,pos0;

  /* Handle generalized arbitrary points + weights */

  if (pfi->ppflag==8) {
    /* "cnt"  is the number of interpolation grids provided in pdef file ("num" in PDEF entry) 
       "pos0" is the file position of the native lat/lon grid we're going to read 
       "ig0"  is the offset into the 2-D grid where the I/O will begin (N.B. when x=1 and y=1, ig0=0)
     */
    cnt  = (gaint)(pfi->ppvals[0]+0.1);   
    pos0 = (e-1)*(pfi->dnum[3]*pfi->tsiz) + (tt-1)*(pfi->tsiz) + pvr->offset + (z-1)*(pfi->gsiz);
    ig0  = (y-1) * pfi->dnum[0] + x - 1;
    /* loop over all grid points in the row */
    for (i=0; i<len; i++) {
      ig = ig0 + i;
      goflg = 0;
      /* get the interpolation indices (ioffs) and their weights (wts) that were read
	 from the pdef file -- for ppflag==8, these index values start at 1, not 0 */
      for (j=0; j<cnt; j++) {
        ioffs[j] = *(pfi->ppi[j]+ig);
        if (ioffs[j] >= 1 && ioffs[j] <= pfi->gsiz) goflg = 1; 
        wts[j] = *(pfi->ppf[j]+ig);
      }
      if (!goflg) {
	*gr = pgr->undef;
	*gru = 0;
      }
      else {
	/* now read the interpolation data values from the native grid */
        goflg = 1;
        j = 0;
        sum = 0.0; wt = 0.0;
        while (j<cnt) {
          if (ioffs[j] >= 1) {
            if (pfi->idxflg) {
	      /* gairow wants the grid offset to be 0-referenced
		 so shift ioffs index back by 1  */
	      rc = gairow(x,y,z,t,e,ioffs[j]-1,1,vals+j,umask+j);    /* grib */
	    }
#if USENETCDF==1
            else if (pfi->ncflg==1) {
	      rc = gancsetup();
	      if (rc) return (rc);
	      /* ncig and ncjg are the i,j indices of the interpolation data value to be read.
		 They are required instead of ioffs for reading data from NetCDF and HDF grids.
	         The grid indices ioffs, ncig, and ncjg all start at 1, not 0. The code in 
		 gancrow and gahrow will shift the indices back so they start at 0 for the I/O */
	      ncig = (gaint)(1+((ioffs[j]-1)%pfi->ppisiz)); 
	      ncjg = (gaint)(1+((ioffs[j]-1)/pfi->ppisiz));
	      rc = gancrow(ncig,ncjg,z,tt,e,1,vals+j,umask+j);    /* netcdf */
	    }
#endif
#if USEHDF==1
            else if (pfi->ncflg==2) {
	      /* see comment above */
	      ncig = (gaint)(1+((ioffs[j]-1)%pfi->ppisiz)); 
	      ncjg = (gaint)(1+((ioffs[j]-1)/pfi->ppisiz));
	      rc = gahrow(ncig,ncjg,z,tt,e,1,vals+j,umask+j);     /* hdfsds */
	    }
#endif
            else {
	      /* the ioffs index is shifted back by 1 here */
	      pos = pos0 + ioffs[j] - 1;
	      rc = garead(pos,1,vals+j,umask+j);                  /* binary */
	    }
            if (rc) return(rc);
            if (*(umask+j)==0) {
              goflg = 0;
              break;
            }
            sum = sum + *(vals+j) * *(wts+j);
            wt = wt + *(wts+j);
          }
          j++;
        }
        if (goflg && wt!=0.0) {
	  /* Result is weighted average */
	  *gr = sum/wt;
	  *gru = 1;
	}
        else {
	  *gr = pgr->undef;
	  *gru = 0;
	}
      }
      gr++; gru++;
    }
  }  /* matches if (pfi->ppflag==8)  */
  else {
    for (i=0; i<len; i++) {
      ig = (y-1) * pfi->dnum[0] + x + i - 1;
      ioff = *(pfi->ppi[0]+ig);              /* ioff index values start at 0 */
      if (ioff<0) {
	*gr = pgr->undef;
	*gru = 0;
      }
      else {
        dx = (gadouble)*(pfi->ppf[0]+ig);
        dy = (gadouble)*(pfi->ppf[1]+ig);
        pos = (e-1)*(pfi->dnum[3]*pfi->tsiz) + (tt-1)*(pfi->tsiz) + pvr->offset + (z-1)*(pfi->gsiz) + ioff;

	/* Get the first two pre-projected grid values */
        if (pfi->idxflg) {                      
	  rc = gairow(x,y,z,t,e,ioff,2,p,umask);                   /* grib */ 
	}
#if USENETCDF==1
        else if (pfi->ncflg==1) {               
	  rc = gancsetup();
	  if (rc) return(rc);
	  ncig = (gaint)(1 + ioff%pfi->ppisiz);
          ncjg = (gaint)(1 + ioff/pfi->ppisiz);
	  rc = gancrow(ncig,ncjg,z,tt,e,2,p,umask);                /* netcdf */  
	}
#endif
#if USEHDF==1
        else if (pfi->ncflg==2) {              
	  ncig = (gaint)(1 + ioff%pfi->ppisiz);
          ncjg = (gaint)(1 + ioff/pfi->ppisiz);
	  rc = gahrow(ncig,ncjg,z,tt,e,2,p,umask);                 /* hdf */  
	}
#endif
        else {                               
	  rc = garead(pos,2,p,umask);                              /* binary */
	}
        if (rc) return(rc);

	/* Get the second two pre-projected grid values */
        if (pfi->idxflg) {
	  rc = gairow(x,y,z,t,e,ioff+pfi->ppisiz,2,p+2,umask+2);   /* grib */
	}
#if USENETCDF==1
        else if (pfi->ncflg==1) {
          ncjg++;
	  rc = gancrow(ncig,ncjg,z,tt,e,2,p+2,umask+2);            /* netcdf */
	}
#endif
#if USEHDF==1
        else if (pfi->ncflg==2) {
          ncjg++; 
	  rc = gahrow(ncig,ncjg,z,tt,e,2,p+2,umask+2);             /* hdf */  
	}
#endif
        else {
	  rc = garead(pos+pfi->ppisiz,2,p+2,umask+2);              /* binary */  
	}
        if (rc) return(rc);

	/* Do the bilinear interpolation, as long as we have no undefs */
        if (umask[0]==0 || umask[1]==0 || umask[2]==0 || umask[3]==0) {
	  *gr = pgr->undef;
	  *gru = 0;
	}
        else {
          g1 = p[0] + (p[1]-p[0])*dx;
          g2 = p[2] + (p[3]-p[2])*dx;
          *gr = g1 + (g2-g1)*dy;
	  *gru = 1;
        }
      }
      gr++; gru++;
    }
  }
  return(0);
}
 

/* Set up variable ID, undef value, and unpacking values for NetCDF variables */
gaint gancsetup (void) {
#if USENETCDF == 1
  gaint vid,error,rc,oldncopts;
  gadouble val;
  
  /* Turn off automatic error handling. */
  ncopts = NC_VERBOSE ;
  oldncopts = ncopts ;
  ncopts = 0;

  /* Get the varid if we haven't already done that for this file */
  if (pvr->ncvid == -888) {
    ncopts = oldncopts ;
    return(1);  /* already tried and failed */
  }
  if (pvr->ncvid == -999) {
    error=0;
    if (pvr->longnm[0] != '\0') {
      rc = nc_inq_varid(pfi->ncid, pvr->longnm, &vid);
    }
    else {
      rc = nc_inq_varid(pfi->ncid, pvr->abbrv, &vid);
    }
    if (rc != NC_NOERR) error=1;
    if (error) {
      pvr->ncvid = -888;  /* set flag so we won't try this variable ever again */
      if (pvr->longnm[0] != '\0') {
	sprintf(pout,"gancsetup error: Variable %s not in netcdf file\n",pvr->longnm);
      }
      else {
	sprintf(pout,"gancsetup error: Variable %s not in netcdf file\n",pvr->abbrv);
      }
      gaprnt (0,pout);
      ncopts = oldncopts ;
      return (1);
    }
    /* No errors, so we can set the varid in the gavar structure */
    pvr->ncvid = vid;
    
    /* If undef attribute name is given, get the undef value */
    if (pfi->undefattrflg) {
      if (nc_get_att_double(pfi->ncid, pvr->ncvid, pfi->undefattr, &val) != NC_NOERR) {
	sprintf(pout,"Warning: Could not retrieve \"%s\" -- using %g instead\n",
		pfi->undefattr,pfi->undef);
	gaprnt(1,pout);
	pvr->undef = pfi->undef;
      }
      else {
	pvr->undef = val;
      }
    }
    /* If no undef attribute name is given, copy the file-wide undef */
    else {
      pvr->undef = pfi->undef;
    }

    /* If data are packed, get the scale factor and offset attribute values */
    if (pfi->packflg) {
      /* initialize values */
      pvr->scale=1.0;
      pvr->add=0.0;
      /* get the scale factor attribute value */
      if (nc_get_att_double(pfi->ncid, pvr->ncvid, pfi->scattr, &val) != NC_NOERR) {
	gaprnt(1,"Warning: Could not retrieve scale factor -- setting to 1.0\n");
	pvr->scale = 1.0;
      }
      else {
	pvr->scale = val;
      }

      /* get add offset if required */
      if (pfi->packflg == 2) {
	/* get the add offset attribute value */
	if (nc_get_att_double(pfi->ncid, pvr->ncvid, pfi->ofattr, &val) != NC_NOERR) {
	  gaprnt(1,"Warning: Could not retrieve add offset -- setting to 0.0\n");
	  pvr->add = 0.0;
	}
	else {
	  pvr->add = val;
	}
      } /* matches if (pfi->packflg == 2) {  */
    }  /* matches if (pfi->packflg) {       */
  }   /* matches if (pvr->ncvid == -999)   */
  return(0); 
#else 
  gaprnt(0,"Reading NetCDF files is not supported in this build\n");
  return(1);  
#endif
}

gaint gancgrid (gadouble *gr, char *gru, gaint id, gaint jd) {
#if USENETCDF == 1
  gaint rc,i,j,grsize,wflag=0;
  gaint xlen,ylen,zlen,tlen,elen;
  gaint x,offset,xx,yy,zz,tt,ee,min,max;
  gaint ifac,jfac,iadd,jadd,pos,ipad,jpad,ilen,jlen;
  gaint xpad,ypad,zpad,tpad,epad,padmin,padmax;
  gaint jbeg,jend,groff,tmpoff,itmp,jtmp,jlimit;
  size_t  start[16],count[16];
  gadouble *grtmp,ulow,uhi;
  char *grutmp;
  gaint oldncopts ;         /* to save and restore setting for automatic error handling */
  
  /* Turn off automatic error handling. */
  ncopts = NC_VERBOSE ;
  oldncopts = ncopts ;
  ncopts = 0;

  rc = gancsetup();
  if (rc) return(rc);

  /* Get the starting point and length for the X dimension */
  if (pgr->dimmin[0] >= 1 && pgr->dimmax[0] <= pfi->dnum[0]) {
    /*  the requested data is within the bounds of the file dimensions */
    xx = pgr->dimmin[0] - 1;
    xlen = pgr->dimmax[0] - pgr->dimmin[0] + 1;
    xpad = 0;
  }
  else {     
    /* the requested data lies outside the file's bounds */
    if (!pfi->wrap) {
      xpad = 0;
      min = pgr->dimmin[0];
      if (min < 1) {                   /* adjust min to be within file limits */
	min = 1;
	xpad = 1 - pgr->dimmin[0];     /* save diff between requested min and file min */
      }
      max = pgr->dimmax[0];
      if (max > pfi->dnum[0]) {        /* adjust max to be within file limits */
	max = pfi->dnum[0];
      }
      xlen = max - min + 1;            /* set length */
      xx = min - 1 ;                   /* set start value */
    }
    else {
      /* read the entire row */
      wflag = 1;
      xx = 0;
      xpad = 0; 
      xlen = pfi->dnum[0];
    }
  }

  /* Get the starting point and length for the Y dimension */
  if (pgr->dimmin[1] >= 1 && pgr->dimmax[1] <= pfi->dnum[1]) {
    /*  the requested data is within the bounds of the file dimensions */
    if (pfi->yrflg) yy = pfi->dnum[1] - pgr->dimmax[1];
    else yy = pgr->dimmin[1] - 1;
    ylen = pgr->dimmax[1] - pgr->dimmin[1] + 1;
    ypad = 0;
  }
  else {     
    /* the requested data lies outside the file's bounds */
    ypad = padmin = padmax = 0;  /* padding for part of requested grid outside boundaries */
    min = pgr->dimmin[1];
    if (min < 1) {                  /* adjust min to be within file limits */
      min = 1;
      padmin = 1 - pgr->dimmin[1];  /* save diff between requested min and file min */
    }
    max = pgr->dimmax[1];
    if (max > pfi->dnum[1]) {                 /* adjust max to be within file limits */
      max = pfi->dnum[1];
      padmax = pgr->dimmax[1] - pfi->dnum[1]; /* save diff between requested max and file max */
    }
    ylen = max - min + 1;                     /* set length */
    if (pfi->yrflg) {         
      yy = pfi->dnum[1] - max;                /* set start value */  
      ypad = ypad + padmax;                   /* set padding */
    }
    else {
      yy = min - 1;
      ypad = ypad + padmin;
    }
  }

  /* Get the starting point and length for the Z dimension */
  if (pgr->dimmin[2] >= 1 && pgr->dimmax[2] <= pfi->dnum[2]) {
    /*  the requested data is within the bounds of the file dimensions */
    if (pfi->zrflg) {
      if (pvr->levels==0) zz = 0;
      else zz = pvr->levels - pgr->dimmax[2];  /* use var nlevs instead of dnum[[2] */
    }
    else zz = pgr->dimmin[2] - 1;
    zlen = pgr->dimmax[2] - pgr->dimmin[2] + 1;
    zpad = 0;
  }
  else {     
    /* the requested data lies outside the file's bounds */
    /* set limits to what's in the file boundaries */
    zpad = padmin = padmax = 0;
    min = pgr->dimmin[2];
    if (min < 1) {
      min = 1;
      padmin = 1 - pgr->dimmin[2];
    }
    max = pgr->dimmax[2];
    if (max > pfi->dnum[2]) {
      max = pfi->dnum[2];
      padmax = pgr->dimmax[2] - pfi->dnum[2];
    }
    zlen = max - min + 1;
    if (pfi->zrflg) {
      if (pvr->levels==0) zz = 0;
      else zz = pvr->levels - max;         
      zpad = zpad + padmax;
    }
    else {
      zz = min - 1;
      zpad = zpad + padmin;
    }
  }

  /* Get the starting point and length for the T dimension */
  if (pgr->dimmin[3] >= 1 && pgr->dimmax[3] <= pfi->dnum[3]) {
    /*  the requested data is within the bounds of the file dimensions */
    tt = pgr->dimmin[3] - 1;
    tlen = pgr->dimmax[3] - pgr->dimmin[3] + 1;
    tpad = 0;
  }
  else {     
    /* the requested data lies outside the file's bounds */
    tpad = 0;
    min = pgr->dimmin[3];
    if (min < 1) {                /* adjust min to be within file limits */
      min = 1;
      tpad = 1 - pgr->dimmin[3];  /* save diff between requested min and file min */
    }
    max = pgr->dimmax[3];
    if (max > pfi->dnum[3]) {     /* adjust max to be within file limits */
      max = pfi->dnum[3];
    }
    tlen = max - min + 1;         /* set length */
    tt = min - 1;                 /* set start value */
  }

  /* Get the starting point and length for the E dimension */
  if (pgr->dimmin[4] >= 1 && pgr->dimmax[4] <= pfi->dnum[4]) {
    /*  the requested data is within the bounds of the file dimensions */
    ee = pgr->dimmin[4] - 1;
    elen = pgr->dimmax[4] - pgr->dimmin[4] + 1;
    epad = 0;
  }
  else {     
    /* the requested data lies outside the file's bounds */
    epad = 0;
    min = pgr->dimmin[4];
    if (min < 1) {                /* adjust min to be within file limits */
      min = 1;
      epad = 1 - pgr->dimmin[4];  /* diff between requested min and file min */
    }
    max = pgr->dimmax[4];
    if (max > pfi->dnum[4]) {     /* adjust max to be within file limits */
      max = pfi->dnum[4];
    }
    elen = max - min + 1;         /* set length */
    ee = min - 1;                 /* set start value */
  }

  /* size of the grid to be read */
  grsize = xlen * ylen * zlen * tlen * elen;

  /* allocate memory for temporary grids to hold data 
     before being placed properly in requested block */
  grtmp = (gadouble *)galloc(grsize * sizeof(gadouble),"grtmp");
  if (grtmp==NULL) {
    gaprnt (0,"gancgrid error: unable to allocate memory for grtmp grid storage \n");
    sprintf(pout,"  grid size = xlen * ylen * zlen * tlen * elen = %d * %d * %d * %d * %d\n",
	    xlen,ylen,zlen,tlen,elen);
    gaprnt (0,pout);
    return (1);
  }
  grutmp = (char *)galloc(grsize * sizeof(char),"grutmp");
  if (grutmp==NULL) {
    gaprnt (0,"gancgrid error: unable to allocate memory for grutmp grid storage \n");
    sprintf(pout,"  grid size = xlen * ylen * zlen * tlen * elen = %d * %d * %d * %d * %d\n",
	    xlen,ylen,zlen,tlen,elen);
    gaprnt (0,pout);
    return (1);
  }

  /* Set up the start and count arrays  */
  /* The units values provided for each variable indicate the mapping 
     of the netcdf variable shape into the grads dimensions */
/*   printf("JMA(gancgrid): id,jd = %d %d\n",id,jd); */
  for (i=0; i<16; i++) {
    start[i] = -999;
    count[i] = -999;
    if (pvr->units[i] == -100) { start[i] = xx; count[i] = xlen; }
    if (pvr->units[i] == -101) { start[i] = yy; count[i] = ylen; }
    if (pvr->units[i] == -102) { start[i] = zz; count[i] = zlen; }
    if (pvr->units[i] == -103) { start[i] = tt; count[i] = tlen; }
    if (pvr->units[i] == -104) { start[i] = ee; count[i] = elen; }
    if (pvr->units[i] >=0) { start[i] = pvr->units[i];  count[i] = 1; }
/*     printf("JMA(gancgrid): i,units,start,count = %2d %4.0f %4d %4d\n", */
/* 	   i,pvr->units[i],(gaint)start[i],(gaint)count[i]); */
  }
  
  /* Now we are ready to do the I/O  */
  rc = nc_get_vara_double(pfi->ncid, pvr->ncvid, start, count, grtmp);
  if (rc != NC_NOERR) {
    sprintf(pout,"gancgrid error: nc_get_vara_double failed; %s\n",nc_strerror(rc));
    gaprnt(0,pout);
    ncopts = oldncopts ;
    return (1);
  }

  /* Set missing data mask values and then unpack grid data if necessary */
  /* use the gavar undef to set the fuzzy test limits */
  /* If gavar undef equals zero, change it to 1/EPSILON */
  if (dequal(pvr->undef, 0.0, 1e-08)==0) {   
    ulow = 1e-5; 
  } 
  else {
    ulow = fabs(pvr->undef/EPSILON);   
  }
  uhi  = pvr->undef + ulow;
  ulow = pvr->undef - ulow;
  /* now set the gagrid undef equal to the gafile undef */
  pgr->undef = pfi->undef;           

  /* Do the fuzzy test for undef values before unpacking */
  for (i=0;i<grsize;i++) {
    /* set the missing data to the gafile undef */
    if (*(grtmp+i) >= ulow &&  *(grtmp+i) <= uhi ) {
	*(grutmp+i) = 0;
	*(grtmp+i) = pfi->undef;   
    }
    else {
	/* Data is not missing */ 
	*(grutmp+i) = 1;
	/* unpack with scale and offset if necessary */
	if (pfi->packflg) {
	  *(grtmp+i) = *(grtmp+i)*pvr->scale + pvr->add; 
	}
    }
  }
 
  if ((id==1 && pfi->yrflg) || (id==2 && pfi->zrflg)) {  /* the i-dimension is reversed */
    ifac = -1;
    iadd = pgr->isiz - 1;
  }
  else {
    ifac = 1;
    iadd = 0;
  }
  if ((jd==1 && pfi->yrflg) || (jd==2 && pfi->zrflg)) {  /* the j-dimension is reversed */
    jfac = -1;
    jadd = pgr->jsiz - 1;
  }
  else {
    jfac = 1;
    jadd = 0;
  }
  ipad = jpad = ilen = jlen = 0;
  if      (id==-1) { ipad = 0;    ilen = 1;    }
  else if (id==0)  { ipad = xpad; ilen = xlen; }
  else if (id==1)  { ipad = ypad; ilen = ylen; }
  else if (id==2)  { ipad = zpad; ilen = zlen; }
  else if (id==3)  { ipad = tpad; ilen = tlen; }
  else if (id==4)  { ipad = epad; ilen = elen; }

  if      (jd==-1) { jpad = 0;    jlen = 1;    }
  else if (jd==0)  { jpad = xpad; jlen = xlen; }
  else if (jd==1)  { jpad = ypad; jlen = ylen; }
  else if (jd==2)  { jpad = zpad; jlen = zlen; }
  else if (jd==3)  { jpad = tpad; jlen = tlen; }
  else if (jd==4)  { jpad = epad; jlen = elen; }

  /* initialize the result grid with missing flags */
  for (i=0; i<pgr->isiz*pgr->jsiz; i++) gru[i] = 0;

  /* copy each element in the tmp grid into the right place in the result grid */
  if (!wflag) {
    pos=0;
    for (j=0; j<jlen; j++) {
      for (i=0; i<ilen; i++) {
	offset = (jadd+(jfac*(j+jpad)))*pgr->isiz + (iadd + (ifac*(i+ipad)));
	gr[offset]  = grtmp[pos];
	gru[offset] = grutmp[pos];
	pos++;
      }
    }
  }
  else {
    /* jbeg and jend are limits of tmp grid within user requested grid */
    if (jd==-1) {
      jbeg = jadd; 
      jend = jbeg;
    }
    else {
      jbeg = jadd+(jfac*(0+jpad));
      jend = jadd+(jfac*((jlen-1)+jpad));
    }
    /* i,j are result grid coordinates */
    /* itmp,jtmp are tmp grid coordinates */
    jtmp=0; 
    j = jbeg;
    jlimit = jend+=jfac; /* this is jend +/-1, the limit for the while loop below */
    while (j != jlimit) {
      /* x are user-requested limits */
      i=0;
      for (x=pgr->dimmin[0]; x<=pgr->dimmax[0]; x++,i++) {   
	/* groff is where point lies inside requested grid */
	groff = j*pgr->isiz + i;       
	/* figure out where x index lies inside tmp grid */
	itmp=x;
	while (itmp<1) itmp = itmp + pfi->dnum[0];
	while (itmp>pfi->dnum[0]) itmp = itmp-(pfi->dnum[0]);  
	/* tmpoff is where this point lies in the grtmp grid */
	tmpoff = jtmp*pfi->dnum[0] + (itmp-1);
	gr[groff]  = grtmp[tmpoff];
	gru[groff] = grutmp[tmpoff];
      }
      jtmp++;
      j+=jfac;
    }
  }
  if (grtmp)  gree(grtmp,"f121");
  if (grutmp) gree(grutmp,"f122");

  ncopts = oldncopts ;
  return(0);
#else 
  gaprnt(0,"Reading NetCDF files is not supported in this build\n");
  return(1);  
#endif
}

/* Read a row varying in the X direction from a netcdf grid */
gaint gancrow (gaint x, gaint y, gaint z, gaint t, gaint e, gaint len, gadouble *gr, char *gru) {
#if USENETCDF == 1
  gaint rc,i,yy,zz;
  size_t  start[16],count[16];
  gadouble   ulow,uhi;
  gaint oldncopts ;         /* to save and restore setting for automatic error handling */
  
  /* Turn off automatic error handling. */
  ncopts = NC_VERBOSE ;
  oldncopts = ncopts ;
  ncopts = 0;

  /* Change the Y indexes if yrev flag is set */
  if (pfi->yrflg) {
    /* one day we might encounter a pre-projected file written upside down... */
    if (pfi->ppflag)             
      yy = pfi->ppjsiz - y;
    else
      yy = pfi->dnum[1] - y;
  }
  else {
    yy = y-1;
  }
  /* Change the Z indexes if zrev flag is set */
  if (pfi->zrflg) {
    if (pvr->levels==0) {
      zz=0;
    }
    else {
      zz = pvr->levels-z;
    }
  } 
  else {
    zz = z-1;
  }

  /* Set up the start and count array.  The units values
     provided for each variable indicate the mapping of the 
     netcdf variable shape into the grads dimensions */
  for (i=0; i<16; i++) {
    start[i] = -999;
    count[i] = -999;
    if (pvr->units[i] == -100) { start[i] = x-1; count[i] = len; }
    if (pvr->units[i] == -101) { start[i] = yy;  count[i] = 1; }
    if (pvr->units[i] == -102) { start[i] = zz;  count[i] = 1; }
    if (pvr->units[i] == -103) { start[i] = t-1; count[i] = 1; }
    if (pvr->units[i] == -104) { start[i] = e-1; count[i] = 1; }
    if (pvr->units[i] >=0) { start[i] = pvr->units[i];  count[i] = 1; }
  }

  /* Now we are ready to do the I/O  */
  rc = nc_get_vara_double(pfi->ncid, pvr->ncvid, start, count, gr);
  if (rc != NC_NOERR) {
    sprintf(pout,"NetCDF Error (gancrow, nc_get_vara_double): %s\n",nc_strerror(rc));
    gaprnt(0,pout);
    ncopts = oldncopts ;
    return (1);
  }

  /* Set missing data values to gafile undef and then unpack if necessary */
  /* use the gavar undef to set the fuzzy test limits */
  /* If gavar undef equals zero, change it to 1/EPSILON */
  if (dequal(pvr->undef, 0.0, 1e-08)==0) {   
    ulow = 1e-5;
  } 
  else {
    ulow = fabs(pvr->undef/EPSILON);   
  }
  uhi  = pvr->undef + ulow;
  ulow = pvr->undef - ulow;
  /* set the gagrid undef equal to the gafile undef */
  pgr->undef = pfi->undef;           
  
  /* Do the fuzzy test for undef values before unpacking */
  for (i=0;i<len;i++) {
    
    /* If data are missing, set the undef mask to zero */
    if (*(gr+i) >= ulow && *(gr+i) <= uhi) {
      *(gru+i) = 0;
    }
    else {
      /* Data is not missing, so unpack with scale and offset if necessary */
      *(gru+i) = 1; 
      if (pfi->packflg) {
	*(gr+i) = *(gr+i)*pvr->scale + pvr->add; 
      }
    }
  }

  ncopts = oldncopts ;
  return(0);
#else 
  gaprnt(0,"Reading NetCDF files is not supported in this build\n");
  return(1);  
#endif
}


/* Read a row varying in the X direction from an HDF-SDS grid */
gaint gahrow (gaint x, gaint y, gaint z, gaint t, gaint e, gaint len, gadouble *gr, char *gru) {
#if USEHDF == 1
gaint rc,i,yy,zz;
int32  start[16],count[16];
int32  sd_id, v_id, sds_id;
int32  data_dtype, n_atts, rank, dim_sizes[MAX_VAR_DIMS];
gadouble  val,ulow,uhi;

int8    *bval;
uint8   *ubval;
int16   *sval;
uint16  *usval;
int32   *ival;
uint32  *uival;
float32 *fval;

  /* Get the vid if we haven't already done that for this file */
  if (pvr->sdvid == -888) return(1);  /* already tried and failed */

  sd_id = pfi->sdid; 
  if (pvr->sdvid == -999) {

    /* Get the variable index number from the variable name */
    if (pvr->longnm[0] != '\0') {
      v_id = SDnametoindex(sd_id, pvr->longnm);
    }
    else {
      v_id = SDnametoindex(sd_id, pvr->abbrv);
    }
    if (v_id==FAIL) {
      pvr->sdvid = -888;
      if (pvr->longnm[0] != '\0') {
	sprintf(pout,"Error: Variable %s not in HDF-SDS file\n",pvr->longnm);
      }
      else {
	sprintf(pout,"Error: Variable %s not in HDF-SDS file\n",pvr->abbrv);
      }
      gaprnt(0,pout);
      return (1);
    }
    pvr->sdvid = v_id; 

    /* If undef attribute name is used, get the undef value */
    if (pfi->undefattrflg) {
      /* Select the variable (get sds_id) */
      v_id = pvr->sdvid;
      sds_id = SDselect(sd_id,v_id);
      if (sds_id==FAIL) {
	if (pvr->longnm[0] != '\0') {
	  sprintf(pout,"Error: SDselect failed for %s \n",pvr->longnm);
	}
	else {
	  sprintf(pout,"Error: SDselect failed for %s \n",pvr->abbrv);
	}
	gaprnt(0,pout);
	return (1);
      }
      /* Retrieve the HDF undef attribute value */
      if (hdfattr(sds_id, pfi->undefattr, &val) != 0) {
	sprintf(pout,"Warning: Could not retrieve undef attribute \"%s\" -- using %g instead\n",
		pfi->undefattr,pfi->undef);
	gaprnt(1,pout);
	pvr->undef = pfi->undef;
      }
      else {
	pvr->undef = val;
      }
    }
    /* If undef attribute name is not given, copy the file-wide undef */
    else {
      pvr->undef = pfi->undef;
    }
 

    /* If data are packed, get the scale factor and offset attribute values */
    if (pfi->packflg) {
      /* initialize values */
      pvr->scale=1.0;
      pvr->add=0.0;

      /* Select the variable (get sds_id) */
      v_id = pvr->sdvid;
      sds_id = SDselect(sd_id,v_id);
      if (sds_id==FAIL) {
	if (pvr->longnm[0] != '\0') {
	  sprintf(pout,"Error: SDselect failed for %s \n",pvr->longnm);
	}
	else {
	  sprintf(pout,"Error: SDselect failed for %s \n",pvr->abbrv);
	}
	gaprnt(0,pout);
	return (1);
      }
      /* Retrieve the scale factor attribute value */
      if (hdfattr(sds_id, pfi->scattr, &pvr->scale) != 0) {
	sprintf(pout,"Warning: Could not retrieve \"%s\" -- setting to 1.0\n",pfi->scattr);
        gaprnt(1,pout);
	pvr->scale = 1.0;
      }
      /* Retrieve the add offset attribute value if required */
      if (pfi->packflg == 2) {
	if (hdfattr(sds_id, pfi->ofattr, &pvr->add) != 0) {
	  sprintf(pout,"Warning: Could not retrieve \"%s\" -- setting to 0.0\n",pfi->ofattr);
	  gaprnt(1,pout);
	  pvr->add = 0.0;
	}
      }
    }
  }

  /* Select the variable (get sds_id) */
  v_id = pvr->sdvid;
  sds_id = SDselect(sd_id,v_id);

  if (sds_id==FAIL) {
    if (pvr->longnm[0] != '\0') {
      sprintf (pout,"Error: SDselect failed for %s \n",pvr->longnm);
    }
    else {
      sprintf (pout,"Error: SDselect failed for %s \n",pvr->abbrv);
    }
    gaprnt(0,pout);
    return (1);
  }

  /* Change the Y indexes if yrev flag is set */
  if (pfi->yrflg) {
    /* one day we might encounter a pre-projected file written upside down... */
    if (pfi->ppflag)             
      yy = pfi->ppjsiz - y;
    else
      yy = pfi->dnum[1] - y;
  }
  else {
    yy = y-1;
  }

  /* Change the Z indexes if zrev flag is set */
  if (pfi->zrflg) {
    if (pvr->levels==0) {
      zz=0;
    }
    else {
      zz = pvr->levels-z;
    }
  } 
  else {
    zz = z-1;
  }

  /* Set up the start and count array.  The units records in the
     descriptor file for each variable indicate the mapping of the 
     hdf-sds variable shape into the grads dimensions */
  for (i=0; i<16; i++) {
    start[i] = -999;
    count[i] = -999;
    if (pvr->units[i] == -100) { start[i] = x-1; count[i] = len;}
    if (pvr->units[i] == -101) { start[i] = yy;  count[i] = 1; }
    if (pvr->units[i] == -102) { start[i] = zz;  count[i] = 1; }
    if (pvr->units[i] == -103) { start[i] = t-1; count[i] = 1; }
    if (pvr->units[i] == -104) { start[i] = e-1; count[i] = 1; }
    if (pvr->units[i] >= 0) { start[i] = pvr->units[i];   count[i] = 1; }
  }

  /* Get the data type */
  if (pvr->longnm[0] != '\0') {
    rc = SDgetinfo(sds_id, pvr->longnm, &rank, dim_sizes, &data_dtype, &n_atts);
  }
  else {
    rc = SDgetinfo(sds_id, pvr->abbrv,  &rank, dim_sizes, &data_dtype, &n_atts);
  }

  /* Data types that are handled are 8-bit unsigned ints (uint8), shorts (int16), 
     ints (int32) and float. shorts and ints are converted to float. 
     Unpacking done after I/O is finished  */
  switch (data_dtype)
  {
    case (DFNT_INT8):   /* definition value 20 */
      bval = (int8 *)galloc(len * sizeof (int8),"bval3");
      if (bval==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype INT8\n");
	return(1);
      }
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)bval) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype INT8\n");
	return(1);
      } 
      else {
	for (i=0; i<len; i++) gr[i] = (gadouble)bval[i];  /* Cast int8 to gadouble */
      }
      gree(bval,"f126");
      break;

    case (DFNT_UINT8):   /* definition value 21 */
      ubval = (uint8 *)galloc(len * sizeof (uint8),"ubval");
      if (ubval==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype UINT8\n");
	return(1);
      }
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)ubval) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype UINT8\n");
	return(1);
      } 
      else {
	for (i=0; i<len; i++) gr[i] = (gadouble)ubval[i];  /* Cast uint8 to gadouble */
      }
      gree(ubval,"f127");
      break;

    case (DFNT_INT16):    /* definition value 22 */
      sval = (int16 *)galloc(len * sizeof(int16),"sval3");
      if (sval==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype INT16\n");
	return(1);
      }
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)sval) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype INT16\n");
	return(1);
      }
      else {
	for (i=0; i<len; i++) gr[i] = (gadouble)sval[i];  /* Cast int16 to gadouble */
      }
      gree(sval,"f128");
      break;

    case (DFNT_UINT16):   /* definition value 23 */
      usval = (uint16 *)galloc(len * sizeof (uint16),"usval");
      if (usval==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype UINT16\n");
	return(1);
      }
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)usval) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype UINT16\n");
	return(1);
      } 
      else {
	for (i=0; i<len; i++) gr[i] = (gadouble)usval[i];  /* Cast uint16 to gadouble */
      }
      gree(usval,"f129");
      break;

    case (DFNT_INT32):    /* definition value 24 */
      ival = (int32 *)galloc(len * sizeof (int32),"ival3");
      if (ival==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype INT32\n");
	return(1);
      }
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)ival) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype INT32\n");
	return(1);
      } 
      else {
	for (i=0; i<len; i++) gr[i] = (gadouble)ival[i];  /* Cast int32 to gadouble */
      }
      gree(ival,"f130");
      break;

    case (DFNT_UINT32):   /* definition value 25 */
      uival = (uint32 *)galloc(len * sizeof (uint32),"uival");
      if (uival==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype UINT32\n");
	return(1);
      }
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)uival) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype UINT32\n");
	return(1);
      } 
      else {
	for (i=0; i<len; i++) gr[i] = (gadouble)uival[i];  /* Cast uint32 to gadouble */
      }
      gree(uival,"f131");
      break;

    case (DFNT_FLOAT32):  /* definition value  5 */
      fval = (float32 *)galloc(len * sizeof (float32),"fval");
      if (fval==NULL) {
	gaprnt(0,"HDF-SDS Error: unable to allocate memory for dtype float32\n");
	return(1);
      }
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)fval) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype float32\n");
	return(1);
      } 
      else {
	for (i=0; i<len; i++) gr[i] = (gadouble)fval[i];  /* Cast uint32 to gadouble */
      }
      gree(fval,"f131");
      break;


    case (DFNT_FLOAT64):  /* definition value  6 */
      if (SDreaddata(sds_id, start, NULL, count, (VOIDP *)gr) != 0) {
	gaprnt(0,"HDF-SDS Read Error for dtype FLOAT64\n");
	return(1);
      } 
      break;

    default:
      sprintf(pout,"HDF-SDS Error: Data type %d not handled\n", data_dtype);
      gaprnt(0,pout);
      return(1);
  };

  /* Set missing data values to exact value if specified */
  /* Use the gavar undef to set the fuzzy test limits */
  /* If gavar undef equals zero, change it to 1/EPSILON */
  if (dequal(pvr->undef, 0.0, 1.0e-08)==0) {   
    ulow = 1e-5;
  } 
  else {
    ulow = fabs(pvr->undef/EPSILON);   
  }
  uhi  = pvr->undef + ulow;
  ulow = pvr->undef - ulow;
  /* set the gagrid undef equal to the gafile undef */
  pgr->undef = pfi->undef;           
  
  /* Do the fuzzy test for undef values before unpacking */
  for (i=0;i<len;i++) {
    /* set the missing data to the gafile undef */
    if (*(gr+i) >= ulow && *(gr+i) <= uhi) {
      *(gru+i) = 0;
    }
    else {
      /* Data is not missing */ 
      *(gru+i) = 1;
      /* unpack with scale and offset if necessary */
      if (pfi->packflg) {
	*(gr+i) = *(gr+i)*pvr->scale + pvr->add; 
      }
    }
  }

return (0);

#endif
  gaprnt(0,"Reading HDF-SDS files is not supported in this build\n");
  return(1);
}


/* Retrieves a non-character HDF-SDS Attribute. */

gaint hdfattr(gaint sds_id, char *attr_name, gadouble *value) {
#if USEHDF == 1
int32   attr_index, attr_dtype, attr_count;
uint8   *battr_val;
int16   *sattr_val;
uint16  *usattr_val;
int32   *iattr_val;
uint32  *uiattr_val;
float32 *fattr_val;
float64 *dattr_val;

  /* Get the attribute index number from its name */
  attr_index = SDfindattr(sds_id, attr_name);
  if (attr_index == -1) {
    sprintf(pout,"Warning: HDF attribute named \"%s\" does not exist\n",attr_name);
    gaprnt(1,pout);
    return(1);
  } 

  /* Get info about the attribute, make sure there's only one value */
  if (SDattrinfo(sds_id, attr_index, attr_name, &attr_dtype, &attr_count) == -1) {
    gaprnt(1,"Warning: SDattrinfo failed\n");
    return(1);
  } 
  else {
    if (attr_count != 1) {
      sprintf(pout,"Warning: HDF attribute named \"%s\" has more than one value\n",attr_name);
      gaprnt(1,pout);
      return(1);
    }

    /* Read the attribute value */
    switch (attr_dtype)
      {
      case (DFNT_UINT8):    /* definition value 21 */
	battr_val = galloc (attr_count * sizeof (uint8),"battrval");
	if (SDreadattr(sds_id, attr_index, battr_val) == -1) {
	  gaprnt(1,"Warning: SDreadattr failed for attribute type UINT8\n");
	  return(1);
	}
	else {
	  *value = *battr_val;
	}
	gree(battr_val,"f132");
	break;
	
      case (DFNT_INT16):    /* definition value 22 */
	sattr_val = galloc (attr_count * sizeof (int16),"sattrval");
	if (SDreadattr(sds_id, attr_index, sattr_val) == -1) {
	  gaprnt(1,"Warning: SDreadattr failed for attribute type INT16\n");
	  return(1);
	}
	else {
	  *value = *sattr_val;
	}
	gree(sattr_val,"f133");
	break;
	
      case (DFNT_UINT16):   /* definition value 23 */
	usattr_val = galloc (attr_count * sizeof (uint16),"usattrval");
	if (SDreadattr(sds_id, attr_index, usattr_val) == -1)  {
	  gaprnt(1,"Warning: SDreadattr failed for attribute type UINT16\n");
	  return(1);
	}
	else {
	  *value = *usattr_val;
	}
	gree(usattr_val,"f134");
	break;
	
      case (DFNT_INT32):    /* definition value 24 */
	iattr_val = galloc (attr_count * sizeof (int32),"iattrval");
	if (SDreadattr(sds_id, attr_index, iattr_val) == -1) {
	  gaprnt(1,"Warning: SDreadattr failed for attribute type INT32\n");
	  return(1);
	}
	else {
	  *value = *iattr_val;
	}
	gree(iattr_val,"f135");
	break;
	
      case (DFNT_UINT32):   /* definition value 25 */
	uiattr_val = galloc (attr_count * sizeof (uint32),"uiattrval");
	if (SDreadattr(sds_id, attr_index, uiattr_val) == -1) {
	  gaprnt(1,"Warning: SDreadattr failed for attribute type UINT32\n");
	  return(1);
	}
	else {
	  *value = *uiattr_val;
	}
	gree(uiattr_val,"f136");
	break;
	
      case (DFNT_FLOAT32):  /* definition value  5 */
	fattr_val = galloc (attr_count * sizeof (float32),"fattrval");
	if (SDreadattr(sds_id, attr_index, fattr_val) == -1) {
	  gaprnt(1,"Warning: SDreadattr failed for attribute type FLOAT32\n");
	  return(1);
	}
	else {
	  *value = *fattr_val;
	}
	gree(fattr_val,"f137");
	break;
	
      case (DFNT_FLOAT64):  /* definition value  6 */
	dattr_val = galloc (attr_count * sizeof (float64),"dattrval");
	if (SDreadattr(sds_id, attr_index, dattr_val) == -1) {
	  gaprnt(1,"Warning: SDreadattr failed for attribute type FLOAT64\n");
	  return(1);
	}
	else {
	  *value = *dattr_val;
	}
	gree(dattr_val,"f138");
	break;
	
      default:
	sprintf(pout,"Warning: HDF Attribute \"%s\" is not a numeric data type (%d)\n", 
		attr_name, attr_dtype);
	gaprnt(1,pout);
	return(1);
      };
  }
  return(0);

#endif
  gaprnt(0,"Reading HDF-SDS files is not supported in this build\n");
  return(1);
}


/* Subroutine to print out NetCDF attributes */
gaint ncpattrs(gaint ncid, char *varnam, char *abbrv, gaint hdrflg, gaint fnum, char* ftit) {
#if USENETCDF == 1
  gaint attr_index, attr_count, rc, i, varid, n_atts;
  char attr_name[MAX_NC_NAME];
  nc_type attr_dtype;
  char   *battr_val;
  char   *cattr_val;
  short  *sattr_val;
  long   *iattr_val;
  gafloat  *fattr_val;
  gadouble *dattr_val;
  gaint error=0;

/* Get the variable id and number of attributes */
  if (cmpwrd("global",abbrv)) {
    varid = NC_GLOBAL;
    rc = nc_inq_natts(ncid,&n_atts);
    if (rc != NC_NOERR) error=1;
  }
  else {
    rc = nc_inq_varid(ncid, varnam, &varid);
    if (rc != NC_NOERR) error=1;
    if (!error) {
      rc = nc_inq_varnatts(ncid, varid, &n_atts);
      if (rc != NC_NOERR) error=1;
    }
  }

  /* Print out the header */
  if (!error) {
    if (hdrflg) {
      if (n_atts > 0) {
	sprintf(pout,"Native Attributes for File %i : %s \n",fnum,ftit);
	gaprnt(2,pout);
      }
    }
  }
  else {
    return(0);  /* zero attributes printed */
  }
  
  /* Loop through list of attributes, print the name of each one */ 
  for (attr_index = 0 ; attr_index < n_atts ; attr_index++) {

    /* Get current attribute's name */
    if (nc_inq_attname(ncid, varid, attr_index, attr_name) == -1) {
      sprintf(pout,"nc_inq_attname failed for variable %s, attribute number %d\n", abbrv, attr_index);
      gaprnt(2,pout);
    }
    else {
      /* Get current attribute's data type and length */
      if (nc_inq_att(ncid, varid, attr_name, &attr_dtype, (size_t*)&attr_count) == -1) {
	sprintf(pout,"nc_inq_att failed for variable %s, attribute number %d\n", abbrv, attr_index);
	gaprnt(2,pout);
      }
      else {
        if (attr_count>0) {
	  /* Retrieve and print out the attribute */
	  switch (attr_dtype) 
	    {
	    case (NC_BYTE):
	      battr_val = (char *) galloc ((attr_count+1) * sizeof (NC_BYTE),"battrval1");
	      if (nc_get_att_schar(ncid, varid, attr_name, (signed char*)battr_val) == -1) {
		gaprnt(2,"nc_get_att_schar failed for type NC_BYTE\n"); 
	      }
	      else {
		gaprnt(2,abbrv); 
		gaprnt(2," Byte "); 
		gaprnt(2,attr_name); 
		gaprnt(2," ");
		for (i=0; i<attr_count; i++) {
		  sprintf(pout,"%d ", (gaint)(battr_val[i])); 
		  gaprnt(2,pout);
		}
		gaprnt(2,"\n");
	      }
	      gree(battr_val,"f139");
	      break;
	    case (NC_CHAR):
	      cattr_val = (char *) galloc ((attr_count+1) * sizeof (NC_CHAR),"cattrval1");
	      if (nc_get_att_text(ncid, varid, attr_name, cattr_val) == -1) {
		gaprnt(2,"nc_get_att_text failed for type NC_CHAR\n"); 
	      }
	      else {
		cattr_val[attr_count]='\0';
		gaprnt(2,abbrv); 
		gaprnt(2," String ");
		gaprnt(2,attr_name); 
		gaprnt(2," ");
		prntwrap(abbrv, attr_name, cattr_val);
	      }
	      gree(cattr_val,"f140");
	      break;
	    case (NC_SHORT):
	      sattr_val = (short *) galloc (attr_count * sizeof (NC_SHORT),"sattrval1");
	      if (nc_get_att_short(ncid, varid, attr_name, sattr_val) == -1) {
		gaprnt(2,"nc_get_att_short failed for type NC_SHORT\n"); 
	      }
	      else {
		gaprnt(2,abbrv); 
		gaprnt(2," Int16 "); 
		gaprnt(2,attr_name); 
		gaprnt(2," ");
		for (i=0; i<attr_count; i++) {
		  sprintf(pout,"%d", (gaint)(sattr_val[i])); 
		  gaprnt(2,pout);
		  if (i<attr_count-1) gaprnt(2,",");
		}
		gaprnt(2,"\n");
	      }
	      gree(sattr_val,"f141");
	      break;
	    case (NC_LONG):
	      iattr_val = (long *) galloc (attr_count * sizeof (NC_LONG),"iattrval1");
	      if (nc_get_att_long(ncid, varid, attr_name, iattr_val) == -1) {
		gaprnt(2,"nc_get_att_long failed for type NC_LONG\n"); 
	      }
	      else {
		gaprnt(2,abbrv); 
		gaprnt(2," Int32 "); 
		gaprnt(2,attr_name); 
		gaprnt(2," ");
		for (i=0; i<attr_count; i++) {
		  sprintf(pout,"%ld", iattr_val[i]); 
		  gaprnt(2,pout);
		  if (i<attr_count-1) gaprnt(2,",");
		}
		gaprnt(2,"\n");
	      }
	      gree(iattr_val,"f142");
	      break;
	    case (NC_FLOAT):
	      fattr_val = (gafloat *) galloc (attr_count * sizeof (gafloat),"fattrval1");
	      if (nc_get_att_float(ncid, varid, attr_name, fattr_val) == -1) {
		gaprnt(2,"nc_get_att_float failed for type NC_FLOAT\n"); 
	      }
	      else {
		gaprnt(2,abbrv); 
		gaprnt(2," Float32 "); 
		gaprnt(2,attr_name); 
		gaprnt(2," ");
		for (i=0; i<attr_count; i++) {
		  sprintf(pout,"%g", fattr_val[i]);
		  gaprnt(2,pout);
		  if (i<attr_count-1) gaprnt(2,",");
		}
		gaprnt(2,"\n");
	      }
	      gree(fattr_val,"f143");
	      break;
	    case (NC_DOUBLE): 
	      dattr_val = (gadouble *) galloc (attr_count * sizeof (gadouble),"dattrval1");
	      if (nc_get_att_double(ncid, varid, attr_name, dattr_val) == -1) {
		gaprnt(2,"nc_get_att_double failed for type NC_FLOAT\n"); 
	      }
	      else {
		gaprnt(2,abbrv); 
		gaprnt(2," Float64 "); 
		gaprnt(2,attr_name); 
		gaprnt(2," ");
		for (i=0; i<attr_count; i++) {
		  sprintf(pout,"%g", dattr_val[i]); 
		  gaprnt(2,pout);
		  if (i<attr_count-1) gaprnt(2,",");
		}
		gaprnt(2,"\n");
	      }
	      gree(dattr_val,"f144");
	      break;
	    default:
	      sprintf(pout,"Failed to retrieve attribute %d of type %d \n", attr_index, attr_dtype);
	      gaprnt(2,pout);
	    };
	} /* end of if statement for attr_count >0 */
      } /* end of if-else statement for ncattinq */
    } /* end of if-else statement for ncattname */
  } /* end of for loop on attr_index */
  return(n_atts);
#endif
  gaprnt(0,"Reading NetCDF attributes is not supported in this build\n");
  return(1);
}


/* Subroutine to print out HDF attributes */
gaint hdfpattrs(gaint sdid, char *varname, char *abbrv, gaint hdrflg, gaint fnum, char* ftit) {
#if USEHDF == 1
  gaint     attr_index, rc, i;
  char    attr_name[MAX_NC_NAME];
  int32   attr_dtype, attr_count;
  char8   *cattr_val=NULL;
  uchar8  *ucattr_val=NULL;
  int8    *icattr_val=NULL;
  uint8   *uicattr_val=NULL;
  int16   *sattr_val=NULL;
  uint16  *usattr_val=NULL;
  int32   *iattr_val=NULL;
  uint32  *uiattr_val=NULL;
  float32 *fattr_val=NULL;
  float64 *dattr_val=NULL;
  gaint error=0;
  char name[MAX_NC_NAME];
  int32 sds_id, n_atts, n_dsets, rank, type, dim_sizes[4];


  /* Get the dataset id and number of attributes */
  if (cmpwrd("global",abbrv)) {
    sds_id = sdid;
    rc = SDfileinfo(sdid, &n_dsets, &n_atts);
    if (rc == -1) error=1;
  }
  else {
    sds_id = SDnametoindex(sdid, varname);
    if (sds_id == -1) error=1;
    if (!error) {
      sds_id = SDselect(sdid,sds_id);
      rc = SDgetinfo(sds_id, name, &rank, dim_sizes, &type, &n_atts);
      if (rc == -1) error=1;
    }
  }
  /* Print out the header */
  if (!error) {
    if (hdrflg) {
      if (n_atts > 0) {
	sprintf(pout,"Native Attributes for File %i : %s \n",fnum,ftit);
	gaprnt(2,pout);
      }
    }
  }
  else {
    return(0);  /* zero attributes printed */
  }

  /* Loop through list of attributes, print the name of each one */ 
  for (attr_index = 0 ; attr_index < n_atts ; attr_index++) {
	 
    /* Get info about the current attribute and then print out Name, Type, and Value */
    if (SDattrinfo(sds_id, attr_index, attr_name, &attr_dtype, &attr_count) == -1) {
      sprintf(pout,"SDattrinfo failed for variable %s, attribute number %d\n", abbrv, attr_index);
      gaprnt(2,pout);
    }
    else {
      switch (attr_dtype) 
	{
	case (DFNT_CHAR8):    /* definition value 4 */
	  cattr_val = (char8*)galloc ((attr_count+1) * sizeof (char8),"cattrval2");
	  if (SDreadattr(sds_id, attr_index, cattr_val) == -1) {
	    gaprnt(2,"SDreadattr failed for type CHAR8\n"); 
	  }
	  else {
	    cattr_val[attr_count]='\0';
	    gaprnt(2,abbrv); 
	    gaprnt(2," String ");
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    prntwrap(abbrv, attr_name, cattr_val);
	  }
	  gree(cattr_val,"f145");
	  break;
	case (DFNT_UCHAR8):   /* definition value 3 */
	  ucattr_val = (uchar8*)galloc ((attr_count+1) * sizeof (uchar8),"ucattrval");
	  if (SDreadattr(sds_id, attr_index, ucattr_val) == -1) { 
	    gaprnt(2,"SDreadattr failed for type UCHAR8\n"); 
	  }
	  else {
	    ucattr_val[attr_count]='\0';
	    gaprnt(2,abbrv); 
	    gaprnt(2," String ");
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    prntwrap(abbrv, attr_name, (char*)ucattr_val);
	  }
	  gree(ucattr_val,"f146");
	  break;
	case (DFNT_INT8):     /* definition value 20 */
	  icattr_val = (int8*)galloc ((attr_count+1) * sizeof (int8),"icattrval2");
	  if (SDreadattr(sds_id, attr_index, icattr_val) == -1) {
	    gaprnt(2,"SDreadattr failed for type INT8\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," Byte "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%d ", (gaint)(icattr_val[i])); 
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(icattr_val,"f147");
	  break;
	case (DFNT_UINT8):    /* definition value 21 */
	  uicattr_val = (uint8*)galloc ((attr_count) * sizeof (uint8),"uicattrval");
	  if (SDreadattr(sds_id, attr_index, cattr_val) == -1) {
	    gaprnt(2,"SDreadattr failed for type UINT8\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," Byte "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%u ", (gauint)(uicattr_val[i])); 
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(uicattr_val,"f148");
	  break;
	case (DFNT_INT16):    /* definition value 22 */
	  sattr_val = (int16*)galloc (attr_count * sizeof (int16),"sattrval2");
	  if (SDreadattr(sds_id, attr_index, sattr_val) == -1) {
	    gaprnt(2,"SDreadattr failed for type INT16\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," Int16 "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%d ", (gaint)(sattr_val[i])); 
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(sattr_val,"f149");
	  break;
	case (DFNT_UINT16):   /* definition value 23 */
	  usattr_val = (uint16*)galloc (attr_count * sizeof (uint16),"usattrval2");
	  if (SDreadattr(sds_id, attr_index, usattr_val) == -1) { 
	    gaprnt(2,"SDreadattr failed for type UINT16\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," UInt16 "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%u ", (gauint)(usattr_val[i])); 
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(usattr_val,"f150");
	  break;
	case (DFNT_INT32):    /* definition value 24 */
	  iattr_val = (int32*)galloc (attr_count * sizeof (int32),"iattrval3");
	  if (SDreadattr(sds_id, attr_index, iattr_val) == -1) {
	    gaprnt(2,"SDreadattr failed for type INT32\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," Int32 "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%d ", iattr_val[i]); 
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(iattr_val,"f151");
	  break;
	case (DFNT_UINT32):   /* definition value 25 */
	  uiattr_val = (uint32*)galloc (attr_count * sizeof (uint32),"uiattrval3");
	  if (SDreadattr(sds_id, attr_index, uiattr_val) == -1) { 
	    gaprnt(2,"SDreadattr failed for type UINT32\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," UInt32 "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%u ", uiattr_val[i]); 
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(uiattr_val,"f152");
	  break;
	case (DFNT_FLOAT32):  /* definition value  5 */
	  fattr_val = (float32*)galloc (attr_count * sizeof (float32),"fattrval3");
	  if (SDreadattr(sds_id, attr_index, fattr_val) == -1) {
	    gaprnt(2,"SDreadattr failed for type FLOAT32\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," Float32 "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%g ", fattr_val[i]);
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(fattr_val,"f153");
	  break;
	case (DFNT_FLOAT64):  /* definition value  6 */
	  dattr_val = (float64*)galloc (attr_count * sizeof (float64),"dattrval3");
	  if (SDreadattr(sds_id, attr_index, dattr_val) == -1) {
	    gaprnt(2,"SDreadattr failed for type FLOAT64\n"); 
	  }
	  else {
	    gaprnt(2,abbrv); 
	    gaprnt(2," Float64 "); 
	    gaprnt(2,attr_name); 
	    gaprnt(2," ");
	    for (i=0; i<attr_count; i++) {
	      sprintf(pout,"%g ", dattr_val[i]); 
	      gaprnt(2,pout);
	    }
	    gaprnt(2,"\n");
	  }
	  gree(dattr_val,"f154");
	  break;
	default:
	  sprintf(pout,"Failed to retrieve attribute %d of type %d \n", attr_index, attr_dtype);
	  gaprnt(2,pout);
	};
    }  /* end of if-else statment following call to SDattrinfo */
  } 
  return(n_atts);
#endif
  gaprnt(0,"Reading HDF-SDS attributes is not supported in this build\n");
  return(1);
}

/* routine to print out a string attribute that may have carriage returns in it */
void prntwrap(char *vname, char *aname, char *str ) {
  char *pos, *line;
  pos = line = str;        
  while (*pos != '\0') { 
    if (*pos == '\n') {
      *pos = '\0';         /* swap null for carriage return */
      gaprnt(2,line);
      /* add varname, attr_type, and attr_name after carriage return */
      sprintf(pout," \n%s String %s ",vname,aname); 
      gaprnt(2,pout);
      *pos = '\n';         /* put the carriage return back in */
      line = pos+1;
    }
    pos++;
  }
  if (line < pos) {    /* Print string that has no carriage returns in it */
    gaprnt(2,line); 
  }
  gaprnt(2,"\n");
}


#endif  /* matches #ifndef STNDALN */

/* Routine to open appropriate file when using file templates */
/* Warning -- changes time value to time with respect to this file */
/* Warning -- also changes ensemble value */

gaint gaopfn(gaint t, gaint e, gaint *ee, gaint *oflg, struct gafile *pfi) {
struct dt dtim, dtimi;
struct gaens *ens;
gaint i,rc,flag,endx,need_new_file;
char *fn=NULL;

  *oflg = 0;
  /* make sure e and t are within range of grid dimensions */
  if (t<1 || t>pfi->dnum[3]) return(-99999);
  if (e<1 || e>pfi->dnum[4]) return(-99999);
  i = pfi->fnums[(e-1)*pfi->dnum[3]+t-1];
  if (i == -1) {
    /* there is no data file associated with this time and ensemble member */
      pfi->fnumc = 0;
      pfi->fnume = 0;
      return(-88888);
  }  

  /* find out if we need to open a new file */
  need_new_file=0;
  if (pfi->tmplat==3 && ((i != pfi->fnumc) || (e != pfi->fnume))) need_new_file=1;
  if (pfi->tmplat==2 && (e != pfi->fnume)) need_new_file=1;
  if (pfi->tmplat==1 && (i != pfi->fnumc)) need_new_file=1;


  /* the current file is not the one we need */
  if (need_new_file) {
    /* close SDF file */
    if (pfi->ncflg) {
      if (pfi->ncflg==1) {
#if USENETCDF == 1
	if (pfi->ncid != -999) nc_close(pfi->ncid);
#endif
      }
      else if (pfi->ncflg==2) {
#if USEHDF == 1
	if (pfi->sdid != -999) SDend(pfi->sdid); 
#endif
      }
    } 
    /* close BUFR file*/
    else if (pfi->bufrflg) {
      if (pfi->bufrdset) {
	gabufr_close(pfi->bufrdset);  /* release memory */
	pfi->bufrdset=NULL;           /* reset the pointer */
      }
    } 
    /* close non-SDF, non-BUFR file */
    else {
      if (pfi->infile!=NULL) fclose(pfi->infile);
    }
    /* release old file name */
    if (pfi->tempname!=NULL) {
      gree(pfi->tempname,"f116");
    }

    /* advance through chain of ensemble structure to get to ensemble 'e' */
    ens=pfi->ens1;    
    endx=1;
    while (endx<e) { endx++; ens++; }
    /* find the filename that goes with current time and ensemble member */
    gr2t(pfi->grvals[3], (gadouble)t, &dtim);   /* current t value */
    gr2t(pfi->grvals[3], ens->gt, &dtimi);      /* initial t for this ensemble member */
    fn = gafndt(pfi->name, &dtim, &dtimi, pfi->abvals[3], pfi->pchsub1, pfi->ens1, t, e, &flag);
    if (fn==NULL) return (-99999);
    /* Open the data file */
    rc = 0;
    pfi->tempname = fn;
    pfi->fnumc = i;
    pfi->fnume = e;

    /* open netcdf */
    if (pfi->ncflg==1) {  
      rc = gaopnc (pfi,1,0);
      if (rc) pfi->ncid = -999;
    }
#if USEHDF ==1
    /* open hdfsds */
    else if (pfi->ncflg==2) { 
      rc = gaophdf (pfi,1,0);
      if (rc) pfi->sdid = -999;
    }
#endif
    /* open all others except BUFR */
    else if (!pfi->bufrflg) {        
      pfi->infile = fopen (fn, "rb");
      if (pfi->infile == NULL) rc = 1;
    } 

    /* Error checking on file open */
    if (rc) {
      if (pfi->errflg && timerr!=t) {
        gaprnt(1,"Warning: Open error, fn = ");
        gaprnt(1,fn);
        gaprnt(1,"\n");
        timerr = t;
      }
      pfi->fnumc = 0;
      pfi->fnume = 0;
      return(-88888);
    }
    *oflg = 1;
  } /* matches if (need_new_file)  */

  if (pfi->tmplat==1) { /* templating on T but not E */
    *ee = e;                /* set relative ensemble number to E for this file */ 
  }
  else {
    *ee = 1;                /* set relative ensemble number to 1 for this file */ 
  }

  if (pfi->tmplat==2) { /* templating on E but not T */
    t = t;
  } 
  else {
    t = 1 + t - pfi->fnumc;   /* set relative t for this file */
  }
  return (t);
}

/* Open a netcdf file */
gaint gaopnc (struct gafile *pfil, gaint tflag, gaint eflag) {
#if  USENETCDF == 1
  struct gavar *pvarl;
  gaint i,rc;

  if (tflag) {
    rc = nc_open(pfil->tempname, NC_NOWRITE, &i);
  } 
  else {
    rc = nc_open(pfil->name, NC_NOWRITE, &i);
  }
  if (rc != NC_NOERR) {
    if (eflag) {
      if (tflag) 
	sprintf(pout,"gaopnc error: nc_open failed to open file %s\n",pfil->tempname);
      else
	sprintf(pout,"gaopnc error: nc_open failed to open file %s\n",pfil->name);
      gaprnt(0,pout);
    }
    return (1);
  }
  pfil->ncid = i;     
  if (pfil->pvar1 != NULL) {
    pvarl = pfil->pvar1;
    for (i=0; i<pfil->vnum; i++) {
      if (pvarl->ncvid != -888) pvarl->ncvid = -999;
      pvarl++;
    }
  }
  return (0);
#else
  return(1);
#endif
}


/* Open an HDF-SDS file */
gaint gaophdf (struct gafile *pfil, gaint tflag, gaint eflag) {
#if USEHDF == 1
struct gavar *pvarl;
gaint i;
int32 sd_id;

  sd_id = -999;
  if (tflag) {
    sd_id = SDstart(pfil->tempname, DFACC_READ);
  } 
  else {
    sd_id = SDstart(pfil->name, DFACC_READ);
  }
  if (sd_id==FAIL) {
    if (eflag) {
      gaprnt(0,"HDF-SDS Error: Unable to open file\n");
    }
    return (1);
  }
  pfil->sdid = sd_id;     
  if (pfil->pvar1 != NULL) {
    pvarl = pfil->pvar1;
    for (i=0; i<pfil->vnum; i++) {
      if (pvarl->sdvid != -888) pvarl->sdvid = -999;
      pvarl++;
    }
  }
  return (0);
#endif
  gaprnt(0,"Opening HDF-SDS files is not supported in this build\n");
  return(1);
}

