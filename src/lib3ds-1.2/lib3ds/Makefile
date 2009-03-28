# Generated automatically from Makefile.in by configure.
# Makefile.in generated automatically by automake 1.4-p5 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


SHELL = /bin/sh

srcdir = .
top_srcdir = ..
prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/lib3ds
pkglibdir = $(libdir)/lib3ds
pkgincludedir = $(includedir)/lib3ds

top_builddir = ..

ACLOCAL = /HDD2/games/SpringSrc/UpSpring/src/lib3ds/missing aclocal
AUTOCONF = /HDD2/games/SpringSrc/UpSpring/src/lib3ds/missing autoconf
AUTOMAKE = /HDD2/games/SpringSrc/UpSpring/src/lib3ds/missing automake
AUTOHEADER = /HDD2/games/SpringSrc/UpSpring/src/lib3ds/missing autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL_PROGRAM}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
CC = gcc
DMALLOC_CFLAGS = 
DMALLOC_LIBS = 
GLUT_CFLAGS = -I/usr/X11R6/include
GLUT_HEADER_DIR = GL
GLUT_LIBS = -lglut -lGLU -lGL
GLU_HEADER_DIR = GL
GL_HEADER_DIR = GL
LIB3DS_BINARY_AGE = 0
LIB3DS_INTERFACE_AGE = 0
LIB3DS_MAJOR_VERSION = 1
LIB3DS_MICRO_VERSION = 0
LIB3DS_MINOR_VERSION = 2
LIB3DS_VERSION = 1.2.0
LT_AGE = 0
LT_CURRENT = 0
LT_RELEASE = 1.2
LT_REVISION = 0
MAKEINFO = /HDD2/games/SpringSrc/UpSpring/src/lib3ds/missing makeinfo
PACKAGE = lib3ds
RANLIB = ranlib
VERSION = 1.2.0

lib3dsdir = $(includedir)/lib3ds

INCLUDES = -I$(top_srcdir)

lib_LIBRARIES = lib3ds.a

lib3ds_a_SOURCES = \
  io.c \
  float.c \
  vector.c \
  matrix.c \
  quat.c \
  tcb.c \
  ease.c \
  chunktable.h \
  chunk.c \
  file.c \
  background.c \
  atmosphere.c \
  shadow.c \
  viewport.c \
  material.c \
  mesh.c \
  camera.c \
  light.c \
  tracks.c \
  node.c


lib3ds_HEADERS = \
  types.h \
  io.h \
  float.h \
  vector.h \
  matrix.h \
  quat.h \
  tcb.h \
  ease.h \
  chunk.h \
  file.h \
  background.h \
  atmosphere.h \
  shadow.h \
  viewport.h \
  material.h \
  mesh.h \
  camera.h \
  light.h \
  tracks.h \
  node.h


EXTRA_DIST = \
  types.txt \
  chunktable.sed

mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = ../config.h
CONFIG_CLEAN_FILES = 
LIBRARIES =  $(lib_LIBRARIES)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I..
CPPFLAGS = 
LDFLAGS = 
LIBS = 
lib3ds_a_LIBADD = 
lib3ds_a_OBJECTS =  io.o float.o vector.o matrix.o quat.o tcb.o ease.o \
chunk.o file.o background.o atmosphere.o shadow.o viewport.o material.o \
mesh.o camera.o light.o tracks.o node.o
AR = ar
CFLAGS = -g -O2
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
HEADERS =  $(lib3ds_HEADERS)

DIST_COMMON =  Makefile.am Makefile.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

TAR = gtar
GZIP_ENV = --best
SOURCES = $(lib3ds_a_SOURCES)
OBJECTS = $(lib3ds_a_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .o .s
$(srcdir)/Makefile.in: Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps lib3ds/Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$(subdir)/$@ CONFIG_HEADERS= $(SHELL) ./config.status


mostlyclean-libLIBRARIES:

clean-libLIBRARIES:
	-test -z "$(lib_LIBRARIES)" || rm -f $(lib_LIBRARIES)

distclean-libLIBRARIES:

maintainer-clean-libLIBRARIES:

install-libLIBRARIES: $(lib_LIBRARIES)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	@list='$(lib_LIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(libdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(libdir)/$$p; \
	  else :; fi; \
	done
	@$(POST_INSTALL)
	@list='$(lib_LIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(RANLIB) $(DESTDIR)$(libdir)/$$p"; \
	    $(RANLIB) $(DESTDIR)$(libdir)/$$p; \
	  else :; fi; \
	done

uninstall-libLIBRARIES:
	@$(NORMAL_UNINSTALL)
	list='$(lib_LIBRARIES)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(libdir)/$$p; \
	done

.c.o:
	$(COMPILE) -c $<

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

lib3ds.a: $(lib3ds_a_OBJECTS) $(lib3ds_a_DEPENDENCIES)
	-rm -f lib3ds.a
	$(AR) cru lib3ds.a $(lib3ds_a_OBJECTS) $(lib3ds_a_LIBADD)
	$(RANLIB) lib3ds.a

install-lib3dsHEADERS: $(lib3ds_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(lib3dsdir)
	@list='$(lib3ds_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(lib3dsdir)/$$p"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(lib3dsdir)/$$p; \
	done

uninstall-lib3dsHEADERS:
	@$(NORMAL_UNINSTALL)
	list='$(lib3ds_HEADERS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(lib3dsdir)/$$p; \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS:  $(HEADERS) $(SOURCES)  $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags  $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(top_builddir)/$(PACKAGE)-$(VERSION)/$(subdir)

subdir = lib3ds

distdir: $(DISTFILES)
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done

info-am:
info: info-am
dvi-am:
dvi: dvi-am
check-am: all-am
check: check-am
installcheck-am:
installcheck: installcheck-am
install-exec-am: install-libLIBRARIES
install-exec: install-exec-am

install-data-am: install-lib3dsHEADERS
install-data: install-data-am

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-am
uninstall-am: uninstall-libLIBRARIES uninstall-lib3dsHEADERS
uninstall: uninstall-am
all-am: Makefile $(LIBRARIES) $(HEADERS)
all-redirect: all-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs:
	$(mkinstalldirs)  $(DESTDIR)$(libdir) $(DESTDIR)$(lib3dsdir)


mostlyclean-generic:

clean-generic:

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-libLIBRARIES mostlyclean-compile \
		mostlyclean-tags mostlyclean-generic

mostlyclean: mostlyclean-am

clean-am:  clean-libLIBRARIES clean-compile clean-tags clean-generic \
		mostlyclean-am

clean: clean-am

distclean-am:  distclean-libLIBRARIES distclean-compile distclean-tags \
		distclean-generic clean-am

distclean: distclean-am

maintainer-clean-am:  maintainer-clean-libLIBRARIES \
		maintainer-clean-compile maintainer-clean-tags \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-am

.PHONY: mostlyclean-libLIBRARIES distclean-libLIBRARIES \
clean-libLIBRARIES maintainer-clean-libLIBRARIES uninstall-libLIBRARIES \
install-libLIBRARIES mostlyclean-compile distclean-compile \
clean-compile maintainer-clean-compile uninstall-lib3dsHEADERS \
install-lib3dsHEADERS tags mostlyclean-tags distclean-tags clean-tags \
maintainer-clean-tags distdir info-am info dvi-am dvi check check-am \
installcheck-am installcheck install-exec-am install-exec \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


 #
 # The 3D Studio File Format Library
 # Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 # All rights reserved.
 #
 # This program is  free  software;  you can redistribute it and/or modify it
 # under the terms of the  GNU Lesser General Public License  as published by 
 # the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 # your option) any later version.
 #
 # This  program  is  distributed in  the  hope that it will  be useful,  but
 # WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 # or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 # License for more details.
 #
 # You should  have received  a copy of the GNU Lesser General Public License
 # along with  this program;  if not, write to the  Free Software Foundation,
 # Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 #
 # $Id: Makefile.am,v 1.11 2002/01/14 12:27:45 jeh Exp $
 #

chunktable.h: chunk.h chunktable.sed
	sed -f chunktable.sed chunk.h >chunktable.h

chunk.c: chunktable.h

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
