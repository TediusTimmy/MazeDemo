/*
Copyright (c) 2022 Thomas DiModica.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of Thomas DiModica nor the names of other contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THOMAS DIMODICA AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THOMAS DIMODICA OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#include "Zone.h"
#include "QuatroStack.h"

#include "external/pcg_random.hpp"

std::shared_ptr<ZoneImpl> ZoneImpl::create(const MetaZone &zone)
 {
   pcg64 rng (zone.seed);

   std::unique_ptr<BitZone> visited = std::make_unique<BitZone>();
   std::shared_ptr<ZoneImpl> res = std::make_shared<ZoneImpl>();
   std::fill(res->cell, res->cell + TWO_BITS, 0xFFFFFFFFFFFFFFFFULL);

   int x = rng() & TOP;
   int y = rng() & TOP;
   visited->SetBit(x, y);

   std::unique_ptr<QuatroStack> stack = std::make_unique<QuatroStack>();

   do
    {
      int n = 0;
      if ((x - 1 >  -1) && (false == visited->GetBit(x - 1, y))) ++n;
      if ((x + 1 < MAX) && (false == visited->GetBit(x + 1, y))) ++n;
      if ((y - 1 >  -1) && (false == visited->GetBit(x, y - 1))) ++n;
      if ((y + 1 < MAX) && (false == visited->GetBit(x, y + 1))) ++n;

      while (n)
       {
         bool found = false;
         while (false == found)
          {
            int d = rng() & 3;
            switch (d)
             {
            case 0:
               if ((x - 1 >  -1) && (false == visited->GetBit(x - 1, y)))
                {
                  res->ClearLeft(x, y);
                  visited->SetBit(x - 1, y);
                  stack->push(0);
                  x = x - 1;
                  found = true;
                }
               break;
            case 1:
               if ((x + 1 < MAX) && (false == visited->GetBit(x + 1, y)))
                {
                  res->ClearLeft(x + 1, y);
                  visited->SetBit(x + 1, y);
                  stack->push(1);
                  x = x + 1;
                  found = true;
                }
               break;
            case 2:
               if ((y - 1 >  -1) && (false == visited->GetBit(x, y - 1)))
                {
                  res->ClearUp(x, y);
                  visited->SetBit(x, y - 1);
                  stack->push(2);
                  y = y - 1;
                  found = true;
                }
               break;
            case 3:
               if ((y + 1 < MAX) && (false == visited->GetBit(x, y + 1)))
                {
                  res->ClearUp(x, y + 1);
                  visited->SetBit(x, y + 1);
                  stack->push(3);
                  y = y + 1;
                  found = true;
                }
               break;
             }
          }

         n = 0;
         if ((x - 1 >  -1) && (false == visited->GetBit(x - 1, y))) ++n;
         if ((x + 1 < MAX) && (false == visited->GetBit(x + 1, y))) ++n;
         if ((y - 1 >  -1) && (false == visited->GetBit(x, y - 1))) ++n;
         if ((y + 1 < MAX) && (false == visited->GetBit(x, y + 1))) ++n;
       }

      stack->Prev(stack->pop(), x, y);
    }
   while (0 != stack->sptr);

   return res;
 }
