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

#ifndef __REGAL_DISPATCH_STATE_H__
#define __REGAL_DISPATCH_STATE_H__

#include "RegalUtil.h"

REGAL_GLOBAL_BEGIN

#include "RegalPrivate.h"

REGAL_GLOBAL_END

REGAL_NAMESPACE_BEGIN

#define REGAL_MAX_DISPATCH_TABLE_STACK_SIZE 5

void InitDispatchTableDebug  (DispatchTable &tbl);
void InitDispatchTableError  (DispatchTable &tbl);
void InitDispatchTableEmu    (DispatchTable &tbl);
void InitDispatchTableLoader (DispatchTable &tbl);
void InitDispatchTableLog    (DispatchTable &tbl);
void InitDispatchTableNacl   (DispatchTable &tbl);
void InitDispatchTableMissing(DispatchTable &tbl);

enum RegalDispatchTableEnum {
   RDT_Debug  = 5,
   RDT_Error  = 4,
   RDT_Emu    = 3,
   RDT_Log    = 2,
   RDT_Driver = 1
};

struct DispatchState {
   void Init()
   {
      for( int i = 0; i < REGAL_MAX_DISPATCH_TABLE_STACK_SIZE + 1; i++ ) {
        stack[i] = NULL;
      }
      stackSize = 0;
      stackCurr = 0; // todo: remove stackCurr
      curr = stack[ stackCurr ];

      // Dispatch table initialization

#if REGAL_DEBUG
      InitDispatchTableDebug( dbgTbl );
#endif

#if REGAL_ERROR
      InitDispatchTableError( errorTbl );
#endif

#if REGAL_LOG
      InitDispatchTableLog( emuTbl );
#endif

      InitDispatchTableEmu( emuTbl );   // overrides emulated functions only

#if REGAL_LOG
      InitDispatchTableLog( logTbl );
#endif

#if defined(__native_client__)
      InitDispatchTableMissing(driverTbl);
      InitDispatchTableNacl   (driverTbl);
#else
      InitDispatchTableLoader( driverTbl );
#endif

      // Dispatch table stack

#if REGAL_DEBUG
      if (Config::enableDebug)
        Insert(0, RDT_Debug);
#endif

#if REGAL_ERROR
      if (Config::enableError)
        Insert(0, RDT_Error);
#endif

      Insert( 0, RDT_Emu );

#if REGAL_LOG
      if (Config::enableLog)
        Insert( 0, RDT_Log );
#endif

      Insert( 0, RDT_Driver );
   }
   void StepDown()
   {
      RegalAssert( stackCurr > 0 );
      stackCurr--;
      curr = stack[stackCurr];
   }
   void StepUp()
   {
      RegalAssert( stackCurr < REGAL_MAX_DISPATCH_TABLE_STACK_SIZE );
      stackCurr++;
      curr = stack[stackCurr];
   }
   DispatchTable *Table( RegalDispatchTableEnum dt )
   {
      switch( dt ) {

#if REGAL_DEBUG
        case RDT_Debug:  return &dbgTbl;
#endif

#if REGAL_ERROR
        case RDT_Error:  return &errorTbl;
#endif

        case RDT_Emu:    return &emuTbl;

#if REGAL_ERROR
        case RDT_Log:    return &logTbl;
#endif

        case RDT_Driver: return &driverTbl;

        default: break;
      }
      return NULL;
   }

   void Insert( int loc, RegalDispatchTableEnum dt )
   {
      DispatchTable * tbl = Table( dt );
      if (loc > stackSize || loc < 0 || tbl == NULL) {
         return;
      }
      if (stackSize >= REGAL_MAX_DISPATCH_TABLE_STACK_SIZE) {
         return;
      }
      stackSize++;
      stackCurr = stackSize - 1;
      for( int i = stackSize-1; i > loc; i-- ) {
         stack[ i ] = stack[ i - 1 ];
      }
      stack[ loc ] = tbl;
      curr = stack[stackCurr];
   }

   void Erase( RegalDispatchTableEnum dt )
   {
      DispatchTable * tbl = Table( dt );
      int slot = 0;
      for( int i = 1; i < REGAL_MAX_DISPATCH_TABLE_STACK_SIZE; i++ ) {
         if (stack[i] == tbl) {
            slot = i;
            break;
         }
      }
      if (slot == REGAL_MAX_DISPATCH_TABLE_STACK_SIZE) {
         return;
      }
      for( int i = slot; i < REGAL_MAX_DISPATCH_TABLE_STACK_SIZE; i++ ) {
         stack[ i ] = stack[ i + 1 ];
      }
      stackSize--;
      stackCurr = stackSize - 1;
      curr = stack[stackCurr];
   }

   DispatchTable *stack[ REGAL_MAX_DISPATCH_TABLE_STACK_SIZE + 1 ];
   DispatchTable *curr;
   int stackSize;
   int stackCurr;

#if REGAL_DEBUG
   DispatchTable dbgTbl;
#endif

#if REGAL_ERROR
   DispatchTable errorTbl;
#endif

   DispatchTable emuTbl;

#if REGAL_LOG
   DispatchTable logTbl;
#endif

   DispatchTable driverTbl;
};

struct DispatchStateScopedStepDown
{
public:
  DispatchStateScopedStepDown(DispatchState &dispatchState)
  : dsp(&dispatchState)
  {
    RegalAssert(dsp);
    dsp->StepDown();
  }

  DispatchStateScopedStepDown(DispatchState *dispatchState)
  : dsp(dispatchState)
  {
    RegalAssert(dsp);
    dsp->StepDown();
  }

  ~DispatchStateScopedStepDown()
  {
    RegalAssert(dsp);
    dsp->StepUp();
  }

private:
  DispatchState *dsp;

  // make these private, not needed

  DispatchStateScopedStepDown(const DispatchStateScopedStepDown &other);
  DispatchStateScopedStepDown &operator=(const DispatchStateScopedStepDown &other);
};

REGAL_NAMESPACE_END

#endif // __REGAL_DISPATCH_STATE_H__
