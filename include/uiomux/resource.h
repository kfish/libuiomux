#ifndef __UIOMUX_RESOURCE_H__
#define __UIOMUX_RESOURCE_H__

/**
 * A mask of multiple resources. A UIOMux handle corresponds to a set of
 * blocks atomically allocated together.
 */
typedef int uiomux_resource_t;

/** Specifies that no resources are selected */
#define UIOMUX_NONE 0

#include <uiomux/arch_sh.h>

#endif /* __UIOMUX_RESOURCE_H__ */
