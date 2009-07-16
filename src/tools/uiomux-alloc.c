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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

#include <uiomux/uiomux.h>

static struct uiomux * uiomux;

void sig_handler(int sig)
{
#ifdef DEBUG
        fprintf (stderr, "Got signal %d\n", sig);
#endif

	uiomux_close (uiomux);

        /* Send ourselves the signal: see http://www.cons.org/cracauer/sigint.html */
        signal(sig, SIG_DFL);
        kill(getpid(), sig);
}

void
alloc (int argc, char *argv[])
{
	size_t len = 1024;
	long val;

	if ((uiomux = uiomux_open()) == NULL)
		return;

	signal (SIGINT, sig_handler);

	if (argc > 2) {
		errno = 0;
		val = strtol (argv[2], NULL, 0);
		if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
		    (errno != 0 && val == 0)) {
			perror("strtol");
			exit(EXIT_FAILURE);
		}
		len = val;
	}

	printf ("Attempting to allocate %d bytes ...\n", len);

	if (uiomux_malloc (uiomux, UIOMUX_SH_VPU, len, 32) == NULL) {
		fprintf (stderr, "uiomux: unable to allocate %d bytes\n", len);
	} else {
		pause ();
	}

	uiomux_close (uiomux);
}
