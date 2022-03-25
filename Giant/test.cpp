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
#include <iostream>

// For MinGW, compile city.cc separately, with -D_MSC_VER

#include "city.h"
#include "pcg_random.hpp"

__uint128_t MakeSeed(uint64_t a, uint64_t b)
 {
   char string [16];
   std::memcpy(string, reinterpret_cast<char*>(static_cast<void*>(&a)), 8);
   std::memcpy(string + 8, reinterpret_cast<char*>(static_cast<void*>(&b)), 8);
   uint128 res = CityHash128(string, 16);
   return static_cast<__uint128_t>(Uint128Low64(res)) | ((static_cast<__uint128_t>(Uint128High64(res))) << 64);
 }

int main (void)
 {
   __uint128_t bob = MakeSeed(1, 2);
   pcg64 larry (bob);
   std::cout << static_cast<uint64_t>(bob) << " " << static_cast<uint64_t>(bob >> 64) << " " << larry() << std::endl;
   return 0;
 }
