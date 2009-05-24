/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

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

#define DRIVER_GAGMAP
#include "grads.h"
#include "gagmap.h"
#include "gx.h"

extern struct gamfcmn mfcmn;
gaint help=0;
void command_line_help(void) ;

gaint main (gaint argc, char *argv[]) {


/* ---------------- command line processing   ------------------- */

/* Initialize */
  skip=0;
  verb=0;         /* verbose default is NO */
  quiet=0;        /* quiet mode default is NO */
  g1ver=3;        /* default version for grib1 maps */
  g2ver=1;        /* default version for grib2 maps */
  scaneof=0;      /* option to ignore failure to find data at end of file */
  scanEOF=0;      /* option to ignore failure to find data at end of file */
  scanlim=1000;   /* the default # of max bytes between records */
  notau=0;        /* force time to be base time */
  tauave=0;       /* use end time (default) for averaged products vs. start time */
  tauflg=0;       /* search for a fixed tau in filling the 4-D volume */
  tauoff=0;       /* the fixed tau in h */
  tau0=0;         /* set the base dtg for tau search */
  update=0;       /* set the base dtg for tau search */
  write_map=1;    /* write out the map  */
  diag=0;         /* full diagnostics */
  mpiflg=0;
  ifile = NULL;
  no_min=0;	  /* keep minutes code */

  mfcmn.fullyear=1; /* initialize the GrADS calendar so it is set to the file calendar in gaddes */

  if (argc>1) {
    iarg = 1;
    while (iarg<argc) {
      flg = 1;
      ch = argv[iarg];
      if (*ch=='-' && *(ch+1)=='i') {         /* input filename, data descriptor file */
        iarg++;
        if (iarg<argc) {
          ifile = argv[iarg];
          flg = 0;
        } else iarg--;                        /* Let error message pop */
      }
      else if (*ch=='-' && *(ch+1)=='0') {    /* Ignore forecast time so that the */
        notau = 1;                            /* reference time is the valid time */ 
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='e') {    /* Ignore junk bytes at end of file (ECMWF) */
        scaneof = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='E') {    /* Ignore junk bytes at middle or end of file (ECMWF) */
        scanEOF = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='1') {    /* Create machine-specific version 1 map */
        g1ver = 1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='2') {    /* Create machine-independent version 2 map */
        g1ver = 2;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='u') {    /* Update existing map (for templated data sets) */
	printf("The -u option has been temporarily disabled\n");
	/* update = 1; */
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='N') {    /* Do not write a map file */
        write_map=0;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='h' && *(ch+2)=='e' && *(ch+3)=='l' && *(ch+4)=='p' ) { /* help */
        help=1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='b') {    /* Valid time for averages is beginning of period */
        tauave=1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='d') {    /* Diagnostic output */
        diag=1;
	flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='s') {    /* Skip over bytes between records */ 
        scanflg = 1;
        ch+=2;
        i = 0;
        while(*(ch+i) && i<900) {
          if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
          i++;
        }
        if (i<900) {
          sscanf(ch,"%i",&scanlim);
          flg = 0;
        }
      }
      else if (*ch=='-' && *(ch+1)=='f') {    /* Match only the given forecast time */
        tauflg = 1;
        ch+=2;
        i = 0;
        while(*(ch+i) && i<900) {
          if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
          i++;
        }
        if (i<900) {
          sscanf(ch,"%i",&tauoff);
          flg = 0;
        }
      }
      else if (*ch=='-' && *(ch+1)=='t' && *(ch+2)=='0') {  /* Match only if base time == initial time */
        tau0=1;
        flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='v') {    /* Verbose mode */
        if(!quiet) verb=1;
        flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='q') {    /* Quiet mode */
        quiet=1;
	verb=0;
        flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='m' && *(ch+2)=='i' && *(ch+3)=='n' && *(ch+4)=='0') { /* Ignore minutes */
        no_min = 1;
        flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='m') {         /* Use base time from descriptor instead of grib header */
        mpiflg = 1;
        flg = 0;
      }
      else if (*ch=='-' && *(ch+1)=='h') {          /* Header bytes to skip before scanning */
        ch+=2;
	i = 0;
	while(*(ch+i) && i<900) {
	  if (*(ch+i)<'0' || *(ch+i)>'9') i = 999;
	  i++;
	}
	if (i<900) {
	  sscanf(ch,"%i",&skip);
	  flg = 0;
	}
      }
      if (flg) {
        printf ("Invalid command line argument: %s  Ignored.\n",argv[iarg]);
      }
      iarg++;
    }

  } 
  else {
    command_line_help();
  }

  if(help) {
    command_line_help();
  }

  mfcmn.cal365=-1; /* initialize the GrADS calendar so it is set to the file calendar in gaddes */
  rc=gribmap();
  return(rc);
}


void command_line_help (void) {

/* output command line options */
printf("gribmap for GrADS Version " GRADS_VERSION "\n");
printf("creates the \"map\" file for using GRIB data in grads\n\n");
printf("Command line options: \n\n");
printf("   -help     prints out this help\n");
printf("   -i fname  provides the name of data descriptor file to map\n");
printf("   -v        verbose option shows details of the mapping\n");
printf("   -q        quiet mode gives no listing except for errors\n");
printf("   -hNNNN    where NNNN is the # of header bytes to look for first GRIB messages (default is 100)\n");
printf("   -sNNNN    where NNNN is the maximum # of bytes to skip between GRIB messages (default is 1000)\n");
/* printf("   -u        updates existing gribmap (N.B. This option is temporarily disabled)\n"); */
printf("   -1        creates a machine specific version 1 map \n");
printf("   -2        creates a machine-INDEPENDENT version 2 map \n");
printf("   -0        ignores the forecast time in the mapping; only uses the base time\n");
printf("   -t0       forces a match if base time in the GRIB header equals the initial time in the descriptor file\n");
printf("   -fNNNN    forces a match for forecast time in hours = NNNNN (e.g., f24 for t=24 h)\n");
printf("   -min0     ignores minutes code \n");
printf("   -e        ignores junk bytes (non GRIB msg) at end of file (e.g., ECMWF GRIB files)\n");
printf("   -E        ignores junk bytes in middle and/or end of GRIB file\n");
printf("   -N        does NOT write out the map \n");
printf("   -m        uses the initial time from the descriptor file instead of the base time in the grib header \n");
printf("   -b        set the valid time for averages to be the beginning of the period rather than the end (default)\n\n");

}

void gaprnt (gaint i, char *ch) {
  printf ("%s",ch);
}

char *gxgsym(char *ch) {
  return (getenv(ch));
}
