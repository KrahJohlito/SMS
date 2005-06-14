/*
#     ___  _ _      ___
#    |    | | |    |
# ___|    |   | ___|    PS2DEV Open Source Project.
#----------------------------------------------------------
# Copyright (c) 200X by Hermes (his code was found at PS2Dev forums posted on Tue Jul 13, 2004 8:06 pm)
# Copyright (c) 2005 by Eugene Plotnikov (T1_XXXX stuff)
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/
#include "Timer.h"

#include <kernel.h>

#define T0_COUNT *(  ( volatile u32* )0x10000000  )
#define T0_MODE  *(  ( volatile u32* )0x10000010  )
#define T0_COMP  *(  ( volatile u32* )0x10000020  )
#define T0_HOLD  *(  ( volatile u32* )0x10000030  )

#define T1_COUNT *(  ( volatile u32* )0x10000800  )
#define T1_MODE  *(  ( volatile u32* )0x10000810  )
#define T1_COMP  *(  ( volatile u32* )0x10000820  )
#define T1_HOLD  *(  ( volatile u32* )0x10000830  )

volatile unsigned long int g_Timer;

static int s_WaitSema;
static int s_T0HandlerID;
static int s_T1HandlerID;

static int T0_Handler ( int aCause ) { 

 g_Timer += 1;

 T0_MODE |= 1024;

 return -1;

}  /* end T0_Handler */

static int T1_Handler ( int aCause ) {

 iSignalSema ( s_WaitSema );

 T1_MODE |= 1024;

 return -1;

}  /* end T1_Handler */

void Timer_Init ( void ) {

 ee_sema_t lSema;

 lSema.init_count = 0;
 lSema.max_count  = 1;
 s_WaitSema = CreateSema ( &lSema );

 T0_COMP  = ( u32 )(  1.0F / ( 256.0F / 147456.0F )  );
 T0_COUNT = 0;
 T0_MODE  = 256 + 128 + 64 + 2;

 T1_COMP  = ( u32 )(  100.0F / ( 256.0F / 147456.0F )  );
 T1_MODE  = 256 + 128 + 64 + 2;

 s_T0HandlerID = AddIntcHandler (  9, T0_Handler, 0 );
 s_T1HandlerID = AddIntcHandler ( 10, T1_Handler, 0 );
 EnableIntc ( 9 );

}  /* end Timer_Init */

void Timer_Destroy ( void ) {

 DisableIntc ( 9 );
 RemoveIntcHandler (  9, s_T0HandlerID );
 RemoveIntcHandler ( 10, s_T1HandlerID );

 DeleteSema ( s_WaitSema );

}  /* end Timer_Destroy */

void Timer_Wait ( unsigned int aPeriod ) {

 unsigned long i, lnParts, lnRem;

 lnParts = aPeriod / 100;
 lnRem   = aPeriod % 100;

 if ( lnParts ) {

  T1_COUNT = 0;
  T1_COMP  = ( u32 )(  100.0F / ( 256.0F / 147456.0F )  );

  EnableIntc  ( 10 );
   for ( i = 0; i < lnParts; ++i ) WaitSema ( s_WaitSema );
  DisableIntc ( 10 );

 }  // end if

 if ( lnRem ) {

  T1_COUNT = 0;
  T1_COMP  = ( u32 )(  lnRem / ( 256.0F / 147456.0F )  );

  EnableIntc  ( 10 );
   WaitSema ( s_WaitSema );
  DisableIntc ( 10 );

 }  // end if

}  /* end Timer_Wait */