/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* Main program for GrADS (Grid Analysis and Display System).
   This program loops on commands from the user, and calls the
   appropriate routines based on the command entered.             */

/* GrADS originally authored by Brian E. Doty.  Many others have 
   contributed over the years...  

   Jennifer M. Adams 
   Reinhard Budich 
   Luigi Calori 
   Wesley Ebisuzaki 
   Mike Fiorino 
   Matthias Grzeschik
   Tom Holt 
   Don Hooper 
   James L. Kinter 
   Steve Lord 
   Gary Love 
   Karin Meier 
   Matthias Munnich 
   Uwe Schulzweida 
   Arlindo da Silva 
   Michael Timlin 
   Pedro Tsai 
   Joe Wielgosz 
   Brian Wilkinson 
   Katja Winger

   We apologize if we have neglected your contribution --
   but let us know so we can add your name to this list.
*/

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
#include <string.h>
#include <math.h>
#include <signal.h>
#include "grads.h"

#if USEGUI == 1
#include "gagui.h"
#endif

#ifdef XLIBEMU
void using_history(void);
void stifle_history (gaint);
#endif

#if READLINE ==1
#include <time.h>
#include <stdlib.h>
#include <readline/history.h>
extern gaint history_length;
void write_command_log(char *logfile);
#endif

struct gacmn gcmn;  
static struct gawgds wgds;   
extern struct gamfcmn mfcmn;

#if defined(XLIBEMU32)
GRXMain (gaint argc, char *argv[])  {
#else
gaint main (gaint argc, char *argv[])  {
#endif

void command_line_help(void) ;
void gxdgeo (char *);
void gxend (void);

char cmd[500];
gaint rc,i,j,land,port,cmdflg,hstflg,gflag,xwideflg,killflg;
gaint metabuff,size=0,g2size=0;
gaint ipcflg = 0; /* for IPC friendly interaction via pipes */
char *icmd,*arg,*rc1;
void gasigcpu() ;
gaint wrhstflg=0; 
#if READLINE == 1
char *logfile,*userhome=NULL;
#endif


/*--- common block sets before gainit ---*/
gcmn.batflg = 0;
land = 0;
port = 0;
cmdflg = 0;
metabuff = 0;
hstflg = 1; 
gflag = 0;
xwideflg = 0;
icmd = NULL;
arg = NULL;
rc1 = NULL;
killflg = 0;

#if READLINE == 1
#ifdef __GO32__  /* MSDOS case */ 
logfile= (char *) malloc(22);
logfile= "c:\windows\grads.log";
#else  /* Unix */
userhome=getenv("HOME");
if (userhome==NULL) {
  logfile=NULL;
}
else {
  logfile= (char *) malloc(strlen(userhome)+12);
  if(logfile==NULL) {
    printf("Memory allocation error for logfile name.\n");
  }
  else {
    strcpy(logfile,userhome);
    strcat(logfile,"/.grads.log");
  }
}
#endif /* __GO32__ */
#endif /* READLINE == 1 */

if (argc>1) {
  for (i=1; i<argc; i++) {
    if (cmdflg==1) {
      icmd = argv[i];
      cmdflg = 0;
    } else if (metabuff) {
      arg = argv[i];
      rc1 = intprs(arg,&size);
      if (rc1!=NULL) {
	metabuff = 0;
      }
/*     } else if (g2buff) { */
/*       arg = argv[i]; */
/*       rc1 = intprs(arg,&g2size); */
/*       if (rc1!=NULL) { */
/* 	g2buff = 0; */
/*       } */
    } else if (*(argv[i])=='-' && 
	       *(argv[i]+1)=='h' && *(argv[i]+2)=='e' && *(argv[i]+3)=='l' && *(argv[i]+4)=='p') {
      command_line_help();
      return(0);
    } else if (gflag) {
      gxdgeo(argv[i]);
#if READLINE ==1
      /* read optional log file name if previous option was "-H" */
    } else if (wrhstflg && *(argv[i])!='-') {
        logfile=argv[i];
#endif
    } else if (*(argv[i])=='-' && *(argv[i]+1)=='g') {
        gflag = 1;
    } else if (*(argv[i])=='-') {
      j = 1;
      while (*(argv[i]+j)) {
	if (*(argv[i]+j)=='b') gcmn.batflg = 1;
	else if (*(argv[i]+j)=='l') land = 1;
	else if (*(argv[i]+j)=='p') port = 1;
	else if (*(argv[i]+j)=='c') cmdflg = 1;
	else if (*(argv[i]+j)=='m') metabuff = 1;
/* 	else if (*(argv[i]+j)=='n') g2buff = 1; */
	else if (*(argv[i]+j)=='W') xwideflg = 1;
	else if (*(argv[i]+j)=='E') hstflg = 0;
	else if (*(argv[i]+j)=='u') {            /* unbuffer output: needed for IPC via pipes */
	  hstflg = 0;                            /* no need for readline in IPC mode */
	  ipcflg = 1;
	  setvbuf(stdin,  (char *) NULL,  _IONBF, 0 );
	  setvbuf(stdout, (char *) NULL,  _IONBF, 0 );
	  setvbuf(stderr, (char *) NULL,  _IONBF, 0 );
	}
	else if (*(argv[i]+j)=='H') wrhstflg = 1; /* 3 flags arg reading */
	else if (*(argv[i]+j)=='x') killflg = 1;
	else printf ("Unknown command line option: %c\n",*(argv[i]+j));
	j++;
      }
    } else printf ("Unknown command line keyword: %s\n",argv[i]);
  }
}

if (cmdflg==1) {
  printf ("-c option was specified, but no command was provided\n");
}
if (metabuff==1) {
  printf ("-m option was specified, but no metafile buffer size was provided\n");
}
/* if (g2buff==1) { */
/*   printf ("-n option was specified, but no GRIB2 cache size was provided\n"); */
/* } */
if (ipcflg) printf("\n<IPC>" );  /* delimit splash screen */

printf ("\nGrid Analysis and Display System (GrADS) Version " GRADS_VERSION "\n");
printf ("Copyright (c) 1988-2008 by Brian Doty and the \n");
printf ("Institute for Global Environment and Society (IGES)\n");
printf ("GrADS comes with ABSOLUTELY NO WARRANTY\n");
printf ("See file COPYRIGHT for more information\n\n");
/* printf ("* * * * * * * * * * * * * * * * * * * * *\n"); */
/* printf ("*    WARNING -- DEVELOPMENT VERSION     *\n"); */
/* printf ("*      MAY NOT BE FUNCTIONAL CODE       *\n"); */
/* printf ("*  NEW FEATURES MAY CHANGE AT ANY TIME  *\n"); */
/* printf ("* * * * * * * * * * * * * * * * * * * * *\n\n"); */

gacfg(0);


if (!land && !port) {
  nxtcmd (cmd,"Landscape mode? ('n' for portrait): ");
  if (cmd[0]=='n') port = 1;
}
if (port) {
  gcmn.xsiz = 8.5;
  gcmn.ysiz = 11.0;
} else {
  gcmn.xsiz = 11.0;
  gcmn.ysiz = 8.5;
}

if(xwideflg) gxwdln();

gainit();
gcmn.pfi1 = NULL;                     /* No data sets open      */
gcmn.pfid = NULL;
gcmn.fnum = 0;
gcmn.dfnum = 0;
gcmn.fseq = 10; 
gcmn.pdf1 = NULL;
gcmn.grflg = 0;
gcmn.devbck = 0;
gcmn.sdfwname = NULL;
gcmn.sdfwtype = 1;
gcmn.sdfwpad = 0;
gcmn.ncwid = -999;
gcmn.attr = NULL;
gcmn.ffile = NULL;
gcmn.fwname = NULL;
gcmn.gtifname = NULL;    /* for GeoTIFF output */
gcmn.tifname = NULL;     /* for KML output */
gcmn.kmlname = NULL;     /* for KML output */
gcmn.fwenflg = BYTEORDER;
gcmn.fwsqflg = 0;        /* default is stream */
gcmn.fwexflg = 0;        /* default is not exact -- old bad way */
gcmn.gtifflg = 1;        /* default geotiff output format is float */
if (size) gcmn.hbufsz = size;
if (g2size) gcmn.g2bufsz = g2size;

gafdef();

gagx(&gcmn);

#if !defined(__CYGWIN32__) && !defined(__GO32__)
  signal(CPULIMSIG, gasigcpu) ;  /* CPU time limit signal; just exit   -hoop */
#endif

#ifdef XLIBEMU  /* Repeat this once X windows come up on MS-DOS */

printf ("\nGrid Analysis and Display System (GrADS) Version " GRADS_VERSION "\n");
printf ("Copyright (c) 1988-2008 by Brian Doty and the \n");
printf ("Institute for Global Environment and Society (IGES)\n");
printf ("GrADS comes with ABSOLUTELY NO WARRANTY\n");
printf ("See file COPYRIGHT for more information\n\n");

gacfg(0);

#if READLINE == 1
using_history();        /* but no readline */ 
stifle_history (128);   /* will remember 128 commands */
#endif

#endif  /* XLIBEMU */

#if READLINE == 1
 if (wrhstflg && logfile != NULL) {
   printf("Command line history in %s\n",logfile);
   history_truncate_file(logfile,256); 
   read_history(logfile); /* read last 256 cmd */
 }
#endif

if (icmd) rc = gacmd(icmd,&gcmn,0);
else      rc = 0;
signal(2,gasig);  /* Trap cntrl c */

#if USEGUI == 1
if (!ipcflg) 
  gagui_main (argc, argv);   /*ams Initializes GAGUI, and if the environment
                               variable GAGUI is set it starts a GUI
                               script. Otherwise, it just returns. ams*/
#endif
if (ipcflg) printf("\n<RC> %d </RC>\n</IPC>\n",rc);

/* Main command line loop */
while (rc>-1) {

  if (killflg) return(99);

#if READLINE == 1
#if defined(MM_NEW_PROMPT) 
  char prompt[12];
  if (hstflg) {
    sprintf(prompt,"ga[%d]> ",history_length+1);
    rc=nxrdln(&cmd[0],prompt);
  }
#else
  if (hstflg) rc=nxrdln(&cmd[0],"ga-> ");
#endif
  else rc=nxtcmd(&cmd[0],"ga> ");
#else
  rc=nxtcmd(&cmd[0],"ga> ");
#endif
  
  if (rc < 0) {
    strcpy(cmd,"quit");   /* on EOF, just quit */
    printf("[EOF]\n");
  }

  if (ipcflg) printf("\n<IPC> %s", cmd );  /* echo command in IPC mode */

  gcmn.sig = 0;
  rc = gacmd(cmd,&gcmn,0);

  if (ipcflg) printf("\n<RC> %d </RC>\n</IPC>\n",rc);
}

/* All done */
gxend();

#if READLINE == 1
 if (wrhstflg) write_command_log(logfile);
#endif

exit(0);

}

/* Initialize most gacmmn values.  Values involving real page size,
   and values involving open files, are not modified   */

void gainit (void) {
gaint i;

  gcmn.wgds = &wgds;
  gcmn.wgds->fname = NULL;
  gcmn.wgds->opts = NULL;
  gcmn.hbufsz = 1000000;
  gcmn.g2bufsz = 10000000;
  gcmn.loopdim = 3;
  gcmn.csmth = 0;
  gcmn.cterp = 1;
  gcmn.cint = 0;
  gcmn.cflag = 0;
  gcmn.ccflg = 0;
  gcmn.cmin = -9.99e33;
  gcmn.cmax = 9.99e33;
  gcmn.arrflg = 0;
  gcmn.arlflg = 1;
  gcmn.ahdsiz = 0.05;
  gcmn.hemflg = -1;
  gcmn.aflag = 0;
  gcmn.axflg = 0;
  gcmn.ayflg = 0;
  gcmn.rotate = 0;
  gcmn.xflip = 0;
  gcmn.yflip = 0;
  gcmn.gridln = -9;
  gcmn.zlog = 0;
  gcmn.coslat = 0;
  gcmn.numgrd = 0;
  gcmn.gout0 = 0;
  gcmn.gout1 = 1;
  gcmn.gout1a = 0;
  gcmn.gout2a = 1;
  gcmn.gout2b = 4;
  gcmn.goutstn = 1;
  gcmn.cmark = -9;
  gcmn.grflag = 1;
  gcmn.grstyl = 5;
  gcmn.grcolr = 15;
  gcmn.blkflg = 0;
  gcmn.dignum = 0;
  gcmn.digsiz = 0.07;
  gcmn.reccol = 1;
  gcmn.recthk = 3;
  gcmn.lincol = 1;
  gcmn.linstl = 1;
  gcmn.linthk = 3;

  gcmn.mproj = 2;
  gcmn.mpdraw = 1;
  gcmn.mpflg = 0;
  gcmn.mapcol = -9; gcmn.mapstl = 1; gcmn.mapthk = 1;
  for (i=0; i<256; i++) {
    gcmn.mpcols[i] = -9;
    gcmn.mpstls[i] = 1;
    gcmn.mpthks[i] = 3;
  }
  gcmn.mpcols[0] = -1;  gcmn.mpcols[1] = -1; gcmn.mpcols[2] = -1;
  gcmn.mpdset[0] = (char *)galloc(7,"mpdset0");
  *(gcmn.mpdset[0]+0) = 'l'; 
  *(gcmn.mpdset[0]+1) = 'o';
  *(gcmn.mpdset[0]+2) = 'w'; 
  *(gcmn.mpdset[0]+3) = 'r';
  *(gcmn.mpdset[0]+4) = 'e'; 
  *(gcmn.mpdset[0]+5) = 's';
  *(gcmn.mpdset[0]+6) = '\0';
  for (i=1;i<8;i++) gcmn.mpdset[i]=NULL;

  gcmn.strcol = 1;
  gcmn.strthk = 3;
  gcmn.strjst = 0;
  gcmn.strrot = 0.0;
  gcmn.strhsz = 0.1;
  gcmn.strvsz = 0.12;
  gcmn.anncol = 1;
  gcmn.annthk = 6;
  gcmn.tlsupp = 0;
  gcmn.xlcol = 1;
  gcmn.ylcol = 1;
  gcmn.xlthck = 4;
  gcmn.ylthck = 4;
  gcmn.xlsiz = 0.11;
  gcmn.ylsiz = 0.11;
  gcmn.xlflg = 0;
  gcmn.ylflg = 0;
  gcmn.xtick = 1;
  gcmn.ytick = 1;
  gcmn.xlint = 0.0;
  gcmn.ylint = 0.0;
  gcmn.xlpos = 0.0;
  gcmn.ylpos = 0.0;
  gcmn.ylpflg = 0;
  gcmn.yllow = 0.0;
  gcmn.xlside = 0;
  gcmn.ylside = 0;
  gcmn.clsiz = 0.09;
  gcmn.clcol = -1;
  gcmn.clthck = -1;
  gcmn.stidflg = 0;
  gcmn.grdsflg = 1;
  gcmn.timelabflg = 1;
  gcmn.stnprintflg = 0;
  gcmn.fgcnt = 0;
  gcmn.barflg = 0;
  gcmn.bargap = 0;
  gcmn.barolin = 0;
  gcmn.clab = 1;
  gcmn.clskip = 1;
  gcmn.xlab = 1;
  gcmn.ylab = 1;
  gcmn.clstr = NULL;
  gcmn.xlstr = NULL;
  gcmn.ylstr = NULL;
  gcmn.xlabs = NULL;
  gcmn.ylabs = NULL;
  gcmn.dbflg = 0;
  gcmn.rainmn = 0.0;
  gcmn.rainmx = 0.0;
  gcmn.rbflg = 0;
  gcmn.miconn = 0;
  gcmn.impflg = 0;
  gcmn.impcmd = 1;
  gcmn.strmden = 5;
  gcmn.frame = 1;
  gcmn.pxsize = gcmn.xsiz;
  gcmn.pysize = gcmn.ysiz;
  gcmn.vpflag = 0;
  gcmn.xsiz1 = 0.0;
  gcmn.xsiz2 = gcmn.xsiz;
  gcmn.ysiz1 = 0.0;
  gcmn.ysiz2 = gcmn.ysiz;
  gcmn.paflg = 0;
  for (i=0; i<10; i++) gcmn.gpass[i] = 0;
  gcmn.btnfc = 1;
  gcmn.btnbc = 0;
  gcmn.btnoc = 1;
  gcmn.btnoc2 = 1;
  gcmn.btnftc = 1;
  gcmn.btnbtc = 0;
  gcmn.btnotc = 1;
  gcmn.btnotc2 = 1;
  gcmn.btnthk = 3;
  gcmn.dlgpc = -1;
  gcmn.dlgfc = -1;
  gcmn.dlgbc = -1;
  gcmn.dlgoc = -1;
  gcmn.dlgth = 3;
  gcmn.dlgnu = 0;
  for (i=0; i<15; i++) gcmn.drvals[i] = 1;
  gcmn.drvals[1] = 0; gcmn.drvals[5] = 0;
  gcmn.drvals[9] = 0;
  gcmn.drvals[14] = 1;
  gcmn.sig = 0;
  gcmn.lfc1 = 2;
  gcmn.lfc2 = 3;
  gcmn.wxcols[0] = 2; gcmn.wxcols[1] = 10; gcmn.wxcols[2] = 11;
  gcmn.wxcols[3] = 7; gcmn.wxcols[4] = 15;
  gcmn.wxopt = 1;
  for (i=0; i<32; i++) gcmn.clct[i] = NULL;
  gcmn.ptflg = 0;
  gcmn.ptopt = 1;
  gcmn.ptden = 5;
  gcmn.ptang = 0;
  gcmn.statflg = 0;
  gcmn.prstr = NULL;  gcmn.prlnum = 8; 
  gcmn.prbnum = 1; gcmn.prudef = 0;
  gcmn.dwrnflg = 1;
  gcmn.xexflg = 0; gcmn.yexflg = 0;

  mfcmn.cal365=-999;
  mfcmn.warnflg=2;
  mfcmn.winx=-999;      /* Window x  */         
  mfcmn.winy=-999;      /* Window y */     
  mfcmn.winw=0;         /* Window width */ 
  mfcmn.winh=0;         /* Window height */ 
  mfcmn.winb=0;         /* Window border width */

}

void gasig(gaint i) {
  if (gcmn.sig) exit(0);
  gcmn.sig = 1;
}

gaint gaqsig (void) {
  return(gcmn.sig);
}

#if READLINE == 1
/* write command history to log file */
void write_command_log(char *logfile) {
   char QuitLabel[60];
   time_t thetime;
   struct tm *ltime;
   if ((thetime=time(NULL))!=0) {  
      ltime=localtime(&thetime);
      strftime(QuitLabel,59,"quit # (End of session: %d%h%Y, %T)",ltime);
      remove_history(where_history());
      add_history(QuitLabel);
   }
   write_history(logfile);  
   return;
}
#endif

/* output command line options */

void command_line_help (void) {
printf("GrADS Version " GRADS_VERSION " --- " GRADS_DESC "\n\n");
printf("Command line options: \n\n");
printf("          -help   Just this help\n");
printf("          -c      run this command (e.g., a script)\n");
printf("          -l      start in landscape mode\n");
printf("          -p      start in portrait mode\n");
printf("          -b      run GrADS in batch mode (no graphics window)\n");
printf("          -g LLLLxHHHH+(-)XXXX+(-)YYYY  set size of graphics window (like X windows)\n");
printf("                LLLL -- length of box in pixels (x side)\n");
printf("                HHHH -- height of box in pixels (y side)\n");
printf("                XXXX -- starting pixel point in x (0,0 is upper lefthand corner)\n");
printf("                YYYY -- starting pixel point in y\n");
printf("          -m NNN  Set metafile buffer size to NNN (must be an integer)\n");
printf("          -n NNN  Set GRIB2 cache size to NNN (must be an integer)\n");
#if READLINE == 1
printf("          -E      turn OFF command line editing\n");
printf("          -u      unbuffer stdout/stderr, turn OFF readline\n");
printf("          -H      turn ON command line log (reading and writing).\n");
#endif
printf("          -W      use X server wide lines (faster) vice s/w (better)\n");
printf("\n   Example:\n");
printf("     grads -lc \"run myscript.gs\" -g 640x480-0+100\n\n");
printf("     runs GrADS in landscape mode with a window 640x480 pixels\n");
printf("     0 pixels from the right and 100 down from the top\n");

}

/* For CPU time limit signal */
void gasigcpu(gaint i) {  
    exit(1) ;
}
