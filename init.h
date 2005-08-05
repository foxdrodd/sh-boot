/*
 * Derived from include/linux/init.h
 */

#ifndef __INIT_H
#define __INIT_H

typedef int (*initcall_t) (void);
typedef void (*exitcall_t) (void);

extern int __initcall_start, __initcall_end;

#define __initcall(fn)								\
	static initcall_t __initcall_##fn __init_call = fn

#define __init		__attribute__ ((__section__ (".text.init")))
#define __init_call	__attribute__ ((unused,__section__ (".initcall.init")))

#endif				/* __INIT_H */
