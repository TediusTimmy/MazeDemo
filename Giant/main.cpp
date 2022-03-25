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

#include <queue>

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "Zone.h"

// If you make more zones visible on screen, you may want to increase
// the size of the least-recently-used cache, cacheMax, below.
const int SCREEN_X = 320;
const int SCREEN_Y = 240;
const int SCALE_X = 2;
const int SCALE_Y = 2;
const int PAD = 10;

const int MAX2 = 2 * MAX;

class Zone
 {
public:
   uint32_t top_z, top_c, left_c, right_c, bottom_z, bottom_c;
   olc::Pixel image [512][512];
 };

void generate(const ZoneDesc& board, ZoneImpl& zone);
bool GetUp(const uint64_t* array, size_t x, size_t y);
bool GetLeft(const uint64_t* array, size_t x, size_t y);
void MakeBMP(const ZoneImpl& board, const char * name);
#include "MakeBMP2.h"

std::unique_ptr<Zone> makeMap(const ZoneDesc& board)
 {
   std::unique_ptr<ZoneImpl> zone = std::make_unique<ZoneImpl>();
   generate(board, *zone.get());
//   MakeBMP(*zone.get(), ("debug_" + std::to_string(board.x) + "_" + std::to_string(board.y) + ".bmp").c_str());

   std::unique_ptr<Zone> ret = std::make_unique<Zone>();

   ret->top_z = zone->top_z;
   ret->top_c = zone->top_c * 2 + 1;
   ret->left_c = zone->left_c * 2 + 1;
   ret->right_c = zone->right_c * 2 + 1;
   ret->bottom_z = zone->bottom_z;
   ret->bottom_c = zone->bottom_c * 2 + 1;

   for (int y = 0; y < MAX; ++y)
    {
      for (int x = 0; x < MAX; ++x)
       {
         ret->image[2 * y + 0][2 * x + 0] = olc::Pixel(0, 0, 0);
         ret->image[2 * y + 0][2 * x + 1] = GetUp(zone->cell, x, y) ? olc::Pixel(0, 0, 0) : olc::Pixel(255, 255, 255);
       }
      for (int x = 0; x < MAX; ++x)
       {
         ret->image[2 * y + 1][2 * x + 0] = GetLeft(zone->cell, x, y) ? olc::Pixel(0, 0, 0) : olc::Pixel(255, 255, 255);
         ret->image[2 * y + 1][2 * x + 1] = olc::Pixel(255, 255, 255);
       }
    }

   return ret;
 }

class Ayers
 {
public:
   std::unique_ptr<Zone> r;
   uint64_t a;
 };

const std::size_t cacheMax = 16U;
std::map<ZoneDesc, Ayers> cache;
Zone& getCache(unsigned int x, unsigned int y)
 {
   static uint64_t h = 0U;
   ++h;
   ZoneDesc zone;
   zone.x = x;
   zone.y = y;

   if (cache.end() == cache.find(zone))
    {
      cache[zone].r = makeMap(zone);
//      MakeBMP2(*cache[zone].r, "core.bmp");
    }
   cache[zone].a = h;

   if (cache.size() > cacheMax)
    {
      std::map<ZoneDesc, Ayers>::iterator found = cache.begin();
      uint64_t b = found->second.a;
      for (std::map<ZoneDesc, Ayers>::iterator iter = cache.begin(); cache.end() != iter; ++iter)
       {
         if (iter->second.a < b)
          {
            b = iter->second.a;
            found = iter;
          }
       }
      cache.erase(found);
    }

   return *cache[zone].r;
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
      return cost > rhs.cost; // Intentionally flip the ordering
    }
 };

void insertChildren(Zone& zone, std::vector<Path>& paths, std::priority_queue<Path>& frontier, int index, int fx, int fy)
 {
   Path parent = paths[index];
   if ((parent.x > 0) && (255 == zone.image[parent.y][parent.x - 1].c.r))
    {
      zone.image[parent.y][parent.x - 1] = olc::Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x - 1 - fx) + std::abs(parent.y - fy), parent.l + 1, parent.x - 1, parent.y));
      frontier.push(paths[paths.size() - 1]);
    }
   if ((parent.x < (MAX2 - 1)) && (255 == zone.image[parent.y][parent.x + 1].c.r))
    {
      zone.image[parent.y][parent.x + 1] = olc::Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x + 1 - fx) + std::abs(parent.y - fy), parent.l + 1, parent.x + 1, parent.y));
      frontier.push(paths[paths.size() - 1]);
    }
   if ((parent.y > 0) && (255 == zone.image[parent.y - 1][parent.x].c.r))
    {
      zone.image[parent.y - 1][parent.x] = olc::Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x - fx) + std::abs(parent.y - 1 - fy), parent.l + 1, parent.x, parent.y - 1));
      frontier.push(paths[paths.size() - 1]);
    }
   if ((parent.y < (MAX2 - 1)) && (255 == zone.image[parent.y + 1][parent.x].c.r))
    {
      zone.image[parent.y + 1][parent.x] = olc::Pixel(128, 128, 128);
      paths.push_back(Path(paths.size(), parent.index, parent.l + std::abs(parent.x - fx) + std::abs(parent.y + 1 - fy), parent.l + 1, parent.x, parent.y + 1));
      frontier.push(paths[paths.size() - 1]);
    }
 }

class MazeSolver : public olc::PixelGameEngine
 {
public:
   MazeSolver()
    {
      sAppName = "MazeSolver Alpha 1";
    }

   bool OnUserCreate() override
    {
      cur_zone.x = 0;
      cur_zone.y = 0;
      next_zone.x = 0;
      next_zone.y = 0;

      pos_x = 1;
      pos_y = 1;
      scr_x = 1;
      scr_y = 1;
      pos_p = 0U;

      cr = 255;
      cg = 0;
      cb = 0;
      cm = 0;

      getCache(cur_zone.x, cur_zone.y);
      return true;
    }

   bool OnUserUpdate(float /*fElapsedTime*/) override
    {
// MANUAL NAVIGATION
#if 0
      if (GetKey(olc::Key::W).bHeld) --pos_y;
      if (GetKey(olc::Key::S).bHeld) ++pos_y;
      if (GetKey(olc::Key::A).bHeld) --pos_x;
      if (GetKey(olc::Key::D).bHeld) ++pos_x;

      if (pos_x < 0)
       {
         pos_x += MAX2;
         if (0 != cur_zone.x)
          {
            --cur_zone.x;
          }
         else
          {
            pos_x = 0;
          }
       }
      if (pos_x >= MAX2)
       {
         pos_x -= MAX2;
         if (0xFFFFFFFF != cur_zone.x)
          {
            ++cur_zone.x;
          }
         else
          {
            pos_x = MAX2 - 1;
          }
       }
      if (pos_y < 0)
       {
         pos_y += MAX2;
         if (0 != cur_zone.y)
          {
            --cur_zone.y;
          }
         else
          {
            pos_y = 0;
          }
       }
      if (pos_y >= MAX2)
       {
         pos_y -= MAX2;
         if (0xFFFFFFFF != cur_zone.y)
          {
            ++cur_zone.y;
          }
         else
          {
            pos_y = MAX2 - 1;
          }
       }
#endif
// SOLVER
      if (pos_p == path.size()) // Do I need to solve for a new path?
       {
         int sx, sy, fx, fy;

         // Find starting point
         if (0U == path.size()) // First path find
          {
            sx = 1;
            sy = 1;
          }
         else // Any time else
          {
            if (0 == pos_x) // Left edge
             {
               sx = MAX2 - 1;
               sy = pos_y;
               scr_x = scr_x + MAX2;
             }
            else if (MAX2 - 1 == pos_x) // Right edge
             {
               sx = 0;
               sy = pos_y;
               scr_x = scr_x - MAX2;
             }
            else // Bottom edge (Maze structure does not currently allow for going up)
             {
               sx = pos_x;
               sy = 0;
               scr_y = scr_y - MAX2;
             }

            // Also advance zone
            cur_zone.x = next_zone.x;
            cur_zone.y = next_zone.y;
          }

         // Find ending point
         Zone& zone = getCache(cur_zone.x, cur_zone.y);
         if (0xFFFFFFFF == cur_zone.y) // Are we on the bottom? You should never get here.
          {
            if (0xFFFFFFFF == cur_zone.x) // We are done and the universe is dead.
             {
               return false;
             }
            // Need to go right
            fx = MAX2 - 1;
            fy = zone.right_c;
            ++next_zone.x;
          }
         else if (cur_zone.x < zone.bottom_z) // Need to go right
          {
            fx = MAX2 - 1;
            fy = zone.right_c;
            ++next_zone.x;
          }
         else if (cur_zone.x < zone.bottom_z) // Need to go left
          {
            fx = 0;
            fy = zone.left_c;
            --next_zone.x;
          }
         else // We are here! Go down.
          {
            fx = zone.bottom_c;
            fy = MAX2 - 1;
            ++next_zone.y;
          }

         // Find the path between (sx, sy) and (fx, fy) with A*
         if ((255 != zone.image[sy][sx].c.r) || (255 != zone.image[fy][fx].c.r)) // Hopefully unneeded
          {
            std::cerr << "Logic error in path finding: cannot reach start/end : (" << sx << ", " << sy << ") -> (" << fx << ", " << fy << ")" << std::endl;
            return false;
          }
         zone.image[sy][sx] = olc::Pixel(128, 128, 128);

         std::vector<Path> paths;
         std::priority_queue<Path> frontier;
         paths.push_back(Path(paths.size(), -1, 1 + std::abs(sx - fx) + std::abs(sy - fy), 1, sx, sy));

         insertChildren(zone, paths, frontier, 0, fx, fy);
         pos_p = 1U; // Use this as an answer found boolean.
         while (false == frontier.empty())
          {
            Path shortest = frontier.top();
            frontier.pop();
            if ((fx == shortest.x) && (fy == shortest.y)) // Have we found the end?
             {
               int walk = shortest.index;
               int length = shortest.l;
               path.resize(length);
               pos_p = 0U;

               --length; // Walk it backwards to build the path.
               while (-1 != paths[walk].prev)
                {
                  path[length] = std::make_pair(paths[walk].x, paths[walk].y);
                  walk = paths[walk].prev;
                  --length;
                }
               path[length] = std::make_pair(paths[walk].x, paths[walk].y);

               break; // Done
             }
            insertChildren(zone, paths, frontier, shortest.index, fx, fy);
          }
         if (0 != pos_p)
          {
//            MakeBMP2(zone, "core.bmp");
            std::cerr << "Logic error in path finding: cannot reach end from start : (" << sx << ", " << sy << ") -> (" << fx << ", " << fy << ")" << std::endl;
            return false;
          }
       }

      pos_x = path[pos_p].first;
      pos_y = path[pos_p].second;
      ++pos_p;

      Zone& zone = getCache(cur_zone.x, cur_zone.y); // Color the current location to show progress
      switch (cm)
       {
      case 0: // cr 255, inc cg
         ++cg;
         if (255 == cg) cm = 1;
         break;
      case 1: // cg 255, dec cr
         --cr;
         if (0 == cr) cm = 2;
         break;
      case 2: // cg 255, inc cb
         ++cb;
         if (255 == cb) cm = 3;
         break;
      case 3: // cb 255, dec cg
         --cg;
         if (0 == cg) cm = 4;
         break;
      case 4: // cb 255, inc cr
         ++cr;
         if (255 == cr) cm = 5;
         break;
      case 5: // cr 255, dec cb
         --cb;
         if (0 == cb) cm = 0;
         break;
       }
      zone.image[pos_y][pos_x] = olc::Pixel(cr, cg, cb);

       {
         int epx = scr_x - pos_x;
         int epy = scr_y - pos_y;
         if (epx < -SCREEN_X / 2 + PAD)
          {
            ++scr_x;
          }
         else if (epx > SCREEN_X / 2 - PAD)
          {
            --scr_x;
          }
         if (epy < -SCREEN_Y / 2 + PAD)
          {
            ++scr_y;
          }
         else if (epy > SCREEN_Y / 2 - PAD)
          {
            --scr_y;
          }
       }

// DRAW SCREEN
      for (int y = 0; y < SCREEN_Y; ++y)
       {
         for (int x = 0; x < SCREEN_X; ++x)
          {
            int ex = scr_x - SCREEN_X / 2 + x;
            int ey = scr_y - SCREEN_Y / 2 + y;

            unsigned int ezx = cur_zone.x;
            unsigned int ezy = cur_zone.y;

            if ( ((0 == ezx) && (ex < 0)) ||
                 ((0 == ezy) && (ey < 0)) ||
                 ((0xFFFFFFFF == ezx) && (ex >= MAX2)) ||
                 ((0xFFFFFFFF == ezy) && (ey >= MAX2)) )
             {
               Draw(x, y, olc::Pixel(0, 0, 0));
             }
            else
             {
               while (ex < 0)
                {
                  --ezx;
                  ex += MAX2;
                }
               while (ex >= MAX2)
                {
                  ++ezx;
                  ex -= MAX2;
                }
               while (ey < 0)
                {
                  --ezy;
                  ey += MAX2;
                }
               while (ey >= MAX2)
                {
                  ++ezy;
                  ey -= MAX2;
                }

               const Zone& zone = getCache(ezx, ezy);
               Draw(x, y, zone.image[ey][ex]);
             }
          }
       }

      return true;
    }

private:
   ZoneDesc cur_zone, next_zone;
   int pos_x, pos_y, scr_x, scr_y;
   std::vector<std::pair<int, int> > path;
   size_t pos_p;
   int cr, cg, cb, cm;
 };

int main (void)
 {
   MazeSolver demo;
   if (demo.Construct(SCREEN_X, SCREEN_Y, SCALE_X, SCALE_Y))
      demo.Start();
   return 0;
 }
