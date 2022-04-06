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
#include <iostream>
#include <queue>

#include "Zone.h"

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

ZoneHolder::~ZoneHolder()
 {
 }

class Path
 {
public:
   int index;
   int prev;
   int cost;
   int l;
   int x;
   int y;

   Path(int index, int prev, int cost, int l, int x, int y) : index(index), prev(prev), cost(cost), l(l), x(x), y(y) { }
   Path(const Path&) = default;
   Path& operator=(const Path&) = default;

   bool operator < (const Path& rhs) const
    {
      if (cost != rhs.cost)
         return cost > rhs.cost; // Intentionally flip the ordering
      else
         return (cost - l) > (rhs.cost - rhs.l); // Else sort by distance to the goal
    }
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

void insertChildren(Zone& zone, std::vector<Path>& paths, std::priority_queue<Path>& frontier, int index, int fx, int fy)
 {
   Path parent = paths[index];
   if ((parent.x > 0) && (255 == zone.image[parent.y][parent.x - 1].r))
    {
      zone.image[parent.y][parent.x - 1] = Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x - 1 - fx) + std::abs(parent.y - fy), parent.l + 1, parent.x - 1, parent.y));
      frontier.push(paths[paths.size() - 1]);
    }
   if ((parent.x < (MAX2 - 1)) && (255 == zone.image[parent.y][parent.x + 1].r))
    {
      zone.image[parent.y][parent.x + 1] = Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x + 1 - fx) + std::abs(parent.y - fy), parent.l + 1, parent.x + 1, parent.y));
      frontier.push(paths[paths.size() - 1]);
    }
   if ((parent.y > 0) && (255 == zone.image[parent.y - 1][parent.x].r))
    {
      zone.image[parent.y - 1][parent.x] = Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x - fx) + std::abs(parent.y - 1 - fy), parent.l + 1, parent.x, parent.y - 1));
      frontier.push(paths[paths.size() - 1]);
    }
   if ((parent.y < (MAX2 - 1)) && (255 == zone.image[parent.y + 1][parent.x].r))
    {
      zone.image[parent.y + 1][parent.x] = Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x - fx) + std::abs(parent.y + 1 - fy), parent.l + 1, parent.x, parent.y + 1));
      frontier.push(paths[paths.size() - 1]);
    }
 }

bool solve(Zone& zone, int sx, int sy, int fx, int fy, Pixel path)
 {
   // Find the path between (sx, sy) and (fx, fy) with A*
   if ((255 != zone.image[sy][sx].r) || (255 != zone.image[fy][fx].r)) // Hopefully unneeded
    {
      std::cerr << "Logic error in path finding: cannot reach start/end : (" << sx << ", " << sy << ") -> (" << fx << ", " << fy << ")" << std::endl;
      return false;
    }
   zone.image[sy][sx] = Pixel(128, 128, 128);

   std::vector<Path> paths;
   std::priority_queue<Path> frontier;
   paths.push_back(Path(paths.size(), -1, 1 + std::abs(sx - fx) + std::abs(sy - fy), 1, sx, sy));

   insertChildren(zone, paths, frontier, 0, fx, fy);
   bool solution = false;
   while (false == frontier.empty())
    {
      Path shortest = frontier.top();
      frontier.pop();
      if ((fx == shortest.x) && (fy == shortest.y)) // Have we found the end?
       {
         int walk = shortest.index;
         solution = true;

         while (-1 != paths[walk].prev)
          {
            zone.image[paths[walk].y][paths[walk].x] = path;
            walk = paths[walk].prev;
          }

         break; // Done
       }
      insertChildren(zone, paths, frontier, shortest.index, fx, fy);
    }
   if (false == solution)
    {
      std::cerr << "Logic error in path finding: cannot reach end from start : (" << sx << ", " << sy << ") -> (" << fx << ", " << fy << ")" << std::endl;
      return false;
    }
   return true;
 }

#include "MakeBMP2.h"

int main (void)
 {
   for (int d = 0; d < 32; ++d)
    {
      ZoneDesc board (0, 0, d);
      MetaZone zone (board);

      std::shared_ptr<ZoneImpl> impl = ZoneImpl::create(zone);

//      impl->MakeBMP("ActualZone.bmp");

      std::unique_ptr<Zone> base = convert(*impl);

      std::unique_ptr<Zone> bottom = std::make_unique<Zone>(*base);
      std::unique_ptr<Zone> right = std::make_unique<Zone>(*base);

//      std::cout << "Bottom : " << (2 * zone.bottom_c + 1) << std::endl;
//      std::cout << "Right : " << (2 * zone.right_c + 1) << std::endl;

      solve(*bottom, 1, 1, 2 * zone.bottom_c + 1, 2047, Pixel(0, 0, 255));
      solve(*right, 1, 1, 2047, 2 * zone.right_c + 1, Pixel(255, 0, 0));

      if (29 == d)
      {
         MakeBMP2(*bottom, "Actual_Bottom.bmp");
         MakeBMP2(*right, "Actual_Right.bmp");
      }

      for (int y = 0; y < MAX2; ++y)
       {
         for (int x = 0; x < MAX2; ++x)
          {
            if (Pixel(255, 0, 0) == right->image[y][x])
             {
               if (Pixel(0, 0, 255) == bottom->image[y][x])
                  bottom->image[y][x].r = right->image[y][x].r;
               else
                  bottom->image[y][x] = right->image[y][x];
             }
            else if (Pixel(128, 128, 128) == right->image[y][x])
             {
               if (Pixel(128, 128, 128) == bottom->image[y][x])
                  bottom->image[y][x] = Pixel(64, 64, 64);
               else if (Pixel(255, 255, 255) == bottom->image[y][x])
                  bottom->image[y][x] = Pixel(255, 128, 128);
             }
            else if (Pixel(128, 128, 128) == bottom->image[y][x])
             {
               bottom->image[y][x] = Pixel(128, 128, 255);
             }
          }
       }

      MakeBMP2(*bottom, ("CoSolution_" + std::to_string(d) + ".bmp").c_str());
    }

   return 0;
 }
