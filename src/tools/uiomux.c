/*
 * UIOMux: a conflict manager for system resources, including UIO devices.
 * Copyright (C) 2009 Renesas Technology Corp.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uiomux/uiomux.h>

extern void alloc (int argc, char *argv[]);

static void
version (void)
{
  printf ("uiomux version " VERSION "\n");
}

static void
usage (void)
{
  printf ("Usage: uiomux <command>\n\n");

  printf ("uiomux is a tool for querying UIO and managing the UIOMux state.\n");

#if 0
  printf ("\nCommands:\n");
  printf ("  help        Display help for a specific command (eg. \"uiomux help info\")\n");
#endif

  printf ("\nReporting:\n");
  printf ("  query       List available UIO device names that can be managed by UIOMux.\n");
  printf ("  info        Show memory layout of each UIO device managed by UIOMux.\n");
  printf ("  meminfo     Show memory allocations of each UIO device managed by UIOMux.\n");

  printf ("\nManagement:\n");
  printf ("  reset       Reset the UIOMux system. This initializes the UIOMux shared state,\n");
  printf ("              including all shared mutexes, and scans UIO memory maps.\n");
  printf ("  destroy     Destroy the UIOMux system. This frees all resources used by the\n");
  printf ("              UIOMux shared state. Note that any subsequent program using UIOMux\n");
  printf ("              will reallocate and initialize this shared state, including this\n");
  printf ("              tool's 'info' and 'reset' commands.\n");

  printf ("\nUtilities:\n");
  printf ("  alloc <n>   Allocate a specified number of bytes.\n");

  printf ("\nOptions:\n");
  printf ("  --version   Show uiomux version info\n");

  printf ("\nPlease report bugs to <linux-sh@vger.kernel.org>\n");
}

static void
query (void)
{
  uiomux_resource_t blocks;
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

  if ((uiomux = uiomux_open ()) == NULL)
    return;

  uiomux_info (uiomux);
  uiomux_close (uiomux);
}

static void
meminfo (void)
{
  struct uiomux * uiomux;

  if ((uiomux = uiomux_open ()) == NULL)
    return;

  uiomux_meminfo (uiomux);
  uiomux_close (uiomux);
}

static void
reset (void)
{
  struct uiomux * uiomux;

  if ((uiomux = uiomux_open ()) == NULL)
    return;

  printf ("Resetting UIOMux system ...\n");
  uiomux_system_reset (uiomux);
  uiomux_close (uiomux);
}

static void
destroy (void)
{
  struct uiomux * uiomux;

  if ((uiomux = uiomux_open ()) == NULL)
    return;

  printf ("Destroying UIOMux system ...\n");
  uiomux_system_destroy (uiomux);
  exit (0);
}

int
main (int argc, char *argv[])
{
  int i;

  if (argc < 2) {
    usage();
    exit (1);
  }

  for (i=1; i<argc; i++) {
    if (!strncmp(argv[i], "--version", 10)) {
      version();
      exit (0);
    }
  }

  if (!strncmp (argv[1], "query", 6)) {
    query ();
  } else if (!strncmp (argv[1], "info", 5)) {
    info ();
  } else if (!strncmp (argv[1], "meminfo", 8)) {
    meminfo ();
  } else if (!strncmp (argv[1], "reset", 6)) {
    reset ();
  } else if (!strncmp (argv[1], "destroy", 8)) {
    destroy ();
  } else if (!strncmp (argv[1], "alloc", 6)) {
    alloc (argc, argv);
  } else {
    usage();
    exit (1);
  }

  exit (0);
}
