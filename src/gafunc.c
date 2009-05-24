/* Copyright (C) 1988-2008 by Brian Doty and the 
   Institute of Global Environment and Society (IGES).  
   See file COPYRIGHT for more information.   */

/*  Originally authored by B. Doty.  
    Some functions provided by others. */

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
#include "grads.h"

/* expose Mike Fiorino's global struct to these routines for warning level setting */
extern struct gamfcmn mfcmn;

static struct gaufb *ufba;  /* Anchor for user function defs */
char *gxgnam(char *);       /* This is also in gx.h */

/* Function routine names.  Add a new function by putting the
   prototype here and adding to the if tests below.  */

gaint ffuser   (struct gaufb  *, struct gafunc *, struct gastat *);
gaint ffsqrt   (struct gafunc *, struct gastat *);
gaint ffsin    (struct gafunc *, struct gastat *);
gaint ffcos    (struct gafunc *, struct gastat *);
gaint fftan    (struct gafunc *, struct gastat *);
gaint ffasin   (struct gafunc *, struct gastat *);
gaint ffacos   (struct gafunc *, struct gastat *);
gaint ffexp    (struct gafunc *, struct gastat *);
gaint fflog    (struct gafunc *, struct gastat *);
gaint fflog10  (struct gafunc *, struct gastat *);
gaint ffabs    (struct gafunc *, struct gastat *);
gaint ffpow    (struct gafunc *, struct gastat *);
gaint ffmag    (struct gafunc *, struct gastat *);
gaint ffatan   (struct gafunc *, struct gastat *);
gaint ffave    (struct gafunc *, struct gastat *);
gaint ffgint   (struct gafunc *, struct gastat *);
gaint ffhdiv   (struct gafunc *, struct gastat *);
gaint ffhcrl   (struct gafunc *, struct gastat *);
gaint ffvint   (struct gafunc *, struct gastat *);
gaint fftlp    (struct gafunc *, struct gastat *);
gaint ffaav    (struct gafunc *, struct gastat *);
gaint fflterp  (struct gafunc *, struct gastat *);
gaint ffscor   (struct gafunc *, struct gastat *);
gaint fftcor   (struct gafunc *, struct gastat *);
gaint fftmav   (struct gafunc *, struct gastat *);
gaint ffmask   (struct gafunc *, struct gastat *);
gaint ffg2s    (struct gafunc *, struct gastat *);
gaint ffg2s2   (struct gafunc *, struct gastat *);
gaint fftv2t   (struct gafunc *, struct gastat *);
gaint fftv2q   (struct gafunc *, struct gastat *);
gaint ffoacr   (struct gafunc *, struct gastat *);
gaint ffoabn   (struct gafunc *, struct gastat *); 
gaint ffsmth   (struct gafunc *, struct gastat *);
gaint ffsave   (struct gafunc *, struct gastat *);
gaint ffsmin   (struct gafunc *, struct gastat *);
gaint ffsmax   (struct gafunc *, struct gastat *);
gaint ffskip   (struct gafunc *, struct gastat *);
gaint ffcnst   (struct gafunc *, struct gastat *);
gaint ffcdif   (struct gafunc *, struct gastat *);
gaint ffmn     (struct gafunc *, struct gastat *); 
gaint ffamn    (struct gafunc *, struct gastat *); 
gaint ffsum    (struct gafunc *, struct gastat *); 
gaint ffsumg   (struct gafunc *, struct gastat *); 
gaint ffasum   (struct gafunc *, struct gastat *); 
gaint ffasumg  (struct gafunc *, struct gastat *); 
gaint ffgrarea (struct gafunc *, struct gastat *); 
gaint ffclgr   (struct gafunc *, struct gastat *);
gaint ffmin    (struct gafunc *, struct gastat *);
gaint ffmax    (struct gafunc *, struct gastat *);
gaint ffminl   (struct gafunc *, struct gastat *);
gaint ffmaxl   (struct gafunc *, struct gastat *);
gaint ffflvl   (struct gafunc *, struct gastat *);
gaint ffsreg   (struct gafunc *, struct gastat *); 
gaint fftreg   (struct gafunc *, struct gastat *); 
gaint ffs2g1d  (struct gafunc *, struct gastat *);
gaint fftv2    (struct gafunc *, struct gastat *, gaint);
gaint ffsmnx   (struct gafunc *, struct gastat *, gaint);
gaint tmaskf   (struct gafunc *, struct gastat *, gaint);
gaint aave     (struct gafunc *, struct gastat *, gaint);  
gaint ave      (struct gafunc *, struct gastat *, gaint);   
gaint scorr    (struct gafunc *, struct gastat *, gaint); 
gaint tvrh2q   (gadouble, gadouble, gadouble, gadouble *, gadouble *);
gaint fndarg   (char *, gaint *);
void cosadj  (struct gagrid *);
char doaave (struct gagrid *, gadouble, gadouble, gadouble, gadouble, gaint, gadouble *);

static char pout[256];   /* Build error msgs here */

char *rtnprs (char *ch, char *name, struct gastat *pst) {
struct gafunc *pfc;
struct gastat *pst2;
struct gaufb *ufb;
char *pos;
gaint pdeep,rc,size;
gaint (*fpntr)(struct gafunc *, struct gastat *)=NULL;

  /* Find this function name and get the function pointer. */

  ufb = ufba;
  while (ufb) {
    if (cmpwrd(ufb->name,name)) break;
    ufb = ufb->ufb;
  }

  if (ufb==NULL) {
    fpntr = NULL;
    if (cmpwrd("sqrt",name)) fpntr = ffsqrt;
    if (cmpwrd("sin",name)) fpntr = ffsin;
    if (cmpwrd("cos",name)) fpntr = ffcos;
    if (cmpwrd("tan",name)) fpntr = fftan;
    if (cmpwrd("asin",name)) fpntr = ffasin;
    if (cmpwrd("acos",name)) fpntr = ffacos;
    if (cmpwrd("exp",name)) fpntr = ffexp;
    if (cmpwrd("log",name)) fpntr = fflog;
    if (cmpwrd("log10",name)) fpntr = fflog10;
    if (cmpwrd("abs",name)) fpntr = ffabs;
    if (cmpwrd("pow",name)) fpntr = ffpow;
    if (cmpwrd("ave",name)) fpntr = ffave;
    if (cmpwrd("mag",name)) fpntr = ffmag;
    if (cmpwrd("atan2",name)) fpntr = ffatan;
    if (cmpwrd("hdivg",name)) fpntr = ffhdiv;
    if (cmpwrd("hcurl",name)) fpntr = ffhcrl;
    if (cmpwrd("vint",name)) fpntr = ffvint;
    if (cmpwrd("tloop",name)) fpntr = fftlp;
    if (cmpwrd("aave",name)) fpntr = ffaav;
    if (cmpwrd("scorr",name)) fpntr = ffscor;
    if (cmpwrd("tcorr",name)) fpntr = fftcor;
    if (cmpwrd("tmave",name)) fpntr = fftmav;
    if (cmpwrd("maskout",name)) fpntr = ffmask;
    if (cmpwrd("gr2stn",name)) fpntr = ffg2s;
    if (cmpwrd("tvrh2q",name)) fpntr = fftv2q;
    if (cmpwrd("tvrh2t",name)) fpntr = fftv2t;
    if (cmpwrd("gint",name)) fpntr = ffgint;
    if (cmpwrd("oacres",name)) fpntr = ffoacr;
    if (cmpwrd("oabin",name)) fpntr = ffoabn;
    if (cmpwrd("smth9",name)) fpntr = ffsmth;
    if (cmpwrd("stnave",name)) fpntr = ffsave;
    if (cmpwrd("stnmin",name)) fpntr = ffsmin;
    if (cmpwrd("stnmax",name)) fpntr = ffsmax;
    if (cmpwrd("skip",name)) fpntr = ffskip;
    if (cmpwrd("const",name)) fpntr = ffcnst;
    if (cmpwrd("cdiff",name)) fpntr = ffcdif;
    if (cmpwrd("mean",name)) fpntr = ffmn;    
    if (cmpwrd("amean",name)) fpntr = ffamn;  
    if (cmpwrd("sum",name)) fpntr = ffsum;    
    if (cmpwrd("sumg",name)) fpntr = ffsumg;  
    if (cmpwrd("asum",name)) fpntr = ffasum;  
    if (cmpwrd("asumg",name)) fpntr = ffasumg; 
    if (cmpwrd("grarea",name)) fpntr = ffgrarea; 
    if (cmpwrd("coll2gr",name)) fpntr = ffclgr;
    if (cmpwrd("min",name)) fpntr = ffmin;
    if (cmpwrd("max",name)) fpntr = ffmax;
    if (cmpwrd("minloc",name)) fpntr = ffminl;
    if (cmpwrd("maxloc",name)) fpntr = ffmaxl;
    if (cmpwrd("fndlvl",name)) fpntr = ffflvl;
    if (cmpwrd("sregr",name)) fpntr = ffsreg;  
    if (cmpwrd("tregr",name)) fpntr = fftreg;  
    if (cmpwrd("s2g1d",name)) fpntr = ffs2g1d;
    if (cmpwrd("lterp",name)) fpntr = fflterp;

    if (fpntr==NULL) {                       /* Didn't find it....      */
      gaprnt (0,"Syntax Error:  Invalid Operand \n");
      sprintf (pout,"  '%s' not a variable or function name\n",name);
      gaprnt (0,pout);
      return (NULL);
    }
  }

  /* Allocate storage for gastat and gafunc structures                */

  size = sizeof(struct gafunc);
  pfc = (struct gafunc *)galloc(size,"funccall");
  size = sizeof(struct gastat);
  pst2 = (struct gastat *)galloc(size,"funcpst");

  *pst2 = *pst;                            /* Provide copy of gastat  */

  /* Parse the argument list                                          */

  pfc->argnum = 0;                         /* Initial arg count       */
  if (*ch=='(') {                          /* If no leading paren..   */
    ch++;                                  /* Past the '('            */
    if (*ch==')') {                        /* Was it '()'???          */
      ch++;                                /*  Point past this func   */
    } else {                               /* We have something       */
      pos = pfc->buff;                     /*  Point to output buffer */
      pdeep = 0;                           /*  No parens nested yet   */
      pfc->argpnt[0] = pos;                /*  Start of 1st arg       */
      while (pdeep!=0 || *ch!=')') {       /*  Until end of args...   */
        if (*ch=='\0' || *ch=='\n') {      /*   End of string?        */
          gaprnt (0,"Syntax Error:  Unmatched parens\n");
          goto err;                        /*    And return           */
        }                                  /*   Is ok                 */
        *pos = *ch;                        /*   Move to output buffer */
        if (*ch=='(') pdeep++;             /*   Track paren nesting   */
        else if (*ch==')') pdeep--;        /*   ditto                 */
        else if (pdeep==0 && *ch==',') {   /*   End of an arg?        */
          *pos = '\0';                     /*    Terminate string     */
          pfc->argnum++;                   /*    Bump arg counter     */
          pfc->argpnt[pfc->argnum] = pos+1; /*   Start of next arg    */
          *(pos+1) = '\0';                 /*    If trailing comma    */
        }                                  /*   endif                 */
        pos++; ch++;                       /*   Bump pointers         */
      }                                    /*  Continue               */
      *pos = '\0';                         /*  terminate final arg    */
      pfc->argnum++;                       /*  Count instead of subscr*/
      ch++;                                /*  Pnt past func call     */
    }                                      /* X                       */
  }                                        /* We have args parsed     */

  /* Everything is all set.  Call the function routine.               */

  if (ufb) rc = ffuser(ufb,pfc,pst2);
  else rc = (*fpntr)(pfc, pst2);           /* Call the function       */

  if (rc==-1) {
    sprintf (pout,"Error in %s : Arg was stn data type\n",name);
    gaprnt (0,pout);
  }

  if (rc) {                                /* If an error occurred... */
    sprintf (pout,"Operation Error:  Error from %s function\n",name);
    gaprnt (0,pout);
    goto err;
  }
  pst->type = pst2->type;
  pst->result = pst2->result;              /* Return result grid      */
  gree(pst2,"f404");
  gree(pfc,"f405");
  return (ch);                             /* And return new pointer  */

err:
  gree(pst2,"f406");
  gree(pfc,"f407");
  return (NULL);

}

/* (JMA this subroutine commented out pending implementation of DLLs.)

   Handle user specified function call.  The args are written out,
   the user's process is invoked, and the result is read back in.  */

int ffuser (struct gaufb *ufb, struct gafunc *pfc, struct gastat *pst) {
/* FILE *ifile=NULL,*ofile; */
/* struct gagrid *pgr; */
/* struct dt dtim; */
/* gadouble (*conv) (gadouble *, gadouble); */
/* gadouble *v,*v0,*gr,*gr0; */
/* float rvals[20],*fval,*fval0; */
/* int rc,iarg,siz,i; */
/* int rdw; */
/* char *ch,rec[80],*gru,*gru0; */

/*   /\* Check number of args *\/ */
/*   if (pfc->argnum<ufb->alo || pfc->argnum>ufb->ahi) { */
/*     sprintf (pout,"Error from %s: Too many or too few args\n",ufb->name); */
/*     gaprnt (0,pout); */
/*     return (1); */
/*   } */

/*   /\* Open the transfer file *\/ */
/*   ofile = fopen(ufb->oname,"wb"); */
/*   if (ofile==NULL) { */
/*     sprintf (pout,"Error from %s: Error opening transfer file\n",ufb->name); */
/*     gaprnt (0,pout); */
/*     sprintf (pout,"  File name: %s\n",ufb->oname); */
/*     gaprnt (0,pout); */
/*     return (1); */
/*   } */
  
/*   for(i=0;i<20;i++) rvals[i]=0.0;    /\* initialize rvals *\/ */

/*   /\* Write header record to transfer file *\/ */
/*   rvals[0] = (float)pfc->argnum; */
/*   if (ufb->sflg) { */
/*     rdw = sizeof(float)*20; */
/*     fwrite (&rdw,sizeof(int),1,ofile); */
/*   } */
/*   rc = fwrite (rvals,sizeof(float),20,ofile); */
/*   if (rc<20) goto werr; */
/*   if (ufb->sflg) fwrite (&rdw,sizeof(int),1,ofile); */

/*   /\* Write args to the transfer file *\/ */
/*   for (iarg=0; iarg<pfc->argnum; iarg++) { */

/*     /\* Handle expression *\/ */
/*     if (ufb->atype[iarg]==1) { */
/*       rc = gaexpr(pfc->argpnt[iarg],pst);         /\* Evaluate      *\/ */
/*       if (rc) { */
/* 	fclose(ofile); */
/* 	return (rc); */
/*       } */
/*       if (pst->type!=1) { */
/* 	fclose(ofile); */
/*         gafree (pst); */
/*         return(-1); */
/*       } */
/*       pgr = pst->result.pgr;                   /\* Fill in header *\/ */
/*       rvals[0] = (float)pgr->undef; */
/*       rvals[1] = pgr->idim; */
/*       rvals[2] = pgr->jdim; */
/*       rvals[3] = pgr->isiz; */
/*       rvals[4] = pgr->jsiz; */
/*       rvals[5] = pgr->ilinr; */
/*       rvals[6] = pgr->jlinr; */
/*       if (pgr->idim>-1 && pgr->ilinr==1) {     /\* Linear scaling info *\/ */
/*         if (pgr->idim==3) { */
/*           gr2t (pgr->ivals,(gadouble)pgr->dimmin[3],&dtim); */
/*           rvals[11] = dtim.yr; */
/*           rvals[12] = dtim.mo; */
/*           rvals[13] = dtim.dy; */
/*           rvals[14] = dtim.hr; */
/*           rvals[15] = dtim.mn; */
/*           rvals[16] = (float)*(pgr->ivals+6); */
/*           rvals[17] = (float)*(pgr->ivals+5); */
/*           rvals[9] = -999.0; */
/*           rvals[10] = -999.0; */
/*         } else { */
/*           conv = pgr->igrab; */
/*           rvals[7] = (float)conv(pgr->ivals,pgr->dimmin[pgr->idim]); */
/*           rvals[8] = (float)*(pgr->ivals); */
/*         } */
/*       } */
/*       if (pgr->jdim>-1 && pgr->jlinr==1) { */
/*         if (pgr->jdim==3) { */
/*           gr2t (pgr->jvals,(gadouble)pgr->dimmin[3],&dtim); */
/*           rvals[11] = dtim.yr; */
/*           rvals[12] = dtim.mo; */
/*           rvals[13] = dtim.dy; */
/*           rvals[14] = dtim.hr; */
/*           rvals[15] = dtim.mn; */
/*           rvals[16] = (float)*(pgr->jvals+6); */
/*           rvals[17] = (float)*(pgr->jvals+5); */
/*           rvals[9] = -999.0; */
/*           rvals[10] = -999.0; */
/*         } else { */
/*           conv = pgr->jgrab; */
/*           rvals[9] = (float)conv(pgr->jvals,pgr->dimmin[pgr->jdim]); */
/*           rvals[10] = (float)*(pgr->jvals); */
/*         } */
/*       } */
/*       /\* Write header *\/ */
/*       siz = pgr->isiz*pgr->jsiz; */
/*       if (ufb->sflg) { */
/*         rdw = sizeof(float)*20; */
/*         fwrite (&rdw,sizeof(int),1,ofile); */
/*       } */
/*       rc = fwrite(rvals,sizeof(float),20,ofile); */
/*       if (rc<20) { */
/*         gafree(pst); */
/*         goto werr; */
/*       } */
/*       /\* Write grid   *\/ */
/*       if (ufb->sflg) fwrite (&rdw,sizeof(int),1,ofile); */
/*       if (ufb->sflg) { */
/*         rdw = sizeof(float)*siz; */
/*         fwrite (&rdw,sizeof(int),1,ofile); */
/*       } */
/*       /\* convert the grid values to floats/undefs *\/ */
/*       gr=pgr->grid; */
/*       gru=pgr->umask; */
/*       fval = NULL; */
/*       fval = (float *)malloc(sizeof(float)*siz); */
/*       fval0 = fval; */
/*       if (fval==NULL) goto merr; */
/*       for (i=0; i<siz; i++) { */
/* 	if (*gru!=0) { */
/* 	  *fval=(float)*gr; */
/* 	} */
/* 	else { */
/* 	  *fval=(float)pgr->undef; */
/* 	} */
/* 	gr++; gru++; fval++; */
/*       } */
/*       fval = fval0; */
/*       rc = fwrite(fval,sizeof(float),siz,ofile); */
/*       if (rc<siz) { */
/* 	gafree(pst); */
/* 	goto werr; */
/*       } */
/*       if (fval!=NULL) { */
/* 	fval = fval0; */
/* 	free(fval); */
/* 	fval0=NULL; */
/*       } */
/*       if (ufb->sflg) fwrite (&rdw,sizeof(int),1,ofile); */

/*       /\* write i dim scaling *\/ */
/*       if (pgr->idim>-1) { */
/* 	fval = NULL; */
/* 	fval = (float *)malloc(sizeof(float)*pgr->isiz); */
/* 	fval0 = fval; */
/* 	if (fval==NULL) goto merr; */
/*         if (pgr->idim==3) { */
/*           for (i=pgr->dimmin[pgr->idim];i<=pgr->dimmax[pgr->idim];i++) { */
/*             *fval = (float)i; */
/*             fval++; */
/*           } */
/*         } else { */
/*           conv = pgr->igrab; */
/*           for (i=pgr->dimmin[pgr->idim];i<=pgr->dimmax[pgr->idim];i++) { */
/*             *fval = (float)conv(pgr->ivals,(gadouble)i); */
/*             fval++; */
/*           } */
/*         } */
/* 	fval = fval0; */
/*         if (ufb->sflg) { */
/*           rdw = sizeof(float)*pgr->isiz; */
/*           fwrite (&rdw,sizeof(int),1,ofile); */
/*         } */
/*         rc = fwrite(fval,sizeof(float),pgr->isiz,ofile); */
/*         if (rc<pgr->isiz) { */
/*           gafree(pst); */
/*           goto werr; */
/*         } */
/* 	if (fval!=NULL) { */
/* 	  fval = fval0; */
/* 	  free(fval); */
/* 	  fval0=NULL; */
/* 	} */
/* 	if (ufb->sflg) fwrite (&rdw,sizeof(int),1,ofile); */
/*       } */
/*       /\* write j dim scaling *\/ */
/*       if (pgr->jdim>-1) { */
/* 	fval = NULL; */
/* 	fval = (float *)malloc(sizeof(float)*pgr->isiz); */
/* 	fval0 = fval; */
/* 	if (fval==NULL) goto merr; */
/*          if (pgr->jdim==3) { */
/*           for (i=pgr->dimmin[pgr->jdim];i<=pgr->dimmax[pgr->jdim];i++) { */
/*             *fval = (float)i; */
/*             fval++; */
/*           } */
/*         } else { */
/*           conv = pgr->jgrab; */
/*           for (i=pgr->dimmin[pgr->jdim];i<=pgr->dimmax[pgr->jdim];i++) { */
/*             *fval = (float)conv(pgr->jvals,(gadouble)i); */
/*             fval++; */
/*           } */
/*         } */
/* 	fval = fval0; */
/* 	if (ufb->sflg) { */
/*           rdw = sizeof(float)*pgr->jsiz; */
/*           fwrite (&rdw,sizeof(int),1,ofile); */
/*         } */
/*         rc = fwrite(fval,sizeof(float),pgr->jsiz,ofile); */
/*         if (rc<pgr->jsiz) { */
/*           gafree(pst); */
/*           goto werr; */
/*         } */
/* 	if (fval!=NULL) { */
/* 	  fval = fval0; */
/* 	  free(fval); */
/* 	  fval0=NULL; */
/* 	} */
/* 	if (ufb->sflg) fwrite (&rdw,sizeof(int),1,ofile); */
/*       } */
/*       gafree(pst);     /\* Done with expr *\/ */
/*     } */

/*     /\* Handle Value *\/ */

/*     else if (ufb->atype[iarg]==2) { */
/*       if (getflt(pfc->argpnt[iarg],rvals)==NULL) { */
/*         sprintf (pout,"Error from %s: Invalid Argument\n",ufb->name); */
/*         gaprnt (0,pout); */
/*         sprintf (pout,"  Expecting arg %i to be a constant\n",iarg); */
/*         gaprnt (0,pout); */
/*       } */
/*       if (ufb->sflg) { */
/*         rdw = sizeof(float); */
/*         fwrite (&rdw,sizeof(int),1,ofile); */
/*       } */
/*       rc = fwrite(rvals,sizeof(float),1,ofile); */
/*       if (rc<1) goto werr; */
/*       if (ufb->sflg) fwrite (&rdw,sizeof(int),1,ofile); */
/*     } */

/*     /\* Handle Character *\/ */

/*     else if (ufb->atype[iarg]==3) { */
/*       ch = pfc->argpnt[iarg]; */
/*       for (i=0; i<80; i++) rec[i] = ' '; */
/*       i = 0; */
/*       while (*ch && i<80) { */
/*         rec[i] = *ch; */
/*         i++; ch++; */
/*       } */
/*       if (ufb->sflg) { */
/*         rdw = 80; */
/*         fwrite (&rdw,sizeof(int),1,ofile); */
/*       } */
/*       rc = fwrite(rec,1,80,ofile); */
/*       if (rc<80) goto werr; */
/*       if (ufb->sflg) fwrite (&rdw,sizeof(int),1,ofile); */
/*     } */
/*   } */

/*   /\* All the args are written.  Close the transfer file *\/ */
/*   fclose (ofile); */

/*   /\* Invoke the user's routine.  *\/ */
/* #ifdef XLIBEMU */
/*   System (ufb->fname); */
/* #else */
/*   system (ufb->fname); */
/* #endif */

/*   /\* Read the user's output file -- our input file. *\/ */
/*   ifile = fopen(ufb->iname,"rb"); */
/*   if (ifile==NULL) { */
/*     sprintf (pout,"Error from %s: Error opening transfer file\n",ufb->name); */
/*     gaprnt (0,pout); */
/*     sprintf (pout,"  File name: %s\n",ufb->iname); */
/*     gaprnt (0,pout); */
/*     return (1); */
/*   } */

/*   /\* Read the header record, which contains the return code *\/ */

/*   if (ufb->sflg) { */
/*     fread(&rdw,sizeof(int),1,ifile); */
/*     if (rdw!=sizeof(float)*20) goto ferr; */
/*   } */
/*   rc = fread(rvals,sizeof(float),20,ifile); */
/*   if (rc<20) goto rerr; */
/*   if (ufb->sflg) fread(&rdw,sizeof(int),1,ifile); */
/*   rc = (int)(*rvals+0.1); */
/*   if (rc!=0) { */
/*     fclose(ifile);   /\*mf ---- add fclose of the udf input file ---- mf*\/ */
/*     return (1); */
/*   } */

/*   /\* If all is ok, read the grid header *\/ */

/*   if (ufb->sflg) { */
/*     fread(&rdw,sizeof(int),1,ifile); */
/*     if (rdw!=sizeof(float)*20) goto ferr; */
/*   } */
/*   rc = fread(rvals,sizeof(float),20,ifile); */
/*   if (rc<20) goto rerr; */
/*   if (ufb->sflg) fread(&rdw,sizeof(int),1,ifile); */

/*   /\* Start building the gagrid block *\/ */

/*   pgr = (struct gagrid *)malloc(sizeof(struct gagrid)); */
/*   if (pgr==NULL) goto merr; */

/*   /\* Fill in and check values *\/ */
/*   pgr->alocf = 0; */
/*   pgr->undef = (gadouble)rvals[0]; */
/*   pgr->idim = (int)(floor(rvals[1]+0.1)); */
/*   pgr->jdim = (int)(floor(rvals[2]+0.1)); */
/*   pgr->iwrld = 0; */
/*   pgr->jwrld = 0; */
/*   pgr->isiz = (int)(rvals[3]+0.1); */
/*   pgr->jsiz = (int)(rvals[4]+0.1); */
/*   pgr->ilinr = (int)(rvals[5]+0.1); */
/*   pgr->jlinr = (int)(rvals[6]+0.1); */
/*   for (i=0; i<5; i++) pgr->dimmin[i] = 1; */
/*   if ( pgr->idim<-1 || pgr->idim>4 ) goto ferr; */
/*   if ( pgr->jdim<-1 || pgr->jdim>4 ) goto ferr; */
/*   if ( pgr->ilinr<0 || pgr->ilinr>1) goto ferr; */
/*   if ( pgr->jlinr<0 || pgr->jlinr>1) goto ferr; */
/*   if ( pgr->jdim>-1 && pgr->idim>pgr->jdim) goto derr; */
/*   if ( pgr->idim==-1 && pgr->isiz!=1) goto ferr; */
/*   if ( pgr->jdim==-1 && pgr->jsiz!=1) goto ferr; */
/*   if ( pgr->isiz<1) goto ferr; */
/*   if ( pgr->jsiz<1) goto ferr; */
/*   for (i=0; i<5; i++) pgr->dimmin[i] = 1; */
/*   if (pgr->idim>-1) pgr->dimmax[pgr->idim] = pgr->isiz; */
/*   if (pgr->jdim>-1) pgr->dimmax[pgr->jdim] = pgr->jsiz; */

/*   if (pgr->idim>-1 && pgr->idim!=pst->idim && pgr->idim!=pst->jdim) goto derr; */
/*   if (pgr->jdim>-1 && pgr->jdim!=pst->idim && pgr->jdim!=pst->jdim) goto derr; */

/*   /\* Set up linear scaling info *\/ */

/*   if (pgr->idim>-1 && pgr->ilinr==1) {     /\* Linear scaling info *\/ */
/*     if (pgr->idim==3) { */
/*       v = (gadouble *)malloc(sizeof(gadouble)*8); */
/*       if (v==NULL) goto merr; */
/*       *v = rvals[11]; */
/*       *(v+1) = rvals[12]; */
/*       *(v+2) = rvals[13]; */
/*       *(v+3) = rvals[14]; */
/*       *(v+4) = rvals[15]; */
/*       *(v+6) = rvals[16]; */
/*       *(v+5) = rvals[17]; */
/*       *(v+7) = -999.9; */
/*       pgr->ivals = v; */
/*       pgr->iavals = v; */
/*     } else { */
/*       v = (gadouble *)malloc(sizeof(gadouble)*6); */
/*       if (v==NULL) goto merr; */
/*       *v = rvals[8]; */
/*       *(v+1) = rvals[7]-rvals[8]; */
/*       *(v+2) = -999.9; */
/*       pgr->ivals = v; */
/*       *(v+3) = 1.0 / rvals[8]; */
/*       *(v+4) = -1.0 * (rvals[7]-rvals[8]) / rvals[8]; */
/*       *(v+5) = -999.9; */
/*       pgr->iavals = v+3; */
/*       pgr->iabgr = liconv; */
/*       pgr->igrab = liconv; */
/*     } */
/*   } */
/*   if (pgr->jdim>-1 && pgr->jlinr==1) {     /\* Linear scaling info *\/ */
/*     if (pgr->jdim==3) { */
/*       v = (gadouble *)malloc(sizeof(gadouble)*8); */
/*       if (v==NULL) goto merr; */
/*       *v = rvals[11]; */
/*       *(v+1) = rvals[12]; */
/*       *(v+2) = rvals[13]; */
/*       *(v+3) = rvals[14]; */
/*       *(v+4) = rvals[15]; */
/*       *(v+6) = rvals[16]; */
/*       *(v+5) = rvals[17]; */
/*       *(v+7) = -999.9; */
/*       pgr->jvals = v; */
/*       pgr->javals = v; */
/*     } else { */
/*       v = (gadouble *)malloc(sizeof(gadouble)*6); */
/*       if (v==NULL) goto merr; */
/*       *v = rvals[10]; */
/*       *(v+1) = rvals[9]-rvals[10]; */
/*       *(v+2) = -999.9; */
/*       pgr->jvals = v; */
/*       *(v+3) = 1.0 / rvals[10]; */
/*       *(v+4) = -1.0 * (rvals[9]-rvals[10]) / rvals[10]; */
/*       *(v+5) = -999.9; */
/*       pgr->javals = v+3; */
/*       pgr->jabgr = liconv; */
/*       pgr->jgrab = liconv; */
/*     } */
/*   } */

/*   /\* Read in the data *\/ */

/*   siz = pgr->isiz * pgr->jsiz; */
/*   fval = (float *)malloc(sizeof(float)*siz); */
/*   fval0 = fval; */
/*   if (fval==NULL) { */
/*     free(pgr); */
/*     goto merr; */
/*   } */
/*   gr = (gadouble *)malloc(sizeof(gadouble)*siz); */
/*   gr0 = gr; */
/*   if (gr==NULL) { */
/*     if (fval) free(fval); */
/*     free(pgr); */
/*     goto merr; */
/*   } */
/*   gru = (char *)malloc(sizeof(char)*siz); */
/*   gru0 = gru; */
/*   if (gru==NULL) { */
/*     if (gr) free(gr); */
/*     if (fval) free(fval); */
/*     free(pgr); */
/*     goto merr; */
/*   } */
/*   if (ufb->sflg) { */
/*     fread(&rdw,sizeof(int),1,ifile); */
/*     if (rdw!=sizeof(float)*siz) goto ferr; */
/*   } */
/*   rc = fread(fval,sizeof(float),siz,ifile); */
/*   if (rc<siz) goto rerr; */
/*   if (ufb->sflg) fread(&rdw,sizeof(int),1,ifile); */
/*   fval=fval0; */
/*   for (i=0;i<siz;i++) { */
/*     if (*fval!=pgr->undef) { */
/*       *gr = *fval; */
/*       *gru = 1; */
/*     } */
/*     else { */
/*       *gru = 0; */
/*     } */
/*     gr++; gru++; fval++; */
/*   } */
/*   gr = gr0; */
/*   gru = gru0; */
/*   pgr->grid = gr; */
/*   pgr->umask = gru; */
/*   if (fval) { */
/*     fval=fval0; */
/*     free(fval); */
/*   } */

/*   /\* Read in non-linear scaling info, if any *\/ */

/*   if (pgr->idim>-1 && pgr->ilinr==0) { */
/*     fval=NULL; */
/*     fval = (float *)malloc(sizeof(float)*(pgr->isiz)); */
/*     fval0 = fval; */
/*     if (fval==NULL) { */
/*       free(pgr->grid); */
/*       free(pgr); */
/*       goto merr; */
/*     } */
/*     v = (gadouble *)malloc(sizeof(gadouble)*(pgr->isiz+2)); */
/*     v0 = v; */
/*     if (v==NULL) { */
/*       if (fval) free(fval); */
/*       free(pgr->grid); */
/*       free(pgr); */
/*       goto merr; */
/*     } */
/*     *v = pgr->isiz; */
/*     if (ufb->sflg) { */
/*       fread(&rdw,sizeof(int),1,ifile); */
/*       if (rdw!=sizeof(float)*pgr->isiz) goto ferr; */
/*     } */
/*     rc = fread(fval,sizeof(float),pgr->isiz,ifile);   /\* read the data into fval array *\/ */
/*     if (ufb->sflg) fread(&rdw,sizeof(int),1,ifile); */
/*     if (rc<pgr->isiz) goto rerr; */
/*     fval=fval0; */
/*     for (i=0;i<pgr->isiz; i++) { */
/*       *(v+1) = (gadouble)*(fval);  /\* copy gadobules into v array *\/ */
/*       v++; fval++; */
/*     } */
/*     *(v+pgr->isiz+1) = -999.9; */
/*     v = v0; */
/*     pgr->ivals = v; */
/*     pgr->iavals = v; */
/*     pgr->iabgr = lev2gr; */
/*     pgr->igrab = gr2lev; */
/*     if (fval) { */
/*       fval=fval0; */
/*       free(fval); */
/*     } */
/*   } */
/*   if (pgr->jdim>-1 && pgr->jlinr==0) { */
/*     fval=NULL; */
/*     fval = (float *)malloc(sizeof(float)*(pgr->isiz)); */
/*     if (fval==NULL) { */
/*       free(pgr->grid); */
/*       free(pgr); */
/*       goto merr; */
/*     } */
/*     v = (gadouble *)malloc(sizeof(gadouble)*(pgr->jsiz+2)); */
/*     if (v==NULL) { */
/*       if (fval) free(fval); */
/*       free(pgr->grid); */
/*       free(pgr); */
/*       goto merr; */
/*     } */
/*     *v = pgr->jsiz; */
/*     if (ufb->sflg) { */
/*       fread(&rdw,sizeof(int),1,ifile); */
/*       if (rdw!=sizeof(float)*pgr->jsiz) goto ferr; */
/*     } */
/*     rc = fread(v+1,sizeof(float),pgr->jsiz,ifile); */
/*     if (rc<pgr->jsiz) goto rerr; */
/*     if (ufb->sflg) fread(&rdw,sizeof(int),1,ifile); */
/*     for (i=0;i<pgr->isiz; i++) { */
/*       *(v+1) = (gadouble)*(fval);  /\* copy gadobules into v array *\/ */
/*       v++; fval++; */
/*     } */
/*     *(v+pgr->jsiz+1) = -999.9; */
/*     pgr->jvals = v; */
/*     pgr->javals = v; */
/*     pgr->jabgr = lev2gr; */
/*     pgr->jgrab = gr2lev; */
/*   } */
/*   fclose(ifile); */

/* #ifndef __CYGWIN32__ */
/*   sprintf(pout,"rm %s",ufb->oname); */
/*   system (pout); */
/*   sprintf(pout,"rm %s",ufb->iname); */
/*   system (pout); */
/* #endif */

/*   /\* We are done.  Return.  *\/ */

/*   pst->result.pgr = pgr; */
/*   pst->type = 1; */

  return (0);

/* werr: */
/*   sprintf (pout,"Error from %s: Error writing to transfer file\n",ufb->name); */
/*   gaprnt (0,pout); */
/*   sprintf (pout,"  File name: %s\n",ufb->oname); */
/*   gaprnt (0,pout); */
/*   fclose(ofile); */
/*   return (1); */

/* rerr: */
/*   sprintf (pout,"Error from %s: Error reading from transfer file\n",ufb->name); */
/*   gaprnt (0,pout); */
/*   sprintf (pout,"  File name: %s\n",ufb->oname); */
/*   gaprnt (0,pout); */
/*   fclose(ifile); */
/*   return (1); */

/* merr: */
/*   sprintf (pout,"Error from %s: Memory Allocation Error\n",ufb->name); */
/*   gaprnt (0,pout); */
/*   fclose (ifile); */
/*   return (1); */

/* ferr: */
/*   sprintf (pout,"Error from %s: Invalid transfer file format\n",ufb->name); */
/*   gaprnt (0,pout); */
/*   sprintf (pout,"  File name: %s\n",ufb->iname); */
/*   gaprnt (0,pout); */
/*   fclose (ifile); */
/*   return (1); */

/* derr: */
/*   sprintf (pout,"Error from %s: Invalid dimension environment ",ufb->name); */
/*   gaprnt (0,pout); */
/*   gaprnt (0,"in result grid\n"); */
/*   fclose (ifile); */
/*   return (1); */
}

/**********************************************************************\
*                                                                      *
*  Function routines follow.  To add a funcion routine, add code here, *
*  and update the gafunc.h file.                                       *
*                                                                      *
\**********************************************************************/

gaint ffsqrt (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt,ecnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from SQRT: Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  ecnt=0;
  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val  = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) {
        if (*val<0.0) {
          *valu = 0;
          ecnt++;
        } else {
	  *val = sqrt(*val);
	  *valu = 1;
	}
      }
	val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask != 0) {
        if (rpt->val < 0.0) {
          rpt->umask = 0;
          ecnt++;
        } else rpt->val = sqrt(rpt->val);
      }
      rpt=rpt->rpt;
    }
  }
  if (ecnt>0) {
    sprintf (pout,"Warning from SQRT:  Data has %i values < zero \n",ecnt);
    gaprnt (1,pout);
    gaprnt (1,"                    These were set to the undefined value \n");
  }
  return (0);
}

gaint ffsin  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from SIN:  Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) *val = sin(*val);
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) rpt->val = sin(rpt->val);
      rpt=rpt->rpt;
    }
  }

  return (0);
}

gaint ffcos  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from COS:  Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) *val = cos(*val);
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) rpt->val = cos(rpt->val);
      rpt=rpt->rpt;
    }
  }

  return (0);
}

gaint fftan  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from TAN:  Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) *val = tan(*val);
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) rpt->val = tan(rpt->val);
      rpt=rpt->rpt;
    }
  }
  return (0);
}

gaint ffasin  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from ASIN:  Too many or too few args \n");
    gaprnt (0,"                  One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) {
        if (*val>1.0 || *val<-1.0) {
	  *valu = 0;
	}
        else {
	  *val = asin(*val);
	}
      }
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) {
        if (rpt->val>1.0 || rpt->val<-1.0) rpt->umask = 0;
        else rpt->val = asin(rpt->val);
      }
      rpt=rpt->rpt;
    }
  }
  return (0);
}

gaint ffacos  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from ACOS:  Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) {
        if (*val>1.0 || *val<-1.0) {
	  *valu = 0;
	}
        else {
	  *val = acos(*val);
	}
      }
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) {
        if (rpt->val>1.0 || rpt->val<-1.0) rpt->umask = 0;
        else rpt->val = acos(rpt->val);
      }
      rpt=rpt->rpt;
    }
  }
  return (0);
}

gaint ffabs (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from ABS:  Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) *val = fabs(*val);
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) rpt->val = fabs(rpt->val);
      rpt=rpt->rpt;
    }
  }

  return (0);
}

gaint ffexp  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from EXP:  Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) *val = exp(*val);
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) rpt->val = exp(rpt->val);
      rpt=rpt->rpt;
    }
  }

  return (0);
}

gaint fflog  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt,ecnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from LOG:  Too many or too few args \n");
    gaprnt (0,"                 One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  ecnt=0;
  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) {
        if (*val<=0.0) {
          *valu = 0;
          ecnt++;
        } 
	else *val = log(*val);
      }
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) {
        if (rpt->val<=0.0) {
          rpt->umask = 0;
          ecnt++;
        } else rpt->val = log(rpt->val);
      }
      rpt=rpt->rpt;
    }
  }
  if (ecnt>0) {
    sprintf (pout,"Warning from LOG:  Data has %i values <= zero \n",ecnt);
    gaprnt (1,pout);
    gaprnt (1,"                   These were set to the undefined value \n");
  }

  return (0);
}

gaint fflog10 (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt,ecnt;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val;
char *valu;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from LOG10:  Too many or too few args \n");
    gaprnt (0,"                   One argument expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  ecnt=0;
  if (pst->type==1) {
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    valu = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (*valu!=0) {
        if (*val<=0.0) {
          *valu = 0;
          ecnt++;
        } else *val = log10(*val);
      }
      val++; valu++;
    }
  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (rpt->umask!=0) {
        if (rpt->val<=0.0) {
          rpt->umask = 0;
          ecnt++;
        } else rpt->val = log10(rpt->val);
      }
      rpt=rpt->rpt;
    }
  }
  if (ecnt>0) {
    sprintf (pout,"Warning from LOG10:  Data has %i values <= zero \n",ecnt);
    gaprnt (1,pout);
    gaprnt (1,"                     These were set to the undefined value \n");
  }

  return (0);
}

gaint ffpow (struct gafunc *pfc, struct gastat *pst) {
struct gastat pst2;
gaint rc;

  if (pfc->argnum!=2) { 
    gaprnt (0,"Error from POW:  Too many or too few args \n");
    gaprnt (0,"                 Two arguments expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  pst2 = *pst;
  rc = gaexpr(pfc->argpnt[1],&pst2);
  if (rc) {
    gafree (pst);
    return (rc);
  }

  rc = gafopr (pst, &pst2, 10);
  if (rc) {
    gafree (pst);
    gafree (&pst2);
  }

  return (rc);
}

gaint ffmag (struct gafunc *pfc, struct gastat *pst) {
struct gastat pst2;
gaint rc;

  if (pfc->argnum!=2) {
    gaprnt (0,"Error from MAG:  Too many or too few args \n");
    gaprnt (0,"                 Two arguments expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  pst2 = *pst;
  rc = gaexpr(pfc->argpnt[1],&pst2);
  if (rc) {
    gafree (pst);
    return (rc);
  }

  rc = gafopr (pst, &pst2, 11);
  if (rc) {
    gafree (pst);
    gafree (&pst2);
  }
  return (rc);
}

/* Perform atan2 function */

gaint ffatan (struct gafunc *pfc, struct gastat *pst) {
struct gastat pst2;
gaint rc;

  if (pfc->argnum!=2) {
    gaprnt (0,"Error from ATAN2:  Too many or too few args \n");
    gaprnt (0,"                   Two arguments expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  pst2 = *pst;
  rc = gaexpr(pfc->argpnt[1],&pst2);
  if (rc) {
    gafree (pst);
    return (rc);
  }

  rc = gafopr (pst, &pst2, 12);
  if (rc) {
    gafree (pst);
    gafree (&pst2);
  }
  return (rc);
}

/* Performs a bi-linear interpolation between two grids, 
   adapted from the external UDF lterp.
*/ 
gaint fflterp (struct gafunc *pfc, struct gastat *pst) {
struct gastat pst2;
struct gagrid *pgr1,*pgr2;
gadouble (*iconv) (gadouble *, gadouble);
gadouble *x1=NULL,*x2=NULL,*y1=NULL,*y2=NULL;
gadouble *gr1,*gr2;
gadouble xd,yd,rd,t1,t2,tstrt,tscl;
gaint rc,i,j,i1,i2,ij,ij2,idir2,jdir2,j1,j2,error=0;
char *gr1u,*gr2u;
  
  if (pfc->argnum!=2) {
    gaprnt (0,"Error from LTERP: Two arguments expected \n");
    return (1);
  }

   /* Evaluate the 2nd expression, the destination grid */
  pst2=*pst;
  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) { error=1; goto err; }
  if (pst->type!=1) {
    gaprnt (0,"Error from LTERP: The 2nd argument is not a grid expression \n");
    error=1; goto err;
  }

  /* Check environment */
  if (pst->idim==-1) {
    gaprnt (0,"Error from LTERP: The destination grid is a constant \n");
    error=1; goto err;
  }
  if (pst->idim==2 || pst->idim==4 || pst->jdim==2 || pst->jdim==4) {
    gaprnt (0,"Error from LTERP: The grids cannot vary in Z or E \n");
    error=1; goto err;
  }

  /* Evaluate the 1st expression, the data values to be interpolated */
  rc = gaexpr(pfc->argpnt[0],&pst2);
  if (rc) { error=1; goto err; }
  if (pst2.type!=1) {
    gaprnt (0,"Error from LTERP: The 1st argument is not a grid expression \n");
    error=1; goto err;
  }

  /* Verify that the varying dimensions are equivalent */
  if (pst->idim!=pst2.idim || pst->jdim!=pst2.jdim) {
    printf ("Error from LTERP: Grids have different varying dimensions \n");
    error=1; goto err;
  }

  /* Get the grids */
  pgr1 = pst2.result.pgr;     /* data source grid (input)  */
  pgr2 = pst->result.pgr;     /* destination grid (result) */
  gr1  = pgr1->grid;
  gr1u = pgr1->umask;
  gr2  = pgr2->grid;
  gr2u = pgr2->umask;

  /* If the time dimension varies, the time increment must be equivalent 
     (we won't interpolate from months to minutes) */
  if (pst->idim==3) {
    if ((*(pgr1->ivals+6)<0.5 && *(pgr2->ivals+6)>0.8) || 
	(*(pgr1->ivals+6)>0.8 && *(pgr2->ivals+6)<0.5)) {
      printf ("Error from LTERP: Grids do not have matching time increments \n");
      error=1; goto err;
    }
  }
  if (pst->jdim==3) {
    if ((*(pgr1->jvals+6)<0.5 && *(pgr2->jvals+6)>0.8) || 
	(*(pgr1->jvals+6)>0.8 && *(pgr2->jvals+6)<0.5)) {
      printf ("Error from LTERP: Grids do not have matching time increments \n");
      error=1; goto err;
    }
  }

  /* get i dim coordinates for both grids */
  if (pgr1->idim>-1) {
    x1 = NULL;
    x1 = (gadouble *)galloc(sizeof(gadouble)*pgr1->isiz,"x1");
    if (x1==NULL) { error=1; goto err; }
    if (pgr1->idim==3) {
      j=0;
      for (i=pgr1->dimmin[pgr1->idim];i<=pgr1->dimmax[pgr1->idim];i++) {
	*(x1+j) = (gadouble)i;
	j++;
      }
    } else {
      iconv = pgr1->igrab;
      j=0;
      for (i=pgr1->dimmin[pgr1->idim];i<=pgr1->dimmax[pgr1->idim];i++) {
	*(x1+j) = iconv(pgr1->ivals,(gadouble)i);
	j++;
      }
    }
  }
  if (pgr2->idim>-1) {
    x2 = NULL;
    x2 = (gadouble *)galloc(sizeof(gadouble)*pgr2->isiz,"x2");
    if (x2==NULL) { error=1; goto err; }
    if (pgr2->idim==3) {
      j=0;
      for (i=pgr2->dimmin[pgr2->idim];i<=pgr2->dimmax[pgr2->idim];i++) {
	*(x2+j) = (gadouble)i;
	j++;
      }
    } else {
      iconv = pgr2->igrab;
      j=0;
      for (i=pgr2->dimmin[pgr2->idim];i<=pgr2->dimmax[pgr2->idim];i++) {
	*(x2+j) = iconv(pgr2->ivals,(gadouble)i);
	j++;
      }
    }
  }

  /* get j dim coordinates for both grids */
  if (pgr1->jdim>-1) {
    y1 = NULL;
    y1 = (gadouble *)galloc(sizeof(gadouble)*pgr1->jsiz,"y1");
    if (y1==NULL) { error=1; goto err; }
    if (pgr1->jdim==3) {
      j=0;
      for (i=pgr1->dimmin[pgr1->jdim];i<=pgr1->dimmax[pgr1->jdim];i++) {
	*(y1+j) = (gadouble)i;
	j++;
      }
    } else {
      iconv = pgr1->jgrab;
      j=0;
      for (i=pgr1->dimmin[pgr1->jdim];i<=pgr1->dimmax[pgr1->jdim];i++) {
	*(y1+j) = iconv(pgr1->jvals,(gadouble)i);
	j++;
      }
    }
  }
  if (pgr2->jdim>-1) {
    y2 = NULL;
    y2 = (gadouble *)galloc(sizeof(gadouble)*pgr2->jsiz,"y2");
    if (y2==NULL) { error=1; goto err; }
    if (pgr2->jdim==3) {
      j=0;
      for (i=pgr2->dimmin[pgr2->jdim];i<=pgr2->dimmax[pgr2->jdim];i++) {
	*(y2+j) = (gadouble)i;
	j++;
      }
    } else {
      iconv = pgr2->jgrab;
      j=0;
      for (i=pgr2->dimmin[pgr2->jdim];i<=pgr2->dimmax[pgr2->jdim];i++) {
	*(y2+j) = iconv(pgr2->jvals,(gadouble)i);
	j++;
      }
    }
  }

  /* Normalize the time coordinates.  We want the grid time values to
     be the same coordinate system for each grid.  */
  if (pst->idim==3 || pst->jdim==3) {
    tstrt = (gadouble)pgr2->dimmin[3];
    if (pst->idim==3) {               
      /* the i dimension is T-varying */
      /* determine scaling factor */
      if (pgr1->ivals[6]>0.8) 
	tscl = pgr2->ivals[6] / pgr1->ivals[6] ;
      else 
	tscl = pgr2->ivals[5] / pgr1->ivals[5] ;
      /* normalize */
      for (i=0; i<pgr2->isiz; i++) *(x2+i) = tstrt+tscl*(gadouble)i;
    } 
    else {                            
      /* the j dimension is T-varying */
      /* determine scaling factor */
      if (pgr1->jvals[6]>0.8) 
	tscl = pgr2->jvals[6] / pgr1->jvals[6] ;
      else 
	tscl = pgr2->jvals[5] / pgr1->jvals[5] ;
      /* normalize */
      for (i=0; i<pgr2->jsiz; i++) *(y2+i) = tstrt+tscl*(gadouble)i;
    }
  }

  /* see which way x y vary with i */
  if (pgr2->ilinr == 0) {
    idir2 = -1;
    if (*(x2+1) > *x2) idir2 = 1;
  } else {
    idir2 = 1;
  }

  if (pgr2->jlinr == 0) {
    jdir2 = -1;
    if (*(y2+1) > *y2) jdir2 = 1;
  } else {
    jdir2 = 1;
  }

  /* Do the interpolation */
  if (pgr1->jdim<0) {
    /* 1D */
    i1 = -1;
    for (i2=0; i2<pgr2->isiz; i2++) {
      if (idir2 == 1) {
	while (i1<pgr1->isiz-1 && *(x1+i1+1)<=*(x2+i2)) i1++;
      } else {
	while (i1<pgr1->isiz-1 && *(x1+i1+1)>=*(x2+i2)) i1++;
      } 
      if (i1==pgr1->isiz-1 && *(x1+i1)==*(x2+i2)) i1--;
      if (i1<0 || i1>pgr1->isiz-2 || *(gr1u+i1)==0 || *(gr1u+i1+1)==0) 
	*(gr2u+i2) = 0;
      else {
        xd = *(x1+i1+1) - *(x1+i1);
        rd = *(gr1+i1+1) - *(gr1+i1);
        *(gr2+i2) = *(gr1+i1)+(*(x2+i2)-*(x1+i1))*rd/xd;
	*(gr2u+i2) = 1;
      }
    }
  } else {
    /* 2D */
    j1 = -1;
    for (j2=0; j2<pgr2->jsiz; j2++) {
      if (jdir2 == 1) {
	while (j1<pgr1->jsiz-1 && *(y1+j1+1)<=*(y2+j2)) j1++;
      } else {
	while (j1<pgr1->jsiz-1 && *(y1+j1+1)>=*(y2+j2)) j1++;
      }
      if (j1==pgr1->jsiz-1 && *(y1+j1)==*(y2+j2)) j1--;
      i1 = -1;
      for (i2=0; i2<pgr2->isiz; i2++) {
	if(idir2 == 1) {
	  while (i1<pgr1->isiz-1 && *(x1+i1+1)<=*(x2+i2)) i1++;
	} else {
	  while (i1<pgr1->isiz-1 && *(x1+i1+1)>=*(x2+i2)) i1++;
	} 
        if (i1==pgr1->isiz-1 && *(x1+i1)==*(x2+i2)) i1--;
        ij = j1*pgr1->isiz+i1;
        ij2 = j2*pgr2->isiz+i2;
        if ( i1<0 || i1>pgr1->isiz-2 || 
	     j1<0 || j1>pgr1->jsiz-2 ||
             *(gr1u+ij)==0 || *(gr1u+ij+1)==0 ||
             *(gr1u+ij+pgr1->isiz)==0 || *(gr1u+ij+pgr1->isiz+1)==0 ) 
	  *(gr2u+ij2) = 0;
        else {
          yd = *(y1+j1+1) - *(y1+j1);
          rd = *(gr1+ij+pgr1->isiz) - *(gr1+ij);
          t1 = *(gr1+ij) + (*(y2+j2)-*(y1+j1))*rd/yd;
          rd = *(gr1+ij+pgr1->isiz+1) - *(gr1+ij+1);
          t2 = *(gr1+ij+1) + (*(y2+j2)-*(y1+j1))*rd/yd;
          xd = *(x1+i1+1) - *(x1+i1);
          *(gr2+ij2) = t1 + (*(x2+i2)-*(x1+i1))*(t2-t1)/xd;
	  *(gr2u+ij2) = 1;
        }
      }
    }
  }
      
err:

  /* release memory */
  if (error) if (pst) gafree (pst); 
  if (&pst2) gafree (&pst2);
  if (x1!=NULL) gree(x1,"f400");
  if (x2!=NULL) gree(x2,"f401");
  if (y1!=NULL) gree(y1,"f402");
  if (y2!=NULL) gree(y2,"f403");
  if (error) 
    return 1;
  else 
    return 0;
}

/* Perform a two-operand operation which may have both grid or
   stn data types involved.  */

gaint gafopr (struct gastat *pst1, struct gastat *pst2, gaint op) {
struct gagrid *pgr;
struct gastn *stn;

  /* Check for grid-grid operation */
  pgr=NULL;
  if (pst1->type == 1 && pst2->type==1) {
    pgr = gagrop(pst1->result.pgr, pst2->result.pgr, op, 1);
    if (pgr==NULL) return (1);
    pst1->type = 1;
    pst1->result.pgr = pgr;
    return (0);
  }

  /* If both stns, do stn-stn operation */
  if (pst1->type==0 && pst2->type==0 ) {
    stn = gastop(pst1->result.stn, pst2->result.stn, op, 1);
    if (stn==NULL) return (1);
    pst1->type = 0;
    pst1->result.stn = stn;
    return (0);
  }

  /* Operation between grid and stn is invalid -- unless the grid
     is really a constant.  Check for this.  */
  if (pst1->type == 1) pgr=pst1->result.pgr;
  if (pst2->type == 1) pgr=pst2->result.pgr;
  if (pgr->idim == -1 && pgr->jdim == -1) {
    if (pst1->type == 0) {
      stn = gascop (pst1->result.stn, pgr->rmin, op, 0);
    } else {
      stn = gascop (pst2->result.stn, pgr->rmin, op, 1);
    }
    if (stn==NULL) return (1);
    gagfre (pgr);
    pst1->type = 0;
    pst1->result.stn = stn;
  } else {
    gaprnt (0,"Operation Error: Incompatable Data Types\n");
    gaprnt (0,"  One operand was stn data, other was grid\n");
    return (1);
  }
  return (0);
}

char *aavenam[4] = {"AAVE","AMEAN","ASUM","ASUMG"};

gaint ffaav (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = aave (pfc, pst, 1);
   return (rc);
}

gaint ffamn (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = aave (pfc, pst, 2);
   return (rc);
}

gaint ffasum (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = aave (pfc, pst, 3);
   return (rc);
}

gaint ffasumg (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = aave (pfc, pst, 4);
   return (rc);
}

gaint aave (struct gafunc *pfc, struct gastat *pst, int sel) {
gadouble (*iconv) (gadouble *, gadouble);
gadouble (*jconv) (gadouble *, gadouble);
struct gagrid *pgr;
struct gafile *pfi;
gadouble res=-999,x1,x2,y1,y2,undef;
gaint dim,wflag,rc,gflag=0;
char *ch,*fnam,resu;

  fnam = aavenam[sel-1];

  /* Check for valid number of args       */
  if (pfc->argnum==2 && !strncmp(pfc->argpnt[1],"global",1)) gflag=1;
  if (pfc->argnum!=5 && !gflag) {
    sprintf(pout,"Error from %s:  Too many or too few args\n",fnam);
    gaprnt(0,pout);
    gaprnt (0,"                  5 arguments expected \n");
    return (1);
  }

  /* Check environment.  Z or T or E can't vary.  */
  if (pst->idim>1 || pst->jdim>1) {
    sprintf(pout,"Error from %s  Invalid environment.  ",fnam);
    gaprnt(0,pout);
    gaprnt (0,"Z, T, or E can't vary.\n");
    return (1);
  }

  /* Parse the dimension expressions */
  pfi = pst->pfid;
  if (gflag) {
    ch = dimprs ("lon=0", pst, pfi, &dim, &x1, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag && sel!=4) x1 = x1 - 0.5;
    ch = dimprs ("lon=360", pst, pfi, &dim, &x2, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag && sel!=4) x2 = x2 + 0.5;
    ch = dimprs ("lat=-90", pst, pfi, &dim, &y1, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag && sel!=4) y1 = y1 - 0.5;
    ch = dimprs ("lat=90", pst, pfi, &dim, &y2, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag && sel!=4) y2 = y2 + 0.5;
  } else {
    pfi = pst->pfid;
    ch = dimprs (pfc->argpnt[1], pst, pfi, &dim, &x1, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag && sel!=4) x1 = x1 - 0.5;
    ch = dimprs (pfc->argpnt[2], pst, pfi, &dim, &x2, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag && sel!=4) x2 = x2 + 0.5;
    ch = dimprs (pfc->argpnt[3], pst, pfi, &dim, &y1, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag && sel!=4) y1 = y1 - 0.5;
    ch = dimprs (pfc->argpnt[4], pst, pfi, &dim, &y2, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag && sel!=4) y2 = y2 + 0.5;
  }

  /* Set up pst block properly to get the grid */
  iconv = pfi->gr2ab[0];
  pst->dmin[0] = iconv(pfi->grvals[0],x1);
  pst->dmax[0] = iconv(pfi->grvals[0],x2);
  jconv = pfi->gr2ab[1];
  pst->dmin[1] = jconv(pfi->grvals[1],y1);
  pst->dmax[1] = jconv(pfi->grvals[1],y2);
  pst->idim = 0;
  pst->jdim = 1;

  /* Get the grid */

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type!=1) {
    gafree (pst);
    return (-1);
  }
  pgr = pst->result.pgr;

  /* Average over the grid  */
  resu = doaave(pgr,pst->dmin[0],pst->dmax[0],pst->dmin[1],pst->dmax[1],sel,&res);
  undef = pgr->undef;
  gafree (pst);
  pgr = gagrvl(res);
  pgr->umin = resu;
  pgr->undef = undef;
  pst->type = 1;
  pst->result.pgr = pgr;
  return (0);

err1:
  sprintf(pout,"Error from %s:  Invalid dimension expression \n",fnam);
  gaprnt(0,pout);
  return (1);
}

gaint ffscor (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = scorr (pfc, pst, 1);
   return (rc);
}

gaint ffsreg (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
  rc = scorr (pfc,pst,2);
  return (rc);
}

gaint scorr (struct gafunc *pfc, struct gastat *pst, gaint sel) {
gadouble (*iconv) (gadouble *, gadouble);
gadouble (*jconv) (gadouble *, gadouble);
struct gagrid *pgr,*pgr2;
struct gafile *pfi;
struct gastat pst2;
gadouble *gr1, *gr2, *gr3, *grid3;
gadouble res,x1,x2,y1,y2,undef,mn1,mn2,s1,s2,ss,cov;
gaint i,dim,wflag,rc,cnt,gflag=0;
char *ch,*gr1u,*gr2u,*grid3u,*gr3u,s1u,s2u,mn1u,mn2u,resu,covu;

   mn1=mn2=s1=s2=cov=ss=res=0;   

  /* Check for valid number of args       */
  if (pfc->argnum==3 && !strncmp(pfc->argpnt[2],"global",1)) gflag=1;
  if (pfc->argnum!=6 && !gflag) {
    gaprnt (0,"Error from SCORR:  Too many or too few args \n");
    gaprnt (0,"                   6 arguments expected \n");
    return (1);
  }

  /* Check environment.  Z or T or E can't vary.  */
  if (pst->idim>1 || pst->jdim>1) {
    gaprnt (0,"Error from SCORR:  Invalid environment.  ");
    gaprnt (0,"Z, T, or E cannot vary.\n");
    return (1);
  }

  /* Parse the dimension expressions */
  pfi = pst->pfid;
  if (gflag) {
    ch = dimprs ("lon=0", pst, pfi, &dim, &x1, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag) x1 = x1 - 0.5;
    ch = dimprs ("lon=360", pst, pfi, &dim, &x2, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag) x2 = x2 + 0.5;
    ch = dimprs ("lat=-90", pst, pfi, &dim, &y1, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag) y1 = y1 - 0.5;
    ch = dimprs ("lat=90", pst, pfi, &dim, &y2, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag) y2 = y2 + 0.5;
  } else {
    ch = dimprs (pfc->argpnt[2], pst, pfi, &dim, &x1, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag) x1 = x1 - 0.5;
    ch = dimprs (pfc->argpnt[3], pst, pfi, &dim, &x2, 1, &wflag);
    if (ch==NULL || dim!=0) goto err1;
    if (!wflag) x2 = x2 + 0.5;
    ch = dimprs (pfc->argpnt[4], pst, pfi, &dim, &y1, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag) y1 = y1 - 0.5;
    ch = dimprs (pfc->argpnt[5], pst, pfi, &dim, &y2, 1, &wflag);
    if (ch==NULL || dim!=1) goto err1;
    if (!wflag) y2 = y2 + 0.5;
  }

  /* Set up pst block properly to get the grids */
  iconv = pfi->gr2ab[0];
  pst->dmin[0] = iconv(pfi->grvals[0],x1);
  pst->dmax[0] = iconv(pfi->grvals[0],x2);
  jconv = pfi->gr2ab[1];
  pst->dmin[1] = jconv(pfi->grvals[1],y1);
  pst->dmax[1] = jconv(pfi->grvals[1],y2);
  pst->idim = 0;
  pst->jdim = 1;
  pst2 = *pst;

  /* Get the first grid */
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type!=1) {
    gafree (pst);
    return (-1);
  }
  pgr = pst->result.pgr;

  /* Get the 2nd grid */
  rc = gaexpr(pfc->argpnt[1],&pst2);
  if (rc) return (rc);
  if (pst2.type!=1) {
    gafree (&pst2);
    gafree (pst);
    return (-1);
  }
  pgr2 = pst2.result.pgr;

  /* Verify that the grids are compatible for operations */
  if (gagchk(pgr,pgr2,0) || gagchk(pgr,pgr2,1)) {
    gaprnt (0,"Error from SCORR:  Incompatable grids\n");
    gafree (&pst2);
    gafree (pst);
    return (1);
  }

  /* Force missing data values to be reflected in each grid.  */
  cnt = pgr->isiz * pgr->jsiz;
  gr1 = pgr->grid;
  gr2 = pgr2->grid;
  gr1u = pgr->umask;
  gr2u = pgr2->umask;
  for (i=0; i<cnt; i++) {
    if (*gr1u==0 || *gr2u==0) {
      *gr1u = 0;
      *gr2u = 0;
    }
    gr1++;  gr2++;
    gr1u++; gr2u++;
  }

  /* Obtain areal average over each grid */
  mn1u = doaave(pgr, pst->dmin[0],pst->dmax[0],pst->dmin[1],pst->dmax[1],1,&mn1);
  mn2u = doaave(pgr2,pst->dmin[0],pst->dmax[0],pst->dmin[1],pst->dmax[1],1,&mn2);

  /* result from doaave was undefined */
  if (mn1u==0 || mn2u==0) {
    resu = 0;
  } 
  else {

    /* Remove the mean from the fields */
    gr1 = pgr->grid;
    gr2 = pgr2->grid;
    gr1u = pgr->umask;
    gr2u = pgr2->umask;
    for (i=0; i<cnt; i++) {
      if (*gr1u!=0) *gr1 = *gr1 - mn1;
      if (*gr2u!=0) *gr2 = *gr2 - mn2;
      gr1++;  gr2++;
      gr1u++; gr2u++;
    }

    /* Get gr1 * gr2; stash in a safe place.  Also get the squares of each variable. */
    grid3 = (gadouble *)galloc(sizeof(gadouble)*cnt,"scorr3");
    if (grid3==NULL) {
      gafree (pst);
      gafree (&pst2);
      gaprnt (0,"Error from SCORR:  Memory Allocation \n");
      return (1);
    }
    grid3u = (char *)galloc(sizeof(char)*cnt,"scorr3u");
    if (grid3u==NULL) {
      if (grid3) gree(grid3,"f408");
      gafree (pst);
      gafree (&pst2);
      gaprnt (0,"Error from SCORR:  Memory Allocation \n");
      return (1);
    }
    gr1 = pgr->grid;
    gr2 = pgr2->grid;
    gr3 = grid3;
    gr1u = pgr->umask;
    gr2u = pgr2->umask;
    gr3u = grid3u;
    for (i=0; i<cnt; i++) {
      if (*gr1u!=0 && *gr2u!=0) {
	*gr3 = *gr1 * *gr2;
	*gr3u = 1;
      }
      else {
	*gr3u = 0;
      }
      if (*gr1u!=0) *gr1 = *gr1 * *gr1;
      if (*gr2u!=0) *gr2 = *gr2 * *gr2;
      gr1++;  gr2++;  gr3++;
      gr1u++; gr2u++; gr3u++;
    }

    /* Get the areal average of the squares, then the
       areal average of gr1 * gr2, then the final result */
    /* hard wire to 1 for now as this is the default behaviour */
    s1u = doaave(pgr, pst->dmin[0],pst->dmax[0],pst->dmin[1],pst->dmax[1],1,&s1);
    s2u = doaave(pgr2,pst->dmin[0],pst->dmax[0],pst->dmin[1],pst->dmax[1],1,&s2);
    gr1 = pgr->grid;
    gr1u = pgr->umask;
    gr3 = grid3;
    gr3u = grid3u;
    for (i=0; i<cnt; i++) {
      if (*gr3u!=0) {
	*gr1 = *gr3;
	*gr1u = 1;
      }
      else {
	*gr1u=0;
      }
      gr1++;  gr3++;
      gr1u++; gr3u++;
    }
    covu = doaave(pgr,pst->dmin[0],pst->dmax[0],pst->dmin[1],pst->dmax[1],1,&cov);
    if (sel == 1) ss = sqrt(s1*s2);
    if (sel == 2) ss = sqrt(s1*s1);
    if (ss>0.0) {
      res = cov/ss;
      resu = 1;
    }
    else resu = 0;
    gree(grid3,"f408a");
    gree(grid3u,"f408c");
  }

  undef = pgr->undef;

  gafree (pst);
  gafree (&pst2);

  pgr = gagrvl(res);
  pgr->umin = resu;
  pgr->undef = undef; 
  pst->type = 1;
  pst->result.pgr = pgr;
  return (0);

err1:
  gaprnt (0,"Error from SCORR:  Invalid dimension expression \n");
  return (1);
}

/* Function that actually does area average over a grid */

char doaave(struct gagrid *pgr, gadouble dmin0, gadouble dmax0,
		gadouble dmin1, gadouble dmax1, gaint sel, gadouble *result) {
gadouble (*iconv) (gadouble *, gadouble);
gadouble (*jconv) (gadouble *, gadouble);
gadouble *ivals, *jvals, *gr;
gadouble rad,sum,w1,w2=0,y1,x1,abs,alo,ahi,alen,wt;
gaint i,j;
char *gru,sumu=0;

  rad = 3.1416/180.0;
  iconv = pgr->igrab;
  jconv = pgr->jgrab;
  ivals = pgr->ivals;
  jvals = pgr->jvals;
  sum = 0.0; 
  wt = 0.0;
  gr  = pgr->grid;
  gru = pgr->umask;
  for (j=0; j<pgr->jsiz; j++) {
    w1 = 1.0;
    y1 = (gadouble)(j+pgr->dimmin[1]);
    abs = jconv(jvals, y1);
    alo = jconv(jvals, y1-0.5);
    ahi = jconv(jvals, y1+0.5);
    alen=fabs(ahi-alo);                /* length of the grid side in world coord */
    if (alo < dmin1) alo = dmin1;
    if (alo > dmax1) alo = dmax1;
    if (ahi < dmin1) ahi = dmin1;
    if (ahi > dmax1) ahi = dmax1;
    if (alo < -90.0) alo = -90.0; if (ahi < -90.0) ahi = -90.0;
    if (alo >  90.0) alo =  90.0; if (ahi >  90.0) ahi =  90.0;
    if (sel==1) {
      w1 = fabs(sin(ahi*rad)-sin(alo*rad));  /* area weighting (aave) */
    } 
    else if (sel==2) { 
      w1 = fabs(ahi-alo);                    /* simple weighting (amean) */
    } 
    else if (sel==3) {
      if (alen > FUZZ_SCALE) {                /* grid weighting (asum) */
	w1=fabs(ahi-alo)/alen;                 
      } 
      else {
	w1=0.0;
      }
    }

    for (i=0; i<pgr->isiz; i++) {
      x1 = (gadouble)(i+pgr->dimmin[0]);
      alo = iconv(ivals, x1-0.5);
      ahi = iconv(ivals, x1+0.5);
      alen=fabs(ahi-alo);
      if (alo < dmin0) alo = dmin0;
      if (alo > dmax0) alo = dmax0;
      if (ahi < dmin0) ahi = dmin0;
      if (ahi > dmax0) ahi = dmax0;

      if(sel==1) {
	w2 = ahi - alo;
      } 
      else if (sel==2) {
	w2 = ahi - alo;
      } 
      else if (sel==3) {
	if (alen > FUZZ_SCALE) {                /* grid weighting (asum) */
	  w2=fabs(ahi-alo)/alen;                 
	} 
	else {
	  w2=0.0;
	}
      } 
      else if (sel==4) {
	w2=1.0;              /* no weighting (asumg) */
      }

      if (*gru != 0) {
	if (sel<=3) {
	  sum = sum + (*gr * w1 * w2);
	} 
	else if (sel==4) {
	  sum = sum + *gr;    /* no weighting (asumg) */   
	}
        wt = wt + (w1 * w2);
      }
      gr++; gru++;
    }
  }
  if (wt>0.0) {
    sumu = 1;
    if (sel<=2) {
      sum = sum / wt;
    }
  } 
  else {
    sumu = 0;
    sum = pgr->undef;
  }
  *result = sum;
  return (sumu);
}

/* Time correlation */

gaint fftcor (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = tmaskf (pfc, pst, 2);
   return (rc);
}

/* Time regression */  /* Timlin */

gaint fftreg (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
  rc = tmaskf (pfc,pst,3);
  return (rc);
}

/* Time mean, masked and arbitrarily weighted. */

gaint fftmav (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = tmaskf (pfc, pst, 1);
   return (rc);
}

char *tmnam[3] = {"TMAVE","TCORR","TREGR"};

/* Following function does time series operations that
   involve a mask grid, where if the first mask grid
   is undefined, the 2nd grid is not even accessed.
   This saves processing time for composite means and
   such.  Variable sel determines the function operation:

     sel = 1:  time mean.
     sel = 2:  time correlation.
     sel = 3:  time regression.     */

gaint tmaskf (struct gafunc *pfc, struct gastat *pst, gaint sel) {
struct gafile *pfi;
struct gagrid *pgr;
gadouble *gr, *mn1, *mn2, *cnt, *s1, *s2, *cov, *wt;
gadouble t1,t2,vv,res=0,v1,v2;
gaint dim,wflag,d1,d2,d,rc,i,siz,size=0;
char *ch,*fnam, uu, *gru, *mn1u, *mn2u, *cntu, *s1u, *s2u, *covu, resu=0;

  cov=s2=s1=cnt=mn2=wt=0;
  s1u=mn2u=mn1u=NULL;

  fnam = tmnam[sel-1];
  /* Check for valid number of args       */
  if (pfc->argnum!=4) {
    sprintf (pout,"Error from %s:  Too many or too few args\n",fnam);
    gaprnt (0,pout);
    gaprnt (0,"                   4 arguments expected \n");
    return (1);
  }

  /* Parse the dimension expression       */
  pfi = pst->pfid;
  ch = dimprs (pfc->argpnt[2], pst, pfi, &dim, &t1, 1, &wflag);
  if (ch==NULL || dim!=3 ) {
    sprintf (pout,"Error from %s:  1st dimension expr invalid\n",fnam);
    gaprnt (0,pout);
    return (1);
  }

  /* Now parse the 2nd dimension expression.  */
  ch = dimprs (pfc->argpnt[3], pst, pfi, &dim, &t2, 1, &wflag);
  if (ch==NULL || dim!=3 || t2<t1) {
    sprintf (pout,"Error from %s:  2nd dimension expr invalid\n",fnam);
    gaprnt (0,pout);
    if (t2<t1) gaprnt(0,"  End time is earlier than start time \n");
    return (1);
  }

  /* Loop through all times and get means */
  if (pst->jdim==dim) pst->jdim = -1;
  d1 =  ceil(t1-0.001);          /* Loop limits are integers    */
  d2 = floor(t2+0.001);          /* No weighting  */
  sprintf (pout,"%s:  dim = %i, start = %i, end = %i\n", fnam, dim, d1, d2);
  gaprnt (2,pout);

  rc = 0;
  mn1 = NULL;
  for (d=d1; d<=d2 && !rc; d++) {
    gr2t (pfi->grvals[3],d,&(pst->tmin));
    pst->tmax = pst->tmin;
    rc = gaexpr(pfc->argpnt[0],pst);
    if (rc) goto err2;
    if (!rc && pst->type==0) {
      gafree (pst);
      goto err3;
    }
    pgr = pst->result.pgr;
    if (pgr->idim != -1 || pgr->isiz!=1 || pgr->jsiz!=1) {
      sprintf (pout,"Error from %s:  1st arg must be 0-D\n",fnam);
      gaprnt (0,pout);
      gafree (pst);
      goto err2;
    }
    vv = pgr->rmin;
    uu = pgr->umin;
    gafree (pst);
    if (uu!=0) {
      rc = gaexpr(pfc->argpnt[1],pst);
      if (rc) goto err2;
      if (!rc && pst->type==0) {
        gafree (pst);
        goto err3;
      }
      pgr = pst->result.pgr;
      siz = pgr->isiz * pgr->jsiz;
      if (mn1==NULL) {
        i = 2;
        if (sel==2 || sel==3) i = 6;
        mn1 = (gadouble *)galloc(sizeof(gadouble)*siz*i,"tmaskmn1");
        if (mn1==NULL) {
          gafree (pst);
          goto err1;
        }
        mn1u = (char *)galloc(sizeof(char)*siz*i,"tmaskmn1u");
        if (mn1u==NULL) {
          gafree (pst);
          goto err1;
        }
        if (sel==1) {
          wt = mn1 + siz;
          for (i=0; i<siz; i++) { 
	    *(mn1+i)=0.0; 
	    *(wt+i)=0.0; 
	  }
        }
        if (sel==2 || sel==3) {
          mn2  = mn1  + siz;       /* these are data grids */
          cnt = mn2 + siz;
          s1  = cnt + siz;
          s2  = s1 + siz;
          cov = s2 + siz;

	  mn2u = mn1u + siz;       /* these are undef masks */
          cntu = mn2u + siz;
          s1u  = cntu + siz;
          s2u  = s1u + siz;
          covu = s2u + siz;

          for (i=0; i<siz; i++) {
            *(mn1+i) = 0.0; 
	    *(mn2+i) = 0.0;
            *(cnt+i) = 0.0;
            *(s1+i)  = 0.0; 
	    *(s2+i)  = 0.0;
            *(cov+i) = 0.0;
          }
        }
        size = siz;
      }
      if (size != siz) {
        gafree (pst);
        goto err2;
      }
      gr  = pgr->grid;
      gru = pgr->umask;
      for (i=0; i<siz; i++) {
        if (uu!=0 && *gru!=0) {
          if (sel==1) {
            *(mn1+i) += *gr * vv;
            *(wt+i) += vv;
          }
          if (sel==2 || sel==3) {
            *(mn1+i) += vv;
            *(mn2+i) += *gr;
            *(cnt+i) += 1.0;
          }
        }
        gr++; gru++;
      }
      gafree (pst);
    }
  }

  /* Calculate mean of each time series */

  if (mn1) {
    if (sel==1) {
      for (i=0; i<size; i++) {
        if (*(wt+i)>0.0) {
          *(mn1+i) = *(mn1+i) / *(wt+i);
	  *(mn1u+i) = 1;
        } else {
          *(mn1u+i) = 0;
        }
      }
    }
    if (sel==2 || sel==3) {
      for (i=0; i<size; i++) {
        if (*(cnt+i)>0.0) {
          *(mn1+i) = *(mn1+i) / *(cnt+i);
          *(mn2+i) = *(mn2+i) / *(cnt+i);
	  *(mn1u+i) = 1;
	  *(mn2u+i) = 1;
        } else {
          *(mn1u+i) = 0;
          *(mn2u+i) = 0;
        }
      }
    }
  }

  /* Loop through time again if needed; do squares and cov.
     Less error checking this time through. */

  if ((sel==2 || sel==3) && mn1) {
    rc = 0;
    for (d=d1; d<=d2 && !rc; d++) {
      gr2t (pfi->grvals[3],d,&(pst->tmin));
      pst->tmax = pst->tmin;
      rc = gaexpr(pfc->argpnt[0],pst);
      if (rc) goto err2;
      pgr = pst->result.pgr;
      vv = pgr->rmin;
      uu = pgr->umin;
      gafree (pst);
      rc = gaexpr(pfc->argpnt[1],pst);
      if (rc) goto err2;
      pgr = pst->result.pgr;
      gr  = pgr->grid;
      gru = pgr->umask;
      for (i=0; i<size; i++) {
        if (uu!=0 && *gru!=0) {
          if (*(cnt+i) > 0.0) {
            v1 = vv - *(mn1+i);
            *(s1+i) += v1*v1;
            v2 = *gr - *(mn2+i);
            *(s2+i) += v2*v2;
            *(cov+i) += v1*v2;
          }
        }
        gr++; gru++;
      }
      gafree (pst);
    }

    for (i=0; i<size; i++) {
      if (*(cnt+i) > 0.0) {
        *(s1+i) = *(s1+i) / *(cnt+i);
        *(s2+i) = *(s2+i) / *(cnt+i);
        *(cov+i) = *(cov+i) / *(cnt+i);
        if (sel==2) {
	  res = sqrt(*(s1+i) * *(s2+i));
	  resu = 1;
	}
        if (sel==3) {
	  res = sqrt(*(s1+i) * *(s1+i));
	  resu = 1;
	}
        if (res==0.0) {
	  resu = 0;
	}
        else {
	  res = *(cov+i)/res;
	  resu = 1;
	}
      } else resu = 0;
      *(s1+i)  = res;
      *(s1u+i) = resu;
    }
  }

  /* Get one final grid, and use it to return the result. */

  gr2t (pfi->grvals[3],d1,&(pst->tmin));
  pst->tmax = pst->tmin;
  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) goto err2;
  if (!rc && pst->type==0) {
    gafree (pst);
    goto err3;
  }
  pgr = pst->result.pgr;
  siz = pgr->isiz * pgr->jsiz;
  gr  = pgr->grid;
  gru = pgr->umask;
  if (mn1) {
    if (size != siz) {
      gafree (pst);
      goto err2;
    }
    if (sel==1) {
      for (i=0; i<siz; i++) {
	if (*(mn1u+i)!=0) {
	  *(gr+i) = *(mn1+i);
	  *(gru+i) = 1;
	}
	else {
	  *(gru+i) = 0;
	}
      }
    }
    if (sel==2 || sel==3) {
      for (i=0; i<siz; i++) {
	if (*(s1u+i)!=0) {
	  *(gr+i) = *(s1+i);
	  *(gru+i) = 1;
	}
	else {
	  *(gru+i) = 0;
	}
      }
    }
    gree(mn1,"f409");
    gree(mn1u,"f410");
  } else {
    for (i=0; i<siz; i++) *(gru+i) = 0;
  }
  return (0);

err1:
  sprintf (pout,"Error from %s:  Memory allocation error\n",fnam);
  gaprnt (0,pout);
  if (mn1) gree(mn1,"f411");
  if (mn1u) gree(mn1u,"f412");
  return (1);
err2:
  sprintf (pout,"Error from %s:  Error getting grids\n",fnam);
  gaprnt (0,pout);
  if (mn1) gree(mn1,"f413");
  if (mn1u) gree(mn1u,"f414");
  return (1);
err3:
  sprintf (pout,"Error from %s:  Args must be grid data\n",fnam);
  gaprnt (0,pout);
  if (mn1) gree(mn1,"f415");
  if (mn1u) gree(mn1u,"416");
  return (1);
}


char *avenam[8] = {"AVE","MEAN","SUM","SUMG","MIN","MAX","MINLOC","MAXLOC"};

gaint ffave (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 1);
   return (rc);
}

gaint ffmn (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 2);
   return (rc);
}

gaint ffsum (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 3);
   return (rc);
}


gaint ffsumg (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 4);
   return (rc);
}

gaint ffmin (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 5);
   return (rc);
}

gaint ffmax (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 6);
   return (rc);
}

gaint ffminl (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 7);
   return (rc);
}

gaint ffmaxl (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
   rc = ave (pfc, pst, 8);
   return (rc);
}


gaint ave (struct gafunc *pfc, struct gastat *pst, gaint sel) {
struct gagrid *pgr1, *pgr2, *pgr;
struct gafile *pfi;
struct dt tinc;
gadouble (*conv) (gadouble *, gadouble);
gadouble gr1, gr2, *sum, *cnt, *val;
gadouble alo, ahi, alen, wlo=0, whi=0, rd1;
gadouble wt, wt1, abs;
gaint mos, mns, wflag;
gaint i, rc, siz, dim, d, d1, d2, dim2, ilin, incr, bndflg;
char *ch,*fnam,*sumu,*cntu,*valu;

  fnam=avenam[sel-1];

  /* Check for valid number of args */
  if (pfc->argnum<3 || pfc->argnum>5) {
    sprintf(pout,"Error from %s:  Too many or too few args \n",fnam);
    gaprnt(0,pout);
    gaprnt (0,"                 3 to 5 arguments expected \n");
    return (1);
  }

  /* Parse the 1st dimension expression */
  pfi = pst->pfid;
  ch = dimprs (pfc->argpnt[1], pst, pfi, &dim, &gr1, 1, &wflag);
  if (ch==NULL) {
    sprintf(pout,"Error from %s:  1st dimension expression invalid\n",fnam);
    gaprnt(0,pout);
    return (1);
  }

  /* Parse the 2nd dimension expression */
  ch = dimprs (pfc->argpnt[2], pst, pfi, &dim2, &gr2, 1, &wflag);
  if (ch==NULL || dim2!=dim || gr2<gr1) {
    sprintf(pout,"Error from %s:  2nd dimension expression invalid\n",fnam);
    gaprnt(0,pout);
    if (dim2!=dim) 
      gaprnt(0,"  Start and end points have different dimensions\n");
    if (gr2<gr1) 
      gaprnt(0,"  End grid point is less than start grid point \n");
    return (1);
  }

  /* Check for 4th argument.  Could be a time increment, or it may
     be option flags.  Time increment only valid for time averaging */
  bndflg = 0;
  incr = 1;
  if (pfc->argnum == 4) {
    if (*(pfc->argpnt[3]) == '-') {    /* Option flags? */
      if (*(pfc->argpnt[3]+1) == 'b') bndflg = 1;
      else {
        sprintf(pout,"Error from %s: Invalid option flags\n",fnam);
        gaprnt(0,pout);
        return(1);
      }
    } else {                           
      /* Must be time increment */
      if (dim!=3) {
        sprintf(pout,"Error from %s: Invalid usage of increment value\n",fnam);
        gaprnt(0,pout);
        gaprnt (0,"                Can only be used with time averaging\n");
        return (1);
      }
      ch = intprs(pfc->argpnt[3],&incr);
      if (ch==NULL) goto err3;

      /* If a relative date/time was given, the increment is obtained
         by looking at the default file structure (which assumes
         knowledge of how date/time conversions are done) */
      if (*ch!='\0') {
        ch = rdtprs(pfc->argpnt[3],&tinc);
        if (ch==NULL) goto err3;
        mos = tinc.yr*12 + tinc.mo;
        mns = tinc.dy*1140 + tinc.hr*60 + tinc.mn;
        val = pfi->grvals[3];
        if (mos>0 && *(val+5)>0) {
          incr = mos / (*(val+5));
          if (mos!=incr*(*(val+5))) goto err3;
        }
        else if (mns>0 && *(val+6)>0) {
          incr = mns / (*(val+6));
          if (mns!=incr*(*(val+6))) goto err3;
        }
        else goto err3;
      }
    }
  }
  if (pfc->argnum == 5) {
    if (*(pfc->argpnt[4]) == '-' &&
        *(pfc->argpnt[4]+1) == 'b') bndflg = 1;
    else {
      sprintf(pout,"Error from %s: Invalid option flags\n",fnam);
      gaprnt(0,pout);
      return(1);
    }
  }

  /* Get the first two grids */

  if (pst->idim==dim) {          /* Fewer varying dims if user */
    pst->idim = pst->jdim;       /* averaging over varying dim */
    pst->jdim = -1;
  }
  ilin = pfi->linear[dim];
  if (pst->jdim==dim) pst->jdim = -1;
  d1 = ceil(gr1-0.001);          /* Ave limits are integers    */
  d2 = floor(gr2+0.001);
  if (bndflg) {
    d1 = floor(gr1+0.5);
    d2 = ceil(gr2-0.5);
    if (dim!=3) {
      conv = pfi->gr2ab[dim];
      wlo = conv(pfi->grvals[dim],gr1);
      whi = conv(pfi->grvals[dim],gr2);
    }
  }

  if(mfcmn.warnflg > 0) {
    if (sel == 1) {
      sprintf (pout,"Averaging.  dim = %i, start = %i, end = %i\n", dim, d1, d2);
    } else {
      sprintf (pout,"%sing.  dim = %i, start = %i, end = %i\n", fnam, dim, d1, d2);
    }
    gaprnt (2,pout);
  }

  /* Figure out weights for 1st grid */
  wt1 = 1.0;                     

  /*-----  time */
  if (dim==3) {
    gr2t (pfi->grvals[3],d1,&(pst->tmin));
    pst->tmax = pst->tmin;
    if (bndflg) {
      rd1 = d1;
      if (gr1 < rd1+0.5) wt1 = (rd1+0.5)-gr1;
      if (gr2 > rd1-0.5) wt1 = gr2 + 0.5 - rd1;
      if (wt1<0.0) wt1=0.0;
    }
  } 
  /*-----  lon,lat,lev,ens */
  else {
    conv = pfi->gr2ab[dim];
    abs = conv(pfi->grvals[dim],d1);
    alo = conv(pfi->grvals[dim],d1-0.5);
    ahi = conv(pfi->grvals[dim],d1+0.5);
    alen=fabs(ahi-alo);
    pst->dmin[dim] = abs;
    pst->dmax[dim] = abs;
    if (bndflg) {
      if (whi<wlo) {
        if (alo > wlo) alo = wlo;
        if (ahi > wlo) ahi = wlo;
        if (alo < whi) alo = whi;
        if (ahi < whi) ahi = whi;
      } else {
        if (alo < wlo) alo = wlo;
        if (ahi < wlo) ahi = wlo;
        if (alo > whi) alo = whi;
        if (ahi > whi) ahi = whi;
      }
    }
    /*-----  lat scaling */
    if (dim==1) {
      if (alo >  90.0) alo =  90.0;
      if (ahi >  90.0) ahi =  90.0;
      if (alo < -90.0) alo = -90.0;
      if (ahi < -90.0) ahi = -90.0;
      if (sel==1) {                                                   /* ave */
	wt1 = fabs(sin(ahi*3.1416/180.0)-sin(alo*3.1416/180.0));
      } else if (sel==2) {                                            /* mean */
	wt1 = fabs(ahi-alo);
      } else if (sel==3) {                                            /* sum */
	if (alen > FUZZ_SCALE) {
	  wt1=fabs(ahi-alo)/alen;
	} else {
	  wt1=0.0;
	}
      } else if (sel==4) {                                            /* sumg */
	wt1=1.0;
      }
    } 
    /* -----   lon,lev,ens scaling */
    else {
      if(sel<=2) {                                        /* ave, mean */
	wt1 = ahi - alo;
      } else if (sel==3) {                                /* sum */
	if (alen > FUZZ_SCALE) {
	  wt1=fabs(ahi-alo)/alen;
	} else {
	  wt1=0.0;
	}
      } else if (sel==4) {                                /* sumg */
	wt1=1.0;
      }
    } 
  }
  
  /* Get first grid */
  rc = gaexpr(pfc->argpnt[0],pst);     
  if (rc) return (rc);
  if (pst->type == 0) {
    gafree (pst);
    return(-1);
  }
  pgr1 = pst->result.pgr;

  d = d1 + incr;                       /* If only grid, just return */
  if (d>d2)  {
    if (sel==7 || sel==8) {
      siz = pgr1->isiz * pgr1->jsiz;
      sum  = pgr1->grid;
      sumu = pgr1->umask;
      for (i=0; i<siz; i++) {
        if (*sumu != 0) *sum = d1;
        sum++; sumu++;
      }
    }
    return (0); 
  }

  /* Figure out weights for 2nd grid */
  wt = 1.0;                    

  /*-----    time 22222222222222 */
  if (dim==3) {
    gr2t (pfi->grvals[3],d,&(pst->tmin));
    pst->tmax = pst->tmin;
    if (bndflg) {
      rd1 = d;
      if (gr1 < rd1+0.5) wt = (rd1+0.5)-gr1;
      if (gr2 > rd1-0.5) wt = gr2 + 0.5 - rd1;
      if (wt<0.0) wt=0.0;
    }
  } 
  /*----- lon,lat,lev,ens 22222222222 */
  else {
    conv = pfi->gr2ab[dim];
    abs = conv(pfi->grvals[dim],d);
    alo = conv(pfi->grvals[dim],d-0.5);
    ahi = conv(pfi->grvals[dim],d+0.5);
    alen=fabs(ahi-alo);
    pst->dmin[dim] = abs;
    pst->dmax[dim] = abs;
    if (bndflg) {
      if (whi<wlo) {
        if (alo > wlo) alo = wlo;
        if (ahi > wlo) ahi = wlo;
        if (alo < whi) alo = whi;
        if (ahi < whi) ahi = whi;
      } else {
        if (alo < wlo) alo = wlo;
        if (ahi < wlo) ahi = wlo;
        if (alo > whi) alo = whi;
        if (ahi > whi) ahi = whi;
      }
    }
    /* ---- lat scaling 2222222222222*/
    if (dim==1) {
      if (alo >  90.0) alo =  90.0;
      if (ahi >  90.0) ahi =  90.0;
      if (alo < -90.0) alo = -90.0;
      if (ahi < -90.0) ahi = -90.0;
      if(sel==1) {                                                 /* ave */
	wt = fabs(sin(ahi*3.1416/180.0)-sin(alo*3.1416/180.0));
      } else if (sel==2) {                                         /* mean */
	wt = fabs(ahi-alo);
      } else if (sel==3) {                                         /* sum */
	if(alen > FUZZ_SCALE) {
	  wt=fabs(ahi-alo)/alen;
	} else  {
	  wt=0.0;
	}
      } else if (sel==4) {                                         /* sumg */
	wt=1.0;
      }
    } 
    /* ---- lon,lev,ens  scaling 2222222222222*/
    else {
      if(sel<=2) {                                  /* ave, mean */
	wt = ahi - alo;
      } else if(sel==3) {                           /* sum */
	if(alen > FUZZ_SCALE) {
	  wt=fabs(ahi-alo)/alen;
	} else {
	  wt=0.0;
	}
      } else if(sel==4) {                           /* sumg */
	wt=1.0;
      }
    }
  }

  rc = gaexpr(pfc->argpnt[0],pst);    /* Get 2nd grid */
  if (rc) {
    gagfre(pgr1);
    return (rc);
  }
  if (pst->type==0) {
    gafree(pst);
    gagfre(pgr1);
    return (-1);
  }
  pgr2 = pst->result.pgr;


  /* We will sum into the first grid, and keep the
     count in the 2nd grid.  Set this up...        */

  siz = pgr1->isiz * pgr1->jsiz;
  sum  = pgr1->grid;
  cnt  = pgr2->grid;
  sumu = pgr1->umask;
  cntu = pgr2->umask;
  for (i=0; i<siz; i++) {
    if (sel>=5 && sel<=8) {
      if (*sumu==0 || *cntu==0) {  
        if (*cntu!=0) {
	  *sum = *cnt; 
	  *sumu = 1;
	  *cnt = d;
	}
        else if (*sumu!=0) {
	  *cnt = d1;
	  *cntu = 1;
	}
      } 
      else {
        if (sel==5 || sel==7) {
          if (*cnt < *sum) {*sum = *cnt; *cnt = d;} 
          else *cnt = d1;
        }
        if (sel==6 || sel==8) {
          if (*cnt > *sum) {*sum = *cnt; *cnt = d;}
          else *cnt = d1;
        }
      }
    } 
    else {
      if (*sumu==0) {
        if (*cntu==0) {
	  *cnt = 0.0;
	  *cntu = 1;
	}
        else {
    	  if (sel<=3) {                                          /* ave, mean sum */
	    *sum = *cnt*wt;
	    *sumu = 1;
	    *cnt = wt;
          } 
	  else if (sel==4) {                                   /* sumg */
	    *sum = *cnt;
	    *sumu = 1;
          }
        }
      } 
      else if (*cntu==0 && (sel<=3) ) {              /* ave, mean sum */
        *cnt = wt1;
	*cntu = 1;
        *sum = *sum*wt1; 
      } else {
        if (sel<=3) {
          *sum = *sum*wt1 + *cnt*wt;                            /* ave, mean sum */
        } 
	else if (sel==4) {
          *sum = *sum + *cnt;
        }
        *cnt = wt1 + wt;
	*cntu = 1;
      }
    }
    cnt++;  sum++;
    cntu++; sumu++;
  }

  /* Now sum the rest of the grids */
  d+=incr;
  rc = 0;
  for (d=d; d<=d2 && !rc; d+=incr) {
    /* Get weight for this grid */
    wt = 1.0;          
    
    /*---- time 3333333*/
    if (dim==3) {
      gr2t (pfi->grvals[3],d,&(pst->tmin));
      pst->tmax = pst->tmin;
      if (bndflg) {
	rd1 = d;
	if (gr1 < rd1+0.5) wt = (rd1+0.5)-gr1;
	if (gr2 > rd1-0.5) wt = gr2 + 0.5 - rd1;
	if (wt<0.0) wt=0.0;
      }
    } 
    /*---- lat,lon,lev,ens 3333333*/
    else {
      conv = pfi->gr2ab[dim];
      abs = conv(pfi->grvals[dim],d);
      alo = conv(pfi->grvals[dim],d-0.5);
      ahi = conv(pfi->grvals[dim],d+0.5);
      alen=fabs(ahi-alo);
      pst->dmin[dim] = abs;
      pst->dmax[dim] = abs;
      if (bndflg) {
	if (whi<wlo) {
	  if (alo > wlo) alo = wlo;
	  if (ahi > wlo) ahi = wlo;
	  if (alo < whi) alo = whi;
	  if (ahi < whi) ahi = whi;
	} else {
	  if (alo < wlo) alo = wlo;
	  if (ahi < wlo) ahi = wlo;
	  if (alo > whi) alo = whi;
	  if (ahi > whi) ahi = whi;
	}
      }
      /*---- lat 3333333*/
      if (dim==1) {
	if (alo >  90.0) alo =  90.0;
	if (ahi >  90.0) ahi =  90.0;
	if (alo < -90.0) alo = -90.0;
	if (ahi < -90.0) ahi = -90.0;
	if(sel==1) {                                                  /* ave */
	  wt = fabs(sin(ahi*3.1416/180.0)-sin(alo*3.1416/180.0));
	} else if (sel==2) {                                          /* mean */
	  wt = fabs(ahi-alo);
	} else if (sel==3) {                                          /* sum */
	  if(alen > FUZZ_SCALE) {
	    wt=fabs(ahi-alo)/alen;
	  } else  {
	    wt=0.0;
	  }
	} else if (sel==4) {                                          /* sumg */
	  wt=1.0;
	}
      } 
      /*---- lon,lev,ens 3333333*/
      else {
	if(sel<=2) {                        /* ave, mean */
	  wt = ahi - alo;
	} else if(sel==3) {                 /* sum */
	  if(alen > FUZZ_SCALE) {
	    wt=fabs(ahi-alo)/alen;
	  } else {
	    wt=0.0;
	  }
	} else if(sel==4) {                 /* sumg */
	  wt=1.0;
	}
      }
    }
    
    rc = gaexpr(pfc->argpnt[0],pst);
    if (!rc && pst->type==0) rc = -1;
    if (!rc) {
      pgr = pst->result.pgr;
      val = pgr->grid;
      cnt = pgr2->grid;
      sum = pgr1->grid;
      valu = pgr->umask;
      cntu = pgr2->umask;
      sumu = pgr1->umask;
      for (i=0; i<siz; i++) {
	if (sel>=5 && sel<=8) {
	  if (*sumu==0 || *valu==0) {  
	    if (*valu!=0) {
	      *sum = *val; 
	      *cnt = d;
	      *sumu = 1;
	      *cntu = 1;
	    }
	  } 
	  else {
	    if ((sel==5 || sel==7) && *val < *sum) {*sum = *val; *cnt = d;} 
	    if ((sel==6 || sel==8) && *val > *sum) {*sum = *val; *cnt = d;}
	  }
	} else {
	  if (*valu!=0) {
	    /* weight for ave,mean,sum  for sumg just accum */
	    if (sel<=3) {
	      *val = *val*wt;
	    }
	    if (*sumu==0) {
	      *sum = *val;
	      *sumu = 1;
	      *cnt += wt;
	    } else {
	      *sum += *val;
	      *cnt += wt;
	    }
	  }
	}
	sum++;  cnt++;  val++;
	sumu++; cntu++; valu++;
      }
      gagfre(pgr);
    }
  }
  
  if (rc) {
    if (rc==-1) gafree (pst);
    gagfre(pgr1);
    gagfre(pgr2);
    sprintf(pout,"Error from %s:  Error getting grids \n",fnam);
    gaprnt(0,pout);
    return (rc);
  } else {
    cnt = pgr2->grid;         /* Normalize if needed */
    sum = pgr1->grid;
    cntu = pgr2->umask;
    sumu = pgr1->umask;
    if (sel==1 || sel==2 || sel==7 || sel==8) {
      for (i=0; i<siz; i++) {
        if (*sumu!=0) {
          if (sel < 3 && *cnt==0.0) {
            sprintf(pout,"Error from %s:  Internal logic check 100\n",fnam);
            gaprnt(0,pout);
            return (1);
          }
          if (sel > 6 && *cntu==0) {
            sprintf(pout,"Error from %s:  Internal logic check 101\n",fnam);
            gaprnt(0,pout);
            return (1);
          }
          if (sel==1 || sel==2) {
	    *sum = *sum / *cnt;
          } else {
            *sum = *cnt;
          }
        }
        sum++;  cnt++;
        sumu++; cntu++;
      }
    }
  }

  gagfre(pgr2);

  pst->type = 1;
  pst->result.pgr = pgr1;

  return (0);

err3:
  sprintf(pout,"Error from %s: Invalid time increment argument\n",fnam);
  gaprnt(0,pout);
  return (1);
}


gaint ffgint (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgr1, *pgr;
struct gafile *pfi;
gadouble (*conv) (gadouble *, gadouble);
gadouble gr1,gr2,*sum, *val;
gadouble wt, abs, abslo, abshi;
gaint i, rc, siz, dim, d, d1, d2, dim2, ilin, bndflg, wflag;
char *ch,*sumu,*valu;

  /* Check for valid number of args       */
  if (pfc->argnum<3 || pfc->argnum>4) {
    gaprnt (0,"Error from GINT:  Too many or too few args \n");
    gaprnt (0,"                  3 or 4 arguments expected \n");
    return (1);
  }

  /* Parse the dimension expression       */
  pfi = pst->pfid;
  ch = dimprs (pfc->argpnt[1], pst, pfi, &dim, &gr1, 1, &wflag);
  if (ch==NULL) {
    gaprnt (0,"Error from GINT:  1st dimension expression invalid\n");
    return (1);
  }

  /* Now parse the 2nd dimension expression.  */
  ch = dimprs (pfc->argpnt[2], pst, pfi, &dim2, &gr2, 1, &wflag);
  if (ch==NULL || dim2!=dim || gr2<gr1) {
    gaprnt(0,"Error from GINT:  2nd dimension expression invalid\n");
    if (dim2!=dim) 
      gaprnt(0,"  Start and end points have different dimensions\n");
    if (gr2<gr1) 
      gaprnt(0,"  End grid point is less than start grid point \n");
    return (1);
  }

  /* Check for 4th argument.  Should be flags.  */
  bndflg = 0;
  if (pfc->argnum == 5) {
    if (*(pfc->argpnt[4]) == '-' &&
        *(pfc->argpnt[4]+1) == 'b') bndflg = 1;
    else {
      gaprnt (0,"Error from GINT: Invalid option flags\n");
      return(1);
    }
  }

  /* Get the first grid.             */
  if (pst->idim==dim) {          /* Fewer varying dims if user */
    pst->idim = pst->jdim;       /* integrating over varng dim */
    pst->jdim = -1;
  }
  ilin = pfi->linear[dim];
  if (pst->jdim==dim) pst->jdim = -1;
  d1 = ceil(gr1-0.001);          /* dim limits must be integer */
  d2 = floor(gr2+0.001);

  if(mfcmn.warnflg > 0) {
    sprintf (pout,"Integrating.  dim = %i, start = %i, end = %i\n", dim, d1, d2);
    gaprnt (2,pout);
  }

  wt = 1.0;                     /* Figure out weight for 1st grid */
  if (dim==3) {
    gr2t (pfi->grvals[3],d1,&(pst->tmin));
    pst->tmax = pst->tmin;
    wt = *(pfi->grvals[3]+5) + *(pfi->grvals[3]+6);
  } else {
    conv = pfi->gr2ab[dim];
    abs = conv(pfi->grvals[dim],d1);
    pst->dmin[dim] = abs;
    pst->dmax[dim] = abs;
    abslo = conv(pfi->grvals[dim],d1-0.5);
    abshi = conv(pfi->grvals[dim],d1+0.5);
    wt = abshi - abslo;
    wt = fabs(wt);
    if (dim==0 || dim==1) wt = wt*6.37E6*3.1416/180.0;
  }
  if (bndflg) wt = wt*0.5;

  rc = gaexpr(pfc->argpnt[0],pst);     /* Get first grid */
  if (rc) return (rc);
  if (pst->type == 0) {
    gafree (pst);
    return(-1);
  }
  pgr1 = pst->result.pgr;

  if (dim==0) {                        /* Adjust weights if needed  */
    if (pgr1->idim==1) cosadj(pgr1);
    else wt = wt * cos(pst->dmin[1]*3.1416/180.0);
  }

  siz = pgr1->isiz * pgr1->jsiz;       /* Apply weights to this grid */
  sum  = pgr1->grid;
  sumu = pgr1->umask;
  for (i=0; i<siz; i++) {
    if (*sumu!=0) *sum = *sum * wt;
    sum++; sumu++;
  }

  d = d1 + 1;
  if (d>d2) return(0);                 /* If only one grid, return  */

  /* Now sum the rest of the grids into the first grid */

  rc = 0;
  for (d=d; d<=d2 && !rc; d++) {
    wt = 1.0;
    if (dim==3) {
      gr2t (pfi->grvals[3],d,&(pst->tmin));
      pst->tmax = pst->tmin;
      wt = *(pfi->grvals[3]+5) + *(pfi->grvals[3]+6);
    } else {
      conv = pfi->gr2ab[dim];
      abs = conv(pfi->grvals[dim],d);
      pst->dmin[dim] = abs;
      pst->dmax[dim] = abs;
      abslo = conv(pfi->grvals[dim],d-0.5);
      abshi = conv(pfi->grvals[dim],d+0.5);
      wt = abshi - abslo;
      wt = fabs(wt);
      if (dim==0 || dim==1) wt = wt*6.37E6*3.1416/180.0;
    }
    if (d==d2 && bndflg) wt = wt*0.5;
    rc = gaexpr(pfc->argpnt[0],pst);
    if (!rc && pst->type==0) rc = -1;
    if (!rc) {
      pgr = pst->result.pgr;
      if (dim==0) {                   /* Adjust weights if needed  */
        if (pgr->idim==1) cosadj(pgr);
        else wt = wt * cos(pst->dmin[1]*3.1416/180.0);
      }
      val  = pgr->grid;
      valu = pgr->umask;
      sum  = pgr1->grid;
      sumu = pgr1->umask;
      for (i=0; i<siz; i++) {
        if (*valu!=0) {
          *val = *val*wt;
          if (*sumu==0) {
	    *sum  = *val;
	    *sumu = 1;
	  }
          else *sum += *val;
        }
        sum++;  val++;
        sumu++; valu++;
      }
      gagfre(pgr);
    }
  }

  if (rc) {
    if (rc==-1) gafree (pst);
    gagfre(pgr1);
    gaprnt (0,"Error from GINT:  Error getting grids \n");
    return (rc);
  }

  pst->type = 1;
  pst->result.pgr = pgr1;

  return (0);
}

void cosadj (struct gagrid *pgr) {
gadouble *ltvals;
gadouble (*ltconv) (gadouble *, gadouble);
gadouble lat,*gr;
gaint i,j;
char *gru;
  ltvals = pgr->ivals;
  ltconv = pgr->igrab;
  gr = pgr->grid;
  gru = pgr->umask;
  for (j=0; j<pgr->jsiz; j++) {
    for (i=0; i<pgr->isiz; i++) {
      lat  = ltconv(ltvals,(gadouble)(i+pgr->dimmin[1]))*3.1416/180.0;
      if (*gru!=0) *gr = *gr * cos(lat);
      gr++; gru++;
    }
  }
}

gaint ffhdiv (struct gafunc *pfc, struct gastat *pst) {
gaint rc,size,i,j;
struct gagrid *pgr1, *pgr2;
gadouble *result;
gadouble *p1, *p2, *p3, *p4, *p;
gadouble lat2, lat4, lat, lon1, lon3, ri, rj, temp;
gadouble *lnvals, *ltvals;
gadouble (*lnconv) (gadouble *, gadouble);
gadouble (*ltconv) (gadouble *, gadouble);
char *resultu, *p1u, *p2u, *p3u, *p4u, *pu;

  result = NULL;
  resultu = NULL;

  /* Check for user errors */
  if (pfc->argnum!=2) {
    gaprnt (0,"Error from HDIVG:  Too many or too few args \n");
    gaprnt (0,"                   Two arguments expected \n");
    return (1);
  }
  if (pst->idim!=0 || pst->jdim!=1) {
    gaprnt (0,"Error from HDIVG:  Invalid dimension environment\n");
    gaprnt (0,"  Horizontal environment (X, Y Varying) is required\n");
    return (1);
  }

  /* Get the u and v fields.  User responsible for validity. */
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gafree (pst);
    return (-1);
  }
  pgr1 = pst->result.pgr;
  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) {
    gagfre(pgr1);
    return (rc);
  }
  if (pst->type==0) {
    gafree (pst);
    gagfre(pgr1);
    return (-1);
  }
  pgr2 = pst->result.pgr;

  /* Check that an operation between these grids is valid */
  if (gagchk(pgr1,pgr2,pst->idim) ||
      gagchk(pgr1,pgr2,pst->jdim) ) {
    gaprnt (0,"Error from HDIVG:  Incompatable grids \n");
    gaprnt (0,"                   Dimension ranges unequal \n");
    goto erret;
  }

  /* Get memory for result grid and umask */
  size = pgr1->isiz * pgr1->jsiz;
  result = (gadouble *)galloc(size*sizeof(gadouble),"hdivres");
  if (result==NULL) {
    gaprnt (0,"Memory Allocation Error:  HDIVG function \n");
    goto erret;
  }
  resultu = (char *)galloc(size*sizeof(char),"hdivresu");
  if (resultu==NULL) {
    gaprnt (0,"Memory Allocation Error:  HDIVG function \n");
    goto erret;
  }

  /* Perform the divergence calculation except at grid borders */
  for (i=0; i<size; i++) *(resultu+i) = 0;

  lnvals = pgr1->ivals;
  ltvals = pgr1->jvals;
  lnconv = pgr1->igrab;
  ltconv = pgr1->jgrab;

  /*             p4
                 |
             p1--p--p3
                 |
                 p2                           */

  p = result + (pgr1->isiz + 1);
  p1 = pgr1->grid + pgr1->isiz;
  p2 = pgr2->grid + 1;
  p3 = p1 + 2;
  p4 = p2 + (2 * pgr1->isiz);

  pu = resultu + (pgr1->isiz + 1);
  p1u = pgr2->umask + pgr2->isiz;
  p2u = pgr1->umask + 1;
  p3u = p1u + 2;
  p4u = p2u + (2 * pgr1->isiz);

  for (j=(pgr1->dimmin[1]+1); j<pgr1->dimmax[1]; j++) {
    rj = (gadouble)j;
    lat  = ltconv(ltvals,rj    ) * 3.1416/180.0;
    lat2 = ltconv(ltvals,rj-1.0) * 3.1416/180.0;
    lat4 = ltconv(ltvals,rj+1.0) * 3.1416/180.0;
    for (i=(pgr1->dimmin[0]+1); i<pgr1->dimmax[0]; i++) {
      if (*p1u!=0 && 
	  *p2u!=0 &&
          *p3u!=0 && 
	  *p4u!=0 ) {
        ri = (gadouble)i;
        lon1 = lnconv(lnvals,ri-1.0) * 3.1416/180.0;
        lon3 = lnconv(lnvals,ri+1.0) * 3.1416/180.0;
        *p = (*p3 - *p1)/(lon3-lon1);
        *p = *p + (*p4*cos(lat4) - *p2*cos(lat2))/(lat4-lat2);
        temp = 6.37E6 * cos(lat);
        if (temp>1E-10) {
	  *p = *p / temp;
	  *pu = 1;
	}
        else *pu = 0;
      }
      p++; p1++; p2++; p3++; p4++;
      pu++; p1u++; p2u++; p3u++; p4u++;
    }
    p+=2; p1+=2; p2+=2; p3+=2; p4+=2;
    pu+=2; p1u+=2; p2u+=2; p3u+=2; p4u+=2;
  }
  gree(pgr1->grid,"f417");
  gree(pgr1->umask,"f418");
  gagfre(pgr2);
  pgr1->grid = result;
  pgr1->umask = resultu;
  pst->type = 1;
  pst->result.pgr = pgr1;
  return (0);

erret:
  if (result!=NULL) gree(result,"f419");
  gagfre(pgr1);
  gagfre(pgr2);
  return (1);

}

gaint ffhcrl (struct gafunc *pfc, struct gastat *pst) {
gaint rc,size,i,j;
struct gagrid *pgr1, *pgr2;
gadouble *result;
gadouble *p1, *p2, *p3, *p4, *p;
gadouble lat2, lat4, lat, lon1, lon3, ri, rj, temp;
gadouble *lnvals, *ltvals;
gadouble (*lnconv) (gadouble *, gadouble);
gadouble (*ltconv) (gadouble *, gadouble);
char *p1u, *p2u, *p3u, *p4u, *pu, *resultu;
  result = NULL;

  /* Check for user errors */

  if (pfc->argnum!=2) {
    gaprnt (0,"Error from HCURL:  Too many or too few args \n");
    gaprnt (0,"                   Two arguments expected \n");
    return (1);
  }
  if (pst->idim!=0 || pst->jdim!=1) {
    gaprnt (0,"Error from HCURL:  Invalid dimension environment\n");
    gaprnt (0,"  Horizontal environment (X, Y Varying) is required\n");
    return (1);
  }

  /* Get the u and v fields.  User responsible for validity. */

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gafree (pst);
    return(-1);
  }
  pgr1 = pst->result.pgr;
  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) {
    gagfre(pgr1);
    return (rc);
  }
  if (pst->type==0) {
    gafree (pst);
    gagfre(pgr1);
    return (-1);
  }
  pgr2 = pst->result.pgr;

  /* Check that an operation between these grids is valid */

  if (gagchk(pgr1,pgr2,pst->idim) ||
      gagchk(pgr1,pgr2,pst->jdim) ) {
    gaprnt (0,"Error from HCURL:  Incompatable grids \n");
    gaprnt (0,"                   Dimension ranges unequal \n");
    goto erret;
  }

  /* Get memory for result grid and umask */
  size = pgr1->isiz * pgr1->jsiz;
  result = (gadouble *)galloc(size*sizeof(gadouble),"hcurlres");
  if (result==NULL) {
    gaprnt (0,"Memory Allocation Error:  HCURL function\n");
    goto erret;
  }
  resultu = (char *)galloc(size*sizeof(char),"hcurlresu");
  if (resultu==NULL) {
    gaprnt (0,"Memory Allocation Error:  HCURL function\n");
    goto erret;
  }

  /* Perform the vorticity calculation except at grid borders */
  for (i=0; i<size; i++) *(resultu+i) = 0;

  lnvals = pgr1->ivals;
  ltvals = pgr1->jvals;
  lnconv = pgr1->igrab;
  ltconv = pgr1->jgrab;

  /*             p4
                 |
             p1--p--p3
                 |
                 p2                           */

  p  = result  + (pgr1->isiz + 1);
  p1  = pgr2->grid + pgr2->isiz;
  p2  = pgr1->grid + 1;
  p3  = p1 + 2;
  p4  = p2  + (2 * pgr1->isiz);

  pu = resultu + (pgr1->isiz + 1);
  p1u = pgr2->umask + pgr2->isiz;
  p2u = pgr1->umask + 1;
  p3u = p1u + 2;
  p4u = p2u + (2 * pgr1->isiz);

  for (j=(pgr1->dimmin[1]+1); j<pgr1->dimmax[1]; j++) {
    rj = (gadouble)j;
    lat  = ltconv(ltvals,rj    ) * 3.1416/180.0;
    lat2 = ltconv(ltvals,rj-1.0) * 3.1416/180.0;
    lat4 = ltconv(ltvals,rj+1.0) * 3.1416/180.0;
    for (i=(pgr1->dimmin[0]+1); i<pgr1->dimmax[0]; i++) {
      if (*p1u!=0 && 
	  *p2u!=0 &&
          *p3u!=0 && 
	  *p4u!=0 ) {
        ri = (gadouble)i;
        lon1 = lnconv(lnvals,ri-1.0) * 3.1416/180.0;
        lon3 = lnconv(lnvals,ri+1.0) * 3.1416/180.0;
        *p = (*p3 - *p1)/(lon3-lon1);
        *p = *p - (*p4*cos(lat4) - *p2*cos(lat2))/(lat4-lat2);
        temp = 6.37E6 * cos(lat);
        if (temp>1E-10) {
	  *p = *p / temp;
	  *pu = 1;
	}
        else {
	  *pu = 0;
	}
      }
      p++;  p1++;  p2++;  p3++;  p4++;
      pu++; p1u++; p2u++; p3u++; p4u++;
    }
    p+=2;  p1+=2;  p2+=2;  p3+=2;  p4+=2;
    pu+=2; p1u+=2; p2u+=2; p3u+=2; p4u+=2;
  }
  gree(pgr1->grid,"f420");
  gree(pgr1->umask,"f421");
  gagfre (pgr2);
  pgr1->grid = result;
  pgr1->umask = resultu;
  pst->type = 1;
  pst->result.pgr = pgr1;
  return (0);

erret:
  if (result!=NULL) gree(result,"f422");
  gagfre(pgr1);
  gagfre(pgr2);
  return (1);

}

gaint fftv2q (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
  rc = fftv2 (pfc, pst, 0);
  return (rc);
}

gaint fftv2t (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
  rc = fftv2 (pfc, pst, 1);
  return (rc);
}

gaint fftv2 (struct gafunc *pfc, struct gastat *pst, gaint tflag) {
struct gagrid *pgrtv, *pgrrh;
gadouble *lvvals;
gadouble (*lvconv) (gadouble *, gadouble);
gaint i,j, rc, errcnt;
gadouble *tv, *rh, t, q, p;
char *tvu, *rhu;

  if (pfc->argnum!=2) {
    if (tflag) {
      gaprnt (0,"Error from TVRH2T:  Too many or too few args \n");
    } else {
      gaprnt (0,"Error from TVRH2Q:  Too many or too few args \n");
    }
    gaprnt (0,"                    Two arguments expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type == 0) {
    gafree (pst);
    return(-1);
  }
  pgrtv = pst->result.pgr;

  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) return (rc);
  if (pst->type == 0) {
    gagfre(pgrtv);
    gafree (pst);
    return(-1);
  }
  pgrrh = pst->result.pgr;

  if ((pgrrh->idim!=pgrtv->idim) || 
      (pgrrh->jdim!=pgrtv->jdim) || 
      (pgrrh->idim>-1 && gagchk(pgrrh,pgrtv,pgrrh->idim)) || 
      (pgrrh->jdim>-1 && gagchk(pgrrh,pgrtv,pgrrh->jdim)) ) {
    gaprnt (0,"Error in TVRH2Q: Grids don't have same scaling");
    gagfre (pgrtv);
    gagfre (pgrrh);
    return (1);
  }

  errcnt = 0;
  if (pgrrh->idim == 2) {
    lvconv = pgrrh->igrab;
    lvvals = pgrrh->ivals;
    tv = pgrtv->grid;
    rh = pgrrh->grid;
    tvu = pgrtv->umask;
    rhu = pgrrh->umask;
    for (j=0; j<pgrrh->jsiz; j++) {
      for (i=0; i<pgrrh->isiz; i++) {
        if (*rhu==0 || *tvu==0) {
          *rhu = 0;
        } else {
          p = lvconv(lvvals, (gadouble)(i+pgrrh->dimmin[2]));
          rc = tvrh2q (p, *tv, *rh, &q, &t);
          if (rc) {
            *rhu = 0;
            errcnt++;
          } else {
            if (tflag) {
	      *rh = t;
	      *rhu = 1;
	    }
            else {
	      *rh = q;
	      *rhu = 1;
	    }
          }
        }
        rh++;  tv++;
        rhu++; tvu++;
      }
    }
  } else if (pgrrh->jdim == 2) {
    lvconv = pgrrh->jgrab;
    lvvals = pgrrh->jvals;
    tv = pgrtv->grid;
    rh = pgrrh->grid;
    tvu = pgrtv->umask;
    rhu = pgrrh->umask;
    for (j=0; j<pgrrh->jsiz; j++) {
      p = lvconv(lvvals, (gadouble)(j+pgrrh->dimmin[2]));
      for (i=0; i<pgrrh->isiz; i++) {
        if (*rhu==0 || *tvu==0) {
          *rhu = 0;
        } else {
          rc = tvrh2q (p, *tv, *rh, &q, &t);
          if (rc) {
            *rhu = 0;
            errcnt++;
          } else {
            if (tflag) {
	      *rh = t;
	      *rhu = 1;
	    }
            else {
	      *rh = q;
	      *rhu = 1;
	    }
          }
        }
        rh++; tv++;
        rhu++; tvu++;
      }
    }
  } else {
    p = pst->dmin[2];
    sprintf (pout," Using fixed pressure level %g mb\n",p);
    if (tflag) {
      gaprnt (2,"Notice from TVRH2T:");
    } else {
      gaprnt (2,"Notice from TVRH2Q:");
    }
    gaprnt (1,pout);
    tv = pgrtv->grid;
    rh = pgrrh->grid;
    tvu = pgrtv->umask;
    rhu = pgrrh->umask;
    for (j=0; j<pgrrh->jsiz; j++) {
      for (i=0; i<pgrrh->isiz; i++) {
        if (*rhu==0 || *tvu==0) {
          *rhu = 0;
        } else {
          rc = tvrh2q (p, *tv, *rh, &q, &t);
          if (rc) {
            *rhu = 0;
            errcnt++;
          } else {
            if (tflag) {
	      *rh = t;
	      *rhu = 1;
	    }
            else {
	      *rh = q;
	      *rhu = 1;
	    }
          }
        }
        rh++; tv++;
        rhu++; tvu++;
      }
    }
  }

  if (errcnt) {
    sprintf (pout," Convergence failed for %i grid points\n",errcnt);
    if (tflag) {
      gaprnt (1,"Warning from TVRH2T:");
    } else {
      gaprnt (1,"Warning from TVRH2Q:");
    }
    gaprnt (1,pout);
  }
  gagfre (pgrtv);
  return (0);
}

/* Routine to convert tv and rh to t and q.  FORTRAN version
   provided by J. Kinter.  Converted to C by B. Doty.    */

gaint tvrh2q (gadouble p, gadouble tv, gadouble rh, gadouble *qret, gadouble *tret) {
gadouble eps,a,b,c,t,q,al10,tc,esat,desdt,qr,f,denom,dfdt,fn,qn,tn;
gaint i;

  eps = 0.622;
  a = 0.7854;
  b = 0.03477;
  c = 0.00412;

  /* Convert rh to fraction.  Convert pressure to pascals. */

  rh = rh * 0.01;
  p = p*100.0;

  /* Set first guess for t and q */

  t = tv;
  q = 0.01 * rh * eps;

  /* Iterate to convergence */

  al10 = log(10.0);
  for (i=0; i<25; i++) {

    /* Set saturation vapor pressure (compute from smisthonian tables)*/

    tc = t - 273.16;
    esat = 100.0 * exp(al10*(a+b*tc)/(1.0+c*tc));
    desdt = al10 * esat * (b-a*c)/((1.0+c*tc)*(1.0+c*tc));
    qr = eps*rh*esat/(p-(1.0-eps)*esat);
    f = q-qr;

    /* Compute derivative of q wrt q given fixed virtual temp and
       rh (constant pressure) */

    denom = p-(1.0-eps)*esat;
    denom = denom * denom;
    dfdt = eps*rh*((p-(1.0-eps)*esat)*desdt+esat*(1.0-eps)*desdt);
    dfdt = 1.0-dfdt/denom;

    /* Newton's method */

    fn = f-f/dfdt;
    qn = fn+qr;
    tn = eps*tv*((1.0-qn)/(eps*(1.0-qn)+qn*(1.0-eps)));

    /* Test for convergence */

    if (fabs((tn-t)/t) <= 1.0E-6) break;
    q = qn;
    t = tn;
  }

  /* Print results */

  if (i==25) return(1);
  *qret = qn;
  *tret = tn;
  return(0);
}

gaint ffvint (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgrb, *pgr, *pgrv;
struct gafile *pfi;
gadouble *ps, *var, *res;
gadouble *lvvals;
gadouble (*lvconv) (gadouble *, gadouble);
gadouble top,clev,ulev,blev,ulevi,blevi,ulevt,blevt,kgm;
gaint rc, size, i, j, lvt;
char *psu, *varu, *resu;

  if (pfc->argnum!=3) {
    gaprnt (0,"Error from VINT:  Too many or too few args \n");
    gaprnt (0,"                  Three arguments expected \n");
    return (1);
  }

  /* Get top pressure level.  It is a character value in 3rd arg */
  if (getdbl(pfc->argpnt[2],&top)==NULL) {
    gaprnt (0,"Error from VINT:  3rd argument invalid. \n");
    return (1);
  }

  /* Get the range of levels from the default file.  Set the
     level in the status block to the first level.  */

  pfi = pst->pfid;
  lvt = pfi->dnum[2];
  if (lvt<3) {
    gaprnt (0,"Error from VINT:  Too few levels in default file \n");
    return (1);
  }
  lvconv = pfi->gr2ab[2];
  lvvals = pfi->grvals[2];
  clev = lvconv(lvvals, 1.0);
  ulev = lvconv(lvvals, 2.0);
  ulev = clev + ((ulev-clev)/2.0);
  pst->dmin[2] = clev;
  pst->dmax[2] = clev;
  if (pst->idim==2) {
    pst->idim = pst->jdim;
    pst->jdim = -1;
  }
  if (pst->jdim==2) pst->jdim = -1;

  /* Get the surface pressure field (1st arg).  User is responsible
     for valid argument.  Then get the lowest level of the
     field to integrate.  */

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (1);
  if (pst->type==0) {
    gafree (pst);
    return (-1);
  }
  pgrb = pst->result.pgr;

  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) {
    gagfre (pgrb);
    return (1);
  }
  if (pst->type==0) {
    gafree (pst);
    gagfre (pgrb);
    return (-1);
  }
  pgr = pst->result.pgr;

  /* Check that the two grids are equivalent.  */
  if (pgrb->isiz!=pgr->isiz || pgrb->jsiz!=pgr->jsiz) {
    gaprnt (0,"Error from VINT:  Incompatible grids. \n");
    goto erret;
  }

  /* Apply appropriate mass weight (kg/m**2) to first level.
     It is assumed the vertical coordinate system is mb.    */
  size = pgr->isiz * pgr->jsiz;
  kgm = 100.0/9.8;
  ps  = pgrb->grid; 
  psu = pgrb->umask;
  res  = pgr->grid; 
  resu = pgr->umask;
  for (i=0; i<size; i++) {
    if (*psu==0 || *resu==0) {
      *resu = 0;
    }
    else if (*ps < clev) {
      *resu = 0;
    }
    else {
      *res = *res * kgm * (*ps - ulev);
      *resu = 1;
    }
    ps++; psu++; res++; resu++;
  }

  /* Go through the intermediate levels and apply mass weight. */
  for (i=2; i<lvt; i++) {
    clev = lvconv(lvvals, (gadouble)i);
    if (clev<top) break;
    ulev = lvconv(lvvals, (gadouble)(i+1));
    ulevi = clev + ((ulev-clev)/2.0);
    blev = lvconv(lvvals, (gadouble)(i-1));
    blevi = clev + ((blev-clev)/2.0);
    pst->dmin[2] = clev;
    pst->dmax[2] = clev;
    rc = gaexpr(pfc->argpnt[1],pst);
    if (rc) goto erret;
    if (pst->type==0) {
      rc = -1;
      gafree (pst);
      goto erret;
    }
    pgrv = pst->result.pgr;
    ps  = pgrb->grid;
    res = pgr->grid;
    var = pgrv->grid;
    psu  = pgrb->umask;
    resu = pgr->umask;
    varu = pgrv->umask;
    for (j=0; j<size; j++) {
      if ((*psu!=0) && (*varu!=0) && (*ps>=clev)) {
        ulevt = ulevi;
        if (top>ulev) ulevt = top;
        blevt = blevi;
        if (*ps<blev) blevt = *ps;
        if (*resu==0) {
	  *res = *var * kgm * (blevt - ulevt);
	  *resu = 1;
	}
        else {
	  *res = *res + (*var * kgm * (blevt - ulevt) );
	  *resu = 1;
	}
      }
      ps++;  res++;  var++;
      psu++; resu++; varu++;
    }
    gafree (pst);
  }

  /* Do top, and last, level */

  clev = lvconv(lvvals, (gadouble)i);
  if (top<=clev) {
    blev = lvconv(lvvals, (gadouble)(i-1));
    blevi = clev + ((blev-clev)/2.0);
    pst->dmin[2] = clev;
    pst->dmax[2] = clev;
    rc = gaexpr(pfc->argpnt[1],pst);
    if (rc) goto erret;
    if (pst->type==0) {
      rc = -1;
      gafree (pst);
      goto erret;
    }
    pgrv = pst->result.pgr;
    ps  = pgrb->grid;
    res = pgr->grid;
    var = pgrv->grid;
    psu  = pgrb->umask;
    resu = pgr->umask;
    varu = pgrv->umask;
    for (i=0; i<size; i++) {
      if (*psu!=0 && *varu!=0) {
        blevt = blevi;
        if (*ps<blev) blevt = *ps;
        if (*resu==0) {
	  *res = *var * kgm * (blevt - top);
	  *resu = 1;
	}
        else {
	  *res = *res + (*var * kgm * (blevt - top) );
	  *resu = 1;
	}
      }
      ps++; res++; var++;
      psu++; resu++; varu++;
    }
  }
  gafree (pst);

  /* Release storage and return */

  rc = 0;
  pst->type = 1;
  pst->result.pgr = pgr;
  gagfre (pgrb);
  return (0);

  /* Error return */

erret:

  gagfre (pgrb);
  gagfre (pgr);
  return (rc);
}

gaint fftlp (struct gafunc *pfc, struct gastat *pst) {
struct gafile *pfi;
struct gagrid *pgr, *res;
gaint size, rc, t1, t2, i, cont;
gadouble gr1, gr2,*in, *out;
char *inu, *outu;

  /* Check for valid number of args       */

  if (pfc->argnum != 1 ) {
    gaprnt (0,"Error from TLOOP:  Too many or too few args \n");
    gaprnt (0,"                   1 argument expected \n");
    return (1);
  }

  /* If t is non-varying, treat this as a no-op.  */

  if (pst->idim!=3 && pst->jdim!=3) {
    rc = gaexpr(pfc->argpnt[0],pst);
    return (rc);
  }

  /* Get start and end times in terms of grid space */

  pfi = pst->pfid;
  gr1 = t2gr(pfi->abvals[3], &pst->tmin);
  gr2 = t2gr(pfi->abvals[3], &pst->tmax);
  gr1 = ceil(gr1-0.001);
  gr2 = floor(gr2+0.001);
  t1 = (gaint)gr1;
  t2 = (gaint)gr2;
  if (t2<t1) t2 = t1;

  /* Get 1st grid. */

  gr2t(pfi->grvals[3], gr1, &pst->tmin);
  if (pst->idim==3) {
    pst->idim = pst->jdim;
    pst->jdim = -1;
  }
  if (pst->jdim==3) pst->jdim = -1;
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gafree (pst);
    return (-1);
  }
  pgr = pst->result.pgr;

  /* Check validity of 1st grid.  It should be a 1-D or a 0-D grid,
     and it should not have a time-varying dimension.  */

  if (pgr->jdim!=-1 || pgr->idim==3) {
    gaprnt (0,"Error from TLOOP: Internal logic check 36\n");
    goto err1;
  }

  /* Create output grid */

  size = sizeof(struct gagrid);
  res = (struct gagrid *)galloc(size,"tloopres");
  if (res==NULL) {
    gaprnt (0,"Memory Allocation Error:  TLOOP function\n");
    goto err1;
  }
  size = 1+t2-t1;
  size = size * pgr->isiz;
  if (size>1) {
    res->grid = (gadouble *)galloc(size*sizeof(gadouble),"tloopgr");
    if (res->grid==NULL) {
      gaprnt (0,"Memory Allocation Error:  TLOOP function\n");
      gree(res,"f423");
      goto err1;
    }
    res->umask = (char *)galloc(size*sizeof(char),"tloopgru");
    if (res->umask==NULL) {
      gaprnt (0,"Memory Allocation Error:  TLOOP function\n");
      gree(res->grid,"f424");
      gree(res,"f425");
      goto err1;
    }
  } else {
    res->grid = &(res->rmin);
    res->umask = &(res->umin);
  }

  res->alocf = 0;
  res->pfile = NULL;
  res->undef = pgr->undef;
  res->pvar  = NULL;
  res->exprsn = NULL;
  for (i=0;i<5;i++) {
    res->dimmin[i] = 0;
    res->dimmax[i] = 0;
  }
  res->dimmin[3] = t1;
  res->dimmax[3] = t2;
  res->jwrld = 0;
  if (pgr->isiz>1) {
    res->dimmin[pgr->idim] = pgr->dimmin[pgr->idim];
    res->dimmax[pgr->idim] = pgr->dimmax[pgr->idim];
    res->idim = pgr->idim;
    res->iwrld = pgr->iwrld;
    res->isiz = pgr->isiz;
    res->igrab = pgr->igrab;
    res->ilinr = pgr->ilinr;
    res->ivals = pgr->ivals;
    if (t1==t2) {
      res->jdim = -1;
      res->jsiz = 1;
    } else {
      res->jdim = 3;
      res->jsiz = 1+t2-t1;
      res->jvals = pfi->grvals[3];
      res->jgrab = NULL;
      res->jlinr = 1;
    }
  } else {
    res->jdim = -1;
    res->jsiz = 1;
    if (t1==t2) {
      res->idim = -1;
      res->isiz = 1;
    } else {
      res->idim = 3;
      res->isiz = 1+t2-t1;
      res->ivals = pfi->grvals[3];
      res->igrab = NULL;
      res->ilinr = 1;
    }
  }

  /* Loop and fill output grid.  */

  cont = 1;
  out = res->grid;
  outu = res->umask;
  while (cont) {
    in = pgr->grid;
    inu = pgr->umask;
    for (i=0; i<pgr->isiz; i++) {
      if (*inu==0) {
	*outu=0;
      }
      else {
	*out = *in;
	*outu = 1;
      }
      in++; inu++; out++; outu++;
    }
    gagfre(pgr);
    t1++;
    if (t1<=t2) {
      gr2t(pfi->abvals[3], (gadouble)t1, &pst->tmin);
      pst->tmax = pst->tmin;
      rc = gaexpr(pfc->argpnt[0],pst);
      if (rc) goto err2;
      pgr = pst->result.pgr;
    } else cont = 0;
  }
  pst->result.pgr = res;
  return (0);

err1:
  gagfre (pgr);
  return (1);

err2:
  gagfre(res);
  return(1);
}

gaint ffmask (struct gafunc *pfc, struct gastat *pst) {
struct gastat pst2;
struct gastn *stn;
struct garpt *rpt;
char *ch,c1,c2;
gaint rc,cnt,flag,i;

  if (pfc->argnum!=2) {
    gaprnt (0,"Error from MASKOUT:  Too many or too few args \n");
    gaprnt (0,"                     Two arguments expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type!=0 || *(pfc->argpnt[1])!='\'') {

    pst2 = *pst;
    rc = gaexpr(pfc->argpnt[1],&pst2);
    if (rc) {
      gafree (pst);
      return (rc);
    }

    rc = gafopr (pst, &pst2, 13);
    if (rc) {
      gafree (pst);
      gafree (&pst2);
    }

    /* Handle maskout of stn data by stid */

  } else {
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      ch = pfc->argpnt[1]+1;
      flag = 1; cnt = 0;
      while (*ch!='\'' && flag && cnt<8) {
        if (*ch!='?') {
          c1 = *ch;
          c2 = rpt->stid[cnt];
          i = c1;
          if (i>64 && i<91) {i+=32; c1=i;}
          i = c2;
          if (i>64 && i<91) {i+=32; c2=i;}
          if (c1 != c2) flag = 0;
        }
        ch++; cnt++;
      }
      if (flag==0) rpt->umask = 0;
      rpt=rpt->rpt;
    }
  }
  return (rc);
}

/* Given a grid and a set of stations, interpolate to the
   stations and return the set of stations.                       */

gaint ffg2s  (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble (*iconv) (gadouble *, gadouble);
gadouble (*jconv) (gadouble *, gadouble);
gadouble *ivars, *jvars, *p1, *p2, *p3, *p4;
gadouble gi,gj,w1,w2,lon,lat,lnmin,lnmax,lnscl,ltmin,ltmax,ltscl;
gaint rc,ig,jg;
char *p1u, *p2u, *p3u, *p4u;

  if (pfc->argnum<2 || pfc->argnum>3) {
    gaprnt (0,"Error from GR2STN:  Too many or too few args \n");
    gaprnt (0,"                    Two arguments expected \n");
    return (1);
  }

  /* If we are doing the form of gr2stn that involves
     interpolating to a profile or time series, branch
     to a different routine */

  if ( (pst->idim == -1 || pst->idim>1) && pst->jdim == -1) {
    rc = ffg2s2 (pfc,pst);
    return (rc);
  }
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gaprnt (0,"Error from GR2STN: 1st argument must be a grid\n");
    gafree (pst);
    return (1);
  }
  pgr = pst->result.pgr;
  if (pgr->idim!=0 || pgr->jdim!=1) {
    gaprnt (0,"Error from GR2STN: Grid must vary in Z or T\n");
    gafree (pst);
    return (1);
  }
  if (!pgr->ilinr || !pgr->jlinr) {
    gaprnt (0,"Error from GR2STN: Grid must have linear scaling\n");
    gafree (pst);
    return (1);
  }
  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) {
    gagfre (pgr);
    return (rc);
  }
  if (pst->type!=0) {
    gaprnt (0,"Error from GR2STN: 2nd argument must be stns\n");
    gafree (pst);
    gagfre (pgr);
    return (1);
  }
  stn = pst->result.stn;

  /* Set up scaling for converting lon-lat to grid units */
  iconv = pgr->igrab;
  ivars = pgr->ivals;
  jconv = pgr->jgrab;
  jvars = pgr->jvals;
  lnmin = iconv(ivars,(gadouble)pgr->dimmin[0]);
  lnmax = iconv(ivars,(gadouble)pgr->dimmax[0]);
  ltmin = iconv(jvars,(gadouble)pgr->dimmin[1]);
  ltmax = iconv(jvars,(gadouble)pgr->dimmax[1]);
  lnscl = (lnmax-lnmin)/((gadouble)pgr->isiz-1);
  ltscl = (ltmax-ltmin)/((gadouble)pgr->jsiz-1);

  /* Now loop through each stn report, convert stn lat/lon to grid
     units, then interpolate from grid to stn */

  rpt = stn->rpt;
  while (rpt!=NULL) {
    lon = rpt->lon;
    lat = rpt->lat;
    if (lon<lnmin) lon+=360.0;
    else if (lon>lnmax) lon-=360.0;
    if (lon<lnmin || lon>=lnmax || lat<ltmin || lat>=ltmax) {
      rpt->umask = 0;
    } else {
      gi = (lon-lnmin)/lnscl;
      gj = (lat-ltmin)/ltscl;
      ig = (gaint)gi; 
      jg = (gaint)gj;
      p1 = pgr->grid + jg*pgr->isiz + ig;
      p1u = pgr->umask + jg*pgr->isiz + ig;
      p2 = p1+1;
      p2u = p1u+1;
      p3 = p2 + pgr->isiz;
      p3u = p2u + pgr->isiz;
      p4 = p1 + pgr->isiz;
      p4u = p1u + pgr->isiz;
      if (*p1u==0 || *p2u==0 || *p3u==0 || *p4u==0) {
        rpt->umask = 0;
      } else {
        gi = gi - (gadouble)ig;
        gj = gj - (gadouble)jg;
/*
 Weighted by distance or use bilinear?  bilinear looks to be
 more valid to me......
        w1 = 1.0 - hypot(gi,gj);
        w2 = 1.0 - hypot(1.0-gi,gj);
        w3 = 1.0 - hypot(1.0-gi,1.0-gj);
        w4 = 1.0 - hypot(gi,1.0-gj);
        if (w1<0.0) w1=0.0;
        if (w2<0.0) w2=0.0;
        if (w3<0.0) w3=0.0;
        if (w4<0.0) w4=0.0;
        rpt->val = *p1*w1 + *p2*w2 + *p3*w3 + *p4*w4;
        rpt->val = rpt->val / (w1+w2+w3+w4);
*/
        w1 = *p1 + (*p2 - *p1)*gi;
        w2 = *p4 + (*p3 - *p4)*gi;
        rpt->val = w1 + (w2-w1)*gj;
      }
    }
    rpt=rpt->rpt;
  }
  gagfre (pgr);
  pst->type = 0;
  pst->result.stn=stn;
  return(0);
}

/* gr2stn where we interpolate to a lat-lon for 
   a profile or time series */

gaint ffg2s2  (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgr, *pgr2;
struct gastn *stn, *stn2;
struct garpt *rpt;
gadouble (*iconv) (gadouble *, gadouble);
gadouble (*icnv) (gadouble *, gadouble);
gadouble (*jcnv) (gadouble *, gadouble);
gadouble *ivars, lon, lat, *grid, lev=0, val=0;
gadouble *p1, *p2, *p3, *p4;
gadouble gi,gj,w1,w2;
gaint i,rc,gr1,gr2,gr,ig,jg;
char *p1u, *p2u, *p3u, *p4u, umask;

  /* Get lat-lon to interpolate to.  This is either provided
     as two string arguments, or as a single stn-data argument. */

  iconv = NULL;
  stn2 = NULL;
  ivars=0;
  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    stn2 = pst->result.stn;
    rpt = stn2->rpt;
    lat = rpt->lat;
    lon = rpt->lon;
  } 
  else {
    pgr = pst->result.pgr;
    if (pgr->idim!=-1 || pgr->jdim!=-1) {
      gaprnt (0,"Error in GR2STN:  2nd arg invalid\n");
      return (1);
    }
    lon = pgr->rmin;
    gafree(pst);
    if (pfc->argnum<3) {
      gaprnt(0,"Error in GR2STN:  3rd Argument Required\n");
      return (1);
    }
    if (getdbl(pfc->argpnt[2],&lat)==NULL) {
      gaprnt (0,"Error from GR2STN:  3rd argument invalid. \n");
      return (1);
    }
  }
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gaprnt (0,"Error from GR2STN: 1st argument must be a grid\n");
    gafree (pst);
    return (1);
  }
  pgr = pst->result.pgr;

  if (pgr->idim==3) {
    ivars = pgr->ivals;
    gr1 = pgr->dimmin[3];
    gr2 = pgr->dimmax[3];
  } else if (pgr->idim==2) {
    iconv = pgr->igrab;
    ivars = pgr->ivals;
    gr1 = pgr->dimmin[2];
    gr2 = pgr->dimmax[2];
  } else if (pgr->idim== -1) {
    gr1 = 1;  gr2 = 1;
  } else if (pgr->idim==4) {
    gaprnt(0,"Error from GR2STN: Only the Z or T dimension may vary\n");
    gafree (pst);
    return (1);
  } else {
    gaprnt (0,"Error from GR2STN:  Logic Error 4\n");
    gafree (pst);
    return (1);
  }

  /* Set up stn structure for the returned data */

  stn = (struct gastn *)galloc(sizeof(struct gastn),"gr2stn");
  if (stn==NULL) {
    gaprnt (0,"Memory Allocation Error:  Station Request Block \n");
    gagfre (pgr);
    return (1);
  }
  stn->rpt = NULL;
  stn->rnum = 0;
  stn->idim = pgr->idim;
  stn->jdim = pgr->jdim;
  stn->undef = pgr->undef;
  stn->pvar = NULL;
  stn->dmin[0] = lon;  stn->dmax[0] = lon;
  stn->dmin[1] = lat;  stn->dmax[1] = lat;
  stn->rflag = 0;
  stn->radius = 1.0;
  stn->sflag = 1;
  if (stn2) {
    for (i=0; i<8; i++) stn->stid[i] = stn2->stid[i];
  } else {
    for (i=0; i<8; i++) stn->stid[i] = 'x';
  }
  stn->tvals = (gadouble *)galloc(sizeof(gadouble)*8,"gr2stnt");
  if (stn->tvals==NULL) {
    gaprnt (0,"Memory Allocation Error:  Station Request Block \n");
    gree(stn,"f426");
    gagfre (pgr);
    return (1);
  }
  stn->jdim = -1;
  if (pgr->idim==3) {
    for (i=0; i<8; i++) *(stn->tvals+i) = *(ivars+i);
    stn->tmin = gr1; stn->tmax = gr2;
    stn->dmin[2] = pst->dmin[2];
    stn->dmax[2] = pst->dmin[2];
    stn->idim = 3;
  } else if (pgr->idim== -1) {
    stn->dmin[2] = pst->dmin[2];
    stn->dmax[2] = pst->dmin[2];
    stn->tmin = 1; stn->tmax = 1;
    *(stn->tvals) = pst->tmin.yr;
    *(stn->tvals+1) = pst->tmin.mo;
    *(stn->tvals+2) = pst->tmin.dy;
    *(stn->tvals+3) = pst->tmin.hr;
    *(stn->tvals+4) = pst->tmin.mn;
    *(stn->tvals+5) = 0.0;
    *(stn->tvals+6) = 1.0;
    *(stn->tvals+7) = -999.9;
  } else {
    stn->dmin[2] = gr1;  
    stn->dmax[2] = gr2;
    stn->tmin = 1; 
    stn->tmax = 1;
    *(stn->tvals) = pst->tmin.yr;
    *(stn->tvals+1) = pst->tmin.mo;
    *(stn->tvals+2) = pst->tmin.dy;
    *(stn->tvals+3) = pst->tmin.hr;
    *(stn->tvals+4) = pst->tmin.mn;
    *(stn->tvals+5) = 0.0;
    *(stn->tvals+6) = 1.0;
    *(stn->tvals+7) = -999.9;
  }

  pst->idim = 0;
  pst->jdim = 1;
  pst->dmin[0] = lon;
  pst->dmax[0] = lon + 0.1;
  pst->dmin[1] = lat;
  pst->dmax[1] = lat + 0.1;
  for (gr=gr1; gr<=gr2; gr++) {
    if (pgr->idim==2) {
      lev = iconv(ivars,(gadouble)gr);
      pst->dmin[2] = lev;
      pst->dmax[2] = lev;
    } else if (pgr->idim==3) {
      gr2t (ivars, (gadouble)gr, &(pst->tmin));
      pst->tmax = pst->tmin;
    }
    rc = gaexpr(pfc->argpnt[0],pst);
    if (rc) {
      gagfre(pgr);
      return (rc);
    }
    pgr2 = pst->result.pgr;
    grid = pgr2->grid;
    icnv = pgr2->iabgr;
    jcnv = pgr2->jabgr;
    gi = icnv(pgr2->iavals,lon) - (gadouble)pgr2->dimmin[0];
    gj = jcnv(pgr2->javals,lat) - (gadouble)pgr2->dimmin[1];
    ig = (gaint)gi; 
    jg = (gaint)gj;
    p1 = pgr2->grid + jg*pgr2->isiz + ig;
    p1u = pgr2->umask + jg*pgr2->isiz + ig;
    p2 = p1+1;
    p2u = p1u+1;
    p3 = p2 + pgr2->isiz;
    p3u = p2u + pgr2->isiz;
    p4 = p1 + pgr2->isiz;
    p4u = p1u + pgr2->isiz;
    if (*p1u==0 || *p2u==0 || *p3u==0 || *p4u==0) {
      umask = 0;
    } else {
      gi = gi - (gadouble)ig;
      gj = gj - (gadouble)jg;
      w1 = *p1 + (*p2 - *p1)*gi;
      w2 = *p4 + (*p3 - *p4)*gi;
      val = w1 + (w2-w1)*gj;
      umask = 1;
    }
    rpt = gaarpt (stn);
    if (rpt==NULL) {
      gaprnt (0,"Memory Allocation Error:  Station Block \n");
      gagfre(pgr);  gafree(pst); gasfre(stn);
      return (1);
    }
    rpt->lat = lat;
    rpt->lon = lon;
    if (pgr->idim==2) { rpt->lev = lev; rpt->tim = 1; }
    else { rpt->lev = stn->dmin[2]; rpt->tim = gr; }
    if (umask==1) {
      rpt->umask=1;
      rpt->val = val;
    }
    else {
      rpt->umask=0;
    }  
    for (i=0; i<8; i++) *(rpt->stid+i) = *(stn->stid+i);
    stn->rnum++;
    gafree(pst);
  }

  pst->result.stn = stn;
  pst->type = 0;
  pst->idim = stn->idim;
  pst->jdim = -1;
  return (0);
}

gaint ffclgr (struct gafunc *pfc, struct gastat *pst) {
  struct gaclct *clct, *clct0;
  struct gastn *stn;
  struct garpt *rpt;
  struct gagrid *pgr;
  gadouble *levs, lev, vlo, vhi, uu=0, *gr;
  gadouble *iv,*jv,diff,lld,lhd,llo,lhi,xdiff;
  gaint i,j,cnt,lcnt=0,scnt,flag,clnm,dim,lflg,ucnt;
  gaint verb,noundef;
  char *gru;

  lflg = 1;
  ucnt = 10;

  /* set verb=1 to turn on debug messages 
     set noundef=1 to use only defined points in vertical interploation
     defaults are 0 */
  verb=0;
  noundef=0;
  
  if (pfc->argnum>3) {
    gaprnt (0,"Error from COLL2GR:  Too many args \n");
    gaprnt (0,"                     One to three arguments expected \n");
    return (1);
  }
  if (intprs(pfc->argpnt[0],&clnm)==NULL) {
    gaprnt (0,"Error from COLL2GR:  1st argument must be an integer\n");
    return(1);
  }
  if (pfc->argnum>1) {
    if (cmpwrd("-u",pfc->argpnt[1])) lflg = 2;
    else if (intprs(pfc->argpnt[1],&i) != NULL) ucnt = i;
    else gaprnt (1,"COLL2GR Warning:  2nd arg Invalid; Ignored\n");
  }
  if (pfc->argnum>2) {
    if (cmpwrd("-n0",pfc->argpnt[2])) noundef=0;
    else if (cmpwrd("-n1",pfc->argpnt[2])) noundef=1;
    else gaprnt (1,"COLL2GR Warning:  3nd arg Invalid; Ignored\n");
  }
  clct0 = *(pst->pclct+clnm);
  clct = clct0;
  if (clct==NULL) {
     sprintf (pout,"Error from COLL2GR:  Collection %i empty\n",clnm);
     gaprnt (0,pout);
     return (1);
  }

  /* Count number of soundings, number of levels, and check dimension validity */
  cnt = 0;
  scnt = 0;
  dim = -1;
  while (clct) {
    stn = clct->stn;
    cnt += stn->rnum;
    scnt += 1;
    if (verb) printf("coll2grd debug msg: Sounding %d has rnum = %d\n",scnt,stn->rnum);
    if (dim==-1) dim = stn->idim;
    if (dim != stn->idim) dim = -999;
    clct = clct->forw;
  }
  if (verb) printf ("coll2gr debug msg: Collection count = %i %i %i\n",scnt,cnt,dim); 
  if (dim == -999 || dim != pst->jdim || pst->idim != 0 || dim<2 || dim>3 ) {
    gaprnt (0,"Error from COLL2GR:  Invalid dimension environment\n");
    return (1);
  }
  if (dim==3) {
     gaprnt  (0,"COLL2GR does not yet support time slices\n");
     return (1);
  }

  if (verb) printf ("levs = %g %g\n",pst->dmin[2],pst->dmax[2]); 

  /* Obtain sorted list of levels or times, depending on
     what sort of interpolation was requested */
  if (lflg==1) cnt = ucnt;
  levs = (gadouble *)galloc(sizeof(gadouble)*cnt,"col2grlevs");
  if (levs==NULL) {
    gaprnt (0,"Memory allocation error: COLL2GR\n");
    return (1);
  }

  if (lflg==1) {
    /* fixed number of levels */
    vlo = pst->dmin[2];
    vhi = pst->dmax[2];
    uu = (vhi - vlo)/((gadouble)cnt-1.0);
    lev = vlo;
    for (i=0; i<cnt; i++) {
      *(levs+i) = lev;
      lev += uu;
    }
    lcnt = cnt;
  } 
  else if (lflg==2) {
    /* union of all levels  */
    diff = fabs(pst->dmin[2]-pst->dmax[2])/1e4;
    lcnt = 0;
    clct = clct0;
    while (clct) {
      stn = clct->stn;
      rpt = stn->rpt;
      while (rpt) {
        lev = rpt->lev;
        i = 0;
        flag = 1;
	/* mf 20021016 -- don't use level if undef */
	if (noundef && (rpt->umask == 0) ) flag = 0;
        if (lev>pst->dmin[2] || lev<pst->dmax[2]) flag = 0;
        while (i<lcnt && flag) {
          if (fabs(*(levs+i)-lev)<diff) {
            flag = 0;
            break;
          }
          if (*(levs+i)<lev) break;
          i = i + 1;
        }
        if (flag) {
          if (i<lcnt) {
            for (j=lcnt; j>i; j--) *(levs+j) = *(levs+j-1);
          }
          *(levs+i) = lev;
          lcnt++;
        }
        rpt = rpt->rpt;
      }
      clct = clct->forw;
    }
  }


  if (verb==2) {
    i = 0;
    printf ("lcnt = %i\n",lcnt);
    while (i<lcnt) {
      j = i;
      while (j<lcnt && j<i+5) {
	printf ("%g ",*(levs+j));
	j = j + 1;
      }
      printf ("\n");
      i = i + 5;
    }
  }


  /* Allocate and fill the interpolated grid */
  gr = (gadouble *)galloc(sizeof(gadouble)*lcnt*scnt,"col2grgr");
  if (gr==NULL) {
    gaprnt (0,"Memory allocation error: collection gridding\n");
    gree(levs,"f427");
    return (1);
  }
  gru = (char *)galloc(sizeof(char)*lcnt*scnt,"col2grgru");
  if (gru==NULL) {
    gaprnt (0,"Memory allocation error: collection gridding\n");
    gree(levs,"f428");
    return (1);
  }
  clct = clct0;
  i = 0;
  while (clct) {
    stn = clct->stn;
    if (i==0) uu = stn->undef;
    for (j=0; j<lcnt; j++) {
      lev = *(levs+j);
      rpt = stn->rpt;
      lld = 9.99e33;
      lhd = 9.99e33;
      llo = lev; lhi = lev;
      vlo = stn->undef;
      vhi = stn->undef;
      flag = 0;
      while (rpt) {
        if (dequal(rpt->lev,lev,1.0e-8)==0) {
          flag = 1;
          break;
        }
        if (rpt->lev<lev) {
	  if (noundef) {
	    if (lev-rpt->lev<lld && (rpt->umask != 0) ) {
	      lld = lev-rpt->lev;
	      llo = rpt->lev;
	      vlo = rpt->val;
	    }
	  } else {
	    if (lev-rpt->lev<lld) {
	      lld = lev-rpt->lev;
	      llo = rpt->lev;
	      vlo = rpt->val;
	    }
          }
        } else {
	  if(noundef) {
	    if (rpt->lev-lev<lhd && (rpt->umask != 0) ) {
	      lhd = rpt->lev-lev;
	      lhi = rpt->lev;
	      vhi = rpt->val;
	    }
	  } else {
	    if (rpt->lev-lev<lhd) {
	      lhd = rpt->lev-lev;
	      lhi = rpt->lev;
	      vhi = rpt->val;
	    }
	  }
	}
        rpt = rpt->rpt;
      }
      if (flag) {
	*(gr+j*scnt+i) = rpt->val;
	*(gru+j*scnt+i) = 1;
      }
      else {
        if (dequal(vhi,stn->undef,1.0e-8)==0 || dequal(vlo,stn->undef,1.0e-8)==0) {
          *(gru+j*scnt+i) = 0;
        } else {
          *(gr+j*scnt+i) = vlo + (vhi-vlo)*(lev-llo)/(lhi-llo);
          *(gru+j*scnt+i) = 1;
        }
      }
    }
    clct = clct->forw;
    i++;
  }

  /* Now create the grid structure for our new grid and
     chain it in all the right places */

  pgr = (struct gagrid *)galloc(sizeof(struct gagrid),"col2grpgr");
  if (pgr==NULL) {
    gaprnt (0,"Memory Allocation Error:  display collection\n");
    gree(gr,"f429");
    gree(levs,"f430");
    return (1);
  }

  pgr->grid = gr;
  pgr->umask = gru;
  pgr->undef = uu;
  pgr->isiz = scnt;
  pgr->jsiz = lcnt;
  pgr->idim  = 0;  /* arbitrary */
  pgr->jdim  = dim;
  pgr->iwrld = 1; pgr->jwrld = 0;
  pgr->dimmin[0] = 1; pgr->dimmax[0] = scnt;
  pgr->dimmin[2] = 1; pgr->dimmax[2] = lcnt;
  pgr->exprsn = NULL;
  pgr->ilinr = 0;
  pgr->jlinr = 0;
  iv = (gadouble *)galloc(sizeof(gadouble)*(scnt+2),"col2griv");
  jv = (gadouble *)galloc(sizeof(gadouble)*(lcnt+2),"col2grjv");
  if (iv==NULL || jv==NULL) {
    gaprnt (0,"Memory Allocation Error:  display collection\n");
    gree(gr,"f431");
    gree(levs,"f432");
    gree(pgr,"f433");
    return (1);
  }
  *iv = (gadouble)scnt;
  *jv = (gadouble)lcnt;
  xdiff = pst->dmax[0] - pst->dmin[0];
  xdiff = xdiff / ( (gadouble)(scnt-1) );
  for (i=1; i<=scnt; i++) *(iv+i) = pst->dmin[0] + (gadouble)(i-1) * xdiff;
  for (i=0; i<lcnt; i++) *(jv+i+1) = *(levs+i);
  *(jv+lcnt+1) = -999.9;
  *(iv+scnt+1) = -999.9;
  pgr->ivals = iv;
  pgr->jvals = jv;
  pgr->iavals = pgr->ivals;
  pgr->javals = pgr->jvals;
  pgr->igrab = gr2lev;
  pgr->jgrab = gr2lev;
  pgr->iabgr = lev2gr;
  pgr->jabgr = lev2gr;
  pgr->alocf = 1;  /* bad news */

  pst->type = 1;
  pst->result.pgr = pgr;

  return (0);
}

/* Given a grid and a set of stations, interpolate to the
   grid using cressman technique and return the grid.             */

static gadouble rads[5] = {10.0, 7.0, 4.0, 2.0, 1.0};

gaint ffoacr (struct gafunc *pfc, struct gastat *pst) {
struct gastat pst2;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble (*iconv) (gadouble *, gadouble);
gadouble (*jconv) (gadouble *, gadouble);
gadouble *ivars, *jvars, *gr, *nw, *newbuf, *p1, *p2, *p3, *p4;
gadouble lon,lat,lnmin,lnmax,lnscl,ltmin,ltmax,ltscl;
gadouble sum;
gadouble e1,e2,e,wsum,x,y,xmin,xmax,ymin,ymax,d,d2,rad,rad2,w;
gaint *flgbuf, *ii;
gaint rc,i,j,p,siz,icnt,irad,radflg;
gadouble fgsum=-1e20;
gadouble radii[30];
char sumu,*gru,*nwu,*newbufu; 

  if (pfc->argnum<2) {
    gaprnt (0,"Error from OACRES:  Too many or too few args \n");
    gaprnt (0,"                    Two arguments expected \n");
    return (1);
  }
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gaprnt (0,"Error from OACRES: 1st argument must be a grid\n");
    gafree (pst);
    return (1);
  }
  pgr = pst->result.pgr;
  if (pgr->idim!=0 || pgr->jdim!=1) {
    gaprnt (0,"Error from OACRES: Grid must vary in X, Y\n");
    gafree (pst);
    return (1);
  }
  if (!pgr->ilinr || !pgr->jlinr) {
    gaprnt (0,"Error from OACRES: Grid must have linear scaling\n");
    gafree (pst);
    return (1);
  }
  pst2 = *pst;
  rc = gaexpr(pfc->argpnt[1],&pst2);
  if (rc) {
    gafree (pst);
    return (rc);
  }
  if (pst2.type!=0) {
    gaprnt (0,"Error from OACRES: 2nd argument must be stns\n");
    gafree (&pst2);
    gafree (pst);
    return (1);
  }
  stn = pst2.result.stn;

  /* Check for user provided radii */
  irad = 5;
  radflg = 0;
  if (pfc->argnum>2) {
    radflg = 1;
    irad = pfc->argnum-2;
    if (irad>30) {
      gaprnt (1,"Warning from OACRES:  Using 30 pass radii\n");
      irad = 30;
    }
    for (i=0; i<irad; i++) {
      if (getdbl(pfc->argpnt[i+2],radii+i)==NULL) {
        gaprnt (0,"Error from OACRES:  Radii must be constant\n");
        gaprnt (0,"                    Using default radii\n");
        radflg = 0;
        i = irad+1;
      } else {
	if (radii[i]<0.0) {
	  i++;
	  if(irad != i+1) {
	    gaprnt (0,"Error from OACRES:  Improper setting of 1st guess\n");
	    gaprnt (0,"                    Must provide the value or make the value the last argument\n");
	    gaprnt (0,"                    Using default radii\n");
	    irad=5;
	    radflg=0;
            i=irad+1;
	  } else if (getdbl(pfc->argpnt[i+2],&fgsum)==NULL) {
	    gaprnt (0,"Error from OACRES:  Improper setting of 1st guess\n");
	    gaprnt (0,"                    Bad value\n");
	    gaprnt (0,"                    Using default radii\n");
	    irad=5;
	    radflg=0;
	    fgsum=-1e20;
            i=irad+2;
	  } else {
	    if(irad<1) radflg=0;
	    i=irad+2;
	    irad=irad-2;
	  }

	}
        if ((radii[i]<0.01 || radii[i]>50.0) && i<irad) {
          gaprnt (0,"Error from OACRES:  Radii must be 0<radii<50\n");
          gaprnt (0,"                    Using default radii\n");
          radflg = 0;
          i = irad+1;
        }
      }
    }
  }

  /* Set up scaling for converting lon-lat to grid units */

  iconv = pgr->igrab;
  ivars = pgr->ivals;
  jconv = pgr->jgrab;
  jvars = pgr->jvals;
  lnmin = iconv(ivars,(gadouble)pgr->dimmin[0]);
  lnmax = iconv(ivars,(gadouble)pgr->dimmax[0]);
  ltmin = iconv(jvars,(gadouble)pgr->dimmin[1]);
  ltmax = iconv(jvars,(gadouble)pgr->dimmax[1]);
  lnscl = (lnmax-lnmin)/((gadouble)pgr->isiz-1);
  ltscl = (ltmax-ltmin)/((gadouble)pgr->jsiz-1);

  /* Now loop through each stn report and convert stn lat/lon to grid coordinates */
  rpt = stn->rpt;
  sum = 0.0;
  icnt = 0;
  while (rpt!=NULL) {
    lon = rpt->lon;
    lat = rpt->lat;
    if (lon<lnmin) lon+=360.0;
    else if (lon>lnmax) lon-=360.0;
    rpt->lon = (lon-lnmin)/lnscl;
    rpt->lat = (lat-ltmin)/ltscl;
    i = (gaint)rpt->lon;
    j = (gaint)rpt->lat;
    if (i<0 || i>(pgr->isiz-1) || 
	j<0 || j>(pgr->jsiz-1) ||
        rpt->umask==0) 
      rpt->work = -999;
    else {
      rpt->work = j*pgr->isiz + i;
      sum = sum + rpt->val;
      icnt++;
      i = (gaint)rpt->lon;
      j = (gaint)rpt->lat;
      rpt->lev = rpt->lon - (gadouble)i;
      rpt->tim = rpt->lat - (gadouble)j;
    }
    rpt=rpt->rpt;
  }
  if (icnt<2) {
    gaprnt (1,"Warning from OACRES:  Less than two stations\n");
    gaprnt (1,"    Grid will be all missing values\n");
    sumu = 0;
  } 
  else {
    sum = sum/((gadouble)icnt);
    sumu = 1;
  }

  /* Need some buffer space */
  siz = pgr->isiz * pgr->jsiz;
  newbuf = (gadouble *)galloc(sizeof(gadouble)*siz,"oacrbuf");
  if (newbuf==NULL) 
    goto err;
  newbufu = (char *)galloc(sizeof(char)*siz,"oacrbufu");
  if (newbufu==NULL) {
    gree(newbuf,"f434");
    goto err;
  }
  flgbuf = (gaint *)galloc(sizeof(gaint)*siz,"oacrbuff");
  if (flgbuf==NULL) {
    gree(newbuf,"f435");
    gree(newbufu,"f436");
    goto err;
  }

  /* Initial grid values are average of station reports */
  if (fabs(fgsum) < 1e20 ) {
    sum=fgsum;
    sumu=1;
  }
  gr = pgr->grid;
  gru = pgr->umask;
  nw = newbuf;
  nwu = newbufu;
  ii = flgbuf;
  for (i=0; i<siz; i++) {
    if (sumu==1) {
      *gr = sum; *gru = 1;
      *nw = sum; *nwu = 1;
    }
    else {
      *gru = 0;
      *nwu = 0;
    }
    *ii = 0;
    ii++; 
    gr++; gru++; 
    nw++; nwu++;
  }
  if (sumu==0) goto retrn;

  /* Perform the objective analysis */
  for (p=0; p<irad; p++) {
    if (radflg) rad = radii[p];
    else rad = rads[p];
    rad2 = rad*rad;
    gr = pgr->grid;
    nw = newbuf;
    nwu = newbufu;
    ii = flgbuf;
    for (j=0; j<pgr->jsiz; j++) {
      y = (gadouble)j;
      ymin = y - rad;
      ymax = y + rad;
      for (i=0; i<pgr->isiz; i++) {
        x = (gadouble)i;
        xmin = x - rad;
        xmax = x + rad;
        sum = 0.0; wsum = 0.0;
        rpt = stn->rpt;
        while (rpt) {
          if (rpt->work==-999 || 
	      rpt->lon < xmin || rpt->lon > xmax ||
              rpt->lat < ymin || rpt->lat > ymax ||
              (d = hypot(x-rpt->lon,y-rpt->lat)) > rad ) rpt = rpt->rpt;
          else {
            d2 = d*d;
            p1 = pgr->grid + rpt->work;
            p2 = p1+1;
            p4 = p1 + pgr->isiz;
            p3 = p4 + 1;
            e1 = *p1 + ( (*p2 - *p1)*rpt->lev );
            e2 = *p4 + ( (*p3 - *p4)*rpt->lev );
            e = e1 + ( (e2 - e1)*rpt->tim );
            e = rpt->val - e;
            w = (rad2-d2)/(rad2+d2);
            sum += e*w;
            wsum += w;
            rpt = rpt->rpt;
          }
        }
        if (wsum>1e-6) {
	  *nw = *gr + sum/wsum;
	  *nwu = 1; 
	}
        else if (p==2) *ii = 1;
        nw++; nwu++; gr++; ii++;
      }
    }
    nw  = newbuf;
    nwu = newbufu;
    gr = pgr->grid;
    gru = pgr->umask;
    for (i=0; i<siz; i++) {
      if (*nwu==1) {
	*gr = *nw;
	*gru = 1;
      }
      else {
	*gru = 0;
      }
      gr++; gru++; nw++; nwu++;
    }
  }
  ii = flgbuf;
  gru = pgr->umask;
  for (i=0; i<siz; i++) {
    if (*ii) *gru = 0;
    gru++; ii++;
  }

  gr  = pgr->grid;
  gru = pgr->umask;

retrn:
  gafree (&pst2);
  gree(newbuf,"f437");
  gree(newbufu,"f438");
  gree(flgbuf,"f439");
  return(0);

err:
  gaprnt (0,"Error in OACRES:  Unable to allocate memory\n");
  gafree (&pst2);
  gafree (pst);
  return (1);
}

gaint ffoabn (struct gafunc *pfc, struct gastat *pst) {
  struct gastat pst2;
  struct gagrid *pgr;
  struct gastn *stn;
  struct garpt *rpt;
  gadouble (*iconv) (gadouble *, gadouble);
  gadouble (*jconv) (gadouble *, gadouble);
  gadouble *ivars, *jvars, *gr;
  gadouble lon,lat,lnmin,lnmax,lnscl,ltmin,ltmax,ltscl;
  gaint *cnt, *ii;
  gaint rc,i,j,siz,icnt,cntflg;
  char *ch, *gru;
  
  if (pfc->argnum<2) {
    gaprnt (0,"Error from OABIN:  Too many or too few args \n");
    gaprnt (0,"                   Two arguments expected \n");
    return (1);
  }
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gaprnt (0,"Error from OABIN: 1st argument must be a grid\n");
    gafree (pst);
    return (1);
  }
  pgr = pst->result.pgr;
  if (pgr->idim!=0 || pgr->jdim!=1) {
    gaprnt (0,"Error from OABIN: Grid must vary in X, Y\n");
    gafree (pst);
    return (1);
  }
  if (!pgr->ilinr || !pgr->jlinr) {
    gaprnt (0,"Error from OABIN: Grid must have linear scaling\n");
    gafree (pst);
    return (1);
  }
  pst2 = *pst;
  rc = gaexpr(pfc->argpnt[1],&pst2);
  if (rc) {
    gafree (pst);
    return (rc);
  }
  if (pst2.type!=0) {
    gaprnt (0,"Error from OABIN: 2nd argument must be stns\n");
    gafree (&pst2);
    gafree (pst);
    return (1);
  }
  stn = pst2.result.stn;

  cntflg = 0;
  if (pfc->argnum>2) {
    ch = pfc->argpnt[2];
    if      (*ch == '-' && *(ch+1) == 'c') cntflg = 1;
    else if (*ch == '-' && *(ch+1) == 'f') cntflg = 2;
    else gaprnt (1,"Warning from OABIN: Invalid flag.  Ignored.\n");
  }

  /* Set up scaling for converting lon-lat to grid units */
  iconv = pgr->igrab;
  ivars = pgr->ivals;
  jconv = pgr->jgrab;
  jvars = pgr->jvals;
  lnmin = iconv(ivars,(gadouble)pgr->dimmin[0]);
  lnmax = iconv(ivars,(gadouble)pgr->dimmax[0]);
  ltmin = iconv(jvars,(gadouble)pgr->dimmin[1]);
  ltmax = iconv(jvars,(gadouble)pgr->dimmax[1]);
  lnscl = (lnmax-lnmin)/((gadouble)pgr->isiz-1);
  ltscl = (ltmax-ltmin)/((gadouble)pgr->jsiz-1);

  /* Now loop through each stn report and convert stn lat/lon to grid  coordinates */
  rpt = stn->rpt;
  icnt = 0;
  while (rpt!=NULL) {
    lon = rpt->lon;
    lat = rpt->lat;
    if (lon<lnmin) lon+=360.0;
    else if (lon>lnmax) lon-=360.0;
    rpt->lon = (lon-lnmin)/lnscl;
    rpt->lat = (lat-ltmin)/ltscl;
    /* nearest grid point center */
    i = (gaint)(rpt->lon+0.5);
    j = (gaint)(rpt->lat+0.5);
    if (i<0 || i>(pgr->isiz-1) || 
	j<0 || j>(pgr->jsiz-1) ||
        rpt->umask==0) 
      rpt->work = -999;
    else {
      rpt->work = j*pgr->isiz + i;
    }
    rpt=rpt->rpt;
  }

  /* cnt space */
  siz = pgr->isiz * pgr->jsiz;
  cnt = (gaint *)galloc(sizeof(gaint)*siz,"oabincnt");
  if (cnt==NULL) {
    goto err;
  }

  /* initialize cnt and grid to 0.0 for summing */
  gr=pgr->grid;
  for(i=0;i<siz;i++) {
    *gr=0.0;
    gr++;
  }
  ii = cnt;
  for (i=0; i<siz; i++) {
    *ii = 0;
    ii++;
  }

  /* Perform the bin analysis */
  rpt = stn->rpt;
  while (rpt) {
    gr = pgr->grid;
    ii = cnt;
    if (rpt->work==-999) {
      rpt = rpt->rpt;
    } 
    else {
      gr += rpt->work;
      ii += rpt->work;
      if( (cntflg <= 1) || (cntflg == 2 && *ii <= 1)) {
	*gr += rpt->val;
	*ii += 1;
      }
      rpt = rpt->rpt;
    }
  }
  
  gr=pgr->grid;
  gru=pgr->umask;
  ii=cnt;
  for(i=0;i<siz;i++) {
    if(*ii > 0) {
      *gr=(*gr)/(*ii);
      if(cntflg==1) *gr=(gadouble)(*ii);
      *gru = 1;
    } else {
      *gru=0;
    }
    gr++ ; gru++; ii++;
  }
  
  gafree (&pst2);
  gree(cnt,"f440");
  return(0);

err:
  gaprnt (0,"Error in OABIN:  Unable to allocate memory\n");
  gafree (&pst2);
  gafree (pst);
  return (1);
}



/* Nine point smoother */

gaint ffsmth  (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgr;
gadouble *buff, *gr, *nw;
gadouble w,s,mid,sid,cor;
gaint i,j,k,rc,siz,p;
char *gru, *nwu, *buffu;

  if (pfc->argnum!=1 && pfc->argnum!=4) {
    gaprnt (0,"Error from SMTH9:  Too many or too few args \n");
    gaprnt (0,"                   One or 4 arguments expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type!=1) {
    gaprnt (0,"Error from SMTH9:  Grid Required\n");
    gafree (pst);
    return (1);
  }

  pgr = pst->result.pgr;
  siz = pgr->isiz * pgr->jsiz;
  buff = (gadouble *)galloc(siz*sizeof(gadouble),"sm9buf");
  if (buff==NULL) {
    gaprnt (0,"Error from SMTH9:  Unable to allocate memory\n");
    gafree (pst);
    return (1);
  }
  buffu = (char *)galloc(siz*sizeof(char),"sm9bufu");
  if (buffu==NULL) {
    gaprnt (0,"Error from SMTH9:  Unable to allocate memory\n");
    gafree (pst);
    return (1);
  }

  mid = 1.0; sid = 0.5; cor = 0.3;
  if (pfc->argnum==4) {
    if (getdbl(pfc->argpnt[1],&mid)==NULL ||
        getdbl(pfc->argpnt[2],&sid)==NULL ||
        getdbl(pfc->argpnt[3],&cor)==NULL )  {
      gaprnt (0,"Error from SMTH9:  Invalid Weight Values\n");
      gaprnt (0,"  Using defaults:  1.0  0.5  0.3\n");
      mid = 1.0; sid = 0.5; cor = 0.3;
    }
  }

  gr = pgr->grid;
  gru = pgr->umask;
  nw = buff;
  nwu = buffu;
  k = 0;
  for (j=0; j<pgr->jsiz; j++) {
    for (i=0; i<pgr->isiz; i++) {
      if (*(gru+k)!=0) {
        s = *(gr+k)*mid;
        w = mid;
        if (i!=0 && *(gru+k-1)!=0) { 
	  s = s + *(gr+k-1)*sid; 
	  w+=sid;
	}
        if (i!=pgr->isiz-1 && *(gru+k+1)!=0) { 
	  s = s + *(gr+k+1)*sid; 
	  w+=sid;
	}
        if (j!=0) {
          p = k - pgr->isiz;
          if (*(gru+p)!=0) { 
	    s = s + *(gr+p)*sid; 
	    w+=sid;
	  }
          if (i!=0 && *(gru+p-1)!=0) { 
	    s = s + *(gr+p-1)*cor; 
	    w+=cor;
	  }
          if (i!=pgr->isiz-1 && *(gru+p+1)!=0) { 
	    s = s + *(gr+p+1)*cor; 
	    w+=cor;
	  }
        }
        if (j!=pgr->jsiz-1) {
          p = k + pgr->isiz;
          if (*(gru+p)!=0) { 
	    s = s + *(gr+p)*sid; 
	    w+=sid;
	  }
          if (i!=0 && *(gru+p-1)!=0) { 
	    s = s + *(gr+p-1)*cor; 
	    w+=cor;
	  }
          if (i!=pgr->isiz-1 && *(gru+p+1)!=0) { 
	    s = s + *(gr+p+1)*cor; 
	    w+=cor;
	  }
        }
        *nw = s/w;
	*nwu = 1;
      } 
      else {
	*nwu = 0;
      }
      nw++; nwu++; k++;
    }
  }
  gr = pgr->grid;
  gru = pgr->umask;
  nw = buff;
  nwu = buffu;
  for (i=0; i<siz; i++) {
    *gr  = *nw;
    *gru = *nwu; 
    gr++; gru++; nw++; nwu++;
  }

  gree(buff,"f441");
  gree(buffu,"f442");
  return (0);
}

/* Station Averaging.  Averaging is done in time only.
   If there are multiple stations per time, they are
   averaged with equal weight, then that value for that time
   is averaged with equal weight with the other times for that
   station.  The user may specify the number of times required
   for an average to be reported for a station.    */

gaint ffsave (struct gafunc *pfc, struct gastat *pst) {
struct gafile *pfi;
struct gastn *stnr, *stn;
struct garpt *rpt,*rpt2;
struct dt tinc,tloc,twrk,tstrt,tend;
gadouble gr1,gr2,*val,sum,cnt,ttt;
gaint dim,dim2,rc,d1,d2,d,fflg,i,mcnt,wflag;
gaint mos=0,mns=0,incr;
char *ch,chs[20],che[20];

  /* Check for X, Y varying environment */

  if (pst->idim!=0 || pst->jdim!=1) {
    gaprnt (0,"Error from STNAVE:  X, Y varying environment required\n");
    return(1);
  }

  /* Check for valid number of args       */

  if (pfc->argnum<3 || pfc->argnum>5) {
    gaprnt (0,"Error from STNAVE:  Too many or too few args \n");
    gaprnt (0,"                    3 to 5 arguments expected \n");
    return (1);
  }

  /* Parse the dimension expression       */

  pfi = pst->pfid;
  ch = dimprs (pfc->argpnt[1], pst, pfi, &dim, &gr1, 1, &wflag);
  if (ch==NULL || dim!=3) {
    gaprnt (0,"Error from STNAVE:  1st dimension expression invalid\n");
    return (1);
  }

  /* Now parse the 2nd dimension expression.  */

  ch = dimprs (pfc->argpnt[2], pst, pfi, &dim2, &gr2, 1, &wflag);
  if (ch==NULL || dim2!=dim) {
    gaprnt (0,"Error from STNAVE:  2nd dimension expression invalid\n");
    return (1);
  }

  /* Get optional arguments:  time increment and min number of times 
     required for a successful average */

  val = pfi->grvals[3];
  if (*(val+5)>0) { mos = (*(val+5)); mns = 0; }
  if (*(val+6)>0) { mns = (*(val+6)); mos = 0; }

  mcnt = 1;
  if (pfc->argnum >= 4) {
    if (*(pfc->argpnt[3]) == '-') {    /* Option flags? */
      i = fndarg (pfc->argpnt[3], &mcnt);
      if (i) return (1);
    } else {                           /* Assume time increment */
                               /*  Done differently from grid ave */
                               /*  to allow increment of 1yr for 
                                   daily data.  Might want to adopts
                                   this for grid ave also.  Think about
                                   a flag to handle leap years?  */
      ch = intprs(pfc->argpnt[3],&incr);
      if (ch==NULL) goto err3;
      if (*(val+5)>0) { mos = incr * (*(val+5)); mns = 0; }
      if (*(val+6)>0) { mns = incr * (*(val+6)); mos = 0; }
      if (*ch!='\0') {
        ch = rdtprs(pfc->argpnt[3],&tinc);
        if (ch==NULL) goto err3;
        mos = tinc.yr*12 + tinc.mo;
        mns = tinc.dy*1140 + tinc.hr*60 + tinc.mn;
        if (mos>0 && *(val+5)>0) {
          incr = mos / (*(val+5));
          if (mos!=incr*(*(val+5))) goto err3;
        }
        else if (mos>0 && *(val+6)>0) {
          incr = mos;  /* not really used */
        }
        else if (mns>0 && *(val+6)>0) {
          incr = mns / (*(val+6));
          if (mns!=incr*(*(val+6))) goto err3;
        }
        else goto err3;
      }
    }
  }
  if (pfc->argnum == 5) {
    i = fndarg (pfc->argpnt[4], &mcnt);
    if (i) return (1);
  }

  tinc.yr = 0; tinc.mo = mos; tinc.dy = 0; tinc.hr = 0; tinc.mn = mns;

  d1 = ceil(gr1-0.001);          /* Ave limits are integers    */
  d2 = floor(gr2+0.001);

  /* Set up the result stn block */

  stnr = (struct gastn *)galloc(sizeof(struct gastn),"stnavr");
  if (stnr==NULL) {
    gaprnt(0,"Memory allocation error:  station averaging function\n");
    return(1);
  }

  /* Loop over time */
  gr2t (pfi->grvals[3],d1,&tstrt);
  gr2t (pfi->grvals[3],d2,&tend);
  gat2ch (&tstrt,5,chs);
  gat2ch (&tend,5,che);
  sprintf (pout,"Stn Averaging.  Dim = %i, Start = %s, End = %s Incr(mos,mns) = %i %i\n", 
	   dim, chs, che, mos, mns);
  gaprnt (2,pout);

  rc = 0;
  fflg = 1;
  d = d1;
  while (d<=d2 && !rc) {
    gr2t (pfi->grvals[3],d,&(pst->tmin));
    pst->tmax = pst->tmin;
    /* evaluate the expression at this time */
    rc = gaexpr(pfc->argpnt[0],pst);
    if (rc || pst->type==1) goto err;
    stn = pst->result.stn;
    /* set up the block to hold the final result */
    if (fflg) {
      fflg = 0;       /* makes sure this is only done once */
      *stnr = *stn;
      stnr->rnum = 0;
      stnr->rpt = NULL;
      stnr->tvals = (gadouble *)galloc(sizeof(gadouble)*8,"stnavtvals");
      if (stnr->tvals==NULL) {
        gaprnt (0,"Memory Allocation Error:  Station Request Block \n");
        goto err;
      }
      for (i=0; i<8; i++) *(stnr->tvals+i) = *(stn->tvals+i);
    }

    /* Average multiple stations for this time, if any.  */

    rpt = stn->rpt;
    while (rpt) {
      rpt->work = 1;
      rpt = rpt->rpt;
    }
    rpt = stn->rpt;
    while (rpt) {
      if (rpt->work && rpt->umask!=0) {
        rpt2 = rpt->rpt;
        sum = rpt->val;
        cnt = 1.0;
	/* check for more reports to include in the average for this time */
        while (rpt2) {
	  /* additional reports must have same stid and location */
          if (rpt2->umask!=0 &&           
              !cmpch(rpt->stid,rpt2->stid,8) &&
              rpt->lat == rpt2->lat && rpt->lon == rpt2->lon) {
            sum = sum + rpt2->val;
            cnt = cnt+1.0;
            rpt2->work = 0;       /* flag these reports as already used in calculation */
          }
          rpt2 = rpt2->rpt;
        }
        rpt->val = sum/cnt;       /* rpt->val now has the average of all reports */
      } 
      rpt = rpt->rpt;
    }

    /* Now sum these with the items already in the result list */

    rpt = stn->rpt;
    while (rpt) {
      if (rpt->work) {
        rpt2 = stnr->rpt;  /* this will be NULL for the first time step */
        while (rpt2) {
          if (!cmpch(rpt->stid,rpt2->stid,8) &&
              rpt->lat == rpt2->lat && rpt->lon == rpt2->lon) break;
          rpt2 = rpt2->rpt;
        }
        if (rpt2==NULL) {
	  /* at first time step, allocate result (rpt2), copy rpt values into it  */
          rpt2 = gaarpt(stnr);
          if (rpt2==NULL) goto err;
          stnr->rnum++;
          *rpt2 = *rpt;
          rpt2->rpt = NULL;
          rpt2->work = 1;
          if (rpt->umask == 0) rpt2->umask = 0;
        } else {
	  /* for subsequent time steps, check if result is undefined */
          if (rpt2->umask == 0) {
	    /* no previous result, copy current time's average into result if defined */
	    if (rpt->umask != 0) {
              rpt2->val = rpt->val;
              rpt2->work = 1;
	      rpt2->umask = 1;
	    }
          } else if (rpt->umask != 0) {
	    /* add current time's average to result, increment work flag */
            rpt2->val = rpt2->val + rpt->val;
            rpt2->work++;
          }
        }
      }
      rpt = rpt->rpt;
    }
    gafree(pst);
   
    /*  Apply time increment */

    gr2t (pfi->grvals[3],d,&tloc);
    twrk = tinc;
    timadd (&tloc, &twrk);
    ttt = t2gr(pfi->grvals[3],&twrk);
    d = ttt;
    if (fabs(((gadouble)d)-ttt) > 0.001) {
      gaprnt(0,"Logic Error 16 in stnave.  Contact Developer(s).\n");
      goto err;
    }
  }

  /* Calculate final result, return */

  rpt = stnr->rpt;
  while (rpt) {
    if (rpt->umask!=0) {
      if (rpt->work<mcnt) rpt->umask = 0;
      else {
        cnt = (gadouble)rpt->work;
        rpt->val = rpt->val/cnt;
	rpt->umask = 1;
      }
    }
    rpt = rpt->rpt;
  }
  pst->result.stn = stnr;
  return(0);

err:
  gaprnt (0,"Error from STNAVE:  Unable to evaluate expression\n");
  gafree(pst);
  gasfre(stnr);
  return(1);
err3:
  sprintf(pout,"Error from STNAVE: Invalid time increment argument\n");
  gaprnt(0,pout);
  return (1);
}

/* Set every nth grid point value to missing in a grid */

gaint ffskip (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgr;
gaint rc,iskip,jskip,ii,jj,i,j;
char *uval;

  if (pfc->argnum<2 || pfc->argnum>3) {
    gaprnt (0,"Error from SKIP:  Too many or too few args \n");
    gaprnt (0,"                  Two or 3 arguments expected \n");
    return (1);
  }
  if (intprs(pfc->argpnt[1],&iskip)==NULL) {
    gaprnt (0,"Error from SKIP:  2nd argument must be an integer \n");
    return(1);
  }
  if (pfc->argnum>2) {
    if (intprs(pfc->argpnt[2],&jskip)==NULL) {
      gaprnt (0,"Error from SKIP:  3rd argument must be an integer\n");
      return(1);
    }
  } 
  else jskip = iskip;

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type!=1) {
    gaprnt (0,"Error from SKIP:  Grid Required\n");
    gafree (pst);
    return (1);
  }

  pgr = pst->result.pgr;
  uval = pgr->umask;
  if (pgr->idim==-1) return(0);

  iskip = iskip - 1;
  jskip = jskip - 1;
  jj = -1;
  for (j=0; j<pgr->jsiz; j++) {
    jj++;
    if (jj>jskip) jj=0;
    ii = -1;
    for (i=0; i<pgr->isiz; i++) {
      ii++;
      if (ii>iskip) ii=0;
      if (ii || jj) *uval = 0;
      uval++;
    }
  }
  return (0);
}

gaint ffgrarea (struct gafunc *pfc, struct gastat *pst) {
  struct gagrid *pgr;
  gadouble *val;
  gaint rc,i,j;
  gadouble (*iconv) (gadouble *, gadouble);
  gadouble (*jconv) (gadouble *, gadouble);
  gadouble *ivals, *jvals;
  gadouble rad,scl,w1,w2,y1,x1,alo,ahi;
  char *uval;

  if (pfc->argnum>3) {
    gaprnt (0,"Error from GRAREA:  Too many args \n");
    gaprnt (0,"                  1 arguments expected \n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type!=1) {
    gaprnt (0,"Error from GRAREA:  Grid Required\n");
    gafree (pst);
    return (1);
  }
  pgr = pst->result.pgr;
  if( (pgr->idim)!=0 || (pgr->jdim)!=1 ) {
    gaprnt (0,"Error from GRAREA:  XY (lon-lat) grid required\n");
    printf("pgr->idim = %d, pgr->jdim= %d\n",pgr->idim , pgr->jdim);
    gafree (pst);
    return (1);
  }

  rad = acos(0.0)/90.0;
  scl=1.0/720.0;
  jconv = pgr->jgrab;
  ivals = pgr->ivals;
  iconv = pgr->igrab;
  jvals = pgr->jvals;
  val = pgr->grid;
  uval = pgr->umask;
  for (j=0; j<pgr->jsiz; j++) {
    y1 = (gadouble)(j+pgr->dimmin[1]);
    alo = jconv(jvals, y1-0.5);
    ahi = jconv(jvals, y1+0.5);
    if (alo < pst->dmin[1]) alo = pst->dmin[1];
    if (alo > pst->dmax[1]) alo = pst->dmax[1];
    if (ahi < pst->dmin[1]) ahi = pst->dmin[1];
    if (ahi > pst->dmax[1]) ahi = pst->dmax[1];
    if (alo < -90.0) alo = -90.0; if (ahi < -90.0) ahi = -90.0;
    if (alo >  90.0) alo =  90.0; if (ahi >  90.0) ahi =  90.0;
    w1 = fabs(sin(ahi*rad)-sin(alo*rad));  /* area weighting (aave) */
      
    for (i=0; i<pgr->isiz; i++) {
      x1 = (gadouble)(i+pgr->dimmin[0]);
      alo = iconv(ivals, x1-0.5);
      ahi = iconv(ivals, x1+0.5);
      if (alo<pst->dmin[0]) alo = pst->dmin[0];
      if (alo>pst->dmax[0]) alo = pst->dmax[0];
      if (ahi<pst->dmin[0]) ahi = pst->dmin[0];
      if (ahi>pst->dmax[0]) ahi = pst->dmax[0];
      w2 = (ahi - alo)*scl;
      if (*uval!=0) *val = w1 * w2;
      val++; uval++;
    }
  }
  return (0);
}

gaint ffcnst  (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,cnt,flg;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val,cnst;
char *ch, *uval;

  if (pfc->argnum<2 || pfc->argnum>3) {
    gaprnt (0,"Error from CONST:  Too many or too few args \n");
    gaprnt (0,"                   Two or 3 arguments expected \n");
    return (1);
  }

  if (getdbl(pfc->argpnt[1],&cnst)==NULL) {
    gaprnt (0,"Error from CONST:  2nd argument must be a constant\n");
    return(1);
  }

  flg = 0;
  if (pfc->argnum>2) {
    ch = pfc->argpnt[2];
    if      (*ch == '-' && *(ch+1) == 'u') flg = 1;
    else if (*ch == '-' && *(ch+1) == 'a') flg = 2;
    else gaprnt (1,"Warning from CONST: Invalid flag.  Ignored.\n");
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    /* gridded data */
    pgr = pst->result.pgr;
    cnt = pgr->isiz * pgr->jsiz;
    val = pgr->grid;
    uval = pgr->umask;
    for (i=0; i<cnt; i++) {
      if (flg==0) {
        if (*uval!=0) {
	  *val = cnst;
	}
      } 
      else if (flg==1) {
        if (*uval==0) {
	  *val = cnst;
	  *uval = 1;       
	}
      } 
      else if (flg==2) {
        *val = cnst;
	*uval = 1;
      }
      val++; uval++;
    }
  } 
  else {
    /* station data */
    stn = pst->result.stn;
    rpt = stn->rpt;
    while (rpt!=NULL) {
      if (flg==0) {
        if (rpt->umask!=0) rpt->val = cnst;
      } else if (flg==1) {
        if (rpt->umask==0) rpt->val = cnst;
      } else if (flg==2) {
        rpt->val = cnst;
      }
      rpt=rpt->rpt;
    }
  }

  return (0);
}

/* Station min or max.  Done over time only.     */

gaint ffsmin (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
  rc = ffsmnx (pfc, pst, 0);
  return (rc);
}

gaint ffsmax (struct gafunc *pfc, struct gastat *pst) {
gaint rc;
  rc = ffsmnx (pfc, pst, 1);
  return (rc);
}

gaint ffsmnx (struct gafunc *pfc, struct gastat *pst, gaint flg) {
struct gafile *pfi;
struct gastn *stnr, *stn;
struct garpt *rpt,*rpt2;
gadouble gr1,gr2;
gaint dim,dim2,rc,d1,d2,d,fflg,i,mcnt,wflag;
char *ch;

  /* Check for X, Y varying environment */
  if (pst->idim!=0 || pst->jdim!=1) {
    gaprnt (0,"Error from STNMIN:  X, Y varying environment required\n");
    return(1);
  }

  /* Check for valid number of args */
  if (pfc->argnum<3 || pfc->argnum>4) {
    gaprnt (0,"Error from STNMIN:  Too many or too few args \n");
    gaprnt (0,"                    3 or 4 arguments expected \n");
    return (1);
  }

  /* Parse the dimension expression */
  pfi = pst->pfid;
  ch = dimprs (pfc->argpnt[1], pst, pfi, &dim, &gr1, 1, &wflag);
  if (ch==NULL || dim!=3) {
    gaprnt (0,"Error from STNMIN:  1st dimension expression invalid\n");
    return (1);
  }

  /* Now parse the 2nd dimension expression. */
  ch = dimprs (pfc->argpnt[2], pst, pfi, &dim2, &gr2, 1, &wflag);
  if (ch==NULL || dim2!=dim) {
    gaprnt (0,"Error from STNMIN:  2nd dimension expression invalid\n");
    return (1);
  }

  /* Get the min number of times required for average, if provided */
  mcnt = 1;
  if (pfc->argnum==4) {
    i = fndarg (pfc->argpnt[3], &mcnt);
    if (i) return (1);
  }
  /* Ave limits are integers */
  d1 = ceil(gr1-0.001);          
  d2 = floor(gr2+0.001);

  /* Set up the result stn block */
  stnr = (struct gastn *)galloc(sizeof(struct gastn),"stnminr");
  if (stnr==NULL) {
    gaprnt(0,"Memory allocation error:  station averaging function\n");
    return(1);
  }

  /* Loop over time */
  rc = 0;
  fflg = 1;
  for (d=d1; d<=d2 && !rc; d+=1) {

    /* Get next set of stations */
    gr2t (pfi->grvals[3],d,&(pst->tmin));
    pst->tmax = pst->tmin;
    rc = gaexpr(pfc->argpnt[0],pst);
    if (rc || pst->type==1) goto err;
    stn = pst->result.stn;
    if (fflg) {
      fflg = 0;
      *stnr = *stn;
      stnr->rnum = 0;
      stnr->rpt = NULL;
      stnr->tvals = (gadouble *)galloc(sizeof(gadouble)*8,"stnmintv");
      if (stnr->tvals==NULL) {
        gaprnt (0,"Memory Allocation Error:  Station Request Block \n");
        goto err;
      }
      for (i=0; i<8; i++) *(stnr->tvals+i) = *(stn->tvals+i);
    }

    /* Tabulate mins or maxes */
    rpt = stn->rpt;
    while (rpt) {
      rpt2 = stnr->rpt;
      while (rpt2) {
        if (!cmpch(rpt->stid,rpt2->stid,8) &&
            rpt->lat == rpt2->lat && rpt->lon == rpt2->lon) break;
        rpt2 = rpt2->rpt;
      }
      if (rpt2==NULL) {
        rpt2 = gaarpt(stnr);
        if (rpt2==NULL) goto err;
        stnr->rnum++;
        *rpt2 = *rpt;
        rpt2->work = 1;
        rpt2->rpt = NULL;
        if (rpt->umask == 0) rpt2->umask = 0;
      } else {
        if (rpt2->umask == 0) {
          rpt2->val = rpt->val;
          rpt2->umask = rpt->umask;
        } else if (rpt->umask != 0) {
          if (flg) {
            if (rpt->val > rpt2->val) rpt2->val = rpt->val;
          } else {
            if (rpt->val < rpt2->val) rpt2->val = rpt->val;
          }
          rpt2->work++;
        }
      }
      rpt = rpt->rpt;
    }
    gafree(pst);
  }

  /* Check min count if provided */
  if (mcnt>1) {
    rpt = stnr->rpt;
    while (rpt) {
      if (rpt->work<mcnt) rpt->umask = 0;
      rpt = rpt->rpt;
    }
  }

  /* Return final result. */
  pst->result.stn = stnr;
  return(0);

err:
  gaprnt (0,"Error from STNMIN:  Unable to evaluate expression\n");
  gafree(pst);
  gasfre(stnr);
  return(1);
}

/*  Find args for the stnave, stnmin, and stnmax functions */

gaint fndarg (char *ch, gaint *iv) {
gaint ival;

   while (*ch==' ') ch++;
   if (*ch!='-' && *(ch+1)!='m') {
     gaprnt (0,"Invalid option argument in STN function\n");
     return (1);
   }

   ch+=2;
   while (*ch==' ') ch++;

   ch = intprs(ch,&ival);
   if (ch==NULL || ival<1) {
     gaprnt (0,"Invalid option argument in STN function\n");
     return (1);
   }

   *iv = ival;
   return (0);
}

gaint ffcdif (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgr;
gaint dim,rc,is,siz,i,j,sflag;
gadouble *res,*rr,*gr;
char *resundef, *rru, *gru;

  /* Check for user errors */

  if (pfc->argnum!=2) {
    gaprnt (0,"Error from CDIFF:  Too many or too few args \n");
    gaprnt (0,"                   Two arguments expected \n");
    return (1);
  }

  if      (*(pfc->argpnt[1])=='x') dim = 0;
  else if (*(pfc->argpnt[1])=='y') dim = 1;
  else if (*(pfc->argpnt[1])=='z') dim = 2;
  else if (*(pfc->argpnt[1])=='t') dim = 3;
  else if (*(pfc->argpnt[1])=='e') dim = 4;
  else {
    gaprnt (0,"Error from CDIFF:  Invalid dimension argument\n");
    gaprnt (0,"  2nd argument must be X, Y, Z, T or E\n");
    return (1);
  }
  sflag = 0;
  if (*(pfc->argpnt[1]+1)=='p') sflag = 1;
  if (*(pfc->argpnt[1]+1)=='m') sflag = 2;
  

  /* Get the result grid. */

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);
  if (pst->type==0) {
    gafree (pst);
    return(-1);
  }
  pgr = pst->result.pgr;

  /* Verify that the dimension specified is a varying dimension */

  if (dim!=pgr->idim && dim!=pgr->jdim) {
    gaprnt (0,"Error from CDIFF:  Specified dimension non varying\n");
    gafree(pst);
    return(1);
  }

  /* Get the output grid and its undef mask */

  siz = pgr->isiz * pgr->jsiz;
  res = (gadouble *)galloc(siz*sizeof(gadouble),"cdifres");
  if (res==NULL) {
    gaprnt (0,"Error from CDIFF:  Memory allocation error\n");
    gafree(pst);
    return (1);
  }
  resundef = (char *)galloc(siz*sizeof(char),"cdifresu");
  if (resundef==NULL) {
    gaprnt (0,"Error from CDIFF:  Memory allocation error\n");
    gafree(pst);
    return (1);
  }

  /* Do the differencing */

  gr = pgr->grid;
  gru = pgr->umask;
  rr = res;
  rru = resundef;
  is = pgr->isiz;
  if (dim==pgr->jdim) {
    for (j=0; j<pgr->jsiz; j++) {
      for (i=0; i<pgr->isiz; i++) {
        if (sflag == 0) {
          if (j==0 || j==pgr->jsiz-1) {
	    *rru = 0;
	  }
          else {
            if (*(gru+is)==0 || *(gru-is)==0) {
              *rru = 0;
            } 
	    else {
	      *rr = *(gr+is) - *(gr-is);
	      *rru = 1;
	    }
          }
        } else if (sflag == 1) {
          if (j==pgr->jsiz-1) {
	    *rru = 0;
	  }
          else {
            if (*(gru+is)==0 || *(gru)==0) {
              *rru = 0;
            } 
	    else {
	      *rr = *(gr+is) - *(gr);
	      *rru = 1;
	    }
          }
        } else {
          if (j==0) {
	    *rru = 0;
	  }
          else {
            if (*(gru)==0 || *(gru-is)==0) {
              *rru = 0;
            } 
	    else {
	      *rr = *(gr) - *(gr-is);
	      *rru = 1;
	    }
          }
        }
        gr++; gru++; rr++; rru++;
      }
    }
  } 
  else {
    for (j=0; j<pgr->jsiz; j++) {
      for (i=0; i<pgr->isiz; i++) {
        if (sflag==0) {
          if (i==0 || i==pgr->isiz-1) {
	    *rru = 0;
	  }
          else {
            if (*(gru+1)==0 || *(gru-1)==0) {
              *rru = 0;
            } 
	    else {
	      *rr = *(gr+1) - *(gr-1);
	      *rru = 1;
	    }
          }
        } 
	else if (sflag==1) {
          if (i==pgr->isiz-1) {
	    *rru = 0;
	  }
          else {
            if (*(gru+1)==0 || *(gru)==0) {
              *rru = 0;
            } 
	    else {
	      *rr = *(gr+1) - *(gr);
	      *rru = 1;
	    }
          }
        } 
	else {
          if (i==0) {
	    *rru = 0;
	  }
          else {
            if (*(gru)==0 || *(gru-1)==0) {
              *rru = 0;
            } 
	    else {
	      *rr = *(gr) - *(gr-1);
	      *rru = 1;
	    }
          }
        }
        gr++; gru++; rr++; rru++;
      }
    }
  }

  gree(pgr->grid,"f443");
  pgr->grid = res;
  pgr->umask = resundef;
  return (0);
}

/* Routine to read the user function definition file, and build
   the appropriate link list of function definition blocks.
   The file name is pointed to by the GAFDEF environment variable;
   if unset then no user functions will be set up */

void gafdef (void) {
/* struct gaufb *ufb, *oufb=NULL; */
/* char *cname; */
/* FILE *cfile; */
/* char rec[260],*ch; */
/* gaint i,j,pass; */

  ufba = NULL;
  return; 

  /* JMA remainder of subroutine commented out pending implementation of DLLs */


/*   /\* Make two passes.  First read user function table, then read */
/*      system function table *\/ */

/*   pass = 0; */
/*   while (pass<2) { */
/*     if (pass==0) { */
/*       cname = getenv("GAUDFT"); */
/*       if (cname==NULL) { */
/*         pass++; */
/*         continue; */
/*       } */
/*       cfile = fopen(cname,"r"); */
/*       if (cfile==NULL) { */
/*         gaprnt(0,"Error opening user function definition table\n"); */
/*         sprintf (pout,"  File name is: %s\n",cname); */
/*         gaprnt (0,pout); */
/*         pass++; */
/* 	gree(cname,"f300"); */
/*         continue; */
/*       } */
/*     } else { */
/*       cname = gxgnam("udft"); */
/*       cfile = fopen(cname,"r"); */
/*       if (cfile==NULL) { */
/* 	gree(cname,"f301"); */
/* 	break; */
/*       } */
/*     } */

/*     /\* Read the file. *\/ */

/*     while (1) { */
/*       ufb = (struct gaufb *)malloc(sizeof(struct gaufb)); */
/*       if (ufb==NULL) goto memerr; */

/*       /\* Read First record (name and arg types) *\/ */

/*       ch = fgets(rec,256,cfile); */
/*       if (ch==NULL) break; */
/*       ch = rec; */
/*       lowcas(ch); */
/*       while (*ch==' ') ch++; */
/*       i = 0; */
/*       while (*ch!=' ' && *ch!='\0' && *ch!='\n') { */
/*         if (i<15) { */
/*           ufb->name[i] = *ch; */
/*           i++; */
/*         } */
/*         ch++; */
/*       } */
/*       ufb->name[i] = '\0'; */
/*       if (*ch!=' ') goto fmterr; */
/*       while (*ch==' ') ch++; */
/*       if (intprs(ch,&(ufb->alo))==NULL) goto fmterr; */
/*       if ( (ch = nxtwrd(ch))==NULL) goto fmterr; */
/*       if (intprs(ch,&(ufb->ahi))==NULL) goto fmterr; */
/*       i = 0; */
/*       while (i<ufb->ahi) { */
/*         if ( (ch = nxtwrd(ch))==NULL) goto fmterr; */
/*         if (cmpwrd("expr",ch)) ufb->atype[i]=1; */
/*         else if (cmpwrd("value",ch)) ufb->atype[i]=2; */
/*         else if (cmpwrd("char",ch)) ufb->atype[i]=3; */
/*         else goto fmterr; */
/*         i++; */
/*       } */

/*       /\* Read 2nd record -- options *\/ */

/*       ch = fgets(rec,256,cfile); */
/*       if (ch==NULL) goto rderr; */
/*       ch = rec; */
/*       lowcas(ch); */
/*       while (*ch==' ') ch++; */
/*       if (*ch=='\n' || *ch=='\0') goto fmterr; */
/*       while (1) { */
/*         if (cmpwrd("direct",ch)) ufb->sflg=0; */
/*         else if (cmpwrd("sequential",ch)) ufb->sflg=1; */
/*         else goto fmterr; */
/*         if ( (ch = nxtwrd(ch))==NULL) break; */
/*       } */

/*       /\* Read 3rd record -- file name of executable *\/ */

/*       ch = fgets(rec,256,cfile); */
/*       if (ch==NULL) goto rderr; */
/*       i = 0; */
/*       while (rec[i]!='\n' && rec[i]!='\0') i++; */
/*       ufb->fname = (char *)malloc(i+1); */
/*       if (ufb->fname==NULL) { */
/*         free(ufb); */
/*         goto memerr; */
/*       } */
/*       for (j=0; j<i; j++) *(ufb->fname+j) = rec[j]; */
/*       *(ufb->fname+i) = '\0'; */

/*       /\* Read 4th record -- file name of data transfer to user *\/ */

/*       ch = fgets(rec,256,cfile); */
/*       if (ch==NULL) goto rderr; */
/*       i = 0; */
/*       while (rec[i]!='\n' && rec[i]!='\0') i++; */
/*       ufb->oname = (char *)malloc(i+1); */
/*       if (ufb->oname==NULL) { */
/*         free(ufb); */
/*         goto memerr; */
/*       } */
/*       for (j=0; j<i; j++) *(ufb->oname+j) = rec[j]; */
/*       *(ufb->oname+i) = '\0'; */

/*       /\* Read 5th record -- file name for data transfer from user *\/ */

/*       ch = fgets(rec,256,cfile); */
/*       if (ch==NULL) goto rderr; */
/*       i = 0; */
/*       while (rec[i]!='\n' && rec[i]!='\0') i++; */
/*       ufb->iname = (char *)malloc(i+1); */
/*       if (ufb->iname==NULL) { */
/*         free(ufb); */
/*         goto memerr; */
/*       } */
/*       for (j=0; j<i; j++) *(ufb->iname+j) = rec[j]; */
/*       *(ufb->iname+i) = '\0'; */

/*       /\* Chain this ufb *\/ */

/*       ufb->ufb = NULL; */

/*       if (ufba==NULL) ufba = ufb; */
/*       else oufb->ufb = ufb; */
/*       oufb = ufb; */
/*     } */

/*     fclose (cfile); */
/*     if (pass>0 && cname!=NULL) gree(cname,"f306"); */
/*     pass++; */
/*   } */
/*   return; */

/* memerr: */
/*   gaprnt(0,"Memory allocation error: user defined functions\n"); */
/*   return; */

/* fmterr: */
/*   gaprnt(0,"Format error in user defined function table:\n"); */
/*   sprintf (pout,"  Processing function name: %s\n",ufb->name); */
/*   gaprnt (0,pout); */
/*   free(ufb); */
/*   goto wname; */

/* rderr: */
/*   gaprnt(0,"Read error on user defined function table:\n"); */
/*   free(ufb); */
/*   goto wname; */

/* wname: */
/*   sprintf (pout,"  File name is: %s\n",cname); */
/*   gaprnt (0,pout); */
/*   if (cname!=NULL) gree(cname,"f309"); */
/*   return; */
}

/* JMA this subroutine commented out pending implementation of DLLs */
void gaqufb (void) {
/* struct gaufb *ufb; */
/* char name[20]; */
/* gaint i; */

/*   ufb = ufba; */
/*   while (ufb) { */
/*     for (i=0; i<8; i++) name[i] = ufb->name[i]; */
/*     name[8] = '\0'; */
/*     sprintf (pout,"%s  Args: %i %i  Exec: %s\n",name,ufb->alo,ufb->ahi, */
/*               ufb->fname); */
/*     gaprnt (2,pout); */
/*     ufb = ufb->ufb; */
/*   } */
}

gaint ffflvl (struct gafunc *pfc, struct gastat *pst) {
struct gagrid *pgr1, *pgr2, *pgrv;
struct gafile *pfi;
gadouble (*lvconv) (gadouble *, gadouble);
gadouble levf, levl, *levs, clev, ulev, flev, llev, lev, lev1, lev2;
gadouble *lvvals, *gr1, *gr2, *grv, *grr, *res;
gaint rc, i, j, lvt, dim, wflag, cnt, size;
char *ch, *resundef, *gr1u, *gr2u, *grru;

  if (pfc->argnum!=4) {
    gaprnt (0,"Error from FNDLVL:  Too many or too few args \n");
    gaprnt (0,"                    Four arguments expected \n");
    return (1);
  }

  /* Get 1st and last level.  They are args 3 and 4 and should
     be valid 'z' dimension expressions. */

  pfi = pst->pfid;

  ch = dimprs (pfc->argpnt[2], pst, pfi, &dim, &levf, 1, &wflag);
  if (ch==NULL || dim!=2) {
    gaprnt (0,"Error from FNDLVL:  Arg 3 an invalid Z dimension expression\n");
    return (1);
  }
  ch = dimprs (pfc->argpnt[3], pst, pfi, &dim, &levl, 1, &wflag);
  if (ch==NULL || dim!=2) {
    gaprnt (0,"Error from FNDLVL:  Arg 4 an invalid Z dimension expression\n");
    return (1);
  }

  /* Determine the levels we need to process, given the range provided
     by the user and the data levels available in the default file */

  lvt = pfi->dnum[2];
  if (lvt<3) {
    gaprnt (0,"Error from FNDLVL:  Too few levels in default file \n");
    return (1);
  }

  levs = (gadouble *)galloc(sizeof(gadouble)*lvt,"fndlevs");
  if (levs==NULL) {
    gaprnt (0,"Error from FNDLVL:  Memory allocation error \n");
    return (1);
  }
  cnt = 0;
  
  lvconv = pfi->gr2ab[2];
  lvvals = pfi->grvals[2];
  clev = lvconv(lvvals, 1.0);
  ulev = lvconv(lvvals, (gadouble)lvt);
  flev = lvconv(lvvals, levf);
  llev = lvconv(lvvals, levl);

  if ( (clev<ulev && flev<llev) ||
       (clev>ulev && flev>llev) ) {  /* User ordering is same as file */
    for (i=1; i<=lvt; i++) {
      lev = lvconv(lvvals, (gadouble)i);
      if ( (flev<llev && (lev>=flev && lev<=llev)) ||
           (flev>llev && (lev<=flev && lev>=llev)) ) {
        *(levs+cnt) = lev;  cnt++; 
      }
    }
  } else {                           /* User ordering is reverse of file */
    for (i=lvt; i>=1; i--) {
      lev = lvconv(lvvals, (gadouble)i);
      if ( (flev<llev && (lev>=flev && lev<=llev)) ||
           (flev>llev && (lev<=flev && lev>=llev)) ) {
        *(levs+cnt) = lev;  cnt++; 
      }
    }
  }

  /* Insure z is not a varying dimension */

  lev1 = *levs;
  if (pst->idim==2) {
    pst->idim = pst->jdim;
    pst->jdim = -1;
    pst->dmin[2] = lev1;
    pst->dmax[2] = lev1;
  }
  if (pst->jdim==2) {
    pst->jdim = -1;
    pst->dmin[2] = lev1;
    pst->dmax[2] = lev1;
  }

  /* Get the level to find (2nd arg).   Must be an expression that 
     yields the same grid the 1st arg will yield. */                              
 
  rc = gaexpr(pfc->argpnt[1],pst);
  if (rc) {
    gree(levs,"f444");
    return (1);
  }
  if (pst->type==0) {
    gree(levs,"f445");
    gafree (pst);
    return (-1);
  }
  pgrv = pst->result.pgr;

  /* Get first grid (at 1st level) from 1st arg. */

  pst->dmin[2] = lev1;
  pst->dmax[2] = lev1;
  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) {
    gree(levs,"f446");
    gagfre (pgrv);
    return (1);
  }
  if (pst->type==0) {
    gree(levs,"f447");
    gafree (pst);
    gagfre (pgrv);
    return (-1);
  }
  pgr1 = pst->result.pgr;

  /* Check that the two grids are equivalent.  */

  if (pgrv->idim!=pgr1->idim || pgrv->jdim != pgr1->jdim ||
      gagchk(pgrv,pgr1,pgrv->idim) || gagchk(pgrv,pgr1,pgrv->jdim)) {
    gaprnt (0,"Error from FNDLVL :  Incompatible grids. \n");
    gree(levs,"f448");
    gafree (pst);
    gagfre (pgrv);
    return (1);
  }

  /* Allocate space to hold the result and its undef mask */

  size = pgrv->isiz * pgrv->jsiz;
  res = (gadouble *)galloc(sizeof(gadouble)*size,"fndlvlres");
  if (res==NULL) {
    gaprnt (0,"Error from FNDLVL :  Memory allocation error \n");
    gree(levs,"f449");
    gafree (pst);
    gagfre (pgrv);
    return (1);
  }
  resundef = (char *)galloc(sizeof(char)*size,"fndlvlresu");
  if (resundef==NULL) {
    gaprnt (0,"Error from FNDLVL :  Memory allocation error \n");
    gree(levs,"f450");
    gafree (pst);
    gagfre (pgrv);
    return (1);
  }
  for (i=0; i<size; i++) *(resundef+i) = 0;

  /* Loop through the levels and, when appropriate, interpolate to find
     the level.  */

  for (i=1; i<cnt; i++) {
    lev2 = *(levs+i);
    pst->dmin[2] = lev2;
    pst->dmax[2] = lev2;
    rc = gaexpr(pfc->argpnt[0],pst);
    if (rc) {             
      gree(levs,"f451");
      gree(res,"f452");
      gagfre (pgr1);
      gagfre (pgrv);
      return (1);
    }
    if (pst->type==0) {
      gree(levs,"f453");
      gree(res,"f454");
      gafree (pst);
      gagfre (pgr1);
      gagfre (pgrv);
      return (-1);
    }
    pgr2 = pst->result.pgr;

    gr1 = pgr1->grid; gr1u = pgr1->umask;
    gr2 = pgr2->grid; gr2u = pgr2->umask;
    grr = res;        grru = resundef;
    grv = pgrv->grid;
    for (j=0; j<size; j++) {
     if (*grru == 0) {                                       /* If we haven't yet found a level...  */
      if (*gr1u != 0 && *gr2u != 0) {                        /* and data is available... */
       if ((*gr1 <  *gr2 && *grv >= *gr1 && *grv <= *gr2) ||
           (*gr1 >= *gr2 && *grv <= *gr1 && *grv >= *gr2)) { /* and the level falls in this layer... */
         if (fabs(*gr2 - *gr1) < 1e-5) {
	   *grr = lev1; 
	   *grru = 1;
	 }
         else {
	   *grr = lev1 + (lev2-lev1)*(*grv - *gr1)/(*gr2 - *gr1);   /* then interpolate. */
	   *grru = 1;
	 }
       }
      }
     }
     gr1++; gr1u++; 
     gr2++; gr2u++;
     grr++; grru++;
     grv++; 
    }
    gagfre(pgr1);
    pgr1 = pgr2;
    lev1 = lev2;
  }

  /* Release storage and return */
  
  gagfre (pgr1);
  if (pgrv->idim>-1) gree(pgrv->grid,"f455");
  if (pgrv->idim==-1) {
    pgrv->rmin = *res;
    pgrv->umin = *resundef;
    gree(res,"f456");
    gree(resundef,"457");
  } 
  else {
    pgrv->grid = res;
    pgrv->umask = resundef;
  }

  pst->type = 1;
  pst->result.pgr = pgrv;
  gree(levs,"f458");
  return (0);
}


/*  Convert a station data time series into a grid; 
    this allows more graphics operations and 
    analytical comparisons.  */

gaint ffs2g1d (struct gafunc *pfc, struct gastat *pst) {
gaint i,rc,size,idim,jdim;
struct gagrid *pgr;
struct gastn *stn;
struct garpt *rpt;
gadouble *val,*tvals;
char *valundef;

  if (pfc->argnum!=1) {
    gaprnt (0,"Error from S2G1D: Too many or too few args \n");
    gaprnt (0,"                  One argument expected \n");
    return (1);
  }

  if (pst->idim != 3 || pst->jdim != -1) {
    gaprnt (0,"Error from S2G1D: Time can be only varying dimension\n");
    return (1);
  }

  rc = gaexpr(pfc->argpnt[0],pst);
  if (rc) return (rc);

  if (pst->type==1) {
    gaprnt (0,"Error from S2G1D: Station data argument expected\n");
    return (1);
  }

  stn = pst->result.stn;

  if (stn->idim != 3 || stn->jdim != -1) {
    gaprnt (0,"Error from S2G1D: Logic Error 4; contact developer\n");
    return (1);
  }

  size = sizeof(struct gagrid);
  pgr = (struct gagrid *)galloc(size,"s2g1dpgr");

  if (pgr==NULL) {
    gaprnt (0,"Memory Allocation Error:  Grid Request Block\n");
    return (1);
  }

  /* Fill in gagrid variables */
  
  pgr->pfile = NULL;
  pgr->undef = stn->undef;
  pgr->isiz = 1 + stn->tmax - stn->tmin;
  pgr->jsiz = 1; 
  idim = stn->idim; jdim = stn->jdim;
  pgr->exprsn = NULL;
  pgr->alocf = 1;
  pgr->pvar  = NULL;
  pgr->idim  = idim;
  pgr->jdim  = jdim;
  pgr->iwrld = 0;  pgr->jwrld = 0;
  for (i=0;i<3;i++) {
    pgr->dimmin[i] = 1;
    pgr->dimmax[i] = 1;
  }
  pgr->dimmin[3] = stn->tmin;
  pgr->dimmax[3] = stn->tmax;
  pgr->ilinr = 1;
  pgr->jlinr = 0;

  tvals = (gadouble *)galloc(sizeof(gadouble)*8,"s2g1dtvals");
  if (tvals == NULL) {
    gaprnt (0,"Memory Allocation Error: Dimension array\n");
    gree(pgr,"f459");
    return (1);
  }

  for (i=0; i<8; i++) {
    *(tvals+i) = *(stn->tvals+i);
  }
  pgr->ivals = tvals;
  pgr->iavals = tvals;
  pgr->jvals = NULL;
  /* this is for the grid */
  pgr->grid = (gadouble *)galloc(sizeof(gadouble)*pgr->isiz,"s2g1dgr");
  if (pgr->grid == NULL) {
    gaprnt (0,"Memory Allocation Error: Data array\n");
    gree(pgr,"f460");
    return (1);
  }
  /* this is for the undef mask */
  pgr->umask = (char *)galloc(sizeof(char)*pgr->isiz,"s2g2dgru");
  if (pgr->umask == NULL) {
    gaprnt (0,"Memory Allocation Error: Undef mask array\n");
    gree(pgr,"f461");
    return (1);
  }

  /* pre-fill the grid with undef values */
  valundef = pgr->umask;
  size = pgr->isiz;
  for (i=0; i<size; i++) {
    *valundef = 0;
    valundef++;
  }
  /* now populate the grid with station values */
  valundef = pgr->umask;
  val = pgr->grid;
  rpt = stn->rpt;
  while (rpt!=NULL) {
    if (rpt->val != stn->undef) {
      i = (gaint)(rpt->tim) - stn->tmin;
      *(val+i) = rpt->val;
      *(valundef+i) = 1;
    }
    rpt=rpt->rpt;
  }

  gafree(pst);

  pst->type = 1;
  pst->result.pgr = pgr;
  
  return (0);
}

