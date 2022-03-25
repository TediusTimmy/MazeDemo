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

struct bmpheader
 {
   char header [54];
   int16_t& magic_number (void) { return *((int16_t *)(&header[0])); }
   int32_t& filesize (void) { return *((int32_t *)(&header[2])); }
   int16_t& reserved1 (void) { return *((int16_t *)(&header[6])); }
   int16_t& reserved2 (void) { return *((int16_t *)(&header[8])); }
   int32_t& pixel_offset (void) { return *((int32_t *)(&header[10])); }
   int32_t& header_size (void) { return *((int32_t *)(&header[14])); }
   int32_t& width (void) { return *((int32_t *)(&header[18])); }
   int32_t& height (void) { return *((int32_t *)(&header[22])); }
   int16_t& planes (void) { return *((int16_t *)(&header[26])); }
   int16_t& bpp (void) { return *((int16_t *)(&header[28])); }
   int32_t& compression (void) { return *((int32_t *)(&header[30])); }
   int32_t& img_size (void) { return *((int32_t *)(&header[34])); }
   int32_t& xscale (void) { return *((int32_t *)(&header[38])); }
   int32_t& yscale (void) { return *((int32_t *)(&header[42])); }
   int32_t& colors (void) { return *((int32_t *)(&header[46])); }
   int32_t& important_colors (void) { return *((int32_t *)(&header[50])); }
 };

void MakeBMP2(const Zone& board, const char * name)
 {
   std::ofstream file (name);
   if (!file)
      return;

   int linesize = MAX2 * 3;

   bmpheader header;
   header.magic_number() = 0x4D42;
   header.filesize() = 54 /*base header*/ + linesize * MAX2 /*line size * lines*/;
   header.reserved1() = 0;
   header.reserved2() = 0;
   header.pixel_offset() = 54;
   header.header_size() = 40;
   header.width() = MAX2;
   header.height() = MAX2;
   header.planes() = 1;
   header.bpp() = 24;
   header.compression() = 0;
   header.img_size() = header.filesize() - header.pixel_offset();
   header.xscale() = 0;
   header.yscale() = 0;
   header.colors() = 0;
   header.important_colors() = 0;

   file.write(header.header, 54);

   std::unique_ptr<char[]> row = std::make_unique<char[]>(linesize);
   int byte = 0;

   for (int y = MAX2 - 1; y > -1; --y)
    {
      byte = 0;
      std::fill(row.get(), row.get() + linesize, '\0');
      for (int x = 0; x < MAX2; ++x)
       {
         row[byte++] = board.image[y][x].c.r;
         row[byte++] = board.image[y][x].c.g;
         row[byte++] = board.image[y][x].c.b;
       }
      file.write(row.get(), linesize);
    }
 }
