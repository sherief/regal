#!/usr/bin/python -B

# Basic tracking at RegalContext level for global
# things such as glBegin/glEnd, glPush/glPop depth, etc

formulae = {
  'Begin' : {
    'entries' : [ 'glBegin' ],
    'impl' : [ 'RegalAssert(_context);',
               '_context->depthBeginEnd++;'
             ]
  },

  'End' : {
    'entries' : [ 'glEnd' ],
    'impl' : [ 'RegalAssert(_context);',
               '_context->depthBeginEnd--;'
             ]
  },

  'PushAttrib' : {
    'entries' : [ 'glPushAttrib' ],
    'impl' : [ 'RegalAssert(_context);',
               '_context->depthPushAttrib++;'
             ]
  },

  'PopAttrib' : {
    'entries' : [ 'glPopAttrib' ],
    'impl' : [ 'RegalAssert(_context);',
               '_context->depthPushAttrib--;'
             ]
  },

}
