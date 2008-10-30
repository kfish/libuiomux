#ifndef __UIOMUX_BLOCKS_H__
#define __UIOMUX_BLOCKS_H__

/** \file
 * UIOMux IP block identifiers
 * 
 * IP blocks are accessed by symbolic name.
 */

/*
 * % or, should these names be independent of the actual hardware, ie.
 * % should this library be arch-independent? obviously that is not
 * % a requirement atm, but would be nice in the long run
 * % then perhaps these should be more configurable
 */

/**
 * A mask of multiple IP blocks. A UIOMux handle corresponds to a set of
 * blocks atomically allocated together.
 */
typedef int uiomux_blockmask_t;

/**
 * The following block identifiers can be bitwise OR'd together to form a
 * \a uiomux_blockmask_t
 */

/** Specifies that no blocks are selected */
#define UIOMUX_BLOCK_NONE (0)

/** Blending Engine Unit */
#define UIOMUX_BLOCK_SH_BEU (1<<0)

/** Capture Engine Unit */
#define UIOMUX_BLOCK_SH_CEU (1<<1)

/** Video Processing Unit */
#define UIOMUX_BLOCK_SH_VPU (1<<2)

#endif /* __UIOMUX_BLOCKS_H__ */
