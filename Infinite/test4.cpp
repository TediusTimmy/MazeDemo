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
   std::shared_ptr<MetaZone> metas [8][8];
   std::unique_ptr<Zone> image [8][8];
   for (int i = 0; i < 8; ++i)
    {
      for (int j = 0; j < 8; ++j)
       {
         if ((0 == i) & (0 == j))
          {
            ZoneDesc current (i, j, 0);
            metas[i][j] = std::make_shared<MetaZone>(current);
          }
         else if (0 == j)
          {
            metas[i][j] = metas[i - 1][j]->getSiblingRight();
          }
         else
          {
            metas[i][j] = metas[i][j - 1]->getSiblingDown();
          }
         ZoneImpl::create(metas[i][j]);
         image[i][j] = convert(*(metas[i][j]->impl));
       }
    }
   std::cerr << "Made it here." << std::endl;
   int pos_x = 1;
   int pos_y = 1;
   int arx = 0;
   int ary = 0;
   std::shared_ptr<MetaZone> cur_zone = metas[arx][ary];
   for (;;)
    {
      cur_zone->updateDirection();
      switch (cur_zone->lastDirection())
       {
      case 0: // We go left
         pos_x -= 2;
         image[arx][ary]->image[pos_y][pos_x + 1] = Pixel(0, 0, 255);
         if (pos_x < 0)
          {
            pos_x += MAX2;
            arx--;
            std::cerr << "Moving to " << arx << ", " << ary << "." << std::endl;
            cur_zone = metas[arx][ary];
          }
         break;
      case 1: // We go right
         pos_x += 2;
         if (pos_x >= MAX2)
          {
            pos_x -= MAX2;
            arx++;
            std::cerr << "Moving to " << arx << ", " << ary << "." << std::endl;
            if (arx > 7) goto loop_out;
            cur_zone = metas[arx][ary];
          }
         image[arx][ary]->image[pos_y][pos_x - 1] = Pixel(0, 0, 255);
         break;
      case 2: // We go up
         pos_y -= 2;
         image[arx][ary]->image[pos_y + 1][pos_x] = Pixel(0, 0, 255);
         if (pos_y < 0)
          {
            pos_y += MAX2;
            ary--;
            std::cerr << "Moving to " << arx << ", " << ary << "." << std::endl;
            cur_zone = metas[arx][ary];
          }
         break;
      case 3: // We go down
         pos_y += 2;
         if (pos_y >= MAX2)
          {
            pos_y -= MAX2;
            ary++;
            std::cerr << "Moving to " << arx << ", " << ary << "." << std::endl;
            cur_zone = metas[arx][ary];
          }
         image[arx][ary]->image[pos_y - 1][pos_x] = Pixel(0, 0, 255);
         break;
       }
      image[arx][ary]->image[pos_y][pos_x] = Pixel(0, 0, 255);
    }
loop_out:
   std::cerr << "Solved." << std::endl;
   for (int i = 0; i < 8; ++i)
    {
      for (int j = 0; j < 8; ++j)
       {
         MakeBMP2(*(image[i][j]), ("Path_" + std::to_string(i) + "_" + std::to_string(j) + ".bmp").c_str());
       }
    }

   return 0;
 }
