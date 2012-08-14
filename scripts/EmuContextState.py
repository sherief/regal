#!/usr/bin/python -B

# Basic tracking at RegalContext level for global
# things such as glBegin/glEnd, glPush/glPop depth, etc

formulae = {
  'Begin' : {
    'entries' : [ 'glBegin' ],
    'impl' : [ 'RegalAssert(rCtx);',
               'rCtx->depthBeginEnd++;'
             ]
  },

  'End' : {
    'entries' : [ 'glEnd' ],
    'impl' : [ 'RegalAssert(rCtx);',
               'rCtx->depthBeginEnd--;'
             ]
  },

  'PushAttrib' : {
    'entries' : [ 'glPushAttrib' ],
    'impl' : [ 'RegalAssert(rCtx);',
               'rCtx->depthPushAttrib++;'
             ]
  },

  'PopAttrib' : {
    'entries' : [ 'glPopAttrib' ],
    'impl' : [ 'RegalAssert(rCtx);',
               'rCtx->depthPushAttrib--;'
             ]
  },

}
