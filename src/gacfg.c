/*  Copyright (C) 1988-2008 by Brian Doty and the 
    Institute of Global Environment and Society (IGES).  
    See file COPYRIGHT for more information.   */


/* file: gacfg.c
 *
 *   Prints the configuration options of this build of GrADS.
 *   This function is invoked at startup and with 'q config'.
 *
 *   REVISION HISTORY:
 *
 *   09sep97   da Silva   Initial code.
 *   12oct97   da Silva   Small revisions, use of gaprnt().
 *   15apr98   da Silva    Added BUILDINFO stuff, made gacfg() void. 
 *   24jun02   K Komine   Added 64-bit mode . 
 *
 *   --
 *   (c) 1997 by Arlindo da Silva
 *
 *   Permission is granted to any individual or institution to use,
 *   copy, or redistribute this software so long as it is not sold for
 *   profit, and provided this notice is retained. 
 *
 */

/* Include ./configure's header file */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include "buildinfo.h"

/* #if READLINE==1 */
/* #include <readline/readline.h> */
/* #endif */

/* #if GXPNG==1 */
/* #include <png.h> */
/* #include <gd.h> */
/* #endif */

#if GRIB2==1
#include <grib2.h>
/* #include <jas_config.h> */
#endif

#if USEHDF==1
#include <mfhdf.h>
#endif

/*
 * gacfg() - Prints several configuration parameters. 
 *
 *           verbose = 0   only config string
 *                   = 1   config string + verbose description
 *                   > 1   no screen display.
 */
void gacfg(int verbose) {
char cmd[1024];
#if USEHDF==1
uint32 majorv=0,minorv=0,release=0;
char hdfverstr[1024];
#endif

 sprintf(cmd,"Config: v%s",GRADS_VERSION);
#if BYTEORDER==1
 strcat(cmd," big-endian");
#else
 strcat(cmd," little-endian");
#endif
#if READLINE==1
 strcat(cmd," readline");
#endif
#if GXPNG==1
 strcat(cmd," printim");
#endif
#if GRIB2==1
 strcat(cmd," grib2");
#endif
#if USENETCDF==1
 strcat(cmd," netcdf");
#endif
#if USEHDF==1
 strcat(cmd," hdf4-sds");
#endif
#if USEDAP==1
 strcat(cmd," opendap-grids");
#endif
#if USEGADAP==1
 strcat(cmd,",stn");
#endif
#if USEGUI==1
 strcat(cmd," athena");
#endif
#if GEOTIFF==1
 strcat(cmd," geotiff");
#endif
 strcat(cmd,"\n");
 gaprnt(verbose,cmd);
 
 if (verbose==0) {
   printf ("Issue 'q config' command for more information.\n");
   return;
 }
 
 gaprnt (verbose, "Grid Analysis and Display System (GrADS) Version " GRADS_VERSION "\n");
 gaprnt (verbose, "Copyright (c) 1988-2008 by Brian Doty and the\n");
 gaprnt (verbose, "Institute for Global Environment and Society (IGES) \n");
 gaprnt (verbose, "This program is distributed WITHOUT ANY WARRANTY \n");
 gaprnt (verbose, "See file COPYRIGHT for more information. \n\n");
 
 gaprnt (verbose, buildinfo );
 
 gaprnt(verbose,"\n\nThis version of GrADS has been configured with the following options:\n");
 
#if BYTEORDER==1
   gaprnt(verbose,"  o Built on a BIG ENDIAN machine\n");
#else 
   gaprnt(verbose,"  o Built on a LITTLE ENDIAN machine\n");
#endif

#if READLINE==1
   gaprnt(verbose,"  o Command line editing ENABLED \n");
   gaprnt(verbose,"      http://tiswww.case.edu/php/chet/readline/rltop.html \n");
/*    sprintf(cmd,   "      readline-%d.%d \n",RL_VERSION_MAJOR,RL_VERSION_MINOR); */
/*    gaprnt(verbose,cmd); */
#else
   gaprnt(verbose,"  o Command line editing DISABLED\n");
#endif

#if GXPNG==1
   gaprnt(verbose,"  o printim command for image output ENABLED \n");
   gaprnt(verbose,"      http://www.zlib.net \n");
/*    sprintf(cmd,   "      zlib-%s  \n",zlibVersion()); */
/*    gaprnt(verbose,cmd); */
   gaprnt(verbose,"      http://www.libpng.org/pub/png/libpng.html \n");
/*    sprintf(cmd,   "      libpng-%s  \n",PNG_LIBPNG_VER_STRING); */
/*    gaprnt(verbose,cmd); */
   gaprnt(verbose,"      http://www.libgd.org/Main_Page \n");
/*    sprintf(cmd,   "      gd-%d.%d.%d  \n",GD_MAJOR_VERSION,GD_MINOR_VERSION,GD_RELEASE_VERSION); */
/*    gaprnt(verbose,cmd); */
#else 
   gaprnt(verbose,"  o printim command DISABLED\n");
#endif

#if GRIB2==1
   gaprnt(verbose,"  o GRIB2 interface ENABLED \n");
   gaprnt(verbose,"      http://www.ijg.org \n");
/*    sprintf(cmd,   "      jpeg-%s \n",JVERSION); */
/*    gaprnt(verbose,cmd); */
   gaprnt(verbose,"      http://www.ece.uvic.ca/~mdadams/jasper \n");
/*    sprintf(cmd,   "      jasper-%s  \n",JAS_VERSION); */
/*    gaprnt(verbose,cmd); */
   gaprnt(verbose,"      http://www.nco.ncep.noaa.gov/pmb/codes/GRIB2 \n");
   sprintf(cmd,   "      %s  \n",G2_VERSION);
   gaprnt(verbose,cmd);
#else 
   gaprnt(verbose,"  o GRIB2 interface DISABLED\n");
#endif
 
#if USENETCDF==1
   gaprnt(verbose,"  o NetCDF interface ENABLED \n");
#if USEDAP==1
   gaprnt(verbose,"      http://www.opendap.org \n");
   sprintf(cmd,   "      libnc-dap %s  \n",nc_inq_libvers());
   gaprnt(verbose,cmd);
#else
   gaprnt(verbose,"      http://www.unidata.ucar.edu/software/netcdf  \n");
   sprintf(cmd,   "      netcdf %s  \n",nc_inq_libvers());
   gaprnt(verbose,cmd);
#endif
#else 
   gaprnt(verbose,"  o NetCDF interface DISABLED\n");
#endif
 
#if USEHDF==1
   gaprnt(verbose,"  o NCSA HDF interface ENABLED \n");
   gaprnt(verbose,"      http://hdf.ncsa.uiuc.edu \n");
   Hgetlibversion(&majorv,&minorv,&release,hdfverstr);
   sprintf(cmd,   "      HDF%d.%dr%d \n",majorv,minorv,release);
   gaprnt(verbose,cmd);
#else 
   gaprnt(verbose,"  o NCSA HDF interface DISABLED\n");
#endif 

#if USEGUI==1
   gaprnt(verbose,"  o Athena Widget GUI ENABLED\n");
#else
   gaprnt(verbose,"  o Athena Widget GUI DISABLED\n");
#endif
 
#if USEDAP==1
   gaprnt(verbose,"  o OPeNDAP gridded data interface ENABLED\n");
   gaprnt(verbose,"      http://www.opendap.org \n");
   sprintf(cmd,   "      libdap %s \n",libdap_version());
   gaprnt(verbose,cmd);
#else
   gaprnt(verbose,"  o OPeNDAP gridded data interface DISABLED\n");
#endif

#if USEGADAP==1
   gaprnt(verbose,"  o OPeNDAP station data interface ENABLED\n");
   gaprnt(verbose,"      http://iges.org/grads/gadoc/supplibs.html \n");
   sprintf(cmd,   "      %s  \n", libgadap_version());
   gaprnt(verbose,cmd);
#else
   gaprnt(verbose,"  o OPeNDAP station data interface DISABLED\n");
#endif

#if GEOTIFF==1
   gaprnt(verbose,"  o GeoTIFF and KML output ENABLED\n");
   gaprnt(verbose,"      http://www.libtiff.org \n");
   gaprnt(verbose,"      http://geotiff.osgeo.org \n");
#else
   gaprnt(verbose,"  o GeoTIFF and KML output DISABLED\n");
#endif
 
 
 gaprnt(verbose,"\nFor additional information please consult http://iges.org/grads\n\n");
 
}
