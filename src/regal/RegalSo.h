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

/*

 Regal sampler object emulation.
 Scott Nations

 */

#ifndef __REGAL_SO_H__
#define __REGAL_SO_H__

#include "RegalUtil.h"

#if REGAL_EMULATION

REGAL_GLOBAL_BEGIN

#include <climits>
#include <cstring>

#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include "RegalEmu.h"
#include "RegalPrivate.h"
#include "RegalContext.h"
#include "RegalContextInfo.h"
#include "RegalSharedMap.h"
#include "RegalToken.h"
#include "linear.h"

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

#define REGAL_EMU_MAX_TEXTURE_UNITS 16
#define REGAL_NUM_TEXTURE_TARGETS 10

struct RegalSo : public RegalEmu
{
    RegalSo()
    : activeTextureUnit(0)
    , nextSamplerObjectId(1)
    {
    }

    RegalSo(const RegalSo &other)
    {
        memcpy(this,&other,sizeof(SamplerObject));
    }

    RegalSo &operator=(const RegalSo &other)
    {
        if (&other!=this)
            memcpy(this,&other,sizeof(SamplerObject));
        return *this;
    }

    ~RegalSo()
    {
    }

    void Init( RegalContext &ctx )
    {
        UNUSED_PARAMETER(ctx);
        activeTextureUnit = 0;
        nextSamplerObjectId = 1;
    }

    GLenum TT_Index2Enum(GLuint index)
    {
        switch (index)
        {
            case 0: return GL_TEXTURE_1D;
            case 1: return GL_TEXTURE_2D;
            case 2: return GL_TEXTURE_3D;
            case 3: return GL_TEXTURE_1D_ARRAY;
            case 4: return GL_TEXTURE_2D_ARRAY;
            case 5: return GL_TEXTURE_RECTANGLE;
            case 6: return GL_TEXTURE_BUFFER;
            case 7: return GL_TEXTURE_CUBE_MAP;
            case 8: return GL_TEXTURE_2D_MULTISAMPLE;
            case 9: return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
            default:
                Warning( "Unhandled texture target index: index = ", index);
                break;
        }
        return REGAL_NUM_TEXTURE_TARGETS;
    }

    GLuint TT_Enum2Index(GLenum texture)
    {
        switch (texture)
        {
            case GL_TEXTURE_1D: return 0;
            case GL_TEXTURE_2D: return 1;
            case GL_TEXTURE_3D: return 2;
            case GL_TEXTURE_1D_ARRAY: return 3;
            case GL_TEXTURE_2D_ARRAY: return 4;
            case GL_TEXTURE_RECTANGLE: return 5;
            case GL_TEXTURE_BUFFER: return 6;
            case GL_TEXTURE_CUBE_MAP: return 7;
            case GL_TEXTURE_2D_MULTISAMPLE: return 8;
            case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: return 9;
            default:
                Warning( "Unhandled texture target enum: texture = ", Token::GLenumToString(texture));
                break;
        }
        return ~(GLuint(0));
    }

    struct SamplerObject
    {
        GLint BorderColor[4];     //  Border color
        GLenum MinFilter;         //  Minification function
        GLenum MagFilter;         //  Magnification function
        GLenum WrapS;             //  Texcoord s wrap mode
        GLenum WrapT;             //  Texcoord t wrap mode
        GLenum WrapR;             //  Texcoord r wrap mode
        GLfloat MinLod;           //  Minimum level of detail
        GLfloat MaxLod;           //  Maximum level of detail
        GLfloat LodBias;          //  Texture level of detail bias (biastexobj)
        GLenum CompareMode;       //  Comparison mode
        GLenum CompareFunc;       //  Comparison function
        GLenum SrgbDecodeExt;     //  sRGB decode mode
        GLfloat MaxAnisotropyExt; //  maximum degree of anisotropy

        SamplerObject()
        : MinFilter(GL_NEAREST_MIPMAP_LINEAR)
        , MagFilter(GL_LINEAR)
        , WrapS(GL_REPEAT)
        , WrapT(GL_REPEAT)
        , WrapR(GL_REPEAT)
        , MinLod(-1000.0)
        , MaxLod(1000.0)
        , LodBias(0.0)
        , CompareMode(GL_NONE)
        , CompareFunc(GL_LEQUAL)
        , SrgbDecodeExt(GL_DECODE_EXT)
        , MaxAnisotropyExt(1.0)
        {
            BorderColor[0] = BorderColor[1] = BorderColor[2] = BorderColor[3] = 0;
        }

        SamplerObject(const SamplerObject &other)
        {
            memcpy(this,&other,sizeof(SamplerObject));
        }

        SamplerObject &operator=(const SamplerObject &other)
        {
            if (&other!=this)
                memcpy(this,&other,sizeof(SamplerObject));
            return *this;
        }

        void SendStateToTextureUnit(GLenum target, GLuint unit, GLuint activeUnit, DispatchTable &tbl);
    };

    struct TextureObject
    {
        GLuint name;
        SamplerObject self;

        TextureObject()
        : name(0)
        {
        }
    };

    struct TextureUnit
    {
        GLuint boundTextureObjects[REGAL_NUM_TEXTURE_TARGETS];
        GLuint boundSamplerObject;

        TextureUnit()
        : boundSamplerObject(0)
        {
            memset(boundTextureObjects,0,sizeof(boundTextureObjects));
        }

        TextureUnit(const TextureUnit &other)
        {
            memcpy(this,&other,sizeof(TextureUnit));
        }

        TextureUnit &operator=(const TextureUnit &other)
        {
            if (&other!=this)
                memcpy(this,&other,sizeof(TextureUnit));
            return *this;
        }
    };

    template <typename T> bool SamplerParameter( RegalContext * ctx, GLuint sampler, GLenum pname, T param )
    {
        if ( pname == GL_TEXTURE_BORDER_COLOR)
            return false;
        return SamplerParameterv(ctx, sampler, pname, &param);
    }

    void TexParameterV( RegalContext * ctx, GLenum target, GLenum pname, const GLint * params )
    {
        ctx->dispatcher.emulation.glTexParameteriv(target, pname, params);
    }

    void TexParameterV( RegalContext * ctx, GLenum target, GLenum pname, const GLuint * params )
    {
        ctx->dispatcher.emulation.glTexParameterIuiv(target, pname, params);
    }

    void TexParameterV( RegalContext * ctx, GLenum target, GLenum pname, const GLfloat * params )
    {
        ctx->dispatcher.emulation.glTexParameterfv(target, pname, params);
    }

    template <typename T> bool SamplerParameterv( RegalContext * ctx, GLuint sampler, GLenum pname, T * params )
    {
        if (!sampler || samplerObjects.count(sampler) < 1)
            return false;

        SamplerObject *so = samplerObjects[sampler];

        switch (pname)
        {
            case GL_TEXTURE_BORDER_COLOR:
                so->BorderColor[0] = static_cast<GLint>(params[0]);
                so->BorderColor[1] = static_cast<GLint>(params[1]);
                so->BorderColor[2] = static_cast<GLint>(params[2]);
                so->BorderColor[3] = static_cast<GLint>(params[3]);
                break;

            case GL_TEXTURE_COMPARE_FUNC:
                so->CompareFunc = static_cast<GLenum>(*params);
                break;

            case GL_TEXTURE_COMPARE_MODE:
                so->CompareMode = static_cast<GLenum>(*params);
                break;

            case GL_TEXTURE_LOD_BIAS:
                so->LodBias = static_cast<GLfloat>(*params);
                break;

            case GL_TEXTURE_MAX_ANISOTROPY_EXT:
                so->MaxAnisotropyExt = static_cast<GLfloat>(*params);
                break;

            case GL_TEXTURE_MAG_FILTER:
                so->MagFilter = static_cast<GLenum>(*params);
                break;

            case GL_TEXTURE_MAX_LOD:
                so->MaxLod = static_cast<GLfloat>(*params);
                break;

            case GL_TEXTURE_MIN_FILTER:
                so->MinFilter = static_cast<GLenum>(*params);
                break;

            case GL_TEXTURE_MIN_LOD:
                so->MinLod = static_cast<GLfloat>(*params);
                break;

            case GL_TEXTURE_WRAP_R:
                so->WrapR = static_cast<GLenum>(*params);
                break;

            case GL_TEXTURE_WRAP_S:
                so->WrapS = static_cast<GLenum>(*params);
                break;

            case GL_TEXTURE_WRAP_T:
                so->WrapT = static_cast<GLenum>(*params);
                break;

            case GL_TEXTURE_SRGB_DECODE_EXT:
                so->SrgbDecodeExt = static_cast<GLenum>(*params);
                break;

            default:
                Warning( "Unhandled texture parameter enum: pname = ", Token::GLenumToString(pname));
                return false;
        }

        GLuint originalActiveUnit = activeTextureUnit;

        for (GLuint unit=0; unit < REGAL_EMU_MAX_TEXTURE_UNITS; unit++)
        {
            TextureUnit &tu = textureUnits[unit];

            if (tu.boundSamplerObject == sampler)
            {
                for (GLuint tti=0; tti < REGAL_NUM_TEXTURE_TARGETS; tti++)
                {
                    GLuint to = tu.boundTextureObjects[tti];
                    if (to)
                    {
                        if (unit != activeTextureUnit)
                            ctx->dispatcher.emulation.glActiveTexture( GL_TEXTURE0 + unit );

                        TexParameterV( ctx, TT_Index2Enum(tti), pname, params);
                    }
                }
            }
        }

        if (activeTextureUnit != originalActiveUnit)
            ctx->dispatcher.emulation.glActiveTexture( GL_TEXTURE0 + originalActiveUnit );

        return true;
    }

    template <typename T> bool GetSamplerParameterv( GLuint sampler, GLenum pname, T * params )
    {
        if (!sampler || samplerObjects.count(sampler) < 1)
            return false;

        SamplerObject *so = samplerObjects[sampler];

        switch (pname)
        {
            case GL_TEXTURE_BORDER_COLOR:
                params[0] = static_cast<T>(so->BorderColor[0]);
                params[1] = static_cast<T>(so->BorderColor[1]);
                params[2] = static_cast<T>(so->BorderColor[2]);
                params[3] = static_cast<T>(so->BorderColor[3]);
                break;

            case GL_TEXTURE_COMPARE_FUNC:
                *params = static_cast<T>(so->CompareFunc);
                break;

            case GL_TEXTURE_COMPARE_MODE:
                *params = static_cast<T>(so->CompareMode);
                break;

            case GL_TEXTURE_LOD_BIAS:
                *params = static_cast<T>(so->LodBias);
                break;

            case GL_TEXTURE_MAX_ANISOTROPY_EXT:
                *params = static_cast<T>(so->MaxAnisotropyExt);
                break;

            case GL_TEXTURE_MAG_FILTER:
                *params = static_cast<T>(so->MagFilter);
                break;

            case GL_TEXTURE_MAX_LOD:
                *params = static_cast<T>(so->MaxLod);
                break;

            case GL_TEXTURE_MIN_FILTER:
                *params = static_cast<T>(so->MinFilter);
                break;

            case GL_TEXTURE_MIN_LOD:
                *params = static_cast<T>(so->MinLod);
                break;

            case GL_TEXTURE_WRAP_R:
                *params = static_cast<T>(so->WrapR);
                break;

            case GL_TEXTURE_WRAP_S:
                *params = static_cast<T>(so->WrapS);
                break;

            case GL_TEXTURE_WRAP_T:
                *params = static_cast<T>(so->WrapT);
                break;

            case GL_TEXTURE_SRGB_DECODE_EXT:
                *params = static_cast<T>(so->SrgbDecodeExt);
                break;

            default:
                Warning( "Unhandled texture parameter enum: pname = ", Token::GLenumToString(pname));
                return false;
        }
        return true;
    }

    template <typename T> void TexParameter( GLenum target, GLenum pname, T param )
    {
        if ( target != GL_TEXTURE_BORDER_COLOR)
            TexParameterv(target, pname, &param);
    }

    template <typename T> void TexParameterv( GLenum target, GLenum pname, T * params )
    {
        GLuint tti = TT_Enum2Index(target);

        if (tti == ~(GLuint(0)))
            return;

        TextureUnit &tu = textureUnits[activeTextureUnit];

        GLuint t = tu.boundTextureObjects[tti];

        if (t)
        {
            TextureObject *to = textureObjects[t];

            if (!to)
                return;

            switch (pname)
            {
                case GL_TEXTURE_BORDER_COLOR:
                    to->self.BorderColor[0] = (GLint)(params[0]);
                    to->self.BorderColor[1] = (GLint)(params[1]);
                    to->self.BorderColor[2] = (GLint)(params[2]);
                    to->self.BorderColor[3] = (GLint)(params[3]);
                    break;

                case GL_TEXTURE_MIN_FILTER:
                    to->self.MinFilter = (GLint)(params[0]);
                    break;

                case GL_TEXTURE_MAG_FILTER:
                    to->self.MagFilter = (GLint)(params[0]);
                    break;

                case GL_TEXTURE_WRAP_S:
                    to->self.WrapS = (GLint)(params[0]);
                    break;

                case GL_TEXTURE_WRAP_T:
                    to->self.WrapT = (GLint)(params[0]);
                    break;

                case GL_TEXTURE_WRAP_R:
                    to->self.WrapR = (GLint)(params[0]);
                    break;

                case GL_TEXTURE_COMPARE_MODE:
                    to->self.CompareMode = (GLint)(params[0]);
                    break;

                case GL_TEXTURE_COMPARE_FUNC:
                    to->self.CompareFunc = (GLint)(params[0]);
                    break;

                case GL_TEXTURE_MIN_LOD:
                    to->self.MinLod = (GLfloat)(params[0]);
                    break;

                case GL_TEXTURE_MAX_LOD:
                    to->self.MaxLod = (GLfloat)(params[0]);
                    break;

                case GL_TEXTURE_LOD_BIAS:
                    to->self.LodBias = (GLfloat)(params[0]);
                    break;

                case GL_TEXTURE_MAX_ANISOTROPY_EXT:
                    to->self.MaxAnisotropyExt = (GLfloat)(params[0]);
                    break;

                case GL_TEXTURE_SRGB_DECODE_EXT:
                    to->self.SrgbDecodeExt = (GLenum)(params[0]);
                    break;

                case GL_DEPTH_STENCIL_TEXTURE_MODE:
                case GL_TEXTURE_BASE_LEVEL:
                case GL_TEXTURE_MAX_LEVEL:
                case GL_TEXTURE_SWIZZLE_A:
                case GL_TEXTURE_SWIZZLE_B:
                case GL_TEXTURE_SWIZZLE_G:
                case GL_TEXTURE_SWIZZLE_R:
                    // Known texture object state that is not in a sampler object.
                    // Everything is cool, just return silently
                    return;

                default:
                    // Otherwise an unrecognezed enum.  Issue a warning...
                    Warning( "Unhandled texture parameter enum: pname = ", Token::GLenumToString(pname));
                    return;
            }
        }
    }

    void ActiveTexture( GLenum tex )
    {
        GLuint unit = tex - GL_TEXTURE0;
        if (unit < REGAL_EMU_MAX_TEXTURE_UNITS)
            activeTextureUnit = unit;
        else
            Warning( "Active texture out of range: ", tex, " >= ",
                Token::GLenumToString(GL_TEXTURE0 + REGAL_EMU_MAX_TEXTURE_UNITS));
    }

    void GenTextures(RegalContext * ctx, GLsizei count, GLuint *textures);
    void DeleteTextures(RegalContext * ctx, GLsizei count, const GLuint * textures);
    void BindTexture(RegalContext * ctx, GLuint unit, GLenum target, GLuint texture);
    void BindTexture(RegalContext * ctx, GLenum target, GLuint texture);

    void GenSamplers(GLsizei count, GLuint *samplers);
    void DeleteSamplers(RegalContext * ctx, GLsizei count, const GLuint * samplers);
    GLboolean IsSampler(GLuint sampler);
    void BindSampler(RegalContext * ctx, GLuint unit, GLuint sampler);

    GLuint activeTextureUnit;
    GLuint nextSamplerObjectId;
    TextureUnit textureUnits[ REGAL_EMU_MAX_TEXTURE_UNITS ];
    SamplerObject defaultTextureObjects[ REGAL_NUM_TEXTURE_TARGETS ];
    std::map<GLuint, SamplerObject*> samplerObjects;
    std::map<GLuint, TextureObject*> textureObjects;
};

REGAL_NAMESPACE_END

#endif // REGAL_EMULATION

#endif // __REGAL_FIXED_FUNCTION_H__
