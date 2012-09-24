#!/usr/bin/python -B

#
# Filter calls that don't exist for ES 2.0
#
# This is the last layer of the emulation dispatch.
#

formulae = {
    'glClientActiveTexture' : {
        'entries' : [ 'glClientActiveTexture' ],
        'impl' : [
         '#if REGAL_FORCE_ES2_PROFILE',
         'return;',
         '#else',
         'if (_context->info->gles)',
         '  return;',
         '#endif' ]
    }
}
