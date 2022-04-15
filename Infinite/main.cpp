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

class MazeSolver : public olc::PixelGameEngine
 {
public:
   MazeSolver() : cur_zone(std::make_shared<MetaZone>(ZoneDesc(0, 0, 0)))
    {
      sAppName = "MazeSolver Infinite Beta 3";
    }

   bool OnUserCreate() override
    {
      pos_x = 1;
      pos_y = 1;
      scr_x = 1;
      scr_y = 1;

      cr = 255;
      cg = 0;
      cb = 0;
      cm = 0;

      tick = true;

      ZoneImpl::create(cur_zone);
      cur_zone->realization = convert(*cur_zone->impl);
      cur_zone->realization->image[pos_y][pos_x] = olc::Pixel(cr, cg, cb);

      theSprite = std::make_unique<olc::Sprite>(SCREEN_X, SCREEN_Y);
      theDecal = std::make_unique<olc::Decal>(theSprite.get());

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
      if (true == tick) // Do I need a new direction?
       {
         cur_zone->updateDirection();
       }

      switch (cur_zone->lastDirection())
       {
      case 0: // We go left
         --pos_x;
         if (pos_x < 0)
          {
            pos_x += MAX2;
            scr_x += MAX2;
            MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
            cur_zone = cur_zone->getSiblingLeft();
          }
         break;
      case 1: // We go right
         ++pos_x;
         if (pos_x >= MAX2)
          {
            pos_x -= MAX2;
            scr_x -= MAX2;
            MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
            cur_zone = cur_zone->getSiblingRight();
          }
         break;
      case 2: // We go up
         --pos_y;
         if (pos_y < 0)
          {
            pos_y += MAX2;
            scr_y += MAX2;
            MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
            cur_zone = cur_zone->getSiblingUp();
          }
         break;
      case 3: // We go down
         ++pos_y;
         if (pos_y >= MAX2)
          {
            pos_y -= MAX2;
            scr_y -= MAX2;
            MetaZone::cacheMeOut(cur_zone); // Remove some poor behavior seen in tests.
            cur_zone = cur_zone->getSiblingDown();
          }
         break;
       }

      tick = !tick;

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
         for (int x = 0; x < SCREEN_X;)
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

            int ix = std::min(MAX2 - ex, SCREEN_X - x);

            if (nullptr != temp.get())
             {
               if (nullptr == temp->realization.get())
                  temp->realization = convert(*temp->impl);

               std::memcpy(theSprite->GetData() + y * SCREEN_X + x, temp->realization->image[ey] + ex, ix * sizeof(olc::Pixel));
             }
            else
             {
               for (int dx = 0; dx < ix; ++dx)
                  theSprite->SetPixel(x + dx, y, olc::Pixel(0, 0, 0));
             }

            x += ix;
          }
       }

      theDecal->Update();
      DrawDecal({0.0f, 0.0f}, theDecal.get());
      return true;
    }

private:
   std::shared_ptr<MetaZone> cur_zone;
   std::unique_ptr<olc::Sprite> theSprite;
   std::unique_ptr<olc::Decal> theDecal;
   int pos_x, pos_y, scr_x, scr_y;
   bool tick;
   int cr, cg, cb, cm;
 };

int main (void)
 {
   MazeSolver demo;
   if (demo.Construct(SCREEN_X, SCREEN_Y, SCALE_X, SCALE_Y))
      demo.Start();
   return 0;
 }
