#!/usr/bin/python -B

ppaFormulae = {
  'PushAtrrib' : {
    'entries'  : [ 'glPushAttrib' ],
    'impl'     : [ '_context->ppa->PushAttrib( _context, ${arg0} );', ],
  },
  'PopAttrib' : {
    'entries' : [ 'glPopAttrib' ],
    'impl'    : [ '_context->ppa->PopAttrib( _context );', ],
  },
  'Enable'    : {
    'entries' : [ 'gl(Enable|Disable)' ],
    'prefix'  : [ '_context->ppa->${m1}( ${arg0plus} );', ],
  },
  'TrackDepth' : {
    'entries'    : [ '(glClearDepth|glDepthFunc|glDepthMask)' ],
    'prefix'     : [ '_context->ppa->${m1}( ${arg0plus} );', ],
  },
  'TrackStencil' : {
    'entries'    : [ '(glClearStencil|glStencilFunc|glStencilFuncSeparate|glStencilMask|glStencilMaskSeparate|glStencilOp|glStencilOpSeparate)' ],
    'prefix'     : [ '_context->ppa->${m1}( ${arg0plus} );', ],
  },
  'TrackPolygon' : {
    'entries'    : [ '(glCullFace|glFrontFace|glPolygonMode|glPolygonOffset)' ],
    'prefix'     : [ '_context->ppa->${m1}( ${arg0plus} );', ],
  },
}
