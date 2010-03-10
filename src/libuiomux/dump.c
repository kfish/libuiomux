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
#include <stdarg.h>

#define FNAME_MAXLINE 4096

struct uiomux;
typedef int uiomux_resource_t;
unsigned long
uiomux_get_mmio(struct uiomux *uiomux, uiomux_resource_t blockmask,
		unsigned long *address, unsigned long *size, void **iomem);

int uiomux_dump_mmio (FILE * stream, struct uiomux *uiomux, uiomux_resource_t blockmask)
{
	unsigned long address, size, *cur;
	unsigned long i, n;
	int tot=0, ret;

	address = uiomux_get_mmio (uiomux, blockmask, NULL,
				   &size, (void **)&cur);

	if (address == 0) return -1;

	n = size / sizeof(unsigned long);

	for (i=0; i<n; i++) {
		if ((ret=fprintf (stream, "%04lx:\t%08lx\n", i, *cur++)) < 0)
			return ret;
		tot += ret;
	}

	return tot;
}

int uiomux_dump_mmio_filename (struct uiomux *uiomux, uiomux_resource_t blockmask,
			       const char *fmt, ...)
{
	va_list ap;
	char buf[FNAME_MAXLINE];
	FILE * fp;
	int ret;

	va_start (ap, fmt);
	vsnprintf (buf, FNAME_MAXLINE, fmt, ap);
	va_end (ap);

	if ((fp = fopen (buf, "w")) == NULL)
		return -1;

	ret = uiomux_dump_mmio (fp, uiomux, blockmask);

	fclose (fp);

	return ret;
}
