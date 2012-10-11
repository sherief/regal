/*
  Copyright (c) 2011-2012 NVIDIA Corporation
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

#ifndef __REGAL_PRIVATE_H__
#define __REGAL_PRIVATE_H__

#include "RegalUtil.h"

REGAL_GLOBAL_BEGIN

#include <GL/Regal.h>

#include <cstdio>
#include <cstring>

#if !REGAL_SYS_WGL
#include <pthread.h>
#endif

#include "RegalUtil.h"
#include "RegalConfig.h"
#include "RegalLookup.h"
#include "RegalDispatch.h"

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

inline GLfloat  fixedToFloat (const GLfixed  v) { return v / 65536.0f;          }
inline GLdouble fixedToDouble(const GLfixed  v) { return v / 65536.0;           }
inline GLfixed  floatToFixed (const GLfloat  v) { return GLfixed(v * 65536.0f); }
inline GLdouble floatToDouble(const GLfloat  v) { return GLdouble(v);           }
inline GLfixed  doubleToFixed(const GLdouble v) { return GLfixed(v * 65536.0);  }
inline GLfloat  doubleToFloat(const GLdouble v) { return GLfloat(v);            }

inline void fixedToFloat(GLfloat *dest, const GLfixed *src, const std::size_t n)
{
  for (std::size_t i=0; i<n; ++i)
    dest[i] = fixedToFloat(src[i]);
}

inline void fixedToDouble(GLdouble *dest, const GLfixed *src, const std::size_t n)
{
  for (std::size_t i=0; i<n; ++i)
    dest[i] = fixedToFloat(src[i]);
}

inline void floatToFixed(GLfixed *dest, const GLfloat *src, const std::size_t n)
{
  for (std::size_t i=0; i<n; ++i)
    dest[i] = floatToFixed(src[i]);
}

inline void floatToDouble(GLdouble *dest, const GLfloat *src, const std::size_t n)
{
  for (std::size_t i=0; i<n; ++i)
    dest[i] = floatToDouble(src[i]);
}

inline void doubleToFixed(GLfixed *dest, const GLdouble *src, const std::size_t n)
{
  for (std::size_t i=0; i<n; ++i)
    dest[i] = doubleToFixed(src[i]);
}

inline void doubleToFloat(GLfloat *dest, const GLdouble *src, const std::size_t n)
{
  for (std::size_t i=0; i<n; ++i)
    dest[i] = doubleToFloat(src[i]);
}

using ::RegalErrorCallback;

#define REGAL_DECL_EXPORT 1
#define REGAL_SYS_WGL_DECLARE_WGL 1

// attrib mapping

enum RegalFixedFunctionAttrib {
    RFF2A_Vertex = 0,
    RFF2A_Weights = 1,
    RFF2A_Normal = 2,
    RFF2A_Color = 3,
    RFF2A_SecondaryColor = 4,
    RFF2A_FogCoord = 5,
    RFF2A_EdgeFlag = 6,
    RFF2A_TexCoord = 7,
    RFF2A_Invalid = 99
};

const GLenum RFFA2Enum[] = {
    GL_VERTEX_ARRAY,
    GL_WEIGHT_ARRAY_ARB,
    GL_NORMAL_ARRAY,
    GL_COLOR_ARRAY,
    GL_SECONDARY_COLOR_ARRAY,
    GL_FOG_COORDINATE_ARRAY,
    GL_EDGE_FLAG_ARRAY,
    GL_TEXTURE_COORD_ARRAY
};

const GLuint RFF2AMap8[] = {  0, 99, 2, 1, 99, 7, 99, 3 };
const GLuint RFF2AInvMap8[] = {
    RFF2A_Vertex,
    RFF2A_Color,
    RFF2A_Normal,
    RFF2A_TexCoord,
    RFF2A_TexCoord,
    RFF2A_TexCoord,
    RFF2A_SecondaryColor,
    RFF2A_FogCoord
};
const GLuint RFF2ATexBegin8 = 3;
const GLuint RFF2ATexEnd8 = 6;
const GLuint RFF2AMap16[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
const GLuint RFF2AInvMap16[] = {
    RFF2A_Vertex,
    RFF2A_Weights,
    RFF2A_Normal,
    RFF2A_Color,
    RFF2A_SecondaryColor,
    RFF2A_FogCoord,
    RFF2A_EdgeFlag,
    RFF2A_Invalid,
    RFF2A_TexCoord, RFF2A_TexCoord, RFF2A_TexCoord, RFF2A_TexCoord,
    RFF2A_TexCoord, RFF2A_TexCoord, RFF2A_TexCoord, RFF2A_TexCoord
};
const GLuint RFF2ATexBegin16 = 8;
const GLuint RFF2ATexEnd16 = 16;

// end attrib mapping

struct RegalContext;

extern RegalContext *currentContext;

#if REGAL_NO_TLS

typedef int Thread;

inline void *RegalPrivateGetCurrentContext()
{
  return currentContext;
}

#else // REGAL_NO_TLS

#if REGAL_SYS_WGL
#if REGAL_WIN_TLS

  // Windows TLS API

  typedef DWORD Thread;

  extern "C" { DWORD __stdcall TlsAlloc    (void);          }
  extern "C" { int   __stdcall TlsFree     (DWORD);         }
  extern "C" { void* __stdcall TlsGetValue (DWORD);         }
  extern "C" { int   __stdcall TlsSetValue (DWORD, void *); }

  extern DWORD regalCurrentContextTLSIDX;

  inline void * RegalPrivateGetCurrentContext()
  {
    return TlsGetValue( regalCurrentContextTLSIDX );
  }

#else // REGAL_WIN_TLS

  // Windows __declspec( thread )

  typedef DWORD Thread;

  extern __declspec( thread ) void *regalCurrentContext;

  inline void * RegalPrivateGetCurrentContext()
  {
    return regalCurrentContext;
  }

#endif // REGAL_WIN_TLS
#else  // REGAL_SYS_WGL
#if REGAL_SYS_OSX

  // Mac OS X

  typedef pthread_t Thread;
  extern pthread_key_t regalPrivateCurrentContextKey;

  inline void *RegalPrivateGetCurrentContext()
  {
    void *v = pthread_getspecific(regalPrivateCurrentContextKey);
    if (!v) {
      RegalMakeCurrent(CGLGetCurrentContext());
      return pthread_getspecific(regalPrivateCurrentContextKey);
    }
    return v;
  }

#else  // REGAL_SYS_OSX

  // Linux, etc

  typedef pthread_t Thread;
  extern pthread_key_t regalPrivateCurrentContextKey;

  inline void *RegalPrivateGetCurrentContext()
  {
    return pthread_getspecific(regalPrivateCurrentContextKey);
  }

#endif // REGAL_SYS_OSX
#endif // REGAL_SYS_WGL
#endif // REGAL_NO_TLS

#if REGAL_SYS_WGL
extern "C" { DWORD __stdcall GetCurrentProcessId (void);          }
extern "C" { DWORD __stdcall GetCurrentThreadId  (void);          }
#endif

// Current process ID

inline std::size_t procId()
{
#if REGAL_SYS_WGL
	return static_cast<std::size_t>(GetCurrentProcessId());
#elif REGAL_SYS_OSX || REGAL_SYS_OSX || REGAL_SYS_IOS || REGAL_SYS_ANDROID || REGAL_SYS_GLX
	return (std::size_t) getpid();
#else
  return 0;
#endif
}

// Current thread ID

inline std::size_t threadId()
{
#if REGAL_SYS_WGL
	return static_cast<std::size_t>(GetCurrentThreadId());
#elif REGAL_SYS_OSX || REGAL_SYS_OSX || REGAL_SYS_IOS || REGAL_SYS_ANDROID || REGAL_SYS_GLX

  // Revisit - pthread_self returns an opaque handle which isn't truely
  // a thread identified.

	return (std::size_t) pthread_self();
#else
  return 0;
#endif
}

REGAL_NAMESPACE_END

#include "RegalContext.h"

#define GET_REGAL_CONTEXT() ((::REGAL_NAMESPACE_INTERNAL::RegalContext *) ::REGAL_NAMESPACE_INTERNAL::RegalPrivateGetCurrentContext())

#endif // REGAL_PRIVATE_H
