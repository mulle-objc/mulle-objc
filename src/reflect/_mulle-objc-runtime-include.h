/*
 *   This file will be regenerated by `mulle-sde reflect` and any edits will be
 *   lost. Suppress generation of this file with:
 *      mulle-sde environment --global \
 *         set MULLE_SOURCETREE_TO_C_INCLUDE_FILE DISABLE
 *
 *   To not generate any header files:
 *      mulle-sde environment --global \
 *         set MULLE_SOURCETREE_TO_C_RUN DISABLE
 */

#ifndef _mulle_objc_runtime_include_h__
#define _mulle_objc_runtime_include_h__

// How to tweak the following mulle-concurrent #include
//    remove:             `mulle-sourcetree mark mulle-concurrent no-header`
//    rename:             `mulle-sde dependency|library set mulle-concurrent include whatever.h`
//    toggle #import:     `mulle-sourcetree mark mulle-concurrent [no-]import`
//    toggle public:      `mulle-sourcetree mark mulle-concurrent [no-]public`
//    toggle optional:    `mulle-sourcetree mark mulle-concurrent [no-]require`
//    remove for os:      `mulle-sourcetree mark mulle-concurrent no-os-<osname>`
# include <mulle-concurrent/mulle-concurrent.h>   // mulle-concurrent

// How to tweak the following mulle-vararg #include
//    remove:             `mulle-sourcetree mark mulle-vararg no-header`
//    rename:             `mulle-sde dependency|library set mulle-vararg include whatever.h`
//    toggle #import:     `mulle-sourcetree mark mulle-vararg [no-]import`
//    toggle public:      `mulle-sourcetree mark mulle-vararg [no-]public`
//    toggle optional:    `mulle-sourcetree mark mulle-vararg [no-]require`
//    remove for os:      `mulle-sourcetree mark mulle-vararg no-os-<osname>`
# include <mulle-vararg/mulle-vararg.h>   // mulle-vararg

// How to tweak the following mulle-data #include
//    remove:             `mulle-sourcetree mark mulle-data no-header`
//    rename:             `mulle-sde dependency|library set mulle-data include whatever.h`
//    toggle #import:     `mulle-sourcetree mark mulle-data [no-]import`
//    toggle public:      `mulle-sourcetree mark mulle-data [no-]public`
//    toggle optional:    `mulle-sourcetree mark mulle-data [no-]require`
//    remove for os:      `mulle-sourcetree mark mulle-data no-os-<osname>`
# include <mulle-data/mulle-data.h>   // mulle-data

#endif
