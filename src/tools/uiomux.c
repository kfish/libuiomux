
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
  printf ("Usage: uiomux <command> [options]\n\n");

  printf ("uiomux is a tool for querying UIO and managing the UIOMux state.\n");

#if 0
  printf ("\nCommands:\n");
  printf ("  help        Display help for a specific command (eg. \"uiomux help info\")\n");
#endif

  printf ("\nReporting:\n");
  printf ("  query       List available UIO device names that can be managed by UIOMux.\n");
  printf ("  info        Show memory layout of each UIO device managed by UIOMux.\n");

  printf ("\nManagement:\n");
  printf ("  reset       Reset the UIOMux system. This initializes the UIOMux shared state,\n");
  printf ("              including all shared mutexes, and scans UIO memory maps.\n");
  printf ("  destroy     Destroy the UIOMux system. This frees all resources used by the\n");
  printf ("              UIOMux shared state. Note that any subsequent program using UIOMux\n");
  printf ("              will reallocate and initialize this shared state, including this\n");
  printf ("              tool's 'info' and 'reset' commands.\n");
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
  printf ("Resetting UIOMux system ...\n");
  uiomux_system_reset (uiomux);
  uiomux_close (uiomux);
}

static void
destroy (void)
{
  struct uiomux * uiomux;

  uiomux = uiomux_open (0);
  printf ("Destroying UIOMux system ...\n");
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

  exit (0);
}
