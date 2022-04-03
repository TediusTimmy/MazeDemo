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

#include <string>

#include "Zone.h"
#include "QuatroStack.h"

const int MAX2 = MAX * 2;

struct Pixel
 {
   uint8_t r;
   uint8_t g;
   uint8_t b;
   Pixel() { }
   Pixel(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) { }
   Pixel& operator= (const Pixel&) = default;
   bool operator== (const Pixel& rhs) { return (r == rhs.r) && (g == rhs.g) && (b == rhs.b); }
 };

class Zone
 {
public:
   Pixel image [MAX2][MAX2];
   Zone() { }
   Zone& operator= (const Zone&) = default;
 };

std::unique_ptr<Zone> convert(const ZoneImpl& from)
 {
   std::unique_ptr<Zone> ret = std::make_unique<Zone>();

   for (int y = 0; y < MAX; ++y)
    {
      for (int x = 0; x < MAX; ++x)
       {
         ret->image[2 * y + 0][2 * x + 0] = Pixel(0, 0, 0);
         ret->image[2 * y + 0][2 * x + 1] = from.GetUp(x, y) ? Pixel(0, 0, 0) : Pixel(255, 255, 255);
       }
      for (int x = 0; x < MAX; ++x)
       {
         ret->image[2 * y + 1][2 * x + 0] = from.GetLeft(x, y) ? Pixel(0, 0, 0) : Pixel(255, 255, 255);
         ret->image[2 * y + 1][2 * x + 1] = Pixel(255, 255, 255);
       }
    }

   return ret;
 }

#include "MakeBMP2.h"

void solve(const ZoneImpl& zone, std::unique_ptr<QuatroStack>& path, std::unique_ptr<BitZone>& visited, int sx, int sy, int fx, int fy);

int main (void)
 {
   for (int d = 0; d < 32; ++d)
    {
      ZoneDesc board (0, 0, d);
      MetaZone zone (board);

      std::shared_ptr<ZoneImpl> impl = ZoneImpl::create(zone);

      std::unique_ptr<QuatroStack> pathDown;
      std::unique_ptr<BitZone> visitedDown;
      std::unique_ptr<QuatroStack> pathRight;
      std::unique_ptr<BitZone> visitedRight;

      solve(*impl, pathDown, visitedDown, 0, 0, zone.bottom_c, 1023);
      solve(*impl, pathRight, visitedRight, 0, 0, 1023, zone.right_c);

      std::unique_ptr<Zone> base = convert(*impl);

      int equal = 0;
      for (; (equal < pathDown->sptr) && (equal < pathRight->sptr); ++equal)
       {
         if (pathDown->seek(equal + 1) != pathRight->seek(equal + 1))
          {
            break;
          }
       }

      int x = 1;
      int y = 1;
      base->image[y][x] = Pixel(255, 0, 255);
      for (int s = 0; s < equal; ++s)
       {
         switch(pathDown->seek(s + 1))
          {
         case 0:
            x = x - 2;
            base->image[y][x] = Pixel(255, 0, 255);
            base->image[y][x + 1] = Pixel(255, 0, 255);
            break;
         case 1:
            x = x + 2;
            base->image[y][x] = Pixel(255, 0, 255);
            base->image[y][x - 1] = Pixel(255, 0, 255);
            break;
         case 2:
            y = y - 2;
            base->image[y][x] = Pixel(255, 0, 255);
            base->image[y + 1][x] = Pixel(255, 0, 255);
            break;
         case 3:
            y = y + 2;
            base->image[y][x] = Pixel(255, 0, 255);
            base->image[y - 1][x] = Pixel(255, 0, 255);
            break;
          }
       }

      int sx = x;
      int sy = y;
      for (int s = equal; s < pathDown->sptr; ++s)
       {
         switch(pathDown->seek(s + 1))
          {
         case 0:
            x = x - 2;
            base->image[y][x] = Pixel(0, 0, 255);
            base->image[y][x + 1] = Pixel(0, 0, 255);
            break;
         case 1:
            x = x + 2;
            base->image[y][x] = Pixel(0, 0, 255);
            base->image[y][x - 1] = Pixel(0, 0, 255);
            break;
         case 2:
            y = y - 2;
            base->image[y][x] = Pixel(0, 0, 255);
            base->image[y + 1][x] = Pixel(0, 0, 255);
            break;
         case 3:
            y = y + 2;
            base->image[y][x] = Pixel(0, 0, 255);
            base->image[y - 1][x] = Pixel(0, 0, 255);
            break;
          }
       }

      x = sx;
      y = sy;
      for (int s = equal; s < pathRight->sptr; ++s)
       {
         switch(pathRight->seek(s + 1))
          {
         case 0:
            x = x - 2;
            base->image[y][x] = Pixel(255, 0, 0);
            base->image[y][x + 1] = Pixel(255, 0, 0);
            break;
         case 1:
            x = x + 2;
            base->image[y][x] = Pixel(255, 0, 0);
            base->image[y][x - 1] = Pixel(255, 0, 0);
            break;
         case 2:
            y = y - 2;
            base->image[y][x] = Pixel(255, 0, 0);
            base->image[y + 1][x] = Pixel(255, 0, 0);
            break;
         case 3:
            y = y + 2;
            base->image[y][x] = Pixel(255, 0, 0);
            base->image[y - 1][x] = Pixel(255, 0, 0);
            break;
          }
       }

      for (y = 0; y < MAX; ++y)
       {
         for (x = 0; x < MAX; ++x)
          {
            if (Pixel(255, 255, 255) == base->image[2 * y + 1][2 * x + 1])
             {
               if ((true == visitedDown->GetBit(x, y)) && (true == visitedRight->GetBit(x, y)))
                {
                  base->image[2 * y + 1][2 * x + 1] = Pixel(64, 64, 64);
                  if (Pixel(255, 255, 255) == base->image[2 * y + 1][2 * x])
                     base->image[2 * y + 1][2 * x] = Pixel(64, 64, 64);
                  if ((x < TOP) && (Pixel(255, 255, 255) == base->image[2 * y + 1][2 * x + 2]))
                     base->image[2 * y + 1][2 * x + 2] = Pixel(64, 64, 64);
                  if (Pixel(255, 255, 255) == base->image[2 * y][2 * x + 1])
                     base->image[2 * y][2 * x + 1] = Pixel(64, 64, 64);
                  if ((y < TOP) && (Pixel(255, 255, 255) == base->image[2 * y + 2][2 * x + 1]))
                     base->image[2 * y + 2][2 * x + 1] = Pixel(64, 64, 64);
                }
               else if (true == visitedDown->GetBit(x, y))
                {
                  base->image[2 * y + 1][2 * x + 1] = Pixel(128, 128, 255);
                  if (Pixel(255, 255, 255) == base->image[2 * y + 1][2 * x])
                     base->image[2 * y + 1][2 * x] = Pixel(128, 128, 255);
                  if ((x < TOP) && (Pixel(255, 255, 255) == base->image[2 * y + 1][2 * x + 2]))
                     base->image[2 * y + 1][2 * x + 2] = Pixel(128, 128, 255);
                  if (Pixel(255, 255, 255) == base->image[2 * y][2 * x + 1])
                     base->image[2 * y][2 * x + 1] = Pixel(128, 128, 255);
                  if ((y < TOP) && (Pixel(255, 255, 255) == base->image[2 * y + 2][2 * x + 1]))
                     base->image[2 * y + 2][2 * x + 1] = Pixel(128, 128, 255);
                }
               else if (true == visitedRight->GetBit(x, y))
                {
                  base->image[2 * y + 1][2 * x + 1] = Pixel(255, 128, 128);
                  if (Pixel(255, 255, 255) == base->image[2 * y + 1][2 * x])
                     base->image[2 * y + 1][2 * x] = Pixel(255, 128, 128);
                  if ((x < TOP) && (Pixel(255, 255, 255) == base->image[2 * y + 1][2 * x + 2]))
                     base->image[2 * y + 1][2 * x + 2] = Pixel(255, 128, 128);
                  if (Pixel(255, 255, 255) == base->image[2 * y][2 * x + 1])
                     base->image[2 * y][2 * x + 1] = Pixel(255, 128, 128);
                  if ((y < TOP) && (Pixel(255, 255, 255) == base->image[2 * y + 2][2 * x + 1]))
                     base->image[2 * y + 2][2 * x + 1] = Pixel(255, 128, 128);
                }
             }
          }
       }

      MakeBMP2(*base, ("CoSolutionDFS_" + std::to_string(d) + ".bmp").c_str());
    }

   return 0;
 }
