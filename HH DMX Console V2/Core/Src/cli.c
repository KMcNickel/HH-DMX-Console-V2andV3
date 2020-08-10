/*
 * cli.c
 *
 *  Created on: Apr 28, 2020
 *      Author: Kyle
 */

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"
#include "main.h"
#include "cli.h"
#include "oled.h"
#include "usbd_cdc_if.h"

enum CommandSectionCompleteStatus
{
    CLI_COMMAND_SECTION_NEW,
    CLI_COMMAND_SECTION_CHANNEL,
    CLI_COMMAND_SECTION_CHANNEL_ADD,
    CLI_COMMAND_SECTION_CHANNEL_REMOVE,
    CLI_COMMAND_SECTION_CHANNEL_THRU,
    CLI_COMMAND_SECTION_OFFSET,
    CLI_COMMAND_SECTION_OFFSET_ENTERED,
    CLI_COMMAND_SECTION_VALUE,
    CLI_COMMAND_SECTION_VALUE_ENTERED,
    CLI_COMMAND_SECTION_VALUE_INCREMENT,
    CLI_COMMAND_SECTION_VALUE_INCREMENT_ENTERED,
    CLI_COMMAND_SECTION_VALUE_DECREMENT,
    CLI_COMMAND_SECTION_VALUE_DECREMENT_ENTERED,
    CLI_COMMAND_SECTION_VALUE_THRU,
    CLI_COMMAND_SECTION_VALUE_THRU_ENTERED,
    CLI_COMMAND_SECTION_TIME,
    CLI_COMMAND_SECTION_TIME_ENTERED,
    CLI_COMMAND_SECTION_RECORD,
    CLI_COMMAND_SECTION_RECORD_ENTERED,
    CLI_COMMAND_SECTION_PLAYBACK,
    CLI_COMMAND_SECTION_PLAYBACK_ENTERED,
	CLI_COMMAND_SECTION_PLAYBACK_TIME,
	CLI_COMMAND_SECTION_PLAYBACK_TIME_ENTERED,
    CLI_COMMAND_SECTION_COMPLETE,
    CLI_COMMAND_COMPLETE,
    CLI_COMMAND_SECTION_ERROR
};

enum commandActionType
{
    CLI_ACTION_SET_CHANNEL_VALUES,
    CLI_ACTION_RECORD_PRESET,
    CLI_ACTION_PLAY_PRESET,
	CLI_ACTION_HIGHLIGHT
};

struct CLI_Data {
    uint16_t command[CLI_MAX_ITEMS];
    uint8_t counter;
    uint16_t chLow;
    uint16_t chHigh;
    uint8_t* values;
    uint16_t highlightedChannel;
    uint8_t highlightedReturnValue;
};

struct CLI_Data cliData;

uint8_t presetData[CLI_PRESET_COUNT][512];
int16_t fadeCoefficient[512];
uint16_t fadeTracker[512];
uint16_t fadeWaitTicks;
uint16_t fadeWaitTracker;
uint8_t newLine[] = "\r\n";
extern TIM_HandleTypeDef htim17;

void CLI_AddItem(uint16_t function)
    {
        bool counterWasZero = cliData.counter == 0;
        if(function > CLI_COMMAND_START)
        {
            cliData.command[cliData.counter] = function;
            cliData.counter++;
        }
        else
        {
            if(counterWasZero)
                cliData.counter++;

            uint16_t result = (cliData.command[cliData.counter - 1] * 10)
                    + function;
            if(result > CLI_COMMAND_START)
            {
                cliData.command[cliData.counter] = function;
                cliData.counter++;
            }
            else
            {
                cliData.command[cliData.counter - 1] = result;
            }
        }
    }

    void CLI_RemoveLastItem()
    {
        if(cliData.counter == 0)
            return;
        if(cliData.command[cliData.counter - 1] > CLI_COMMAND_START)
        {
            cliData.command[cliData.counter - 1] = 0;
            cliData.counter--;
        }
        else
        {
            cliData.command[cliData.counter - 1] =
                    cliData.command[cliData.counter - 1] / 10;
            if(cliData.command[cliData.counter - 1] == 0)
                cliData.counter--;
        }
    }

    void CLI_ClearHighlight()
    {
		*(cliData.values + cliData.highlightedChannel) = cliData.highlightedReturnValue;
		cliData.highlightedChannel = cliData.highlightedReturnValue = 0;
		OLED_String("              ", 14, 0, 0);
		OLED_DrawScreen();
    }

    void CLI_Clear()
    {
    	if(cliData.highlightedChannel != 0)
    		CLI_ClearHighlight();
        for(cliData.counter = 0;
                cliData.counter < CLI_MAX_ITEMS; cliData.counter++)
            cliData.command[cliData.counter] = 0;
        cliData.counter = 0;
    }

    bool CLI_ProcessCommand()
    {
        uint16_t i, j;
        bool secActiveChannels[513];
        bool activeChannels[513];
        uint8_t chVals[513];
        bool useTXActiveChannels = false;
        int16_t thruPrefix = CLI_COMMAND_START;
        uint8_t lowVal = 0;
        uint8_t highVal = 0;
        int16_t valIncDec = 0;
        uint16_t offset = 1;
        uint8_t time = 0;
        uint8_t presetNum = 0;
        enum commandActionType cmdAction = CLI_ACTION_SET_CHANNEL_VALUES;
        enum CommandSectionCompleteStatus cmdStatus = CLI_COMMAND_SECTION_NEW;

        for(i = 0; i < cliData.counter; i++)
        {
            switch(cmdStatus)
            {
                case CLI_COMMAND_SECTION_NEW:
                    for(j = 0; j < 513; j++)
                    {
                        secActiveChannels[j] = false;
                        activeChannels[j] = false;
                        chVals[j] = 0;
                    }

                    lowVal = 0;
                    highVal = 0;
                    offset = 1;
                    time = 0;
                    presetNum = 0;
                    thruPrefix = CLI_COMMAND_START;
                    useTXActiveChannels = false;
                    valIncDec = 0;
                    cmdAction = CLI_ACTION_SET_CHANNEL_VALUES;
                    switch(cliData.command[i])
                    {
                        case BtnRecord:
                            cmdStatus = CLI_COMMAND_SECTION_RECORD;
                            break;
                        case BtnPreset:
                            cmdStatus = CLI_COMMAND_SECTION_PLAYBACK;
                            break;
                        case BtnMinus:
                            useTXActiveChannels = true;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_DECREMENT;
                            break;
                        case BtnPlus:
                            useTXActiveChannels = true;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_INCREMENT;
                            break;
                        case BtnLast:
                        	if(cliData.highlightedChannel == 0)
                        		cliData.highlightedChannel = 512;
							else
							{
								activeChannels[cliData.highlightedChannel] = true;
								chVals[cliData.highlightedChannel] = cliData.highlightedReturnValue;
								if(cliData.highlightedChannel == 1)
									cliData.highlightedChannel = 512;
								else cliData.highlightedChannel--;

							}
							activeChannels[cliData.highlightedChannel] = true;
							chVals[cliData.highlightedChannel] = 255;
							cliData.highlightedReturnValue = *(cliData.values + cliData.highlightedChannel);
							cmdAction = CLI_ACTION_HIGHLIGHT;
							cmdStatus = CLI_COMMAND_COMPLETE;
                        	break;
                        case BtnNext:
                        	if(cliData.highlightedChannel == 0)
                        		cliData.highlightedChannel = 1;
                        	else
                        	{
                        		activeChannels[cliData.highlightedChannel] = true;
								chVals[cliData.highlightedChannel] = cliData.highlightedReturnValue;
                        		if(cliData.highlightedChannel == 512)
                        			cliData.highlightedChannel = 1;
                        		else cliData.highlightedChannel++;

                        	}
                    		activeChannels[cliData.highlightedChannel] = true;
							chVals[cliData.highlightedChannel] = 255;
                        	cliData.highlightedReturnValue = *(cliData.values + cliData.highlightedChannel);
                        	cmdAction = CLI_ACTION_HIGHLIGHT;
                        	cmdStatus = CLI_COMMAND_COMPLETE;
                        	break;
                        case SWITCH_VALID_CHANNELS:
                            thruPrefix = cliData.command[i];
                            secActiveChannels[cliData.command[i]] = true;
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                            break;
                        case BtnThru:
                            thruPrefix = 1;
                            secActiveChannels[1] = true;
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_THRU;
                            break;
                        case BtnAt:
                            useTXActiveChannels = true;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE;
                            break;
                        case BtnFull:
                            useTXActiveChannels = true;
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        case BtnEnter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;

                    }
                    break;
                case CLI_COMMAND_SECTION_CHANNEL:
                    switch(cliData.command[i])
                    {
                        case BtnOffset:
                            cmdStatus = CLI_COMMAND_SECTION_OFFSET;
                            break;
                        case BtnMinus:
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_REMOVE;
                            break;
                        case BtnPlus:
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_ADD;
                            break;
                        case BtnLast:
                        case BtnNext:
                        	if(cliData.highlightedChannel != 0)
							{
								activeChannels[cliData.highlightedChannel] = true;
								chVals[cliData.highlightedChannel] = cliData.highlightedReturnValue;
							}
                        	cliData.highlightedChannel = cliData.command[i - 1];
							activeChannels[cliData.highlightedChannel] = true;
							chVals[cliData.highlightedChannel] = 255;
							cliData.highlightedReturnValue = *(cliData.values + cliData.highlightedChannel);
							cmdAction = CLI_ACTION_HIGHLIGHT;
							cmdStatus = CLI_COMMAND_COMPLETE;
                        	break;
                        case BtnThru:
                            cmdStatus = CLI_COMMAND_SECTION_CHANNEL_THRU;
                            break;
                        case BtnAt:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE;
                            break;
                        case BtnFull:
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_CHANNEL_ADD:
                    if(cliData.command[i] > 0 && cliData.command[i] < 513)
                    {
                        thruPrefix = cliData.command[i];
                        secActiveChannels[cliData.command[i]] = true;
                        cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                    }
                    else
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    break;
                case CLI_COMMAND_SECTION_CHANNEL_REMOVE:
                    if(cliData.command[i] > 0 && cliData.command[i] < 513)
                    {
                        thruPrefix = cliData.command[i] * -1;
                        secActiveChannels[cliData.command[i]] = false;
                        cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                    }
                    else
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    break;
                case CLI_COMMAND_SECTION_CHANNEL_THRU:
                    if(thruPrefix == CLI_COMMAND_START)
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    else
                    {
                        switch(cliData.command[i])
                        {
                            case BtnOffset:
                                for(j = thruPrefix; j <= 512; j++)
                                {
                                    secActiveChannels[j] = true;
                                }
                                cmdStatus = CLI_COMMAND_SECTION_OFFSET;
                                break;
                            case SWITCH_VALID_CHANNELS:
                                if(thruPrefix > cliData.command[i])
                                    for(j = cliData.command[i]; j <= thruPrefix; j++)
                                    {
                                        secActiveChannels[j] = true;
                                    }
                                else
                                    for(j = thruPrefix; j <= cliData.command[i]; j++)
                                    {
                                        secActiveChannels[j] = true;
                                    }
                                cmdStatus = CLI_COMMAND_SECTION_CHANNEL;
                                break;
                            case BtnAt:
                                for(j = thruPrefix; j <= 512; j++)
                                {
                                    secActiveChannels[j] = true;
                                }
                                cmdStatus = CLI_COMMAND_SECTION_VALUE;
                                break;
                            case BtnFull:
                                for(j = thruPrefix; j <= 512; j++)
                                {
                                    secActiveChannels[j] = true;
                                }
                                lowVal = 255;
                                highVal = 255;
                                cmdStatus = CLI_COMMAND_COMPLETE;
                                break;
                            default:
                                cmdStatus = CLI_COMMAND_SECTION_ERROR;
                                break;
                        }
                    }
                    break;
                case CLI_COMMAND_SECTION_OFFSET:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_CHANNELS:
                            offset = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_OFFSET_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_OFFSET_ENTERED:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    switch(cliData.command[i])
                    {
                        case BtnAt:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE;
                            break;
                        case BtnFull:
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;

                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PERCENT_VALUES:
                            lowVal = highVal = cliData.command[i] * 2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_ENTERED;
                            break;
                        case BtnMinus:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_DECREMENT;
                            break;
                        case BtnPlus:
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_INCREMENT;
                            break;
                        case BtnThru:
                            //LowVal will already be set
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_THRU;
                            break;
                        case BtnFull:
                            lowVal = 255;
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_ENTERED:
                    switch(cliData.command[i])
                    {
                        case BtnTime:
                            cmdStatus = CLI_COMMAND_SECTION_TIME;
                            break;
                        case BtnPlus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case BtnThru:
                            //LowVal will already be set
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_THRU;
                            break;
                        case BtnEnter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_INCREMENT:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PERCENT_VALUES:
                            valIncDec = cliData.command[i] * 2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_INCREMENT_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_INCREMENT_ENTERED:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    switch(cliData.command[i])
                    {
                        case BtnPlus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case BtnEnter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_DECREMENT:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PERCENT_VALUES:
                            valIncDec = cliData.command[i] * -2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_DECREMENT_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_DECREMENT_ENTERED:
                    cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    switch(cliData.command[i])
                    {
                        case BtnPlus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case BtnEnter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_THRU:
                    switch(cliData.command[i])
                    {
                        case BtnTime:
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_SECTION_TIME;
                            break;
                        case BtnPlus:
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case SWITCH_VALID_PERCENT_VALUES:
                            if(cliData.command[i] > lowVal)
                                highVal = cliData.command[i] * 2.55;
                            else
                                lowVal = cliData.command[i] * 2.55;
                            cmdStatus = CLI_COMMAND_SECTION_VALUE_THRU_ENTERED;
                            break;
                        case BtnFull:
                        case BtnEnter:
                            highVal = 255;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_VALUE_THRU_ENTERED:
                    switch(cliData.command[i])
                    {
                        case BtnTime:
                            cmdStatus = CLI_COMMAND_SECTION_TIME;
                            break;
                        case BtnPlus:
                            cmdStatus = CLI_COMMAND_SECTION_COMPLETE;
                            break;
                        case BtnEnter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_TIME:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_TIME:
                            time = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_TIME_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_TIME_ENTERED:
                    switch(cliData.command[i])
                    {
                        case BtnEnter:
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_RECORD:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PRESETS:
                            presetNum = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_RECORD_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_RECORD_ENTERED:
                    switch(cliData.command[i])
                    {
                        case BtnEnter:
                            cmdAction = CLI_ACTION_RECORD_PRESET;
                            cmdStatus = CLI_COMMAND_COMPLETE;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                case CLI_COMMAND_SECTION_PLAYBACK:
                    switch(cliData.command[i])
                    {
                        case SWITCH_VALID_PRESETS:
                            presetNum = cliData.command[i];
                            cmdStatus = CLI_COMMAND_SECTION_PLAYBACK_ENTERED;
                            break;
                        default:
                            cmdStatus = CLI_COMMAND_SECTION_ERROR;
                            break;
                    }
                    break;
                    case CLI_COMMAND_SECTION_PLAYBACK_ENTERED:
                        switch(cliData.command[i])
                        {
                        	case BtnTime:
                        		cmdStatus = CLI_COMMAND_SECTION_PLAYBACK_TIME;
                        		break;
                            case BtnEnter:
                                cmdAction = CLI_ACTION_PLAY_PRESET;
                                cmdStatus = CLI_COMMAND_COMPLETE;
                                break;
                            default:
                                cmdStatus = CLI_COMMAND_SECTION_ERROR;
                                break;
                        }
                        break;
					case CLI_COMMAND_SECTION_PLAYBACK_TIME:
						switch(cliData.command[i])
						{
							case SWITCH_VALID_TIME:
								time = cliData.command[i];
								cmdStatus = CLI_COMMAND_SECTION_PLAYBACK_TIME_ENTERED;
								break;
							default:
								cmdStatus = CLI_COMMAND_SECTION_ERROR;
								break;
						}
						break;
					case CLI_COMMAND_SECTION_PLAYBACK_TIME_ENTERED:
						switch(cliData.command[i])
						{
							case BtnEnter:
								cmdAction = CLI_ACTION_PLAY_PRESET;
								cmdStatus = CLI_COMMAND_COMPLETE;
								break;
							default:
								cmdStatus = CLI_COMMAND_SECTION_ERROR;
								break;
						}
						break;
                case CLI_COMMAND_SECTION_COMPLETE:
                case CLI_COMMAND_COMPLETE:
                case CLI_COMMAND_SECTION_ERROR:
                    //All of this is done after the switch
                    break;
            }

            if(cmdStatus == CLI_COMMAND_SECTION_COMPLETE || cmdStatus == CLI_COMMAND_COMPLETE)
            {
                if(cmdAction == CLI_ACTION_SET_CHANNEL_VALUES)
                {
                    uint32_t fanAmount;
                    uint16_t k = 0, l = offset, secActiveChannelCount = 0;
                    if((lowVal || highVal) && valIncDec)    //Setting literal values AND Inc/Decrementing
                        cmdStatus = CLI_COMMAND_SECTION_ERROR;
                    else
                    {
                        if(useTXActiveChannels)
                        {
                            for(j = 1; j < 513; j++)
                            {
                                if(*(cliData.values + j))
                                {
                                    secActiveChannels[j] = true;
                                    if(!activeChannels[j])
                                        chVals[j] = *(cliData.values + j);
                                }
                            }
                        }
                        for(j = 1; j < 513; j++)
                        {
                            if(secActiveChannels[j])
                            {
                                if(l == offset)
                                {
                                    activeChannels[j] = true;
                                    secActiveChannelCount++;
                                    l = 0;
                                }
                                l++;
                            }
                        }
                        if(valIncDec == 0)
                        {
                        	k = 1;
                            if(secActiveChannelCount == 1 || secActiveChannelCount == 0)
                                fanAmount = 0;
                            else
                            {
                                fanAmount = ((highVal - lowVal) * 10000) / (secActiveChannelCount - 1);
                            }
                            for(j = 0; j < 513; j++)
                            {
                                if(secActiveChannels[j])
                                {
                                    chVals[j] = lowVal + ((fanAmount * k) / 10000);
                                    k++;
                                }
                            }
                        }
                        else
                        {
                            for(j = 0; j < 513; j++)
                            {
                                if(secActiveChannels[j])
                                {
                                	if((int16_t) *(cliData.values + j) + valIncDec > 255)
										chVals[j] = 255;
                                	else if((int16_t) *(cliData.values + j) + valIncDec < 0)
										chVals[j] = 0;
                                	else chVals[j] = *(cliData.values + j) + valIncDec;
                                }
                            }
                        }
                    }
                }
            }
            if(cmdStatus == CLI_COMMAND_COMPLETE ||
                    cmdStatus == CLI_COMMAND_SECTION_ERROR)
                break;
        }



        if(cmdStatus == CLI_COMMAND_SECTION_ERROR)
            return false;

        if(cmdStatus == CLI_COMMAND_COMPLETE)
        {
        	HAL_TIM_Base_Stop_IT(&htim17);

        	if(cmdAction != CLI_ACTION_HIGHLIGHT && cliData.highlightedChannel != 0)
        	{
        		activeChannels[cliData.highlightedChannel] = true;
        		chVals[cliData.highlightedChannel] = cliData.highlightedReturnValue;
        		cliData.highlightedChannel = cliData.highlightedReturnValue = 0;
				OLED_String("              ", 14, 0, 0);
				OLED_DrawScreen();
        	}
        	if(cmdAction == CLI_ACTION_HIGHLIGHT)
        	{
        		cmdAction = CLI_ACTION_SET_CHANNEL_VALUES;
        		char channnelNum[15];
				sprintf(channnelNum, "Highlight: %-3d", cliData.highlightedChannel);
				OLED_String(channnelNum, 14, 0, 0);
				OLED_DrawScreen();
        	}

            if(time == 0)
            {
            	if(fadeWaitTicks != 0)
            	{
                	OLED_String("        ", 8, 0, 0);
        			OLED_DrawScreen();
            	}
            	fadeWaitTicks = 0;
                switch(cmdAction)
                {
                    case CLI_ACTION_SET_CHANNEL_VALUES:
                        for(j = 1; j < 513; j++)
                        {
                            if(activeChannels[j])
                                *(cliData.values + j) = chVals[j];
                        }
                        break;
                    case CLI_ACTION_PLAY_PRESET:
                        for(j = 1; j < 513; j++)
                        {
                            *(cliData.values + j) = presetData[presetNum - 1][j - 1];
                        }
                        break;
                    case CLI_ACTION_RECORD_PRESET:
                        for(j = 1; j < 513; j++)
                        {
                            uint8_t val = *(cliData.values + j);
                            presetData[presetNum - 1][j - 1] = val;
                        }
                        break;
                    default:
                        break;
                }
            }
            else
            {
                fadeWaitTracker = 0;
                fadeWaitTicks = time;
                switch(cmdAction)
                {
                    case CLI_ACTION_SET_CHANNEL_VALUES:
                        for(j = 1; j < 513; j++)
                        {
                            fadeTracker[j - 1] = *(cliData.values + j) * CLI_TICKS_PER_SECOND;
                            if(!activeChannels[j])
                                fadeCoefficient[j - 1] = 0;
                            else
                                fadeCoefficient[j - 1] =
                                        chVals[j] - *(cliData.values + j);
                        }
                        break;
                    case CLI_ACTION_PLAY_PRESET:
                        for(j = 1; j < 513; j++)
                        {
                            fadeTracker[j - 1] = *(cliData.values + j) * CLI_TICKS_PER_SECOND;
                            fadeCoefficient[j - 1] =
                                    presetData[presetNum - 1][j - 1] -  *(cliData.values + j);
                        }
                        break;
                    default:
                        break;
                }
                char secsLeft[9];
				sprintf(secsLeft, "Fade: %-2d", fadeWaitTicks);
				OLED_String(secsLeft, 8, 0, 0);
				OLED_DrawScreen();
                HAL_TIM_Base_Start_IT(&htim17);
            }
        }
        return true;
    }

    char* CLI_FunctionToString(uint16_t function, char* str)
    {
        switch(function)
        {
            case BtnThru:
                strcpy(str, "Thru");
                break;
            case BtnAt:
                strcpy(str, "At");
                break;
            case BtnFull:
                strcpy(str, "Full *");
                break;
            case BtnEnter:
                strcpy(str, "*");
                break;
            case BtnBksp:
                strcpy(str, "Bksp");
                break;
            case BtnClear:
                strcpy(str, "Clear");
                break;
            case BtnPlus:
                strcpy(str, "+");
                break;
            case BtnMinus:
                strcpy(str, "-");
                break;
            case BtnLast:
                strcpy(str, "Last *");
                break;
            case BtnNext:
                strcpy(str, "Next *");
                break;
            case BtnRecord:
                strcpy(str, "Record");
                break;
            case BtnPreset:
                strcpy(str, "Preset");
                break;
            case BtnOffset:
                strcpy(str, "Offset");
                break;
            case BtnTime:
                strcpy(str, "Time");
                break;
            default:
                sprintf(str, "%d", function);
                break;
        }
        return str;
    }

    void CLI_PrintCommand()
    {
        char string[64] = "";
        uint8_t i, j = 0;

        for(i = 0; i < cliData.counter; i++)
        {
            CLI_FunctionToString(cliData.command[i], string + strlen(string));
            j = strlen(string);
            string[j] = ' ';
            string[j + 1] = '\0';
        }
        string[j] = '\0';

        CDC_Transmit_FS((uint8_t*) string, strlen(string));

        OLED_ClearLine(1);
        OLED_ClearLine(2);
        OLED_ClearLine(3);
        OLED_StringAutoLine(string, strlen(string), 0, 1, 3);
        OLED_DrawScreen();
    }

    void CLI_PrintError (uint16_t function)
    {
        OLED_ClearLine(3);
        char string[15] = "Invalid Key: \"";
        OLED_String(string, 14, 0, 3);
        CLI_FunctionToString(function, string);
        OLED_String(string, strlen(string), 84, 3);
        OLED_String("\"", 1, 84 + (strlen(string) * 6), 3);
        OLED_DrawPage(3);
    }

    void CLI_Timer_Callback ()
    {
        uint16_t j;
        fadeWaitTracker++;
        if(!(fadeWaitTracker % fadeWaitTicks))
        {
            for(j = 1; j < 513; j++)
            {
                fadeTracker[j - 1] += fadeCoefficient[j - 1];
                *(cliData.values + j) = fadeTracker[j - 1] >> 7;
            }
        }
        if(fadeWaitTracker == fadeWaitTicks * CLI_TICKS_PER_SECOND)
        {
        	HAL_TIM_Base_Stop_IT(&htim17);
        	OLED_String("        ", 8, 0, 0);
			OLED_DrawScreen();
        }
        else
        {
        	if(!(fadeWaitTracker % 128))
			{
				char secsLeft[9];
				sprintf(secsLeft, "Fade: %-2d", fadeWaitTicks - (fadeWaitTracker / CLI_TICKS_PER_SECOND));
				OLED_String(secsLeft, 8, 0, 0);
				OLED_DrawScreen();
			}
        }
    }



    void CLI_Init(uint8_t* dmxBuf)
        {
            for(cliData.counter = 0; cliData.counter < 5; cliData.counter++)
                cliData.command[cliData.counter] = 0;
            cliData.counter = 0;
            cliData.chLow = 1;
            cliData.chHigh = 512;
            cliData.values = dmxBuf;
        }

        void CLI_AddToCommand(uint16_t function)
        {
            if(function == BtnClear)
            {
                CLI_Clear();
                CLI_PrintCommand();
            }
            else if(function == BtnBksp)
            {
                CLI_RemoveLastItem();
                CLI_PrintCommand();
            }
            else if(function == BtnFull || function == BtnLast || function == BtnNext || function == BtnEnter)
            {
                CLI_AddItem(function);
                if(CLI_ProcessCommand())
                {
                    CLI_PrintCommand();
                    cliData.counter = 0;
                    cliData.command[0] = 0;
                }
                else
                {
                	CLI_RemoveLastItem();
                	CLI_PrintError(function);
                }
            }
            else
            {
                if(cliData.counter != CLI_MAX_ITEMS)
                {
                    CLI_AddItem(function);
                    if(CLI_ProcessCommand())
                        CLI_PrintCommand();
                    else
                    {
                        CLI_RemoveLastItem();
                        CLI_PrintError(function);
                    }
                }
                else
                    CLI_PrintError(function);
            }
        }
