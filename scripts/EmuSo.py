#!/usr/bin/python -B

soFormulae = {

    # TODO

    'GenSamplers' : {
        'entries' : [ 'glGenSamplers' ],
        'impl' : [ '_context->so->GenSamplers( ${arg0}, ${arg1} );' ],
    },
    'DeleteSamplers' : {
        'entries' : [ 'glDeleteSamplers' ],
        'impl' : [ '_context->so->DeleteSamplers( _context, ${arg0}, ${arg1} );' ],
    },
    'IsSampler' : {
        'entries' : [ 'glIsSampler' ],
        'impl' : [ 'return _context->so->IsSampler( ${arg0} );' ],
    },
    'BindSampler' : {
        'entries' : [ 'glBindSampler' ],
        'impl' : [ 'return _context->so->BindSampler( _context,  ${arg0}, ${arg1} );' ],
    },
    'GetSamplerParameterv' : {
        'entries' : [ 'glGetSamplerParameter(I|)(u|)(f|i)v' ],
        'impl' : [
            'if ( !_context->so->GetSamplerParameterv( ${arg0plus} ) ) {',
            '   _context->dispatcher.emulation.glGetSamplerParameter${m1}${m2}${m3}v( ${arg0plus} );',
            '}',
        ]
    },
    'SamplerParameter' : {
        'entries' : [ 'glSamplerParameter(I|)(u|)(f|i)(v|)' ],
        'impl' : [
            'if ( !_context->so->SamplerParameter${m4}( _context, ${arg0plus} ) ) {',
            '   _context->dispatcher.emulation.glSamplerParameter${m1}${m2}${m3}${m4}( ${arg0plus} );',
            '}',
        ]
    },
    'ActiveTexture' : {
        'entries' : [ 'glActiveTexture' ],
        'prefix' : [ '_context->so->ActiveTexture( ${arg0} );' ],
    },
    'GenTextures' : {
        'entries' : [ 'glGenTextures' ],
        'impl' : [ '_context->so->GenTextures( _context, ${arg0plus} );' ],
    },
    'DeleteTextures' : {
        'entries' : [ 'glDeleteTextures' ],
        'prefix' : [ '_context->so->DeleteTextures( _context, ${arg0plus} );' ],
    },
    'BindTexture' : {
        'entries' : [ 'glBindTexture' ],
        'impl' : [ 'return _context->so->BindTexture( _context, ${arg0plus} );' ],
    },
    'TexParameter' : {
        'entries' : [ 'glTexParameter(I|)(u|)(f|i)(v|)(EXT|)' ],
        'prefix' : [ '_context->so->TexParameter${m4}( ${arg0plus} );' ],
    },
}
