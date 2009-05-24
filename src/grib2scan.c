/*  Copyright (C) 2006-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.
    Written by Brian Doty and Jennifer M. Adams  */

#ifdef HAVE_CONFIG_H
#include "config.h"

/* If autoconfed, only include malloc.h when it's presen */
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#else /* undef HAVE_CONFIG_H */

#include <malloc.h>

#endif /* HAVE_CONFIG_H */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <gatypes.h>

/* global variables */
struct gag2 {
  gaint discipline,parcat,parnum;     /* Parameter identifiers */
  gaint yr,mo,dy,hr,mn,sc;            /* Reference Time */
  gaint sig;                          /* Significance of Reference Time */
  gaint numdp;                        /* Number of data points */
  gaint gdt;                          /* Grid Definition Template */
  gaint pdt;                          /* Product Definition Template */
  gaint drt;                          /* Data Representation Template */
  gaint trui;                         /* Time range units indicator */
  gaint ftime;                        /* Forecast time */
  gaint lev1type,lev1sf,lev1;         /* Level 1 type, scale factor, scaled value */
  gaint lev2type,lev2sf,lev2;         /* Level 2 type, scale factor, scaled value */
  gaint enstype,enspertnum,ensderiv;  /* Ensemble metadata */
  gaint comptype;                     /* Compression type (for JPEG2000 compression) */
  gaint bmsflg;                       /* Bit Map Section flag */
};
gaint verb=0;

/* Function Declarations */
gaint gagby (unsigned char *, gaint, gaint);
gaint gagbb (unsigned char *, gaint, gaint);
gaint sect1 (unsigned char *, struct gag2 *);
gaint sect3 (unsigned char *, struct gag2 *);
gaint sect4 (unsigned char *, struct gag2 *);
gaint sect5 (unsigned char *, struct gag2 *);
gaint sect4sp (unsigned char *, struct gag2 *, gaint, gaint);
void CodeTable0p0  (gaint);
void CodeTable1p2  (gaint);
void CodeTable3p1  (gaint);
void CodeTable4p4  (gaint);
void CodeTable4p7  (gaint);
void CodeTable4p10 (gaint);
void CodeTable5p0  (gaint);
gadouble scaled2dbl(gaint, gaint);


/* MAIN PROGRAM */
gaint main (gaint argc, char *argv[]) {
 FILE *gfile=NULL;
 struct gag2 g2;
 off_t fpos;
 char *ch;
 unsigned char s0[16],work[260],*rec;
 unsigned char *s1,*s2,*s3,*s4,*s5,*s6,*s7,*s8;
 gaint i,flg,iarg,rc,edition,rlen,roff,field,recnum;
 gaint s1len,s2len,s3len=0,s4len,s5len,s6len,s7len;

  
 /* Scan input args. The filename is the argument that is not an option */
 flg = 0;
 if (argc>1) {
   iarg = 1;
   while (iarg<argc) {
     ch = argv[iarg];
     if (*(ch)=='-' && *(ch+1)=='v') { 
       verb=1;
     } else { 
       gfile = fopen(ch,"rb");
       if (gfile==NULL) {
	 printf ("Could not open input file %s\n",ch);
	 return(1);
       }
       flg = 1;
     }
     iarg++;
   }
 } 
 if (!flg) {
   printf ("Usage: grib2scan [-v] filename\n");
   return (1);
 }

 /* Main loop through all GRIB records in the file */
 fpos = 0;
 recnum=1;
 while (recnum) {
   
   /* Scan for the 'GRIB' char string */
   flg = 0;
   while (1) {
     rc = fseeko(gfile,fpos,SEEK_SET);
     rc = fread(work,1,260,gfile);
     if (rc<260) {
       printf ("EOF\n");
       return(0);
     }
     for (i=0; i<257; i++) { 
       if (*(work+i+0)=='G' && 
	   *(work+i+1)=='R' &&
	   *(work+i+2)=='I' && 
	   *(work+i+3)=='B') { 
	 fpos = fpos + (off_t)i;
	 flg = 1;
	 i = 999;
       }
     }
     if (flg) break;
     else fpos = fpos + (off_t)(256);
   }
   
   /* Section 0, the Indicator Section */
   rc = fseeko(gfile,fpos,SEEK_SET);
   rc = fread(s0,1,16,gfile);
   if (rc < 16) {
     printf ("I/O Error Reading Section 0\n");
     return(99);
   }
   if (s0[0]!='G' || s0[1]!='R' || s0[2]!='I' || s0[3]!='B') {
     printf ("GRIB indicator not found \n");
     return (2);
   }
   edition = gagby(s0,7,1);
   if (edition != 2) {
     printf ("GRIB edition number not 2; value = %i\n",edition);
     return (3);
   }
   rlen = gagby(s0,8,4);   /* Note: need to fix gamach.c and elsewhere */  
   if (rlen!=0) {
     printf ("GRIB record length too large\n");
     return (4);
   }
   rlen = gagby(s0,12,4);   
   g2.discipline = gagby(s0,6,1);
   i = fpos;
   printf ("\nRecord %d ",recnum);
   if (verb) printf("starts at %i of length %i \n",i,rlen); 
   else printf("\n");
   if (verb) {
     printf ("  Discipline=%d ",g2.discipline);
     CodeTable0p0(g2.discipline);
     printf ("\n");
   }
   
   /* Read the entire GRIB 2 record */
   rec = (unsigned char *)malloc(rlen);
   if (rec==NULL) {
     printf ("memory allocation error\n");
     return (99);
   }
   rc = fseeko(gfile,fpos,SEEK_SET);
   rc = fread(rec,1,rlen,gfile);
   if (rc!=rlen) {
     printf ("I/O Error reading GRIB2 record \n");
     return(99);
   }
   
   /* Section 1, the Identification Section */
   roff = 16;
   s1 = rec+roff;              
   i = gagby(s1,4,1);
   if (i!=1) {
     printf ("Header error, section 1 expected, found %i\n",i);
     return (5);
   }
   s1len = gagby(s1, 0,4);     
   rc = sect1(s1,&g2);
   if (rc) return (rc);
   roff += s1len;
   
   /* Loop over multiple fields */
   field = 1;
   while (1) {
     
     printf (" Field %d ",field);
     if (verb>1) printf("starts at location %i\n",roff);
     else printf("\n");

     /* Section 2, the Local Use Section */
     s2 = rec+roff;
     i = gagby(s2,4,1);
     if (i==2) {
       s2len = gagby(s2,0,4);
       roff += s2len;
     } else s2len = 0;
     
     /* Section 3, the Grid Definition Section */
     s3 = rec+roff;
     i = gagby(s3,4,1);
     if (i==3) {
       s3len = gagby(s3,0,4);  
       rc = sect3(s3,&g2);
       if (rc) return (rc);
       roff += s3len;
     } else if (field==1) {
       printf ("Header error, section 3 expected, found %i\n",i);
       return (5);
     } 
     
     /* Section 4, the Product Definition Section */
     s4 = rec+roff;
     i = gagby(s4,4,1);
     if (i!=4) {
       printf ("Header error, section 4 expected, found %i\n",i);
       return (5);
     }
     s4len  = gagby(s4,0,4);
     rc = sect4(s4,&g2);
     if (rc) return (rc);
     roff += s4len;

     /* Section 5, the Data Representation Section */
     s5 = rec+roff;
     i = gagby(s5,4,1);
     if (i!=5) {
       printf ("Header error, section 5 expected, found %i\n",i);
       return (5);
     }
     s5len = gagby(s5,0,4);
     rc = sect5(s5,&g2);
     if (rc) return (rc);
     roff += s5len;
     
     /* Section 6, the Bit Map Section*/
     s6 = rec+roff;
     i = gagby(s6,4,1);
     if (i==6) {
       s6len = gagby(s6,0,4);
       g2.bmsflg = gagby(s6,5,1);
       if (verb>1) printf("  BMI=%d \n",g2.bmsflg);
       roff += s6len;
     } else s6len = 0;
     
     /* Section 7, the Data Section */
     s7 = rec+roff;
     i = gagby(s7,4,1);
     if (i!=7) {
       printf ("Header error, section 7 expected, found %i\n",i);
       return (5);
     }
     s7len = gagby(s7,0,4);
     if (verb>1) printf ("  Lengths of Sections 1-7: %i %i %i %i %i %i %i\n",
		       s1len, s2len, s3len, s4len, s5len, s6len, s7len);
     roff += s7len;
     
     /* Section 8, the End Section */
     s8 = rec+roff;
     if (*s8=='7' && *(s8+1)=='7' && *(s8+2)=='7' && *(s8+3)=='7') {
       break;
     }
     
     /* If it wasn't the End, look for another field */
     field++;
   }
   
   fpos = fpos + (off_t)rlen;
   free(rec);
   recnum++;
 }
 return(0);
}

/* Look at contents of Section 1 */
gaint sect1 (unsigned char *s1, struct gag2 *pg2) {
  pg2->sig = gagby(s1,11,1);     
  pg2->yr  = gagby(s1,12,2);
  pg2->mo  = gagby(s1,14,1);
  pg2->dy  = gagby(s1,15,1);
  pg2->hr  = gagby(s1,16,1);
  pg2->mn  = gagby(s1,17,1);
  pg2->sc  = gagby(s1,18,1);
  printf ("  Reference Time = %4i-%02i-%02i %02i:%02i:%02i  ",
	  pg2->yr,pg2->mo,pg2->dy,pg2->hr,pg2->mn,pg2->sc); 
  CodeTable1p2(pg2->sig);
  printf("\n");
  return(0);

 }

/* Look at contents of Section 3 */
gaint sect3 (unsigned char *s3, struct gag2 *pg2) {
  gaint nx,ny,angle,lat1,lon1,di,dj,lon2,lat2,nlats;
  gaint rlon,dx,dy,latin1,latin2;
  gaint sbit1,sbit2,sbit3,sbit4;

  pg2->numdp = gagby(s3,6,4);    
  pg2->gdt   = gagby(s3,12,2);
  printf ("  GDT=%i ",pg2->gdt);
  CodeTable3p1(pg2->gdt);
  printf(" nx*ny=%d\n",pg2->numdp);
  if (verb) {
    if (pg2->gdt==0) {         /* lat-lon grid */
      nx    = gagby(s3,30,4);
      ny    = gagby(s3,34,4);
      angle = gagby(s3,38,4);
      if (angle==0) {
	lat1 = gagby(s3,46,4);
	lon1 = gagby(s3,50,4);
	di   = gagby(s3,63,4);
	dj   = gagby(s3,67,4);
	printf("   XDEF %i linear %f %f\n",nx,lon1*1e-6,di*1e-6);
	printf("   YDEF %i linear %f %f\n",ny,lat1*1e-6,dj*1e-6);
      }
    }
    else if (pg2->gdt==20) {   /* Polar Stereographic */
      nx    = gagby(s3,30,4);
      ny    = gagby(s3,34,4);
      lat1  = gagby(s3,38,4);
      lon1  = gagby(s3,42,4);
      rlon  = gagby(s3,51,4);
      dx    = gagby(s3,55,4);
      dy    = gagby(s3,59,4);
      printf("   nx=%i ny=%i lon1=%f lat1=%f\n",nx, ny, lon1*1e-6, lat1*1e-6);
      printf("   reflon=%f dx=%f dy=%f\n",rlon*1e-6,dx*1e-3,dy*1e-3); 

    }
    else if (pg2->gdt==30) {   /* Lambert Conformal */
      nx     = gagby(s3,30,4);
      ny     = gagby(s3,34,4);
      lat1   = gagby(s3,38,4);
      lon1   = gagby(s3,42,4);
      rlon   = gagby(s3,51,4);
      dx     = gagby(s3,55,4);
      dy     = gagby(s3,59,4);
      latin1 = gagby(s3,65,4);
      latin2 = gagby(s3,69,4);
      printf("   nx=%i ny=%i lon1=%f lat1=%f\n",nx, ny, lon1*1e-6, lat1*1e-6);
      printf("   reflon=%f dx=%f dy=%f\n",rlon*1e-6, dx*1e-3, dy*1e-3); 
      printf("   latin1=%f latin2=%f \n",latin1*1e-6, latin2*1e-6);
      if (verb>1) {
	sbit1 = gagbb(s3+64,0,1);
	sbit2 = gagbb(s3+64,1,1);
	sbit3 = gagbb(s3+64,2,1);
	sbit4 = gagbb(s3+64,3,1);
	printf("   scan_mode_bits = %i%i%i%i\n",sbit1, sbit2, sbit3, sbit4);
      }      
    }
    else if (pg2->gdt==40) {   /* Gaussian lat-lon grid */
      nx    = gagby(s3,30,4);
      ny    = gagby(s3,34,4);
      angle = gagby(s3,38,4);
      if (angle==0) {
	lat1  = gagby(s3,46,4);
	lon1  = gagby(s3,50,4);
	lat2  = gagby(s3,55,4);
	lon2  = gagby(s3,59,4);
	di    = gagby(s3,63,4);
	nlats = gagby(s3,67,4);
	printf("   nx=%i ny=%i di=%f nlats=%i\n",nx, ny, di*1e-6,nlats);
	printf("   lon: %f to %f\n", lon1*1e-6, lon2*1e-6);
	printf("   lat: %f to %f\n", lat1*1e-6, lat2*1e-6);
      }
    }
  }
  return (0);
}

/* Look at contents of Section 4 */
gaint sect4 (unsigned char *s4, struct gag2 *pg2) {
  gaint enstotal,sp;
  gaint fpnum,fptot,fptyp,llsf,llval,ulsf,ulval;
  gadouble ll=0,ul=0;
  gadouble lev1=0,lev2=0;
  
  pg2->pdt      = gagby(s4, 7,2);
  pg2->parcat   = gagby(s4, 9,1); 
  pg2->parnum   = gagby(s4,10,1); 
  pg2->trui     = gagby(s4,17,1); 
  pg2->ftime    = gagby(s4,18,4); 
  pg2->lev1type = gagby(s4,22,1);
  pg2->lev1sf   = gagby(s4,23,1);
  pg2->lev1     = gagby(s4,24,4);
  pg2->lev2type = gagby(s4,28,1);
  pg2->lev2sf   = gagby(s4,29,1);
  pg2->lev2     = gagby(s4,30,4);
  /* get the statistical process if not instantaneous value */
  sp = -999;
  if      (pg2->pdt ==  8) sp = sect4sp (s4, pg2, 34, 46);
  else if (pg2->pdt ==  9) sp = sect4sp (s4, pg2, 47, 59);
  else if (pg2->pdt == 10) sp = sect4sp (s4, pg2, 35, 47);
  else if (pg2->pdt == 11) sp = sect4sp (s4, pg2, 37, 49);
  else if (pg2->pdt == 12) sp = sect4sp (s4, pg2, 36, 48);

  if (sp==-999) {
    printf ("  PDT=%i Forecast Time = %d ",pg2->pdt,pg2->ftime);
    CodeTable4p4(pg2->trui);
    printf ("\n");
    printf ("   Parameter: disc,cat,num = %d,%d,%d\n",
	    pg2->discipline,pg2->parcat,pg2->parnum);
  }
  else if (sp==255) {
    printf ("   Parameter: disc,cat,num = %d,%d,%d\n",
	    pg2->discipline,pg2->parcat,pg2->parnum);
  }
  else
    printf ("   Parameter: disc,cat,num,sp = %d,%d,%d,%d\n",
	    pg2->discipline,pg2->parcat,pg2->parnum,sp);


  if (pg2->lev1 != -1) lev1 = scaled2dbl(pg2->lev1sf,pg2->lev1);

  if (pg2->lev2type != 255) {  /* we have two level types */
    if (pg2->lev2 != -1) {
      lev2 = scaled2dbl(pg2->lev2sf,pg2->lev2);
      if (pg2->lev2type == pg2->lev1type) 
	printf ("   Levels: ltype,lval,lval2 = %d,%g,%g ",pg2->lev1type, lev1,lev2);
      else
	printf ("   Levels: ltype,lval,lval2,ltype2 = %d,%g,%g,%d ",
		pg2->lev1type, lev1,lev2,pg2->lev2type);
      if (verb) 
	printf("  (sf1,sval1,sf2,sval2 = %d %d %d %d) \n",pg2->lev1sf,pg2->lev1,pg2->lev2sf,pg2->lev2);
      else printf ("\n");
    }
    else {  /* level values are missing */
      if (pg2->lev2type == pg2->lev1type) 
	printf ("   Levels: ltype = %d \n",pg2->lev1type);
      else
	printf ("   Levels: ltype,,,ltype2 = %d,,,%d \n",pg2->lev1type,pg2->lev2type);
    }
  }	
  else {    /* only one level type */
    if (pg2->lev1 != -1) {
      printf ("   Level: ltype,lval = %d,%g ",pg2->lev1type,lev1);
      if (verb) printf("  (sf,sval = %d %d) \n",pg2->lev1sf,pg2->lev1);
      else printf ("\n");
    }
    else
      printf ("   Level: ltype = %d \n",pg2->lev1type);  /* level value is missing */
  }

  /* Ensemble Metadata */
  if (pg2->pdt==1 || pg2->pdt==2 || pg2->pdt==11 || pg2->pdt==12) {
    if (pg2->pdt==1 || pg2->pdt==11) {   /* individual ensemble members */
      pg2->enstype    = gagby(s4,34,1); 
      pg2->enspertnum = gagby(s4,35,1);
      enstotal        = gagby(s4,36,1);
      printf("   Ens: type,pert = %d,%d ",pg2->enstype,pg2->enspertnum); 
      if (verb) printf("(total=%d) \n",enstotal);
      else printf("\n");
    }
    else {                            /* derived fields from all ensemble members */
      pg2->ensderiv   = gagby(s4,34,1);
      enstotal        = gagby(s4,35,1);
      printf("   Ens: deriv = %d  ",pg2->ensderiv);
      if (verb) CodeTable4p7(pg2->ensderiv);
      printf("\n");
    }
  }
  /* Probability Forecasts */
  if (pg2->pdt==9) {
    fpnum = gagby(s4,34,1);
    fptot = gagby(s4,35,1);
    fptyp = gagby(s4,36,1);
    llsf  = gagby(s4,37,1);
    llval = gagby(s4,38,4);
    ulsf  = gagby(s4,42,1);
    ulval = gagby(s4,43,4);
    ll = scaled2dbl(llsf,llval);
    ul = scaled2dbl(ulsf,ulval);
    printf("   Forecast Probability Number %d out of %d  Type = %d\n",fpnum,fptot,fptyp); 
    printf("   Lower and Upper Limits: %g %g\n",ll,ul); 
  }
  return(0);
}

/* get info about statistical process in section 4 */
gaint sect4sp (unsigned char *s4, struct gag2 *pg2, gaint pos1, gaint pos2) {
gaint endyr,endmo,enddy,endhr,endmn,endsc;
gaint var1,var2,var3,var4,var5,numtr,sp;
  sp=-999;
  endyr = gagby(s4,pos1+0,2);
  endmo = gagby(s4,pos1+2,1);
  enddy = gagby(s4,pos1+3,1);
  endhr = gagby(s4,pos1+4,1);
  endmn = gagby(s4,pos1+5,1);
  endsc = gagby(s4,pos1+6,1);
  numtr = gagby(s4,41,1);
  if (numtr) {
    sp   = gagby(s4,pos2+0,1);
    var1 = gagby(s4,pos2+1,1);
    var2 = gagby(s4,pos2+2,1);
    var3 = gagby(s4,pos2+3,4);
    var4 = gagby(s4,pos2+7,1);
    var5 = gagby(s4,pos2+8,4);
    if (var5==0) {   /* continuous statistical processing function */
      printf("  PDT=%d ",pg2->pdt);
      if (sp<=255) {
	printf("(%d ",var3);
	CodeTable4p4(var2);
	CodeTable4p10(sp);
	printf(") ");
      }
      printf("EndTime = %4i-%02i-%02i %02i:%02i \n",endyr,endmo,enddy,endhr,endmn);
    }
  }
  return (sp);
}


/* Look at contents of Section 5 */
gaint sect5 (unsigned char *s5, struct gag2 *pg2) {
  gafloat r;
  gaint e,d,nbits,otype;

  if (verb) {
    pg2->drt = gagby(s5,9,2);
    r        = gagby(s5,11,4);
    e        = gagby(s5,15,2);
    d        = gagby(s5,17,2);
    nbits    = gagby(s5,19,1);
    otype    = gagby(s5,20,1);
    printf ("  DRT=%i ",pg2->drt);
    CodeTable5p0(pg2->drt);
    if (pg2->drt == 40) {  
      pg2->comptype = gagby(s5,21,1);
      if (pg2->comptype==0) printf(" (Lossless) ");
      if (pg2->comptype==1) printf(" (Lossy) ");
    }
    printf("\n");
  }
  return(0);
}


/* Discipline */
void CodeTable0p0 (gaint i) {
  if      (i== 0) printf ("(Meteorological)");
  else if (i== 1) printf ("(Hydrological)");
  else if (i== 2) printf ("(Land Surface)");
  else if (i== 3) printf ("(Space)");
  else if (i==10) printf ("(Oceanographic)");
}
/* Significance of Reference Time */
void CodeTable1p2 (gaint i) {
  if      (i==0) printf ("Analysis");
  else if (i==1) printf ("Start of Forecast");
  else if (i==2) printf ("Verifying Time of Forecast");
  else if (i==3) printf ("Observation Time");
}
/* Grid Definition Template Number */
void CodeTable3p1 (gaint i) {
  if      (i==0)   printf("(Lat/Lon)");
  else if (i==1)   printf("(Rotated Lat/Lon)");
  else if (i==2)   printf("(Stretched Lat/Lon)");
  else if (i==3)   printf("(Rotated and Stretched Lat/Lon)");
  else if (i==10)  printf("(Mercator)");
  else if (i==20)  printf("(Polar Stereographic)");
  else if (i==30)  printf("(Lambert Conformal)");
  else if (i==31)  printf("(Albers Equal Area)");
  else if (i==40)  printf("(Gaussian Lat/Lon)");
  else if (i==41)  printf("(Rotated Gaussian Lat/Lon)");
  else if (i==42)  printf("(Stretched Gaussian Lat/Lon)");
  else if (i==43)  printf("(Rotated and Stretched Gaussian Lat/Lon)");
  else if (i==50)  printf("(Spherical Harmonic Coefficients)");
  else if (i==51)  printf("(Rotated Spherical Harmonic Coefficients)");
  else if (i==52)  printf("(Stretched Spherical Harmonic Coefficients)");
  else if (i==53)  printf("(Rotated and Stretched Spherical Harmonic Coefficients)");
  else if (i==90)  printf("(Orthoraphic)");
  else if (i==100) printf("(Triangular Grid Based on an Icosahedron)");
  else if (i==110) printf("(Equatorial Azimuthal Equidistant Projection)");
  else if (i==120) printf("(Azimuth-Range Projection)");
}
/* Time Range Unit Indicator */
void CodeTable4p4 (gaint i) {
  if      (i==0)  printf ("Minute ");
  else if (i==1)  printf ("Hour ");
  else if (i==2)  printf ("Day ");
  else if (i==3)  printf ("Month ");
  else if (i==4)  printf ("Year ");
  else if (i==5)  printf ("Decade ");
  else if (i==6)  printf ("Normal ");
  else if (i==7)  printf ("Century ");
  else if (i==10) printf ("3-Hour ");
  else if (i==11) printf ("6-Hour ");
  else if (i==12) printf ("12-Hour ");
  else if (i==13) printf ("Second ");
}
/* Derived Ensemble Forecast */
void CodeTable4p7 (gaint i) {
  if      (i==0)  printf ("Unweighted Mean All Members ");
  else if (i==1)  printf ("Weighted Mean All Members ");
  else if (i==2)  printf ("StdDev (Cluster Mean) ");
  else if (i==3)  printf ("StdDev (Cluster Mean, normalized) ");
  else if (i==4)  printf ("Spread of All Members ");
  else if (i==5)  printf ("Large Anomaly Index All Members ");
  else if (i==6)  printf ("Unweighted Mean Cluster Members ");
}
/* Type of Statistical Processing */
void CodeTable4p10 (gaint i) {
  if      (i==0)  printf ("Average");
  else if (i==1)  printf ("Accumulation");
  else if (i==2)  printf ("Maximum");
  else if (i==3)  printf ("Minimum");
  else if (i==4)  printf ("Diff(end-beg)");
  else if (i==5)  printf ("RMS");
  else if (i==6)  printf ("StdDev");
  else if (i==7)  printf ("Covariance");
  else if (i==8)  printf ("Diff(beg-end)");
  else if (i==9)  printf ("Ratio");
  else            printf ("SP=%d",i);
}
/* Data Representation Template Number */
void CodeTable5p0 (gaint i) {
  if      (i==0)  printf ("(Grid Point Data - Simple Packing)");
  else if (i==1)  printf ("(Matrix Value at Grid Point - Simple Packing)");
  else if (i==2)  printf ("(Grid Point Data - Complex Packing)");
  else if (i==3)  printf ("(Grid Point Data - Complex Packing and Spatial Differencing)");
  else if (i==40) printf ("(Grid Point Data - JPEG2000 Compression)");
  else if (i==41) printf ("(Grid Point Data - PNG Compression )");
  else if (i==50) printf ("(Spectral Data - Simple Packing)");
  else if (i==51) printf ("(Spectral Data - Simple Packing)");
}

void gaprnt (gaint i, char *ch) {
  printf ("%s",ch);
}
char *gxgsym(char *ch) {
  return (getenv(ch));
}


