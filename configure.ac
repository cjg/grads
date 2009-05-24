dnl
dnl configure.ac: 
dnl
dnl  Source file for GrADS auto-configuration script. 
dnl  Run autoreconf to generate a configure script from this file
dnl  See acinclude.m4 for definitions of the GA_xxx macros   

# The version number
# below is the "master" version that will appear in all code, etc.
AC_INIT(GrADS, [2.0.a5], [jma@iges.org])

# Require autoconf 2.52 (comes with Mac OS X 10.2) or newer
AC_PREREQ(2.52) 

# Supporting scripts are in etc/
AC_CONFIG_AUX_DIR(etc)

# Require automake 1.6 (comes with Mac OS X 10.2) or newer,
# don't #define PACKAGE and VERSION,
# disable dependency checking
AM_INIT_AUTOMAKE([1.6 no-define no-dependencies])

AM_CONFIG_HEADER([src/config.h])

# The following macro prevents annoying interactions between CVS and 
# automake, which result in make attempting to invoke autotools 
# even when the configure script is up to date
AM_MAINTAINER_MODE 

# binaries go to ./bin by default, rather than /usr/bin. 
AC_PREFIX_DEFAULT(`pwd`)

# ----------------------------------------------------
#  Additional arguments for the configure script
# ----------------------------------------------------

AC_ARG_WITH(readline, [AC_HELP_STRING([--with-readline],
                                      [command line editing])])
AC_ARG_WITH(printim,  [AC_HELP_STRING([--with-printim],
                                      [image output])])
AC_ARG_WITH(grib2,    [AC_HELP_STRING([--with-grib2],
                                      [GRIB2 data format])])
AC_ARG_WITH(sdf,      [AC_HELP_STRING([--with-sdf],
                                      [self-describing formats (HDF4,NetCDF,OPeNDAP)])])
AC_ARG_WITH(geotiff,  [AC_HELP_STRING([--with-geotiff],
                                      [geotiff output])])
AC_ARG_WITH(dap,      [AC_HELP_STRING([--with-dap],
                                      [extra gradsdap executable, OPeNDAP-enabled])])
AC_ARG_WITH(gui,      [AC_HELP_STRING([--with-gui],
                                      [Athena X11 widget-based GUI])])
AC_ARG_WITH(xlibemu,  [AC_HELP_STRING([--with-xlibemu],
          		              [Use X11 Emulation library (Windows only)])])

# ----------------------------------------------------
# Checks for programs
# ----------------------------------------------------

echo
echo "Checking for programs"
echo "------------------"
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
echo

echo "Setting host specific options"
echo "-------------------------"

# ----------------------------------------------------
# Get host platform
# ----------------------------------------------------

AC_CANONICAL_HOST()

# ----------------------------------------------------
# Set host specific options
# ----------------------------------------------------

echo Host machine:   "$host"
case "$host" in 

  alpha-*-osf*)
    # Option needed for IEEE floating points
    CFLAGS="$(CFLAGS) -ieee"
  ;;

  i*86-pc-linux-*)
    CFLAGS="$CFLAGS -rdynamic"
  ;;

  x86_64-*-linux-*)
    CFLAGS="$CFLAGS -rdynamic"
  ;;

  ia64-*-linux-*)
    CFLAGS="$CFLAGS -rdynamic"
  ;;

  sparc-sun-solaris*)
    is_solaris="yes"
dnl checked in AC_PATH_XTRA in X_PRE_LIBS
    host_ldadd="-lsocket -lnsl -lw"
dnl    guilibadd="-lSM -lICE"
  ;;

  *-pc-cygwin)
	AC_CHECK_LIB(rpclib,svc_register,, AC_MSG_ERROR([Fatal: Sun RPC library not found]))
	host_ldadd="-lrpclib"
	exe_ext=".exe"
    AC_SUBST(host_ldadd)
    if test "$with_xlibemu" != "no" ; then
      echo "Check for X emulation libraries..."
      AC_CHECK_LIB(grx20, main, [use_xlibemu=yes])
      if test "$use_xlibemu" = "yes" ; then
       	xlibemu_libs="-lgrx20 -mwindows"
	AC_SUBST(xlibemu_libs)
      fi       
    fi    
  ;;

  *darwin*)
    is_darwin="yes"
  ;;

  cray-*-*)
    is_cray="yes"
  ;;

  *-hpux11*)
     is_hpux11="yes"
  ;;

esac

if test is_cray!="yes" ; then
  AC_DEFINE(GRADS_CRAY, 0, [Machine is NOT a Cray])
else
  AC_DEFINE(GRADS_CRAY, 1, [Machine is a Cray])
fi

if test is_hpux11!="yes" ; then
  AC_DEFINE(GRADS_HP64, 0, [Machine is NOT a 64-bit HP])
else
  AC_DEFINE(GRADS_HP64, 1, [Machine is 64-bit HP])
fi

# ----------------------------------------------------
# Check for basic libraries.
# ----------------------------------------------------

echo
echo "Checking system libraries, headers, and compiler features"
echo "-----------------------------------------"

AC_CHECK_LIB(m,cos,, AC_MSG_ERROR([Fatal: Math library not found]))
AC_PATH_XTRA

echo

# ----------------------------------------------------
# Check for basic header files.
# ----------------------------------------------------

AC_HEADER_STDC
AC_CHECK_HEADERS(malloc.h sys/file.h unistd.h)
echo

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl joew - In autoconf 2.52, AC_C_BIGENDIAN seems to ignore its
dnl parameters, so I test the internal variable that it sets.  Note
dnl that AC_BIG_ENDIAN defines its own macro, WORDS_BIGENDIAN, so
dnl really it would make most sense to replace the instances of
dnl BYTEORDER in the GrADS source; but that might cause confusion
dnl among longtime users.
AC_C_BIGENDIAN
if test $ac_cv_c_bigendian = "yes" ; then
   AC_DEFINE(BYTEORDER,1,[BIG_ENDIAN machine])
else
   AC_DEFINE(BYTEORDER,0,[little_endian machine])
fi

dnl The following tests all check for functions and typedefs that are
dnl required for GrADS, but 1) I don't think we support any platforms
dnl where they fail, and 2) if they do, there is no backup plan
dnl anyway. So I figure we might as well speed the script up a bit by
dnl commenting them out, unless/until there is some actual handling
dnl for failure.

dnl AC_TYPE_SIZE_T AC_STRUCT_TM

dnl Checks for library functions.
dnl AC_TYPE_SIGNAL
dnl AC_FUNC_STRFTIME
dnl AC_FUNC_VPRINTF
dnl AC_CHECK_FUNCS(strdup strstr strtod strtol)

dnl In this case we do have an alternative - valprs()
AC_CHECK_FUNCS(strtod)

dnl Check for and set up large file support via fseeko()
AC_TYPE_OFF_T
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

# ----------------------------------------------------
# Check for optional feature support
# ----------------------------------------------------

echo
echo Checking for optional feature support
echo ------------------------------------


# ----------------------------------------------------
# Set supplibs directory name
# ----------------------------------------------------

GA_SET_SUPPLIBS([. ..])


AC_ARG_ENABLE([dyn-supplibs], 
[  --enable-dyn-supplibs   Use dynamic system libraries to support optional features when
                          required libs are not present in "supplibs" directory. [[default=yes]] 
  --disable-dyn-supplibs  Do not use system libraries; look only in GrADS "supplibs" directory.
                          Use this option to maximize binary portability. 
],, 
[enableval=yes])
case "${enableval}" in
  yes)
    ga_dyn_supplibs=yes
  ;;
  no)
    ga_dyn_supplibs=no
  ;;
esac


AC_MSG_CHECKING([whether to use dynamic linking])
if test "$ga_dyn_supplibs" = "yes" ; then
   AC_MSG_RESULT([yes (Warning: binaries may not be suitable for distribution)])
else
   AC_MSG_RESULT([no])
fi


AC_ARG_VAR(SUPPLIBS, 
           [Custom path (must be absolute) to a GrADS supplib distribution])



# ----------------------------------------------------
# Check for optional feature support
# ----------------------------------------------------

use_gui=no
use_freq=no
use_xaw_xlibs=no
use_readline=no
have_tiff=no
use_geotiff=no
have_zlib=no
have_libpng=no
have_jpeg=no
use_printim=no
have_grib2=no
have_udunits=no
use_hdf=no
use_nc=no
use_dap=no
use_gadap=no

# look for libraries to support gui interface
echo
if test "$with_gui" != "no" ; then
  echo "Checking for libraries to support GUI interface ..."
  GA_CHECK_GUI([
    use_gui=yes
    GA_SET_LIB_VAR(gui_libs, [sx neXtaw])
    if test z"$gui_libs_Xext" != 'z'; then
      gui_libs="$gui_libs -lXext"
    fi
    gui_libs="$gui_libs -lXpm -lXmu -lXt"
  ])
  if test $use_gui != "yes" -a "$ga_dyn_supplibs" = "yes" ; then
    GA_CHECK_LIBSX([use_gui=yes;use_freq=yes],[use_gui=yes;use_freq=no])
    gui_libs="$GA_LIBSX_LIBS"
    grads_xlibs="$XAW_XLIBS"
    use_xaw_xlibs=yes
  fi
fi
if test $use_gui = "yes" ; then
# if the gui is built, we use the X flags from Xaw.
  if test "$use_xaw_xlibs" != 'yes'; then
    grads_xlibs="$X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
    AC_SUBST(grads_xlibs)
  fi
  if test $use_freq = 'yes'; then
    AC_DEFINE(USEFREQ, 1, [Use GetFile from freq])
  else
    AC_DEFINE(USEFREQ, 0, [Use GetFile from freq])
  fi
  AC_SUBST(gui_libs)
  AC_DEFINE(USEGUI, 1, [Enable GUI widgets]) 
  echo "+ GUI enabled"
else
  AC_DEFINE(USEGUI, 0, [Enable GUI widgets]) 
  echo  "- GUI disabled"
fi

# look for libraries to support command line editing
echo
if test "$with_readline" != "no" ; then
  # check if supplibs directory exists 
  if test "Z$ga_supplib_dir" != "Z" ; then 
    echo "Checking in supplibs for libraries to support command line editing ..."
    readline_libadd=""
    GA_SET_FLAGS([" " readline])
    AC_CHECK_HEADER(readline.h,
    [ AC_CHECK_HEADER(history.h, 
      [ AC_CHECK_LIB(readline, tgetent, 
        [ use_readline=yes
        ],
        [ AC_CHECK_LIB(ncurses, tgetent,
          [ use_readline=yes
    	  readline_libadd=" -lncurses"],
          [ AC_CHECK_LIB(termcap, tgetent,
            [ use_readline=yes
              readline_libadd=" -ltermcap"
            ])
          ])
        ])
      ])
    ])
    GA_UNSET_FLAGS
  fi
  if test $use_readline = "yes" ; then
    GA_SET_INCLUDE_VAR([readline_inc],[""])
    GA_SET_LIB_VAR([readline_libs],[readline])
    readline_libs="$readline_libs $readline_libadd"
  else
    if test "$ga_dyn_supplibs" = "yes" ; then
      echo
      echo "Checking in system locations for libraries to support command line editing ..."
      GA_LIB_READLINE([use_readline=yes readline_libs=""])
    fi 
  fi
fi
if test $use_readline = "yes" ; then
  AC_SUBST(readline_inc)
  AC_SUBST(readline_libs)
  AC_DEFINE(READLINE, 1, [Enable command line editing]) 
  echo "+ Command line editing enabled"
else
  AC_DEFINE(READLINE, 0, [Enable command line editing]) 
  echo "- Command line editing disabled"
fi

# look for libraries to support geotiff
echo
if test "$with_geotiff" != "no" ; then
  # check if supplibs directory exists 
  if test "Z$ga_supplib_dir" != "Z" ; then 
    echo "Checking in supplibs for libraries to support geotiff ..."
    GA_SET_FLAGS([tiff])
    AC_CHECK_HEADERS([tiff.h tiffio.h], 
    [ AC_CHECK_LIB([tiff], [main], 
      [ have_tiff=yes 
      ])
    ])
    GA_UNSET_FLAGS
    if test "$have_tiff" = "yes" ; then
      GA_SET_FLAGS([geotiff])
      AC_CHECK_HEADER(geotiffio.h,
      [ AC_CHECK_LIB([geotiff], [main], 
        [ use_geotiff=yes
          GA_SET_INCLUDE_VAR([geotiff_inc], [tiff geotiff])
          GA_SET_LIB_VAR([geotiff_libs], [tiff geotiff])
        ])
      ])
      GA_UNSET_FLAGS
    fi
  fi
  # if we haven't got what we need, add a macro to look outside supplibs
fi
if test "$use_geotiff" != "yes" ; then
  AC_DEFINE(GEOTIFF, 0, [Enable geotiff])
  echo "- geotiff disabled" 
else
  AC_SUBST(geotiff_inc)
  AC_SUBST(geotiff_libs)
  AC_DEFINE(GEOTIFF, 1, [Enable geotiff])
  echo "+ geotiff enabled" 
fi


# look for libraries to support printim
echo
if test "$with_printim" != "no" ; then
  # check if supplibs directory exists 
  if test "Z$ga_supplib_dir" != "Z" ; then 
    echo "Checking in supplibs for libraries to support printim ..."
    GA_SET_FLAGS([zlib])
    AC_CHECK_HEADERS([zlib.h], 
    [ AC_CHECK_LIB([z], [compress], 
      [ have_zlib=yes 
      ])
    ])
    GA_UNSET_FLAGS
    GA_SET_FLAGS([libpng12])
    AC_CHECK_LIB([png12], [main], 
    [ have_libpng=yes 
    ])
    GA_UNSET_FLAGS
    GA_SET_FLAGS([])
    AC_CHECK_LIB([jpeg], [main], 
    [ have_jpeg=yes 
    ])
    GA_UNSET_FLAGS
    if test "$have_zlib" = "yes" -a "$have_libpng" = "yes" ; then
      GA_SET_FLAGS([gd])
      AC_CHECK_HEADER(gd.h,
      [ AC_CHECK_LIB([gd], [gdImageCreate], 
        [ use_printim=yes
          GA_SET_INCLUDE_VAR([printim_inc], [gd])
	  if test "$have_jpeg" = "yes" ; then
	    GA_SET_LIB_VAR([printim_libs], [gd png12 z jpeg])
	  else
  	    GA_SET_LIB_VAR([printim_libs], [gd png12 z])
          fi
        ])
      ])
      GA_UNSET_FLAGS
    fi
  fi
  # if we haven't got what we need, look outside supplibs
  if test "$use_printim" != "yes" -a "$ga_dyn_supplibs" = "yes" ; then
    echo
    echo "Checking in system locations for libraries to support printim ..."
    GA_CHECK_LIB_GD(
    [ use_printim=yes 
      printim_libs=$GD_LIBS
      printim_inc=$GD_CFLAGS
    ])
  fi
fi
if test "$use_printim" != "yes" ; then
  AC_DEFINE(GXPNG, 0, [Enable printim using zlib, libpng, and libgd])
  echo "- printim disabled (Warning: gxtran will not be built)" 
else
  extra_utils="$extra_utils gxtran$exe_ext"
  AC_SUBST(printim_inc)
  AC_SUBST(printim_libs)
  AC_DEFINE(GXPNG, 1, [Enable printim using zlib, libpng, and libgd])
  echo "+ printim enabled" 
fi

# look for libraries to support grib2
echo
if test "$with_grib2" != "no" ; then
  # check if supplibs directory exists 
  if test "Z$ga_supplib_dir" != "Z" ; then 
    echo "Checking in supplibs for libraries to support grib2 ..."
    if test "$have_zlib" = "no" ; then
      GA_SET_FLAGS([zlib])
      AC_CHECK_HEADERS([zlib.h], 
      [ AC_CHECK_LIB([z], [compress], 
        [ have_zlib=yes 
        ])
      ])
      GA_UNSET_FLAGS
    fi
    if test "$have_libpng" = "no" ; then
      GA_SET_FLAGS([libpng12])
      AC_CHECK_LIB([png12], [main], 
      [ have_libpng=yes 
      ])
      GA_UNSET_FLAGS
    fi
    if test "$have_zlib" = "yes" -a "$have_libpng" = "yes" ; then
      have_libjasper=no
      GA_SET_FLAGS([])
      AC_CHECK_LIB([jasper], [main], 
      [ have_libjasper=yes 
      ])
      GA_UNSET_FLAGS
      if test "$have_libjasper" = "yes" ; then
        GA_SET_FLAGS([grib2c])
        AC_CHECK_HEADER(grib2.h,
        [ AC_CHECK_LIB([grib2c], [main], 
          [ have_grib2=yes 
            GA_SET_INCLUDE_VAR([grib2_inc], [grib2c])
            GA_SET_LIB_VAR([grib2_libs], [grib2c jasper png12 z])
          ])
        ])
        GA_UNSET_FLAGS
      fi
    fi
  fi
    # if we haven't got what we need, look outside supplibs
  if test "$have_grib2" != "yes" -a "$ga_dyn_supplibs" = "yes" ; then
    echo
    echo "Checking in system locations for grib2 libraries ..."
    GA_CHECK_LIB_GRIB2(
    [ have_grib2=yes 
      grib2_libs=$G2_LIBS
    ])
  fi
fi
if test "$have_grib2" != "yes" ; then
  AC_DEFINE(GRIB2, 0, [Enable grib2])
  echo "- grib2 disabled " 
else
  extra_utils="$extra_utils grib2scan$exe_ext"
  AC_SUBST(grib2_inc)
  AC_SUBST(grib2_libs)
  AC_DEFINE(GRIB2, 1, [Enable grib2])
  echo "+ grib2 enabled" 
fi

# check if user wants support for self-describing files?
echo
if test "$with_sdf" = "no" ; then 
  AC_DEFINE(USENETCDF, 0, [Enable netcdf])
  AC_DEFINE(USEHDF, 0, [Enable hdf4])
  echo No support for self-describing files:
  echo "- netcdf disabled"
  echo "- hdf4 disabled"
  echo "- gradsdap (OPeNDAP/netCDF) build disabled"
else
  # look for libraries to support hdf4
  # check if supplibs directory exists 
  if test "Z$ga_supplib_dir" != "Z" ; then 
    echo "Checking in supplibs for libraries to support hdf4 ..."
    # look for zlib
    if test "$have_zlib" = "no" ; then
      GA_SET_FLAGS([zlib])
      AC_CHECK_HEADERS([zlib.h], 
      [ AC_CHECK_LIB([z], [compress], 
        [ have_zlib=yes 
        ])
      ])
      GA_UNSET_FLAGS
    fi
    # look for udunits
    GA_SET_FLAGS([udunits])
    AC_CHECK_HEADERS([udunits.h], 
    [ AC_CHECK_LIB([udunits], [utInit], 
      [ have_udunits=yes 
      ])
    ])
    GA_UNSET_FLAGS	
    # look for jpeg
    if test "$have_jpeg" = "no" ; then
      GA_SET_FLAGS([])
      AC_CHECK_LIB([jpeg], [main], 
      [ have_jpeg=yes 
      ])
      GA_UNSET_FLAGS
    fi
    if test "$have_zlib" = "yes" -a "$have_udunits" = "yes" -a "$have_jpeg" = "yes" ; then
      GA_SET_FLAGS([hdf])
      AC_CHECK_LIB([sz], [main],
      [ AC_CHECK_HEADER([mfhdf.h],
        [ AC_CHECK_LIB([df], [main], 
          [ AC_CHECK_LIB([mfhdf], [main],
            [ use_hdf=yes 
              GA_SET_INCLUDE_VAR([hdf_inc], [hdf udunits])
              GA_SET_LIB_VAR([hdf_libs], [mfhdf df udunits sz jpeg z])
            ])
          ])
        ])
      ])
      GA_UNSET_FLAGS
    fi
  fi
  # if we haven't got what we need, look outside supplibs
  if test $use_hdf != "yes" -a "$ga_dyn_supplibs" = "yes" ; then
    echo
    echo "Checking in system locations for libraries to support hdf ..."
    GA_CHECK_UDUNITS(
    [ AC_CHECK_HDF4_NETCDF(
      [ use_hdf=yes
      ],
      [ use_hdf=yes
      ])
      if test "$use_hdf" = 'yes'; then
        LDFLAGS="$LDFLAGS $HDF4_LDFLAGS"
        hdf_libs="$HDF4_LIBS $UDUNITS_LIBS"
      fi
    ])
  fi

  if test $use_hdf != "yes" ; then
    AC_DEFINE(USEHDF, 0, [Enable hdf4])
    echo "- hdf4 disabled"  
  else
    AC_SUBST(hdf_inc)
    AC_SUBST(hdf_libs)
    AC_DEFINE(USEHDF, 1, [Enable hdf4])
    echo "+ hdf4 enabled"
  fi

  # look for libraries to support netcdf
  echo
  # check if supplibs directory exists 
  if test "Z$ga_supplib_dir" != "Z" ; then 
    echo "Checking in supplibs for libraries to support netcdf ..."
    if test "$have_udunits" = "no" ; then
      GA_SET_FLAGS([udunits])
      AC_CHECK_HEADERS([udunits.h], 
      [ AC_CHECK_LIB([udunits], [utInit], 
        [ have_udunits=yes 
        ])
      ])
      GA_UNSET_FLAGS
    fi
    if test "$have_udunits" = "yes" ; then
      GA_SET_FLAGS([netcdf])
      AC_CHECK_HEADER([netcdf.h],
      [ AC_CHECK_LIB([netcdf], [main],
        [ use_nc="yes"
          GA_SET_INCLUDE_VAR(nc_inc, [udunits netcdf])
          GA_SET_LIB_VAR(nc_libs, [netcdf udunits])
        ])
      ])
      GA_UNSET_FLAGS
    fi
  fi
  # if we haven't got what we need, look outside supplibs
  if test $use_nc != "yes"  -a "$ga_dyn_supplibs" = "yes" ; then
    echo
    echo "Checking in system locations for libraries to support netcdf ..."
    # check for netcdf 3 
    GA_CHECK_UDUNITS(
    [ AC_CHECK_NETCDF(
      [ use_nc=yes 
        LDFLAGS="$LDFLAGS $NC_LDFLAGS"
        nc_libs="$NC_LIBS $UDUNITS_LIBS"
      ],,
      [3])
    ])
  fi

  if test $use_nc = "yes" ; then
    AC_SUBST(nc_inc)
    AC_SUBST(nc_libs)
    AC_DEFINE(USENETCDF, 1, [Enable netcdf])
    echo "+ netcdf enabled"
  else
    AC_DEFINE(USENETCDF, 0, [Enable netcdf])
    echo "- netcdf disabled"
  fi

  # look for libraries to support OPeNDAP
  echo
  if test "$with_dap" != "no" ; then
    # check if supplibs directory exists 
    if test "Z$ga_supplib_dir" != "Z" ; then 
      echo "Checking in supplibs for libraries to support OPeNDAP access ..."
      if test "$have_udunits" = "no" ; then
        GA_SET_FLAGS([udunits])
        AC_CHECK_HEADERS([udunits.h], 
        [ AC_CHECK_LIB([udunits], [utInit], 
          [ have_udunits=yes 
          ])
        ])
        GA_UNSET_FLAGS
      fi
      if test "$have_udunits" = "yes" ; then
        GA_SET_FLAGS([libnc-dap])
        AC_CHECK_HEADER(netcdf.h,
        [ AC_CHECK_LIB(dap,main,
          [ AC_CHECK_LIB(dapclient,main,
            [ AC_CHECK_LIB(nc-dap,main,
              [ AC_CHECK_LIB(xml2,main,
                [ AC_CHECK_LIB(curl,main,
                  [ use_dap="yes"
                    GA_SET_INCLUDE_VAR(dap_inc, [udunits libnc-dap])
          	    GA_SET_LIB_VAR(dap_libs, [udunits nc-dap dapclient dap xml2 curl])
       	          ])
      	        ])
              ])
            ])
          ])
        ])
        GA_UNSET_FLAGS
        if test $use_dap != "no" ; then
          GA_SET_FLAGS([gadap])
          AC_CHECK_HEADER(gadap.h,
          [ AC_CHECK_LIB(gadap, main, 
            [ use_gadap="yes"
              GA_SET_INCLUDE_VAR(gadap_inc, [gadap])
              GA_SET_LIB_VAR(gadap_lib, [gadap])
            ])
          ])
          GA_UNSET_FLAGS
        fi
      fi
    fi
    if test $use_dap != "no" ; then
      # we found what is needed in supplibs, now add linking options
      if test "$is_darwin" = "yes" ; then 
        dap_extra_libs="-lpthread -lm -liconv"
      else
        dap_extra_libs="-lpthread -lm -ldl"
      fi
      # merge dap and gadap 
      dap_libs="$gadap_lib $dap_libs $dap_extra_libs"
      dap_inc="$gadap_inc $dap_inc"
    fi
    if test $use_dap != "yes" -a "$ga_dyn_supplibs" = "yes" ; then
      # we do not have what we need, look outside supplibs
      echo
      echo "Checking in system locations for libraries to support OPeNDAP access ..."
      GA_CHECK_UDUNITS(
      [ AC_CHECK_NC_DODS(
        [ use_dap=yes 
          dap_libs="$UDUNITS_LIBS $NC_DAP_LIBS"
        ])
      ])
      AC_CHECK_HEADER(gadap.h,
      [ AC_CHECK_LIB(gadap, main, 
        [ use_gadap="yes"
          dap_libs="$dap_libs -lgadap"
        ])
      ])
    fi
  fi
  if test $use_dap = "yes" ; then
    extra_bins="$extra_bins gradsdap$exe_ext"
    if test $use_gadap = "yes" ; then	
      gadap_def="-DUSEGADAP=1"
      echo "+ gradsdap build enabled -- OPeNDAP interface for gridded and station data"
    else
      gadap_def="-DUSEGADAP=0"
      echo "+ gradsdap build enabled -- OPeNDAP interface for gridded data only"
    fi
    AC_SUBST(gadap_def)
    AC_SUBST(dap_inc)
    AC_SUBST(dap_libs)
    AC_DEFINE(USENETCDF, 1, [Enable netcdf])
  else
    echo "- gradsdap (OPeNDAP/netCDF) build disabled"
  fi
fi
echo

#  ----------------------------------------------------
# Send test results to makefile and config.h
# ----------------------------------------------------

# Some non-configuration-dependent GrADS macros
AC_DEFINE(GRADS_VERSION, "AC_PACKAGE_VERSION", [GrADS version])
AC_DEFINE(GRADS_DESC,, [Obsolete feature description string])

# These are used to add the necessary source files to Makefile targets
AM_CONDITIONAL(READLINE, [ test "$use_readline"  = "yes" ] )
AM_CONDITIONAL(USEGUI,   [ test "$use_gui"  = "yes" ] )
AM_CONDITIONAL(GXPNG,    [ test "$use_printim"  = "yes" ] )
AM_CONDITIONAL(GEOTIFF,  [ test "$use_geotiff"  = "yes" ] )
AM_CONDITIONAL(USELATS,  [ test "$use_lats" = "yes" ] )
AM_CONDITIONAL(USEGADAP, [ test "$use_gadap" = "yes" ] )
AM_CONDITIONAL(XLIBEMU,  [ test "$use_xlibemu" = "yes" ] )

# These control which binaries are built
AM_CONDITIONAL(USEDAP,   [ test "$use_dap" = "yes" ] )
AM_CONDITIONAL(USENC,    [ test "$use_nc" = "yes" ] )
AM_CONDITIONAL(USEHDF,   [ test "$use_hdf" = "yes" ] )
AC_SUBST(extra_bins)
AC_SUBST(extra_utils)

# Host specific linker flags
AC_SUBST(host_ldadd)


dnl ----------------------------------------------------
dnl Print summary and write output files
dnl ----------------------------------------------------

build_date=`date +%D`
build_host=`uname -sr`
build_info="Built `date` for ${host}"

   echo \
"  +=========================================================================+"
   echo "   GrADS "AC_PACKAGE_VERSION ":" $build_info
   echo \
"  +=========================================================================+"
   echo     
   echo "  +----------------------------------+"
   echo "  |                                  |"
   echo "  | Configuration Summary            |"
   echo "  |                                  |"
if test "$use_readline" = "yes" ; then
   echo "  |  + readline enabled              |"
else
   echo "  |  - readline disabled             |"
fi 
if test "$use_printim" = "yes" ; then
   echo "  |  + printim enabled               |"
else
   echo "  |  - printim disabled              |"
fi 
if test "$use_nc" = "yes" ; then
   echo "  |  + netcdf enabled                |"
else
   echo "  |  - netcdf disabled               |"
fi 
if test "$use_hdf" = "yes" ; then
   echo "  |  + hdf4 enabled                  |"
else
   echo "  |  - hdf4 disabled                 |"
fi 
if test "$have_grib2" = "yes" ; then
   echo "  |  + grib2 enabled                 |"
else
   echo "  |  - grib2 disabled                |"
fi 
if test "$use_geotiff" = "yes" ; then
   echo "  |  + geotiff enabled               |"
else
   echo "  |  - geotiff disabled              |"
fi 
if test "$use_gui" = "yes" ; then
   echo "  |  + GUI enabled                   |"
else
   echo "  |  - GUI disabled                  |"
fi 
   echo "  |                                  |"
   echo "  | Build Summary                    |"
   echo "  |                                  |"
   echo "  |  + grads enabled                 |"
if test "$use_dap" = "yes" ; then
   echo "  |  + gradsdap enabled              |"
  if test "$use_gadap" = "yes" ; then
   echo "  |    (both grid & stn interfaces)  |"
  else 
   echo "  |    (grid interface only)         |"
  fi
else
   echo "  |  - gradsdap (OPeNDAP) disabled   |"
fi 
   echo "  |                                  |"
if test "$ga_dyn_supplibs" = "yes" ; then
   echo "  |  + Dynamic linking enabled       |"
else
   echo "  |  - Dynamic linking disabled      |"
fi 
   echo "  |                                  |"
   echo "  +----------------------------------+"

echo "configure: creating src/VERSION"
echo AC_PACKAGE_VERSION > src/VERSION

echo "configure: creating src/buildinfo.h"
mkdir -p src
echo "static char *buildinfo = \"${build_info}\";" > src/buildinfo.h

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

echo
echo "Run 'make && make install' to build GrADS."
echo "Executables will be installed" \
     "to bin/ in current dir, unless prefix was specified."
echo
