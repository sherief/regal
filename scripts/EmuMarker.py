#!/usr/bin/python -B

formulae = {

    # GL_EXT_debug_marker

    'Insert' : {
        'entries' : [ 'glInsertEventMarkerEXT' ],
        'impl' : [ '_context->marker->InsertEventMarker( _context, ${arg0plus} );',
                   'RegalAssert(_context->info);',
                   'if (!_context->info->gl_ext_debug_marker) return;' ]
    },
    'Push' : {
        'entries' : [ 'glPushGroupMarkerEXT' ],
        'impl' : [ '_context->marker->PushGroupMarker( _context, ${arg0plus} );',
                   'RegalAssert(_context->info);',
                   'if (!_context->info->gl_ext_debug_marker) return;' ]
    },
    'Pop' : {
        'entries' : [ 'glPopGroupMarkerEXT' ],
        'impl' : [ '_context->marker->PopGroupMarker( _context );',
                   'RegalAssert(_context->info);',
                   'if (!_context->info->gl_ext_debug_marker) return;' ]
    },

    # GL_GREMEDY_string_marker
    
    'GREMEDY' : {
        'entries' : [ 'glStringMarkerGREMEDY' ],
        'impl' : [ '_context->marker->InsertEventMarker( _context, ${arg0plus} );',
                   'RegalAssert(_context->info);',
                   'if (!_context->info->gl_gremedy_string_marker) return;' ]
    }
}
