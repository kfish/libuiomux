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

#ifndef __UIOMUX_ARCH_SH_H__
#define __UIOMUX_ARCH_SH_H__

/** \file
 * UIOMux resource identifiers for SuperH Mobile.
 * 
 * IP blocks are accessed by symbolic name.
 */

/**
 * The following block identifiers can be bitwise OR'd together to form a
 * \a uiomux_resource_t
 */

/** Blending Engine Unit */
#define UIOMUX_SH_BEU (1<<0)

/** Capture Engine Unit */
#define UIOMUX_SH_CEU (1<<1)

/** JPEG Unit */
#define UIOMUX_SH_JPU (1<<2)

/** Video Processing Unit */
#define UIOMUX_SH_VEU (1<<3)

/** Video Processing Unit */
#define UIOMUX_SH_VPU (1<<4)

#endif /* __UIOMUX_ARCH_SH_H__ */
