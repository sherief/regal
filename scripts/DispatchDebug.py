#!/usr/bin/python -B

debugDispatchFormulae = {
    'MatrixMode' : {
        'entries' : [ 'glMatrixMode' ],
        'prefix' : [ '_context->dbg->MatrixMode( _context, ${arg0plus} );', ],
    },
    'ClientActiveTexture' : {
        'entries' : [ 'gl(Client|)(ActiveTexture)' ],
        'prefix' : [ '_context->dbg->${m1}${m2}( _context, ${arg0plus} );', ],
    },
}

