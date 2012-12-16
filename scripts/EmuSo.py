#!/usr/bin/python -B

soFormulae = {

    # TODO

    'GenSamplers' : {
        'entries' : [ 'glGenSamplers' ],
        'impl' : [ '_context->so->GenSamplers( ${arg0plus} );' ],
    },
    'DeleteSamplers' : {
        'entries' : [ 'glDeleteSamplers' ],
        'impl' : [ '_context->so->DeleteSamplers( ${arg0plus} );' ],
    },
    'IsSampler' : {
        'entries' : [ 'glIsSampler' ],
        'impl' : [ 'return _context->so->IsSampler( ${arg0} );' ],
    },
    'BindSampler' : {
        'entries' : [ 'glBindSampler' ],
        'impl' : [ '_context->so->BindSampler( ${arg0plus} );' ],
    },
    'GetSamplerParameterv' : {
        'entries' : [ 'glGetSamplerParameter(I|)(u|)(f|i)v' ],
        'impl' : [
            'if ( !_context->so->GetSamplerParameterv( _context, ${arg0plus} ) ) {',
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
        'impl' : [
            'if ( !_context->so->ActiveTexture( _context, ${arg0plus} ) ) {',
            '   _context->dispatcher.emulation.glActiveTexture( ${arg0plus} );',
            '}',
        ]
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
        'impl' : [
            'if ( !_context->so->BindTexture( _context, ${arg0plus} ) ) {',
            '   _context->dispatcher.emulation.glBindTexture( ${arg0plus} );',
            '}',
        ]
    },
    'TexParameter' : {
        'entries' : [ 'glTexParameter(I|)(u|)(f|i)(v|)(EXT|)' ],
        'prefix' : [ '_context->so->TexParameter${m4}( _context, ${arg0plus} );', ]
    },
    'GetTexParameterv' : {
        'entries' : [ 'glGetTexParameter(I|)(u|)(f|i)v' ],
        'impl' : [
            'if ( !_context->so->GetTexParameterv( _context, ${arg0plus} ) ) {',
            '   _context->dispatcher.emulation.glGetTexParameter${m1}${m2}${m3}v( ${arg0plus} );',
            '}',
        ]
    },
    'PreDraw' : {
        'entries' : [ 'gl(Multi|)Draw(Arrays|Element|Elements)(Instanced|Indirect|BaseVertex|InstancedBaseVertex|Array|)(ARB|EXT|AMD|ATI|APPLE|)' ],
        'prefix' : [ '_context->so->PreDraw( _context );', ],
    },
}
