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

#include <uiomux/uiomux.h>

#include "uiomux_tests.h"

int
main (int argc, char *argv[])
{
  UIOMux * uiomux, * uiomux_2;
  int ret;

  INFO ("Opening first UIOMux");
  uiomux = uiomux_open();
  if (uiomux == NULL)
    FAIL ("Opening first UIOMux");

  INFO ("Opening second UIOMux");
  uiomux_2 = uiomux_open();
  if (uiomux_2 == NULL)
    FAIL ("Opening second UIOMux");

  INFO ("Closing second UIOMux");
  ret = uiomux_close(uiomux_2);
  if (ret != 0)
    FAIL ("Closing second UIOMux");

  INFO ("Closing first UIOMux");
  ret = uiomux_close(uiomux);
  if (ret != 0)
    FAIL ("Closing first UIOMux");


  exit (0);
}
