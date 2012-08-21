#!/usr/bin/python -B

objFormulae = {
    'Buffers' : {
        'entries' : [ 'gl(GenBuffers|DeleteBuffers|BindBuffer)(ARB|)' ],
        'impl' : [ '_context->obj->${m1}( _context, ${arg0plus} );', ],
    },
    'VAOs' : {
        'entries' : [ 'gl(GenVertexArrays|DeleteVertexArrays|BindVertexArray|IsVertexArray)(ARB|)' ],
        'impl' : [ '_context->obj->${m1}( _context, ${arg0plus} );', ],
    },
    'IsObj' : {
        'entries' : [ 'glIs(Buffer|VertexArray)(ARB|)' ],
        'impl' : [ 'return _context->obj->Is${m1}( _context, ${arg0plus} );', ],
    },
}
