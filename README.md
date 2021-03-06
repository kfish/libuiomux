UIOMux
======

UIOMux is a conflict manager for system resources, including UIO devices.

Copyright (C) 2009 Renesas Technology Corp.

This source archive contains:

  * libuiomux: the libuiomux shared library
  * uiomux: a commandline tool for querying and manipulating UIOMux


Installation
------------

This archive uses GNU autotools. For full usage instructions see the file
INSTALL. The conventional build procedure applies:

    $ ./configure
    $ make
    $ make install

To run various unit tests on the build host, do:

    $ make check


Overview
--------

UIOMux multiplexes access to named resources, including devices which are
available via UIO.

UIOMux consists of a user-level shared library, libuiomux, which manages
a shared memory segment containing mutexes for each managed resource. This
segment and its mutexes are shared amongst all processes and threads on
the system, to provide system-wide locking. In this way, libuiomux can be
used to manage contention across multiple simultaneous processes and threads.

UIOMux allows simultaneous locking of access to multiple resources, with
deterministic locking and unlocking order to avoid circular waiting.
Processes or threads requiring simultaneous access to more than one resource
should lock and unlock them simultaneously via libuiomux.

UIO devices report activity through a read of their file descriptor. UIOMux
provides a simplified interface for waiting for a UIO managed resource.

UIOMux provides functions for allocating the UIO memory reserved by the
kernel for each device. Allocations are tied to the calling process ID, and
are cleared on process exit.

UIOMux can save and restore memory-mapped IO registers associated with a
UIO device. Registers are saved on uiomux_unlock() and restored on
uiomux_lock(), if intervening users have used the device.

Additionally, UIOMux can be queried for whether or not a resource is available
on the currently running system.


Commandline tool
----------------

    Usage: uiomux <command>
    
    uiomux is a tool for querying UIO and managing the UIOMux state.
    
    Reporting:
      query       List available UIO device names that can be managed by UIOMux.
      info        Show memory layout of each UIO device managed by UIOMux.
      meminfo     Show memory allocations of each UIO device managed by UIOMux.
    
    Management:
      reset       Reset the UIOMux system. This initializes the UIOMux shared state,
                  including all shared mutexes, and scans UIO memory maps.
      destroy     Destroy the UIOMux system. This frees all resources used by the
                  UIOMux shared state. Note that any subsequent program using UIOMux
                  will reallocate and initialize this shared state, including this
                  tool's 'info' and 'reset' commands.

    Utilities:
      alloc <n>   Allocate a specified number of bytes.


libuiomux API
-------------

  * Documentation

If doxygen is installed, API documentation will be built in doc/libuiomux. HTML
documentation is created by default in doc/libuiomux/html. A PDF version can be
created if LaTeX is installed; in this case, run 'make' in doc/libuiomux/latex.

  * Summary

libuiomux can be used to manage various named resources. Currently the
following names are defined:

    UIOMUX_SH_BEU
    UIOMUX_SH_CEU
    UIOMUX_SH_JPU
    UIOMUX_SH_VEU
    UIOMUX_SH_VPU

These identifiers are bitmasked together as the return value from
uiomux_query(), or as arguments to uiomux_lock() and uiomux_unlock().

At any time, an application may retrieve a printable name for a resource:

    const char * name;

    name = uiomux_name(resource);

To query which resources are available on the running system, call:

    uiomux_blockmask_t available_resources;

    available_resources = uiomux_query();

The return value is a bitmask consisting of some of the above symbols OR'd
together. An application may use this function to fall back to a software
implementation if a needed hardware resource is not available, or to disable
relevant funtionality at runtime.

A process or thread wishing to use the locking facilities of libuiomux
should start by calling uiomux_open() to obtain a UIOMux* handle:

    UIOMux * uiomux;

    uiomux = uiomux_open();

To request exclusive access to a resource or a set of resources, call
uiomux_lock():

    uiomux_blockmask_t resources;

    uiomux_lock(resources);

where resources is the name of a resource, or multiple resource names OR'd
together. Each call to uiomux_lock() must be paired with a corresponding
call to uiomux_unlock():

    uiomux_unlock(resources);

Failure to unlock can lead to system-wide starvation of the locked resource.
Note however that all locks obtained via libuiomux will be automatically
unlocked on program termination to minimize the potential damage caused by
rogue processes.

UIO devices report activity through a read of their file descriptor. A
simplified interface is offered for waiting for a UIO managed resource:

    uiomux_sleep (uiomux, resource);

UIOMux also provides a co-operative memory allocation scheme. To allocate
memory from a UIO managed resource:

    uiomux_malloc (uiomux, resource, size, alignment);

To free allocated memory from a UIO managed resource:

    uiomux_free (uiomux, resource, address, size);

Note that any outstanding allocations are removed on process exit, and any
invalid allocations are cleared on uiomux_open().

Finally, each process or thread that opened a UIOMux* handle should
close it by calling uiomux_close(). This will remove associated memory maps,
unlock locked resources and mark used memory for deallocation:

    uiomux_close(uiomux);


License
-------

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA  02110-1301 USA

See the file COPYING for details.
