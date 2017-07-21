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
 *  Header file for Joystick.c.
 */

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

	/* Includes: */
		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <string.h>

		#include "Descriptors.h"

		#include <LUFA/Drivers/USB/USB.h>
		#include <LUFA/Drivers/Board/Joystick.h>
		#include <LUFA/Drivers/Board/LEDs.h>
		#include <LUFA/Drivers/Board/Buttons.h>
		#include <LUFA/Platform/Platform.h>

	/* Macros: */
		/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
		#define LEDMASK_USB_NOTREADY      LEDS_LED1

		/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
		#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

		/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
		#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

		/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
		#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)

		// Mapping of buttons to the state array
		#define MAP_FIREB_BUTTON	0
		#define MAP_FIREY_BUTTON	1
		#define MAP_SELECT_BUTTON	2
		#define MAP_START_BUTTON	3
		#define MAP_UP_BUTTON		4
		#define MAP_DOWN_BUTTON		5
		#define MAP_LEFT_BUTTON		6
		#define MAP_RIGHT_BUTTON	7
		#define MAP_FIREA_BUTTON    8
		#define MAP_FIREX_BUTTON    9
		#define MAP_FIREL_BUTTON    10
		#define MAP_FIRER_BUTTON    11
		
		
		
		
		// The number of physical buttons
		#define NUMBER_OF_BUTTONS		12
	
		// Define the de-bounce tolerance
		#define DEBOUNCE_TOLERANCE		10
		
		// Physical button states
		#define BUTTON_OFF	0
		#define BUTTON_ON	1
	
	/* Type Defines: */
		/** Type define for the joystick HID report structure, for creating and sending HID reports to the host PC.
		 *  This mirrors the layout described to the host in the HID report descriptor, in Descriptors.c.
		 */
		typedef struct
		{
			uint16_t Button; /**< Bit mask of the currently pressed joystick buttons */
			//uint16_t buttonMask; // Bit mask of the currently pressed joystick buttons
			uint8_t  HAT;
			uint8_t  X; /**< Current absolute joystick X position, as a signed 8-bit integer */
			uint8_t  Y; /**< Current absolute joystick Y position, as a signed 8-bit integer */
			uint8_t  Slider; /**< Bit mask of the currently pressed joystick buttons */
			uint8_t  Z; /**< Bit mask of the currently pressed joystick buttons */
			uint8_t  VendorSpec;
		} USB_JoystickReport_Input_t;

		typedef struct
		{
			uint16_t Button; /**< Bit mask of the currently pressed joystick buttons */
			uint8_t  HAT;
			int8_t  X; /**< Current absolute joystick X position, as a signed 8-bit integer */
			int8_t  Y; /**< Current absolute joystick Y position, as a signed 8-bit integer */
			uint8_t  Slider; /**< Bit mask of the currently pressed joystick buttons */
			uint8_t  Z; /**< Bit mask of the currently pressed joystick buttons */
		} USB_JoystickReport_Output_t;

	/* Function Prototypes: */
		void SetupHardware(void);
		void HID_Task(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);

		void readJoystickStates(void);
		void performDebounce(void);
		
		bool GetNextReport(USB_JoystickReport_Input_t* const ReportData, USB_JoystickReport_Input_t* const previousReportData, uint8_t joystickNumber);

#endif

