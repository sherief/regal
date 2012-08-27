#include <ppapi/c/pp_errors.h>
#include <ppapi/c/pp_module.h>
#include <ppapi/c/pp_var.h>
#include <ppapi/c/ppb.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/c/ppb_messaging.h>
#include <ppapi/c/ppb_var.h>
#include <ppapi/c/ppb_view.h>
#include <ppapi/c/ppp.h>
#include <ppapi/c/ppp_instance.h>
#include <ppapi/c/ppp_messaging.h>
#include <ppapi/c/ppp_input_event.h>
#include <ppapi/c/ppb_graphics_3d.h>
#include <ppapi/c/ppb_opengles2.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/c/ppb_input_event.h>

#include "render.h"

#define GL_LOG_ERROR_REGAL    0x9319     /* 37657 */
#define GL_LOG_WARNING_REGAL  0x931a     /* 37658 */
#define GL_LOG_INFO_REGAL     0x931b     /* 37659 */
#define GL_LOG_APP_REGAL      0x931c     /* 37660 */
#define GL_LOG_DRIVER_REGAL   0x931d     /* 37661 */
#define GL_LOG_INTERNAL_REGAL 0x931e     /* 37662 */
#define GL_LOG_DEBUG_REGAL    0x931f     /* 37663 */
#define GL_LOG_STATUS_REGAL   0x9320     /* 37664 */
#define GL_LOG_HTTP_REGAL     0x9321     /* 37665 */

#define GL_ERROR_REGAL     0x9322     /* 37666 */
#define GL_DEBUG_REGAL     0x9323     /* 37667 */
#define GL_LOG_REGAL       0x9324     /* 37668 */
#define GL_EMULATION_REGAL 0x9325     /* 37669 */
#define GL_DRIVER_REGAL    0x9326     /* 37670 */

#include <cstdio>
#include <cstring>
#include <cstdarg>

static PPB_Messaging          *ppb_messaging_interface = NULL;
static PPB_Var                *ppb_var_interface = NULL;
static PPB_View               *ppb_view_interface = NULL;
static PPB_Graphics3D         *ppb_graphics3d_interface = NULL;
static PPB_Instance           *ppb_instance_interface = NULL;
static PPB_OpenGLES2          *ppb_opengl_interface = NULL;
static PPB_KeyboardInputEvent *ppb_keyboard_interface = NULL;
static PPB_InputEvent         *ppb_input_interface = NULL;
static PP_Resource             opengl_context = 0;

PP_Instance printfInstance = 0;

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

static void messagePrint(const char* str) {
  if (ppb_messaging_interface != NULL && printfInstance != 0) {
    ppb_messaging_interface->PostMessage(printfInstance, ::CStrToVar(str));  
  }
}

void messagePrintf(const char* str, ...) {
  const int bufferSize = 1024;
  char buff[bufferSize];
  buff[0] = '\0';
  va_list vl;
  va_start(vl, str);
  vsnprintf(&buff[0], bufferSize, str, vl);
  va_end(vl);
  messagePrint(buff);
}

static void myLogCallback(GLenum stream, GLsizei length, const char *message, GLvoid *context)
{
  messagePrint(message);
}

static void myDraw(void *user_data, int32_t result)
{
  dreamTorusDisplay(true);

  PP_CompletionCallback ccb;
  ccb.func = myDraw;
  ccb.user_data = NULL;
  ccb.flags = 0;
  ppb_graphics3d_interface->SwapBuffers(opengl_context, ccb);
}

extern "C"
{
  extern void RegalMakeCurrent(PP_Resource ctx, struct PPB_OpenGLES2 *interface);
  extern void glLogMessageCallbackREGAL(void (*callback)(GLenum, GLsizei, const char *, GLvoid *));
  extern void glEnable(GLenum cap);
};

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {
  printfInstance = instance;
  messagePrintf("Regal Dream Torus (GLIBC)");
  int32_t attribs[] = { PP_GRAPHICS3DATTRIB_WIDTH, 512, PP_GRAPHICS3DATTRIB_HEIGHT, 512, PP_GRAPHICS3DATTRIB_NONE};
  opengl_context = ppb_graphics3d_interface->Create(instance, opengl_context, attribs);
  ppb_instance_interface->BindGraphics(instance, opengl_context);

  RegalMakeCurrent(opengl_context, ppb_opengl_interface);
  glLogMessageCallbackREGAL(myLogCallback);

//  glEnable(GL_LOG_APP_REGAL);
  glEnable(GL_ERROR_REGAL);

  myDraw(NULL, 0);
  return PP_TRUE;
}

/**
 * Called when the NaCl module is destroyed. This will always be called,
 * even if DidCreate returned failure. This routine should deallocate any data
 * associated with the instance.
 * @param[in] instance The identifier of the instance representing this NaCl
 *     module.
 */
static void Instance_DidDestroy(PP_Instance instance)
{
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
static void Instance_DidChangeView(PP_Instance instance, PP_Resource view_resource) {
  PP_Rect rect;
  ppb_view_interface->GetRect(view_resource, &rect);
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
static void Instance_DidChangeFocus(PP_Instance instance, PP_Bool has_focus)
{
  if (has_focus) {
    messagePrintf("Gained focus");
  } else {
    messagePrintf("Lost focus");
  }
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
static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance, PP_Resource url_loader)
{
  return PP_FALSE;
}

// Entry points for the module.
// Initialize needed interfaces: PPB_Core, PPB_Messaging and PPB_Var.

PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id, PPB_GetInterface get_browser)
{
  ppb_messaging_interface  = (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface        = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  ppb_view_interface       = (PPB_View*)(get_browser(PPB_VIEW_INTERFACE));
  ppb_graphics3d_interface = (PPB_Graphics3D*)(get_browser(PPB_GRAPHICS_3D_INTERFACE));
  ppb_instance_interface   = (PPB_Instance*)(get_browser(PPB_INSTANCE_INTERFACE));
  ppb_opengl_interface     = (PPB_OpenGLES2*)(get_browser(PPB_OPENGLES2_INTERFACE));
  ppb_keyboard_interface   = (PPB_KeyboardInputEvent*)(get_browser(PPB_KEYBOARD_INPUT_EVENT_INTERFACE));
  ppb_input_interface      = (PPB_InputEvent*)(get_browser(PPB_INPUT_EVENT_INTERFACE));
  return PP_OK;
}

// Returns an interface pointer for the interface of the given name

PP_EXPORT const void* PPP_GetInterface(const char* interface_name)
{
  if (!strcmp(interface_name, PPP_INSTANCE_INTERFACE))
  {
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

// Called before the plugin module is unloaded.
 
PP_EXPORT void PPP_ShutdownModule()
{
}
