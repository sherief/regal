#!/usr/bin/python -B

#
# Dispatch for generating C code, similar to
# application or driver logging.
#

from string import Template, upper, replace
from copy   import deepcopy

from ApiCodeGen   import *
from ApiUtil      import outputCode
from ApiUtil      import typeIsVoid
from ApiType      import typeStrip
from ApiRegal     import logParameter
from ApiRegal     import cCodeParameter
from ApiRegal     import helperMap

from RegalContextInfo import cond

# Code generation for dispatch table init.

def apiDispatchCodeInitCode(apis, args, dispatchName):

  code = ''

  for api in apis:
    if api.name=='gl':

      for function in api.functions:

        if not function.needsContext:
          continue

        if getattr(function,'regalOnly',False)==True:
          continue

        name = function.name
        code += '  tbl.%s = %s_%s;\n' % ( name, dispatchName, name )

  return code

# Template for RegalDispatchCode.cpp

dispatchCodeTemplate = Template('''${AUTOGENERATED}
${LICENSE}

#include "pch.h" /* For MS precompiled header support */

#include "RegalUtil.h"

#if REGAL_CODE

REGAL_GLOBAL_BEGIN

#include <string>

#include <boost/print/string_list.hpp>

#include "RegalLog.h"
#include "RegalPush.h"
#include "RegalToken.h"
#include "RegalHelper.h"
#include "RegalContext.h"

using namespace ::REGAL_NAMESPACE_INTERNAL::Logging;
using namespace ::REGAL_NAMESPACE_INTERNAL::Token;

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

using namespace ::boost::print;

${API_FUNC_DEFINE}

void InitDispatchTableCode(DispatchTable &tbl)
{
${API_GLOBAL_DISPATCH_INIT}
}

REGAL_NAMESPACE_END

#endif
''')

# Code generation for generating C code from GL API calls

def generateDispatchCode(apis, args):

  code = ''

  for api in apis:
    if api.name=='gl':

      for function in api.functions:

        if not function.needsContext:
          continue

        if getattr(function,'regalOnly',False)==True:
          continue

        f = deepcopy(function)

        name = f.name
        params = paramsDefaultCode(f.parameters, True)

        callParams = paramsNameCode(f.parameters)
        rType  = typeCode(f.ret.type)

        code += 'static %sREGAL_CALL %s%s(%s) \n{\n' % (rType, 'code_', name, params)
        code += '    RegalContext *_context = REGAL_GET_CONTEXT();\n'
        code += '    RegalAssert(_context);\n'

        code += '    DispatchTable *_next = _context->dispatcher.code._next;\n'
        code += '    RegalAssert(_next);\n'
        code += '    '
        if not typeIsVoid(rType):
          code += '%s _ret = '%(rType)
        code += '_next->call(&_next->%s)(%s);\n' % ( name, callParams )

        prefix = ''
        suffix = ''

        if not f.trace:
          prefix += '/* '
          suffix += ' */'

        header =  '    std::string indent((_context->depthBeginEnd + _context->depthPushAttrib)*2,\' \');\n'
        header += '    string_list< ::std::string > _code;\n'
        body   =  ''

        if len(f.parameters)==0:
          body += '    %s_code << indent << "%s();%s\\n";\n'%(prefix,f.name,suffix)
        if len(f.parameters)>0:
          body += '    %s_code << indent << "%s(";\n'%(prefix,f.name)

        if len(f.parameters)>0:
          delim = False
          for i in f.parameters:
            if delim:
              body += '    _code << ", "; '
            else:
              body += '                   '
#           p = cCodeParameter(f,i)
            p = logParameter(f,i)
            if p==None:
                body += '_code << "/* %s = ?? */";\n'%(i.name)
            elif p.startswith('boost::print::array'):
                type = typeStrip(i.type)
                size = i.size
                if i.maxSize != None:
                  size = i.maxSize
                if i.input:
                  if p.find('helper')==-1 and type!='GLchar' and type!='GLcharARB':
                    header += '    size_t _%sIndex = _context->codeInputNext++;\n'%(i.name)
                    header += '    _code << indent << \"const %s i\" << _%sIndex << \"[%s] = \" '%(type,i.name,size)
                    header += '<< array<%s,const char * const>(%s,%s,\"\",\"{ \",\" };\\n\",\", \");\n'%(type,i.name,size)
                    body += '_code << \"i\" << _%sIndex;\n'%(i.name)
                  else:
                    body += '_code << "/* %s = ?? */";\n'%(i.name)
                else:
                  if p.find('helper')==-1 and type!='GLchar' and type!='GLcharARB':
                    header += '    size_t _%sIndex = _context->codeOutputNext++;\n'%(i.name)
                    header += '    _code << indent << \"%s o\" << _%sIndex << \"[\" << (%s) << \"];\\n";\n'%(type,i.name,size)
                    body += '_code << \"o\" << _%sIndex;\n'%(i.name)
                  else:
                    body += '_code << "/* %s = ?? */";\n'%(i.name)

            # glTexImage2D etc

            elif i.size != None and (isinstance(i.size, str) or isinstance(i.size, unicode)) and i.size.startswith('helperGLPixelImageSize'):
              header += '    size_t _%sIndex = _context->codeInputNext++;\n'%(i.name)
              header += '    _code << indent << \"const GLubyte i\" << _%sIndex << \"[\" << helper::size::pixelImage(%s << \"] = \" '%(i.name,i.size.split('(',1)[1])
              header += '<< array<GLubyte,const char * const>(static_cast<const GLubyte *>(%s),helper::size::pixelImage(%s,\"\",\"{ \",\" }\",\",\") '%(i.name,i.size.split('(',1)[1])
              header += '<< \";\\n\";\n'
              body += '_code << \"i\" << _%sIndex;\n'%(i.name)

            elif p.startswith('boost::print::optional'):
              if i.cast != None:
                body += '_code << reinterpret_cast<%s>(%s);\n'%(i.cast,i.name)
              else:
                body += '_code << %s;\n'%(i.name)
            else:
                body += '_code << %s;\n'%(p)
            delim = True

          body += '    _code << ");%s\\n";\n'%(suffix)

        body += '    printf("%s",_code.str().c_str());\n'

        code += header + body

        if not typeIsVoid(rType):
          code += '    return _ret;\n'
        code += '}\n\n'

    code += '\n'

  funcInit   = apiDispatchCodeInitCode( apis, args, 'code' )

  # Output

  substitute = {}
  substitute['LICENSE']         = args.license
  substitute['AUTOGENERATED']   = args.generated
  substitute['COPYRIGHT']       = args.copyright
  substitute['API_FUNC_DEFINE'] = code
  substitute['API_GLOBAL_DISPATCH_INIT'] = funcInit

  outputCode( '%s/RegalDispatchCode.cpp' % args.srcdir, dispatchCodeTemplate.substitute(substitute))
