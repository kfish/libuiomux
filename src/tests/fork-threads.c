#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <pthread.h>

#include <uiomux/uiomux.h>

#include "uiomux_tests.h"

#define NR_THREADS 10

static UIOMux * uiomux;

void *
thread_main (void * arg)
{
  int threadid = * (int *)arg;
  char buf[256];


  uiomux_lock (uiomux, UIOMUX_SH_BEU);
  snprintf (buf, 256, "BEU Locked by thread %d", threadid);
  INFO (buf);
  uiomux_unlock (uiomux, UIOMUX_SH_BEU);

  usleep (7);

  uiomux_lock (uiomux, UIOMUX_SH_CEU);
  snprintf (buf, 256, "CEU Locked by thread %d", threadid);
  INFO (buf);
  uiomux_unlock (uiomux, UIOMUX_SH_CEU);

  usleep (3);

  uiomux_lock (uiomux, UIOMUX_SH_BEU | UIOMUX_SH_VEU);
  snprintf (buf, 256, "BEU and VEU Locked by thread %d", threadid);
  INFO (buf);
  uiomux_unlock (uiomux, UIOMUX_SH_BEU | UIOMUX_SH_VEU);

  usleep (1);

  uiomux_lock (uiomux, UIOMUX_SH_JPU);
  snprintf (buf, 256, "JPU Locked by thread %d", threadid);
  INFO (buf);
  uiomux_unlock (uiomux, UIOMUX_SH_JPU);

  return NULL;
}

void
fork_main (char * who)
{
  pthread_t threads[NR_THREADS];
  int i, nums[NR_THREADS];
  char buf[256];

  for (i = 0; i < NR_THREADS; i++) {
    nums[i] = i;
    snprintf (buf, 256, "%s creating thread %d", who, i);
    INFO (buf);
    pthread_create (&threads[i], NULL, thread_main, &nums[i]);
  }

  for (i = 0; i < NR_THREADS; i++) {
    snprintf (buf, 256, "%s joining thread %d", who, i);
    INFO (buf);
    pthread_join (threads[i], NULL);
  }
}

int
main (int argc, char *argv[])
{
  pid_t pid;
  int  ret;

  INFO ("Opening UIOMux for BEU");
  uiomux = uiomux_open();
  if (uiomux == NULL)
    FAIL ("Opening UIOMux");

  if ((pid = fork()) < 0) {
    FAIL ("Forking");
  }

  if (pid == 0) {
    /* Child */
    fork_main ("Child");
  } else {
    /* Parent */
    fork_main ("Parent");
    waitpid (pid, NULL, 0);
  }

  INFO ("Closing UIOMux");
  ret = uiomux_close(uiomux);
  if (ret != 0)
    FAIL ("Closing UIOMux");

  exit (0);
}
