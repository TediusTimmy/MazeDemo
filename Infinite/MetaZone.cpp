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

#include "external/city.h"
#include "external/pcg_random.hpp"

void MetaZone::turtleDown(std::vector<char>& stack) const
 {
   if ((0 != desc.x) || (0 != desc.y) || (nullptr != turtle.get()))
    {
      stack.push_back(desc.x & 255);
      stack.push_back(desc.x >> 8);
      stack.push_back(desc.y & 255);
      stack.push_back(desc.y >> 8);

      if (nullptr != turtle.get())
       {
         turtle->turtleDown(stack);
       }
    }
 }

__uint128_t MakeSeed(uint32_t a, uint32_t b, uint32_t c, __uint128_t d)
 {
   char string [28];
   std::memcpy(string, reinterpret_cast<char*>(static_cast<void*>(&a)), 4);
   std::memcpy(string + 4, reinterpret_cast<char*>(static_cast<void*>(&b)), 4);
   std::memcpy(string + 8, reinterpret_cast<char*>(static_cast<void*>(&c)), 4);
   std::memcpy(string + 12, reinterpret_cast<char*>(static_cast<void*>(&d)), 16);
   uint128 res = CityHash128(string, 28);
   return static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
 }

__uint128_t MakeSeed(uint32_t a, uint32_t b, uint32_t c, uint32_t d, __uint128_t e)
 {
   char string [32];
   std::memcpy(string, reinterpret_cast<char*>(static_cast<void*>(&a)), 4);
   std::memcpy(string + 4, reinterpret_cast<char*>(static_cast<void*>(&b)), 4);
   std::memcpy(string + 8, reinterpret_cast<char*>(static_cast<void*>(&c)), 4);
   std::memcpy(string + 12, reinterpret_cast<char*>(static_cast<void*>(&d)), 4);
   std::memcpy(string + 16, reinterpret_cast<char*>(static_cast<void*>(&e)), 16);
   uint128 res = CityHash128(string, 32);
   return static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
 }

MetaZone::MetaZone(const ZoneDesc& zone, std::shared_ptr<MetaZone> turtle_ptr) : desc(zone), turtle(turtle_ptr), children(8U)
 {
   if (nullptr != turtle.get())
    {
      std::vector<char> temp;
      turtle->turtleDown(temp);
      uint128 res = CityHash128(&temp[0], temp.size());
      turtle_hash = static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
    }
   else
    {
      uint128 res = CityHash128("", 0);
      turtle_hash = static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
    }

   seed = MakeSeed(desc.x, desc.y, desc.d, turtle_hash);

   pcg64 top    (MakeSeed(desc.x, (desc.y - 1) & TOP, desc.y, desc.d, turtle_hash));
   pcg64 left   (MakeSeed((desc.x - 1) & TOP, desc.x, desc.y, desc.d, turtle_hash));
   pcg64 right  (MakeSeed(desc.x, (desc.x + 1) & TOP, desc.y, desc.d, turtle_hash));
   pcg64 bottom (MakeSeed(desc.x, desc.y, (desc.y + 1) & TOP, desc.d, turtle_hash));

   top_c = top() & TOP;
   left_c = left() & TOP;
   right_c = right() & TOP;
   bottom_c = bottom() & TOP;
 }

bool MetaZone::isOpenUp() const
 {
   if ((0 == desc.x) && (0 == desc.y) && (nullptr == turtle.get())) // If we are at the top-left, return false.
    {
      return false;
    }

   if (0 == desc.y)
    {
      if (desc.x == turtle->top_c)
         return turtle->isOpenUp();
      return false;
    }

   return false == turtle->impl->GetUp(desc.x, desc.y);
 }

bool MetaZone::isOpenLeft() const
 {
   if ((0 == desc.x) && (0 == desc.y) && (nullptr == turtle.get())) // If we are at the top-left, return false.
    {
      return false;
    }

   if (0 == desc.x)
    {
      if (desc.y == turtle->left_c)
         return turtle->isOpenLeft();
      return false;
    }

   return false == turtle->impl->GetLeft(desc.x, desc.y);
 }

std::shared_ptr<MetaZone> MetaZone::getSiblingUp()
 {
   if (desc.y > 0) // Base case : lookup in parent
    {
      if (nullptr == turtle.get()) // If my parent doesn't exist, create them
       {
         turtle = std::make_shared<MetaZone>(ZoneDesc(0, 0, desc.d + 1)); // Parent not existing implies (x, y) is (0, 0)
         ZoneImpl::create(turtle);
       }

      std::shared_ptr<MetaZone> temp = turtle->children.get(ZoneDesc(desc.x, desc.y - 1, desc.d)); // Lookup sibling in parent

      if (nullptr == temp.get()) // If my sibling doesn't exist, create them
       {
         temp = std::make_shared<MetaZone>(ZoneDesc(desc.x, desc.y - 1, desc.d), turtle);
         ZoneImpl::create(temp);
         turtle->children.add(temp->desc, temp);
       }

      return temp;
    }

   // Now, for looking in the zone above. Recurse down as far as needed to do so
   if (nullptr == turtle) // If I don't have a parent, then I'm in the top-left and there is no up
      return turtle;

   std::shared_ptr<MetaZone> temp_par = turtle->getSiblingUp(); // Get my parent's up sibling
   if (nullptr == temp_par) // Stop if it doesn't have one
      return temp_par;

   std::shared_ptr<MetaZone> temp = temp_par->children.get(ZoneDesc(desc.x, TOP, desc.d)); // Lookup cousin

   if (nullptr == temp.get()) // If my cousin doesn't exist, create them
    {
      temp = std::make_shared<MetaZone>(ZoneDesc(desc.x, TOP, desc.d), temp_par);
      ZoneImpl::create(temp);
      temp_par->children.add(temp->desc, temp);
    }

   return temp;
 }

std::shared_ptr<MetaZone> MetaZone::getSiblingDown()
 {
   if (desc.y < TOP) // Base case : lookup in parent
    {
      if (nullptr == turtle.get()) // If my parent doesn't exist, create them
       {
         turtle = std::make_shared<MetaZone>(ZoneDesc(0, 0, desc.d + 1)); // Parent not existing implies (x, y) is (0, 0)
         ZoneImpl::create(turtle);
       }

      std::shared_ptr<MetaZone> temp = turtle->children.get(ZoneDesc(desc.x, desc.y + 1, desc.d)); // Lookup sibling in parent

      if (nullptr == temp.get()) // If my sibling doesn't exist, create them
       {
         temp = std::make_shared<MetaZone>(ZoneDesc(desc.x, desc.y + 1, desc.d), turtle);
         ZoneImpl::create(temp);
         turtle->children.add(temp->desc, temp);
       }

      return temp;
    }

   // Now, for looking in the zone below. Recurse down as far as needed to do so
   if (nullptr == turtle) // If my parent doesn't exist, create them
    {
      turtle = std::make_shared<MetaZone>(ZoneDesc(0, 0, desc.d + 1)); // Parent not existing implies (x, y) is (0, 0)
      ZoneImpl::create(turtle);
    }

   std::shared_ptr<MetaZone> temp_par = turtle->getSiblingDown(); // Get my parent's down sibling : it MUST have a down sibling
   std::shared_ptr<MetaZone> temp = temp_par->children.get(ZoneDesc(desc.x, 0, desc.d)); // Lookup cousin

   if (nullptr == temp.get()) // If my cousin doesn't exist, create them
    {
      temp = std::make_shared<MetaZone>(ZoneDesc(desc.x, 0, desc.d), temp_par);
      ZoneImpl::create(temp);
      temp_par->children.add(temp->desc, temp);
    }

   return temp;
 }

std::shared_ptr<MetaZone> MetaZone::getSiblingLeft()
 {
   if (desc.x > 0) // Base case : lookup in parent
    {
      if (nullptr == turtle.get()) // If my parent doesn't exist, create them
       {
         turtle = std::make_shared<MetaZone>(ZoneDesc(0, 0, desc.d + 1)); // Parent not existing implies (x, y) is (0, 0)
         ZoneImpl::create(turtle);
       }

      std::shared_ptr<MetaZone> temp = turtle->children.get(ZoneDesc(desc.x - 1, desc.y, desc.d)); // Lookup sibling in parent

      if (nullptr == temp.get()) // If my sibling doesn't exist, create them
       {
         temp = std::make_shared<MetaZone>(ZoneDesc(desc.x - 1, desc.y, desc.d), turtle);
         ZoneImpl::create(temp);
         turtle->children.add(temp->desc, temp);
       }

      return temp;
    }

   // Now, for looking in the zone left. Recurse down as far as needed to do so
   if (nullptr == turtle) // If I don't have a parent, then I'm in the top-left and there is no left
      return turtle;

   std::shared_ptr<MetaZone> temp_par = turtle->getSiblingLeft(); // Get my parent's left sibling
   if (nullptr == temp_par) // Stop if it doesn't have one
      return temp_par;

   std::shared_ptr<MetaZone> temp = temp_par->children.get(ZoneDesc(TOP, desc.y, desc.d)); // Lookup cousin

   if (nullptr == temp.get()) // If my cousin doesn't exist, create them
    {
      temp = std::make_shared<MetaZone>(ZoneDesc(TOP, desc.y, desc.d), temp_par);
      ZoneImpl::create(temp);
      temp_par->children.add(temp->desc, temp);
    }

   return temp;
 }

std::shared_ptr<MetaZone> MetaZone::getSiblingRight()
 {
   if (desc.x < TOP) // Base case : lookup in parent
    {
      if (nullptr == turtle.get()) // If my parent doesn't exist, create them
       {
         turtle = std::make_shared<MetaZone>(ZoneDesc(0, 0, desc.d + 1)); // Parent not existing implies (x, y) is (0, 0)
         ZoneImpl::create(turtle);
       }

      std::shared_ptr<MetaZone> temp = turtle->children.get(ZoneDesc(desc.x + 1, desc.y, desc.d)); // Lookup sibling in parent

      if (nullptr == temp.get()) // If my sibling doesn't exist, create them
       {
         temp = std::make_shared<MetaZone>(ZoneDesc(desc.x + 1, desc.y, desc.d), turtle);
         ZoneImpl::create(temp);
         turtle->children.add(temp->desc, temp);
       }

      return temp;
    }

   // Now, for looking in the zone right. Recurse down as far as needed to do so
   if (nullptr == turtle) // If my parent doesn't exist, create them
    {
      turtle = std::make_shared<MetaZone>(ZoneDesc(0, 0, desc.d + 1)); // Parent not existing implies (x, y) is (0, 0)
      ZoneImpl::create(turtle);
    }

   std::shared_ptr<MetaZone> temp_par = turtle->getSiblingRight(); // Get my parent's right sibling : it MUST have a right sibling
   std::shared_ptr<MetaZone> temp = temp_par->children.get(ZoneDesc(0, desc.y, desc.d)); // Lookup cousin

   if (nullptr == temp.get()) // If my cousin doesn't exist, create them
    {
      temp = std::make_shared<MetaZone>(ZoneDesc(0, desc.y, desc.d), temp_par);
      ZoneImpl::create(temp);
      temp_par->children.add(temp->desc, temp);
    }

   return temp;
 }

void solve(const ZoneImpl& zone, std::unique_ptr<QuatroStack>& path, int sx, int sy, int fx, int fy);

class Solver
 {
public:
   std::unique_ptr<QuatroStack> right, down;
   int equal, location;
   bool useRight;
 };

void MetaZone::updateDirection()
 {
   if (nullptr == solve.get()) // Is this the very first call?
    {
      solve = std::make_shared<Solver>();

      if (nullptr == turtle.get()) // I don't have a parent, so I am the top-left
       {
         // I'm sorry: I didn't realize I did this until the last minute.
         ::solve(*impl, solve->down, 0, 0, bottom_c, TOP);
         ::solve(*impl, solve->right, 0, 0, TOP, right_c);

         solve->equal = 0;
         for (; (solve->equal < solve->down->sptr) && (solve->equal < solve->right->sptr); ++solve->equal)
          {
            if (solve->down->seek(solve->equal + 1) != solve->right->seek(solve->equal + 1))
             {
               break;
             }
          }
         solve->location = 0;
         last_direction = solve->down->seek(solve->location + 1);

         return;
       }
      else // My parent will guide me (don't I wish I had that in life?)
       {
         int from = turtle->last_direction;
         turtle->updateDirection();
         int to = turtle->last_direction;

         // TODO : finish
       }
    }

   if (solve->location < solve->equal)
    {
      ++solve->location;
      last_direction = solve->down->seek(solve->location + 1);
    }
   // TODO : Finish
 }
