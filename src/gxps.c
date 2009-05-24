/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */

/* 
 * Include ./configure's header file
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <math.h>


int help=0;
void command_line_help(void) ;

char * getflt (char *, float *);

char buff[132];
int pnt;
FILE *infile;
FILE *outfile;
int rotflg;

void mycln (char *, int *);
void chout (char *, int);
void chend (void);
int nxtcmd (char *, char *);
void blkbck (void);
void xycnv (short, short, float *, float *);
void xycnva4 (short, short, float *, float *);

float grey[100];
float red[100],green[100],blue[100];

/* These values should match the RGB values in gxX.c and gxhpng.c and the documentation */
float reds[16]   = {1.0, 0.0, 0.98, 0.00, 0.12, 0.00, 0.94, 0.90, 0.94, 0.63, 0.63, 0.00, 0.90, 0.00, 0.51, 0.67};
float greens[16] = {1.0, 0.0, 0.24, 0.86, 0.24, 0.78, 0.00, 0.86, 0.51, 0.00, 0.90, 0.63, 0.69, 0.82, 0.00, 0.67};
float blues[16]  = {1.0, 0.0, 0.24, 0.00, 1.00, 0.78, 0.51, 0.19, 0.16, 0.78, 0.19, 1.00, 0.18, 0.55, 0.86, 0.67};
float greys[16]  = {1.0, 0.0, 0.16, 0.46, 0.70, 0.58, 0.10, 0.34, 0.22, 0.82, 0.40, 0.64, 0.28, 0.52, 0.76, 0.50};

char *lwdesc[12] = {" 0.001 w",
                    " 0.006 w",
                    " 0.009 w",
                    " 0.012 w",
                    " 0.015 w",
                    " 0.018 w",
                    " 0.021 w",
                    " 0.024 w",
                    " 0.027 w",
                    " 0.030 w",
                    " 0.033 w",
                    " 0.036 w"};

static int rflag, cflag, dflag;
static float bwide;

main (int argc, char *argv[])  {
short opts[4];
int cmd, i, j;
char ch[20];
int len, cont, iflag, oflag;
int lcolor,lwide,sflag,fflag,fcnt,ccnt,filflg,scnt,a4flag,bdflag;
float xlo,xhi,ylo,yhi,xpos,ypos,xsiz,ysiz;
char in[100],out[100],*ifi,*ofi,ctld[2];

  /* Initialize */

  ctld[0] = (char)4;
  ctld[1] = '\n';
  for (i=0; i<100; i++) {
    grey[i] = -999.0;
    red[i] = -999.0;
    green[i] = -999.0;
    blue[i] = -999.0;
  }
  for (i=0; i<16; i++) {
    grey[i] = greys[i];
    red[i] = reds[i];
    green[i] = greens[i];
    blue[i] = blues[i];
  }

  /* Parse command line arguments */

  i = 1;
  iflag = 0; oflag = 0;
  cflag = 0; rflag = 0; dflag = 1;
  a4flag = 0;
  bwide = 0.5;
  bdflag=0;

  ifi = NULL; ofi = NULL;
  while (i<argc) {
    if (*(argv[i])=='-' && *(argv[i]+1)=='h' && *(argv[i]+2)=='e' && *(argv[i]+3)=='l' && *(argv[i]+4)=='p' ) {
      command_line_help();
      return(0);
    } else if (*(argv[i])=='-') {
      j = 1;
      while (*(argv[i]+j)) {
        if (*(argv[i]+j)=='i') iflag = 1;
        else if (*(argv[i]+j)=='o') oflag = 1;
        else if (*(argv[i]+j)=='c') cflag = 1;
        else if (*(argv[i]+j)=='r') rflag = 1;
        else if (*(argv[i]+j)=='a') a4flag = 1;
        else if (*(argv[i]+j)=='d') dflag = 0;
        else if (*(argv[i]+j)=='b') bdflag = 1;
        else printf ("Unknown flag %c; ignored.\n",argv[i]+j);
        j++;
      }
    } else {
      if (iflag) {ifi = argv[i]; iflag = 0;}
      else if (oflag) {ofi = argv[i]; oflag = 0;}
      else if (bdflag)  {
        if (getflt(argv[i],&bwide)==NULL) {
          printf ("Invalid border width\n");
          bwide = 0.5;
        }
      }
      else printf ("Unknown argument: %s.  Ignored.\n",argv[i]);
    }
    i++;
  }
  if (ifi==NULL) {
    command_line_help();
    nxtcmd (in,"Enter input file: ");
    ifi = in;
  }
  if (ofi==NULL) {
    nxtcmd (out,"Enter output file: ");
    ofi = out;
  }

  if (rflag) {
    grey[0] = greys[1]; grey[1] = greys[0];
    red[0] = reds[1]; red[1] = reds[0];
    green[0] = greens[1]; green[1] = greens[0];
    blue[0] = blues[1]; blue[1] = blues[0];
  }

  /* Open files */

  infile = fopen(ifi,"rb");
  if (infile == NULL) {
    printf ("Input file does not exist: \n");
    printf ("  %s\n",ifi);
    return(1);
  }
  outfile = fopen(ofi,"wb");
  if (outfile==NULL) {
    printf ("Error opening output file: \n");
    printf ("  %s\n",ofi);
    return(1);
  }

  /* Translate the file */

  pnt = 0;
  fcnt = 0;

  ccnt=0; 
  cont = 1;
  while (cont) {
    fread (opts , sizeof(short), 1, infile);
    cmd = opts[0];
    ccnt++;
    /* End of plotting */

    if (cmd==-9) {
      cont = 0;
      chend();
      if (fflag) {fwrite ("showpage\n",1,9,outfile); fcnt++;}
      if (dflag) fwrite (ctld,1,2,outfile);
      printf ("Number of frames = %i\n",fcnt);
      fclose (outfile);
    }

    /* Start of plotting */

    else if (cmd==-1) {
      fread ((char *)opts, sizeof(short), 2, infile);
      fwrite("%!\n",1,3,outfile);
      fwrite("initgraphics 1 setlinecap 1 setlinejoin\n",1,40,outfile);
      fwrite("72 72 scale\n",1,12,outfile);
      fwrite("/m {moveto} def\n",1,16,outfile);
      fwrite("/d {lineto} def\n",1,16,outfile);
      fwrite("/c {setrgbcolor} def\n",1,21,outfile);
      fwrite("/g {setgray} def\n",1,17,outfile);
      fwrite("/n {newpath} def\n",1,17,outfile);
      fwrite("/w {setlinewidth} def\n",1,22,outfile);
      fwrite("/s {stroke} def\n",1,16,outfile);
      fwrite("/f {fill} def\n",1,14,outfile);
      blkbck();
      chout(" 0 w",4);
      xsiz = opts[0];
      ysiz = opts[1];
      rotflg=0;
      if (ysiz>xsiz) rotflg=1;
      fflag = 1;
      sflag = 0;
    }

    /* New Page */

    else if (cmd==-2) {
      if (sflag) {chout(" s",2); sflag=0;}
      chend();
      fwrite ("gsave showpage\n",1,15,outfile);
      fcnt++;
      fflag = 0;
    }

    /* Set color */

    else if (cmd==-3) {
      if (!fflag) {
        chout(" grestore",9);
        blkbck();
      }
      if (sflag) {chout(" s",2); sflag=0;}
      fread ((char *)opts, sizeof(short), 1, infile);
      lcolor = opts[0];
      if (lcolor<0) lcolor=0;
      if (lcolor>99) lcolor=99;
      if (cflag) {
        if (red[lcolor]<-900.0) lcolor=15;
        sprintf(ch," %.5g %.5g %.5g c",red[lcolor],green[lcolor],blue[lcolor]);
      } else {
        if (lcolor>0) sprintf(ch," %.5g g",grey[1]);
        else sprintf(ch," %.5g g",grey[0]);
      }
      mycln(ch,&len);
      chout(ch,len);
      fflag = 1;
    }

    /* Set line width */

    else if (cmd==-4) {
      if (!fflag) {
        chout(" grestore",9);
        blkbck();
      }
      if (sflag) {chout(" s",2); sflag=0;}
      fread ((char *)opts, sizeof(short), 2, infile);
      i = opts[0];
      if (i>12) i=12;
      if (i<1) i=1;
      chout(lwdesc[i-1],8);
      lwide = i;
      fflag = 1;
    }

    /* Define new color (mapped into grey scale via green intensity) */

    else if (cmd==-5){
      fread ((char *)opts, sizeof(short), 4, infile);
      i = opts[0];
      if (i>15 && i<100) {
        if (cflag) {
          red[i] = ((float)opts[1])/255.0;
          green[i] = ((float)opts[2])/255.0;
          blue[i] = ((float)opts[3])/255.0;
          if (red[i]<0.0) red[i]=0.0;
          if (red[i]>1.0) red[i]=1.0;
          if (green[i]<0.0) green[i]=0.0;
          if (green[i]>1.0) green[i]=1.0;
          if (blue[i]<0.0) blue[i]=0.0;
          if (blue[i]>1.0) blue[i]=1.0;
        } else {
          grey[i] = ((float)(opts[2]))/255.0;
          if (grey[i]<0.0) grey[i]=0.0;
          if (grey[i]>1.0) grey[i]=1.0;
          grey[i] = 1.0 - grey[i];
        }
      }
    }

    /* Rectangle fill */

    else if (cmd==-6){
      if (!fflag) {
        chout(" grestore",9);
        blkbck();
      }
      if (sflag) {chout(" s",2); sflag=0;}
      fread ((char *)opts, sizeof(short), 4, infile);

      if(a4flag) {
	xycnva4 (opts[0],opts[2],&xlo,&ylo);
	xycnva4 (opts[1],opts[3],&xhi,&yhi);
      } else { 
	xycnv (opts[0],opts[2],&xlo,&ylo);
	xycnv (opts[1],opts[3],&xhi,&yhi);
      }

      if (!cflag) {
        if (grey[lcolor]<-100.0) sprintf(ch," %.5g g",grey[15]);
        else sprintf(ch," %.5g g",grey[lcolor]);
        mycln(ch,&len);
        chout(ch,len);
      }
      chout(" n",2);
      sprintf(ch," %.5g %.5g m",xlo,ylo);
      mycln(ch,&len);
      chout(ch,len);
      sprintf(ch," %.5g %.5g d",xhi,ylo);
      mycln(ch,&len);
      chout(ch,len);
      sprintf(ch," %.5g %.5g d",xhi,yhi);
      mycln(ch,&len);
      chout(ch,len);
      sprintf(ch," %.5g %.5g d",xlo,yhi);
      mycln(ch,&len);
      chout(ch,len);
      sprintf(ch," %.5g %.5g d",xlo,ylo);
      mycln(ch,&len);
      chout(ch,len);
      chout(" f",2);
      if (!cflag) {
        if (lcolor>0) sprintf(ch," %.5g g",grey[1]);
        else sprintf(ch," %.5g g",grey[0]);
        mycln(ch,&len);
        chout(ch,len);
      }
      fflag = 1;
    }

    /* Start fill */

    else if (cmd==-7){
      fread ((char *)opts, sizeof(short), 1, infile);
      if (!cflag) {
        if (grey[lcolor]<-100.0) sprintf(ch," %.5g g",grey[15]);
        else sprintf(ch," %.5g g",grey[lcolor]);
        mycln(ch,&len);
        chout(ch,len);
      }
      filflg = 1;
      fflag = 1;
    }

    /* End fill */

    else if (cmd==-8){
      if (sflag) {chout(" f",2); sflag=0;}
      if (!cflag) {
        if (lcolor>0) sprintf(ch," %.5g g",grey[1]);
        else sprintf(ch," %.5g g",grey[0]);
        mycln(ch,&len);
        chout(ch,len);
      }
      filflg = 0;
      fflag = 1;
    }

    /* Move to */

    else if (cmd==-10){
      if (!fflag) {
        chout(" grestore",9);
        blkbck();
      }
      if (sflag) {chout(" s",2); sflag=0; scnt = 0;}
      fread ((char *)opts, sizeof(short), 2, infile);
      if(a4flag) {
	xycnva4 (opts[0],opts[1],&xpos,&ypos);
      } else { 
	xycnv (opts[0],opts[1],&xpos,&ypos);
      }

      fflag = 1;
    }

    /* Draw to */

    else if (cmd==-11){
      if (!fflag) {
        chout(" grestore",9);
        blkbck();
      }
      fread (opts, sizeof(short), 2, infile);
      if (!sflag) {
        chout (" n",2);         
        sprintf(ch," %.5g %.5g m",xpos,ypos);
        mycln(ch,&len);
        chout(ch,len);
        scnt = 0;
      }

      if(a4flag) {
	xycnva4 (opts[0],opts[1],&xpos,&ypos);
      } else { 
	xycnv (opts[0],opts[1],&xpos,&ypos);
      }

      if (scnt>511 && !filflg) {    /* Keep vector counts small */
        sprintf(ch," %.5g %.5g d",xpos,ypos);
        mycln(ch,&len);
        chout(ch,len);
        chout (" s n",4);
        sprintf(ch," %.5g %.5g m",xpos,ypos);
        mycln(ch,&len);
        chout(ch,len);
        scnt = 0;
      }
      sprintf(ch," %.5g %.5g d",xpos,ypos);
      mycln(ch,&len);
      chout(ch,len);
      sflag = 1;
      scnt++;
      fflag = 1;
    }
    else if (cmd==-20) {       /* Draw button -- ignore */
      fread ((char *)opts, sizeof(short), 1, infile);
    }
    else {
      printf ("Invalid command found %i \n",cmd);
      return(1);
    }
  }
  return(0);
}

void blkbck () {
  if (!rflag) return;
  chout(" 0 0 0 c",8);
  chout(" n 0 0 m 8.5 0 d",16);
  chout(" 8.5 11 d",9);
  chout(" 0 11 d",7);
  chout(" 0 0 d f",8);
}

void mycln (char *ch, int *len) {
int i,j,flag,cnt;
  i = 0;
  j = 0;
  flag = 0;
  cnt = 0;
  while (ch[j]) {
    ch[i] = ch[j];
    if (flag) {
      if (ch[j]<'0'||ch[j]>'9') {flag = 0; cnt=0;}
      else cnt++;
    }
    if (ch[j]=='.') flag = 1;
    if (flag&&cnt>3) j++;
    else {i++; j++;}
  }
  *len = i;
}

void chout(char *ch, int len) {
int i;

  if (len+pnt>130) {
    buff[pnt] = '\n';
    pnt++;
    fwrite (buff,1,pnt,outfile);
    pnt = 0;
  }
  for (i=0; i<len; i++) {
    buff[pnt] = *(ch+i);
    pnt++;
  }
}

void chend (void) {

  buff[pnt] = '\n';
  pnt++;
  fwrite (buff,1,pnt,outfile);
  pnt = 0;
}

int nxtcmd (char *cmd, char *prompt) {
int past,cnt;

  printf ("%s ",prompt);
  past = 0;
  cnt = 0;
  while (1) {
    *cmd = getchar();
    if (*cmd == EOF) return (-1);
    if (*cmd == '\n') {
      *cmd = '\0';
      return (cnt);
    }
    if (past || *cmd != ' ') {
      cmd++; cnt++; past = 1;
    }
  }
}

void xycnv (short ix, short iy, float *x, float *y) {

  if (rotflg) {
    *x = ((float)ix)/1000.0;
    *y = ((float)iy)/1000.0;
  } else {
    *x = 8.5 - ((float)iy)/1000.0;
    *y = ((float)ix)*0.001;
  }
  *x = bwide + *x*((8.5-bwide*2.0)/8.5);
  *y = bwide + *y*((11.0-bwide*2.0)/11.0);
}

/* To Mike
     Now I finished the adjustment and test for A4 paper.
                                              Thank you ! 
                                    1995.10.19   H.Koide */

void xycnva4 (short ix, short iy, float *x, float *y) {

  if (rotflg) {
    *x = ((float)ix)/1000.0;
    *y = ((float)iy)/1025.6;
  } else {
    *x = 8.5 - ((float)iy)/1000.0;
    *y = ((float)ix)/1025.6;
  }
  *x = bwide + *x*((8.5-bwide*2.0)/8.5);
  *y = bwide + *y*((11.0-bwide*2.0)/11.0);
}

/* Converts a character string to a floating point number */
char *getflt (char *ch, float *val) {
  char * pos;

  *val = (float)strtod(ch, &pos);
  if (pos==ch) {
    return NULL;
  } else {
    return pos;
  }
}


void command_line_help(void) {
/*--- 
  output command line options 
---*/

printf("gxps for GrADS Version " GRADS_VERSION "\n\n");
printf("Convert GrADS meta files (from print command in grads) to postscript (level 1)\n\n");
printf("Command line options: \n\n");
printf("          -help   Just this help\n");
printf("          -i      input GrADS meta file\n");
printf("          -o      output postscript file\n");
printf("          -c      color output (default is black and white)\n");
printf("          -r      reverse background (typically black) default is white background\n");
printf("          -b      border width in inches (default is 0.5)\n");
printf("          -a      create ps suitable for A4 printers\n");
printf("          -d      do NOT add ctrl-d to end of ps file (added for some HP printers, generally ignored)");
printf("   Example:\n\n");
printf("   gxps -b 0.10 -c -r -i myplot.gm -o myplot.ps\n\n");
printf("   makes a color ps file with a black background and a border width plot of 0.10 inches\n\n");

printf("\n");

}
