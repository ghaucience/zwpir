/***************************************************************************
*
* Copyright (c) 2001-2013
* Sigma Designs, Inc.
* All Rights Reserved
*
*---------------------------------------------------------------------------
*
* Description: Timer service functions that handle delayed functions calls.
 *              The time resolution is 1 second.
 *              Features:
 *                       1. Timer Start.
 *                        2. Timer Stop.
 *                        3. Timer Restart.
 *                        4. Getr time passed since start.
 *                        5. Runs once, many times or forever.
*
* Author: Samer Seoud
*
* Last Changed By: $Author: tro $
* Revision: $Revision: 0.00 $
* Last Changed: $Date: 2013/06/25 14:38:36 $
*
****************************************************************************/

#include "config_lib.h"

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include <ZW_basis_api.h>
#include <appl_timer_api.h>

#ifdef ZW_DEBUG_APPL_TIMER
#include <ZW_uart_api.h>
#define ZW_DEBUG_APPL_TIMER_SEND_BYTE(data) ZW_DEBUG_SEND_BYTE(data)
#define ZW_DEBUG_APPL_TIMER_SEND_NUM(data)  ZW_DEBUG_SEND_NUM(data)
#define ZW_DEBUG_APPL_TIMER_SEND_NL()  	   ZW_DEBUG_SEND_NL()
#else
#define ZW_DEBUG_APPL_TIMER_SEND_BYTE(data)
#define ZW_DEBUG_APPL_TIMER_SEND_NUM(data)
#endif


/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/

/****************************************************************************/
/*                              PRIVATE DATA                                */
/****************************************************************************/
typedef struct _ZWAVEPLUS_APPL_TIMER_ELEMENT_
{
  DWORD  startTime;
  DWORD  tickCount;
  DWORD  timeoutValue;
  VOID_CALLBACKFUNC(func)(void);
  BYTE  repeats;
} ZWAVEPLUS_APPL_TIMER_ELEMENT;


static ZWAVEPLUS_APPL_TIMER_ELEMENT applTimerArray[APPL_TIMER_MAX];
static IDWORD applTickTime;  /* global counter that is incremented every 10 msec */
/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/
void ZCB_ApplTimerAction( void );

code const void (code * ZCB_ApplTimerAction_p)(void) = &ZCB_ApplTimerAction;
/*================================   TimerAction   ===========================
**    Walk through the timer elements and calls the timeout functions.
**
**--------------------------------------------------------------------------*/
void                      /*RET Nothing    */
ZCB_ApplTimerAction( void )       /* IN Nothing    */
{
  register IBYTE timerHandle;
  applTickTime++;
  /* walk through the timer array */
  for (timerHandle = 0; timerHandle < APPL_TIMER_MAX; timerHandle++)
  {
    if (applTimerArray[timerHandle].tickCount)
    {
      if (--applTimerArray[timerHandle].tickCount == 0) /* count change from 1 to 0 */
      {
        if (applTimerArray[timerHandle].repeats)
        { /* repeat timer action */
          applTimerArray[timerHandle].tickCount = applTimerArray[timerHandle].timeoutValue;
          if (applTimerArray[timerHandle].repeats != APPL_TIMER_FOREVER)
          { /* repeat limit */
            applTimerArray[timerHandle].repeats--;
          }
        }

        if (NULL != applTimerArray[timerHandle].func)
        {
          /* call the timeout function */
          applTimerArray[timerHandle].func();
        }
      }
    }
  }
}

/*===========================   ApplTimerGetTime   ==========================
**    Get the time passed since the start of the application timer
**    It return the time (in secs) passed since the slow timer is created
**    The time is identified with the timer handle
**--------------------------------------------------------------------------*/
DWORD /*RETURN: if it is less than 0xFFFFFFFF then it is the number of seconds passed since the timer was*/
      /*        created or restarted. If equal 0xFFFFFFFF then the timer handle is invalid*/
ApplTimerGetTime(BYTE btimerHandle)
{
  ZW_DEBUG_APPL_TIMER_SEND_BYTE('G');
  ZW_DEBUG_APPL_TIMER_SEND_NUM(btimerHandle);
  btimerHandle--;  /* Index 0...n */
  if ((btimerHandle < APPL_TIMER_MAX) && (applTimerArray[btimerHandle].timeoutValue))
  { /* valid timer element number */
    return applTickTime - applTimerArray[btimerHandle].startTime;
  }
  return (DWORD)-1;


}


/*================================   ApplTimerRestart  ===========================
**    Set the specified timer back to the initial value.
**    The timer start time will be sat�to the current value of the slow timer subsystem time
**--------------------------------------------------------------------------*/
BYTE                      /*RET TRUE if timer restarted   */
ApplTimerRestart(
  BYTE btimerHandle)       /* IN Timer number to restart   */
{
  ZW_DEBUG_APPL_TIMER_SEND_BYTE('R');
  ZW_DEBUG_APPL_TIMER_SEND_NUM(btimerHandle);
  btimerHandle--;  /* Index 0...n */
  if ((btimerHandle < APPL_TIMER_MAX) && (applTimerArray[btimerHandle].timeoutValue))
  { /* valid timer element number */
    applTimerArray[btimerHandle].tickCount = applTimerArray[btimerHandle].timeoutValue;
    applTimerArray[btimerHandle].startTime = applTickTime;
    return TRUE;
  }
  return(FALSE);
}


/*===============================   ApplTimerStop  ===========================
**    Stop the specified timer.
**    and set timerhandle to 0
**--------------------------------------------------------------------------*/
void
ApplTimerStop(
  BYTE *pbTimerHandle)
{
  ZW_DEBUG_APPL_TIMER_SEND_BYTE('C');
  ZW_DEBUG_APPL_TIMER_SEND_NUM(*pbTimerHandle);
  --*pbTimerHandle;  /* Index 0...n */
  if (*pbTimerHandle < APPL_TIMER_MAX)
  {
    /* valid timer element number */
    applTimerArray[*pbTimerHandle].startTime = 0;
    applTimerArray[*pbTimerHandle].tickCount = 0;
    applTimerArray[*pbTimerHandle].timeoutValue = 0; /* stop the timer */
  }
  *pbTimerHandle = 0;
}


/*============================   ApplTimerStart   =========================
** Creat a slow timer instance
**
**--------------------------------------------------------------------------*/
BYTE                        /* Returns 1 to APPL_TIMER_MAX the timer handle created, 0 no timer is created */
ApplTimerStart(
  VOID_CALLBACKFUNC(func)(void), /*IN  Timeout function address          */
  DWORD ltimerTicks,          /*IN  Timeout value (in seconds)  */
  BYTE brepeats)             /*IN  Number of function calls (-1: forever)  */
{
  register BYTE timerHandle;
  for (timerHandle = 0; timerHandle < APPL_TIMER_MAX; timerHandle++)
  {
    /* find first free timer element */
    if (applTimerArray[timerHandle].tickCount == 0)
    {
      if (!ltimerTicks)
      {
        ltimerTicks++; /* min 1 sec. */
      }
      if (brepeats && (brepeats != APPL_TIMER_FOREVER))
      {
        brepeats--; /*brepeats = 0 then timer runs 1 timer, brepeats = 1 then timer run 2 times */
      }
      /* create new active timer element */
      applTimerArray[timerHandle].startTime = applTickTime;
      applTimerArray[timerHandle].timeoutValue = ltimerTicks;
      applTimerArray[timerHandle].tickCount = ltimerTicks;
#ifdef ZW_APPL_TIMER_DEBUG
      ZW_DEBUG_APPL_TIMER_SEND_BYTE('T');
      ZW_DEBUG_APPL_TIMER_SEND_NUM((BYTE)(ltimerTicks>>24));
      ZW_DEBUG_APPL_TIMER_SEND_NUM((BYTE)(ltimerTicks>>16);
      ZW_DEBUG_APPL_TIMER_SEND_NUM((BYTE)(ltimerTicks>>8);
      ZW_DEBUG_APPL_TIMER_SEND_NUM((BYTE)ltimerTicks);
#endif
      applTimerArray[timerHandle].repeats = brepeats;
      applTimerArray[timerHandle].func = func;
      return (timerHandle + 1);
    }
  }
  ZW_DEBUG_APPL_TIMER_SEND_BYTE('S');
  return (0);
}
/*============================   ApplTimerInit   =========================
** Initalize the ZWave plus application timer subsystem.
** Must only be called once
**
**--------------------------------------------------------------------------*/


BOOL
ApplTimerInit()
{
  register BYTE tmp;
  for (tmp = 0; tmp < APPL_TIMER_MAX; tmp++)
  {
    applTimerArray[tmp].tickCount = applTimerArray[tmp].timeoutValue = 0;
  }
  applTickTime = 0;

  if (TimerStart(ZCB_ApplTimerAction, 100, 0xFF) == 0xFF)
    return FALSE;
  else
    return TRUE;
}

