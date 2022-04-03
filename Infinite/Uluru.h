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

#ifndef ULURU_H
#define ULURU_H

#include <map>
#include <memory>

template<class T>
class Ayers
 {
public:
   std::shared_ptr<T> r;
   uint64_t a;
 };

template<class T, class D>
class Uluru // A Least-Recently Used cache
 {
private:
   uint64_t h;
   size_t cacheMax;
   std::map<D, Ayers<T> > cache;
public:
   Uluru(size_t maxSize) : h(0U), cacheMax(maxSize) { }

   std::shared_ptr<T> get(const D& from)
    {
      ++h;
      if (cache.end() == cache.find(from))
       {
         return std::shared_ptr<T>();
       }
      cache[from].a = h;
      return cache[from].r;
    }
   void add(const D& from, const std::shared_ptr<T>& n)
    {
      cache[from].r = std::move(n);
      if (cache.size() > cacheMax)
       {
         typename std::map<D, Ayers<T> >::iterator found = cache.begin();
         uint64_t b = found->second.a;
         for (typename std::map<D, Ayers<T> >::iterator iter = cache.begin(); cache.end() != iter; ++iter)
          {
            if (iter->second.a < b)
             {
               b = iter->second.a;
               found = iter;
             }
          }
         cache.erase(found);
       }
    }
 };

#endif /* ULURU_H */
