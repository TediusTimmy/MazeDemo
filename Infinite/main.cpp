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

#define OLC_PGE_APPLICATION
#include "external/olcPixelGameEngine.h"

#include "Zone.h"
#include "Uluru.h"
#include "QuatroStack.h"

// If you make more zones visible on screen, you may want to increase
// the size of the least-recently-used cache, cacheMax, below.
const int SCREEN_X = 320;
const int SCREEN_Y = 240;
const int SCALE_X = 2;
const int SCALE_Y = 2;
const int PAD = 10;

const int MAX2 = MAX * 2;

class Zone
 {
public:
   olc::Pixel image [MAX2][MAX2];
   Zone() { }
   Zone& operator= (const Zone&) = default;
 };

std::shared_ptr<Zone> convert(const ZoneImpl& from)
 {
   std::shared_ptr<Zone> ret = std::make_shared<Zone>();

   for (int y = 0; y < MAX; ++y)
    {
      for (int x = 0; x < MAX; ++x)
       {
         ret->image[2 * y + 0][2 * x + 0] = olc::Pixel(0, 0, 0);
         ret->image[2 * y + 0][2 * x + 1] = from.GetUp(x, y) ? olc::Pixel(0, 0, 0) : olc::Pixel(128, 128, 128);
       }
      for (int x = 0; x < MAX; ++x)
       {
         ret->image[2 * y + 1][2 * x + 0] = from.GetLeft(x, y) ? olc::Pixel(0, 0, 0) : olc::Pixel(128, 128, 128);
         ret->image[2 * y + 1][2 * x + 1] = olc::Pixel(128, 128, 128);
       }
    }

   return ret;
 }

void solve(const ZoneImpl& zone, std::unique_ptr<QuatroStack>& path, int sx, int sy, int fx, int fy);

 // 1 : We weren't at the zone's designated exit when leaving.
 // 2 : We weren't at the far side of the zone when leaving.
 // 3 : The exit of the old zone wasn't the entrance of the new zone.
 // 4 : The side we went through wasn't open.
void pathError(int level) __attribute__((noinline)); // GCC, why are you so stupid?
void pathError(int level)
 {
   std::cerr << "Error in pathfinding " << level << "." << std::endl;
 }

class MazeSolver : public olc::PixelGameEngine
 {
public:
   MazeSolver() : cur_zone(std::make_shared<MetaZone>(ZoneDesc(0, 0, 0), std::make_shared<MetaZone>(ZoneDesc(0, 0, 1))))
    {
      sAppName = "MazeSolver Alpha 2";
    }

   bool OnUserCreate() override
    {
      pos_x = 1;
      pos_y = 1;
      scr_x = 1;
      scr_y = 1;
      pos_p = 0U;

      cr = 255;
      cg = 0;
      cb = 0;
      cm = 0;

      ZoneImpl::create(cur_zone->turtle);
      ZoneImpl::create(cur_zone);
      cur_zone->realization = convert(*cur_zone->impl);
      cur_zone->turtle->children.add(cur_zone->desc, cur_zone);

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
         std::shared_ptr<MetaZone> temp = cur_zone->getSiblingLeft();
         if (nullptr != temp.get())
          {
            cur_zone = temp;
          }
         else
          {
            pos_x = 0;
          }
       }
      if (pos_x >= MAX2)
       {
         pos_x -= MAX2;
         std::shared_ptr<MetaZone> temp = cur_zone->getSiblingRight();
         if (nullptr != temp.get())
          {
            cur_zone = temp;
          }
         else
          {
            pos_x = MAX2 - 1;
          }
       }
      if (pos_y < 0)
       {
         pos_y += MAX2;
         std::shared_ptr<MetaZone> temp = cur_zone->getSiblingUp();
         if (nullptr != temp.get())
          {
            cur_zone = temp;
          }
         else
          {
            pos_y = 0;
          }
       }
      if (pos_y >= MAX2)
       {
         pos_y -= MAX2;
         std::shared_ptr<MetaZone> temp = cur_zone->getSiblingDown();
         if (nullptr != temp.get())
          {
            cur_zone = temp;
          }
         else
          {
            pos_y = MAX2 - 1;
          }
       }
      scr_x = pos_x;
      scr_y = pos_y;
#endif
// SOLVER
#if 1
      if (pos_p == path.size()) // Do I need to solve for a new path?
       {
         int sx = 0, sy = 0, fx = 0, fy = 0, prev = -1;

         // Find starting point
         if (0U == path.size()) // First path find
          {
            sx = 0; // Path finding occurs using passages, not pixels
            sy = 0;
          }
         else // Any time else
          {
            prev = cur_zone->turtle->lastDirection();
            switch (prev)
             {
            case 0: // We went left
               sx = TOP;
               sy = cur_zone->left_c;
               if (static_cast<uint32_t>(pos_y) != (2 * cur_zone->left_c + 1)) pathError(1);
               if (0 != pos_x) pathError(2);
               if (false == cur_zone->isOpenLeft()) pathError(4);
               MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
               cur_zone = cur_zone->getSiblingLeft();
               if (static_cast<uint32_t>(sy) != cur_zone->right_c) pathError(3);
               scr_x += MAX2;
               break;
            case 1: // We went right
               sx = 0;
               sy = cur_zone->right_c;
               if (static_cast<uint32_t>(pos_y) != (2 * cur_zone->right_c + 1)) pathError(1);
               if ((MAX2 - 1) != pos_x) pathError(2);
               MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
               cur_zone = cur_zone->getSiblingRight();
               if (static_cast<uint32_t>(sy) != cur_zone->left_c) pathError(3);
               if (false == cur_zone->isOpenLeft()) pathError(4);
               scr_x -= MAX2;
               break;
            case 2: // We went up
               sx = cur_zone->top_c;
               sy = TOP;
               if (static_cast<uint32_t>(pos_x) != (2 * cur_zone->top_c + 1)) pathError(1);
               if (0 != pos_y) pathError(2);
               MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
               if (false == cur_zone->isOpenUp()) pathError(4);
               cur_zone = cur_zone->getSiblingUp();
               if (static_cast<uint32_t>(sx) != cur_zone->bottom_c) pathError(3);
               scr_y += MAX2;
               break;
            case 3: // We went down
               sx = cur_zone->bottom_c;
               sy = 0;
               if (static_cast<uint32_t>(pos_x) != (2 * cur_zone->bottom_c + 1)) pathError(1);
               if ((MAX2 - 1) != pos_y) pathError(2);
               MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
               cur_zone = cur_zone->getSiblingDown();
               if (static_cast<uint32_t>(sx) != cur_zone->top_c) pathError(3);
               if (false == cur_zone->isOpenUp()) pathError(4);
               scr_y -= MAX2;
               break;
             }
          }

         // Find ending point
         cur_zone->turtle->updateDirection();
         switch (cur_zone->turtle->lastDirection())
          {
         case 0: // We go left
            fx = 0;
            fy = cur_zone->left_c;
            break;
         case 1: // We go right
            fx = TOP;
            fy = cur_zone->right_c;
            break;
         case 2: // We go up
            fx = cur_zone->top_c;
            fy = 0;
            break;
         case 3: // We go down
            fx = cur_zone->bottom_c;
            fy = TOP;
            break;
          }

         // Build solution path
         std::unique_ptr<QuatroStack> solution;
         solve(*cur_zone->impl, solution, sx, sy, fx, fy);

         path.reserve(2 * (solution->sptr + 1));
         path.resize(0U);
         sx = 2 * sx + 1;
         sy = 2 * sy + 1;
         switch (prev) // If we have a previous zone, add in the transition, if it is in our zone
          {
         case 1:
            path.push_back(std::make_pair(sx - 1, sy));
            break;
         case 3:
            path.push_back(std::make_pair(sx, sy - 1));
            break;
          }
         path.push_back(std::make_pair(sx, sy));
         for (int i = 0; i < solution->sptr; ++i)
          {
            switch(solution->seek(i + 1))
             {
            case 0:
               path.push_back(std::make_pair(sx - 1, sy));
               path.push_back(std::make_pair(sx - 2, sy));
               sx = sx - 2;
               break;
            case 1:
               path.push_back(std::make_pair(sx + 1, sy));
               path.push_back(std::make_pair(sx + 2, sy));
               sx = sx + 2;
               break;
            case 2:
               path.push_back(std::make_pair(sx, sy - 1));
               path.push_back(std::make_pair(sx, sy - 2));
               sy = sy - 2;
               break;
            case 3:
               path.push_back(std::make_pair(sx, sy + 1));
               path.push_back(std::make_pair(sx, sy + 2));
               sy = sy + 2;
               break;
             }
          }
         switch (cur_zone->turtle->lastDirection()) // If we have a next zone, but the transition is in our zone, fill it
          {
         case 0:
            path.push_back(std::make_pair(sx - 1, sy));
            break;
         case 2:
            path.push_back(std::make_pair(sx, sy - 1));
            break;
          }
         pos_p = 0U;
       }

      pos_x = path[pos_p].first;
      pos_y = path[pos_p].second;
      ++pos_p;

      switch (cm) // Color the current location to show progress
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
      cur_zone->realization->image[pos_y][pos_x] = olc::Pixel(cr, cg, cb);

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
#endif

// DRAW SCREEN
      for (int y = 0; y < SCREEN_Y; ++y)
       {
         for (int x = 0; x < SCREEN_X; ++x)
          {
            int ex = scr_x - SCREEN_X / 2 + x;
            int ey = scr_y - SCREEN_Y / 2 + y;

            std::shared_ptr<MetaZone> temp = cur_zone;

            while (ex < 0)
             {
               if (nullptr != temp.get())
                  temp = temp->getSiblingLeft();
               ex += MAX2;
             }
            while (ex >= MAX2)
             {
               if (nullptr != temp.get())
                  temp = temp->getSiblingRight();
               ex -= MAX2;
             }
            while (ey < 0)
             {
               if (nullptr != temp.get())
                  temp = temp->getSiblingUp();
               ey += MAX2;
             }
            while (ey >= MAX2)
             {
               if (nullptr != temp.get())
                  temp = temp->getSiblingDown();
               ey -= MAX2;
             }

            if (nullptr != temp.get())
             {
               if (nullptr == temp->realization.get())
                  temp->realization = convert(*temp->impl);

               Draw(x, y, temp->realization->image[ey][ex]);
             }
            else
               Draw(x, y, olc::Pixel(0, 0, 0));
          }
       }

      return true;
    }

private:
   std::shared_ptr<MetaZone> cur_zone;
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
