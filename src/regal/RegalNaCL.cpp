/*
  Copyright (c) 2012 John McCutchan
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
  OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <GL/Regal.h>

#include "RegalNaCL.h"

static RegalNaCLPrintfCallback _printfCallback = NULL;
static int32_t _naclGLES2Context = 0;
static PPB_OpenGLES2* _naclGLES2Interface = NULL;

void _naclPrintf(const char* str, ...);
void* _naclGetProcAddress(const char* lookupName);

struct naclProcEntry {
  const char* name;
  void* address;
};

#ifdef REGAL_SYS_NACL

// NACL implementation
#include "ppapi/c/ppb_opengles2.h"

void RegalNaCLRegisterPrintfCallback(RegalNaCLPrintfCallback callback) {
  _printfCallback = callback;
}

void RegalNaCLRegisterInterface(PPB_OpenGLES2* interface) {
  _naclGLES2Interface = interface;
}

void RegalNaCLRegisterContext(int32_t context) {
  _naclGLES2Context = context;
}

RegalNaCLPrintfCallback RegalNaCLGetPrintfCallback() {
	return _printfCallback;
}

PPB_OpenGLES2* RegalNaCLGetInterface() {
	return _naclGLES2Interface;
}

int32_t RegalNaCLGetContext() {
	return _naclGLES2Context;
}

static void naclGlGetIntegerv(GLenum pname, GLint* params) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->GetIntegerv(_naclGLES2Context, pname, params);
}

static void naclGlGenBuffers(GLsizei n, GLuint* buffers) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->GenBuffers(_naclGLES2Context, n, buffers);
}

static void naclGlBindBuffer(GLenum target, GLuint buffer) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->BindBuffer(_naclGLES2Context, target, buffer);
}

static void naclGlBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->BufferData(_naclGLES2Context, target, size, data, usage);
}

static void naclGlClearDepthf(GLclampf depth) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->ClearDepthf(_naclGLES2Context, depth);
}

static void naclGlFlush() {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->Flush(_naclGLES2Context);
}

static void naclGlFinish() {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->Finish(_naclGLES2Context);
}

static void naclGlClear(GLbitfield mask) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->Clear(_naclGLES2Context, mask);
}

static void naclGlClearColor(float r, float g, float b, float a) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->ClearColor(_naclGLES2Context, r, g, b, a);
}

static const GLubyte* naclGlGetString(GLenum name) {
  if (_naclGLES2Interface == NULL) {
    return NULL;
  }
  const GLubyte* r = _naclGLES2Interface->GetString(_naclGLES2Context, name);
  return r;
}

static void naclGlEnableVertexAttribArray(GLuint index) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->EnableVertexAttribArray(_naclGLES2Context, index);
}

static void naclGlDisableVertexAttribArray(GLuint index) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->DisableVertexAttribArray(_naclGLES2Context, index);
}

static void naclGlVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->VertexAttribPointer(_naclGLES2Context, indx, size, type, normalized, stride, ptr);
}

static GLuint naclGlCreateProgram() {
  if (_naclGLES2Interface == NULL) {
    return 0;
  }
  return _naclGLES2Interface->CreateProgram(_naclGLES2Context);
}

static void naclGlDeleteProgram(GLuint program) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->DeleteProgram(_naclGLES2Context, program);
}

static void naclGlLinkProgram(GLuint program) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->LinkProgram(_naclGLES2Context, program);
}

static void naclGlUseProgram(GLuint program) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->UseProgram(_naclGLES2Context, program);
}

static GLuint naclGlCreateShader(GLenum type) {
  if (_naclGLES2Interface == NULL) {
    return 0;
  }
  return _naclGLES2Interface->CreateShader(_naclGLES2Context, type);
}

static void naclGlDeleteShader(GLuint shader) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->DeleteShader(_naclGLES2Context, shader);
}

static void naclGlShaderSource(GLuint shader, GLsizei count, const char** str, const GLint* length) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  for (int i = 0; i < count; i++) {
    //_naclPrintf("[%d] - %s\n", count, str[i]);
  }
  _naclGLES2Interface->ShaderSource(_naclGLES2Context, shader, count, str, length);
}

static void naclGlCompileShader(GLuint shader) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->CompileShader(_naclGLES2Context, shader);
}

static void naclGlGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->GetShaderInfoLog(_naclGLES2Context, shader, bufsize, length, infolog);
}

static void naclGlGetProgramInfoLog( GLuint program, GLsizei bufsize, GLsizei* length, char* infolog) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->GetProgramInfoLog(_naclGLES2Context, program, bufsize, length, infolog);
}

static void naclGlAttachShader(GLuint program, GLuint shader) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->AttachShader(_naclGLES2Context, program, shader);
}

static void naclGlDetachShader(GLuint program, GLuint shader) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->DetachShader(_naclGLES2Context, program, shader);
}

static void naclGlBindAttribLocation(GLuint program, GLuint index, const char* name) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  //_naclPrintf("Binding %s to %d\n", name, index);
  _naclGLES2Interface->BindAttribLocation(_naclGLES2Context, program, index, name);
}

static GLint naclGlGetUniformLocation(GLuint program, const char* name) {
  if (_naclGLES2Interface == NULL) {
    return -1;
  }
  return _naclGLES2Interface->GetUniformLocation(_naclGLES2Context, program, name);
}

static void naclGlDrawArrays(GLenum mode, GLint first, GLsizei count) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->DrawArrays(_naclGLES2Context, mode, first, count);
}

static void naclGlDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->DrawElements(_naclGLES2Context, mode, count, type, indices);
}

static void naclGlViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->Viewport(_naclGLES2Context, x, y, width, height);
}

static void naclGlUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->UniformMatrix4fv(_naclGLES2Context, location, count, transpose, value);
}

static void naclGlUniform4fv(GLint location, GLsizei count, const GLfloat* v) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->Uniform4fv(_naclGLES2Context, location, count, v);
}

static void naclGlDepthFunc(GLenum func) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->DepthFunc(_naclGLES2Context, func);
}

static void naclGlEnable(GLenum cap) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->Enable(_naclGLES2Context, cap);
}

static void naclGlDisable(GLenum cap) {
  if (_naclGLES2Interface == NULL) {
    return;
  }
  _naclGLES2Interface->Disable(_naclGLES2Context, cap);
}

naclProcEntry _nacl_lookup[] = {
  { "glFinish", (void*)naclGlFinish },
  { "glFlush", (void*)naclGlFlush },
  { "glGetString", (void*)naclGlGetString },
  { "glClear", (void*)naclGlClear },
  { "glClearColor", (void*)naclGlClearColor },
  { "glGetIntegerv", (void*)naclGlGetIntegerv },
  { "glBindBuffer", (void*)naclGlBindBuffer },
  { "glBufferData", (void*)naclGlBufferData },
  { "glClearDepthf", (void*)naclGlClearDepthf },
  { "glGenBuffers", (void*)naclGlGenBuffers },
  { "glDisableVertexAttribArray", (void*)naclGlDisableVertexAttribArray },
  { "glEnableVertexAttribArray", (void*)naclGlEnableVertexAttribArray },
  { "glVertexAttribPointer", (void*)naclGlVertexAttribPointer },
  { "glCreateProgram", (void*)naclGlCreateProgram },
  { "glDeleteProgram", (void*)naclGlDeleteProgram },
  { "glCreateShader", (void*)naclGlCreateShader },
  { "glDeleteShader", (void*)naclGlDeleteShader },
  { "glShaderSource", (void*)naclGlShaderSource },
  { "glCompileShader", (void*)naclGlCompileShader },
  { "glGetShaderInfoLog", (void*)naclGlGetShaderInfoLog},
  { "glGetProgramInfoLog", (void*)naclGlGetProgramInfoLog },
  { "glAttachShader", (void*)naclGlAttachShader},
  { "glDetachShader", (void*)naclGlDetachShader},
  { "glUseProgram", (void*)naclGlUseProgram},
  { "glLinkProgram", (void*)naclGlLinkProgram },
  { "glBindAttribLocation", (void*)naclGlBindAttribLocation},
  { "glGetUniformLocation", (void*)naclGlGetUniformLocation },
  { "glDrawArrays", (void*)naclGlDrawArrays},
  { "glDrawElements", (void*)naclGlDrawElements},
  { "glViewport", (void*)naclGlViewport},
  { "glUniformMatrix4fv", (void*)naclGlUniformMatrix4fv },
  { "glUniform4fv", (void*)naclGlUniform4fv},
  { "glDepthFunc", (void*)naclGlDepthFunc},
  { "glEnable", (void*)naclGlEnable},
  { "glDisable", (void*)naclGlDisable},
  { NULL, NULL }  
};

#else // !REGAL_SYS_NACL

void RegalNaCLRegisterPrintfCallback(RegalNaCLPrintfCallback callback) {
	// NOP
}

void RegalNaCLRegisterInterface(PPB_OpenGLES2* interface) {
	// NOP
}

void RegalNaCLRegisterContext(int32_t context) {
	// NOP
}

RegalNaCLPrintfCallback RegalNaCLGetPrintfCallback() {
	return NULL;
}

PPB_OpenGLES2* RegalNaCLGetInterface() {
	return NULL;
}

int32_t RegalNaCLGetContext() {
	return 0;
}

naclProcEntry _nacl_lookup[] = {
	{ NULL, NULL }
};

#endif // REGAL_SYS_NACL

// Hooks that Regal can call
void _naclPrintf(const char* str, ...) {
  const int bufferSize = 1024;
  char buff[bufferSize];
  if (_printfCallback == NULL) {
  	return;
  }
  buff[0] = '\0';
  va_list vl;
  va_start(vl, str);
  vsnprintf(&buff[0], bufferSize, str, vl);
  va_end(vl);
  _printfCallback(buff);
}

void* _naclGetProcAddress(const char* lookupName) {
  _naclPrintf("Looking for %s\n", lookupName);
  for (int i = 0; _nacl_lookup[i].name != NULL; i++) {
    const naclProcEntry* entry = &_nacl_lookup[i];
    if (!strcmp(entry->name, lookupName)) {
      return entry->address;
    }
  }
  _naclPrintf("Returning NULL for %s\n", lookupName);
  return NULL;
}
