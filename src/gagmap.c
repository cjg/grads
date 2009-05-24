/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/*  Values output into the grib1 map file:
     Header:
     hipnt info: 0 - version number (1)
                 1 - number of times in file
                 2 - number of records per time
                 3 - Grid type
                   255 - user defined grid.  descriptor
                         describes grid exactly; one record
                         per grid.
                    29 - Predefined grid set 29 and 30.
                         Two records per grid.
     hfpnt info:  None
     Info:
     intpnt info (for each mapped grib record) :
                 0 - position of start of data in file
                 1 - position of start of bit map in file
                 2 - number of bits per data element
     fltpnt info :
                 0 - decimal scale factor for this record
                 1 - binary scale factor
                 2 - reference value
*/

#ifdef HAVE_CONFIG_H
#include "config.h"

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#else /* undef HAVE_CONFIG_H */
#include <malloc.h>
#endif /* HAVE_CONFIG_H */

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "grads.h"
#include "gagmap.h"
#if GRIB2
#include "grib2.h"
#endif


/* global variables */
extern struct gamfcmn mfcmn;
struct dt rtime;   /* time based on input map */
struct dt ftime;   /* time based on dd file */
static off_t flen;
gaint ng1elems=3;
gaint ng2elems=2;

/*  Routine to scan a grib1 or grib2 file and output an index (map) file. */

gaint gribmap (void) {

#if GRIB2
 unsigned char *cgrib=NULL;
 g2int  listsec0[3],listsec1[13],numlocal,numfields,n;
 g2int  unpack,expand,lskip,lgrib,iseek;
 gribfield  *gfld;
 char *ptr;
 size_t  lengrib;
 struct gag2indx *g2indx;
#endif
 char *ch=NULL;
 gaint ret,ierr,flag,rcgr,record;
 gaint rc,i,e,tmin=0,tmax=0,told,tcur,fnum,didmatch=0;
 gaint sp,ioff,eoff,it,write_map;
 struct gafile *pfi;
 struct dt dtim,dtimi;
 struct gaens *ens;

#if GRIB2
 unpack=0;
 expand=0;
#endif
 mfile=NULL;
 write_map=1;

 /* Get the descriptor file name */
 if (ifile==NULL) {
   printf ("\n");
   cnt = nxtcmd (cmd,"Enter name of Data Descriptor file: ");
   if (cnt==0) return(1);
   getwrd(crec,cmd,250);
   ifile = crec;
 }
 
 /* Allocate memory for gafile structure */
 pfi = getpfi();
 if (pfi==NULL) {
   printf ("gribmap error: unable to allocate memory for gafile structure\n");
   return(1);
 }

 /* Parse the descriptor file */
 rc = gaddes (ifile, pfi, 0);
 if (rc) return(1);

 /* Check index flags */
 if (pfi->idxflg!=1 && pfi->idxflg!=2) {
   printf ("gribmap error: data descriptor file is not for GRIB data\n");
   return(1);
 } 

 /* * GRIB1 * */
 else if (pfi->idxflg==1) {

   /* Allocate memory for gaindx structure */
   pindx = (struct gaindx *)galloc(sizeof(struct gaindx),"pindxgm");
   if (pindx==NULL) {
     printf ("grib1map error: unable to allocate memory for pindx\n");
     return(1);
   }
   
   /* Save the initial time from the descriptor file for the tau0 option and the map file */
   btimdd.yr = *(pfi->abvals[3]);
   btimdd.mo = *(pfi->abvals[3]+1);
   btimdd.dy = *(pfi->abvals[3]+2);
   btimdd.hr = *(pfi->abvals[3]+3);
   btimdd.mn = *(pfi->abvals[3]+4);
   if (no_min) btimdd.mn = 0;
   
   /* Set up for this grid type */
   if (pfi->grbgrd<-900 || pfi->grbgrd==255) {
     nrec = 1;
     gtype[0] = 255;
   } else if (pfi->grbgrd>-1 && pfi->ppflag) {
     nrec=1;
     gtype[0] = pfi->grbgrd;
   } else if (pfi->grbgrd==29) {
     nrec = 2;
     gtype[0] = 29;
     gtype[1] = 30;
     if (pfi->dnum[0]!=144 || pfi->dnum[1]!=73 ||
	 pfi->linear[0]!=1 || pfi->linear[1]!=1 ||
	 *(pfi->grvals[0])!= 2.5 || *(pfi->grvals[0]+1) != -2.5 ||
	 *(pfi->grvals[1])!= 2.5 || *(pfi->grvals[1]+1) != -92.5 ) {
       printf("grib1map error: grid specification for GRIB grid type 29/30.\n");
       printf("                grid scaling must indicate a 2.5 x 2.5 grid\n");
       printf("                grid size must be 144 x 73\n");
       printf("                grid must go from 0 to 357.5 and -90 to 90\n");
       return(1);
     }
   } else {
     nrec = 1;
     gtype[0] = pfi->grbgrd;
   }
   
   /* Set up grib1 index and initialize values */
   pindx->type   = g1ver;
   pindx->hinum  = 4;
   pindx->hfnum  = 0;
   pindx->intnum = nrec * ng1elems * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
   pindx->fltnum = nrec * ng1elems * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
   pindx->hipnt  = (gaint *)galloc(sizeof(gaint)*pindx->hinum,"hipntgm");
   pindx->intpnt = (gaint *)galloc(sizeof(gaint)*pindx->intnum,"intpntgm");
   pindx->fltpnt = (gafloat *)galloc(sizeof(gafloat)*pindx->fltnum,"fltpntgm");
   if (pindx->hipnt==NULL || pindx->intpnt==NULL || pindx->fltpnt==NULL) {
     printf ("grib1map error: unable to allocate memory for index pointers\n");
     return(1);
   }
   for (i=0; i<pindx->intnum; i++) *(pindx->intpnt+i) = -999;
   for (i=0; i<pindx->fltnum; i++) *(pindx->fltpnt+i) = -999; 
   *(pindx->hipnt+0) = g1ver;
   *(pindx->hipnt+1) = pfi->dnum[3];
   *(pindx->hipnt+2) = pfi->trecs;
   *(pindx->hipnt+3) = pfi->grbgrd;
   if (pfi->grbgrd<-900) *(pindx->hipnt+3) = 255;
   
   /* Loop over all files in the data set */
   gfile = NULL;
   for (e=1,ens=pfi->ens1; e<=pfi->dnum[4]; e++,ens++) {
     tcur = 0;
     while (1) {    /* loop over all times for this ensemble */
       if (pfi->tmplat) {
	 /* make sure no file is open */
	 if (gfile!=NULL) {
	   fclose(gfile);
	   gfile=NULL;
	 }
	 /* advance to first valid time step for this ensemble */
	 if (tcur==0) {
	   told = 0;
	   tcur = 1;
	   while (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1) tcur++;  
	 }
	 else {  /* tcur!=0 */
	   told = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	   /* increment time step until fnums changes */
	   while (told==pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] && tcur<=pfi->dnum[3]) {
	     tcur++;
	   }
	 }

	 /* make sure we haven't advanced past end of time axis */
	 if (tcur>pfi->dnum[3]) break;

	 /* check if we're past all valid time steps for this ensemble */
	 if ((told != -1) && (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1)) break;

	 /* Find the range of t indexes that have the same fnums value.
	    These are the times that are contained in this particular file */
	 tmin = tcur;
	 tmax = tcur-1;
	 fnum = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];

	 if (fnum != -1) {
	   while (fnum == pfi->fnums[(e-1)*pfi->dnum[3]+tmax]) tmax++;
	   gr2t(pfi->grvals[3], (gadouble)tcur, &dtim); 
	   gr2t(pfi->grvals[3], ens->gt, &dtimi);
	   ch = gafndt(pfi->name, &dtim, &dtimi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,tcur,e,&flag);
	   if (ch==NULL) {
	     printf(" grib1map error: couldn't determine data file name for e=%d t=%d\n",e,tcur);
	     return(1);
	   }
	 }
       }
       else { 
	 /* Data set is not templated */
	 ch = pfi->name;
	 tmin = 1;
	 tmax = pfi->dnum[3];
       }
       
       /* Open this GRIB file and position to start of first record */
       if (!quiet) printf(" grib1map:  opening GRIB file: %s \n",ch);
       gfile = fopen(ch,"rb");
       if (gfile==NULL) {
	 if (pfi->tmplat) {
	   if (!quiet) printf (" grib1map warning: could not open GRIB file: %s\n",ch);
	   continue;
	 } else {
	   printf (" grib1map error: could not open GRIB file: %s\n",ch);
	   return(1);
	 }
       }
       if (pfi->tmplat) gree(ch,"312");
       
       /* Get file size */
       fseeko(gfile,0L,2);
       flen = ftello(gfile);
       
       /* Set up to skip appropriate amount and position */
       if (skip > -1) {
	 fpos = skip;
       }
       else {
	 fseeko (gfile,0,0);
	 rc = fread (rec,1,100,gfile);
	 if (rc<100) {
	   printf (" grib1map error: I/O error reading header\n");
	   return(1);
	 }
	 len = gagby(rec,88,4);
	 fpos = len*2 + 100;
       }
       
       /* Main Loop */
       irec=1;
       while (1) {
	 /* read a grib record */
	 rc = gribhdr(&ghdr);      
	 if (rc) break;
	 /* compare to each 2-d variable in the 5-D data volume
	    defined by the descriptor file for a match */
	 rcgr = gribrec(&ghdr,pfi,pindx,tmin,tmax,e);
	 if (rcgr==0) didmatch=1;
	 if (rcgr>=100) didmatch=rcgr;
	 irec++;
       }
       
       /* see how we did */
       if (rc==50) {
	 printf (" grib1map error: I/O error reading GRIB file\n");
	 printf ("                possible cause is premature EOF\n");
	 break;
       }
       if (rc>1 && rc!=98) {
	 printf (" grib1map error: GRIB file format error (rc = %i)\n",rc);
	 return(rc);
       }
       
       /* break out if not templating */
       if (!pfi->tmplat) break;
       
     } /* end of while (1) loop */
   } /* end of for (e=1; e<=pfi->dnum[4]; e++) loop */
   
   if (!quiet) printf (" grib1map:  reached end of files\n");
   
   /* check if file closed already for case where template was set,
      but it was not templated and the template code above closed it. */
   if (gfile!=NULL) {
     fclose (gfile);
     gfile=NULL;
   }
   
   /* open the map file */
   if (write_map) {
     mfile = fopen(pfi->mnam,"wb");
     if (mfile==NULL) {
       printf (" grib1map error: could not open index file: %s\n",pfi->mnam);
       return(1);
     } 
     else {
       if (!quiet) printf(" grib1map:  writing the map...\n\n");
       /* output the map depending on version # */
       if (g1ver==1) {
	 fwrite (pindx,sizeof(struct gaindx),1,mfile);
	 if (pindx->hinum>0)  fwrite(pindx->hipnt,sizeof(gaint),pindx->hinum,mfile);
	 if (pindx->hfnum>0)  fwrite(pindx->hfpnt,sizeof(gafloat),pindx->hfnum,mfile);
	 if (pindx->intnum>0) fwrite(pindx->intpnt,sizeof(gaint),pindx->intnum,mfile);
	 if (pindx->fltnum>0) fwrite(pindx->fltpnt,sizeof(gafloat),pindx->fltnum,mfile);
	 fclose (mfile);
       }
       else {
	 rc = wtgmap();
	 if (rc == 601) {
	   printf(" grib1map error: overflow in float -> IBM float conversion\n");
	   fclose (mfile);
	   return (601); 
	 }
	 fclose (mfile);
       }
     }
   }
   return (didmatch);
 }

#if GRIB2
 else /* GRIB2 */ {

   /* Set up g2index and initialize values */
   g2indx = (struct gag2indx *)malloc(sizeof(struct gag2indx));
   if (g2indx==NULL) {
     printf ("grib2map error: unable to allocate memory for g2indx\n");
     return(1);
   }
   g2indx->version = 1; 
   g2indx->g2intnum = ng2elems * pfi->trecs * pfi->dnum[3] * pfi->dnum[4];
   g2indx->g2intpnt = (gaint *)malloc(sizeof(gaint)*g2indx->g2intnum);
   if (g2indx->g2intpnt==NULL) {
     printf ("grib2map error: unable to allocate memory for g2indx->g2intpnt\n");
     goto err;
   }
   for (i=0; i<g2indx->g2intnum; i++) g2indx->g2intpnt[i] = -999;

   /* Break out point for case with E>1 but data files are only templated over T */
   if (pfi->dnum[4]>1 && pfi->tmplat==1) {
     /* Loop over all files in the data set */ 
     gfile=NULL;
     e=1;
     ens=pfi->ens1; 
     tcur = 0;
     while (1) {  /* loop over all times */
       /* make sure no file is open */
       if (gfile!=NULL) {
	 fclose(gfile);
	 gfile=NULL;
       }
       if (tcur==0) { /* first time step */
	 told = 0;
	 tcur = 1;
       }
       else {  /* tcur!=0 */
	 told = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	 /* increment time step until fnums changes */
	 while (told==pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] && tcur<=pfi->dnum[3]) {
	   tcur++;
	 }
       }
       /* make sure we haven't advanced past end of time axis */
       if (tcur>pfi->dnum[3]) break;
       
       
       /* Find the range of t indexes that have the same fnums value.
	  These are the times that are contained in this particular file */
       tmin = tcur;
       tmax = tcur-1;
       fnum = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
       if (fnum != -1) {
	 while (fnum == pfi->fnums[(e-1)*pfi->dnum[3]+tmax]) tmax++;
	 gr2t(pfi->grvals[3], (gadouble)tcur, &dtim); 
	 gr2t(pfi->grvals[3], ens->gt, &dtimi);
	 ch = gafndt(pfi->name, &dtim, &dtimi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,tcur,e,&flag);
	 if (ch==NULL) {
	   printf("grib2map error: couldn't determine data file name for e=%d t=%d\n",e,tcur);
	   goto err;
	 }
       }
       /* Open this GRIB file and position to start of first record (s/b subroutine) */
       if (!quiet) printf("grib2map: scanning GRIB2 file: %s \n",ch);
       gfile = fopen(ch,"rb");
       if (gfile==NULL) {
	 if (!quiet) printf ("grib2map warning: could not open GRIB file: %s\n",ch);
	 continue;
       }
       gree(ch,"f311a");
       /* Loop over fields in the grib file and find matches */
       iseek=0;
       record=1;
       while (1) {
	 /* move to next grib message in file */
	 seekgb(gfile,iseek,32000,&lskip,&lgrib);
	 if (lgrib == 0) break;    /* end loop at EOF or problem */
	 
	 /* read the message into memory */
	 cgrib = (unsigned char *)galloc(lgrib,"cgrib2");
	 if (cgrib == NULL) {
	   printf("grib2map error: unable to allocate memory for record %d at byte %ld\n",record,iseek); 
	   goto err;
	 }
	 ret = fseek(gfile,lskip,SEEK_SET);
	 lengrib = fread(cgrib,sizeof(unsigned char),lgrib,gfile);
	 if (lengrib < lgrib) {
	   printf("grib2map error: unable to read record %d at byte %ld\n",record,iseek); goto err;
	 }
	 
	 /* Get info about grib2 message */
	 ierr = 0;
	 ierr = g2_info(cgrib,listsec0,listsec1,&numfields,&numlocal);
	 if (ierr) {
	   printf("grib2map error: g2_info failed: ierr=%d\n",ierr); goto err;
	 }
	 for (n=0; n<numfields; n++) {
	   ierr = 0;
	   ierr = g2_getfld(cgrib,n+1,unpack,expand,&gfld);
	   if (ierr) {
	     printf("grib2map error: g2_getfld failed: ierr=%d\n",ierr); goto err;
	   }
	   
	   /* get statistical process type from grib field */
	   sp = g2sp(gfld);
	   
	   /* print out useful codes from grib2 field */
	   if (verb) g2prnt(gfld,record,n+1,sp);
	   
	   /* Check grid properties */
	   rc = g2grid_check(gfld,pfi,record,n+1);
	   if (rc) {
	     if (verb) printf("\n");
	     g2_free(gfld);   	   
	     break; 
	   }
	   
	   /* Check time values in grib field */
	   it = g2time_check(gfld,listsec1,pfi,record,n+1,tmin,tmax);
	   if (it==-99) {
	     if (verb) printf("\n");
	     g2_free(gfld);   	   
	     break;
	   }
	   it = (it-1)*pfi->trecs;  /* number of records per time */
	   
	   /* Check if the variable is a match */
	   ioff = g2var_match(gfld,pfi,sp);
	   if (ioff==-999) {
	     if (verb) printf("\n");
	     g2_free(gfld);   	   
	     break;
	   }
	   
	   /* check if ensemble codes match */
	   e = g2ens_match(gfld,pfi);
	   if (e==-999) {
	     if (verb) printf("\n");
	     g2_free(gfld);   	   
	     break;
	   }
	   eoff = (e-1)*pfi->dnum[3]*pfi->trecs;  /* number of records per ensemble */
	   
	   /* fill in the gribmap entry */
	   if (verb) printf("  MATCH \n");
	   g2fill (eoff,it+ioff,ng2elems,iseek,n+1,g2indx);
	   g2_free(gfld); 
	 }
	 /* free memory containing grib record */
	 gree(cgrib,"f310");
	 cgrib=NULL;
	 record++;                 /* increment grib record counter */
	 iseek = lskip+lgrib;      /* increment byte offset to next grib msg in file */
       }  /* end of while(1) loop over all fields in the grib message*/
     } /* end of while loop over all times */
     
   }
   else {
   /* All data sets except those that have E>1 but are templated only over T */

   /* Loop over all files in the data set */ 
   gfile=NULL;
   for (e=1,ens=pfi->ens1; e<=pfi->dnum[4]; e++,ens++) {
     tcur = 0;
     while (1) {  /* loop over all times for this ensemble */
       if (pfi->tmplat) {
	 /* make sure no file is open */
	 if (gfile!=NULL) {
	   fclose(gfile);
	   gfile=NULL;
	 }
	 /* advance to first valid time step for this ensemble */
	 if (tcur==0) {
	   told = 0;
	   tcur = 1;
	   while (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1) tcur++;  
	 }
	 else {  /* tcur!=0 */
	   told = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	   /* increment time step until fnums changes */
	   while (told==pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] && tcur<=pfi->dnum[3]) {
	     tcur++;
	   }
	 }
	 
	 /* make sure we haven't advanced past end of time axis */
	 if (tcur>pfi->dnum[3]) break;
	 
	 /* check if we're past all valid time steps for this ensemble */
	 if ((told != -1) && (pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1] == -1)) break;
	 
	 /* Find the range of t indexes that have the same fnums value.
	    These are the times that are contained in this particular file */
	 tmin = tcur;
	 tmax = tcur-1;
	 fnum = pfi->fnums[(e-1)*pfi->dnum[3]+tcur-1];
	 if (fnum != -1) {
	   while (fnum == pfi->fnums[(e-1)*pfi->dnum[3]+tmax]) tmax++;
	   gr2t(pfi->grvals[3], (gadouble)tcur, &dtim); 
	   gr2t(pfi->grvals[3], ens->gt, &dtimi);
	   ch = gafndt(pfi->name, &dtim, &dtimi, pfi->abvals[3], pfi->pchsub1, pfi->ens1,tcur,e,&flag);
	   if (ch==NULL) {
	     printf("grib2map error: couldn't determine data file name for e=%d t=%d\n",e,tcur);
	     goto err;
	   }
	 }
       }
       else {  
	 /* only one data file to open */
	 ch = pfi->name;
	 tmin = 1;
	 tmax = pfi->dnum[3];
       }
       
       /* Open this GRIB file and position to start of first record (s/b subroutine) */
       if (!quiet) printf("grib2map: scanning GRIB2 file: %s \n",ch);
       gfile = fopen(ch,"rb");
       if (gfile==NULL) {
	 if (pfi->tmplat) {
	   if (!quiet) printf ("grib2map warning: could not open GRIB file: %s\n",ch);
	   continue;
	 }
	 else {
	   printf ("grib2map error: could not open GRIB file: %s\n",ch);
	   goto err;
	 }
       }
       if (pfi->tmplat) gree(ch,"f311");
       
       /* Loop over fields in the grib file and find matches */
       iseek=0;
       record=1;
       while (1) {
	 /* move to next grib message in file */
	 seekgb(gfile,iseek,32000,&lskip,&lgrib);
	 if (lgrib == 0) break;    /* end loop at EOF or problem */
	 
	 /* read the message into memory */
	 cgrib = (unsigned char *)galloc(lgrib,"cgrib2");
	 if (cgrib == NULL) {
	   printf("grib2map error: unable to allocate memory for record %d at byte %ld\n",record,iseek); 
	   goto err;
	 }
	 ret = fseek(gfile,lskip,SEEK_SET);
	 lengrib = fread(cgrib,sizeof(unsigned char),lgrib,gfile);
	 if (lengrib < lgrib) {
	   printf("grib2map error: unable to read record %d at byte %ld\n",record,iseek); goto err;
	 }
	 
	 /* Get info about grib2 message */
	 ierr = 0;
	 ierr = g2_info(cgrib,listsec0,listsec1,&numfields,&numlocal);
	 if (ierr) {
	   printf("grib2map error: g2_info failed: ierr=%d\n",ierr); goto err;
	 }
	 for (n=0; n<numfields; n++) {
	   ierr = 0;
	   ierr = g2_getfld(cgrib,n+1,unpack,expand,&gfld);
	   if (ierr) {
	     printf("grib2map error: g2_getfld failed: ierr=%d\n",ierr); goto err;
	   }
	   
	   /* get statistical process type from grib field */
	   sp = g2sp(gfld);
	   
	   /* print out useful codes from grib2 field */
	   if (verb) g2prnt(gfld,record,n+1,sp);
	     
	   /* Check grid properties */
	   rc = g2grid_check(gfld,pfi,record,n+1);
	   if (rc) {
	     if (verb) printf("\n");
	     g2_free(gfld);   	   
	     break; 
	   }
	   
	   /* Check time values in grib field */
	   it = g2time_check(gfld,listsec1,pfi,record,n+1,tmin,tmax);
	   if (it==-99) {
	     if (verb) printf("\n");
	     g2_free(gfld);   	   
	     break;
	   }
	   it = (it-1)*pfi->trecs;  /* number of records per time */
	   
	   /* Check if the variable is a match */
	   ioff = g2var_match(gfld,pfi,sp);
	   if (ioff==-999) {
	     if (verb) printf("\n");
	     g2_free(gfld);   	   
	     break;
	   }
			    
	   if (pfi->tmplat) {
	     /* make sure grib codes match for this ensemble */
	     rc = g2ens_check(ens,gfld);
	     if (rc==1) {
	       if (verb) printf("\n");
	       g2_free(gfld);   	   
	       break;
	     }
	   } 
	   else {
	     /* check if ensemble codes match */
	     e = g2ens_match(gfld,pfi);
	     if (e==-999) {
	       if (verb) printf("\n");
	       g2_free(gfld);   	   
	       break;
	     }
	   }
	   eoff = (e-1)*pfi->dnum[3]*pfi->trecs;  /* number of records per ensemble */

	   /* fill in the gribmap entry */
	   if (verb) printf("  MATCH \n");
	   g2fill (eoff,it+ioff,ng2elems,iseek,n+1,g2indx);
	   g2_free(gfld); 

	 }
	 /* free memory containing grib record */
	 gree(cgrib,"f310");
 	 cgrib=NULL;
	 record++;                 /* increment grib record counter */
	 iseek = lskip+lgrib;      /* increment byte offset to next grib msg in file */

       }  /* end of while(1) loop over all fields in the grib message*/

       /* break out if not templating -- only need to scan one grib file */
       if (!pfi->tmplat) goto done;

     } /* end of while(1) loop over all grib files for a given ensemble member*/
   } /* end of loop over ensemble members: for (e=1,ens=pfi->ens1; e<=pfi->dnum[4]; e++,ens++) */
   } /* end of else statement for if (pfi->dnum[4]>1 && pfi->tmplat==1)  */
   
   if (!quiet) printf ("grib2map: reached end of files\n");


done:   
   /* check if file not closed */
   if (gfile!=NULL) {
     fclose (gfile);
     gfile=NULL;
   }
   
   /* Write out the index file */
   if (write_map) {
     rc=wtg2map(pfi,g2indx);
     if (rc) return (rc);
   }
   return(0);
   
err: 
   if (g2indx->g2intpnt) gree(g2indx->g2intpnt,"f314");
   if (g2indx) gree(g2indx,"f315");
   if (cgrib) gree(cgrib,"f316");
   return(1);
 }

#endif  /* matches #if GRIB2 */

}



/* Routine to read a GRIB header and process info */

gaint gribhdr (struct grhdr *ghdr) {
 struct dt atim;
 unsigned char rec[50000],*ch,*gds;
 gaint i,len ,rc,sign,mant;
 off_t cpos;
 
 if (fpos+50>=flen) return(1);

 /* look for data between records BY DEFAULT */ 
 i = 0;
 fpos += i;
 rc = fseek(gfile,fpos,0);
 if (rc) return(50);
 ch=&rec[0];
 rc = fread(ch,sizeof(char),4,gfile);
 while ((fpos < flen-4) && (i < scanlim) && 
	!(*(ch+0)=='G' && 
	  *(ch+1)=='R' &&
	  *(ch+2)=='I' &&
	  *(ch+3)=='B')) {
   fpos++;
   i++;
   rc = fseeko(gfile,fpos,0);
   if (rc) return(50);
   rc = fread(ch,sizeof(char),4,gfile);
   if (rc<4) return(50);
 } 
 
 if (i == scanlim) {
   printf("grib1map error: GRIB header not found in scanning between records\n");
   printf("                try increasing the value of the -s argument\n");

   if (scaneof) return(98);
   if (scanEOF) return(0);
   return(52);
 } 
 else if (fpos == flen-4) {
   if (scaneof) return(98);
   if (scanEOF) return(0);
   return (53);
 }
 
 /* SUCCESS redo the initial read */    
 rc = fseek(gfile,fpos,0);
 if (rc) return(50);
 rc = fread(rec,1,8,gfile);
 if (rc<8) {
   if (fpos+8 >= flen) return(61);
   else return(62);
 }
 
 cpos = fpos;
 ghdr->vers = gagby(rec,7,1);
 if (ghdr->vers>1) {
   printf ("grib1map error: file is not GRIB version 0 or 1, version number is %i\n",ghdr->vers);
   if (scaneof) return(98);
   return (99);
 }
 
 if (ghdr->vers==0) {
   cpos += 4;
   rc = fseek(gfile,cpos,0);
   if (rc) return(50);
 } else {
   ghdr->len = gagby(rec,4,3);
   cpos = cpos + 8;
   rc = fseeko(gfile,cpos,0);
   if (rc) return(50);
 }
 
 /* Get PDS length, read rest of PDS */
 rc = fread(rec,1,3,gfile);
 if (rc<3) return(50);
 len = gagby(rec,0,3);
 ghdr->pdslen = len;
 cpos = cpos + len;
 rc = fread(rec+3,1,len-3,gfile);
 if (rc<len-3) return(50);
 
 /* Get info from PDS */
 ghdr->id = gagby(rec,6,1);
 ghdr->gdsflg = gagbb(rec+7,0,1);
 ghdr->bmsflg = gagbb(rec+7,1,1);
 ghdr->parm = gagby(rec,8,1);
 ghdr->ltyp = gagby(rec,9,1);
 ghdr->level = gagby(rec,10,2);
 ghdr->l1 = gagby(rec,10,1);
 ghdr->l2 = gagby(rec,11,1);
 if (mpiflg) {                 
   /* use initial time from the descriptor file instead of base time from grib header */
   ghdr->btim.yr = *(pfi->abvals[3]);
   ghdr->btim.mo = *(pfi->abvals[3]+1);
   ghdr->btim.dy = *(pfi->abvals[3]+2);
   ghdr->btim.hr = *(pfi->abvals[3]+3);
   ghdr->btim.mn = *(pfi->abvals[3]+4);
   if (no_min) ghdr->btim.mn = 0;
 } else {
   ghdr->btim.yr = gagby(rec,12,1);
   ghdr->btim.mo = gagby(rec,13,1);
   ghdr->btim.dy = gagby(rec,14,1);
   ghdr->btim.hr = gagby(rec,15,1);
   ghdr->btim.mn = gagby(rec,16,1);
   if (no_min) ghdr->btim.mn = 0;
 }
 if (ghdr->btim.hr>23) ghdr->btim.hr = 0;  /* Special for NCAR */
 if (len>24) {
   ghdr->cent = gagby(rec,24,1);
   ghdr->btim.yr = ghdr->btim.yr + (ghdr->cent-1)*100;
 } else {
   ghdr->cent = -999;
   if (!(mpiflg) || !(mfcmn.fullyear)) {
     if (ghdr->btim.yr>49) ghdr->btim.yr += 1900;
     if (ghdr->btim.yr<50) ghdr->btim.yr += 2000;
   }
 }
 ghdr->ftu = gagby(rec,17,1);
 ghdr->tri = gagby(rec,20,1);
 if (ghdr->tri==10) {
   ghdr->p1 = gagby(rec,18,2);
   ghdr->p2 = 0;
 } else {
   ghdr->p1 = gagby(rec,18,1);
   ghdr->p2 = gagby(rec,19,1);
 }
 
 ghdr->fcstt = ghdr->p1;
 if (ghdr->tri>1 && ghdr->tri<6) 
   ghdr->fcstt=ghdr->p2;
 if ((tauave) && ghdr->tri==3) 
   ghdr->fcstt=ghdr->p1;
 atim.yr=0; atim.mo=0; atim.dy=0; atim.hr=0; atim.mn=0;
 if      (ghdr->ftu== 0) atim.mn = ghdr->fcstt;
 else if (ghdr->ftu== 1) atim.hr = ghdr->fcstt;
 else if (ghdr->ftu==10) atim.hr = ghdr->fcstt*3;   /* added 3Hr incr */
 else if (ghdr->ftu==11) atim.hr = ghdr->fcstt*6;   /* added 6Hr incr */  
 else if (ghdr->ftu==12) atim.hr = ghdr->fcstt*12;  /* added 12Hr incr */
 else if (ghdr->ftu== 2) atim.dy = ghdr->fcstt;
 else if (ghdr->ftu== 3) atim.mo = ghdr->fcstt;
 else if (ghdr->ftu== 4) atim.yr = ghdr->fcstt;
 else ghdr->fcstt = -999;

 /*  if notau != 0 then FORCE the valid DTG to be the base DTG */ 
 if (notau) ghdr->fcstt = -999 ;
 
 /*  add the forecast time to the time of this grib field */
 if (ghdr->fcstt>-900) {
   timadd(&(ghdr->btim),&atim);
   ghdr->dtim.yr = atim.yr;
   ghdr->dtim.mo = atim.mo;
   ghdr->dtim.dy = atim.dy;
   ghdr->dtim.hr = atim.hr;
   ghdr->dtim.mn = atim.mn;
 } else {
   ghdr->dtim.yr = ghdr->btim.yr;
   ghdr->dtim.mo = ghdr->btim.mo;
   ghdr->dtim.dy = ghdr->btim.dy;
   ghdr->dtim.hr = ghdr->btim.hr;
   ghdr->dtim.mn = ghdr->btim.mn;
 }
 if (len>25) {
   ghdr->dsf = (gafloat)gagbb(rec+26,1,15);
   i = gagbb(rec+26,0,1);
   if (i) ghdr->dsf = -1.0*ghdr->dsf;
   ghdr->dsf = pow(10.0,ghdr->dsf);
 } else ghdr->dsf = 1.0;
 
 /* If it is there, get info from GDS */
 if (ghdr->gdsflg) {
   rc = fread(rec,1,3,gfile);
   if (rc<3) return(50);
   len = gagby(rec,0,3);
   ghdr->gdslen = len;
   cpos = cpos + len;
   
   /* handle generic grid where the lon/lats are coded from the GDS */
   gds = (unsigned char *)malloc(len+3);
   if (gds==NULL) return(51);
   rc = fread(gds+3,1,len-3,gfile);
   if (rc<len-3) return(50);
   ghdr->gtyp  = gagby(gds,4,1);
   ghdr->gicnt = gagby(gds,6,2);
   ghdr->gjcnt = gagby(gds,8,2);
   ghdr->gsf1  = gagbb(gds+27,0,1);
   ghdr->gsf2  = gagbb(gds+27,1,1);
   ghdr->gsf3  = gagbb(gds+27,2,1);
   free(gds);
 } 
 else ghdr->gdslen = 0;

 /* Get necessary info about BMS if it is there */
 if (ghdr->bmsflg) {
   rc = fread(rec,1,6,gfile);
   if (rc<6) return(50);
   len = gagby(rec,0,3);
   ghdr->bmsflg = len;
   ghdr->bnumr = gagby(rec,4,2);
   ghdr->bpos = cpos+6;
   cpos = cpos + len;
   rc = fseeko(gfile,cpos,0);
 } 
 else ghdr->bmslen = 0;

 /* Get necessary info from data header */
 rc = fread(rec,1,11,gfile);
 if (rc<11) return(50);
 len = gagby(rec,0,3);
 ghdr->bdslen = len;
 ghdr->iflg = gagbb(rec+3,0,2);
 i = gagby(rec,4,2);
 if (i>32767) i = 32768-i;
 ghdr->bsf = pow(2.0,(gafloat)i);
 
 i = gagby(rec,6,1);
 sign = 0;
 if (i>127) {
   sign = 1;
   i = i - 128;
 }
 mant = gagby(rec,7,3);
 if (sign) mant = -mant;
 ghdr->ref = (gafloat)mant * pow(16.0,(gafloat)(i-70));
 
 ghdr->bnum = gagby(rec,10,1);
 ghdr->dpos = cpos+11;

 if (ghdr->vers==0) {
   fpos = fpos + 8 + ghdr->pdslen + ghdr->gdslen +
     ghdr->bmslen + ghdr->bdslen;
 } 
 else fpos = fpos + ghdr->len;
 
 return(0);
 
}

/* Routine to determine the location of the GRIB record in terms of the GrADS data set
   and fill in the proper values at the proper slot location. */

gaint gribrec (struct grhdr *ghdr, struct gafile *pfi, struct gaindx *pindx, 
	     gaint tmin, gaint tmax, gaint e) {
 gadouble (*conv) (gadouble *, gadouble);
 gadouble z,t;
 struct gavar *pvar;
 gaint i,ioff,iz,it,joff,nsiz,flag,eoff;
 
 /* Verify that we are looking at the proper grid type */
 joff =0;
 nsiz = nrec * ng1elems ;
 if (ghdr->iflg) {
   if (verb) {
     printf ("GRIB record contains harmonic or complex packing\n");
     printf ("  Record is skipped.\n");
     printf ("  Variable is %i\n",ghdr->parm);
   }
   return(10);
 }
 if (pfi->grbgrd==255 || pfi->grbgrd<-900) {
   if (!ghdr->gdsflg) {
     if (verb) {
       printf ("GRIB record contains pre-defined grid type: "); 
       printf ("GrADS descriptor specifies type 255\n");
       gribpr(ghdr);
     }
     return(20);
   } 
   if ( pfi->ppflag) {
     if ( ghdr->gicnt != 65535 && 
	  ((ghdr->gicnt != pfi->ppisiz) || (ghdr->gjcnt != pfi->ppjsiz)) ) {
       if (verb) {
	 printf ("GRIB grid size does not match descriptor: "); 
	 gribpr(ghdr);
       }
       return(300);
     }
   } else {
     if (ghdr->gicnt != 65535 && 
	 ((ghdr->gicnt != pfi->dnum[0]) || (ghdr->gjcnt != pfi->dnum[1]))) {
       if (verb) {
	 printf ("GRIB grid size does not match descriptor:");
	 gribpr(ghdr);
       }
       return(301);
     }
   }
 } 
 else {
   /* special case for GRIB grid number (dtype grib NNN) == 29 */
   if (pfi->grbgrd==29) {
     if (ghdr->id!=29 && ghdr->id!=30) {
       if (verb) {
	 printf("Record has wrong GRIB grid type: ") ; 
	 gribpr(ghdr);
       }
       return(400);     
     }
     if (ghdr->id==29) joff = ng1elems;
     nsiz = 2 * ng1elems ;
   } else {
     if (ghdr->id != pfi->grbgrd) {
       if (verb) {
	 printf("%s","Record has wrong GRIB grid type: "); 
	 gribpr(ghdr);
       }
       return(401);     
     }
   }
 }
 
 /* Calculate the grid time for this record.  
    If it is non-integer or if it is out of bounds, just return. */

 /* Check for given forecast time, tauoff (the -fhr switch) */
 if (tauflg && (ghdr->ftu==1 && ghdr->fcstt!=tauoff)) {
   if (verb) {
     printf("%s %d","--f-- Forecast Time does not match : ",tauoff);
     gribpr(ghdr);
   }
   return(32);
 }
 
 /* Check if base time in grib record matches initial time in descriptor file (the -t0 switch) */
 if (tau0 &&
     ((ghdr->btim.yr != btimdd.yr) ||
      (ghdr->btim.mo != btimdd.mo) ||
      (ghdr->btim.dy != btimdd.dy) ||
      (ghdr->btim.hr != btimdd.hr) ||
      (ghdr->btim.mn != btimdd.mn))) {
   if (verb) {
     printf("%s","--b-- Base Time does not match Initial Time in DD: "); 
     gribpr(ghdr);
   }
   return(34);
 }

 /* Check if valid time is within grid time limits */
 t = t2gr(pfi->abvals[3],&(ghdr->dtim));
 if (t<0.99 || t>((gafloat)(pfi->dnum[3])+0.01)) {
   if (verb) {
     printf("%s","----- Time out of bounds: "); 
     gribpr(ghdr);
   }
   return(36);
 }

 /* Check if valid time is an integer */
 it = (gaint)(t+0.01);
 if (fabs((gafloat)it - t)>0.01) {
   if (verb) {
     printf("----- Time non-integral. %g %g: ",(gafloat)it,t);  
     gribpr(ghdr);
   }
   return(38);
 }

 /* Check if valid time matches range of times for this file  */
 if (it<tmin || it>tmax) {
   if (verb) {
     printf("----- Time out of file limits: ");  
     gribpr(ghdr);
   }
   return(39);
 }
 it = (it-1)*pfi->trecs;
 eoff = (e-1)*pfi->dnum[3]*pfi->trecs;  /* number of records per ensemble */

 /* See if we can match up this grid with a variable in the data descriptor file */
 pvar = pfi->pvar1;
 i = 0;
 flag=0;
 while (i<pfi->vnum) {
   if (pvar->levels>0) {      /* multi level data */
     if (dequal(pvar->units[0],ghdr->parm,1e-8)==0 && 
	 dequal(pvar->units[8],ghdr->ltyp,1e-8)==0) {
       /* look for time range indicator match */
       if (pvar->units[10] < -900 || dequal(pvar->units[10],ghdr->tri,1e-8)==0) {
	 conv = pfi->ab2gr[2];
	 z = conv(pfi->abvals[2],ghdr->level);
	 if (z>0.99 && z<((gafloat)(pvar->levels)+0.01)) {
	   iz = (gaint)(z+0.5);
	   /* check if levels match */
	   if (fabs(z-(gafloat)iz) < 0.01) {
	     iz = (gaint)(z+0.5);
	     ioff = pvar->recoff + iz - 1;
	     gribfill (eoff,it+ioff,joff,nsiz,ghdr,pindx);
	     flag=1;
	     i = pfi->vnum + 1;   /* Force loop to stop */
	   }
	 }
       }
     }
   } 
   else {       /* sfc data */
     if (dequal(pvar->units[0],ghdr->parm,1e-8)==0 && dequal(pvar->units[8],ghdr->ltyp,1e-8)==0) {
       if ((pvar->units[10] < -900 && 
	    dequal(pvar->units[9],ghdr->level,1e-8)==0) ||
	   (pvar->units[10] > -900 && 
	    dequal(pvar->units[9],ghdr->l1,1e-8)==0 && dequal(pvar->units[10],ghdr->l2,1e-8)==0) || 
	   (dequal(pvar->units[10],ghdr->tri,1e-8)==0 && dequal(pvar->units[9],ghdr->level,1e-8)==0)) {
	 ioff = pvar->recoff;
	 gribfill (eoff,it+ioff,joff,nsiz,ghdr,pindx);
	 i = pfi->vnum+1;  /* Force loop to stop */
	 flag=1;
       }
     }
   }
   pvar++; i++;
 }
 
 if (flag && verb) printf("!!!!! MATCH: "); 
 if (!flag && verb) printf("..... NOOOO: "); 
 if (verb) gribpr(ghdr); 
 
 return (flag ? 0 : 1);
 
}


/* Routine to fill in values for this record, now that we have found how it matches.  
   We are not handling the time aspect as yet */

void gribfill (gaint eoff, gaint ioff, gaint joff, gaint nsiz, struct grhdr *ghdr, struct gaindx *pindx) {
  ioff = nsiz*(eoff+ioff) + joff;
  *(pindx->intpnt+ioff) = ghdr->dpos;
  if (ghdr->bmsflg) *(pindx->intpnt+ioff+1) = ghdr->bpos;
  *(pindx->intpnt+ioff+2) = ghdr->bnum;
  *(pindx->fltpnt+ioff)   = ghdr->dsf;
  *(pindx->fltpnt+ioff+1) = ghdr->bsf;
  *(pindx->fltpnt+ioff+2) = ghdr->ref;
}


/* Routine to print out fields from the grib header */

void gribpr(struct grhdr *ghdr) {
  printf ("% 5i % 10ld % 3i % 1i % 5i % 4i % 4i %-5i % 10i % 10i % 3i ",
	  irec,fpos,ghdr->id,ghdr->gdsflg,ghdr->bmsflg,ghdr->parm,ghdr->ltyp,
	  ghdr->level,ghdr->dpos,ghdr->bpos,ghdr->bnum);
  printf ("btim: %04i%02i%02i%02i:%02i ",ghdr->btim.yr,
	  ghdr->btim.mo,ghdr->btim.dy,ghdr->btim.hr,ghdr->btim.mn);
  printf ("tau: % 6i ",ghdr->fcstt);
  printf ("dtim: %04i%02i%02i%02i:%02i ",ghdr->dtim.yr,
	  ghdr->dtim.mo,ghdr->dtim.dy,ghdr->dtim.hr,ghdr->dtim.mn);
  printf("\n");
}


/* Routine to write out machine independent grib1 map file */

gaint wtgmap(void) {
gaint i,nb,bcnt,idum;
gafloat fdum;
unsigned char *map;
unsigned char ibmfloat[4];
 
 /* calculate the size of the version==1 index file */
 nb = 2 + (4*4) +         /* version in byte 2, then 4 ints with number of each data type */
   pindx->hinum*sizeof(gaint) +
   pindx->hfnum*sizeof(gaint) +
   pindx->intnum*sizeof(gaint) +
   pindx->fltnum*sizeof(gafloat) ;
 
 /* add additional info */
 if (g1ver==2) {
   nb=nb+7;      /* base time (+ sec)  for compatibility with earlier version 2 maps */
   nb=nb+8*4;    /* grvals for time <-> grid conversion */
 }

 /* allocate space for the map */
 map = (unsigned char *)malloc(nb);
 if (map == NULL) {
   fprintf(stderr,"grib1map error: memory allocation error creating the map\n");
   return(60);
 }

 /* write out the version number and the sizes of the header and index arrays */
 bcnt=0;
 gapby(0,map,bcnt,1);      bcnt++  ;   /* set the first byte to 0 */
 gapby(g1ver,map,bcnt,1);  bcnt++  ;   /* set the second byte to the version number */
 putint(pindx->hinum,map,&bcnt);              /* # ints in header   */
 putint(pindx->hfnum,map,&bcnt);              /* # floats in header   */
 putint(pindx->intnum,map,&bcnt);             /* # index ints   */
 putint(pindx->fltnum,map,&bcnt);             /* # index floats   */
 
 if (g1ver==2) {
   /* write out base time for consistency with earlier version 2 maps */
   /* base time not needed for version 3 */
   gapby(btimdd.yr,map,bcnt,2);  bcnt+=2 ;   /* initial year */
   gapby(btimdd.mo,map,bcnt,1);  bcnt++  ;   /* initial month */ 
   gapby(btimdd.dy,map,bcnt,1);  bcnt++  ;   /* initial day */
   gapby(btimdd.hr,map,bcnt,1);  bcnt++  ;   /* initial hour */
   gapby(btimdd.mn,map,bcnt,1);  bcnt++  ;   /* initial minute */
   gapby(0,map,bcnt,1);          bcnt++  ;   /* initial second */
 } 

 /* write the header */
 if (pindx->hinum) {
   for (i=0;i<pindx->hinum;i++) {
     idum=*(pindx->hipnt+i);
     putint(idum,map,&bcnt);
   }
 }

 /* write the indices */
 for (i=0;i<pindx->intnum;i++) {
   idum=*(pindx->intpnt+i);
   putint(idum,map,&bcnt);
 }
 for (i=0;i<pindx->fltnum;i++) {
   fdum=*(pindx->fltpnt+i);
   rc=flt2ibm(fdum, ibmfloat); 
   if (rc<0) return(601);
   memcpy(&map[bcnt],ibmfloat,4); bcnt+=4;
 }
 
 if (g1ver==2) {
   /* write out the factors for converting from grid to absolute time */ 
   /* the conversion vals are not needed for version 3 */
   for (i=0;i<8;i++) {
     fdum=*(pfi->grvals[3]+i);
     rc=flt2ibm(fdum, ibmfloat); 
     if (rc<0) return(601);
     memcpy(&map[bcnt],ibmfloat,4); bcnt+=4;
   }
 } 

 /* write to the map file */
 fwrite(map,1,bcnt,mfile);
 free(map);
 return(0);

}

/* Routine to dump a 4 byte int into a character stream */

void putint(gaint dum, unsigned char *buf, gaint *off) {
 gaint offset;

 offset=*off;
 if (dum < 0) {
   dum=-dum;
   gapby(dum,buf,offset,4);
   gapbb(1,buf+offset,0,1);
 } else {
   gapby(dum,buf,offset,4);
 }
 offset+=4;
 *off=offset;
 
}


#if GRIB2

/* Routine to fill in values for grib2 record, now that we know it matches. */
void g2fill (gaint eoff, gaint ioff, gaint ng2elems, g2int iseek, g2int fldnum, 
		struct gag2indx *g2indx) {
  ioff = ng2elems*(eoff+ioff);
  *(g2indx->g2intpnt+ioff+0) = iseek;
  *(g2indx->g2intpnt+ioff+1) = fldnum;
}

/* Routine to write out grib2 index file 

     g2ver=1 : machine dependent. contains the version number, followed by 
               the array size N, followed by the array of N numbers. 
               All are 4-byte integers (type gaint). 

     A test to see if byte-swapping is required	to read the index file is done
     in gaddes.c, when the data descriptor file is opened. 
*/


gaint wtg2map(struct gafile *pfi, struct gag2indx *g2indx) {
  FILE *mfile;
  gaint rc;
  
  /* open the index file */
  mfile = fopen(pfi->mnam,"wb");
  if (mfile==NULL) {
    printf ("error: Unable to open index file: %s\n",pfi->mnam);
    return(1);
  } 
  
  printf("grib2map: Writing out the index file \n");
  /* write the version number */
  rc = fwrite(&g2indx->version, sizeof(gaint),1,mfile);
  if (rc!=1) {
    printf("error: Unable to write version number to index file, rc=%d \n",rc);
    return(1);
  }  
  /* write the array size */
  rc = fwrite(&g2indx->g2intnum,sizeof(gaint),1,mfile);
  if (rc!=1) {
    printf("error: Unable to write g2intnum to index file, rc=%d \n",rc);
    return(1);
  }  
  /* writhe the array of index values */
  rc = fwrite(g2indx->g2intpnt,sizeof(gaint),g2indx->g2intnum,mfile);
  if (rc!=g2indx->g2intnum) {
    printf("error: Unable to write g2intpnt to index file, rc=%d \n",rc);
    return(1);
  }  
  fclose(mfile);
  return(0);
  
}

/* Checks grid properties for a grib2 field. 
   Returns 0 if ok, 1 if doesn't match descriptor */

gaint g2grid_check (gribfield *gfld, struct gafile *pfi, gaint r, gaint f) {
gaint xsize=0,ysize=0;

  /* Check total number of grid points */
  if (pfi->grbgrd==255 || pfi->grbgrd<-900) {
    if (((pfi->ppflag) && (gfld->ngrdpts != pfi->ppisiz * pfi->ppjsiz)) ||
        ((pfi->ppflag==0) && (gfld->ngrdpts != pfi->dnum[0] * pfi->dnum[1]))) {
      if (verb) printf ("number of grid points does not match descriptor ");
      return(1);
    }
  } 
  /* Check nx and ny for Lat/Lon, Polar Stereographic, and Lambert Conformal grids */
  if (pfi->ppflag) {
    xsize = pfi->ppisiz;
    ysize = pfi->ppjsiz;
  } else {
    xsize = pfi->dnum[0];
    ysize = pfi->dnum[1];
  }
  if (gfld->igdtmpl[7] != -1) {
    if (gfld->igdtnum==0 || gfld->igdtnum==40 || gfld->igdtnum==20 || gfld->igdtnum==30) {
      if (gfld->igdtmpl[7] != xsize) {
	if (verb) printf ("x dimensions are not equal: nx=%d xsize=%d",gfld->igdtmpl[7],xsize); 
	return(1);
      } 
      if (gfld->igdtmpl[8] != ysize) {
	if (verb) printf ("y dimensions are not equal: nx=%d xsize=%d",gfld->igdtmpl[8],ysize); 
	return(1);
      }
    }
  }
  return(0);
}

/* Checks time metadata in grib2 message. 
   Returns integer value of time axis index if ok, -99 if not */
gaint g2time_check (gribfield *gfld, g2int *listsec1, struct gafile *pfi, 
		    gaint r, gaint f, gaint tmin, gaint tmax) {
  struct dt tref,tfld,tvalid;
  gaint it,tfield;
  gafloat t;

  /* Get reference time from Section 1 of GRIB message */
  tref.yr = listsec1[5];
  tref.mo = listsec1[6];
  tref.dy = listsec1[7];
  tref.hr = listsec1[8];
  tref.mn = listsec1[9];
  tfield = tfld.yr = tfld.mo = tfld.dy = tfld.hr = tfld.mn = 0;  /* initialize */
 	 
  if (notau) {
    /* use reference time as valid time */
    tvalid.yr = tref.yr;
    tvalid.mo = tref.mo;
    tvalid.dy = tref.dy;
    tvalid.hr = tref.hr;
    tvalid.mn = tref.mn;
  }
  else {
    /* For fields at a point in time (PDT<8) */
    printf("JMA(gagmap): gfld->ipdtnum=%d\n",gfld->ipdtnum);
    if (gfld->ipdtnum < 8) {
      if      (gfld->ipdtmpl[7]== 0) tfld.mn = gfld->ipdtmpl[8];     
      else if (gfld->ipdtmpl[7]== 1) tfld.hr = gfld->ipdtmpl[8];
      else if (gfld->ipdtmpl[7]== 2) tfld.dy = gfld->ipdtmpl[8];
      else if (gfld->ipdtmpl[7]== 3) tfld.mo = gfld->ipdtmpl[8];
      else if (gfld->ipdtmpl[7]== 4) tfld.yr = gfld->ipdtmpl[8];	 
      else if (gfld->ipdtmpl[7]==10) tfld.hr = gfld->ipdtmpl[8]*3;   /* 3Hr incr */
      else if (gfld->ipdtmpl[7]==11) tfld.hr = gfld->ipdtmpl[8]*6;   /* 6Hr incr */  
      else if (gfld->ipdtmpl[7]==12) tfld.hr = gfld->ipdtmpl[8]*12;  /* 2Hr incr */
      else tfield=-99;
      if (tfield==-99) {
	/* use reference time as valid time */
	tvalid.yr = tref.yr;
	tvalid.mo = tref.mo;
	tvalid.dy = tref.dy;
	tvalid.hr = tref.hr;
	tvalid.mn = tref.mn;
      }
      else {
	/* add forecast time to reference time to get valid time */
	timadd(&tref,&tfld);
	tvalid.yr = tfld.yr;
	tvalid.mo = tfld.mo;
	tvalid.dy = tfld.dy;
	tvalid.hr = tfld.hr;
	tvalid.mn = tfld.mn;
      }
    }
    /* For fields that are statistically processed over a time interval 
       e.g. averages, accumulations, extremes, et al. (8<=PDT<15) 
       valid time is the end of the overall time interval */
    else if (gfld->ipdtnum == 8) {
      tvalid.yr = gfld->ipdtmpl[15];
      tvalid.mo = gfld->ipdtmpl[16];
      tvalid.dy = gfld->ipdtmpl[17];
      tvalid.hr = gfld->ipdtmpl[18];
      tvalid.mn = gfld->ipdtmpl[19];
    }
/*     else if (gfld->ipdtnum == 9) { */
/*       tvalid.yr = gfld->ipdtmpl[22]; */
/*       tvalid.mo = gfld->ipdtmpl[23]; */
/*       tvalid.dy = gfld->ipdtmpl[24]; */
/*       tvalid.hr = gfld->ipdtmpl[25]; */
/*       tvalid.mn = gfld->ipdtmpl[26]; */
/*     } */
/*     else if (gfld->ipdtnum == 10) { */
/*       tvalid.yr = gfld->ipdtmpl[16]; */
/*       tvalid.mo = gfld->ipdtmpl[17]; */
/*       tvalid.dy = gfld->ipdtmpl[18]; */
/*       tvalid.hr = gfld->ipdtmpl[19]; */
/*       tvalid.mn = gfld->ipdtmpl[20]; */
/*     } */
    else if (gfld->ipdtnum == 11) {
      tvalid.yr = gfld->ipdtmpl[18];
      tvalid.mo = gfld->ipdtmpl[19];
      tvalid.dy = gfld->ipdtmpl[20];
      tvalid.hr = gfld->ipdtmpl[21];
      tvalid.mn = gfld->ipdtmpl[22];
    }
    else if (gfld->ipdtnum == 12) {
      tvalid.yr = gfld->ipdtmpl[17];
      tvalid.mo = gfld->ipdtmpl[18];
      tvalid.dy = gfld->ipdtmpl[19];
      tvalid.hr = gfld->ipdtmpl[20];
      tvalid.mn = gfld->ipdtmpl[21];
    }
    /*   else if (gfld->ipdtnum == 13) { */
    /*     tvalid.yr = gfld->ipdtmpl[31]; */
    /*     tvalid.mo = gfld->ipdtmpl[32]; */
    /*     tvalid.dy = gfld->ipdtmpl[33]; */
    /*     tvalid.hr = gfld->ipdtmpl[34]; */
    /*     tvalid.mn = gfld->ipdtmpl[35]; */
    /*   } */
    /*   else if (gfld->ipdtnum == 14) { */
    /*     tvalid.yr = gfld->ipdtmpl[30]; */
    /*     tvalid.mo = gfld->ipdtmpl[31]; */
    /*     tvalid.dy = gfld->ipdtmpl[32]; */
    /*     tvalid.hr = gfld->ipdtmpl[33]; */
    /*     tvalid.mn = gfld->ipdtmpl[34]; */
    /*   } */
    else {
      printf("Product Definition Template %ld not handled \n",gfld->ipdtnum);
      return(-99);
    }  
  }
  /* Check if valid time is within grid limits */
  t = t2gr(pfi->abvals[3],&tvalid);
  if (t<0.99 || t>((gafloat)(pfi->dnum[3])+0.01)) {
    printf("valid time %4d%02d%02d%02d:%02d (t=%g) is outside grid limits",
	   tvalid.yr,tvalid.mo,tvalid.dy,tvalid.hr,tvalid.mn,t);
    return(-99);
  }
  /* Check if valid time is an integer */
  it = (gaint)(t+0.01);
  if (fabs((gafloat)it - t)>0.01) {
    printf("valid time %4d%02d%02d%02d:%02d (t=%g) has non-integer grid index",
	   tvalid.yr,tvalid.mo,tvalid.dy,tvalid.hr,tvalid.mn,t);
    return(-99);
  }
  /* Check if valid time matches range of times for this file  */
  if (it<tmin || it>tmax) {
    printf("valid time %4d%02d%02d%02d:%02d (t=%s) is outside file limits (%d-%d)",
	   tvalid.yr,tvalid.mo,tvalid.dy,tvalid.hr,tvalid.mn,it,tmin,tmax);
    return(-99);
  }
  return (it);
}

/* Loops over variables in descriptor file, looking for match to current grib2 field. 
   If variables match, returns offset, if not, returns -999 */

gaint g2var_match (gribfield *gfld, struct gafile *pfi, gaint sp) {
  struct gavar *pvar;
  gadouble lev1,lev2,z;
  gadouble (*conv) (gadouble *, gadouble);
  gaint rc1,rc2,rc3,rc4,rc5;
  gaint i,ioff,iz;
  
  /* Get level values from grib field */
  lev1 = scaled2dbl(gfld->ipdtmpl[10],gfld->ipdtmpl[11]);
  /* Check if we've got info on 2nd level */
  if (gfld->ipdtmpl[12] != 255) 
    lev2 = scaled2dbl(gfld->ipdtmpl[13],gfld->ipdtmpl[14]);
  else 
    lev2 = -999;
  
  /* See if we match any variables in the descriptor file */
  pvar = pfi->pvar1;
  ioff = -999;
  i = 0;
  while (i<pfi->vnum) {
    if (pvar->levels>0) {      
      /* Z-varying data */
      rc1 = dequal(pvar->units[0],(gadouble)gfld->discipline,1e-8); /* discipline */
      rc2 = dequal(pvar->units[1],(gadouble)gfld->ipdtmpl[0],1e-8); /* category   */
      rc3 = dequal(pvar->units[2],(gadouble)gfld->ipdtmpl[1],1e-8); /* number     */
      rc4 = dequal(pvar->units[3],(gadouble)sp,1e-8);		    /* SP         */
      rc5 = dequal(pvar->units[8],(gadouble)gfld->ipdtmpl[9],1e-8); /* LTYPE1     */
      if (rc1==0 && rc2==0 && rc3==0 && rc4==0 && rc5==0) {   /* all the above match */
	
	/* get a Z value for level 1 */
	conv = pfi->ab2gr[2];
	z = conv(pfi->abvals[2],lev1);
	if (z>0.99 && z<((gadouble)(pvar->levels)+0.01)) {
	  iz = (gaint)(z+0.5);
	  /* make sure Z value for level 1 is an integer */
	  if (fabs(z-(gadouble)iz) < 0.01) {
	    ioff = pvar->recoff + iz - 1;
	    return(ioff); 
	  }
	}
      }
    }
    else {       
      /* non-Z-varying data */
      rc1 = dequal(pvar->units[0],(gadouble)gfld->discipline,1e-8); /* discipline */
      rc2 = dequal(pvar->units[1],(gadouble)gfld->ipdtmpl[0],1e-8); /* category   */
      rc3 = dequal(pvar->units[2],(gadouble)gfld->ipdtmpl[1],1e-8); /* number     */
      rc4 = dequal(pvar->units[3],(gadouble)sp,1e-8);               /* SP         */
      rc5 = dequal(pvar->units[8],(gadouble)gfld->ipdtmpl[9],1e-8); /* LTYPE1     */
      if (rc1==0 && rc2==0 && rc3==0 && rc4==0 && rc5==0) {   /* all the above match */
	
	/* check if level value(s) match those given in descriptor file */
	if (
	    (pvar->units[9] < -900)                /* LVAL not given */
	    ||  
	    (pvar->units[10] < -900 &&             /* LVAL2 not given */
	     dequal(pvar->units[9],lev1,1e-8)==0)                 /* and LVAL1 matches */ 
	    ||  
	    (pvar->units[10] > -900 &&             /* LVAL2 is given */
	     dequal(pvar->units[9],lev1,1e-8)==0 &&               /* and LVAL1 matches */
	     dequal(pvar->units[10],lev2,1e-8)==0)                /* and LVAL2 matches */
	    ||
	    (pvar->units[10] > -900 &&             /* LVAL2 is given */
	     pvar->units[11] > -900 &&             /* LTYPE2 is given */
	     dequal(pvar->units[9],lev1,1e-8)==0 &&               /* and LVAL1 matches */
	     dequal(pvar->units[10],lev2,1e-8)==0 &&              /* and LVAL2 matches */
	     dequal(pvar->units[11],gfld->ipdtmpl[12],1e-8)==0)   /* and LTYPE2 matches */
	    ) { 
	  ioff = pvar->recoff;
	  return(ioff);
	}
      }
    }
    pvar++; i++;
  }  /* end of loop over variables in descriptor file */
  return(ioff);
}

/* Loops over ensembles to see if ensemble codes match current grib2 field 
   If size of ensemble dimension is 1, no checks are done, returns e=1.
   Returns ensemble index e if codes are present and match, -999 otherwise */
gaint g2ens_match (gribfield *gfld, struct gafile *pfi) {
  struct gaens *ens;
  gaint e;
  e=1;
  if (pfi->dnum[4]==1) {
    e=1;
    return(e); 
  }
  else {
    for (e=1,ens=pfi->ens1; e<=pfi->dnum[4]; e++,ens++) {
      /* PDT 0 or 8 and no grib codes */
      if (ens->grbcode[0]==-999 && ens->grbcode[1]==-999 && 
	  (gfld->ipdtnum==0 || gfld->ipdtnum==8)) {    
	if (verb) printf("pdt=%d ",gfld->ipdtnum);
	return(e);
      }
      if (ens->grbcode[0]>-900) {
	if (ens->grbcode[1]>-900) {
	  /* PDT 1 or 11 */
	  if ((gfld->ipdtnum==1 || gfld->ipdtnum==11) &&
	      ((ens->grbcode[0] == gfld->ipdtmpl[15]) && 
	       (ens->grbcode[1] == gfld->ipdtmpl[16]))) {
	    if (verb) printf("pdt=%d ens=%d,%d ",gfld->ipdtnum,ens->grbcode[0],ens->grbcode[1]);
	    return(e);
	  }
	}
	else {
	  /* PDT 2 or 12 */
	  if ((gfld->ipdtnum==2 || gfld->ipdtnum==12) &&
	      (ens->grbcode[0] == gfld->ipdtmpl[15])) {
	    if (verb) printf("pdt=%d ens=%d ",gfld->ipdtnum,ens->grbcode[0]);
	    return(e);
	  }
	}
      }
    }
    if (verb) {
      printf("pdt=%d ",gfld->ipdtnum);
      if (gfld->ipdtnum==1 || gfld->ipdtnum==11) 
	printf("ens=%d,%d ",gfld->ipdtmpl[15],gfld->ipdtmpl[16]);
      if (gfld->ipdtnum==2 || gfld->ipdtnum==12) 
	printf("ens=%d ",gfld->ipdtmpl[15]);
    }
    return(-999);
  }
}

/* Checks ensemble codes, if provided in descriptor file. 
   Returns 0 if ok or not provided, 1 if codes don't match. */
gaint g2ens_check (struct gaens *ens, gribfield *gfld) {
  /* print out ensemble info */
  if (verb) {
    printf("pdt=%d ",gfld->ipdtnum);
    if (gfld->ipdtnum==1 || gfld->ipdtnum==11) 
      printf("ens=%d,%d ",gfld->ipdtmpl[15],gfld->ipdtmpl[16]);
    if (gfld->ipdtnum==2 || gfld->ipdtnum==12) 
      printf("ens=%d ",gfld->ipdtmpl[15]);
  }
  if (ens->grbcode[0]>-900) {
    if (ens->grbcode[1]>-900) {
      /* PDT 1 or 11 */
      if ((gfld->ipdtnum==1 || gfld->ipdtnum==11) &&
	  ((ens->grbcode[0] == gfld->ipdtmpl[15]) && 
	   (ens->grbcode[1] == gfld->ipdtmpl[16]))) return(0);
      else return(1);
    }
    else {
      /* PDT 2 or 12 */
      if ((gfld->ipdtnum==2 || gfld->ipdtnum==12) &&
	  (ens->grbcode[0] == gfld->ipdtmpl[15])) return(0);
      else return(1);
    }
  }
  /* PDT 0 or 8 and no grib codes */
  if (ens->grbcode[0]==-999 && ens->grbcode[1]==-999 &&
      (gfld->ipdtnum==0 || gfld->ipdtnum==8)) return(0);
  else return(1);
}

/* Gets the statistical process used to derive a variable.
   returns -999 for variables "at a point in time" */
gaint g2sp (gribfield *gfld) {
  gaint sp;
  sp = -999;
  if (gfld->ipdtnum ==  8) sp = gfld->ipdtmpl[23];
  if (gfld->ipdtnum ==  9) sp = gfld->ipdtmpl[30];
  if (gfld->ipdtnum == 10) sp = gfld->ipdtmpl[24];
  if (gfld->ipdtnum == 11) sp = gfld->ipdtmpl[26];
  if (gfld->ipdtnum == 12) sp = gfld->ipdtmpl[25];
  if (sp==255) sp = -999;
  return(sp);
}

/* prints out relevant info from a grib2 record */
void g2prnt (gribfield *gfld, gaint r, g2int f, gaint sp) {
  /* print record/field number */
  printf("%d.%ld: ",r,f);
  /* print level info */
  if (gfld->ipdtmpl[10]==-127) 
    printf("lev1=%ld ",gfld->ipdtmpl[9]); /* just print the level1 type */
  else
    printf("lev1=%ld,%g ",gfld->ipdtmpl[9],scaled2dbl(gfld->ipdtmpl[10],gfld->ipdtmpl[11]));
  
  if (gfld->ipdtmpl[12]<255) {
    if (gfld->ipdtmpl[13]==-127) 
      printf("lev1=%ld ",gfld->ipdtmpl[12]); /* just print the level2 type */
    else
      printf("lev2=%ld,%g ",gfld->ipdtmpl[12],scaled2dbl(gfld->ipdtmpl[13],gfld->ipdtmpl[14])); 
  }
  /* print variable info */
  if (sp==-999)
    printf("var=%ld,%ld,%ld ",gfld->discipline,gfld->ipdtmpl[0],gfld->ipdtmpl[1]);
  else
    printf("var=%ld,%ld,%ld,%d ",gfld->discipline,gfld->ipdtmpl[0], gfld->ipdtmpl[1],sp);
}


#endif  /* matches #if GRIB2 */



