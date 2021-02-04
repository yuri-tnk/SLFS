/*
  Implementation of a 32-bit KISS generator which uses no multiply instructions.
  Public Domain.

  From:

   "Good Practice in (Pseudo) Random Number Generation for
    Bioinformatics Applications",
    David Jones,
    UCL Bioinformatics Group

*/

#include "../../../fs_common/flash_fs.h"
#include "slfs_test.h"

static unsigned long x = 123456789UL,
                     y = 234567891UL,
                     z = 345678912UL,
                     w = 456789123UL,
                     c = 0UL;

//----------------------------------------------------------------------------
void slfs_randomize(void)
{
   int i;

   x = 123456789UL;
   y = 234567891UL;
   z = 345678912UL;
   w = 456789123UL;
   c = 0UL;

   for(i=0; i<20; i++)
      slfs_rand();
}

//----------------------------------------------------------------------------
unsigned long slfs_rand(void)
{
   long t;

   y ^= (y<<5);
   y ^= (y>>7);
   y ^= (y<<22);
   t = z + w + c;
   z = w;
   c = t < 0L;
   w = t & 0x7FFFFFFFUL; // 2147483647;
   x += 1411392427UL;

   return x + y + w;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------




