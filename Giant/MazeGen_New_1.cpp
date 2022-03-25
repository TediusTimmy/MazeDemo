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

#include <cstdint>
#include <memory>

#include "city.h"
#include "pcg_random.hpp"

#include <iostream>
#include <iomanip>

#include "Zone.h"

__uint128_t MakeSeed(uint64_t a, uint64_t b)
 {
   char string [16];
   std::memcpy(string, reinterpret_cast<char*>(static_cast<void*>(&a)), 8);
   std::memcpy(string + 8, reinterpret_cast<char*>(static_cast<void*>(&b)), 8);
   uint128 res = CityHash128(string, 16);
   return static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
 }

__uint128_t MakeSeed3(uint64_t a, uint64_t b, uint64_t c)
 {
   char string [24];
   std::memcpy(string, reinterpret_cast<char*>(static_cast<void*>(&a)), 8);
   std::memcpy(string + 8, reinterpret_cast<char*>(static_cast<void*>(&b)), 8);
   std::memcpy(string + 16, reinterpret_cast<char*>(static_cast<void*>(&c)), 8);
   uint128 res = CityHash128(string, 24);
   return static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
 }

__uint128_t MakeSeed1(uint64_t a)
 {
   char string [8];
   std::memcpy(string, reinterpret_cast<char*>(static_cast<void*>(&a)), 8);
   uint128 res = CityHash128(string, 8);
   return static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
 }

void SetVisited(uint64_t* array, size_t x, size_t y)
 {
   size_t bit = y * MAX + x;
   array[bit / 64] |= 1ULL << (bit % 64);
 }

bool GetVisited(const uint64_t* array, size_t x, size_t y)
 {
   size_t bit = y * MAX + x;
   return (0 != (array[bit / 64] & 1ULL << (bit % 64)));
 }

void SetStack(uint64_t* stack, int ptr, int val)
 {
   ptr <<= 1;
   stack[ptr / 64] &= ~(3ULL << (ptr % 64));
   stack[ptr / 64] |= static_cast<uint64_t>(val & 3) << (ptr % 64);
 }

int GetStack(const uint64_t* stack, int ptr)
 {
   ptr <<= 1;
   return (stack[ptr / 64] >> (ptr % 64)) & 3;
 }

void ClearUp(uint64_t* array, size_t x, size_t y) // Assuming Set
 {
   size_t bit = (y * MAX + x) * 2;
   array[bit / 64] ^= 1ULL << (bit % 64);
 }

void ClearLeft(uint64_t* array, size_t x, size_t y) // Assuming Set
 {
   size_t bit = (y * MAX + x) * 2;
   array[bit / 64] ^= 2ULL << (bit % 64);
 }

bool GetUp(const uint64_t* array, size_t x, size_t y)
 {
   size_t bit = (y * MAX + x) * 2;
   return (0 != (array[bit / 64] & (1ULL << (bit % 64))));
 }

bool GetLeft(const uint64_t* array, size_t x, size_t y)
 {
   size_t bit = (y * MAX + x) * 2;
   return (0 != (array[bit / 64] & (2ULL << (bit % 64))));
 }

void Prev(int direction, int& x, int& y)
 {
   switch (direction)
    {
   case 0:
      ++x;
      break;
   case 1:
      --x;
      break;
   case 2:
      ++y;
      break;
   case 3:
      --y;
      break;
    }
 }

void create(pcg64& rng, ZoneImpl& zone)
 {
   std::unique_ptr<uint64_t[]> visited = std::make_unique<uint64_t[]>(ONE_BIT);
   std::fill(visited.get(), visited.get() + ONE_BIT, 0U);
   std::fill(zone.cell, zone.cell + TWO_BITS, 0xFFFFFFFFFFFFFFFFULL);

   int x = rng() & TOP;
   int y = rng() & TOP;
   SetVisited(visited.get(), x, y);

   std::unique_ptr<uint64_t[]> stack = std::make_unique<uint64_t[]>(TWO_BITS);
   int sptr = 0;

   do
    {
      int n = 0;
      if ((x - 1 >  -1) && (false == GetVisited(visited.get(), x - 1, y))) ++n;
      if ((x + 1 < MAX) && (false == GetVisited(visited.get(), x + 1, y))) ++n;
      if ((y - 1 >  -1) && (false == GetVisited(visited.get(), x, y - 1))) ++n;
      if ((y + 1 < MAX) && (false == GetVisited(visited.get(), x, y + 1))) ++n;

      while (n)
       {
         bool found = false;
         while (false == found)
          {
            int d = rng() & 3;
            switch (d)
             {
            case 0:
               if ((x - 1 >  -1) && (false == GetVisited(visited.get(), x - 1, y)))
                {
                  ClearLeft(zone.cell, x, y);
                  SetVisited(visited.get(), x - 1, y);
                  ++sptr;
                  SetStack(stack.get(), sptr, 0);
                  x = x - 1;
                  found = true;
                }
               break;
            case 1:
               if ((x + 1 < MAX) && (false == GetVisited(visited.get(), x + 1, y)))
                {
                  ClearLeft(zone.cell, x + 1, y);
                  SetVisited(visited.get(), x + 1, y);
                  ++sptr;
                  SetStack(stack.get(), sptr, 1);
                  x = x + 1;
                  found = true;
                }
               break;
            case 2:
               if ((y - 1 >  -1) && (false == GetVisited(visited.get(), x, y - 1)))
                {
                  ClearUp(zone.cell, x, y);
                  SetVisited(visited.get(), x, y - 1);
                  ++sptr;
                  SetStack(stack.get(), sptr, 2);
                  y = y - 1;
                  found = true;
                }
               break;
            case 3:
               if ((y + 1 < MAX) && (false == GetVisited(visited.get(), x, y + 1)))
                {
                  ClearUp(zone.cell, x, y + 1);
                  SetVisited(visited.get(), x, y + 1);
                  ++sptr;
                  SetStack(stack.get(), sptr, 3);
                  y = y + 1;
                  found = true;
                }
               break;
             }
          }

         n = 0;
         if ((x - 1 >  -1) && (false == GetVisited(visited.get(), x - 1, y))) ++n;
         if ((x + 1 < MAX) && (false == GetVisited(visited.get(), x + 1, y))) ++n;
         if ((y - 1 >  -1) && (false == GetVisited(visited.get(), x, y - 1))) ++n;
         if ((y + 1 < MAX) && (false == GetVisited(visited.get(), x, y + 1))) ++n;
       }

      Prev(GetStack(stack.get(), sptr), x, y);
      --sptr;
    }
   while (0 != sptr);
 }

void generate(const ZoneDesc& board, ZoneImpl& zone)
 {
   pcg64 top    (MakeSeed1(board.y));                       // Determines WHICH x-zone will get the opening, and WHERE it will be.
   pcg64 left   (MakeSeed3(board.x - 1, board.x, board.y)); // Determines WHERE the opening is.
   pcg64 right  (MakeSeed3(board.x, board.x + 1, board.y)); // Determines WHERE the opening is. Useful for path finding.
   pcg64 bottom (MakeSeed1(board.y + 1));                   // Determines WHICH x-zone will get the opening, and WHERE it will be. Useful for path finding.

   zone.top_z = top();
   zone.top_c = top() & TOP;
   zone.left_c = left() & TOP;
   zone.right_c = right() & TOP;
   zone.bottom_z = bottom();
   zone.bottom_c = bottom() & TOP;

   pcg64 rng  (MakeSeed(board.x, board.y));

   create(rng, zone);

   if (0 != board.x)
      ClearLeft(zone.cell, 0, zone.left_c);
   if ((0 != board.y) && (zone.top_z == board.y))
      ClearUp(zone.cell, zone.top_c, 0);
 }

/*
void uglyPrint(const ZoneImpl& board)
 {
   for (int y = 0; y < MAX; ++y)
    {
      for (int x = 0; x < MAX; ++x)
       {
         std::cout << '+';
         std::cout << (GetUp(board.cell, x, y) ? '-' : ' ');
       }
      std::cout << '\n';
      for (int x = 0; x < MAX; ++x)
       {
         std::cout << (GetLeft(board.cell, x, y) ? '|' : ' ');
         std::cout << ' ';
       }
      std::cout << '\n';
    }
 }
*/
#include "MakeBMP.h"
/*
int main (void)
 {
   for (int x = 1; x < 33; ++x)
    {
      ZoneDesc board;
      board.x = x;
      board.y = 11;

      std::unique_ptr<ZoneImpl> zone = std::make_unique<ZoneImpl>();

       // Generate maze
      generate(board, *zone.get());

       // Print Maze
      //uglyPrint(*zone.get());
      MakeBMP(*zone.get(), ("test" + std::to_string(x) + ".bmp").c_str());
    }

   return 0;
 }
*/
