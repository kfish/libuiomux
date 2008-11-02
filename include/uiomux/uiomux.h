
#ifndef __UIOMUX_H__
#define __UIOMUX_H__

#include <uiomux/uiomux_blocks.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \mainpage
 *
 * \section intro UIOMux: A multiplexor for accessing Userspace IO devices
 *
 * This is the documentation for the UIOMux C API.
 * The role of the resource allocation manager is to
 * mediate access to the custom IP blocks in the SuperH MobileR series SoC.
 *
 * UIOMux multiplexes access to the IP blocks which are made available via the
 * Linux kernel's UIO interface.
 *
 * Features:
 *  - Allows multiple IP blocks to be shared via a single kernel-provided
 *    UIO device
 *  - Efficiency: mediates access, but does not interfere with it.
 *    Users must understand that they co-operatively access the
 *    all devices available via UIO; it is not possible via this
 *    interface for the kernel to provide memory protection
 *
 *  - Portability: Allows userspace applications to query
 *    which blocks are available on this platform.
 *    the response to that can be configured at build time
 *    or set in a system config file (/etc/uiomux/uiomux.conf)
 *    but the point is to allow application portability
 *    and the possibility to ship a single codec binary or
 *    application binary that works on multiple similar chips
 *    (eg. an application could fall back to software if a needed
 *    block is not available, or disable relevant funtionality at
 *    runtime)
 * 
 * UIOMux keeps track of events for each * UIO block, and information
 * about the owner of each.
 * 
 * Attempts to access a claim access to a block that is already in use 
 * will fail, as will attempts to poll for events on a block that has
 * not been claimed.
 * 
 * The user of UIOMux can ask for multiple blocks atomically, so that if
 * it is not possible to get exclusive access to all of a group of blocks,
 * then the request fails. This interface is provided in order to
 * reduce contention and livelock, for example an applications requesting
 * exclusive access to both an encoder and decoder, or a camera and an
 * encoder. In order to specify operatiosn on multiple blocks, block
 * identifiers are bitmasked together.
 *
 * \subsection contents Contents
 * 
 * - \link uiomux.h uiomux.h \endlink:
 * Documentation of the UIOMux C API
 *
 * - \link configuration Configuration \endlink:
 * Customizing libuiomux to only read or write.
 *
 * - \link building Building \endlink:
 * \section API
 * 
 */

/** \defgroup configuration Configuration
 * \section configure ./configure
 *
 * It is possible to customize the functionality of libuiomux
 * by using various ./configure flags when
 * building it from source.
 *
 * For general information about using ./configure, see the file
 * \link install INSTALL \endlink
 *
 */

/** \defgroup install Installation
 * \section install INSTALL
 *
 * \include INSTALL
 */

/** \defgroup building Building against libuiomux
 *
 *
 * \section autoconf Using GNU autoconf
 *
 * If you are using GNU autoconf, you do not need to call pkg-config
 * directly. Use the following macro to determine if libuiomux is
 * available:
 *
 <pre>
 PKG_CHECK_MODULES(UIOMUX, uiomux >= 0.5.0,
                   HAVE_UIOMUX="yes", HAVE_UIOMUX="no")
 if test "x$HAVE_UIOMUX" = "xyes" ; then
   AC_SUBST(UIOMUX_CFLAGS)
   AC_SUBST(UIOMUX_LIBS)
 fi
 </pre>
 *
 * If libuiomux is found, HAVE_UIOMUX will be set to "yes", and
 * the autoconf variables UIOMUX_CFLAGS and UIOMUX_LIBS will
 * be set appropriately.
 *
 * \section pkg-config Determining compiler options with pkg-config
 *
 * If you are not using GNU autoconf in your project, you can use the
 * pkg-config tool directly to determine the correct compiler options.
 *
 <pre>
 UIOMUX_CFLAGS=`pkg-config --cflags uiomux`

 UIOMUX_LIBS=`pkg-config --libs uiomux`
 </pre>
 *
 */

/** \file
 * The libuiomux C API.
 *
 * \section general Generic semantics
 *
 * All access is managed via a UIOMux handle. This can be instantiated
 * by calling:
 * - uiomux_open() - Requested access to specified IP blocks
 *
 * To finish using a UIOMux handle, it should be closed with uiomux_close().
 */

/**
 * An opaque handle to a set of blocks. This is returned by uiomux_open(),
 * and is passed to all other uiomux_* functions.
 */
typedef void UIOMux;

/**
 * Query which blocks are available on this platform.
 * the response to that can be configured at build time
 * or set in a system config file (/etc/uiomux/uiomux.conf)
 * but the point is to allow application portability
 * and the possibility to ship a single codec binary or
 * application binary that works on multiple similar chips
 * (eg. an application could fall back to software if a needed
 * block is not available, or disable relevant funtionality at
 * runtime)
 */
uiomux_blockmask_t
uiomux_query(void);

/**
 * Retrieve a printable name for an IP block:
 */
const char * uiomux_name(uiomux_blockmask_t block);

/**
 * Print info about UIO maps to stdout
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_info (UIOMux * uiomux);

/**
 * Create a new UIOMux object,
 * gaining access to an IP block. This then allows access to a list of
 * maps relevant to that IP block, usable by a user of that IP block.
 * 
 * \retval NULL on system error; check errno for details
 */
UIOMux *
uiomux_open (uiomux_blockmask_t blockmask);

/**
 * Close a UIOMux handle, removing exclusive access, removing memory maps, etc.
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_close (UIOMux * uiomux);

/**
 * Lock a UIOMux handle for access to specified blocks.
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_lock (UIOMux * uiomux, uiomux_blockmask_t blockmask);

/**
 * Unlock a UIOMux handle for access to specified blocks.
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_unlock (UIOMux * uiomux, uiomux_blockmask_t blockmask);

/**
 * Destroy the UIOMux system, removing exclusive access, removing memory
 * maps, etc.
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_system_destroy (UIOMux * uiomux);

#ifdef __cplusplus
}
#endif

#endif /* __UIOMUX_H__ */

