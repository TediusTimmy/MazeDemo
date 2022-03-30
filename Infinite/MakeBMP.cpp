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

#include <fstream>
#include <iostream>

#include "bmpheader.h"

#include "Zone.h"

void ZoneImpl::UglyPrint() const
 {
   for (int y = 0; y < MAX; ++y)
    {
      for (int x = 0; x < MAX; ++x)
       {
         std::cout << '+';
         std::cout << (GetUp(x, y) ? '-' : ' ');
       }
      std::cout << '\n';
      for (int x = 0; x < MAX; ++x)
       {
         std::cout << (GetLeft(x, y) ? '|' : ' ');
         std::cout << ' ';
       }
      std::cout << '\n';
    }
 }

void AddNextBit(int& work, int& loc, int& byte, std::unique_ptr<char[]>& row, int bit)
 {
   bit &= 1;
   work |= (bit << loc);
   --loc;
   if (-1 == loc)
    {
      row[byte] = work;
      work = 0;
      loc = 7;
      ++byte;
    }
 }

void ZoneImpl::MakeBMP(const char * name) const
 {
   std::ofstream file (name);
   if (!file)
      return;

   int linesize = (MAX * 2) / 8; // we need 2x2 pixels per passage
   if (linesize < 4)
      linesize = 4;

   bmpheader header;
   header.magic_number() = 0x4D42;
   header.filesize() = 54 /*base header*/ + 8 /*palette*/ + linesize * (MAX * 2) /*line size * lines*/;
   header.reserved1() = 0;
   header.reserved2() = 0;
   header.pixel_offset() = 54 + 8;
   header.header_size() = 40;
   header.width() = MAX * 2;
   header.height() = MAX * 2;
   header.planes() = 1;
   header.bpp() = 1;
   header.compression() = 0;
   header.img_size() = header.filesize() - header.pixel_offset();
   header.xscale() = 0;
   header.yscale() = 0;
   header.colors() = 0;
   header.important_colors() = 0;

   file.write(header.header, 54);
   file.write("\0\0\0\0""\377\377\377\0", 8);

   std::unique_ptr<char[]> row = std::make_unique<char[]>(linesize);
   int work = 0, loc = 7, byte = 0;

   for (int y = TOP; y > -1; --y)
    {
      work = 0;
      loc = 7;
      byte = 0;
      std::fill(row.get(), row.get() + linesize, '\0');
      for (int x = 0; x < MAX; ++x)
       {
         AddNextBit(work, loc, byte, row, (GetLeft(x, y) ? 0 : 1));
         AddNextBit(work, loc, byte, row, 1);
       }
      file.write(row.get(), linesize);

      work = 0;
      loc = 7;
      byte = 0;
      std::fill(row.get(), row.get() + linesize, '\0');
      for (int x = 0; x < MAX; ++x)
       {
         AddNextBit(work, loc, byte, row, 0);
         AddNextBit(work, loc, byte, row, (GetUp(x, y) ? 0 : 1));
       }
      file.write(row.get(), linesize);
    }
 }
