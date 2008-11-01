
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

int
main (int argc, char *argv[])
{
  struct uiomux * uiomux;

  if (argc < 2) {
    usage();
    exit (1);
  }

  uiomux = uiomux_open (0);

  if (!strncmp (argv[1], "reset", 6)) {
    printf ("Resetting uiomux system...\n");
    uiomux_system_reset (uiomux);
  } else if (!strncmp (argv[1], "destroy", 8)) {
    printf ("Destroying uiomux system...\n");
    uiomux_system_destroy (uiomux);
    exit (0);
  }

  uiomux_close (uiomux);
}
