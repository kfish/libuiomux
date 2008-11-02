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

  usleep (10);

  uiomux_lock (uiomux, UIOMUX_SH_CEU);
  snprintf (buf, 256, "CEU Locked by thread %d", threadid);
  INFO (buf);
  uiomux_unlock (uiomux, UIOMUX_SH_CEU);

  usleep (5);

  uiomux_lock (uiomux, UIOMUX_SH_BEU);
  snprintf (buf, 256, "BEU Locked by thread %d", threadid);
  INFO (buf);
  uiomux_unlock (uiomux, UIOMUX_SH_BEU);

  usleep (1);

  uiomux_lock (uiomux, UIOMUX_SH_JPU);
  snprintf (buf, 256, "JPU Locked by thread %d", threadid);
  INFO (buf);
  uiomux_unlock (uiomux, UIOMUX_SH_JPU);

  return NULL;
}

int
main (int argc, char *argv[])
{
  pid_t pid;
  pthread_t threads[NR_THREADS];
  int i, ret, nums[NR_THREADS];
  char buf[256];


  INFO ("Opening UIOMux for BEU");
  uiomux = uiomux_open();
  if (uiomux == NULL)
    FAIL ("Opening UIOMux");

  for (i = 0; i < NR_THREADS; i++) {
    nums[i] = i;
    snprintf (buf, 256, "Creating thread %d", i);
    INFO (buf);
    pthread_create (&threads[i], NULL, thread_main, &nums[i]);
  }

  for (i = 0; i < NR_THREADS; i++) {
    snprintf (buf, 256, "Joining thread %d", i);
    INFO (buf);
    pthread_join (threads[i], NULL);
  }

  INFO ("Closing UIOMux");
  ret = uiomux_close(uiomux);
  if (ret != 0)
    FAIL ("Closing UIOMux");

  exit (0);
}
