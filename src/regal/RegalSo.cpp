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

#if REGAL_EMULATION

REGAL_GLOBAL_BEGIN

#include <cstring>

#include <string>
using std::string;

#include <boost/print/string_list.hpp>
typedef boost::print::string_list<string> string_list;

#include "RegalSo.h"
#include "RegalLog.h"
#include "RegalToken.h"

#include "lookup3.h"

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

using namespace ::REGAL_NAMESPACE_INTERNAL::Logging;
using namespace ::REGAL_NAMESPACE_INTERNAL::Token;

typedef RegalSo RSO;

void
RSO::GenSamplers(GLsizei count, GLuint *samplers)
{
    for (GLsizei ii=0; ii<count; ii++)
    {
        SamplingState* s = new SamplingState();
        samplerObjects[ nextSamplerObjectId ] = s;
        *samplers = nextSamplerObjectId;
        samplers++;
        nextSamplerObjectId++;
    }
}

void
RSO::DeleteSamplers(GLsizei count, const GLuint * samplers)
{
    for (GLsizei ii=0; ii<count; ii++)
    {
        GLuint s = samplers[ii];
        if (s && samplerObjects.count(s) > 0)
        {
            SamplingState* p = samplerObjects[s];
            for (GLsizei unit=0; unit < REGAL_EMU_MAX_TEXTURE_UNITS; unit++)
            {
                if (textureUnits[unit].boundSamplerObject == p)
                    BindSampler(unit, 0);
            }
            samplerObjects.erase(s);
            delete p;
        }
    }
}

GLboolean
RSO::IsSampler(GLuint sampler)
{
    return samplerObjects.count( sampler ) > 0 ? GL_TRUE : GL_FALSE;
}

void
RSO::BindSampler(GLuint unit, GLuint so)
{
    if (unit >= REGAL_EMU_MAX_TEXTURE_UNITS)
    {
        Warning("Texture unit out of range: ", unit, " >= ", REGAL_EMU_MAX_TEXTURE_UNITS);
        return;
    }

    if (so && samplerObjects.count(so) < 1)
    {
        Warning("Unrecognized sampler object: so = ", so);
        return;
    }

    textureUnits[unit].boundSamplerVersion = mainVer.Update();
    textureUnits[unit].boundSamplerObject = samplerObjects[so];
}

void
RSO::GenTextures(RegalContext * ctx, GLsizei count, GLuint *textures)
{
    ctx->dispatcher.emulation.glGenTextures(count, textures);

    for (GLsizei ii=0; ii<count; ii++)
    {
        GLuint to = textures[ii];
        TextureState* ts = new TextureState();
        textureObjects[ to ] = ts;
    }
}

void
RSO::DeleteTextures(RegalContext * ctx, GLsizei count, const GLuint * textures)
{
    GLuint originallyActiveUnit = activeTextureUnit;

    for (GLsizei ii=0; ii<count; ii++)
    {
        GLuint t = textures[ii];
        if (t && textureObjects.count(t) > 0)
        {
            TextureState* p = textureObjects[t];

            for (GLuint unit=0; unit < REGAL_EMU_MAX_TEXTURE_UNITS; unit++)
            {
                TextureUnit &tu = textureUnits[unit];

                for (GLuint jj=0; jj < REGAL_NUM_TEXTURE_TARGETS; jj++)
                {
                    if (p == tu.boundTextureObjects[jj])
                    {
                        if (activeTextureUnit != unit)
                            ActiveTexture(ctx, GL_TEXTURE0 + unit );

                        BindTexture(ctx, unit, TT_Index2Enum(jj), 0);
                    }
                }
            }

            textureObjects.erase(t);
            delete p;
        }
    }

    if (activeTextureUnit != originallyActiveUnit)
        ActiveTexture(ctx, GL_TEXTURE0 + originallyActiveUnit );
}

bool
RSO::BindTexture(RegalContext * ctx, GLenum target, GLuint to)
{
    return BindTexture(ctx, activeTextureUnit, target, to);
}

bool
RSO::BindTexture(RegalContext * ctx, GLuint unit, GLenum target, GLuint to)
{
    if (unit >= REGAL_EMU_MAX_TEXTURE_UNITS)
    {
        Warning("Texture unit out of range: ", unit, " >= ", REGAL_EMU_MAX_TEXTURE_UNITS);
        return false;
    }

    GLuint tti = TT_Enum2Index(target);

    if (tti >= REGAL_NUM_TEXTURE_TARGETS)
    {
        Warning("Invalid texture target: target = ", Token::GLenumToString(target));
        return false;
    }

    TextureState* ts = NULL;

    if (to && textureObjects.count(to) < 1)
    {
        ts = new TextureState();
        textureObjects[ to ] = ts;
    }
    else
    {
        ts = textureObjects[to];
    }

    DispatchTable &tbl = ctx->dispatcher.emulation;

    GLuint originallyActiveUnit = activeTextureUnit;

    if (activeTextureUnit != unit)
        ActiveTexture(ctx, GL_TEXTURE0 + unit );

    tbl.glBindTexture(target, to);

    if (activeTextureUnit != originallyActiveUnit)
        ActiveTexture(ctx, GL_TEXTURE0 + originallyActiveUnit );

    textureUnits[unit].boundTextureVersions[tti] = mainVer.Update();
    textureUnits[unit].boundTextureObjects[tti] = ts;

    return true;
}

bool
RSO::ActiveTexture( RegalContext * ctx, GLenum tex )
{
    GLuint unit = tex - GL_TEXTURE0;
    if (unit >= REGAL_EMU_MAX_TEXTURE_UNITS)
    {
        Warning( "Active texture out of range: ", tex, " >= ",
            Token::GLenumToString(GL_TEXTURE0 + REGAL_EMU_MAX_TEXTURE_UNITS));
        return false;
    }
    activeTextureUnit = unit;
    ctx->dispatcher.emulation.glActiveTexture( tex );
    return true;
}

void
RSO::PreDraw( RegalContext * ctx )
{
    GLuint originallyActiveUnit = activeTextureUnit;

    for (GLuint unit=0; unit < REGAL_EMU_MAX_TEXTURE_UNITS; unit++)
    {
        TextureUnit &tu = textureUnits[unit];

        SamplingState *pSS = tu.boundSamplerObject;

        bool needToUpdate = (tu.boundSamplerVersion > tu.ver);

        for (GLuint tt=0; tt < REGAL_NUM_TEXTURE_TARGETS; tt++)
        {
            SamplingState *app = pSS;
            SamplingState *drv = NULL;

            TextureState* ts = tu.boundTextureObjects[tt];

            if (tu.boundTextureVersions[tt] > tu.ver)
                needToUpdate = true;

            if (ts)
            {
                if (!app)
                    app = &ts->app;
                drv = &ts->drv;
            }
            else
            {
                if (!app)
                    app = &defaultTextureObjects[tt].app;
                drv = &defaultTextureObjects[tt].drv;
            }

            if (!app || !drv)
                continue;

            if (app->ver > tu.ver)
                needToUpdate = true;

            if (needToUpdate)
            {
                GLenum target = TT_Index2Enum(tt);
                SendStateToDriver(ctx, unit, target, *app, *drv);
            }
        }
        tu.ver = mainVer.Current();
    }

    if (activeTextureUnit != originallyActiveUnit)
        ActiveTexture(ctx, GL_TEXTURE0 + originallyActiveUnit );

    mainVer.Reset();
}

void
RSO::SendStateToDriver(RegalContext * ctx, GLuint unit, GLenum target, SamplingState& app, SamplingState& drv)
{
    if (target == GL_TEXTURE_2D_MULTISAMPLE ||
        target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY ||
        drv.ver >= app.ver )
    {
        drv.ver = mainVer.Current();
        return;
    }

    if (REGAL_FORCE_ES2_PROFILE || ctx->info->gles)
    {
        if (target != GL_TEXTURE_2D && target != GL_TEXTURE_CUBE_MAP)
        {
            drv.ver = mainVer.Current();
            return;
        }
    }

    DispatchTable &tbl = ctx->dispatcher.emulation;

    if (app.BorderColor[0] != drv.BorderColor[0] ||
        app.BorderColor[1] != drv.BorderColor[1] ||
        app.BorderColor[2] != drv.BorderColor[2] ||
        app.BorderColor[3] != drv.BorderColor[3])
    {
        if (activeTextureUnit != unit)
            ActiveTexture(ctx, GL_TEXTURE0 + unit );
        tbl.glTexParameteriv( target, GL_TEXTURE_BORDER_COLOR, app.BorderColor);
        drv.BorderColor[0] = app.BorderColor[0];
        drv.BorderColor[1] = app.BorderColor[1];
        drv.BorderColor[2] = app.BorderColor[2];
        drv.BorderColor[3] = app.BorderColor[3];
    }

    if (target != GL_TEXTURE_RECTANGLE)
    {
        if (app.MinFilter != drv.MinFilter)
        {
            if (activeTextureUnit != unit)
                ActiveTexture(ctx, GL_TEXTURE0 + unit );
            tbl.glTexParameteri( target, GL_TEXTURE_MIN_FILTER, app.MinFilter);
            drv.MinFilter = app.MinFilter;
        }

        if (app.MagFilter != drv.MagFilter)
        {
            if (activeTextureUnit != unit)
                ActiveTexture(ctx, GL_TEXTURE0 + unit );
            tbl.glTexParameteri( target, GL_TEXTURE_MAG_FILTER, app.MagFilter);
            drv.MagFilter = app.MagFilter;
        }

        if (app.WrapS != drv.WrapS)
        {
            if (activeTextureUnit != unit)
                ActiveTexture(ctx, GL_TEXTURE0 + unit );
            tbl.glTexParameteri( target, GL_TEXTURE_WRAP_S, app.WrapS);
            drv.WrapS = app.WrapS;
        }
    }

    if (target == GL_TEXTURE_2D ||
        target == GL_TEXTURE_3D ||
        target == GL_TEXTURE_CUBE_MAP)
    {
        if (app.WrapT != drv.WrapT)
        {
            if (activeTextureUnit != unit)
                ActiveTexture(ctx, GL_TEXTURE0 + unit );
            tbl.glTexParameteri( target, GL_TEXTURE_WRAP_T, app.WrapT);
            drv.WrapT = app.WrapT;
        }
    }

    if (target == GL_TEXTURE_3D)
    {
        if (app.WrapR != drv.WrapR)
        {
            if (activeTextureUnit != unit)
                ActiveTexture(ctx, GL_TEXTURE0 + unit );
            tbl.glTexParameteri( target, GL_TEXTURE_WRAP_R, app.WrapR);
            drv.WrapR = app.WrapR;
        }
    }

    if (app.MinLod != drv.MinLod)
    {
        if (activeTextureUnit != unit)
            ActiveTexture(ctx, GL_TEXTURE0 + unit );
        tbl.glTexParameterf( target, GL_TEXTURE_MIN_LOD, app.MinLod);
        drv.MinLod = app.MinLod;
    }

    if (app.MaxLod != drv.MaxLod)
    {
        if (activeTextureUnit != unit)
            ActiveTexture(ctx, GL_TEXTURE0 + unit );
        tbl.glTexParameterf( target, GL_TEXTURE_MAX_LOD, app.MaxLod);
        drv.MaxLod = app.MaxLod;
    }

    if (app.LodBias != drv.LodBias)
    {
        if (activeTextureUnit != unit)
            ActiveTexture(ctx, GL_TEXTURE0 + unit );
        tbl.glTexParameterf( target, GL_TEXTURE_LOD_BIAS, app.LodBias);
        drv.LodBias = app.LodBias;
    }

    if (app.CompareMode != drv.CompareMode)
    {
        if (activeTextureUnit != unit)
            ActiveTexture(ctx, GL_TEXTURE0 + unit );
        tbl.glTexParameteri( target, GL_TEXTURE_COMPARE_MODE, app.CompareMode);
        drv.CompareMode = app.CompareMode;
    }

    if (app.CompareFunc != drv.CompareFunc)
    {
        if (activeTextureUnit != unit)
            ActiveTexture(ctx, GL_TEXTURE0 + unit );
        tbl.glTexParameteri( target, GL_TEXTURE_COMPARE_FUNC, app.CompareFunc);
        drv.CompareFunc = app.CompareFunc;
    }

    if (ctx->info->gl_ext_texture_srgb_decode)
    {
        if (app.SrgbDecodeExt != drv.SrgbDecodeExt)
        {
            if (activeTextureUnit != unit)
                ActiveTexture(ctx, GL_TEXTURE0 + unit );
            tbl.glTexParameteri( target, GL_TEXTURE_SRGB_DECODE_EXT, app.SrgbDecodeExt);
            drv.SrgbDecodeExt = app.SrgbDecodeExt;
        }
    }

    if (ctx->info->gl_ext_texture_filter_anisotropic)
    {
        if (app.MaxAnisotropyExt != drv.MaxAnisotropyExt)
        {
            if (activeTextureUnit != unit)
                ActiveTexture(ctx, GL_TEXTURE0 + unit );
            tbl.glTexParameterf( target, GL_TEXTURE_MAX_ANISOTROPY_EXT, app.MaxAnisotropyExt);
            drv.MaxAnisotropyExt = app.MaxAnisotropyExt;
        }
    }

    drv.ver = mainVer.Current();
}

REGAL_NAMESPACE_END

#endif // REGAL_EMULATION
