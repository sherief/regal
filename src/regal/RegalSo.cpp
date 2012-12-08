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
typedef RegalSo::SamplerObject RSSO;

void
RSO::GenSamplers(GLsizei count, GLuint *samplers)
{
    for (GLsizei ii=0; ii<count; ii++)
    {
        SamplerObject* s = new SamplerObject();
        samplerObjects[ nextSamplerObjectId ] = s;
        *samplers = nextSamplerObjectId;
        samplers++;
        nextSamplerObjectId++;
    }
}

void
RSO::DeleteSamplers(RegalContext * ctx, GLsizei count, const GLuint * samplers)
{
    for (GLsizei ii=0; ii<count; ii++)
    {
        GLuint s = samplers[ii];
        if (s && samplerObjects.count(s) > 0)
        {
            for (GLsizei unit=0; unit < REGAL_EMU_MAX_TEXTURE_UNITS; unit++)
            {
                if (textureUnits[unit].boundSamplerObject == s)
                    BindSampler(ctx, unit, 0);
            }
            SamplerObject* p = samplerObjects[s];
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
RSO::BindSampler(RegalContext * ctx, GLuint unit, GLuint newSO)
{
    if (unit >= REGAL_EMU_MAX_TEXTURE_UNITS)
    {
        Warning("Texture unit out of range: ", unit, " >= ", REGAL_EMU_MAX_TEXTURE_UNITS);
        return;
    }

    if (newSO && samplerObjects.count(newSO) < 1)
    {
        Warning("Unrecognized sampler object: newSO = ", newSO);
        return;
    }

    TextureUnit &tu = textureUnits[unit];

    if (tu.boundSamplerObject == newSO)
        return;
    
    DispatchTable &tbl = ctx->dispatcher.emulation;

    GLuint originallyActiveUnit = activeTextureUnit;

    if (unit != activeTextureUnit)
        tbl.glActiveTexture( GL_TEXTURE0 + unit );

    SamplerObject *pSO = NULL;

    if (newSO)
        pSO = samplerObjects[newSO];

    for (GLsizei tti=0; tti < REGAL_NUM_TEXTURE_TARGETS; tti++)
    {
        GLenum target = TT_Index2Enum(tti);

        if (pSO)
        {
            pSO->SendStateToTextureUnit(target, unit, unit, tbl);
        }
        else
        {
            GLuint to = tu.boundTextureObjects[tti];
            if (to)
                textureObjects[to]->self.SendStateToTextureUnit(target, unit, unit, ctx->dispatcher.emulation);
            else
                defaultTextureObjects[tti].SendStateToTextureUnit(target, unit, unit, ctx->dispatcher.emulation);
        }
    }

    if (unit != originallyActiveUnit)
        tbl.glActiveTexture( GL_TEXTURE0 + originallyActiveUnit );

    tu.boundSamplerObject = newSO;
}

void
RSO::GenTextures(RegalContext * ctx, GLsizei count, GLuint *textures)
{
    ctx->dispatcher.emulation.glGenTextures(count, textures);

    for (GLsizei ii=0; ii<count; ii++)
    {
        GLuint name = textures[ii];
        TextureObject* t = new TextureObject();
        t->name = name;
        textureObjects[ name ] = t;
    }
}

void
RSO::DeleteTextures(RegalContext * ctx, GLsizei count, const GLuint * textures)
{
    DispatchTable &tbl = ctx->dispatcher.emulation;

    GLuint originallyActiveUnit = activeTextureUnit;

    for (GLsizei ii=0; ii<count; ii++)
    {
        GLuint t = textures[ii];
        if (t && textureObjects.count(t) > 0)
        {
            for (GLuint unit=0; unit < REGAL_EMU_MAX_TEXTURE_UNITS; unit++)
            {
                TextureUnit &tu = textureUnits[unit];

                for (GLuint jj=0; jj < REGAL_NUM_TEXTURE_TARGETS; jj++)
                {
                    if (t == tu.boundTextureObjects[jj])
                    {
                        if (unit != activeTextureUnit)
                            tbl.glActiveTexture( GL_TEXTURE0 + unit );

                        BindTexture(ctx, unit, TT_Index2Enum(jj), 0);
                    }
                }
            }
            TextureObject* p = textureObjects[t];
            textureObjects.erase(t);
            delete p;
        }
    }

    if (activeTextureUnit != originallyActiveUnit)
        tbl.glActiveTexture( GL_TEXTURE0 + originallyActiveUnit );
}

void
RSO::BindTexture(RegalContext * ctx, GLenum target, GLuint newTO)
{
    BindTexture(ctx, activeTextureUnit, target, newTO);
}

void
RSO::BindTexture(RegalContext * ctx, GLuint unit, GLenum target, GLuint newTO)
{
    if (unit >= REGAL_EMU_MAX_TEXTURE_UNITS)
    {
        Warning("Texture unit out of range: ", unit, " >= ", REGAL_EMU_MAX_TEXTURE_UNITS);
        return;
    }

    GLuint tti = TT_Enum2Index(target);

    if (tti >= REGAL_NUM_TEXTURE_TARGETS)
    {
        Warning("Invalid texture target: target = ", Token::GLenumToString(target));
        return;
    }

    if (newTO && textureObjects.count(newTO) < 1)
    {
        TextureObject* t = new TextureObject();
        t->name = newTO;
        textureObjects[ newTO ] = t;
    }

    TextureUnit &tu = textureUnits[unit];

    GLuint oldTO = tu.boundTextureObjects[tti];

    if (tu.boundSamplerObject)
    {
        if (oldTO)
        {
            RegalAssert(textureObjects.count(oldTO) > 0);
            textureObjects[oldTO]->self.SendStateToTextureUnit(target, unit, activeTextureUnit, ctx->dispatcher.emulation);
        }
        else
        {
            defaultTextureObjects[tti].SendStateToTextureUnit(target, unit, activeTextureUnit, ctx->dispatcher.emulation);
        }
    }

    ctx->dispatcher.emulation.glBindTexture(target, newTO);

    if (tu.boundSamplerObject)
    {
        samplerObjects[tu.boundSamplerObject]->SendStateToTextureUnit(target, unit, activeTextureUnit, ctx->dispatcher.emulation);
    }
    else
    {
        if (newTO)
            textureObjects[newTO]->self.SendStateToTextureUnit(target, unit, activeTextureUnit, ctx->dispatcher.emulation);
        else
            defaultTextureObjects[tti].SendStateToTextureUnit(target, unit, activeTextureUnit, ctx->dispatcher.emulation);
    }

    tu.boundTextureObjects[tti] = newTO;
}

void
RSSO::SendStateToTextureUnit(GLenum target, GLuint unit, GLuint currentlyActiveUnit, DispatchTable &tbl)
{
    GLuint originallyActiveUnit = currentlyActiveUnit;

    if (unit != currentlyActiveUnit)
        tbl.glActiveTexture( GL_TEXTURE0 + unit );

    tbl.glTexParameteriv(target, GL_TEXTURE_BORDER_COLOR, BorderColor);
    tbl.glTexParameteriv(target, GL_TEXTURE_MIN_FILTER, (GLint*)&MinFilter);
    tbl.glTexParameteriv(target, GL_TEXTURE_MAG_FILTER, (GLint*)&MagFilter);
    tbl.glTexParameteriv(target, GL_TEXTURE_WRAP_S, (GLint*)&WrapS);
    tbl.glTexParameteriv(target, GL_TEXTURE_WRAP_T, (GLint*)&WrapT);
    tbl.glTexParameteriv(target, GL_TEXTURE_WRAP_R, (GLint*)&WrapR);
    tbl.glTexParameterfv(target, GL_TEXTURE_MIN_LOD, &MinLod);
    tbl.glTexParameterfv(target, GL_TEXTURE_MAX_LOD, &MaxLod);
    tbl.glTexParameterfv(target, GL_TEXTURE_LOD_BIAS, &LodBias);
    tbl.glTexParameteriv(target, GL_TEXTURE_COMPARE_MODE, (GLint*)&CompareMode);
    tbl.glTexParameteriv(target, GL_TEXTURE_COMPARE_FUNC, (GLint*)&CompareFunc);
    tbl.glTexParameteriv(target, GL_TEXTURE_SRGB_DECODE_EXT, (GLint*)&SrgbDecodeExt);
    tbl.glTexParameterfv(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, &MaxAnisotropyExt);

    if (unit != originallyActiveUnit)
        tbl.glActiveTexture( GL_TEXTURE0 + originallyActiveUnit );
}

REGAL_NAMESPACE_END

#endif // REGAL_EMULATION
