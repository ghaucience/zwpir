/***************************************************************************
*
* Copyright (c) 2001-2014
* Sigma Designs, Inc.
* All Rights Reserved
*
*---------------------------------------------------------------------------
*
* Description: Some nice descriptive description.
*
* Author: Thomas Roll
*
* Last Changed By: $Author: tro $
* Revision: $Revision: 0.00 $
* Last Changed: $Date: 2014/06/02 10:34:49 $
*
****************************************************************************/

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/
#include <ZW_string.h>
#include <ZW_uart_api.h>
/****************************************************************************/
/*                      PRIVATE TYPES and DEFINITIONS                       */
/****************************************************************************/

/****************************************************************************/
/*                              PRIVATE DATA                                */
/****************************************************************************/

/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                            PRIVATE FUNCTIONS                             */
/****************************************************************************/




/*============================ ZW_strlen ===============================
** Function description
** Calc string len
**
** Side effects: 
**
**-------------------------------------------------------------------------*/
BYTE
ZW_strlen(BYTE* str)
{
	BYTE *s;
	for (s = str; *s; ++s)
	{
	  ;
	}
	return(s - str);
}

