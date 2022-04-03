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

#ifndef ZONE_H
#define ZONE_H

#include <memory>
#include <vector>

#include "Limits.h"
#include "Uluru.h"

class ZoneDesc
 {
public:
   uint32_t x, y, d;

   ZoneDesc(uint32_t x, uint32_t y, uint32_t d) : x(x), y(y), d(d) { }
   ZoneDesc(const ZoneDesc&) = default;
   ZoneDesc& operator= (const ZoneDesc&) = default;

   bool operator < (const ZoneDesc& rhs) const
    {
      if (x != rhs.x) return x < rhs.x;
      if (y != rhs.y) return y < rhs.y;
      return d < rhs.d;
    }
 };

class MetaZone // A class for building the fine-structure of the maze
 {
public:
   uint32_t x, y, d;                                  // Location
   uint32_t top_c, left_c, right_c, bottom_c;         // Path finding variables
   __uint128_t turtle_hash;                           // RNG helper
   __uint128_t seed;                                  // The seed
   std::shared_ptr<MetaZone> turtle;                  // If turtle is NULL, implies (0, 0, d + 1)
   Uluru<MetaZone, ZoneDesc> children;                // LRU cache of sub-mazes at (x, y, d - 1)

   MetaZone(const ZoneDesc& zone, std::shared_ptr<MetaZone> turtle_ptr = std::shared_ptr<MetaZone>());
   MetaZone(const MetaZone&) = default;
   MetaZone& operator= (const MetaZone&) = default;

private:
   void turtleDown(std::vector<char>&) const;
 };

class ZoneImpl
 {
public:
   uint32_t top_c, left_c, right_c, bottom_c; // Path finding variables
   uint64_t cell [TWO_BITS];                  // 2 bits per cell with 64 bits in type

   static std::shared_ptr<ZoneImpl> create (const MetaZone &zone);

   void ClearUp(size_t x, size_t y);
   void ClearLeft(size_t x, size_t y);

   bool GetUp(size_t x, size_t y) const;
   bool GetLeft(size_t x, size_t y) const;
   bool GetDown(size_t x, size_t y) const; // Poor locality
   bool GetRight(size_t x, size_t y) const;

   void UglyPrint() const;
   void MakeBMP(const char* name) const;
 };

#endif /* ZONE_H */
