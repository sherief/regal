#include <cstdio>
#include <cstring>
#include <cstdarg>
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppb_view.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/ppb_graphics_3d.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_opengles2.h"
//#include "ppapi/gles2/gl2ext_ppapi.h"
//#include "GLES2/gl2.h"

#define REGAL_NACL_HACK 1
#include <GL/Regal.h>
#undef REGAL_NAL_HACK



static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;
static PPB_View* ppb_view_interface = NULL;
static PPB_Graphics3D* ppb_graphics3d_interface = NULL;
static PPB_Instance* ppb_instance_interface = NULL;
static PPB_OpenGLES2* ppb_opengl_interface = NULL;
static PP_Resource opengl_context = 0;

struct ViewPort {
  int width;
  int height;
};

PP_Instance printfInstance = 0;

static ViewPort vp = {512, 512};
static float clearColor = 0.1;

/**
 * Creates new string PP_Var from C string. The resulting object will be a
 * refcounted string object. It will be AddRef()ed for the caller. When the
 * caller is done with it, it should be Release()d.
 * @param[in] str C string to be converted to PP_Var
 * @return PP_Var containing string.
 */
static struct PP_Var CStrToVar(const char* str) {
  if (ppb_var_interface != NULL) {
    return ppb_var_interface->VarFromUtf8(str, strlen(str));
  }
  return PP_MakeUndefined();
}

void _naclPrintf(const char* str, ...) {
  char buff[512];
  buff[0] = '\0';
  va_list vl;
  va_start(vl, str);
  vsnprintf(&buff[0], 511, str, vl);
  va_end(vl);
  if (ppb_messaging_interface != NULL && printfInstance != 0) {
    ppb_messaging_interface->PostMessage(printfInstance, ::CStrToVar(buff));  
  }
  
}

struct naclProcEntry {
  const char* name;
  void* address;
};

static void naclGlGetIntegerv(GLenum pname, GLint* params) {
  ppb_opengl_interface->GetIntegerv(opengl_context, pname, params);
}

static void naclGlGenBuffers(GLsizei n, GLuint* buffers) {
  ppb_opengl_interface->GenBuffers(opengl_context, n, buffers);
}

static void naclGlBindBuffer(GLenum target, GLuint buffer) {
  if (target == GL_ELEMENT_ARRAY_BUFFER) {
    _naclPrintf("Binding ELEMENTS to %x\n", buffer);
  } else {
    _naclPrintf("Binding VERTEX to %x\n", buffer);
  }
  
  ppb_opengl_interface->BindBuffer(opengl_context, target, buffer);
}

/*
Drawing (4) 0...3
Uploading 768 bytes to 34962
*/
static void naclGlBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage) {
  if (target == GL_ARRAY_BUFFER) {
    float* fp = (float*)data;
    int count = size/(sizeof(float));
    //_naclPrintf("Dumping vertex data:\n");
    for (int i = 0; i < count; i++) {
      //_naclPrintf("%d - %f\n", i, fp[i]);
    }
  }
  //_naclPrintf("Uploading %d bytes to %d\n", size, target);
  ppb_opengl_interface->BufferData(opengl_context, target, size, data, usage);
}

static void naclGlClearDepthf(GLclampf depth) {
  ppb_opengl_interface->ClearDepthf(opengl_context, depth);
}

static void naclGlFlush() {
  ppb_opengl_interface->Flush(opengl_context);
}

static void naclGlFinish() {
  ppb_opengl_interface->Finish(opengl_context);
}

static void naclGlClear(GLbitfield mask) {
  ppb_opengl_interface->Clear(opengl_context, mask);
}

static void naclGlClearColor(float r, float g, float b, float a) {
  ppb_opengl_interface->ClearColor(opengl_context, r, g, b, a);
}

static const GLubyte* naclGlGetString(GLenum name) {
  const GLubyte* r = ppb_opengl_interface->GetString(opengl_context, name);
  _naclPrintf("%d -> %s\n", name, r);
  return r;
}

static void naclGlEnableVertexAttribArray(GLuint index) {
  _naclPrintf("Enabling array %d\n", index);
  ppb_opengl_interface->EnableVertexAttribArray(opengl_context, index);
}

static void naclGlDisableVertexAttribArray(GLuint index) {
  _naclPrintf("Disabling array %d\n", index);
  ppb_opengl_interface->DisableVertexAttribArray(opengl_context, index);
}

static void naclGlVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr) {
  const char* typeString = "Unknown";
  if (type == GL_FLOAT) {
    typeString = "FLOAT";
  }
  _naclPrintf("VP: %d SIZE: %d x %s STRIDE: %d (offset: %p)", indx, size, typeString, stride, ptr);
  ppb_opengl_interface->VertexAttribPointer(opengl_context, indx, size, type, normalized, stride, ptr);
}

static GLuint naclGlCreateProgram() {
  return ppb_opengl_interface->CreateProgram(opengl_context);
}

static void naclGlDeleteProgram(GLuint program) {
  ppb_opengl_interface->DeleteProgram(opengl_context, program);
}

static void naclGlLinkProgram(GLuint program) {
  ppb_opengl_interface->LinkProgram(opengl_context, program);
}

static void naclGlUseProgram(GLuint program) {
  ppb_opengl_interface->UseProgram(opengl_context, program);
}

static GLuint naclGlCreateShader(GLenum type) {
  return ppb_opengl_interface->CreateShader(opengl_context, type);
}

static void naclGlDeleteShader(GLuint shader) {
  ppb_opengl_interface->DeleteShader(opengl_context, shader);
}

static void naclGlShaderSource(GLuint shader, GLsizei count, const char** str, const GLint* length) {
  _naclPrintf("Shader source:\n");
  for (int i = 0; i < count; i++) {
    _naclPrintf("[%d] - %s\n", i, str[i]);
  }
  ppb_opengl_interface->ShaderSource(opengl_context, shader, count, str, length);
}

static void naclGlCompileShader(GLuint shader) {
  ppb_opengl_interface->CompileShader(opengl_context, shader);
}

static void naclGlGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog) {
  ppb_opengl_interface->GetShaderInfoLog(opengl_context, shader, bufsize, length, infolog);
  //_naclPrintf("ShaderInfoLog - %s\n", infolog);
}

static void naclGlGetProgramInfoLog( GLuint program, GLsizei bufsize, GLsizei* length, char* infolog) {
  ppb_opengl_interface->GetProgramInfoLog(opengl_context, program, bufsize, length, infolog);
  //_naclPrintf("ProgramInfoLog - %s\n", infolog);
}

static void naclGlAttachShader(GLuint program, GLuint shader) {
  ppb_opengl_interface->AttachShader(opengl_context, program, shader);
}

static void naclGlDetachShader(GLuint program, GLuint shader) {
  ppb_opengl_interface->DetachShader(opengl_context, program, shader);
}

static void naclGlBindAttribLocation(GLuint program, GLuint index, const char* name) {
  _naclPrintf("Binding %s to %d\n", name, index);
  ppb_opengl_interface->BindAttribLocation(opengl_context, program, index, name);
}

static GLint naclGlGetUniformLocation(GLuint program, const char* name) {
  return ppb_opengl_interface->GetUniformLocation(opengl_context, program, name);
}

static void naclGlDrawArrays(GLenum mode, GLint first, GLsizei count) {
  //_naclPrintf("Drawing (%d) %d...%d", mode, first, first+count);
  ppb_opengl_interface->DrawArrays(opengl_context, mode, first, count);
}

static void naclGlDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
  ppb_opengl_interface->DrawElements(opengl_context, mode, count, type, indices);
}

static void naclGlViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
  ppb_opengl_interface->Viewport(opengl_context, x, y, width, height);
}

static void naclGlUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) {
  ppb_opengl_interface->UniformMatrix4fv(opengl_context, location, count, transpose, value);
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
  { NULL, NULL }
};

void* _naclGetProcAddress(const char* lookupName) {
  for (int i = 0; _nacl_lookup[i].name != NULL; i++) {
    const naclProcEntry* entry = &_nacl_lookup[i];
    if (!strcmp(entry->name, lookupName)) {
      _naclPrintf("Returning %p for %s\n", entry->address, entry->name);
      return entry->address;
    }
  }
  _naclPrintf("Returning NULL for %s\n", lookupName);
  return NULL;
}

void staticTriangle() {
  clearColor += 0.01;
  if (clearColor > 1.0) {
    clearColor = 0.0;
  }
  glClearColor(clearColor, clearColor, clearColor, 1.0);
  glClearDepthf(0.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, 512, 512);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glBegin(GL_TRIANGLES);
        glColor3f(1.0, 0.0, 0.0);
        glVertex3f(0, 0, 0);
        glColor3f(0.0, 1.0, 0.0);
        glVertex3f(1, 0, 0);
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(0, 1, 0);
  glEnd();  
}

void rotatingTriangle() {
  static float rtri = 0.0;
  clearColor += 0.01;
  if (clearColor > 1.0) {
    clearColor = 0.0;
  }
  glClearColor(clearColor, clearColor, clearColor, 1.0);
  glClearDepthf(0.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, 512, 512);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glRotatef(rtri,0.0f,1.0f,0.0f);  
  glBegin(GL_TRIANGLES);
        glColor3f(1.0, 0.0, 0.0);
        glVertex3f(0, 0, 0);
        glColor3f(0.0, 1.0, 0.0);
        glVertex3f(1, 0, 0);
        glColor3f(0.0, 0.0, 1.0);
        glVertex3f(0, 1, 0);
  glEnd();
  rtri += 0.2f;
}

static void frameCallback(void* user_data, int32_t result) {
  

  rotatingTriangle();
  PP_CompletionCallback ccb;
  ccb.func = frameCallback;
  ccb.user_data = NULL;
  ccb.flags = 0;
  ppb_graphics3d_interface->SwapBuffers(opengl_context, ccb);
}


/**
 * Called when the NaCl module is instantiated on the web page. The identifier
 * of the new instance will be passed in as the first argument (this value is
 * generated by the browser and is an opaque handle).  This is called for each
 * instantiation of the NaCl module, which is each time the <embed> tag for
 * this module is encountered.
 *
 * If this function reports a failure (by returning @a PP_FALSE), the NaCl
 * module will be deleted and DidDestroy will be called.
 * @param[in] instance The identifier of the new instance representing this
 *     NaCl module.
 * @param[in] argc The number of arguments contained in @a argn and @a argv.
 * @param[in] argn An array of argument names.  These argument names are
 *     supplied in the <embed> tag, for example:
 *       <embed id="nacl_module" dimensions="2">
 *     will produce two arguments, one named "id" and one named "dimensions".
 * @param[in] argv An array of argument values.  These are the values of the
 *     arguments listed in the <embed> tag.  In the above example, there will
 *     be two elements in this array, "nacl_module" and "2".  The indices of
 *     these values match the indices of the corresponding names in @a argn.
 * @return @a PP_TRUE on success.
 */


static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {
  printfInstance = instance;
  ppb_messaging_interface->PostMessage(instance, 
                                       CStrToVar("Hello a World (GLIBC)"));
  int32_t attribs[] = { PP_GRAPHICS3DATTRIB_WIDTH, 512, PP_GRAPHICS3DATTRIB_HEIGHT, 512, PP_GRAPHICS3DATTRIB_NONE};
  opengl_context = ppb_graphics3d_interface->Create(instance, opengl_context, attribs);
  ppb_instance_interface->BindGraphics(instance, opengl_context);
  RegalMakeCurrent(reinterpret_cast<void*>(opengl_context));
  frameCallback(NULL, 0);
  return PP_TRUE;
}

/**
 * Called when the NaCl module is destroyed. This will always be called,
 * even if DidCreate returned failure. This routine should deallocate any data
 * associated with the instance.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 */
static void Instance_DidDestroy(PP_Instance instance) {
}

/**
 * Called when the position, the size, or the clip rect of the element in the
 * browser that corresponds to this NaCl module has changed.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 * @param[in] position The location on the page of this NaCl module. This is
 *     relative to the top left corner of the viewport, which changes as the
 *     page is scrolled.
 * @param[in] clip The visible region of the NaCl module. This is relative to
 *     the top left of the plugin's coordinate system (not the page).  If the
 *     plugin is invisible, @a clip will be (0, 0, 0, 0).
 */
static void Instance_DidChangeView(PP_Instance instance,
                                   PP_Resource view_resource) {
  PP_Rect rect;
  ppb_view_interface->GetRect(view_resource, &rect);
  vp.width = rect.size.width;
  vp.height = rect.size.height;
  {
  //char blah[512];
  //snprintf(&blah[0], 512, "(%d, %d) x (%d, %d)", rect.point.x, rect.point.y, rect.point.x + rect.size.width, rect.point.y + rect.size.height);
  //ppb_messaging_interface->PostMessage(instance, CStrToVar(blah));  
  }
}

/**
 * Notification that the given NaCl module has gained or lost focus.
 * Having focus means that keyboard events will be sent to the NaCl module
 * represented by @a instance. A NaCl module's default condition is that it
 * will not have focus.
 *
 * Note: clicks on NaCl modules will give focus only if you handle the
 * click event. You signal if you handled it by returning @a true from
 * HandleInputEvent. Otherwise the browser will bubble the event and give
 * focus to the element on the page that actually did end up consuming it.
 * If you're not getting focus, check to make sure you're returning true from
 * the mouse click in HandleInputEvent.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 * @param[in] has_focus Indicates whether this NaCl module gained or lost
 *     event focus.
 */
static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

/**
 * Handler that gets called after a full-frame module is instantiated based on
 * registered MIME types.  This function is not called on NaCl modules.  This
 * function is essentially a place-holder for the required function pointer in
 * the PPP_Instance structure.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 * @param[in] url_loader A PP_Resource an open PPB_URLLoader instance.
 * @return PP_FALSE.
 */
static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  /* NaCl modules do not need to handle the document load function. */
  return PP_FALSE;
}



/**
 * Entry points for the module.
 * Initialize needed interfaces: PPB_Core, PPB_Messaging and PPB_Var.
 * @param[in] a_module_id module ID
 * @param[in] get_browser pointer to PPB_GetInterface
 * @return PP_OK on success, any other value on failure.
 */
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser) {
  ppb_messaging_interface = (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  ppb_view_interface = (PPB_View*)(get_browser(PPB_VIEW_INTERFACE));
  ppb_graphics3d_interface = (PPB_Graphics3D*)(get_browser(PPB_GRAPHICS_3D_INTERFACE));
  ppb_instance_interface = (PPB_Instance*)(get_browser(PPB_INSTANCE_INTERFACE));
  ppb_opengl_interface = (PPB_OpenGLES2*)(get_browser(PPB_OPENGLES2_INTERFACE));
  return PP_OK;
}


/**
 * Returns an interface pointer for the interface of the given name, or NULL
 * if the interface is not supported.
 * @param[in] interface_name name of the interface
 * @return pointer to the interface
 */
PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0) {
    static PPP_Instance instance_interface = {
      &Instance_DidCreate,
      &Instance_DidDestroy,
      &Instance_DidChangeView,
      &Instance_DidChangeFocus,
      &Instance_HandleDocumentLoad,
    };
    return &instance_interface;
  }
  return NULL;
}


/**
 * Called before the plugin module is unloaded.
 */
PP_EXPORT void PPP_ShutdownModule() {
}

/*
int main(int argc, char** argv) {
	printf("Hello World\n");
	glBegin(GL_TRIANGLES);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f);
	glEnd();
}
*/