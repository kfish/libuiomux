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

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#include <uiomux/uiomux.h>

#include "uiomux_tests.h"

int
main (int argc, char *argv[])
{
  UIOMux * uiomux;
  pid_t pid;
  int ret;

  INFO ("Opening UIOMux for BEU");
  uiomux = uiomux_open();
  if (uiomux == NULL)
    FAIL ("Opening UIOMux");

  if ((pid = fork()) < 0) {
    FAIL ("Forking");
  }

  if (pid == 0) {
    /* Child */
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by child");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by child");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by child");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
  } else {
    /* Parent */
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by parent");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by parent");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by parent");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);

    waitpid (pid, NULL, 0);
  }

  INFO ("Closing UIOMux");
  ret = uiomux_close(uiomux);
  if (ret != 0)
    FAIL ("Closing UIOMux");

  exit (0);
}
