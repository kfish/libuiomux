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
  uiomux = uiomux_open();
  if (uiomux == NULL)
    FAIL ("Opening UIOMux");

  if ((pid = fork()) < 0) {
    FAIL ("Forking");
  }

  if (pid == 0) {
    /* Child */
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Child locking BEU, will exit without unlocking");
    exit(1);
  } else {
    /* Parent */

    INFO ("Waiting for child to exit");
    waitpid (pid, NULL, 0);

    INFO ("Parent attempting to lock BEU.");
    INFO ("If this hangs, consider FAILed, kill and run 'uiomux reset'");
    uiomux_lock (uiomux, UIOMUX_SH_BEU);
    INFO ("Parent locked BEU");
    uiomux_unlock (uiomux, UIOMUX_SH_BEU);
    INFO ("Parent unlocked BEU");
  }

  INFO ("Closing UIOMux");
  ret = uiomux_close(uiomux);
  if (ret != 0)
    FAIL ("Closing UIOMux");

  exit (0);
}
