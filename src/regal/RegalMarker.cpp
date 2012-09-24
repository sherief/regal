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

#include "pch.h" /* For MS precompiled header support */

#include "RegalUtil.h"

REGAL_GLOBAL_BEGIN

#include "md5.h"

#include "RegalMarker.h"
#include "RegalPrivate.h"
#include "RegalDispatcher.h"

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

void Marker::FrameTerminator(RegalContext &context)
{
  Internal("Regal::Marker::FrameTerminator ",&context);

  if
  (
    Config::frameMd5Color    ||
    Config::frameMd5Stencil  ||
    Config::frameMd5Depth    ||
    Config::frameSaveColor   ||
    Config::frameSaveStencil ||
    Config::frameSaveDepth
  )
  {
    DispatchTable *_next = &context.dispatcher.front();
    RegalAssert(_next);

    // Get the current viewport

    GLint viewport[4];
    memset(viewport,0,sizeof(viewport));
    _next->call(&_next->glGetIntegerv)(GL_VIEWPORT, viewport);

    const GLint &width  = viewport[2];
    const GLint &height = viewport[3];

    if (width>0 && height>0)
    {
      if (Config::frameMd5Color || Config::frameSaveColor)
      {
        // Read the pixels as RGBA

        const GLint bufferSize = width*height*4*sizeof(GLbyte);
        GLbyte *buffer = (GLbyte *) calloc(bufferSize,1);

       _next->call(&_next->glReadPixels)(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

        // Do once we have the pixels, could we do the rest in another
        // thread?

        // Compute pixel md5sum

        MD5Context md5c;
        MD5Init(&md5c);
        MD5Update(&md5c, buffer, bufferSize);

        unsigned char digest[16];
        MD5Final(digest, &md5c);

        free(buffer);

        // Convert md5sum to string

        char md5sum[sizeof(digest)*2+1];
        for (size_t i=0; i<sizeof(digest); ++i)
          sprintf(md5sum+i*2,"%02x",digest[i]);

        Info("Color md5sum ",md5sum);
      }
    }
  }
}

REGAL_NAMESPACE_END
