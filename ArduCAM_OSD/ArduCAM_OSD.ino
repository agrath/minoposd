/*

Copyright (c) 2011.  All rights reserved.
An Open Source Arduino based OSD and Camera Control project.

Program  : ArduCAM-OSD (Supports the variant: minimOSD)
Version  : V1.9, 14 February 2012
Author(s): Sandro Benigno
Coauthor(s):
Jani Hirvinen   (All the EEPROM routines)
Michael Oborne  (OSD Configutator)
Mike Smith      (BetterStream and Fast Serial libraries)
Special Contribuitor:
Andrew Tridgell by all the support on MAVLink
Doug Weibel by his great orientation since the start of this project
Contributors: James Goppert, Max Levine
and all other members of DIY Drones Dev team
Thanks to: Chris Anderson, Jordi Munoz


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>

*/

/* ************************************************************ */
/* **************** MAIN PROGRAM - MODULES ******************** */
/* ************************************************************ */

#undef PROGMEM 
#define PROGMEM __attribute__(( section(".progmem.data") )) 

#undef PSTR 
#define PSTR(s) (__extension__({static prog_char __c[] PROGMEM = (s); &__c[0];})) 


/* **********************************************/
/* ***************** INCLUDES *******************/

//#define membug 
//#define FORCEINIT  // You should never use this unless you know what you are doing 


// AVR Includes
#include <FastSerial.h>
#include <AP_Common.h>
#include <AP_Math.h>
#include <math.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
// Get the common arduino functions
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "wiring.h"
#endif
#include <EEPROM.h>
#include <SimpleTimer.h>

#ifdef membug
#include <MemoryFree.h>
#endif

// Configurations
#include "OSD_Config.h"
#include "ArduCam_Max7456.h"
#include "OSD_Vars.h"
#include "OSD_Func.h"

// JRChange: OpenPilot UAVTalk:
#ifdef PROTOCOL_UAVTALK
#include "UAVTalk.h"
#endif
// JRChange: Flight Batt on MinimOSD:
#ifdef FLIGHT_BATT_ON_MINIMOSD
#include "FlightBatt.h"
#endif

// Amedee: Analog RSSI on MinimOSD:
#ifdef ANALOG_RSSI_ON_MINIMOSD
#include "AnalogRssi.h"
#endif

/* *************************************************/
/* ***************** DEFINITIONS *******************/

//OSD Hardware 
//#define ArduCAM328
#define MinimOSD

#define TELEMETRY_SPEED  57600  // How fast our MAVLink telemetry is coming to Serial port

#ifdef USE_WITH_MINRXOSD
#define BOOTTIME         8000   // Time in milliseconds that we show boot loading bar and wait user input
#else
#define BOOTTIME         2000   // Time in milliseconds that we show boot loading bar and wait user input
#endif

// Objects and Serial definitions
FastSerialPort0(Serial);
OSD osd; //OSD object 



/* **********************************************/
/* ***************** SETUP() *******************/

void setup()
{
#ifdef ArduCAM328
	pinMode(10, OUTPUT); // USB ArduCam Only
#endif
	pinMode(MAX7456_SELECT, OUTPUT); // OSD CS

	Serial.begin(TELEMETRY_SPEED);

#ifdef membug
	Serial.println(freeMem());
#endif

	// Prepare OSD for displaying 
	unplugSlaves();
	osd.init();

	// Start 
	startPanels();
	delay(500);

	// OSD debug for development (Shown at start)
#ifdef membug
	osd.setPanel(1, 1);
	osd.openPanel();
	osd.printf("%i", freeMem());
	osd.closePanel();
#endif

	// Just to easy up development things
#ifdef FORCEINIT
	InitializeOSD();
#endif

	// JRChange: Flight Batt on MinimOSD:
	// Check EEPROM to see if we have initialized the battery values already
	//if (readEEPROM(BATT_CHK) != BATT_VER) {
	//writeBattSettings();
	//}

	// Get correct panel settings from EEPROM
	readSettings();
	for (panel = 0; panel < npanels; panel++) readPanelSettings();
	panel = 0; //set panel to 0 to start in the first navigation screen
	// Show bootloader bar
	loadBar();

	// JRChange: Flight Batt on MinimOSD:
//#ifdef FLIGHT_BATT_ON_MINIMOSD
//	flight_batt_init();
//#endif

	analogReference(DEFAULT); //5V vref

	// JRChange: PacketRxOk on MinimOSD:
#ifdef PACKETRXOK_ON_MINIMOSD
	PacketRxOk_init();
#endif

//analogReference gets called above to set the global analog vref
//#ifdef ANALOG_RSSI_ON_MINIMOSD
//	analog_rssi_init();
//#endif

#ifdef USE_WITH_MINRXOSD
	delay(1000);
#endif

	// House cleaning, clear display and enable timers
	osd.clear();

} // END of setup();



/* ***********************************************/
/* ***************** MAIN LOOP *******************/

// Mother of all happenings, The loop()
// As simple as possible.
void loop()
{
	// JRChange: OpenPilot UAVTalk:

	if (uavtalk_read()) {
		OnTick();
	}

}

/* *********************************************** */
/* ******** functions used in main loop() ******** */
void OnTick()			// duration is up to approx. 10ms depending on choosen display features
{

#ifdef FLIGHT_BATT_ON_MINIMOSD
	flight_batt_read();
#endif


#ifdef ANALOG_RSSI_ON_MINIMOSD
	analog_rssi_read();
	rssi = (int16_t)osd_rssi;
	if (!rssiraw_on) rssi = (int16_t)((float)(rssi - rssipersent) / (float)(rssical - rssipersent)*100.0f);
	if (rssi < -99) rssi = -99;
#endif

	writePanels();			// writing enabled panels (check OSD_Panels Tab)
}


void unplugSlaves() {
	//Unplug list of SPI
#ifdef ArduCAM328
	digitalWrite(10, HIGH); // unplug USB HOST: ArduCam Only
#endif
	digitalWrite(MAX7456_SELECT, HIGH); // unplug OSD
}
