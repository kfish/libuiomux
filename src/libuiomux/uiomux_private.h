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

#ifndef __UIOMUX_PRIVATE_H__
#define __UIOMUX_PRIVATE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <uiomux/resource.h>

#include "uio.h"

/***********************************************************
 * Library-private defines
 */

#define UIOMUX_BLOCK_MAX 16

#define UIOMUX_STATE_VERSION 1

/***********************************************************
 * Library-private Types
 */

struct uiomux_block {
  struct uio * uio;
  int nr_registers;
  long * registers;
};

struct uiomux_mutex {
  pthread_mutex_t mutex;
  pthread_t prev_holder;
};

struct uiomux_state {
  /* The base address of the memory map */
  void * proper_address;

  /* Version of this state */
  int version;

  /* Number of blocks allocated and initialized */
  int num_blocks;

  /* Mutexes */
  struct uiomux_mutex mutex[UIOMUX_BLOCK_MAX];
};

struct uiomux {
  /* Shared state */
  struct uiomux_state * shared_state;

  /* Blocks */
  struct uiomux_block blocks[UIOMUX_BLOCK_MAX]; 
};

/***********************************************************
 * Library-private functions
 */

struct uiomux_state *
get_shared_state (void);

int
init_shared_state (struct uiomux_state * state);

int
unmap_shared_state (struct uiomux_state * state);

int
destroy_shared_state (struct uiomux_state * state);

int
uiomux_close (struct uiomux * uiomux);

const char *
uiomux_name(uiomux_resource_t resource);

#endif /* __UIOMUX_PRIVATE_H__ */
