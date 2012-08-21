#!/usr/bin/python -B

binFormulae = {
    'ShaderBinary' : {
        'entries' : [ 'glShaderBinary(ARB|)' ],
        'impl' : [ '_context->bin->ShaderBinary( _context, ${arg0plus} );', ],
    }
}
