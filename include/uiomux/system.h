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

#ifndef __UIOMUX_SYSTEM_H__
#define __UIOMUX_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/** \file
 * UIOMux system API.
 *
 * These functions are used by the commandline tool uiomux, and should not
 * be called by normal applications.
 */

/**
 * Reset the UIOMux system, initializing the associated shared memory segment
 * and all shared mutexes. Note that this is done transparently by the
 * first process to call uiomux_open(), so this function does not need to be
 * used by normal applications. It is usually called by the commandline tool
 * 'uiomux reset'.
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_system_reset (UIOMux * uiomux);

/**
 * Destroy the UIOMux system, removing the associated shared memory segment
 * and destroying all shared mutexes. This will make UIOMux unusable by
 * other applications which have previously opened it, so must not be used
 * by normal applications. It is usually called by the commandline tool
 * 'uiomux destroy'.
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_system_destroy (UIOMux * uiomux);

#ifdef __cplusplus
}
#endif

#endif /* __UIOMUX_SYSTEM_H__ */

