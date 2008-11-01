#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

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

  if ((pid = fork()) < 0) {
    FAIL ("Forking");
  }

  if (pid == 0) {
    /* Child */
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by child");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by child");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by child");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
  } else {
    /* Parent */
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by parent");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by parent");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Locked by parent");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);

    waitpid (pid, NULL, 0);
  }

  INFO ("Closing UIOMux");
  ret = uiomux_close(uiomux);
  if (ret != 0)
    FAIL ("Closing UIOMux");

  exit (0);
}
