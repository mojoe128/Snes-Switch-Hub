/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the Joystick demo. This file contains the main tasks of the demo and
 *  is responsible for the initial application hardware configuration.
 */

#include "Joystick.h"
// Array for storing the physical button state, de-bounce counter and current de-bounced button state

struct buttonState
{
	uint8_t physicalState; // On or off
	uint8_t state; // On or off
	uint8_t debounceCount;
};

// Array for storing the physical state of all four joysticks
struct joystickState
{
	struct buttonState button[NUMBER_OF_BUTTONS];
} joyStick[4];

// Global for storing the previously sent reports for each joystick
USB_JoystickReport_Input_t previousJoystickReportData0;
USB_JoystickReport_Input_t previousJoystickReportData1;
USB_JoystickReport_Input_t previousJoystickReportData2;
USB_JoystickReport_Input_t previousJoystickReportData3;

/*** Button Mappings ****
The Pokken controller exposes 13 buttons, of which only 10 have physical
controls available. The Switch is fairly loose regarding the use of
descriptors, and can be expanded to a full 16 buttons (at least).

Of these 16 buttons, the Switch has 14 of them physically available: the four
face buttons, the four shoulder buttons, -/+, the stick clicks, Home, and
Capture (in that order). A curious thought to explore would be to see if we
can go beyond two bytes; along with the HAT, the Switch Pro Controller also
has an additional nibble of buttons available (which may correspond to Up,
Down, Left and Right as specific buttons instead of a 'directional unit').
**** Button Mappings ***/
#define PAD_Y       0x01
#define PAD_B       0x02
#define PAD_A       0x04
#define PAD_X       0x08
#define PAD_L       0x10
#define PAD_R       0x20
#define PAD_ZL      0x40
#define PAD_ZR      0x80
#define PAD_SELECT  0x100 
#define PAD_START   0x200
#define PAD_LS      0x400
#define PAD_RS      0x800
#define PAD_HOME    0x1000
#define PAD_CAPTURE 0x2000

// The following buttons map to buttons within Pokken, as most of this code was originally used for my Pokken fightstick.
#define POKKEN_PKMNMOVE PAD_A
#define POKKEN_JUMP     PAD_B
#define POKKEN_HOMING   PAD_X
#define POKKEN_LONGDIST PAD_Y
#define POKKEN_SUPPORT  PAD_L
#define POKKEN_GUARD    PAD_R

#define POKKEN_COUNTER (PAD_X | PAD_A)
#define POKKEN_GRAB    (PAD_Y | PAD_B)
#define POKKEN_BURST   (PAD_L | PAD_R)

/*
The following ButtonMap variable defines all possible buttons within the
original 13 bits of space, along with attempting to investigate the remaining
3 bits that are 'unused'. This is what led to finding that the 'Capture'
button was operational on the stick.
*/
uint16_t ButtonMap[16] = {
	0x01,
	0x02,
	0x04,
	0x08,
	0x10,
	0x20,
	0x40,
	0x80,
	0x100,
	0x200,
	0x400,
	0x800,
	0x1000,
	0x2000,
	0x4000,
	0x8000,
};




/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();
	GlobalInterruptEnable();

	while (1)
	{
		readJoystickStates();
		// Perform button and joystick debouncing
		performDebounce();
		
		HID_Task();
		USB_USBTask();
		
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
	
	DDRD  &= ~0xFF;
	PORTD |=  0xFF;

	
	
	//data 3 
	DDRB  &= ~((1 << 2));
	PORTB |=  (1 << 2);
	//data 2 
	DDRB  &= ~((1 << 5));
	PORTB |=  (1 << 5);
	//data 1 
	DDRF  &= ~((1 << 4));
	PORTF |=  (1 << 4);
	//data 0
	DDRF  &= ~((1 << 5));
	PORTF |=  (1 << 5);
	
	// clock 
	DDRF  |= (1 << 7);
	PORTF &= ~((1 << 7));
	
	//latch
	DDRF  |= (1 << 6);
	PORTF &= ~((1 << 6));
	
	
	
	/* Hardware Initialization */
	USB_Init();
}

// Read the joystick button states for all 4 joysticks
void readJoystickStates(void)
{
// Set joystick latch low
	PORTF &= ~(1 << 6);
	
	for (uint16_t buttonNumber = 0; buttonNumber < NUMBER_OF_BUTTONS; buttonNumber++)
	{
		// Set joystick clock low
		_delay_us(6);
		PORTF &= ~(1 << 7);
		
		// Read the data pin state for all joysticks
		if (!(PINF & (1 << 5))) joyStick[0].button[buttonNumber].physicalState = BUTTON_OFF;
		else joyStick[0].button[buttonNumber].physicalState = BUTTON_ON;
		
		if (!(PINF & (1 << 4))) joyStick[1].button[buttonNumber].physicalState = BUTTON_OFF;
		else joyStick[1].button[buttonNumber].physicalState = BUTTON_ON;
		
		if (!(PINB & (1 << 5))) joyStick[2].button[buttonNumber].physicalState = BUTTON_OFF;
		else joyStick[2].button[buttonNumber].physicalState = BUTTON_ON;
		
		if (!(PINB & (1 << 2))) joyStick[3].button[buttonNumber].physicalState = BUTTON_OFF;
		else joyStick[3].button[buttonNumber].physicalState = BUTTON_ON;
		
		
		
		// Set joystick clock high
		_delay_us(6);
		PORTF |= (1 << 7);
	}
	
	// Set joystick latch high
	PORTF |= (1 << 6);
	
	//end	
	
}

// Debounce buttons and joysticks on and off to improve joystick feedback
void performDebounce(void)
{
	for (uint8_t joystickNumber = 0; joystickNumber < 4; joystickNumber++)
	{
		// De-bounce all buttons on and off
		for (uint16_t buttonNumber = 0; buttonNumber < NUMBER_OF_BUTTONS; buttonNumber++)
		{
			// De-bounce on
			if (joyStick[joystickNumber].button[buttonNumber].physicalState ==
				BUTTON_ON && joyStick[joystickNumber].button[buttonNumber].state == BUTTON_OFF)
			{
				// If the de-bounce tolerance is met change state otherwise
				// increment the de-bounce counter
				if (joyStick[joystickNumber].button[buttonNumber].debounceCount > DEBOUNCE_TOLERANCE)
				joyStick[joystickNumber].button[buttonNumber].state = BUTTON_ON;
				else joyStick[joystickNumber].button[buttonNumber].debounceCount++;
			}
			// De-bounce off
			if (joyStick[joystickNumber].button[buttonNumber].physicalState
				== BUTTON_OFF && joyStick[joystickNumber].button[buttonNumber].state == BUTTON_ON)
			{			
				// If the de-bounce tolerance is met change state otherwise
				// increment the de-bounce counter
				if (joyStick[joystickNumber].button[buttonNumber].debounceCount > DEBOUNCE_TOLERANCE)
				joyStick[joystickNumber].button[buttonNumber].state = BUTTON_OFF;
				else joyStick[joystickNumber].button[buttonNumber].debounceCount++;
			}
			// Reset de-bounce counter
			if (joyStick[joystickNumber].button[buttonNumber].physicalState ==
				joyStick[joystickNumber].button[buttonNumber].state)
				joyStick[joystickNumber].button[buttonNumber].debounceCount = 0;
		}
	}		
}


/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Device_Connect(void)
{
	/* Indicate USB enumerating */
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the USB management and joystick reporting tasks.
 */
void EVENT_USB_Device_Disconnect(void)
{
	/* Indicate USB not ready */
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured and the joystick reporting task started.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup HID Report Endpoint */
	//ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	//ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	
	ConfigSuccess &=Endpoint_ConfigureEndpoint(JOYSTICK0_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &=Endpoint_ConfigureEndpoint(JOYSTICK1_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &=Endpoint_ConfigureEndpoint(JOYSTICK2_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &=Endpoint_ConfigureEndpoint(JOYSTICK3_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	/* Indicate endpoint configuration success or failure */
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
	/* Handle HID Class specific requests */
	switch (USB_ControlRequest.bRequest)
	{
		case HID_REQ_GetReport:
			if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
			{
				USB_JoystickReport_Input_t JoystickReportData;

			// Check which joystick the control request refers to:
				switch (USB_ControlRequest.wIndex)
				{
					case 0x00:
						// Create the next joystick0 HID report to send to the host
						GetNextReport(&JoystickReportData, &previousJoystickReportData0, 0);

						Endpoint_ClearSETUP();

						// Write the joystick0 report data to the control endpoint
						Endpoint_Write_Control_Stream_LE(&JoystickReportData, sizeof(JoystickReportData));
						Endpoint_ClearOUT();
						break;

					case 0x01:
						// Create the next joystick1 HID report to send to the host
						GetNextReport(&JoystickReportData, &previousJoystickReportData1,  1);

						Endpoint_ClearSETUP();

						// Write the joystick1 report data to the control endpoint
						Endpoint_Write_Control_Stream_LE(&JoystickReportData, sizeof(JoystickReportData));
						Endpoint_ClearOUT();
						break;
					case 0x02:
						// Create the next joystick1 HID report to send to the host
						GetNextReport(&JoystickReportData, &previousJoystickReportData2,  2);

						Endpoint_ClearSETUP();

						// Write the joystick1 report data to the control endpoint
						Endpoint_Write_Control_Stream_LE(&JoystickReportData, sizeof(JoystickReportData));
						Endpoint_ClearOUT();
						break;	
					case 0x03:
						// Create the next joystick1 HID report to send to the host
						GetNextReport(&JoystickReportData, &previousJoystickReportData3,  3);

						Endpoint_ClearSETUP();

						// Write the joystick1 report data to the control endpoint
						Endpoint_Write_Control_Stream_LE(&JoystickReportData, sizeof(JoystickReportData));
						Endpoint_ClearOUT();
						break;
				}				
			}

			break;	
	}
}

/** Fills the given HID report data structure with the next HID report to send to the host.
 *
 *  \param[out] ReportData  Pointer to a HID report data structure to be filled
 *
 *  \return Boolean \c true if the new report differs from the last report, \c false otherwise
 */
bool GetNextReport(USB_JoystickReport_Input_t* const ReportData, USB_JoystickReport_Input_t* const previousReportData, uint8_t joystickNumber)
{
	
	bool inputChanged = false;
	
	/* Clear the report contents */
	memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
	
	//turn off unused joystick axis
	ReportData->Slider = 128;
	ReportData->Z = 128;
	ReportData->HAT = 0xff;
	
	
	// Set the joystick button status
	if (!(joyStick[joystickNumber].button[MAP_FIREB_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[1];
	if (!(joyStick[joystickNumber].button[MAP_FIREY_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[0];
	if (!(joyStick[joystickNumber].button[MAP_SELECT_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[12];
	if (!(joyStick[joystickNumber].button[MAP_START_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[9];
	
	//if (!(joyStick[joystickNumber].button[MAP_DOWN_BUTTON].state == BUTTON_ON))			ReportData->HAT = 0x01;
	
	//if (!(joyStick[joystickNumber].button[MAP_DOWN_BUTTON].state == BUTTON_ON))			ReportData->HAT = 0x04;
	
	//if (!(joyStick[joystickNumber].button[MAP_LEFT_BUTTON].state == BUTTON_ON))			ReportData->HAT = 0x06;
	
	//if (!(joyStick[joystickNumber].button[MAP_RIGHT_BUTTON].state == BUTTON_ON))			ReportData->HAT = 0x02;
	
	if (!(joyStick[joystickNumber].button[MAP_FIREA_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[2];
	if (!(joyStick[joystickNumber].button[MAP_FIREX_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[3];
	if (!(joyStick[joystickNumber].button[MAP_FIREL_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[4];
	if (!(joyStick[joystickNumber].button[MAP_FIRER_BUTTON].state == BUTTON_ON))	ReportData->Button |= ButtonMap[5];
	
	
	// Set the left joystick direction states
	if (!(joyStick[joystickNumber].button[MAP_UP_BUTTON].state == BUTTON_ON))			ReportData->Y = 0;
	else if (!(joyStick[joystickNumber].button[MAP_DOWN_BUTTON].state == BUTTON_ON))	ReportData->Y = 255;
	else ReportData->Y = 128;
	
	if (!(joyStick[joystickNumber].button[MAP_LEFT_BUTTON].state == BUTTON_ON))		ReportData->X = 0;
	else if (!(joyStick[joystickNumber].button[MAP_RIGHT_BUTTON].state == BUTTON_ON))	ReportData->X = 255;
	else ReportData->X = 128;
	
	/*
	// Set the right joystick direction states
	if (!(joyStick[joystickNumber].button[MAP_UP_BUTTON].state == BUTTON_ON))			ReportData->Z = 0;
	else if (!(joyStick[joystickNumber].button[MAP_DOWN_BUTTON].state == BUTTON_ON))	ReportData->Z = 255;
	else ReportData->Z = 128;
	
	if (!(joyStick[joystickNumber].button[MAP_LEFT_BUTTON].state == BUTTON_ON))		ReportData->Slider = 0;
	else if (!(joyStick[joystickNumber].button[MAP_RIGHT_BUTTON].state == BUTTON_ON))	ReportData->Slider = 255;
	else ReportData->Slider = 128;
	*/
	
	// Check to see if the joystick state has changed since the last report was sent
	if (ReportData->Button != previousReportData->Button) inputChanged = true;
	if (ReportData->X != previousReportData->X) inputChanged = true;
	if (ReportData->Y != previousReportData->Y) inputChanged = true;
	
	// Save the current joystick status for later comparison
	memcpy(previousReportData, ReportData, sizeof(USB_JoystickReport_Input_t));
	
	return inputChanged;
	
	/*
	if(!(PINF & (1 << 4))) {
	// button press
	ReportData->Button |= ButtonMap[12];
	ReportData->Slider = 255;
	}
	*/
	
	
	
}

/** Function to manage HID report generation and transmission to the host. */
void HID_Task(void)
{
	
	// Device must be connected and configured for the task to run
		if (USB_DeviceState != DEVICE_STATE_Configured)
		  return;
	
	// Select the Joystick 0 Report Endpoint
	Endpoint_SelectEndpoint(JOYSTICK0_EPADDR);

	// Check to see if the host is ready for another packet
	if (Endpoint_IsINReady())
	{
		USB_JoystickReport_Input_t JoystickReportData0;

		/* Create the next HID report to send to the host */
		GetNextReport(&JoystickReportData0,&previousJoystickReportData0, 0);

		/* Write Joystick Report Data */
		Endpoint_Write_Stream_LE(&JoystickReportData0, sizeof(JoystickReportData0), NULL);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();

		/* Clear the report data afterwards */
		memset(&JoystickReportData0, 0, sizeof(JoystickReportData0));
	}
	
	// Select the Joystick 1 Report Endpoint
	Endpoint_SelectEndpoint(JOYSTICK1_EPADDR);
	
	// Check to see if the host is ready for another packet
	if (Endpoint_IsINReady())
	{
		USB_JoystickReport_Input_t JoystickReportData1;

		/* Create the next HID report to send to the host */
		GetNextReport(&JoystickReportData1,&previousJoystickReportData1, 1);

		/* Write Joystick Report Data */
		Endpoint_Write_Stream_LE(&JoystickReportData1, sizeof(JoystickReportData1), NULL);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();

		/* Clear the report data afterwards */
		memset(&JoystickReportData1, 0, sizeof(JoystickReportData1));
	}
	// Select the Joystick 1 Report Endpoint
	Endpoint_SelectEndpoint(JOYSTICK2_EPADDR);
	
	// Check to see if the host is ready for another packet
	if (Endpoint_IsINReady())
	{
		USB_JoystickReport_Input_t JoystickReportData2;

		/* Create the next HID report to send to the host */
		GetNextReport(&JoystickReportData2,&previousJoystickReportData2, 2);

		/* Write Joystick Report Data */
		Endpoint_Write_Stream_LE(&JoystickReportData2, sizeof(JoystickReportData2), NULL);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();

		/* Clear the report data afterwards */
		memset(&JoystickReportData2, 0, sizeof(JoystickReportData2));
	}
	// Select the Joystick 1 Report Endpoint
	Endpoint_SelectEndpoint(JOYSTICK3_EPADDR);
	
	// Check to see if the host is ready for another packet
	if (Endpoint_IsINReady())
	{
		USB_JoystickReport_Input_t JoystickReportData3;

		/* Create the next HID report to send to the host */
		GetNextReport(&JoystickReportData3,&previousJoystickReportData3, 3);

		/* Write Joystick Report Data */
		Endpoint_Write_Stream_LE(&JoystickReportData3, sizeof(JoystickReportData3), NULL);

		/* Finalize the stream transfer to send the last packet */
		Endpoint_ClearIN();

		/* Clear the report data afterwards */
		memset(&JoystickReportData3, 0, sizeof(JoystickReportData3));
	}
}


