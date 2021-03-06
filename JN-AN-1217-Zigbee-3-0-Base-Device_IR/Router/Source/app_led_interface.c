/*****************************************************************************
 *
 * MODULE:             JN-AN-1217
 *
 * COMPONENT:          app_led_interface.c
 *
 * DESCRIPTION:        DK4 DR1175 Led interface (White Led)
 *
 ****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5179].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2016. All rights reserved
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include "dbg.h"
#include "LightingBoard.h"
#include "AppHardwareApi.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define BOARD_DIMMER_BIT               (13)
#define BOARD_DIMMER_PIN               (1UL << BOARD_DIMMER_BIT)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/




/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: APP_vSetLed
 *
 * DESCRIPTION: Set the LEDs
 *
 * PARAMETER: boolean
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void APP_vSetLed(bool_t bOn)
{
    (bOn) ? vAHI_DioSetOutput(BOARD_DIMMER_PIN , 0) : vAHI_DioSetOutput(0 , BOARD_DIMMER_PIN);
}

/****************************************************************************
 *
 * NAME: APP_vLedInitialise
 *
 * DESCRIPTION: LED Initialization
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void APP_vLedInitialise(void)
{
    vAHI_DioSetDirection(0, BOARD_DIMMER_PIN);
    vAHI_DioSetOutput(BOARD_DIMMER_PIN , 0);
    APP_vSetLed(FALSE);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
