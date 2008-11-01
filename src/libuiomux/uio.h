
#ifndef __UIOMUX_UIO_H__
#define __UIOMUX_UIO_H__

struct uio_device {
  char *name;
  char *path;
  int fd;
};

struct uio_map {
  unsigned long address;
  unsigned long size;
  void *iomem;
};

struct uio {
  struct uio_device dev;
  struct uio_map mmio;
  struct uio_map mem;
};

struct uio *
uio_open (const char * name);

int
uio_close (struct uio * uio);

#endif /* __UIOMUX_UIO_H__ */
