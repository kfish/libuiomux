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

  return 0;
}

struct uio *
uio_open (const char * name)
{
  struct uio * uio;
  int ret;

  uio = (struct uio *) calloc (1, sizeof (struct uio));
  if (uio == NULL) return NULL;

  ret = locate_uio_device(name, &uio->dev);
  if (ret < 0) {
    uio_close (uio);
    return NULL;
  }
	
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
