/*
  Copyright (c) 2011 NVIDIA Corporation
  Copyright (c) 2011-2012 Cass Everitt
  Copyright (c) 2012 Scott Nations
  Copyright (c) 2012 Mathias Schott
  Copyright (c) 2012 Nigel Stewart
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
  OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __REGAL_PUSH_H__
#define __REGAL_PUSH_H__

#include "RegalUtil.h"

REGAL_GLOBAL_BEGIN

#include <cstring>    // For memcpy

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

//
// A utility class for temporarily pushing the memory of an object to
// the stack.  When the Push object goes out of scope, the memory
// is copied back again.
//

template<typename T>
struct Push
{
public:
  Push(T &var)
  : _var(&var)
  {
    std::memcpy(_buffer,_var,sizeof(T));
  }

  Push(T *var)
  : _var(var)
  {
    if (_var)
      std::memcpy(_buffer,_var,sizeof(T));
  }

  Push(T &var, T offset)
  : _var(&var)
  {
    std::memcpy(_buffer,_var,sizeof(T));
    (*_var) += offset;
  }

  Push(T *var, T offset)
  : _var(var)
  {
    if (_var)
    {
      std::memcpy(_buffer,_var,sizeof(T));
      (*_var) += offset;
    }
  }

  ~Push()
  {
    if (_var)
      std::memcpy(_var,&_buffer,sizeof(T));
  }

private:
  T    *_var;
  char  _buffer[sizeof(T)];
};

REGAL_NAMESPACE_END

#endif
