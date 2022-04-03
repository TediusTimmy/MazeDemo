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

#include <iostream>

#include "Zone.h"
#include "QuatroStack.h"

#include "external/pcg_random.hpp"

void solve(const ZoneImpl& zone, std::unique_ptr<QuatroStack>& path, std::unique_ptr<BitZone>& visited, int sx, int sy, int fx, int fy)
 {
   pcg64 rng (42); // This is a fairly "lol random" number.

   visited = std::make_unique<BitZone>();

   int x = sx;
   int y = sy;
   visited->SetBit(x, y);

   path = std::make_unique<QuatroStack>();

   for (;;)
    {
      if ((fx == x) && (fy == y))
       {
         return;
       }

      int n = 0;
      if ((x - 1 >  -1) && (false == zone.GetLeft(x, y))  && (false == visited->GetBit(x - 1, y))) ++n;
      if ((x + 1 < MAX) && (false == zone.GetRight(x, y)) && (false == visited->GetBit(x + 1, y))) ++n;
      if ((y - 1 >  -1) && (false == zone.GetUp(x, y))    && (false == visited->GetBit(x, y - 1))) ++n;
      if ((y + 1 < MAX) && (false == zone.GetDown(x, y))  && (false == visited->GetBit(x, y + 1))) ++n;

      while (n)
       {
         bool found = false;
         while (false == found)
          {
            int d = rng() & 3;
            switch (d)
             {
            case 0:
               if ((x - 1 >  -1) && (false == zone.GetLeft(x, y))  && (false == visited->GetBit(x - 1, y)))
                {
                  visited->SetBit(x - 1, y);
                  path->push(0);
                  x = x - 1;
                  found = true;
                }
               break;
            case 1:
               if ((x + 1 < MAX) && (false == zone.GetRight(x, y)) && (false == visited->GetBit(x + 1, y)))
                {
                  visited->SetBit(x + 1, y);
                  path->push(1);
                  x = x + 1;
                  found = true;
                }
               break;
            case 2:
               if ((y - 1 >  -1) && (false == zone.GetUp(x, y))    && (false == visited->GetBit(x, y - 1)))
                {
                  visited->SetBit(x, y - 1);
                  path->push(2);
                  y = y - 1;
                  found = true;
                }
               break;
            case 3:
               if ((y + 1 < MAX) && (false == zone.GetDown(x, y))  && (false == visited->GetBit(x, y + 1)))
                {
                  visited->SetBit(x, y + 1);
                  path->push(3);
                  y = y + 1;
                  found = true;
                }
               break;
             }
          }

         if ((fx == x) && (fy == y))
          {
            return;
          }

         n = 0;
         if ((x - 1 >  -1) && (false == zone.GetLeft(x, y))  && (false == visited->GetBit(x - 1, y))) ++n;
         if ((x + 1 < MAX) && (false == zone.GetRight(x, y)) && (false == visited->GetBit(x + 1, y))) ++n;
         if ((y - 1 >  -1) && (false == zone.GetUp(x, y))    && (false == visited->GetBit(x, y - 1))) ++n;
         if ((y + 1 < MAX) && (false == zone.GetDown(x, y))  && (false == visited->GetBit(x, y + 1))) ++n;
       }

      if (0 == path->sptr)
       {
         break;
       }

      path->Prev(path->pop(), x, y);
    }

   std::cerr << "Path finding error: no solution from (" << sx << ", " << sy << ") to ("  << fx << ", " << fy << ")" << std::endl;
 }
