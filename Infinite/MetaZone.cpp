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

#ifdef DEBUG_PATHFINDING
 #include <iostream>
 const char* decode = "LRUD";
#endif /* DEBUG_PATHFINDING */

#include "Zone.h"
#include "QuatroStack.h"

#include "external/city.h"
#include "external/pcg_random.hpp"

std::vector<int> MetaZone::directions;

void MetaZone::cacheMeOut(std::shared_ptr<MetaZone> me)
 {
   if (nullptr != me->turtle.get())
    {
      me->turtle->children.add(me->desc, me);
      cacheMeOut(me->turtle);
    }
 }

bool MetaZone::zerosAllTheWayDown() const
 {
   if ((0 == desc.x) && (0 == desc.y))
    {
      if (nullptr != turtle.get())
       {
         return turtle->zerosAllTheWayDown();
       }
      return true;
    }
   return false;
 }

void MetaZone::turtleDown(std::vector<char>& stack) const
 {
   if (false == zerosAllTheWayDown())
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

__uint128_t MakeSeed2(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
 {
   char string [16];
   std::memcpy(string, reinterpret_cast<char*>(static_cast<void*>(&a)), 4);
   std::memcpy(string + 4, reinterpret_cast<char*>(static_cast<void*>(&b)), 4);
   std::memcpy(string + 8, reinterpret_cast<char*>(static_cast<void*>(&c)), 4);
   std::memcpy(string + 12, reinterpret_cast<char*>(static_cast<void*>(&d)), 4);
   uint128 res = CityHash128(string, 16);
   return static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
 }

MetaZone::MetaZone(const ZoneDesc& zone, std::shared_ptr<MetaZone> turtle_ptr) : desc(zone), turtle(turtle_ptr), children((zone.d <= CUTOFF) ? CACHE_MAX : CACHE_MIN)
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

   pcg64 top    (MakeSeed2(desc.x, (desc.y - 1) & TOP, desc.y, desc.d));
   pcg64 left   (MakeSeed2((desc.x - 1) & TOP, desc.x, desc.y, desc.d));
   pcg64 right  (MakeSeed2(desc.x, (desc.x + 1) & TOP, desc.y, desc.d));
   pcg64 bottom (MakeSeed2(desc.x, desc.y, (desc.y + 1) & TOP, desc.d));

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
   if (nullptr == turtle.get()) // If I don't have a parent, then I'm in the top-left and there is no up
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
   if (nullptr == turtle.get()) // If my parent doesn't exist, create them
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
   if (nullptr == turtle.get()) // If I don't have a parent, then I'm in the top-left and there is no left
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
   if (nullptr == turtle.get()) // If my parent doesn't exist, create them
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

void MetaZone::normalUpdate()
 {
   int from = turtle->lastDirection();
   turtle->updateDirection();
   int to = turtle->lastDirection();

   int sx = 0, sy = 0, fx = 0, fy = 0;
   switch (from)
    {
   case 0:
      sx = TOP;
      sy = right_c;
      break;
   case 1:
      sx = 0;
      sy = left_c;
      break;
   case 2:
      sx = bottom_c;
      sy = TOP;
      break;
   case 3:
      sx = top_c;
      sy = 0;
      break;
    }
   switch (to)
    {
   case 0:
      fx = 0;
      fy = left_c;
      break;
   case 1:
      fx = TOP;
      fy = right_c;
      break;
   case 2:
      fx = top_c;
      fy = 0;
      break;
   case 3:
      fx = bottom_c;
      fy = TOP;
      break;
    }
   ::solve(*impl, solve->down, sx, sy, fx, fy);
   solve->down->push(to); // Add the final step

#ifdef DEBUG_PATHFINDING
std::cout << "At (" << desc.x << ", " << desc.y << ", " << desc.d << ") :" << std::endl;
std::cout << "\tExits : top " << top_c << ", bottom " << bottom_c << ", left " << left_c << ", right " << right_c << std::endl;
std::cout << "\tPath length " << solve->down->sptr  << " from " << decode[from] << " to " << decode[to] << std::endl;
std::cout << "\tSolving (" << sx << ", " << sy << ") -> (" << fx << ", " << fy << ") : " << solve->down->sptr << std::endl;
std::cout << "Path : ";
for (int i = 0; i < solve->down->sptr; ++i) std::cout << decode[solve->down->seek(i + 1)];
std::cout << std::endl;
#endif /* DEBUG_PATHFINDING */

   solve->equal = solve->down->sptr;
   solve->location = 0;
   lastDirection() = solve->down->seek(solve->location + 1);
 }

void MetaZone::fullPath()
 {
   if (nullptr == turtle.get()) // If my parent doesn't exist, create them
    {
      turtle = std::make_shared<MetaZone>(ZoneDesc(0, 0, desc.d + 1)); // Parent not existing implies (x, y) is (0, 0)
      ZoneImpl::create(turtle);
    }

   if (nullptr == turtle->solve.get())
    {
      ++solve->location;
      turtle->updateDirection(); // Now, update their path

      switch (turtle->lastDirection()) // Only two options: right or down
       {
      case 1:
         solve->useRight = true;
         solve->equal = solve->right->sptr;
         lastDirection() = solve->right->seek(solve->location + 1);
         break;
      case 3:
         solve->useRight = false;
         solve->equal = solve->down->sptr;
         lastDirection() = solve->down->seek(solve->location + 1);
         break;
       }
#ifdef DEBUG_PATHFINDING
std::cout << "\tFinal path length for (" << desc.x << ", " << desc.y << ", " << desc.d << ") : " << solve->equal << std::endl;
#endif /* DEBUG_PATHFINDING */
    }
   else // Parent exists and we are following a path.
    {
      solve->useRight = false;
      normalUpdate();
    }
 }

void MetaZone::updateDirection()
 {
   if (nullptr == solve.get()) // Is this the very first call?
    {
      if (desc.d >= directions.size())
       {
         directions.resize(desc.d + 1, -1);
       }
      solve = std::make_shared<Solver>();
      solve->useRight = false;

      if (-1 == directions[desc.d]) // I don't have a previous direction, so I am the top-left
       {
         // I'm sorry: I didn't realize I did this until the last minute.
         ::solve(*impl, solve->down, 0, 0, bottom_c, TOP);
         ::solve(*impl, solve->right, 0, 0, TOP, right_c);
         solve->down->push(3); // Add the final step out
         solve->right->push(1);

         solve->equal = 0;
         for (; (solve->equal < solve->down->sptr) && (solve->equal < solve->right->sptr); ++solve->equal)
          {
            if (solve->down->seek(solve->equal + 1) != solve->right->seek(solve->equal + 1))
             {
               break;
             }
          }
         --solve->equal;

#ifdef DEBUG_PATHFINDING
std::cout << "At (" << desc.x << ", " << desc.y << ", " << desc.d << ") :" << std::endl;
std::cout << "\tExits : top " << top_c << ", bottom " << bottom_c << ", left " << left_c << ", right " << right_c << std::endl;
std::cout << "\tFirst path length " << solve->equal << std::endl;
std::cout << "\tSolving Down (0, 0) -> (" << bottom_c << ", " << TOP << ") : " << solve->down->sptr << std::endl;
std::cout << "Down : ";
for (int i = 0; i < solve->down->sptr; ++i) std::cout << decode[solve->down->seek(i + 1)];
std::cout << std::endl;
std::cout << "\tSolving Right (0, 0) -> (" << TOP << ", " << right_c << ") : " << solve->right->sptr << std::endl;
std::cout << "Right : ";
for (int i = 0; i < solve->right->sptr; ++i) std::cout << decode[solve->right->seek(i + 1)];
std::cout << std::endl;
#endif /* DEBUG_PATHFINDING */

         if (-1 == solve->equal) // Do we have to compute the full path anyway?
          {
            solve->location = -1;
            fullPath();
          }
         else
          {
            solve->location = 0;
            lastDirection() = solve->down->seek(solve->location + 1);
          }
       }
      else // My parent will guide me (don't I wish I had that in life?)
       {
         normalUpdate();
       }
    }

   else if (solve->location < solve->equal) // Subsequent calls before deciding which way to go, or after deciding which way to go, or when we know which way to go.
    {
      ++solve->location;
      if (false == solve->useRight)
         lastDirection() = solve->down->seek(solve->location + 1);
      else
         lastDirection() = solve->right->seek(solve->location + 1);
    }
   else // Recurse down and decide which way to go
    {
      fullPath();
    }
#ifdef DEBUG_PATHFINDING
std::cout << "Move " << solve->location << " for (" << desc.x << ", " << desc.y << ", " << desc.d << ") : " << decode[lastDirection()] << std::endl;
#endif /* DEBUG_PATHFINDING */
 }
