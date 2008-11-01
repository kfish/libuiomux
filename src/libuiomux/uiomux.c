
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include "uiomux_private.h"
#include "uio.h"

/* #define DEBUG */

#if 0
static char * devname = "/dev/uio0";
#else
static char * devname = "/dev/zero";
#endif

static uiomux_blockmask_t allocated = UIOMUX_NONE;

static int
uiomux_unlock_all (struct uiomux * uiomux)
{
  pthread_mutex_t * mutex;
  int i;

  /* unlock mutexes */
  for (i=UIOMUX_BLOCK_MAX-1; i >= 0; i--) {
#ifdef DEBUG
    printf ("uiomux_unlock_all: Unlocking block %d\n", i);
#endif
    mutex = &uiomux->shared_state->mutex[i];
    pthread_mutex_unlock (mutex);
  }

  return 0;
}

static void
uiomux_on_exit (int exit_status, void * arg)
{
  struct uiomux * uiomux = (struct uiomux *)arg;

  if (uiomux == NULL) return;

#ifdef DEBUG
  fprintf (stderr, "uiomux_on_exit: IN\n");
#endif

  /* Only attempt the unlock and free if the shared_state is still correctly
   * mapped at its proper address */
  if (uiomux->shared_state && uiomux->shared_state->proper_address == uiomux->shared_state) {
    uiomux_close (uiomux);
#if 0
    uiomux_unlock_all (uiomux);
    unmap_shared_state(uiomux->shared_state); 
    free (uiomux);
#endif
  }
}

struct uiomux *
uiomux_open (uiomux_blockmask_t blocks)
{
  struct uiomux * uiomux;
  struct uiomux_state * state;
  struct uiomux_block * block;
  const char * name = NULL;
  int i;

  /* Get the shared state, creating and initializing it if necessary */
  state = get_shared_state();

  if (state == NULL) return NULL;

  uiomux = (struct uiomux *)calloc(1, sizeof(*uiomux));

  uiomux->shared_state = state;

  /* Allocate space for register store */
  for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
    block = &uiomux->blocks[i];
    if ((name = uiomux_name (1<<i)) != NULL) {
      if ((block->uio = uio_open (name)) != NULL) {
#ifdef DEBUG
        printf ("Allocating %ld bytes for %s registers...\n",
                block->uio->mmio.size, block->uio->dev.name);
#endif
        block->registers = (long *)malloc(block->uio->mmio.size);
      }
    }
  }

  /* Register on_exit() cleanup function */
  on_exit (uiomux_on_exit, uiomux);
  
  return uiomux;
}

int
uiomux_close (struct uiomux * uiomux)
{
  struct uiomux_block * block;
  int i;

  if (uiomux == NULL) return -1;

  uiomux_unlock_all (uiomux);
  unmap_shared_state (uiomux->shared_state);

  /* Mark this as NULL to invalidate uiomux for uiomux_on_exit */
  uiomux->shared_state = NULL;

  for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
    block = &uiomux->blocks[i];
    uio_close (block->uio);
    if (block->registers != NULL)
      free (block->registers);
  }

  free (uiomux);

  return 0;
}

int
uiomux_system_reset (struct uiomux * uiomux)
{
  if (uiomux == NULL) return -1;

  if (uiomux->shared_state == NULL) {
    uiomux->shared_state = get_shared_state ();
  }
    
  init_shared_state (uiomux->shared_state);

  return 0;
}

int
uiomux_system_destroy (struct uiomux * uiomux)
{
  destroy_shared_state (uiomux->shared_state);

  uiomux_close (uiomux);

  return 0;
}

int
uiomux_lock (struct uiomux * uiomux, uiomux_blockmask_t blockmask)
{
  pthread_mutex_t * mutex;
  int i;

  /* lock mutex */
  for (i=0; i < UIOMUX_BLOCK_MAX; i++) {
    if (blockmask & (1<<i)) {
#ifdef DEBUG
      printf ("Locking block %d\n", i);
      printf ("uiomux: %x\n", uiomux);
      printf ("uiomux->shared_state: %x\n", uiomux->shared_state);
#endif
      mutex = &uiomux->shared_state->mutex[i];
      pthread_mutex_lock (mutex);
    }
  }

  /* restore registers */

  return 0;
}

int
uiomux_unlock (struct uiomux * uiomux, uiomux_blockmask_t blockmask)
{
  pthread_mutex_t * mutex;
  int i;

  /* store registers */

  /* unlock mutex */
  for (i=UIOMUX_BLOCK_MAX-1; i >= 0; i--) {
    if (blockmask & (1<<i)) {
#ifdef DEBUG
      printf ("Unlocking block %d\n", i);
#endif
      mutex = &uiomux->shared_state->mutex[i];
      pthread_mutex_unlock (mutex);
    }
  }

  return 0;
}

const char *
uiomux_name(uiomux_blockmask_t block)
{
  switch (block) {
    case UIOMUX_SH_BEU:
      return "BEU";
      break;
    case UIOMUX_SH_CEU:
      return "CEU";
      break;
    case UIOMUX_SH_JPU:
      return "JPU";
      break;
    case UIOMUX_SH_VEU:
      return "VEU";
      break;
    case UIOMUX_SH_VPU:
      return "VPU";
      break;
    default:
      return NULL;
  }
}

uiomux_blockmask_t
uiomux_query(void)
{
  uiomux_blockmask_t blocks = UIOMUX_NONE;
  struct uio * uio;
  const char * name = NULL;
  int i;

  for (i=0; i < UIOMUX_BLOCK_MAX; i++) {
    if ((name = uiomux_name (1<<i)) != NULL) {
      if ((uio = uio_open (name)) != NULL) {
        blocks |= (1<<i);
        uio_close (uio);
      }
    }
  }

  return blocks;
}

int
uiomux_info (struct uiomux * uiomux)
{
  uiomux_blockmask_t blocks = UIOMUX_NONE;
  struct uiomux_state * state;
  struct uiomux_block * block;
  int i;

  for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
    block = &uiomux->blocks[i];
    if (block->uio != NULL) {
      printf ("%s: %s", block->uio->dev.path, block->uio->dev.name);
      printf ("\tmmio\t%8lx\t%-lx bytes\n\tmem\t%8lx\t%-lx bytes\n",
              block->uio->mmio.address, block->uio->mmio.size,
              block->uio->mem.address, block->uio->mem.size);
    }
  }

  return 0;
}

#if 0
UIOMux *
uiomux_open (uiomux_blockmask_t blocks)
{
  UIOMux * uiomux;

  /* Check if the allocation is allowed */
  if ((blocks & allocated) != 0) {
    return NULL;
  }

  /* If so, go for it ... */
  uiomux = malloc (sizeof(*uiomux));
  if ((uiomux->fd = open (devname, O_RDONLY)) == -1) {
    free (uiomux);
    return NULL;
  } 

  uiomux->blocks = blocks;

  allocated |= blocks;

  return uiomux;
}

int
uiomux_close (UIOMux * uiomux)
{
  if (uiomux == NULL) {
    return -1;
  }

  /* Deallocate blocks */
  allocated ^= uiomux->blocks;

  free (uiomux);

  return 0;
}

int
uiomux_poll(UIOMux * uiomux, uiomux_blockmask_t blocks)
{
  /* Check outstanding polls for requested blocks */

  /* otherwise, poll() */

  /* Check result against blockmask */

  /* Cache remaining blocks */

  return 0;
}

int
uiomux_read(UIOMux * uiomux)
{
  /* Read from UIO */

  return 0;
}
#endif

#if 0
int
uiomux_write(UIOMux * uiomux)
{
  /* Write to UIO */

  return 0;
}

int
uiomux_mmap(UIOMux * uiomux)
{
  /* Handle mmap */

  return 0;
}
#endif
