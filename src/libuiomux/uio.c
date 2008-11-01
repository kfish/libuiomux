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

static int fgets_with_openclose(char *fname, char *buf, size_t maxlen) {
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
	udp->fd = open(buf, O_RDWR|O_SYNC /*| O_NONBLOCK*/);

	if (udp->fd < 0) {
		perror("open");
		return -1;
	}

	return 0;
}

static int setup_uio_map(struct uio_device *udp, int nr, struct uio_map *ump)
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
			  PROT_READ|PROT_WRITE, MAP_SHARED,
			  udp->fd, nr * getpagesize());

	if (ump->iomem == MAP_FAILED)
		return -1;

	return 0;
}

struct uio_device uio_dev;
struct uio_map uio_mmio, uio_mem;

int
uio_close (struct uio * uio)
{
  if (uio == NULL) return -1;

  if (uio->mem.iomem)
    munmap (uio->mem.iomem, uio->mem.size);

  if (uio->mmio.iomem)
    munmap (uio->mmio.iomem, uio->mmio.size);

  if (uio->dev.fd > 0) close (uio->dev.fd);
  if (uio->dev.path) free (uio->dev.path);
  if (uio->dev.name) free (uio->dev.name);

  free (uio);
}

struct uio *
uio_open (const char * name)
{
  struct uio * uio;
  int ret;

  uio = (struct uio *) calloc (1, sizeof (struct uio));
  if (uio == NULL) return NULL;

  ret = locate_uio_device(name, &uio->dev);
  if (ret < 0) return NULL;
	
#ifdef DEBUG
  printf("uio_open: Found matching UIO device at %s\n", uio_dev.path);
#endif

  ret = setup_uio_map(&uio->dev, 0, &uio->mmio);
  if (ret < 0) {
    uio_close (uio);
    return NULL;
  }

  ret = setup_uio_map(&uio->dev, 1, &uio->mem);
  if (ret < 0) {
    uio_close (uio);
    return NULL;
  }

  return uio;
}

#if 0
unsigned long m4iph_reg_table_read(unsigned long *addr, unsigned long *data, long nr_longs)
{
	unsigned long *reg_base = uio_mmio.iomem;
	int k;

	reg_base += ((unsigned long)addr - VP4_CTRL) / 4;

	for (k = 0; k < nr_longs; k++)
		data[k] = reg_base[k];

#if DEBUG
	for (offset = 0; offset < nr_longs; offset++)
		printf("%s: addr = %p, data = %08lx\n", __FUNCTION__, addr++, *data++);
#endif
	return nr_longs;
}

void m4iph_reg_table_write(unsigned long *addr, unsigned long *data, long nr_longs)
{
	unsigned long *reg_base = uio_mmio.iomem;
	int k;

	reg_base += ((unsigned long)addr - VP4_CTRL) / 4;

	for (k = 0; k < nr_longs; k++)
		reg_base[k] = data[k];

#if DEBUG
	for (offset = 0; offset < nr_longs; offset++) {
		printf("%s: addr = %p, data = %08lx\n", __FUNCTION__, addr, *data);
		addr++;
		data++;
	}
#endif
}

int m4iph_sdr_open(void)
{
	sdr_base = sdr_start = uio_mem.address;
	sdr_end = sdr_base + uio_mem.size;
	return 0;
}

void m4iph_sdr_close(void)
{
	sdr_base = sdr_start = sdr_end = 0;
}

void *m4iph_map_sdr_mem(void *address, int size)
{
	return uio_mem.iomem + ((unsigned long)address - uio_mem.address);
}

int m4iph_unmap_sdr_mem(void *address, int size)
{
	return 0;
}

int m4iph_sync_sdr_mem(void *address, int size)
{
	return 0;
}

unsigned long m4iph_sdr_read(unsigned char *address, unsigned char *buffer,
				unsigned long count)
{
	unsigned char *buf;
	unsigned long addr;
	int roundoff;
	int pagesize = getpagesize();

	if ((unsigned long)address + count >= sdr_end || (unsigned long)address < sdr_start) {
		fprintf(stderr, "%s: Invalid read from SDR memory. ", __FUNCTION__);
		fprintf(stderr, "address = %p, count = %ld\n", address,	count);
		return 0;
	}
	addr = (unsigned long)address & ~(pagesize - 1);
	roundoff = (unsigned long)address - addr;
	buf = (unsigned char *)m4iph_map_sdr_mem((void *)addr, count + roundoff);
	if (buf == NULL) {
		printf("%s: Aborting since mmap() failed.\n", __FUNCTION__);
		printf("%s: address = %p, buffer = %p, count = %ld\n", __FUNCTION__, address, buffer, count);
		abort();
	}
	memcpy(buffer, buf + roundoff, count);
	m4iph_unmap_sdr_mem(buf, count + roundoff);
	return count;
}

void m4iph_sdr_write(unsigned char *address, unsigned char *buffer, 
		unsigned long count)
{
	unsigned char *buf;
	unsigned long addr;
	int roundoff;
	int pagesize = getpagesize();

	if ((unsigned long)address + count >= sdr_end || (unsigned long)address < sdr_start) {
		fprintf(stderr, "%s: Invalid write to SDR memory. ", __FUNCTION__);
		fprintf(stderr, "address = %p, count = %ld\n", address, count);
		return;
	}
	addr = (unsigned long)address & ~(pagesize - 1);
	roundoff = (unsigned long)address - addr;
	buf = (unsigned char *)m4iph_map_sdr_mem((void *)addr, count + roundoff);
	if (buf == NULL) {
		printf("%s: Aborting since mmap() failed.\n", __FUNCTION__);
		printf("%s: address = %p, buffer = %p, count = %ld\n", __FUNCTION__, address, buffer, count);
		abort();
	}
	memcpy(buf + roundoff, buffer, count);
	m4iph_unmap_sdr_mem(buf, count + roundoff);
}

void m4iph_sdr_memset(unsigned long *address, unsigned long data, unsigned long count)
{
	unsigned char *buf;
	unsigned long addr;
	int roundoff;
	int pagesize = getpagesize();

	addr = (unsigned long)address & ~(pagesize - 1);
	roundoff = (unsigned long)address - addr;
	buf = (unsigned char *)m4iph_map_sdr_mem((void *)addr, count + roundoff);
	if (buf == NULL) {
		printf("%s: Aborting since mmap() failed.\n", __FUNCTION__);
		printf("%s: address = %p, data = %08lx, count = %ld\n", __FUNCTION__, address, data, count);
		abort();
	}
	memset(buf, data, count);
	m4iph_unmap_sdr_mem(buf, count + roundoff);
}

/* Allocate sdr memory */
void *m4iph_sdr_malloc(unsigned long count, int align)
{
	unsigned long ret;
	int size;
	
	ret = ((sdr_base + (align - 1)) & ~(align - 1));
	size = ret - sdr_base + count;

	if (sdr_base + size >= sdr_end) {
		fprintf(stderr, "%s: Allocation of size %ld failed\n", __FUNCTION__, count);
		printf("sdr_base = %08lx, sdr_end = %08lx\n", sdr_base, sdr_end);
		return NULL;
	}
	sdr_base += size;
	return (void *)ret;
}
#endif
