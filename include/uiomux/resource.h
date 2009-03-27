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

#ifndef __UIOMUX_RESOURCE_H__
#define __UIOMUX_RESOURCE_H__

/** \file
 * UIOMux resource type.
 * 
 * IP blocks are accessed by symbolic name.
 * This file includes architecture-specific resource names.
 */

/**
 * A mask of multiple resources. A UIOMux handle corresponds to a set of
 * blocks atomically allocated together.
 */
typedef int uiomux_resource_t;

/** Specifies that no resources are selected */
#define UIOMUX_NONE 0

#include <uiomux/arch_sh.h>

#endif /* __UIOMUX_RESOURCE_H__ */
