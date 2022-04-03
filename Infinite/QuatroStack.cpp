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

#include <algorithm>

#include "QuatroStack.h"

void QuatroStack::push(int val)
 {
   ++sptr;
   int ptr = sptr << 1;
   stack[ptr / 64] &= ~(3ULL << (ptr % 64));
   stack[ptr / 64] |= static_cast<uint64_t>(val & 3) << (ptr % 64);
 }

int QuatroStack::pop()
 {
   int ptr = sptr << 1;
   --sptr;
   return (stack[ptr / 64] >> (ptr % 64)) & 3;
 }

int QuatroStack::get(int depth) const
 {
   int ptr = (sptr - depth) << 1;
   return (stack[ptr / 64] >> (ptr % 64)) & 3;
 }

int QuatroStack::seek(int location) const
 {
   int ptr = location << 1;
   return (stack[ptr / 64] >> (ptr % 64)) & 3;
 }

void QuatroStack::Prev(int direction, int& x, int& y)
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

void QuatroStack::Next(int direction, int& x, int& y)
 {
   switch (direction)
    {
   case 0:
      --x;
      break;
   case 1:
      ++x;
      break;
   case 2:
      --y;
      break;
   case 3:
      ++y;
      break;
    }
 }

BitZone::BitZone()
 {
   std::fill(zone, zone + ONE_BIT, 0U);
 }

void BitZone::SetBit(size_t x, size_t y)
 {
   size_t bit = y * MAX + x;
   zone[bit / 64] |= 1ULL << (bit % 64);
 }

bool BitZone::GetBit(size_t x, size_t y) const
 {
   size_t bit = y * MAX + x;
   return (0 != (zone[bit / 64] & 1ULL << (bit % 64)));
 }
