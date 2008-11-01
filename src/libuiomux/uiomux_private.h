#ifndef __UIOMUX_PRIVATE_H__
#define __UIOMUX_PRIVATE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <uiomux/uiomux_blocks.h>

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

struct uiomux_state {
  /* The base address of the memory map */
  void * proper_address;

  /* Version of this state */
  int version;

  /* Number of blocks allocated and initialized */
  int num_blocks;

  /* Mutexes */
  pthread_mutex_t mutex[UIOMUX_BLOCK_MAX];
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
uiomux_name(uiomux_blockmask_t block);

#endif /* __UIOMUX_PRIVATE_H__ */
