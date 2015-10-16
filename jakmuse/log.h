/*
Copyright (c) 2015, Vlad Mesco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef LOG_H
#define LOG_H

#ifdef _MSC_VER
# define __func__ __FUNCTION__
#endif

#define LOGW(X) #X
#define LOGQ(X) LOGW(X)

#if 1
#define LOG(F, ...) do{\
    fprintf(stderr, __FILE__ ":" "%s" ":" LOGQ(__LINE__) ":    " F "\n", \
            __func__, \
            __VA_ARGS__); \
}while(0)
#else
#define LOG(F, ...) do{\
    fprintf(stderr, __FILE__ ":" LOGQ(__LINE__) ": " F " (" __func__ ")" "\n", __VA_ARGS__); \
}while(0)
#endif

#define LOGF(MASK, F, ...) do{\
    auto&& msk = (MASK); \
    if((msk & LOG_flags) == msk) LOG(F, __VA_ARGS__); \
}while(0)

#define LOGS(MASK) do{ LOG_flags = (MASK); }while(0)

#define LOGE(FLAG) (LOG_flags & (FLAG) != 0)

#define LOG_BLOCKS 0x1
#define LOG_PARSER 0x2
#define LOG_MAINLOOP 0x4
#define LOG_INTERPRETER 0x8
//... LOG_otherstuff

extern unsigned LOG_flags;

#endif
