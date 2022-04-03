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

#include "external/city.h"
#include "external/pcg_random.hpp"

void MetaZone::turtleDown(std::vector<char>& stack) const
 {
   if ((0 != x) || (0 != y) || (nullptr != turtle.get()))
    {
      stack.push_back(x & 255);
      stack.push_back(x >> 8);
      stack.push_back(y & 255);
      stack.push_back(y >> 8);

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

MetaZone::MetaZone(const ZoneDesc& zone, std::shared_ptr<MetaZone> turtle_ptr) : turtle(turtle_ptr), children(8U)
 {
   x = zone.x;
   y = zone.y;
   d = zone.d;

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

   seed = MakeSeed(x, y, d, turtle_hash);

   pcg64 top    (MakeSeed(x, (y - 1) & TOP, y, d, turtle_hash));
   pcg64 left   (MakeSeed((x - 1) & TOP, x, y, d, turtle_hash));
   pcg64 right  (MakeSeed(x, (x + 1) & TOP, y, d, turtle_hash));
   pcg64 bottom (MakeSeed(x, y, (y + 1) & TOP, d, turtle_hash));

   top_c = top() & TOP;
   left_c = left() & TOP;
   right_c = right() & TOP;
   bottom_c = bottom() & TOP;
 }
