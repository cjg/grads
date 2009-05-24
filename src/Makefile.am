##
##          Makefile.am  for GrADS
##
######################################################################

######################################################################
#
# Targets
#

# Targets that will built by "make all"
bin_PROGRAMS	       = grads $(extra_bins) \
		         gribmap gxeps gxps stnmap wgrib gribscan bufrscan \
			 $(extra_utils)

# Targets that will not always be built
EXTRA_PROGRAMS	       = gxtran gradsdap grib2scan

# Convenience target
utils: bufrscan gribscan gribmap gxeps gxps stnmap wgrib $(extra_utils)

######################################################################
#
# Compiler and linker settings
#

# Paths to external headers and libraries
supp_include_dir	= $(SUPPLIBS)/include
supp_lib_dir		= $(SUPPLIBS)/lib

# Settings used for all binaries
#  the include/dap and include/netcdf directories are added below
#  to COMPILE_C and COMPILE_DAP
LDADD			= -L$(supp_lib_dir) 
INCLUDES		= $(readline_inc) $(printim_inc) $(grib2_inc) \
			  $(hdf_inc) $(geotiff_inc) \
                          $(X_CFLAGS) $(XAW_CFLAGS) $(GD_CFLAGS) $(HDF4_CFLAGS)

# Settings used for all GrADS binaries
common_ldadd            = $(LDADD) $(X_LIBS) $(xlibemu_libs) $(host_ldadd) -lX11

######################################################################
#
# Headers and data
#

# Headers must be listed here to be included in the distribution.  The
# "noinst_" prefix prevents "make install" from trying to do anything
# with them
hdr_core            = gagmap.h grads.h gs.h gatypes.h \
		      gvt.h gx.h gxmap.h wx.h
hdr_bufr	    = gabufr.h
hdr_x11		    = gx.h bitmaps.h
hdr_sdf             = gasdf.h gasdf_std_time.h 
hdr_gui		    = gagui.h
hdr_pc		    = pcx11e.h
hdr_png             = mtables.c
noinst_HEADERS	    = $(hdr_core) $(hdr_x11) $(hdr_sdf) $(hdr_png) $(hdr_gui) \
 $(hdr_pc) $(hdr_bufr)

# Get rid of buildinfo.h on "make distclean"
DISTCLEANFILES	       = buildinfo.h


######################################################################
#
# Configuration-dependent sources & headers
#

if USEGADAP
src_gadap 	    = dodstn.c
obj_gadap           = dodstn.o
endif

if XLIBEMU
src_pc 		    = pcx11e.c
endif

if USEGUI
src_gui             = gagui.c gsgui.c
endif


##############################################################
#
# Sources for GrADS binaries. Unfortunately, automake doesn't allow
# substitions in _SOURCES variables, so these lists have to be repeated
# explicitly for all GrADS builds.

##############################################################
##
## grads
##

# Sources to be compiled
grads_SOURCES          = grads.c gxsubs.c gxmeta.c gxchpl.c gxcntr.c gxstrm.c \
			 gxwmap.c gxshad.c gaexpr.c gafunc.c gautil.c gagx.c \
			 gscrpt.c gamach.c bufrstn.c gabufr.c gabufrtbl.c \
			 gxX.c gxdxwd.c galloc.c $(src_gui) $(src_pc)

# Sources that will not always be compiled
# Since gradsc is the only GrADS binary that is not conditionally compiled,
# any source file that we want to go in the distribution must be listed.
EXTRA_grads_SOURCES    = gagmap.c gagui.c gsgui.c pcx11e.c \
			 gauser.c gaddes.c gaio.c gacfg.c gxhpng.c \
			 gasdf.c dodstn.c

# Object files that should not be reused by the other GrADS binaries,
# plus libraries needed by this GrADS binary
grads_LDADD	       = gaddes-c.o gacfg-c.o gaio-c.o gauser-c.o gasdf-c.o \
                         $(common_ldadd) $(readline_libs) $(printim_libs) \
			 $(grib2_libs) $(hdf_libs) $(nc_libs) $(gui_libs) $(geotiff_libs) 

# Custom compilation for object files specific to this GrADS binary
COMPILE_C = $(COMPILE) $(nc_inc) -DUSEDAP=0 -DUSEGADAP=0

gaddes-c.o: $(srcdir)/gaddes.c
	$(COMPILE_C) -o gaddes-c.o -c $(srcdir)/gaddes.c
gacfg-c.o: $(srcdir)/gacfg.c
	$(COMPILE_C) -o gacfg-c.o -c $(srcdir)/gacfg.c
gaio-c.o: $(srcdir)/gaio.c
	$(COMPILE_C) -o gaio-c.o -c $(srcdir)/gaio.c
gauser-c.o: $(srcdir)/gauser.c
	$(COMPILE_C) -o gauser-c.o -c $(srcdir)/gauser.c
gasdf-c.o: $(srcdir)/gasdf.c
	$(COMPILE_C) -o gasdf-c.o -c $(srcdir)/gasdf.c

##############################################################
#
# gradsdap
#

if USEDAP

# Sources to be compiled
gradsdap_SOURCES       = grads.c gxsubs.c gxmeta.c gxchpl.c gxcntr.c gxstrm.c \
			 gxwmap.c gxshad.c gaexpr.c gafunc.c gautil.c gagx.c \
			 gscrpt.c gamach.c bufrstn.c gabufr.c gabufrtbl.c \
			 gxX.c gxdxwd.c galloc.c \
		         $(src_gui) $(src_pc) $(src_gadap)

# Sources that will not always be compiled
EXTRA_gradsdap_SOURCES = gagmap.c gagui.c gsgui.c pcx11e.c \
			 gauser.c gaddes.c gaio.c gacfg.c gxhpng.c \
			 gasdf.c dodstn.c

# Custom link command for gradsdap, using C++ linker instead of C linker
gradsdap_LINK          = $(CXX) $(AM_CFLAGS) $(CFLAGS) \
			 $(AM_LDFLAGS) $(LDFLAGS) -o $@


# Object files that should not be reused by the other GrADS binaries,
# plus libraries needed by this GrADS binary
gradsdap_LDADD	       = gaddes-dap.o gacfg-dap.o gaio-dap.o gauser-dap.o gasdf-dap.o \
			 $(common_ldadd) $(readline_libs) $(printim_libs) $(dap_libs) \
			 $(hdf_libs) $(grib2_libs) $(gui_libs) $(geotiff_libs) 


# Custom compilation for object files specific to this GrADS binary
COMPILE_DAP = $(COMPILE) $(dap_inc) -DUSEDAP=1 $(gadap_def) 

gaddes-dap.o: $(srcdir)/gaddes.c
	$(COMPILE_DAP) -o gaddes-dap.o -c $(srcdir)/gaddes.c
gacfg-dap.o: $(srcdir)/gacfg.c
	$(COMPILE_DAP) -o gacfg-dap.o -c $(srcdir)/gacfg.c
gaio-dap.o: $(srcdir)/gaio.c
	$(COMPILE_DAP) -o gaio-dap.o -c $(srcdir)/gaio.c
gauser-dap.o: $(srcdir)/gauser.c
	$(COMPILE_DAP) -o gauser-dap.o -c $(srcdir)/gauser.c
gasdf-dap.o: $(srcdir)/gasdf.c
	$(COMPILE_DAP) -o gasdf-dap.o -c $(srcdir)/gasdf.c
if USEGADAP
dodstn.o: $(srcdir)/dodstn.c
	$(COMPILE_DAP) -o dodstn.o -c $(srcdir)/dodstn.c
endif
endif

##############################################################
#
# Utilities
#

# Object files that need to be recompiled for standalone utilities
gaddes.sa.o: $(srcdir)/gaddes.c
	$(COMPILE) -DSTNDALN -o gaddes.sa.o -c $(srcdir)/gaddes.c

gautil.sa.o: $(srcdir)/gautil.c
	$(COMPILE) -DSTNDALN -o gautil.sa.o -c $(srcdir)/gautil.c

pcx11e.sa.o: $(srcdir)/pcx11e.c
	$(COMPILE) -DSTNDALN -o pcx11e.sa.o -c $(srcdir)/pcx11e.c

gxmeta.sa.o: $(srcdir)/gxmeta.c
	$(COMPILE) -DSTNDALN -o gxmeta.sa.o -c $(srcdir)/gxmeta.c

##############################################################
#
# stnmap
#
stnmap_SOURCES	       = stnmap.c gamach.c galloc.c
stnmap_LDADD	       = gaddes.sa.o gautil.sa.o $(LDADD) $(readline_libs)

##############################################################
#
# gribmap
#
gribmap_SOURCES	       = gribmap.c gagmap.c gamach.c galloc.c
gribmap_LDADD	       = gaddes.sa.o gautil.sa.o $(LDADD) $(readline_libs) $(grib2_libs)

##############################################################
#
# wgrib
#
wgrib_SOURCES	       = wgrib.c

##############################################################
#
# gxps
#
gxps_SOURCES	       = gxps.c
gxeps_SOURCES	       = gxeps.c

##############################################################
#
# gxtran
#
gxtran_SOURCES	       = gxtran.c gxsubs.c gxchpl.c gxX.c gxdxwd.c galloc.c $(src_tran_pc)
EXTRA_gxtran_SOURCES   = pcx11e.c
gxtran_LDADD	       = gxmeta.sa.o $(LDADD) $(X_LIBS) $(printim_libs) \
			 -lX11 $(xlibemu_libs)

##############################################################
#
# gribscan
#
gribscan_SOURCES       = gribscan.c gamach.c galloc.c
gribscan_LDADD	       = gautil.sa.o $(LDADD) $(readline_libs)

##############################################################
#
# grib2scan
#
grib2scan_SOURCES      = grib2scan.c gamach.c galloc.c
grib2scan_LDADD	       = gautil.sa.o $(LDADD) $(grib2_libs) $(readline_libs)

##############################################################
#
# bufrscan
#
bufrscan_SOURCES       = bufrscan.c gabufr.c gabufrtbl.c gamach.c

