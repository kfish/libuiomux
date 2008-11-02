#ifndef __UIOMUX_ARCH_SH_H__
#define __UIOMUX_ARCH_SH_H__

/** \file
 * UIOMux resource identifiers for SuperH Mobile.
 * 
 * IP blocks are accessed by symbolic name.
 */

/**
 * The following block identifiers can be bitwise OR'd together to form a
 * \a uiomux_resource_t
 */

/** Blending Engine Unit */
#define UIOMUX_SH_BEU (1<<0)

/** Capture Engine Unit */
#define UIOMUX_SH_CEU (1<<1)

/** JPEG Unit */
#define UIOMUX_SH_JPU (1<<2)

/** Video Processing Unit */
#define UIOMUX_SH_VEU (1<<3)

/** Video Processing Unit */
#define UIOMUX_SH_VPU (1<<4)

#endif /* __UIOMUX_ARCH_SH_H__ */
