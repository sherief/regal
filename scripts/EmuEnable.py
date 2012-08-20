#!/usr/bin/python -B

# GL_REGAL_enable implementation

formulae = {
    'Enable' : {
        'entries' : [ 'glEnable' ],
        'impl' : [
'switch(${arg0})',
'{',
'  case GL_ERROR_REGAL:      /* TODO */ return;',
'  case GL_DEBUG_REGAL:      /* TODO */ return;',
'  case GL_LOG_REGAL:        /* TODO */ return;',
'  case GL_EMULATION_REGAL:  /* TODO */ return;',
'  case GL_LOADER_REGAL:     /* TODO */ return;',
'  default: break;',
'}', ],
    },
    'Disable' : {
        'entries' : [ 'glDisable' ],
        'impl' : [
'switch(${arg0})',
'{',
'  case GL_ERROR_REGAL:      /* TODO */ return;',
'  case GL_DEBUG_REGAL:      /* TODO */ return;',
'  case GL_LOG_REGAL:        /* TODO */ return;',
'  case GL_EMULATION_REGAL:  /* TODO */ return;',
'  case GL_LOADER_REGAL:     /* TODO */ return;',
'  default: break;',
'}', ],
    },
    'IsEnabled' : {
        'entries' : [ 'glIsEnabled' ],
        'impl' : [
'switch(${arg0})',
'{',
'  case GL_ERROR_REGAL:      /* TODO */ return GL_TRUE;',
'  case GL_DEBUG_REGAL:      /* TODO */ return GL_TRUE;',
'  case GL_LOG_REGAL:        /* TODO */ return GL_TRUE;',
'  case GL_EMULATION_REGAL:  /* TODO */ return GL_TRUE;',
'  case GL_LOADER_REGAL:     /* TODO */ return GL_TRUE;',
'  default: break;',
'}', ],
    },
}
