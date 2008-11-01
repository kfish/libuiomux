#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include <uiomux/uiomux.h>

#include "uiomux_tests.h"

int
main (int argc, char *argv[])
{
  UIOMux * uiomux;
  pid_t pid;
  int ret;

  INFO ("Opening UIOMux for BEU");
  uiomux = uiomux_open(UIOMUX_SH_BEU);
  if (uiomux == NULL)
    FAIL ("Opening UIOMux");

  uiomux_lock (uiomux, UIOMUX_SH_BEU);
  INFO ("Locked");
  uiomux_unlock (uiomux, UIOMUX_SH_BEU);
  INFO ("Unlocked");

  INFO ("Closing UIOMux");
  ret = uiomux_close(uiomux);
  if (ret != 0)
    FAIL ("Closing UIOMux");

  exit (0);
}
