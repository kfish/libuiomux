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

#ifndef __UIOMUX_DUMP_H__
#define __UIOMUX_DUMP_H__

#include <stdio.h>

/** \file
 * UIOMux memory-mapped I/O dump functions.
 *
 * This file contains debugging routines for dumping MMIO regions.
 */

/*
 * Dump the contents of the MMIO region for a UIO managed resource to a file handle.
 * \param stream A FILE handle
 * \param uiomux A UIOMux handle
 * \param resource A single named resource
 * \returns Number of characters written
 * \retval 0 Failure: resource not managed, or more than one resource given.
 * \retval <0 Failure: error writing to stream.
 */
int uiomux_dump_mmio (FILE * stream, UIOMux * uiomux, uiomux_resource_t resource);

/*
 * Dump the contents of the MMIO region for a UIO managed resource to a file.
 * The name of the file can be given as a format string.
 * \param uiomux A UIOMux handle
 * \param resource A single named resource
 * \param fmt The file name
 * \returns Number of characters written
 * \retval 0 Failure: resource not managed, or more than one resource given.
 * \retval <0 Failure: error writing to stream.
 */
int uiomux_dump_mmio_filename (UIOMux * uiomux, uiomux_resource_t resource,
			       const char *fmt, ...);

#endif /* __UIOMUX_DUMP_H__ */
