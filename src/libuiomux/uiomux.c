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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include "uiomux_private.h"
#include "uio.h"

/* #define DEBUG */

/* #define SAVE_RESTORE_REGISTERS */

static int uiomux_unlock_all(struct uiomux *uiomux)
{
	pthread_mutex_t *mutex;
	int i;

	/* unlock mutexes */
	for (i = UIOMUX_BLOCK_MAX - 1; i >= 0; i--) {
#ifdef DEBUG
		fprintf(stderr, "%s: Unlocking block %d\n", __func__, i);
#endif
		mutex = &uiomux->shared_state->mutex[i].mutex;
		pthread_mutex_unlock(mutex);
	}

	return 0;
}

/*
 * Deallocate all memory.
 */
static int uiomux_reset_mem(struct uiomux *uiomux)
{
	int i, j, owners_len;
	char *o;

	owners_len =
	    uiomux->shared_state->size - sizeof(struct uiomux_state);
	o = (char *) uiomux->shared_state + sizeof(struct uiomux_state);

	memset(o, 0, owners_len);

	return 0;
}

/*
 * Deallocate memory which is allocated to dead processes.
 */
static int uiomux_update_mem(struct uiomux *uiomux)
{
	int i, j, owners_len;
	char *o;
	pid_t *p, known_bad[8], known_good[8];
	int kb = 0, kg = 0;	/* indices into known_ tables */
	char fname[256];
	int ret;
	struct stat statbuf;

	memset(known_bad, 0, sizeof(known_bad));
	memset(known_good, 0, sizeof(known_good));

	owners_len =
	    (uiomux->shared_state->size -
	     sizeof(struct uiomux_state)) / sizeof(pid_t);
	o = (char *) uiomux->shared_state + sizeof(struct uiomux_state);
	p = (pid_t *) o;

	for (i = 0; i < owners_len; i++) {
		if (*p != 0) {
			/* Check cached good values */
			for (j = 0; j < kg; j++) {
				if (*p == known_good[j]) {
					goto update_next;
				}
			}
			/* Check cached bad values */
			for (j = 0; j < kb; j++) {
				if (*p == known_bad[j]) {
					*p = 0;
					goto update_next;
				}
			}

			/* Check against /proc/pid */
			snprintf(fname, 256, "/proc/%d", *p);
			if (stat(fname, &statbuf) == 0) {
				/* /proc/pid good */
				if (kg < 8) {
					known_good[kg] = *p;
					kg++;
				}
			} else {
				/* /proc/pid bad */
				if (kb < 8) {
					known_bad[kb] = *p;
					kb++;
				}
				*p = 0;
			}
		}

	      update_next:
		p++;
	}

	return 0;
}

/*
 * Deallocate memory allocated to current process.
 */
static int uiomux_free_mem(struct uiomux *uiomux)
{
	int i, j, owners_len;
	char *o;
	pid_t *p, mypid;

	mypid = getpid();

	owners_len =
	    (uiomux->shared_state->size -
	     sizeof(struct uiomux_state)) / sizeof(pid_t);
	o = (char *) uiomux->shared_state + sizeof(struct uiomux_state);
	p = (pid_t *) o;

	for (i = 0; i < owners_len; i++) {
		if (*p == mypid)
			*p = 0;
		p++;
	}

	return 0;
}

/**
 * uiomux_on_exit()
 *
 * This function is registered by uiomux_open() to be called on exit to deallocate
 * any resources still held by this UIOMux* handle. In particular, it ensures to
 * unlock any held mutexes before exiting, otherwise they remain locked in the
 * system.
 *
 * In order to do so, it first attempts to check if the UIOMux* handle is still
 * valid, ie. whether or not it has already been freed. It does this by checking
 * the shared_state value, which is the first member of struct uiomux. We ensure
 * that this is valid by delaying the free() of uiomux until program exit, which
 * is why uiomux_delete() below frees everything but the uiomux* itself.
 *
 * This mechanism avoids the following error being reported by valgrind in the
 * common case where uiomux has* already been freed:
 *
 * ==31337== Invalid read of size 4
 * ==31337==    at 0x403C909: uiomux_on_exit (uiomux.c:51)
 * ==31337==    by 0x408D528: exit (in /lib/tls/i686/cmov/libc-2.6.1.so)
 * ==31337==    by 0x8048699: main (uiomux.c:108)
 * ==31337==  Address 0x41AB028 is 0 bytes inside a block of size 196 free'd
 * ==31337==    at 0x402237F: free (vg_replace_malloc.c:233)
 * ==31337==    by 0x403C7E0: uiomux_delete (uiomux.c:111)
 * ==31337==    by 0x403C8E1: uiomux_close (uiomux.c:122)
 * ==31337==    by 0x804868D: main (uiomux.c:65)
 */
static void uiomux_on_exit(int exit_status, void *arg)
{
	struct uiomux *uiomux = (struct uiomux *) arg;

	if (uiomux == NULL)
		return;

#ifdef DEBUG
	fprintf(stderr, "%s: IN\n", __func__);
#endif

	/* Only attempt the unlock and free if the shared_state is still correctly
	 * mapped at its proper address */
	if (uiomux->shared_state
	    && uiomux->shared_state->proper_address ==
	    uiomux->shared_state) {
		uiomux_close(uiomux);
	}

	/* Finally, free uiomux */
	free(uiomux);
}

struct uiomux *uiomux_open(void)
{
	struct uiomux *uiomux;
	struct uiomux_state *state;
	struct uiomux_block *block;
	const char *name = NULL;
	int i;

	/* Get the shared state, creating and initializing it if necessary */
	state = get_shared_state();

	if (state == NULL)
		return NULL;

	uiomux = (struct uiomux *) calloc(1, sizeof(*uiomux));

	uiomux->shared_state = state;

	/* Allocate space for register store */
	for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
		block = &uiomux->blocks[i];
		if ((name = uiomux_name(1 << i)) != NULL) {
			if ((block->uio = uio_open(name)) != NULL) {
#ifdef DEBUG
				fprintf(stderr,
					"%s: Allocating %ld bytes for %s registers...\n",
					__func__, block->uio->mmio.size,
					block->uio->dev.name);
#endif
				block->nr_registers =
				    block->uio->mmio.size / 4;
				block->registers =
				    (long *) malloc(block->uio->mmio.size);
			}
		}
	}

	/* Register on_exit() cleanup function */
	on_exit(uiomux_on_exit, uiomux);

	/* Update memory allocs */
	uiomux_update_mem(uiomux);

	return uiomux;
}

static void uiomux_delete(struct uiomux *uiomux)
{
	struct uiomux_block *block;
	int i;

	/* Mark this as NULL to invalidate uiomux for uiomux_on_exit */
	uiomux->shared_state = NULL;

	for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
		block = &uiomux->blocks[i];
		uio_close(block->uio);
		if (block->registers != NULL)
			free(block->registers);
	}

	/* uiomux will be free'd on exit */
}

int uiomux_close(struct uiomux *uiomux)
{
	if (uiomux == NULL)
		return -1;

#ifdef DEBUG
	fprintf(stderr, "%s: IN\n", __func__);
#endif

	uiomux_free_mem(uiomux);
	uiomux_unlock_all(uiomux);
	unmap_shared_state(uiomux->shared_state);

	uiomux_delete(uiomux);

	return 0;
}

int uiomux_system_reset(struct uiomux *uiomux)
{
	if (uiomux == NULL)
		return -1;

	if (uiomux->shared_state == NULL) {
		uiomux->shared_state = get_shared_state();
	}

	if (uiomux->shared_state == NULL) {
		fprintf(stderr, "Incorrect version, cannot reset.\n");
		return -1;
	}

	init_shared_state(uiomux->shared_state);
	uiomux_reset_mem(uiomux);

	return 0;
}

int uiomux_system_destroy(struct uiomux *uiomux)
{
	destroy_shared_state(uiomux->shared_state);

	uiomux_delete(uiomux);

	return 0;
}

int uiomux_lock(struct uiomux *uiomux, uiomux_resource_t blockmask)
{
	pthread_mutex_t *mutex;
	struct uiomux_block *block;
	unsigned long *reg_base;
	int i, k, ret;

	for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
		if (blockmask & (1 << i)) {
			/* lock mutex */
#ifdef DEBUG
			fprintf(stderr, "%s: PID %d locking block %d\n",
				__func__, getpid(), i);
#endif
			mutex = &uiomux->shared_state->mutex[i].mutex;
			ret = pthread_mutex_lock(mutex);
			if (ret != 0)
				fprintf(stderr,
					"%s: FAILED Locking block %d\n",
					__func__, i);
#ifdef DEBUG
			else
				fprintf(stderr,
					"%s: PID %d LOCKED block %d\n",
					__func__, getpid(), i);
#endif


#ifdef SAVE_RESTORE_REGISTERS
			/* restore registers */
			if (!pthread_equal
			    (uiomux->shared_state->mutex[i].prev_holder,
			     pthread_self())) {
				block = &uiomux->blocks[i];
				if (block->uio) {
					reg_base = block->uio->mmio.iomem;
					for (k = 0;
					     k < block->nr_registers;
					     k++) {
						reg_base[k] =
						    block->registers[k];
					}
				}
			}
#endif

		}
	}


	return 0;
}

int uiomux_unlock(struct uiomux *uiomux, uiomux_resource_t blockmask)
{
	pthread_mutex_t *mutex;
	struct uiomux_block *block;
	unsigned long *reg_base;
	int i, k, ret;

	for (i = UIOMUX_BLOCK_MAX - 1; i >= 0; i--) {
		if (blockmask & (1 << i)) {
#ifdef SAVE_RESTORE_REGISTERS
			/* store registers */
			block = &uiomux->blocks[i];
			if (block->uio) {
				reg_base = block->uio->mmio.iomem;
				for (k = 0; k < block->nr_registers; k++) {
					block->registers[k] = reg_base[k];
				}
			}
#endif

			/* record last holder */
			uiomux->shared_state->mutex[i].prev_holder =
			    pthread_self();

			/* unlock mutex */
#ifdef DEBUG
			fprintf(stderr, "%s: PID %d unlocking block %d\n",
				__func__, getpid(), i);
#endif
			mutex = &uiomux->shared_state->mutex[i].mutex;
			ret = pthread_mutex_unlock(mutex);
			if (ret != 0)
				fprintf(stderr,
					"%s: FAILED Unlocking block %d\n",
					__func__, i);
#ifdef DEBUG
			else
				fprintf(stderr,
					"%s: PID %d UNLOCKED block %d\n",
					__func__, getpid(), i);
#endif
		}
	}

	return 0;
}

#define MULTI_BIT(x) (((long)x)&(((long)x)-1))

static int
uiomux_get_block_index(struct uiomux *uiomux, uiomux_resource_t blockmask)
{
	struct uiomux_block *block;
	int i;

	/* Invalid if multiple bits are set */
	if (MULTI_BIT(blockmask)) {
#ifdef DEBUG
		fprintf(stderr, "%s: Multiple blocks specified\n",
			__func__);
#endif
		return -1;
	}

	for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
		if (blockmask & (1 << i)) {
			return i;
		}
	}

	return -1;
}

long uiomux_sleep(struct uiomux *uiomux, uiomux_resource_t blockmask)
{
	struct uiomux_block *block;
	long ret = 0;
	int i;

	/* Invalid if multiple bits are set, or block not found */
	if ((i = uiomux_get_block_index(uiomux, blockmask)) == -1)
		return -1;

	block = &uiomux->blocks[i];

	if (block->uio) {
#ifdef DEBUG
		fprintf(stderr, "%s: Waiting for block %d\n", __func__, i);
#endif
		ret = uio_sleep(block->uio);
	}

	return ret;
}

void *uiomux_malloc(struct uiomux *uiomux, uiomux_resource_t blockmask,
		    size_t size, int align)
{
	pthread_mutex_t *mutex;
	struct uiomux_block *block;
	void *ret = NULL;
	int i;

	/* Invalid if multiple bits are set, or block not found */
	if ((i = uiomux_get_block_index(uiomux, blockmask)) == -1)
		return NULL;

	block = &uiomux->blocks[i];

	if (block->uio) {
#ifdef DEBUG
		fprintf(stderr, "%s: Allocating %d bytes for block %d\n",
			__func__, size, i);
#endif
		mutex = &uiomux->shared_state->mutex[i].mutex;
		pthread_mutex_lock(mutex);

		ret = uio_malloc(block->uio,
				 uiomux->shared_state->owners[i],
				 size, align);

		pthread_mutex_unlock(mutex);
	}

	return ret;
}

void
uiomux_free(struct uiomux *uiomux, uiomux_resource_t blockmask,
	    void *address, size_t size)
{
	pthread_mutex_t *mutex;
	struct uiomux_block *block;
	int i;

	/* Invalid if multiple bits are set, or block not found */
	if ((i = uiomux_get_block_index(uiomux, blockmask)) == -1)
		return;

	block = &uiomux->blocks[i];

	if (block->uio) {
#ifdef DEBUG
		fprintf(stderr, "%s: Freeing memory for block %d\n",
			__func__, i);
#endif
		mutex = &uiomux->shared_state->mutex[i].mutex;
		pthread_mutex_lock(mutex);

		uio_free(block->uio, uiomux->shared_state->owners[i],
			 address, size);

		pthread_mutex_unlock(mutex);
	}
}

unsigned long
uiomux_get_mmio(struct uiomux *uiomux, uiomux_resource_t blockmask,
		unsigned long *address, unsigned long *size, void **iomem)
{
	struct uiomux_block *block;
	int i;

	/* Invalid if multiple bits are set, or block not found */
	if ((i = uiomux_get_block_index(uiomux, blockmask)) == -1)
		return 0;

	block = &uiomux->blocks[i];

	/* Invalid if no uio associated with it */
	if (block->uio == NULL)
		return 0;

	if (address)
		*address = block->uio->mmio.address;
	if (size)
		*size = block->uio->mmio.size;
	if (iomem)
		*iomem = block->uio->mmio.iomem;

	return block->uio->mmio.address;
}

unsigned long
uiomux_get_mem(struct uiomux *uiomux, uiomux_resource_t blockmask,
	       unsigned long *address, unsigned long *size, void **iomem)
{
	struct uiomux_block *block;
	int i;

	/* Invalid if multiple bits are set, or block not found */
	if ((i = uiomux_get_block_index(uiomux, blockmask)) == -1)
		return 0;

	/* Invalid if no uio associated with it */
	if (block->uio == NULL)
		return 0;

	block = &uiomux->blocks[i];

	if (address)
		*address = block->uio->mem.address;
	if (size)
		*size = block->uio->mem.size;
	if (iomem)
		*iomem = block->uio->mem.iomem;

	return block->uio->mem.address;
}

static unsigned long
uio_map_virt_to_phys(struct uio_map *map, void *virt_address)
{
	if ((virt_address >= map->iomem)
	    && ((unsigned long) (virt_address - map->iomem) < map->size))
		return map->address +
		    ((unsigned long) (virt_address - map->iomem));

	return (unsigned long) -1;
}

static void *
uio_map_phys_to_virt(struct uio_map *map, long phys_address)
{
	if ((phys_address >= map->address)
	    && ((unsigned long) (phys_address - map->address) < map->size))
		return map->iomem + (phys_address - map->address);

	return NULL;
}

unsigned long
uiomux_virt_to_phys(struct uiomux *uiomux, uiomux_resource_t blockmask,
		    void *virt_address)
{
	struct uiomux_block *block;
	unsigned long ret;
	int i;

	/* Invalid if multiple bits are set, or block not found */
	if ((i = uiomux_get_block_index(uiomux, blockmask)) == -1)
		return 0;

	block = &uiomux->blocks[i];

	/* Invalid if no uio associated with it */
	if (block->uio == NULL)
		return 0;

	if ((ret =
	     uio_map_virt_to_phys(&block->uio->mem,
				  virt_address)) != (unsigned long) -1)
		return ret;

	if ((ret =
	     uio_map_virt_to_phys(&block->uio->mmio,
				  virt_address)) != (unsigned long) -1)
		return ret;

	return 0;
}

void *
uiomux_phys_to_virt(struct uiomux *uiomux, uiomux_resource_t blockmask,
		    unsigned long phys_address)
{
	struct uiomux_block *block;
	void * ret;
	int i;

	/* Invalid if multiple bits are set, or block not found */
	if ((i = uiomux_get_block_index(uiomux, blockmask)) == -1)
		return NULL;

	block = &uiomux->blocks[i];

	/* Invalid if no uio associated with it */
	if (block->uio == NULL)
		return NULL;

	if ((ret =
	     uio_map_phys_to_virt(&block->uio->mem,
				  phys_address)) != (unsigned long) -1)
		return ret;

	if ((ret =
	     uio_map_phys_to_virt(&block->uio->mmio,
				  phys_address)) != (unsigned long) -1)
		return ret;

	return NULL;
}

const char *uiomux_name(uiomux_resource_t block)
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
		return "VPU5";
		break;
	default:
		return NULL;
	}
}

uiomux_resource_t uiomux_query(void)
{
	uiomux_resource_t blocks = UIOMUX_NONE;
	struct uio *uio;
	const char *name = NULL;
	int i;

	for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
		if ((name = uiomux_name(1 << i)) != NULL) {
			if ((uio = uio_open(name)) != NULL) {
				blocks |= (1 << i);
				uio_close(uio);
			}
		}
	}

	return blocks;
}

static int uiomux_showversion(struct uiomux *uiomux)
{
	printf("uiomux " VERSION ", built for shared state version %d\n",
	       UIOMUX_STATE_VERSION);
	fflush(stdout);

	printf("Current runtime state version %d\n",
	       uiomux->shared_state->version);

	return 0;
}

int uiomux_info(struct uiomux *uiomux)
{
	uiomux_resource_t blocks = UIOMUX_NONE;
	struct uiomux_state *state;
	struct uiomux_block *block;
	int i;
	long pagesize;

	uiomux_showversion(uiomux);

	pagesize = sysconf(_SC_PAGESIZE);

	for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
		block = &uiomux->blocks[i];
		if (block->uio != NULL) {
			printf("%s: %s", block->uio->dev.path,
			       block->uio->dev.name);
			printf
			    ("\tmmio\t0x%8lx\t0x%8lx bytes (%ld pages)\n\tmem\t0x%8lx\t0x%8lx bytes (%ld pages)\n",
			     block->uio->mmio.address,
			     block->uio->mmio.size,
			     block->uio->mmio.size / pagesize,
			     block->uio->mem.address, block->uio->mem.size,
			     block->uio->mem.size / pagesize);
		}
	}

	return 0;
}

int uiomux_meminfo(struct uiomux *uiomux)
{
	uiomux_resource_t blocks = UIOMUX_NONE;
	struct uiomux_state *state;
	struct uiomux_block *block;
	int i;
	long pagesize;

	uiomux_showversion(uiomux);

	pagesize = sysconf(_SC_PAGESIZE);

	for (i = 0; i < UIOMUX_BLOCK_MAX; i++) {
		block = &uiomux->blocks[i];
		if (block->uio != NULL) {
			printf("%s: %s", block->uio->dev.path,
			       block->uio->dev.name);
			uio_meminfo(block->uio,
				    uiomux->shared_state->owners[i]);
		}
	}

	return 0;
}
