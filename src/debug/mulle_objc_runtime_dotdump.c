//
//  mulle_objc_runtime_dotdump.c
//  mulle-objc
//
//  Created by Nat! on 25.10.15.
//  Copyright (c) 2015 Nat! - Mulle kybernetiK.
//  Copyright (c) 2015 Codeon GmbH.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.

//

//

#include "mulle_objc_runtime_dotdump.h"

#include "mulle_objc.h"
#include "mulle_objc_html.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mulle_thread/mulle_thread.h>
#include <mulle_concurrent/mulle_concurrent.h>

#include "c_set.inc"


//
// just don't output stuff with ampersands for now
// What is this used for ?
//
void   mulle_objc_methodlist_dump( struct _mulle_objc_methodlist *list)
{
   unsigned int   i;

   for( i = 0; i < list->n_methods; i++)
      printf( "{ "
              "name = \"%s\""
              "signature = \"%s\""
              "methodid = %08x"
              "bits = 0x%x"
              "implementation = %p"
              " }",
               list->methods[ i].descriptor.name,
               list->methods[ i].descriptor.signature,
               list->methods[ i].descriptor.methodid,
               list->methods[ i].descriptor.bits,
               list->methods[ i].implementation);
}


# pragma mark - "styling"

static struct _mulle_objc_colored_string    descriptortable_title =
{
   "descriptors",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    staticstringtable_title =
{
   "static strings",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    categorytable_title =
{
   "categories",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    protocoltable_title =
{
   "protocols",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    classestoload_title =
{
   "classes to load",
   "black",
   "white"
};


static struct _mulle_objc_colored_string    categoriestoload_title =
{
   "categories to load",
   "black",
   "white"
};



# pragma mark - walker runtime callback

static char  *mulle_objc_loadclasslist_html_row_description( intptr_t  classid, void *value)
{
   struct mulle_concurrent_pointerarray   *array = value;

   return( mulle_concurrent_pointerarray_html_description( array,
                                                           mulle_objc_loadclass_html_row_description,
                                                           NULL));
}


static char  *mulle_objc_loadcategorylist_html_row_description( intptr_t  classid, void *value)
{
   struct mulle_concurrent_pointerarray   *array = value;
   
   return( mulle_concurrent_pointerarray_html_description( array,
                                                           mulle_objc_loadcategory_html_row_description,
                                                           NULL));
}



static void   print_runtime( struct _mulle_objc_runtime *runtime, FILE *fp)
{
   char   *label;
   int    i;
   
   label = mulle_objc_runtime_html_description( runtime);
   fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n", runtime, label, "component");
   free( label);

   if( mulle_concurrent_hashmap_count( &runtime->descriptortable))
   {
      fprintf( fp, "\"%p\" -> \"%p\" [ label=\"descriptortable\" ];\n",
              runtime, &runtime->descriptortable);
      
      label = mulle_concurrent_hashmap_html_description( &runtime->descriptortable,
                                                         mulle_objc_methoddescriptor_html_row_description,
                                                         &descriptortable_title);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &runtime->descriptortable, label, "box");
      free( label);
   }
   
   for( i = 0; i < MULLE_OBJC_S_FASTCLASSES; i++)
      if( _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer))
         fprintf( fp, "\"%p\" -> \"%p\" [ label=\"fastclass #%d\" ];\n",
            runtime, _mulle_atomic_pointer_nonatomic_read( &runtime->fastclasstable.classes[ i].pointer), i);

   if( mulle_concurrent_pointerarray_get_count( &runtime->staticstrings))
   {
      label = mulle_concurrent_pointerarray_html_description( &runtime->staticstrings,
                                                              mulle_objc_staticstring_html_row_description,
                                                              &staticstringtable_title);
   
      fprintf( fp, "\"%p\" -> \"%p\" [ label=\"staticstrings\" ];\n",
              runtime, &runtime->staticstrings);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &runtime->staticstrings, label, "box");
      free( label);
   }

   if( mulle_concurrent_hashmap_count( &runtime->waitqueues.classestoload))
   {
      fprintf( fp, "\"%p\" -> \"%p\" [ label=\"classestoload\" ];\n",
              runtime, &runtime->waitqueues.classestoload);
      
      label = mulle_concurrent_hashmap_html_description( &runtime->waitqueues.classestoload,
                                                         mulle_objc_loadclasslist_html_row_description,
                                                         &classestoload_title);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &runtime->waitqueues.classestoload, label, "box");
      free( label);
   }

   if( mulle_concurrent_hashmap_count( &runtime->waitqueues.categoriestoload))
   {
      fprintf( fp, "\"%p\" -> \"%p\" [ label=\"categoriestoload\" ];\n",
              runtime, &runtime->waitqueues.categoriestoload);
      
      label = mulle_concurrent_hashmap_html_description( &runtime->waitqueues.categoriestoload,
                                                         mulle_objc_loadcategorylist_html_row_description,
                                                         &categoriestoload_title);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n",
              &runtime->waitqueues.categoriestoload, label, "box");
      free( label);
   }
   
   fprintf( fp, "\n\n");
}


# pragma mark - walker class callback

struct dump_info
{
   c_set  set;
   FILE   *fp;
};


static void   print_class( struct _mulle_objc_class *cls, FILE *fp, int is_meta)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   char                                             *label;
   struct _mulle_objc_cache                         *cache;
   struct _mulle_objc_class                         *superclass;
   struct _mulle_objc_infraclass                    *infra;
   struct _mulle_objc_metaclass                     *meta;
   struct _mulle_objc_methodlist                    *methodlist;
   unsigned int                                     i;

   label = mulle_objc_class_html_description( cls, is_meta ? "goldenrod" : "blue");
   fprintf( fp, "\"%p\" [ label=<%s>, shape=\"%s\" ];\n", cls, label, is_meta ? "component" : "box");
   free( label);

   if( _mulle_objc_class_get_runtime( cls))
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"runtime\" ];\n", cls,  _mulle_objc_class_get_runtime( cls));

   superclass = _mulle_objc_class_get_superclass( cls);
   if( superclass)
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"super\" ];\n", cls, superclass);

   meta = _mulle_objc_class_get_metaclass( cls);
   if( meta)
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"meta\" ];\n", cls, meta);

   infra = _mulle_objc_class_get_infraclass( cls);
   if( infra)
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"infra\" ];\n", cls, infra);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &cls->methodlists);
   while( methodlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( methodlist->n_methods)
      {
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"methodlist #%d\" ];\n",
                 cls, methodlist, i++);
         
         label = mulle_objc_methodlist_html_description( methodlist);
         fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", methodlist, label);
         free( label);
      }
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   
   cache = _mulle_objc_cachepivot_atomic_get_cache( &cls->cachepivot.pivot);
   if( cache->n)
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"cache\" ];\n",
              cls, cache);
      
      label = mulle_objc_cache_html_description( cache);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", cache, label);
      free( label);
   }
}


static void   print_classpair( struct _mulle_objc_class *cls, FILE *fp)
{
   char                                         *label;
   struct _mulle_objc_classpair                 *pair;
   struct _mulle_objc_infraclass                *prop_cls;
   struct _mulle_objc_protocolclassenumerator   rover;
   unsigned int                                 i;

   pair = _mulle_objc_class_get_classpair( cls);

   if( ! (_mulle_objc_class_get_inheritance( cls) & MULLE_OBJC_CLASS_DONT_INHERIT_PROTOCOLS))
   {
      i = 0;
      rover = _mulle_objc_classpair_enumerate_protocolclasses( pair);
      while( prop_cls = _mulle_objc_protocolclassenumerator_next( &rover))
      {
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"protocol inherit #%u\" ];\n",
                 cls, prop_cls, i++);
      }
      _mulle_objc_protocolclassenumerator_done( &rover);
   }
   fprintf( fp, "\n\n");

   if( mulle_concurrent_pointerarray_get_count( &pair->protocolids))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"protocolids\" ];\n",
              cls, &pair->protocolids);

      label = mulle_objc_protocols_html_description( &pair->protocolids,
                                                     &protocoltable_title);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", &pair->protocolids, label);
      free( label);
   }

   if( mulle_concurrent_pointerarray_get_count( &pair->categoryids))
   {
      fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"categoryids\" ];\n",
              cls, &pair->categoryids);

      label = mulle_objc_categories_html_description( &pair->categoryids,
                                                      &categorytable_title);
      fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", &pair->categoryids, label);
      free( label);
   }
}


static void   print_infraclass( struct _mulle_objc_infraclass *infra, FILE *fp)
{
   struct mulle_concurrent_pointerarrayenumerator   rover;
   char                                             *label;
   struct _mulle_objc_ivarlist                      *ivarlist;
   struct _mulle_objc_propertylist                  *propertylist;
   unsigned int                                     i;

   print_class( _mulle_objc_infraclass_as_class( infra), fp, 0);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &infra->ivarlists);
   while( ivarlist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( ivarlist->n_ivars)
      {
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"ivarlist #%d\" ];\n",
                 infra, ivarlist, i++);
         
         label = mulle_objc_ivarlist_html_description( ivarlist);
         fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", ivarlist, label);
         free( label);
      }
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   i = 0;
   rover = mulle_concurrent_pointerarray_enumerate( &infra->propertylists);
   while( propertylist = _mulle_concurrent_pointerarrayenumerator_next( &rover))
   {
      if( propertylist->n_properties)
      {
         fprintf( fp, "\"%p\" -> \"%p\"  [ label=\"propertylist #%d\" ];\n",
                 infra, propertylist, i++);
         
         label = mulle_objc_propertylist_html_description( propertylist);
         fprintf( fp, "\"%p\" [ label=<%s>, shape=\"none\" ];\n", propertylist, label);
         free( label);
      }
   }
   mulle_concurrent_pointerarrayenumerator_done( &rover);

   print_classpair( _mulle_objc_infraclass_as_class( infra), fp);
}


static void   print_metaclass( struct _mulle_objc_metaclass *cls, FILE *fp)
{
   print_class( _mulle_objc_metaclass_as_class( cls), fp, 1);

   print_classpair( _mulle_objc_metaclass_as_class( cls), fp);
}


static int   callback( struct _mulle_objc_runtime *runtime,
                       void *p,
                       enum mulle_objc_walkpointertype_t type,
                       char *key,
                       void *parent,
                       void *userinfo)
{
   FILE                            *fp;
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct dump_info                *info;

   assert( p);

   if( key)
      return( mulle_objc_walk_ok);

   info = userinfo;
   fp   = info->fp;

   if( c_set_member( &info->set, p))
      return( mulle_objc_walk_dont_descend);
   c_set_add( &info->set, p);

   switch( type)
   {
   case mulle_objc_walkpointer_is_category  :
   case mulle_objc_walkpointer_is_protocol  :
   case mulle_objc_walkpointer_is_classpair :
      break;

   case mulle_objc_walkpointer_is_runtime  :
      runtime = p;
      print_runtime( runtime, fp);
      break;

   case mulle_objc_walkpointer_is_infraclass :
      infra = p;
      print_infraclass( infra, fp);
      break;

   case mulle_objc_walkpointer_is_metaclass :
      meta = p;
      print_metaclass( meta, fp);
      break;

   case mulle_objc_walkpointer_is_method :
   case mulle_objc_walkpointer_is_property :
   case mulle_objc_walkpointer_is_ivar :
      break;
   }

   return( mulle_objc_walk_ok);
}


# pragma mark - runtime dump


void   _mulle_objc_runtime_dotdump( struct _mulle_objc_runtime *runtime, FILE *fp)
{
   struct dump_info  info;

   c_set_init( &info.set);
   info.fp = fp;

   fprintf( fp, "digraph mulle_objc_runtime\n{\n");
   mulle_objc_runtime_walk( runtime, callback, &info);
   fprintf( fp, "}\n");

   c_set_done( &info.set);
}


void   mulle_objc_dotdump_runtime( void)
{
   struct _mulle_objc_runtime   *runtime;

   runtime = mulle_objc_inlined_get_runtime();
   if( ! runtime)
      return;

   _mulle_objc_runtime_dotdump( runtime, stdout);
}


void   mulle_objc_dotdump_runtime_to_file( char *filename)
{
   struct _mulle_objc_runtime   *runtime;
   FILE                         *fp;

   runtime = mulle_objc_inlined_get_runtime();
   if( ! runtime)
   {
      fprintf( stderr, "No runtime found!\n");
      return;
   }

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   _mulle_objc_runtime_dotdump( runtime, fp);
   fclose( fp);

   fprintf( stderr, "Dumped \"%s\"\n", filename);
}


//
// create successive number of dumps
// for conversion into a movie
//
void   mulle_objc_dotdump_runtime_to_tmp( void)
{
   static mulle_atomic_pointer_t   counter;
   auto char                       buf[ 32];
   int                             nr;
   int                             max;
   char                            *s;
   
   nr = (int) (intptr_t) _mulle_atomic_pointer_increment( &counter);
   s = getenv( "MULLE_OBJC_DOTDUMP_MAX");
   if( s)
   {
      max = atoi( s);
      if( max && nr >= max)
         return;
   }

   sprintf( buf, "/tmp/runtime_%06d.dot", nr);
   mulle_objc_dotdump_runtime_to_file( buf);
}


# pragma mark - class dump


void   _mulle_objc_class_dotdump( struct _mulle_objc_class *cls, FILE *fp)
{
   extern mulle_objc_walkcommand_t
      mulle_objc_classpair_walk( struct _mulle_objc_classpair *,
                                 mulle_objc_walkcallback_t,
                                 void *);
   struct dump_info              info;
   struct _mulle_objc_classpair  *pair;

   pair = _mulle_objc_class_get_classpair( cls);

   c_set_init( &info.set);
   info.fp = fp;

   fprintf( fp, "digraph mulle_objc_class\n{\n");

   mulle_objc_classpair_walk( pair, callback, &info);
   fprintf( fp, "}\n");

   c_set_done( &info.set);
}


void   mulle_objc_class_dotdump_to_file( struct _mulle_objc_class *cls,
                                         char *filename)
{
   FILE   *fp;

   fp = fopen( filename, "w");
   if( ! fp)
   {
      perror( "fopen:");
      return;
   }

   _mulle_objc_class_dotdump( cls, fp);
   fclose( fp);

   fprintf( stderr, "Dumped \"%s\"\n", filename);
}


void   mulle_objc_class_dotdump_to_tmp( struct _mulle_objc_class *cls)
{
   mulle_objc_class_dotdump_to_file( cls, "/tmp/mulle_objc_infraclass.dot");
}


