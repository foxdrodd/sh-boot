/* $Id: linkage.h,v 1.2 2000/02/27 01:47:52 gniibe Exp $
 *
 */
#ifndef __LINKAGE_H
#define __LINKAGE_H

#ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#define asmlinkage CPP_ASMLINKAGE

#define STRINGIFY(X) #X
#define SYMBOL_NAME_STR(X) STRINGIFY(SYMBOL_NAME(X))
#define SYMBOL_NAME(X) X
#define SYMBOL_NAME_LABEL(X) X/**/:

#define __ALIGN .balign 4
#define __ALIGN_STR ".balign 4"

#ifdef __ASSEMBLY__

#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#define ENTRY(name) \
  .globl SYMBOL_NAME(name); \
  ALIGN; \
  SYMBOL_NAME_LABEL(name)

#endif

#endif
