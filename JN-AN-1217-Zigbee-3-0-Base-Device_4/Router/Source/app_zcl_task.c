/*****************************************************************************
 *
* MODULE:              JN-AN-1217
 *
 * COMPONENT:          app_zcl_task.c
 *
 * DESCRIPTION:       Base Device application - ZCL Interface
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
#include <AppApi.h>
#include "pdum_apl.h"
#include "pdum_gen.h"
#include "PDM.h"
#include "dbg.h"
#include "zps_gen.h"
#include "PDM_IDs.h"
#include "ZTimer.h"
#include "zcl.h"
#include "zcl_options.h"
#include "app_led_interface.h"
#include "app_zcl_task.h"
#include "app_router_node.h"
#include "app_common.h"
#include "app_main.h"
#include "base_device.h"
#include "app_events.h"
#include <string.h>
#include "app_reporting.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_ZCL
#define TRACE_ZCL   TRUE
#else
#define TRACE_ZCL   TRUE
#endif



/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#define ZCL_TICK_TIME           ZTIMER_TIME_MSEC(100)


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent);
PRIVATE void APP_ZCL_cbEndpointCallback(tsZCL_CallBackEvent *psEvent);
PUBLIC void APP_vHandleIdentify(uint16 u16Time);
PRIVATE void APP_vHandleClusterCustomCommands(tsZCL_CallBackEvent *psEvent);
PRIVATE void APP_vHandleClusterUpdate(tsZCL_CallBackEvent *psEvent);
PRIVATE void APP_vZCL_DeviceSpecific_Init(void);
/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

PUBLIC tsZHA_BaseDevice sBaseDevice;
PUBLIC tsZHA_BaseDevice sBaseDeviceSwitch1;
PUBLIC tsZHA_BaseDevice sBaseDeviceSwitch2;
PUBLIC tsZHA_BaseDevice sBaseDeviceSwitch3;

PUBLIC tsStartUpAttrib sZCLAttribs;
/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE uint8 u8IdentifyCount = 0;
PRIVATE bool_t bIdentifyState = FALSE;

PRIVATE void vLoadZCLRecord( void)
{
    uint16 u16ByteRead;
    PDM_eReadDataFromRecord(PDM_ID_APP_ONOFF,
                &sZCLAttribs,
                sizeof(tsStartUpAttrib), &u16ByteRead);

	sBaseDevice.sOnOffServerCluster.bOnOff 		  = (bool_t)sZCLAttribs.u8OnOffEp1;
	sBaseDeviceSwitch1.sOnOffServerCluster.bOnOff = (bool_t)sZCLAttribs.u8OnOffEp2;
	sBaseDeviceSwitch2.sOnOffServerCluster.bOnOff = (bool_t)sZCLAttribs.u8OnOffEp3;
	sBaseDeviceSwitch3.sOnOffServerCluster.bOnOff = (bool_t)sZCLAttribs.u8OnOffEp4;

	APP_vSetLed(sBaseDeviceSwitch3.sOnOffServerCluster.bOnOff);
	APP_vSetLed1(sBaseDeviceSwitch2.sOnOffServerCluster.bOnOff);
	APP_vSetLed2(sBaseDeviceSwitch1.sOnOffServerCluster.bOnOff);
	APP_vSetLed3(sBaseDevice.sOnOffServerCluster.bOnOff);
}
/****************************************************************************
 *
 * NAME: APP_ZCL_vInitialise
 *
 * DESCRIPTION:
 * Initialises ZCL related functions
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_ZCL_vInitialise(void)
{
	DBG_vPrintf(TRUE, "\nErr: APP_ZCL_vInitialise\n");
    teZCL_Status eZCL_Status;

    /* Initialise ZLL */
    eZCL_Status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
        DBG_vPrintf(TRACE_ZCL, "\nErr: eZLO_Initialise:%d", eZCL_Status);
    }

    /* Start the tick timer */
    if(ZTIMER_eStart(u8TimerZCL, ZCL_TICK_TIME) != E_ZTIMER_OK)
    {
        DBG_vPrintf(TRACE_ZCL, "APP: Failed to Start Tick Timer\n");
    }

    /* Register Light EndPoint 0*/
    eZCL_Status =  eZHA_RegisterBaseDeviceEndPoint(ROUTER_APPLICATION_ENDPOINT,
                                                   &APP_ZCL_cbEndpointCallback,
                                                   &sBaseDevice);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
            DBG_vPrintf(TRACE_ZCL, "Error: eZHA_RegisterBaseDeviceEndPoint(Switch1): %02x\r\n", eZCL_Status);
    }

    /* Register Light EndPoint 1*/
    eZCL_Status = eZHA_RegisterBaseDeviceEndPoint(ROUTER_SWITCH1_ENDPOINT,
                                                   &APP_ZCL_cbEndpointCallback,
                                                   &sBaseDeviceSwitch1);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
              DBG_vPrintf(TRACE_ZCL, "Error: eZHA_RegisterBaseDeviceEndPoint(Switch2): %02x\r\n", eZCL_Status);
    }

    /* Register Light EndPoint 2*/
    eZCL_Status = eZHA_RegisterBaseDeviceEndPoint(ROUTER_SWITCH2_ENDPOINT,
                                                   &APP_ZCL_cbEndpointCallback,
                                                   &sBaseDeviceSwitch2);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
              DBG_vPrintf(TRACE_ZCL, "Error: eZHA_RegisterBaseDeviceEndPoint(Switch3): %02x\r\n", eZCL_Status);
    }

    /* Register Light EndPoint 2*/
    eZCL_Status = eZHA_RegisterBaseDeviceEndPoint(ROUTER_SWITCH3_ENDPOINT,
                                                   &APP_ZCL_cbEndpointCallback,
                                                   &sBaseDeviceSwitch3);
    if (eZCL_Status != E_ZCL_SUCCESS)
    {
              DBG_vPrintf(TRACE_ZCL, "Error: eZHA_RegisterBaseDeviceEndPoint(Switch4): %02x\r\n", eZCL_Status);
    }

    APP_vZCL_DeviceSpecific_Init();
    if(u8check)vLoadZCLRecord();
}


/****************************************************************************
 *
 * NAME: APP_ZCL_vSetIdentifyTime
 *
 * DESCRIPTION:
 * Sets the remaining time in the identify cluster
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_ZCL_vSetIdentifyTime(uint16 u16Time)
{
    sBaseDevice.sIdentifyServerCluster.u16IdentifyTime = u16Time;
}


/****************************************************************************
 *
 * NAME: APP_cbTimerZclTick
 *
 * DESCRIPTION:
 * Task kicked by the tick timer
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_cbTimerZclTick(void *pvParam)
{
    static uint32 u32Tick10ms = 9;
    static uint32 u32Tick1Sec = 99;

    tsZCL_CallBackEvent sCallBackEvent;

    if(ZTIMER_eStart(u8TimerZCL, ZTIMER_TIME_MSEC(10)) != E_ZTIMER_OK)
    {
        DBG_vPrintf(TRACE_ZCL, "APP: Failed to Start Tick Timer\n");
    }

    u32Tick10ms++;
    u32Tick1Sec++;

    /* Wrap the Tick10ms counter and provide 100ms ticks to cluster */
    if (u32Tick10ms > 9)
    {
        eZCL_Update100mS();
        u32Tick10ms = 0;
    }
    /* Wrap the 1 second  counter and provide 1Hz ticks to cluster */
    if(u32Tick1Sec > 99)
    {
        u32Tick1Sec = 0;
        sCallBackEvent.pZPSevent = NULL;
        sCallBackEvent.eEventType = E_ZCL_CBET_TIMER;
        vZCL_EventHandler(&sCallBackEvent);
    }
}

/****************************************************************************
 *
 * NAME: APP_ZCL_vEventHandler
 *
 * DESCRIPTION:
 * Main ZCL processing task
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void APP_ZCL_vEventHandler(ZPS_tsAfEvent *psStackEvent)
{
	DBG_vPrintf(TRUE, "\n:::APP_ZCL_vEventHandler:::\n");
    tsZCL_CallBackEvent sCallBackEvent;
    sCallBackEvent.pZPSevent = psStackEvent;

    DBG_vPrintf(TRACE_ZCL, "ZCL_Task endpoint event:%d \n", psStackEvent->eType);
    sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
    vZCL_EventHandler(&sCallBackEvent);

}

PUBLIC uint8 app_u8GetDeviceEndpoint( void)
{
    return ROUTER_APPLICATION_ENDPOINT;
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: APP_ZCL_cbGeneralCallback
 *
 * DESCRIPTION:
 * General callback for ZCL events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent)
{
	DBG_vPrintf(TRUE, "\n:::APP_ZCL_cbGeneralCallback:::\n");
    switch (psEvent->eEventType)
    {

    case E_ZCL_CBET_LOCK_MUTEX:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Lock Mutex\r\n");
        break;

    case E_ZCL_CBET_UNLOCK_MUTEX:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Unlock Mutex\r\n");
        break;

    case E_ZCL_CBET_UNHANDLED_EVENT:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Unhandled Event\r\n");
        break;

    case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Read attributes response");
        break;

    case E_ZCL_CBET_READ_REQUEST:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Read request");
        break;

    case E_ZCL_CBET_DEFAULT_RESPONSE:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Default response");
        break;

    case E_ZCL_CBET_ERROR:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Error");
        break;

    case E_ZCL_CBET_TIMER:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: Timer");
        break;

    case E_ZCL_CBET_ZIGBEE_EVENT:
        DBG_vPrintf(TRACE_ZCL, "\nEVT: ZigBee");
        break;

    case E_ZCL_CBET_CLUSTER_CUSTOM:
        DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Custom");
        break;

    default:
        DBG_vPrintf(TRACE_ZCL, "\nInvalid event type");
        break;

    }

}


/****************************************************************************
 *
 * NAME: APP_ZCL_cbEndpointCallback
 *
 * DESCRIPTION:
 * Endpoint specific callback for ZCL events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_ZCL_cbEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
	DBG_vPrintf(TRUE, "\n:::APP_ZCL_cbEndpointCallback:::\n");
	DBG_vPrintf(TRUE, "\n:::sBaseDevice.sOnOffServerCluster.bOnOff :::\n");
    switch (psEvent->eEventType)
    {
        case E_ZCL_CBET_LOCK_MUTEX:
            break;

        case E_ZCL_CBET_UNLOCK_MUTEX:
            break;

        case E_ZCL_CBET_UNHANDLED_EVENT:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Unhandled event");
            break;

        case E_ZCL_CBET_READ_INDIVIDUAL_ATTRIBUTE_RESPONSE:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Rd Attr Rsp %04x AS %d", psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum,
                psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus);
            break;

        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Read attributes response");
            break;

        case E_ZCL_CBET_READ_REQUEST:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Read request");
            break;

        case E_ZCL_CBET_DEFAULT_RESPONSE:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Default response");
            break;

        case E_ZCL_CBET_ERROR:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Error");
            break;

        case E_ZCL_CBET_TIMER:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Timer");
            break;

        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: ZigBee");
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Custom Cl %04x\n", psEvent->uMessage.sClusterCustomMessage.u16ClusterId);
            APP_vHandleClusterCustomCommands(psEvent);
        break;

        case E_ZCL_CBET_WRITE_INDIVIDUAL_ATTRIBUTE:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Write Individual Attribute Status %02x\n", psEvent->eZCL_Status);
        break;

        case E_ZCL_CBET_REPORT_INDIVIDUAL_ATTRIBUTE:
        {
            tsZCL_IndividualAttributesResponse    *psIndividualAttributeResponse = &psEvent->uMessage.sIndividualAttributeResponse;
            DBG_vPrintf(TRACE_ZCL,"Individual Report attribute for Cluster = %d\n", psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum);
            DBG_vPrintf(TRACE_ZCL,"eAttributeDataType = %d\n", psIndividualAttributeResponse->eAttributeDataType);
            DBG_vPrintf(TRACE_ZCL,"u16AttributeEnum = %d\n", psIndividualAttributeResponse->u16AttributeEnum );
            DBG_vPrintf(TRACE_ZCL,"eAttributeStatus = %d\n", psIndividualAttributeResponse->eAttributeStatus );
        }
            break;

        case E_ZCL_CBET_REPORT_INDIVIDUAL_ATTRIBUTES_CONFIGURE:
        {
            tsZCL_AttributeReportingConfigurationRecord    *psAttributeReportingRecord = &psEvent->uMessage.sAttributeReportingConfigurationRecord;
            DBG_vPrintf(TRACE_ZCL,"Individual Configure Report Cluster %d Attrib %d Type %d Min %d Max %d IntV %d Direcct %d Change %d\n",
                    psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum,
                    psAttributeReportingRecord->u16AttributeEnum,
                    psAttributeReportingRecord->eAttributeDataType,
                    psAttributeReportingRecord->u16MinimumReportingInterval,
                    psAttributeReportingRecord->u16MaximumReportingInterval,
                    psAttributeReportingRecord->u16TimeoutPeriodField,
                    psAttributeReportingRecord->u8DirectionIsReceived,
                    psAttributeReportingRecord->uAttributeReportableChange);

            if (E_ZCL_SUCCESS == psEvent->eZCL_Status)
            {

                vSaveReportableRecord( psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum,
                                       psAttributeReportingRecord);

            }
            /*else if(E_ZCL_RESTORE_DEFAULT_REPORT_CONFIGURATION == psEvent->eZCL_Status)
            {

                vRestoreDefaultRecord(app_u8GetDeviceEndpoint(),
                		              psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum,
                		              psAttributeReportingRecord);
            }*/
        }
        break;

        case E_ZCL_CBET_CLUSTER_UPDATE:
            DBG_vPrintf(TRACE_ZCL, "Update Id %04x\n", psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum);
            APP_vHandleClusterUpdate(psEvent);
            break;
        case E_ZCL_CBET_REPORT_REQUEST:
            break;

        default:
            DBG_vPrintf(TRACE_ZCL, "\nEP EVT: Invalid evt type 0x%x", (uint8)psEvent->eEventType);
            break;
    }
}

/****************************************************************************
 *
 * NAME: APP_vHandleIdentify
 *
 * DESCRIPTION:
 * Application identify handler
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void APP_vHandleIdentify(uint16 u16Time)
{
	DBG_vPrintf(TRUE, "\n:::APP_vHandleIdentify:::\n");
    static bool bActive = FALSE;
    if (u16Time == 0)
    {
            /*
             * Restore to off/off state
             */
    	DBG_vPrintf(TRUE, "\n:::bActive = FALSE:::\n");
        //APP_vSetLed( sBaseDeviceSwitch1.sOnOffServerCluster.bOnOff);
        bActive = FALSE;
    }
    else
    {
        /* Set the Identify levels */
        if (!bActive) {
            bActive = TRUE;
            u8IdentifyCount = 5;
            bIdentifyState = TRUE;
            //APP_vSetLed( TRUE );
        }
    }
}

#ifdef CLD_IDENTIFY_10HZ_TICK
/****************************************************************************
 *
 * NAME: vIdEffectTick
 *
 * DESCRIPTION:
 * ZLO Device Specific identify tick
 *
 * PARAMETER: void
 *
 * RETURNS: void
 *
 ****************************************************************************/
PUBLIC void vIdEffectTick(uint8 u8Endpoint)
{

    if ((u8Endpoint == ROUTER_APPLICATION_ENDPOINT) &&
        (sBaseDeviceSwitch1.sIdentifyServerCluster.u16IdentifyTime > 0 ))
    {
        u8IdentifyCount--;
        if (u8IdentifyCount == 0)
        {
            u8IdentifyCount = 5;
            bIdentifyState = (bIdentifyState)? FALSE: TRUE;
            //APP_vSetLed( bIdentifyState);
        }
    }
}
#endif

/****************************************************************************
 *
 * NAME: APP_vHandleClusterCustomCommands
 *
 * DESCRIPTION:
 * callback for ZCL cluster custom command events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_vHandleClusterCustomCommands(tsZCL_CallBackEvent *psEvent)
{
	DBG_vPrintf(TRUE, "\n:::APP_vHandleClusterCustomCommands : %d:::\n", psEvent->u8EndPoint);
      switch(psEvent->uMessage.sClusterCustomMessage.u16ClusterId)
      {
        case GENERAL_CLUSTER_ID_ONOFF:
        {
        	DBG_vPrintf(TRUE, "\n:::GENERAL_CLUSTER_ID_ONOFF : %d:::\n", sBaseDevice.sOnOffServerCluster.bOnOff);
        	if(psEvent->u8EndPoint == ROUTER_APPLICATION_ENDPOINT){
        		APP_vSetLed3(sBaseDevice.sOnOffServerCluster.bOnOff);
        		APP_vReportStatusButtonImmediately(1);
        	}
        	if(psEvent->u8EndPoint == ROUTER_SWITCH1_ENDPOINT){
        		APP_vSetLed2(sBaseDeviceSwitch1.sOnOffServerCluster.bOnOff);
        		APP_vReportStatusButtonImmediately(2);
        	}
        	if(psEvent->u8EndPoint == ROUTER_SWITCH2_ENDPOINT){
        		APP_vSetLed(sBaseDeviceSwitch2.sOnOffServerCluster.bOnOff);
        		APP_vReportStatusButtonImmediately(3);
        	}
        	if(psEvent->u8EndPoint == ROUTER_SWITCH3_ENDPOINT){
        		APP_vSetLed1(sBaseDeviceSwitch3.sOnOffServerCluster.bOnOff);
        		APP_vReportStatusButtonImmediately(4);
        	}
        }
        break;

        case CLOSURE_CLUSTER_ID_MYSCENES:
        {
        	DBG_vPrintf(TRUE, "\n:::CLOSURE_CLUSTER_ID_MYSCENES:::\n");
        	if(psEvent->u8EndPoint == ROUTER_APPLICATION_ENDPOINT){
        		eSceneID[0] = sBaseDevice.sMyScenesServerCluster.u8MyScenes;
        	}
        	if(psEvent->u8EndPoint == ROUTER_SWITCH1_ENDPOINT){
        		eSceneID[1] = sBaseDeviceSwitch1.sMyScenesServerCluster.u8MyScenes;
        	}
        	if(psEvent->u8EndPoint == ROUTER_SWITCH2_ENDPOINT){
        		eSceneID[2] = sBaseDeviceSwitch2.sMyScenesServerCluster.u8MyScenes;
        	}
        	if(psEvent->u8EndPoint == ROUTER_SWITCH3_ENDPOINT){
        		eSceneID[3] = sBaseDeviceSwitch3.sMyScenesServerCluster.u8MyScenes;
        	}
            PDM_eSaveRecordData( PDM_ID_APP_SCENES,
                                 &eSceneID[0],
                                 sizeof(uint8)*4);
        }
        break;

        case GENERAL_CLUSTER_ID_IDENTIFY:
        {
        	DBG_vPrintf(TRUE, "\n:::GENERAL_CLUSTER_ID_IDENTIFY : %d:::\n", sBaseDevice.sOnOffServerCluster.bOnOff);
            tsCLD_IdentifyCallBackMessage *psCallBackMessage = (tsCLD_IdentifyCallBackMessage*)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
            if (psCallBackMessage->u8CommandId == E_CLD_IDENTIFY_CMD_IDENTIFY)
            {
            	if(psEvent->u8EndPoint == ROUTER_APPLICATION_ENDPOINT){
            		APP_vHandleIdentify(sBaseDevice.sIdentifyServerCluster.u16IdentifyTime);
            	}
            	if(psEvent->u8EndPoint == ROUTER_SWITCH1_ENDPOINT){
            		APP_vHandleIdentify(sBaseDeviceSwitch1.sIdentifyServerCluster.u16IdentifyTime);
            	}
            	if(psEvent->u8EndPoint == ROUTER_SWITCH2_ENDPOINT){
            		APP_vHandleIdentify(sBaseDeviceSwitch2.sIdentifyServerCluster.u16IdentifyTime);
            	}
            	if(psEvent->u8EndPoint == ROUTER_SWITCH3_ENDPOINT){
            		APP_vHandleIdentify(sBaseDeviceSwitch3.sIdentifyServerCluster.u16IdentifyTime);
            	}
            }
        }
        break;

        case GENERAL_CLUSTER_ID_BASIC:
        {
        	DBG_vPrintf(TRUE, "\n:::GENERAL_CLUSTER_ID_BASIC : %d:::\n", sBaseDevice.sOnOffServerCluster.bOnOff);
            tsCLD_BasicCallBackMessage *psCallBackMessage = (tsCLD_BasicCallBackMessage*)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
            if (psCallBackMessage->u8CommandId == E_CLD_BASIC_CMD_RESET_TO_FACTORY_DEFAULTS )
            {
                DBG_vPrintf(TRACE_ZCL, "Basic Factory Reset Received\n");

            	if(psEvent->u8EndPoint == ROUTER_APPLICATION_ENDPOINT){
            		memset(&sBaseDevice,0,sizeof(tsZHA_BaseDevice));
            	}
            	if(psEvent->u8EndPoint == ROUTER_SWITCH1_ENDPOINT){
            		memset(&sBaseDeviceSwitch1,0,sizeof(tsZHA_BaseDevice));
            	}
            	if(psEvent->u8EndPoint == ROUTER_SWITCH2_ENDPOINT){
            	    memset(&sBaseDeviceSwitch2,0,sizeof(tsZHA_BaseDevice));
            	}
            	if(psEvent->u8EndPoint == ROUTER_SWITCH3_ENDPOINT){
            	    memset(&sBaseDeviceSwitch3,0,sizeof(tsZHA_BaseDevice));
            	}
                APP_vZCL_DeviceSpecific_Init();
                eZHA_RegisterBaseDeviceEndPoint(ROUTER_APPLICATION_ENDPOINT,
                                                &APP_ZCL_cbEndpointCallback,
                                                &sBaseDevice);
                eZHA_RegisterBaseDeviceEndPoint(ROUTER_SWITCH1_ENDPOINT,
                                                &APP_ZCL_cbEndpointCallback,
                                                &sBaseDeviceSwitch1);
                eZHA_RegisterBaseDeviceEndPoint(ROUTER_SWITCH2_ENDPOINT,
                                                &APP_ZCL_cbEndpointCallback,
                                                &sBaseDeviceSwitch2);
                eZHA_RegisterBaseDeviceEndPoint(ROUTER_SWITCH3_ENDPOINT,
                                                &APP_ZCL_cbEndpointCallback,
                                                &sBaseDeviceSwitch3);
            }
        }
        break;
    }
}

/****************************************************************************
 *
 * NAME: APP_vHandleClusterUpdate
 *
 * DESCRIPTION:
 * callback for ZCL cluster update events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_vHandleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
	DBG_vPrintf(TRUE, "\n:::APP_vHandleClusterUpdate:::\n");
    if (psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum == GENERAL_CLUSTER_ID_IDENTIFY)
    {
    	if(psEvent->u8EndPoint == ROUTER_APPLICATION_ENDPOINT){
            APP_vHandleIdentify(sBaseDevice.sIdentifyServerCluster.u16IdentifyTime);
            if(sBaseDevice.sIdentifyServerCluster.u16IdentifyTime == 0)
            {
            	DBG_vPrintf(TRUE, "\n:::provide callback to BDB handler for identify on Target 1:::\n");
                tsBDB_ZCLEvent  sBDBZCLEvent;
                /* provide callback to BDB handler for identify on Target */
                sBDBZCLEvent.eType = BDB_E_ZCL_EVENT_IDENTIFY;
                sBDBZCLEvent.psCallBackEvent = psEvent;
                BDB_vZclEventHandler(&sBDBZCLEvent);
            }
    	}
    	if(psEvent->u8EndPoint == ROUTER_SWITCH1_ENDPOINT){
            APP_vHandleIdentify(sBaseDeviceSwitch1.sIdentifyServerCluster.u16IdentifyTime);
            if(sBaseDeviceSwitch1.sIdentifyServerCluster.u16IdentifyTime == 0)
            {
            	DBG_vPrintf(TRUE, "\n:::provide callback to BDB handler for identify on Target 2:::\n");
                tsBDB_ZCLEvent  sBDBZCLEvent;
                /* provide callback to BDB handler for identify on Target */
                sBDBZCLEvent.eType = BDB_E_ZCL_EVENT_IDENTIFY;
                sBDBZCLEvent.psCallBackEvent = psEvent;
                BDB_vZclEventHandler(&sBDBZCLEvent);
            }
    	}
    	if(psEvent->u8EndPoint == ROUTER_SWITCH2_ENDPOINT){
            APP_vHandleIdentify(sBaseDeviceSwitch2.sIdentifyServerCluster.u16IdentifyTime);
            if(sBaseDeviceSwitch2.sIdentifyServerCluster.u16IdentifyTime == 0)
            {
            	DBG_vPrintf(TRUE, "\n:::provide callback to BDB handler for identify on Target 3:::\n");
                tsBDB_ZCLEvent  sBDBZCLEvent;
                /* provide callback to BDB handler for identify on Target */
                sBDBZCLEvent.eType = BDB_E_ZCL_EVENT_IDENTIFY;
                sBDBZCLEvent.psCallBackEvent = psEvent;
                BDB_vZclEventHandler(&sBDBZCLEvent);
            }
    	}
    	if(psEvent->u8EndPoint == ROUTER_SWITCH3_ENDPOINT){
            APP_vHandleIdentify(sBaseDeviceSwitch3.sIdentifyServerCluster.u16IdentifyTime);
            if(sBaseDeviceSwitch3.sIdentifyServerCluster.u16IdentifyTime == 0)
            {
            	DBG_vPrintf(TRUE, "\n:::provide callback to BDB handler for identify on Target 3:::\n");
                tsBDB_ZCLEvent  sBDBZCLEvent;
                /* provide callback to BDB handler for identify on Target */
                sBDBZCLEvent.eType = BDB_E_ZCL_EVENT_IDENTIFY;
                sBDBZCLEvent.psCallBackEvent = psEvent;
                BDB_vZclEventHandler(&sBDBZCLEvent);
            }
    	}
    }
}



/****************************************************************************
 *
 * NAME: APP_vZCL_DeviceSpecific_Init
 *
 * DESCRIPTION:
 * ZCL specific initialization
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void APP_vZCL_DeviceSpecific_Init(void)
{
    sBaseDevice.sOnOffServerCluster.bOnOff = FALSE;
    memcpy(sBaseDevice.sBasicServerCluster.au8ManufacturerName, "DICOM", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBaseDevice.sBasicServerCluster.au8ModelIdentifier, "D-2SW-A4", CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBaseDevice.sBasicServerCluster.au8DateCode, "20150212", CLD_BAS_DATE_SIZE);
    memcpy(sBaseDevice.sBasicServerCluster.au8SWBuildID, "1000-0001", CLD_BAS_SW_BUILD_SIZE);

    sBaseDeviceSwitch1.sOnOffServerCluster.bOnOff = FALSE;
    memcpy(sBaseDeviceSwitch1.sBasicServerCluster.au8ManufacturerName, "DICOM", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBaseDeviceSwitch1.sBasicServerCluster.au8ModelIdentifier, "D-2SW-A4", CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBaseDeviceSwitch1.sBasicServerCluster.au8DateCode, "20170310", CLD_BAS_DATE_SIZE);
    memcpy(sBaseDeviceSwitch1.sBasicServerCluster.au8SWBuildID, "1000-0001", CLD_BAS_SW_BUILD_SIZE);

    sBaseDeviceSwitch2.sOnOffServerCluster.bOnOff = FALSE;
    memcpy(sBaseDeviceSwitch2.sBasicServerCluster.au8ManufacturerName, "DICOM", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBaseDeviceSwitch2.sBasicServerCluster.au8ModelIdentifier, "D-2SW-A4", CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBaseDeviceSwitch2.sBasicServerCluster.au8DateCode, "20190310", CLD_BAS_DATE_SIZE);
    memcpy(sBaseDeviceSwitch2.sBasicServerCluster.au8SWBuildID, "1000-0001", CLD_BAS_SW_BUILD_SIZE);

    sBaseDeviceSwitch3.sOnOffServerCluster.bOnOff = FALSE;
    memcpy(sBaseDeviceSwitch3.sBasicServerCluster.au8ManufacturerName, "DICOM", CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBaseDeviceSwitch3.sBasicServerCluster.au8ModelIdentifier, "D-2SW-A4", CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBaseDeviceSwitch3.sBasicServerCluster.au8DateCode, "20190310", CLD_BAS_DATE_SIZE);
    memcpy(sBaseDeviceSwitch3.sBasicServerCluster.au8SWBuildID, "1000-0001", CLD_BAS_SW_BUILD_SIZE);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
