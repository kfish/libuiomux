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

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "uio.h"

/* #define DEBUG */

static int fgets_with_openclose(char *fname, char *buf, size_t maxlen)
{
	FILE *fp;

	if ((fp = fopen(fname, "r")) != NULL) {
		fgets(buf, maxlen, fp);
		fclose(fp);
		return strlen(buf);
	} else {
		return -1;
	}
}


#define MAXUIOIDS  100
#define MAXNAMELEN 256

static int locate_uio_device(const char *name, struct uio_device *udp)
{
	char fname[MAXNAMELEN], buf[MAXNAMELEN];
	int uio_id, i;

	for (uio_id = 0; uio_id < MAXUIOIDS; uio_id++) {
		sprintf(fname, "/sys/class/uio/uio%d/name", uio_id);
		if (fgets_with_openclose(fname, buf, MAXNAMELEN) < 0)
			continue;
		if (strncmp(name, buf, strlen(name)) == 0)
			break;
	}

	if (uio_id >= MAXUIOIDS)
		return -1;

	udp->name = strdup(buf);
	udp->path = strdup(fname);
	udp->path[strlen(udp->path) - 4] = '\0';

	sprintf(buf, "/dev/uio%d", uio_id);
	udp->fd = open(buf, O_RDWR | O_SYNC /*| O_NONBLOCK */ );

	if (udp->fd < 0) {
		perror("open");
		return -1;
	}

	return 0;
}

static int setup_uio_map(struct uio_device *udp, int nr,
			 struct uio_map *ump)
{
	char fname[MAXNAMELEN], buf[MAXNAMELEN];

	sprintf(fname, "%s/maps/map%d/addr", udp->path, nr);
	if (fgets_with_openclose(fname, buf, MAXNAMELEN) <= 0)
		return -1;

	ump->address = strtoul(buf, NULL, 0);

	sprintf(fname, "%s/maps/map%d/size", udp->path, nr);
	if (fgets_with_openclose(fname, buf, MAXNAMELEN) <= 0)
		return -1;

	ump->size = strtoul(buf, NULL, 0);

	ump->iomem = mmap(0, ump->size,
			  PROT_READ | PROT_WRITE, MAP_SHARED,
			  udp->fd, nr * sysconf(_SC_PAGESIZE));

	if (ump->iomem == MAP_FAILED)
		return -1;

	return 0;
}

int uio_close(struct uio *uio)
{
	if (uio == NULL)
		return -1;

	if (uio->mem.iomem)
		munmap(uio->mem.iomem, uio->mem.size);

	if (uio->mmio.iomem)
		munmap(uio->mmio.iomem, uio->mmio.size);

	if (uio->dev.fd > 0)
		close(uio->dev.fd);
	if (uio->dev.path)
		free(uio->dev.path);
	if (uio->dev.name)
		free(uio->dev.name);

	free(uio);

	return 0;
}

struct uio *uio_open(const char *name)
{
	struct uio *uio;
	int ret;

	uio = (struct uio *) calloc(1, sizeof(struct uio));
	if (uio == NULL)
		return NULL;

	ret = locate_uio_device(name, &uio->dev);
	if (ret < 0) {
		uio_close(uio);
		return NULL;
	}
#ifdef DEBUG
	printf("uio_open: Found matching UIO device at %s\n",
	       uio->dev.path);
#endif

	ret = setup_uio_map(&uio->dev, 0, &uio->mmio);
	if (ret < 0) {
		uio_close(uio);
		return NULL;
	}

	ret = setup_uio_map(&uio->dev, 1, &uio->mem);
	if (ret < 0) {
		uio_close(uio);
		return NULL;
	}

	return uio;
}

long uio_sleep(struct uio *uio)
{
	int fd;

	fd = uio->dev.fd;

	/* Enable interrupt in UIO driver */
	{
		unsigned long enable = 1;

		write(fd, &enable, sizeof(u_long));
	}

	/* Wait for an interrupt */
	{
		unsigned long n_pending;

		read(fd, &n_pending, sizeof(u_long));
	}

	//m4iph_vpu4_int_handler();

	//avcbd_idr_adjust( global_context );
	return 0;
}

/* Returns index */
static int uio_mem_find(pid_t * owners, int max, int count)
{
	int i, c, base = -1;

	for (i = 0; i < max; i++) {
		if (base == -1) {	/* No start yet */
			if (owners[i] == 0) {
				base = i;
				c = 1;
			}
		} else {	/* Got a base */
			if (owners[i] == 0) {
				c++;
			} else {
				base = -1;
				c = 0;
			}
		}
		if (c == count) {
#ifdef DEBUG
			fprintf(stderr, "%s: Found %d available pages at index %d\n",
				__func__, c, base);
#endif
			return base;
		}
	}

	return -1;
}

static int
uio_mem_alloc_to(pid_t * owners, int offset, int count, pid_t pid)
{
	pid_t *p = &owners[offset];
	int i;

	for (i = 0; i < count; i++)
		*p++ = pid;

	return 0;
}

static int uio_mem_alloc(pid_t * owners, int offset, int count)
{
	return uio_mem_alloc_to(owners, offset, count, getpid());
}

static int uio_mem_free(pid_t * owners, int offset, int count)
{
	return uio_mem_alloc_to(owners, offset, count, 0);
}

void *uio_malloc(struct uio *uio, pid_t * owners, size_t size, int align)
{
	unsigned long mem_base;
	int pagesize, pages_req, pages_max;
	int base;

	if (uio->mem.address == 0) {
		fprintf(stderr,
			"%s: Allocation failed: uio->mem.address NULL\n",
			__func__);
		return NULL;
	}

	pagesize = sysconf(_SC_PAGESIZE);

	pages_max = uio->mem.size / pagesize;
	pages_req = (size / pagesize) + 1;

	if ((base = uio_mem_find(owners, pages_max, pages_req)) == -1)
		return NULL;

	uio_mem_alloc(owners, base, pages_req);
	mem_base = uio->mem.address + (base * pagesize);

	return (void *) mem_base;
}

void uio_free(struct uio *uio, pid_t * owners, void *address, size_t size)
{
	int pagesize, base, pages_req;

#ifdef DEBUG
	fprintf(stderr, "%s: IN\n", __func__);
#endif

	pagesize = sysconf(_SC_PAGESIZE);

	base = ((long) address - uio->mem.address) / pagesize;
	pages_req = (size / pagesize) + 1;
	uio_mem_free(owners, base, pages_req);
}

static void print_usage(int pid, long base, long top)
{
	char fname[MAXNAMELEN], cmdline[MAXNAMELEN];

	printf("0x%08lx-0x%08lx : ", base, top);

	if (pid == 0) {
		printf("----\n");
	} else {
		sprintf(fname, "/proc/%d/cmdline", pid);
		if (fgets_with_openclose(fname, cmdline, MAXNAMELEN) < 0) {
			printf("%d\n", pid);
		} else {
			printf("%d %s\n", pid, cmdline);
		}
	}
}


void uio_meminfo(struct uio *uio, pid_t * owners)
{
	int i, pagesize, pages;
	long addr, base, top;
	pid_t pid = 0, new_pid;

	pagesize = sysconf(_SC_PAGESIZE);
	pages = (uio->mem.size / pagesize) + 1;

	base = addr = uio->mem.address;
	top = addr + pagesize - 1;
	for (i = 0; i < pages; i++) {
		new_pid = *owners++;
		if (new_pid == pid) {
			top += pagesize;
		} else {
			if (i > 0) {
				/* Prev seg. ended */
				print_usage(pid, base, top);
			}
			base = addr;
			top = addr + pagesize - 1;
			pid = new_pid;
		}
		addr += pagesize;
	}
	print_usage(pid, base, top);
}
