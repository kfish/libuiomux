
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include "uiomux_private.h"

/* #define DEBUG */

/* name for shm_open */
#define UIOMUX_SHM_NAME "/uiomux-shm"

/***********************************************************
 */

#ifdef DEBUG
#define debug_perror(s) perror(s)
#define debug_info(s) fprintf(stderr, s "\n")
#else
#define debug_perror(s)
#define debug_info(s)
#endif

int
init_shared_state (struct uiomux_state * state)
{
  struct uiomux_block * block;
  pthread_mutexattr_t attr;
  pthread_mutex_t * mutex;
  struct uio * uio;
  int i;

  state->version = UIOMUX_STATE_VERSION;
  state->num_blocks = UIOMUX_BLOCK_MAX;

  pthread_mutexattr_init (&attr);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

  for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
    /* Initialize mutex, regardless */
    mutex = &state->mutex[i].mutex;
    pthread_mutex_init (mutex, &attr);
  }

  pthread_mutexattr_destroy (&attr);

  return 0;
}

static struct uiomux_state *
create_shared_state (void)
{
  struct uiomux_state * state;
  int shm_descr;
  size_t size;

  size = sizeof(struct uiomux_state);

  shm_descr = shm_open (UIOMUX_SHM_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRWXU);
  if (shm_descr == -1) {
    debug_perror ("create_shraed_state: shm_open");
    return NULL;
  }

  if (ftruncate (shm_descr, size) < 0 ) {
    debug_perror ("create_shraed_state: ftruncate");
  }

  state = (struct uiomux_state *) mmap(0, size, PROT_READ | PROT_WRITE,
                                       MAP_SHARED, shm_descr, (long)0);
  if (state == MAP_FAILED) {
    debug_perror ("create_shraed_state: mmap");
    return NULL;
  }

  close (shm_descr);

  state->proper_address = (void *)state;
  
  return state;
}

static struct uiomux_state *
map_shared_state (void)
{
  struct uiomux_state * state;
  int shm_descr;
  void * exact_address;
  size_t size;

  debug_info ("map_shared_state: IN");

  size = sizeof(struct uiomux_state);

  shm_descr = shm_open (UIOMUX_SHM_NAME, O_RDWR, S_IRWXU);
  if (shm_descr == -1) {
    debug_perror ("map_shared_state: shm_open");
    return NULL;
  }

  debug_info ("map_shared_state: mmap ...");
  state = (struct uiomux_state *) mmap(0, size, PROT_READ | PROT_WRITE,
                                       MAP_SHARED, shm_descr, (long)0);
  if (state == MAP_FAILED) {
    debug_perror ("map_shared_state: mmap");
    close (shm_descr);
    return NULL;
  }

  if (state->proper_address != (void *)state) {
    debug_info ("map_shared_state: Not mapped at proper address, trying with MAP_FIXED ...");
    exact_address = state->proper_address;
#ifdef DEBUG
    fprintf (stderr, "map_shared_state: exact_address %x\n", exact_address);
#endif
    munmap ((void *)state, size);
    state = (struct uiomux_state *) mmap(exact_address, size, PROT_READ | PROT_WRITE,
                                         MAP_SHARED | MAP_FIXED, shm_descr, (long)0);
    if (state == MAP_FAILED) {
      debug_perror ("map_shared_state: mmap (MAP_FIXED)");
      close (shm_descr);
      return NULL;
    }
  }

  if (state->version != UIOMUX_STATE_VERSION || state->num_blocks < UIOMUX_BLOCK_MAX) {
    debug_info ("map_shared_state: Incorrect version or num_blocks, unmapping and closing ...");
    munmap ((void *)state, size);
    close (shm_descr);
    /* XXX: Set error code for incorrect UIOMux version */
    return NULL;
  }

  close (shm_descr);

  return state;
}

int
unmap_shared_state (struct uiomux_state * state)
{
  if (state == NULL) return -1;
  if (state != state->proper_address) return -2;

  return munmap ((void *)state, sizeof (struct uiomux_state));
}

int
destroy_shared_state (struct uiomux_state * state)
{
  pthread_mutex_t * mutex;
  int i, ret;

  if (state == NULL) return -1;
  if (state->proper_address != state) return -1;

  for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
    /* Destroy mutex */
    mutex = &state->mutex[i].mutex;
    ret = pthread_mutex_destroy (mutex);
    if (ret == EBUSY) {
#ifdef DEBUG
      perror ("pthread_mutex_destroy");
#endif
    }
  }

  munmap ((void *)state, sizeof (struct uiomux_state));

  if (shm_unlink (UIOMUX_SHM_NAME) < 0) {
    debug_perror ("shm_unlink");
  }

  return 0;
}

struct uiomux_state *
get_shared_state (void)
{
  struct uiomux_state * state;

  state = create_shared_state ();

  if (state == NULL) {
    switch (errno) {
    case EEXIST:
#ifdef DEBUG
      fprintf (stderr, "get_shared_state: already created, mapping ...\n");
#endif
      state = map_shared_state ();
      break;
    default:
      perror ("uiomux: Could not create shared state");
      break;
    }
  } else {
#ifdef DEBUG
    fprintf (stderr, "get_shared_state: created, initializing ...\n");
#endif
    init_shared_state (state);
  }

  return state;
}
