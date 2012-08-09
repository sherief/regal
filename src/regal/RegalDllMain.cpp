// dllmain.cpp : Defines the entry point for the DLL application.

#include "pch.h" /* For MS precompiled header support */

#include "RegalUtil.h"

REGAL_GLOBAL_BEGIN

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#endif

REGAL_GLOBAL_END

#ifdef _WIN32

REGAL_NAMESPACE_BEGIN

REGAL_NAMESPACE_END

#pragma warning( disable: 4273 )

//
// We have to hijack SetPixelFormat to make sure
// LoadLibrary("OpenGL32.dll") happens first...
//

typedef BOOL (WINAPI *SetPixelFormatPROC)(__in HDC hdc, __in int format, __in CONST PIXELFORMATDESCRIPTOR * ppfd);

BOOL WINAPI SetPixelFormat(__in HDC hdc, __in int iPixelFormat, __in CONST PIXELFORMATDESCRIPTOR * ppfd)
{
  static SetPixelFormatPROC spf = NULL;
  static bool initialized = false;

  if (!initialized)
  {
    initialized = true;

    // Force LoadLibrary of opengl32.dll
    Regal::GetProcAddress( NULL );

    // Pass call through to gdi32.dll

    HMODULE lib_GDI = LoadLibraryA( "gdi32.dll" );
    spf = (SetPixelFormatPROC) ::GetProcAddress( lib_GDI, "SetPixelFormat" );
  }
  return spf ? spf( hdc, iPixelFormat, ppfd ) : 0;
}

BOOL APIENTRY
DllMain
(
  HMODULE module,
  DWORD   reason,
  LPVOID  reserved
)
{
  UNUSED_PARAMETER(module);
  UNUSED_PARAMETER(reserved);

  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

#endif
