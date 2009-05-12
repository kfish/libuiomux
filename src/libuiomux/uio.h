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

#ifndef __UIOMUX_UIO_H__
#define __UIOMUX_UIO_H__

struct uio_device {
  char *name;
  char *path;
  int fd;
};

struct uio_map {
  unsigned long address;
  unsigned long size;
  void *iomem;
};

struct uio {
  struct uio_device dev;
  struct uio_map mmio;
  struct uio_map mem;
};

struct uio *
uio_open (const char * name);

int
uio_close (struct uio * uio);

long
uio_sleep (struct uio * uio);

void *
uio_malloc (struct uio * uio, unsigned long * mem_base_p,
            size_t size, int align);

void
uio_free (struct uio * uio, size_t size);

#endif /* __UIOMUX_UIO_H__ */
