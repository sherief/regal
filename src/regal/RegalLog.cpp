/*
  Copyright (c) 2011 NVIDIA Corporation
  Copyright (c) 2011-2012 Cass Everitt
  Copyright (c) 2012 Scott Nations
  Copyright (c) 2012 Mathias Schott
  Copyright (c) 2012 Nigel Stewart
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

#include "pch.h" /* For MS precompiled header support */

#include "RegalUtil.h"

REGAL_GLOBAL_BEGIN

#include <cstdio>
#include <cstdarg>

#include <boost/print/string_list.hpp>

#include "RegalLog.h"
#include "RegalJson.h"
#include "RegalTimer.h"
#include "RegalMarker.h"
#include "RegalThread.h"
#include "RegalContext.h"

#if !(REGAL_SYS_WGL || (REGAL_SYS_PPAPI && !defined(__native_client__)))
#include <pthread.h>
#endif

// Otherwise we'd need to #include <windows.h>

#if defined(_WIN32) && !defined(__native_client__)
extern "C"
{
  __declspec(dllimport) void __stdcall OutputDebugStringA( __in_opt const char* lpOutputString);
}
#endif

#if REGAL_SYS_ANDROID
#include <android/log.h>
#endif

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

using ::std::string;
using ::std::list;
using ::boost::print::trim;
using ::boost::print::print_string;

using namespace ::boost::print;

typedef string_list<string> string_list;

namespace Logging {

  bool enableError    = true;
  bool enableWarning  = true;
  bool enableInfo     = true;
  bool enableApp      = false;
  bool enableDriver   = false;
  bool enableInternal = false;
  bool enableHttp     = true;

  int  maxLines  = (REGAL_LOG_MAX_LINES);
  int  maxBytes  = (REGAL_LOG_MAX_BYTES);
  bool frameTime = false;
  bool pointers  = (REGAL_LOG_POINTERS);
  bool thread    = false;
  bool callback  = (REGAL_LOG_CALLBACK);

  bool         log          = (REGAL_LOG);
  std::string  logFilename  = "stdout";
  FILE        *logOutput    = NULL;

  bool         json         = false;
  std::string  jsonFilename;
  FILE        *jsonOutput   = NULL;

  std::list<std::string> *buffer = NULL;
  std::size_t             bufferSize  = 0;
  std::size_t             bufferLimit = 500;

  bool initialized = false;

#if REGAL_LOG_ONCE
  bool once      = (REGAL_LOG_ONCE);
  std::set<std::string> uniqueErrors;
  std::set<std::string> uniqueWarnings;
#endif

  Timer                   timer;

  void Init()
  {
#ifndef REGAL_NO_GETENV
    const char *tmp;

    tmp = GetEnv("REGAL_LOG_ERROR");
    if (tmp) enableError = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG_WARNING");
    if (tmp) enableWarning = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG_INFO");
    if (tmp) enableInfo = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG_APP");
    if (tmp) enableApp = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG_DRIVER");
    if (tmp) enableDriver = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG_INTERNAL");
    if (tmp) enableInternal = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG_HTTP");
    if (tmp) enableHttp = atoi(tmp)!=0;

    //

    tmp = GetEnv("REGAL_LOG_API");
    if (tmp && atoi(tmp)) enableApp = enableDriver = true;

    tmp = GetEnv("REGAL_LOG_ALL");
    if (tmp && atoi(tmp)) enableError = enableWarning = enableInfo = enableApp = enableDriver = enableInternal = enableHttp = true;

    //

    tmp = GetEnv("REGAL_LOG_MAX_LINES");
    if (tmp) maxLines = atoi(tmp);

    tmp = GetEnv("REGAL_LOG_MAX_BYTES");
    if (tmp) maxBytes = atoi(tmp);

#if REGAL_LOG_ONCE
    tmp = GetEnv("REGAL_LOG_ONCE");
    if (tmp) once = atoi(tmp)!=0;
#endif

    tmp = GetEnv("REGAL_FRAME_TIME");
    if (tmp) frameTime = atoi(tmp)!=0;

#if REGAL_LOG_POINTERS
    tmp = GetEnv("REGAL_LOG_POINTERS");
    if (tmp) pointers = atoi(tmp)!=0;
#endif

#if REGAL_LOG_THREAD
    tmp = GetEnv("REGAL_LOG_THREAD");
    if (tmp) thread = atoi(tmp)!=0;
#endif

    tmp = GetEnv("REGAL_LOG_CALLBACK");
    if (tmp) callback = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG");
    if (tmp) log = atoi(tmp)!=0;

    tmp =  GetEnv("REGAL_LOG_FILE");
    if (tmp) logFilename = tmp;

    tmp = GetEnv("REGAL_LOG_JSON");
    if (tmp) json = atoi(tmp)!=0;

    tmp = GetEnv("REGAL_LOG_JSON_FILE");
    if (tmp) jsonFilename = tmp;

    tmp = GetEnv("REGAL_HTTP_LOG_LIMIT");
    if (tmp) bufferLimit = atoi(tmp);
#endif

#ifdef REGAL_HTTP_LOG_LIMIT
    bufferLimit = REGAL_HTTP_LOG_LIMIT;
#endif

    // TODO - clean this up at shutdown...

    if (bufferLimit)
      buffer = new list<string>();

    // Text logging

    if (logFilename.length())
      logOutput = fileOpen(logFilename.c_str(),"wt");

    // JSON logging

    if (jsonFilename.length())
      jsonOutput = fileOpen(jsonFilename.c_str(),"wt");
    if (jsonOutput)
      fprintf(jsonOutput,"%s","{ \"traceEvents\" : [\n");

    Internal("Logging::Init","()");

    initialized = true;

#if REGAL_LOG
    Info("REGAL_LOG          ", log            ? "enabled" : "disabled");
#endif

#if REGAL_LOG_ERROR
    Info("REGAL_LOG_ERROR    ", enableError    ? "enabled" : "disabled");
#endif

#if REGAL_LOG_WARNING
    Info("REGAL_LOG_WARNING  ", enableWarning  ? "enabled" : "disabled");
#endif

#if REGAL_LOG_INFO
    Info("REGAL_LOG_INFO     ", enableInfo     ? "enabled" : "disabled");
#endif

#if REGAL_LOG_APP
    Info("REGAL_LOG_APP      ", enableApp      ? "enabled" : "disabled");
#endif

#if REGAL_LOG_DRIVER
    Info("REGAL_LOG_DRIVER   ", enableDriver   ? "enabled" : "disabled");
#endif

#if REGAL_LOG_INTERNAL
    Info("REGAL_LOG_INTERNAL ", enableInternal ? "enabled" : "disabled");
#endif

#if REGAL_LOG_HTTP
    Info("REGAL_LOG_HTTP     ", enableHttp     ? "enabled" : "disabled");
#endif

#if REGAL_LOG_JSON
    Info("REGAL_LOG_JSON     ", json           ? "enabled" : "disabled");
#endif

#if REGAL_LOG_CALLBACK
    Info("REGAL_LOG_CALLBACK ", callback       ? "enabled" : "disabled");
#endif

#if REGAL_LOG_STDOUT
    Info("REGAL_LOG_STDOUT   ", stdOut         ? "enabled" : "disabled");
#endif

#if REGAL_LOG_ONCE
    Info("REGAL_LOG_ONCE     ", once           ? "enabled" : "disabled");
#endif

#if REGAL_LOG_POINTERS
    Info("REGAL_LOG_POINTERS ", pointers       ? "enabled" : "disabled");
#endif

#if REGAL_LOG_THREAD
    Info("REGAL_LOG_THREAD   ", thread         ? "enabled" : "disabled");
#endif
  }

  void Cleanup()
  {
    Internal("Logging::Cleanup","()");

    initialized = false;

    if (logOutput)
      fileClose(&logOutput);

    if (jsonOutput)
    {
      fprintf(jsonOutput,"%s","{} ] }\n");
      fileClose(&jsonOutput);
    }
  }

  void
  writeJSON(Json::Output &jo)
  {
    jo.object("logging");

    jo.member("error",     enableError);
    jo.member("warning",   enableWarning);
    jo.member("info",      enableInfo);
    jo.member("app",       enableApp);
    jo.member("driver",    enableDriver);
    jo.member("internal",  enableInternal);
    jo.member("http",      enableHttp);

    jo.member("maxLines",  maxLines);
    jo.member("maxBytes",  maxBytes);

#if REGAL_LOG_ONCE
    jo.member("once",      once);
#endif

    jo.member("frameTime", frameTime);
    jo.member("pointers",  pointers);
    jo.member("thread",    thread);

    jo.member("callback",    callback);
    jo.member("log",         log);
    jo.member("filename",    logFilename);
    jo.member("json",        json);
    jo.member("jsonFile",    jsonFilename);
    jo.member("bufferLimit", bufferLimit);

    jo.end();
  }

  inline size_t indent()
  {
    // For OSX we need avoid REGAL_GET_CONTEXT implicitly
    // trying to create a RegalContext and triggering more
    // (recursive) logging.

#if !REGAL_SYS_WGL && !REGAL_NO_TLS
    if (!Thread::currentContextKey || !pthread_getspecific(Thread::currentContextKey))
      return 0;
#endif

    RegalContext *rCtx = REGAL_GET_CONTEXT();

    // Clamp indentation to avoid underflow situation (more pops than pushes)
    // If size_t depthBeginEnd wraps around to a huge number, we probably won't have
    // enough RAM for all those spaces...

    const size_t indentMax = size_t(128);

    size_t indent = 0;

    if (rCtx)
    {
      indent += std::min(indentMax,rCtx->depthBeginEnd  *2);
      indent += std::min(indentMax,rCtx->depthPushMatrix*2);
      indent += std::min(indentMax,rCtx->depthPushAttrib*2);
      indent += std::min(indentMax,rCtx->depthNewList   *2);
      indent += std::min(indentMax,rCtx->marker ? rCtx->marker->indent() : 0);
    }

    return indent;
  }

  inline string message(const char *prefix, const char *delim, const char *name, const string &str)
  {
    static const char *trimSuffix = " ...";
    string_list trimPrefix;
    trimPrefix << print_string(prefix ? prefix : "",delim ? delim : "");
    if (thread)
      trimPrefix << print_string(hex(Thread::threadId()),delim ? delim : "");
    trimPrefix << print_string(string(indent(),' '),name ? name : "",name ? " " : "");
    return print_string(trim(str.c_str(),'\n',maxLines>0 ? maxLines : ~0,trimPrefix.str().c_str(),trimSuffix), '\n');
  }

  inline string jsonObject(const char *prefix, const char *name, const string &str)
  {
    //
    // http://www.altdevblogaday.com/2012/08/21/using-chrometracing-to-view-your-inline-profiling-data/
    //
    // object {
    // "cat": "MY_SUBSYSTEM",  //catagory
    // "pid": 4260,  //process ID
    // "tid": 4776, //thread ID
    // "ts": 2168627922668, //time-stamp of this event
    // "ph": "B", // Begin sample
    // "name": "doSomethingCostly", //name of this event
    // "args": { //arguments associated with this event.
    //}
    //

    string_list os;
    os << "{\n";
    os << ::boost::print::json::member(::boost::print::json::pair("cat",prefix));
    os << ::boost::print::json::member(::boost::print::json::pair("pid",Thread::procId()));
    os << ::boost::print::json::member(::boost::print::json::pair("tid",Thread::threadId()%(1<<16)));
    os << ::boost::print::json::member(::boost::print::json::pair("ts",timer.now()));

    if (!name)
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","I"));
      os << ::boost::print::json::member(::boost::print::json::pair("name",str));
      os << "\"args\" : {} \n";
    }
    else if (!strcmp(name,"glBegin"))
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","B"));
      os << ::boost::print::json::member(::boost::print::json::pair("name","glBegin"));
      os << "\"args\" : { ";
      os << ::boost::print::json::member(::boost::print::json::pair("inputs",str),false);
      os << "}\n";
    }
    else if (!strcmp(name,"glEnd"))
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","E"));
      os << ::boost::print::json::member(::boost::print::json::pair("name","glBegin"));
      os << "\"args\" : {} \n";
    }
    else if (!strcmp(name,"glPushMatrix"))
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","B"));
      os << ::boost::print::json::member(::boost::print::json::pair("name","glPushMatrix"));
      os << "\"args\" : { ";
      os << ::boost::print::json::member(::boost::print::json::pair("inputs",str),false);
      os << "}\n";
    }
    else if (!strcmp(name,"glPopMatrix"))
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","E"));
      os << ::boost::print::json::member(::boost::print::json::pair("name","glPushMatrix"));
      os << "\"args\" : {} \n";
    }
    else if (!strcmp(name,"glPushGroupMarkerEXT"))
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","B"));
      os << ::boost::print::json::member(::boost::print::json::pair("name","glPushGroupMarkerExt"));
      os << "\"args\" : { ";
      os << ::boost::print::json::member(::boost::print::json::pair("inputs",str),false);
      os << "}\n";
    }
    else if (!strcmp(name,"glPopGroupMarkerEXT"))
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","E"));
      os << ::boost::print::json::member(::boost::print::json::pair("name","glPushGroupMarkerExt"));
      os << "\"args\" : {} \n";
    }
    else
    {
      os << ::boost::print::json::member(::boost::print::json::pair("ph","I"));
      os << ::boost::print::json::member(::boost::print::json::pair("name",name ? name : ""));
      os << "\"args\" : { ";
      os << ::boost::print::json::member(::boost::print::json::pair("inputs",str),false);
      os << "}\n";
    }
    os << "},\n";
    return os.str();
  }
  // Append to the log buffer

  inline void append(string &str)
  {
    if (buffer)
    {
      buffer->push_back(string());
      buffer->back().swap(str);
      bufferSize++;

      // Prune the buffer list, as necessary

      while (bufferSize>bufferLimit)
      {
        buffer->pop_front();
        --bufferSize;
      }
    }
  }

#ifndef REGAL_LOG_TAG
#define REGAL_LOG_TAG "Regal"
#endif

  void Output(const Mode mode, const char *file, const int line, const char *prefix, const char *delim, const char *name, const string &str)
  {
    if (initialized && str.length())
    {
      string m = message(prefix,delim,name,str);

      // TODO - optional Regal source line numbers.
#if 1
      UNUSED_PARAMETER(file);
      UNUSED_PARAMETER(line);
#else
      m = print_string(file,":",line," ",m);
#endif

#if REGAL_LOG_ONCE
      if (once)
        switch (mode)
        {
          case LOG_WARNING:
            if (uniqueWarnings.find(m)!=uniqueWarnings.end())
              return;
            uniqueWarnings.insert(m);
            break;

          case LOG_ERROR:
            if (uniqueErrors.find(m)!=uniqueErrors.end())
              return;
            uniqueErrors.insert(m);
            break;

          default:
            break;
        }
#endif

      RegalContext *rCtx = NULL;

#if !REGAL_SYS_WGL && !REGAL_NO_TLS
      if (Thread::currentContextKey && pthread_getspecific(Thread::currentContextKey))
        rCtx = REGAL_GET_CONTEXT();
#else
      rCtx = REGAL_GET_CONTEXT();
#endif

#if REGAL_LOG_CALLBACK
      if (callback && rCtx && rCtx->logCallback)
        rCtx->logCallback(GL_LOG_INFO_REGAL, (GLsizei) m.length(), m.c_str(), reinterpret_cast<void *>(rCtx->sysCtx));
#endif

#if REGAL_SYS_WGL
      OutputDebugStringA(m.c_str());
#elif REGAL_SYS_ANDROID
      // ANDROID_LOG_INFO
      // ANDROID_LOG_WARN
      // ANDROID_LOG_ERROR
      __android_log_write(ANDROID_LOG_INFO, REGAL_LOG_TAG, m.c_str());
#endif

#if REGAL_LOG_JSON
      if (json && jsonOutput)
      {
        string m = jsonObject(prefix,name,str);
        fwrite(m.c_str(),m.length(),1,jsonOutput);
      }
#endif

#if REGAL_LOG
      if (log && logOutput)
      {
#if REGAL_SYS_ANDROID
#else
        fprintf(logOutput, "%s", m.c_str());
        fflush(logOutput);
#endif
      }
#endif

      append(m);
    }
  }
}

REGAL_NAMESPACE_END
