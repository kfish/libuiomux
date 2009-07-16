## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) \
           -I$(top_srcdir)/include

UIOMUXDIR = ../libuiomux
UIOMUX_LIBS = $(UIOMUXDIR)/libuiomux.la $(PTHREAD_LIBS) $(RT_LIBS)

bin_PROGRAMS = uiomux

uiomux_SOURCES = uiomux.c uiomux-alloc.c
uiomux_LDADD = $(UIOMUX_LIBS)
