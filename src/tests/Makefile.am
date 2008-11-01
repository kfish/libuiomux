## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) \
           -I$(top_srcdir)/include

UIOMUXDIR = ../libuiomux
UIOMUX_LIBS = $(UIOMUXDIR)/libuiomux.la $(PTHREAD_LIBS) $(RT_LIBS)

# Test programs

TESTS_ENVIRONMENT = $(VALGRIND_ENVIRONMENT)

test: check

basic_tests = noop double-open lock-unlock fork threads fork-threads exit-locked

noinst_PROGRAMS = $(basic_tests)
noinst_HEADERS = uiomux_tests.h

TESTS = $(basic_tests)

noop_SOURCES = noop.c
noop_LDADD = $(UIOMUX_LIBS)

double_open_SOURCES = double-open.c
double_open_LDADD = $(UIOMUX_LIBS)

lock_unlock_SOURCES = lock-unlock.c
lock_unlock_LDADD = $(UIOMUX_LIBS)

fork_SOURCES = fork.c
fork_LDADD = $(UIOMUX_LIBS)

threads_SOURCES = threads.c
threads_LDADD = $(UIOMUX_LIBS)

fork_threads_SOURCES = fork-threads.c
fork_threads_LDADD = $(UIOMUX_LIBS)

exit_locked_SOURCES = exit-locked.c
exit_locked_LDADD = $(UIOMUX_LIBS)
