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

const int MAX = 256;
const int TOP = MAX - 1;
const size_t ONE_BIT = (MAX < 64) ? (MAX * MAX) / 64 : MAX * (MAX / 64);
const size_t TWO_BITS = (MAX < 64) ? (MAX * MAX) / 32 : MAX * (MAX / 32);

struct ZoneImpl
 {
   public:
      uint32_t top_z, top_c, left_c, right_c, bottom_z, bottom_c; // Path finding variables.
      uint64_t cell [TWO_BITS]; // 2 bits per cell with 64 bits in type
 };

class ZoneDesc
 {
   public:
      uint32_t x, y;

      bool operator < (const ZoneDesc& rhs) const
       {
         if (x != rhs.x) return x < rhs.x;
         return y < rhs.y;
       }
 };

#endif /* ZONE_H */
