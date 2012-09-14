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

#ifndef __REGAL_DISPATCHER_H__
#define __REGAL_DISPATCHER_H__

#include "RegalUtil.h"

REGAL_GLOBAL_BEGIN

#include <vector>

#include "RegalDispatch.h"

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

struct Dispatcher
{
public:

#if REGAL_DEBUG
   DispatchTable debug;
#endif

#if REGAL_ERROR
   DispatchTable error;
#endif

#if REGAL_EMULATION
   DispatchTable emulation;
#endif

#if REGAL_LOG
   DispatchTable logging;
#endif

   DispatchTable driver;   // Underlying OpenGL/ES implementation

   DispatchTable missing;  // Must have this last

public:
  Dispatcher();
  ~Dispatcher();

  void push_back(DispatchTable &table, bool enable);

  void enable(DispatchTable &table);
  void disable(DispatchTable &table);

  bool isEnabled(DispatchTable &table) const;

  DispatchTable &
  table()
  {
    std::size_t i = current;
    while (!enabled[i])
      ++i;
    RegalAssert(i<enabled.size());
    RegalAssert(enabled[i]);
    return *enabled[i];
  }

  // Lookup a function pointer from the current
  // dispatch table, or deeper in the stack as
  // necessary.

  template<typename T>
  T call(T *func)
  {
    RegalAssert(func);
    if (*func)
      return *func;

    RegalAssert(current<enabled.size());

    while (!enabled[current])
      ++current;

    DispatchTable *t = enabled[current];

    RegalAssert(reinterpret_cast<void *>(func)>=reinterpret_cast<void *>(t));
    RegalAssert(reinterpret_cast<void *>(func)< reinterpret_cast<void *>(t+1));

    std::size_t offset = reinterpret_cast<char *>(func) - reinterpret_cast<char *>(t);

    T f = *func;

    while (!f)
    {
      for (++current; !enabled[current]; ++current) {}
      RegalAssert(current<enabled.size());
      RegalAssert(enabled[current]);
      f = *reinterpret_cast<T *>(reinterpret_cast<char *>(enabled[current])+offset);
    }

    return f;
  }

  struct ScopedStep
  {
  public:
    ScopedStep(Dispatcher &dispatcher)
    : _dispatcher(dispatcher),
      _previous(dispatcher.current)
    {
      ++dispatcher.current;
      
      // Skip disabled layers

      while (!dispatcher.enabled[dispatcher.current])
        ++dispatcher.current;

      RegalAssert(dispatcher.current<dispatcher.enabled.size());
      RegalAssert(dispatcher.current<dispatcher.disabled.size());
    }

    ScopedStep(Dispatcher *dispatcher)
    : _dispatcher(*dispatcher),
      _previous(dispatcher->current)
    {
      RegalAssert(dispatcher);

      ++dispatcher->current;
      
      // Skip disabled layers

      while (!dispatcher->enabled[dispatcher->current])
        ++dispatcher->current;

      RegalAssert(dispatcher->current<dispatcher->enabled.size());
      RegalAssert(dispatcher->current<dispatcher->disabled.size());
    }

    ~ScopedStep()
    {
      _dispatcher.current = _previous;
    }

  private:
    Dispatcher  &_dispatcher;
    std::size_t  _previous;    // Previous current

    // make these private, not needed

    ScopedStep(const ScopedStep &other);
    ScopedStep &operator=(const ScopedStep &other);
  };

private:
  std::vector<DispatchTable *> enabled;
  std::vector<DispatchTable *> disabled;

  std::size_t                  current;   // 0 is the top, enabled.size()-1 the bottom
};

REGAL_NAMESPACE_END

#endif // __REGAL_DISPATCHER__
