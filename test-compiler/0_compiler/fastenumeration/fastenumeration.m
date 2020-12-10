#include <mulle-objc-runtime/mulle-objc-runtime.h>


typedef struct
{
    unsigned long state;
    id  * itemsPtr;
    unsigned long *mutationsPtr;
    unsigned long extra[5];
} NSFastEnumerationState;


@protocol NSFastEnumeration

// this is unsigned long because of the compiler, it should be the same
// as NSUInteger, but that's not 100% sure.
- (unsigned long) countByEnumeratingWithState:(NSFastEnumerationState *) rover
                                   objects:(id *) buffer
                                     count:(unsigned long) len;
- (unsigned long) count;

@end


@interface Bar <NSFastEnumeration>
@end


@implementation Bar

- (void) print
{
   printf("Bar\n");
}


- (unsigned long) countByEnumeratingWithState:(NSFastEnumerationState *) rover
                                      objects:(id *) buffer
                                        count:(unsigned long) len
{
   id               *sentinel;
   unsigned long    remain;

   remain = 20 - rover->state;
   if( ! remain)
      return( 0);

   if( remain < len)
      len = remain;

   rover->state   += len;
   rover->itemsPtr = buffer;

   sentinel = &buffer[ len];
   while( buffer < sentinel)
      *buffer++ = self;

   rover->mutationsPtr = &rover->extra[ 4];

   return( len);
}

- (unsigned long) count
{
   return( 20);
}

@end


@interface Foo  : Bar
@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}

- (void) dealloc
{
   _mulle_objc_instance_free( self);
}

@end


main()
{
   Foo   *foo;
   Bar   *bar;

   foo = [Foo new];
   for( bar in foo)
      [bar print];
   [foo dealloc];

   return( 0);
}
