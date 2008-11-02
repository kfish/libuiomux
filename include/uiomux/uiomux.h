
#ifndef __UIOMUX_H__
#define __UIOMUX_H__

#include <uiomux/resource.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \mainpage
 *
 * \section intro UIOMux: A conflict manager for system resources, including UIO devices.
 *
 * This is the documentation for the UIOMux C API.
 *
 * The user-level shared library libuiomux manages a shared memory segment
 * containing mutexes for each managed resource. This segment and its mutexes
 * are shared amongst all processes and threads on the system, to provide
 * system-wide locking. In this way, libuiomux can be used to manage
 * contention across multiple simultaneous processes and threads.  
 *
 * UIOMux allows simultaneous locking of access to multiple resources, with
 * deterministic locking and unlocking order to avoid circular waiting.
 * Processes or threads requring simultaneous access to more than one resource
 * should lock and unlock them simultaneously via libuiomux.
 *
 * UIOMux will save and restore of memory-mapped IO registers associated with
 * a UIO device. Registers are saved on uiomux_unlock() and restored on
 * uiomux_lock(), if intervening users have used the device.
 *
 * Additionally, UIOMux can be queried for whether or not a resource is
 * available on the currently running system. This allows a vendor to
 * create a single codec or application binary that works across a family
 * of similar processors. An application can fall back to software if a
 * required hardware resource is not available, or disable relevant
 * funtionality at runtime.
 * 
 * \subsection contents Contents
 * 
 * - \link uiomux.h uiomux.h \endlink:
 * Documentation of the UIOMux C API
 *
 * - \link configuration Configuration \endlink:
 * Customizing libuiomux
 *
 * - \link building Building \endlink:
 * Building applications against libuiomux
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
 PKG_CHECK_MODULES(UIOMUX, uiomux >= 0.6.0,
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
 * \section query Query
 *
 * At any time, an application may retrieve a printable name for a resource
 * by calling uiomux_name(). 
 * To query which resources are available on the running system, call
 * uiomux_query().
 *
 * \section locking Locking and unlocking
 *
 * A process or thread wishing to use the locking facilities of libuiomux
 * should start by calling uiomux_open() to obtain a UIOMux* handle.
 *
 * To request exclusive access to a resource or a set of resources, call
 * uiomux_lock(), passing as argument the name of a resource, or multiple
 * resource names OR'd together. Each call to uiomux_lock() must be paired
 * with a corresponding call to uiomux_unlock().  Failure to unlock can
 * lead to system-wide starvation of the locked resource. Note however that
 * all locks obtained via libuiomux will be automatically unlocked on program
 * termination to minimize the potential damage caused by rogue processes.
 *
 * Finally, each process or thread that opened a UIOMux* handle should
 * close it by calling uiomux_close(). This will remove associated memory maps,
 * unlock locked resources and mark used memory for deallocation.
 */

/**
 * An opaque handle to a set of blocks. This is returned by uiomux_open(),
 * and is passed to other uiomux_* functions.
 */
typedef void UIOMux;

/**
 * Query which blocks are available on this platform.
 * \retval Bitwise OR of available resource names.
 */
uiomux_resource_t
uiomux_query(void);

/**
 * Retrieve a printable name for an IP block:
 */
const char * uiomux_name(uiomux_resource_t resource);

/**
 * Print info about UIO maps to stdout
 * \param uiomux A UIOMux handle
 * \retval 0 Success
 */
int
uiomux_info (UIOMux * uiomux);

/**
 * Create a new UIOMux object,
 * 
 * \retval NULL on system error; check errno for details.
 */
UIOMux *
uiomux_open (void);

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
 * \param resources A named resource, or multiple OR'd together
 * \retval 0 Success
 */
int
uiomux_lock (UIOMux * uiomux, uiomux_resource_t resources);

/**
 * Unlock a UIOMux handle for access to specified blocks.
 * \param uiomux A UIOMux handle
 * \param resources A named resource, or multiple OR'd together
 * \retval 0 Success
 */
int
uiomux_unlock (UIOMux * uiomux, uiomux_resource_t resources);

#include <uiomux/system.h>

#ifdef __cplusplus
}
#endif

#endif /* __UIOMUX_H__ */

