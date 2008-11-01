
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uiomux/uiomux.h>

static void
version (void)
{
  printf ("uiomux version " VERSION "\n");
}

static void
usage (void)
{
  printf ("uiomux <command> [options]\n");
}

static void
query (void)
{
  uiomux_blockmask_t blocks;
  int i;

  blocks = uiomux_query ();

  for (i=0; i < 16; i++) {
    if (blocks & (1<<i)) {
      puts (uiomux_name (1<<i));
    }
  }
}

static void
info (void)
{
  struct uiomux * uiomux;

  uiomux = uiomux_open (0);
  uiomux_info (uiomux);
  uiomux_close (uiomux);
}

static void
reset (void)
{
  struct uiomux * uiomux;

  uiomux = uiomux_open (0);
  printf ("Resetting uiomux system...\n");
  uiomux_system_reset (uiomux);
  uiomux_close (uiomux);
}

static void
destroy (void)
{
  struct uiomux * uiomux;

  uiomux = uiomux_open (0);
  printf ("Destroying uiomux system...\n");
  uiomux_system_destroy (uiomux);
  exit (0);
}

int
main (int argc, char *argv[])
{
  if (argc < 2) {
    usage();
    exit (1);
  }

  if (!strncmp (argv[1], "query", 6)) {
    query ();
  } else if (!strncmp (argv[1], "info", 5)) {
    info ();
  } else if (!strncmp (argv[1], "reset", 6)) {
    reset ();
  } else if (!strncmp (argv[1], "destroy", 8)) {
    destroy ();
  }
}
