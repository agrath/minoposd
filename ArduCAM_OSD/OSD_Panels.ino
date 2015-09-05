//
//
//

/*

refactoring started

TODO:

	refactor:
		switchPanels
		casts (double) -> (float) etc.

	maybe implement usage of panCallsign

*/


#include "OSD_Config.h"

#ifdef FLIGHT_BATT_ON_MINIMOSD
#include "FlightBatt.h"
#endif

#ifdef PACKETRXOK_ON_MINIMOSD
#include "PacketRxOk.h"
#endif


#define PWM_LO			1200	// [us]	PWM low value
#define PWM_HI			1800	// [us]	PWM high value
#define PWM_OFFSET		100	// [us]	PWM offset for detecting stick movement

#define SETUP_TIME		30000	// [ms]	time after boot while we can enter the setup menu
#define SETUP_DEBOUNCE_TIME	500	// [ms]	time for RC-TX stick debouncing
#define SETUP_LOWEST_MENU	1	//	lowest shown setup menue item
#ifndef FLIGHT_BATT_ON_MINIMOSD
#define SETUP_HIGHEST_MENU	2	//	highest shown setup menue item
#else
#define SETUP_HIGHEST_MENU	11	//	highest shown setup menue item
#endif

#define WARN_FLASH_TIME		1000	// [ms]	time with which the warnings are flashing
#define WARN_RECOVER_TIME	4000	// [ms]	time we stay in the first panel after last warning
#ifdef JR_SPECIALS
#define WARN_MAX		6	//	number of implemented warnings
#else
#define WARN_MAX		5	//	number of implemented warnings
#endif

#define MODE_SWITCH_TIME	2000	// [ms]	time for mode switching


/******* GLOBAL VARS *******/

static boolean		setup_menu_active = false;
static boolean		warning_active = false;

static float		convert_speed = 0;
static float		convert_length = 0;
static int16_t		convert_length_large = 0;
static uint8_t		unit_speed = 0;
static uint8_t		unit_length = 0;
static uint8_t		unit_length_large = 0;

static int16_t		chan1_raw_middle = 0;
static int16_t		chan2_raw_middle = 0;


/******* MAIN FUNCTIONS *******/


/******************************************************************/
// Panel  : startPanels
// Output : Logo panel and initialization
/******************************************************************/
void startPanels() {
	osd.clear();
	panLogo();		// display logo
	set_converts();	// initialize the units
}


/******************************************************************/
// Panel  : writePanels
// Output : Write the panels
/******************************************************************/
void writePanels() {

	//#ifdef JR_SPECIALS
	//	// reset osd_home to current position when 2 TX switches are in a special position
	//	if (osd_chan6_raw > PWM_HI && osd_chan7_raw < PWM_LO) {
	//		osd_home_lat = osd_lat;
	//		osd_home_lon = osd_lon;
	//		osd_home_alt = osd_alt;
	//		osd.setPanel(10, 3);
	//		osd.openPanel();
	//		osd.printf_P(PSTR("reset home"));
	//		osd.closePanel();
	//	}
	//	else if (ch_toggle > 3) switchPanels();									// switch panels
	//#else
	//	if (ch_toggle > 3) switchPanels();										// switch panels
	//#endif

		//if (setup_menu_active) {
		//	panSetup();
		//}
		//else {
	if (ISd(0, Warn_BIT)) panWarn(panWarn_XY[0][0], panWarn_XY[1][0]);					// ever check/display warnings
	if (panel < npanels) {											// first or second panel

		if (ISd(panel, RSSI_BIT))		panRSSI(panRSSI_XY[0][panel], panRSSI_XY[1][panel]);
		if (ISa(panel, Rol_BIT))		panRoll(panRoll_XY[0][panel], panRoll_XY[1][panel]);
		if (ISa(panel, Pit_BIT))		panPitch(panPitch_XY[0][panel], panPitch_XY[1][panel]);
		if (ISc(panel, Thr_BIT))		panThr(panThr_XY[0][panel], panThr_XY[1][panel]);
		if (ISc(panel, FMod_BIT))		panFlightMode(panFMod_XY[0][panel], panFMod_XY[1][panel]);
		if (ISa(panel, BatA_BIT))		panBatt_A(panBatt_A_XY[0][panel], panBatt_A_XY[1][panel]);
		if (ISc(panel, CurA_BIT))		panCur_A(panCur_A_XY[0][panel], panCur_A_XY[1][panel]);
		if (ISa(panel, Bp_BIT))			panBatteryPercent(panBatteryPercent_XY[0][panel], panBatteryPercent_XY[1][panel]);
		if (ISe(panel, TEMP_BIT))		panTxPID(panTemp_XY[0][panel], panTemp_XY[1][panel]);
		if (ISb(panel, Time_BIT))		panTime(panTime_XY[0][panel], panTime_XY[1][panel]);

		if (ISc(panel, Hor_BIT))		panHorizon(panHorizon_XY[0][panel], panHorizon_XY[1][panel]);

	}
	else { // off panel
		panOff();
	}
	//}

#ifdef membug
	// OSD debug for development
	osd.setPanel(13, 4);
	osd.openPanel();
	osd.printf("%i", freeMem());
	osd.closePanel();
#endif
}



/******************************************************************/
// Panel  : panOff
// Needs  : -
// Output : -
/******************************************************************/
void panOff(void) {
#ifdef JR_SPECIALS
	// SEARCH GLITCH
	panGPS(panGPS_XY[0][0], panGPS_XY[1][0]);
	panRSSI(panRSSI_XY[0][0], panRSSI_XY[1][0]);
	panGPL(panGPL_XY[0][0], panGPL_XY[1][0]);
	panFlightMode(panFMod_XY[0][0], panFMod_XY[1][0]);
	panThr(panThr_XY[0][0], panThr_XY[1][0]);
#endif
}


/******************************************************************/
// Panel  : panWarn
// Needs  : X, Y locations
// Output : Warnings if there are any
/******************************************************************/
void panWarn(int first_col, int first_line) {
	static char* warning_string;
	static uint8_t last_warning_type = 1;
	static uint8_t warning_type = 0;
	static unsigned long warn_text_timer = 0;
	static unsigned long warn_recover_timer = 0;
	int cycle;

	if (millis() > warn_text_timer) {				// if the text or blank text has been shown for a while
		if (warning_type) {					// there was a warning, so we now blank it out for a while
			last_warning_type = warning_type;			// save the warning type for cycling
			warning_type = 0;
			warning_string = "            ";			// blank the warning
			warn_text_timer = millis() + WARN_FLASH_TIME / 2;	// set clear warning time
		}
		else {
			cycle = last_warning_type;				// start the warning checks cycle where we left it last time
			do {				                // cycle through the warning checks
				if (++cycle > WARN_MAX) cycle = 1;
				switch (cycle) {
				case 1:						// DISARMED
					if (osd_armed < 2) {
						warning_type = cycle;
						warning_string = "  disarmed  ";
					}
					break;
				case 2:						// No telemetry communication
				{
					if (uavtalk_state() != TELEMETRYSTATS_STATE_CONNECTED) {
						warning_type = cycle;
						warning_string = "   no tel   ";
					}
					break;
				}
				case 4:						// BATT LOW
#if defined FLIGHT_BATT_ON_MINIMOSD || defined FLIGHT_BATT_ON_REVO
					if (osd_vbat_A < battv / 10.0) {
#else
					if (osd_vbat_A < float(battv) / 10.0 || osd_battery_remaining_A < batt_warn_level) {
#endif
						warning_type = cycle;
						warning_string = "  batt low  ";
					}
					break;
				case 5:						// RSSI LOW
					if (rssi < rssi_warn_level && rssi != -99 && !rssiraw_on) {
						warning_type = cycle;
						//warning_string = "link quality";
						warning_string = "  rssi low  ";
					}
					break;
#ifdef JR_SPECIALS
				case 6:						// FAILSAFE
					if (osd_chan8_raw > PWM_HI) {
						warning_type = cycle;
						warning_string = "  failsafe  ";
					}
					break;
#endif
				}
			} while (!warning_type && cycle != last_warning_type);
			if (warning_type) {					// if there a warning
				warning_active = true;				// then set warning active
				warn_text_timer = millis() + WARN_FLASH_TIME;	// set show warning time
				warn_recover_timer = millis() + WARN_RECOVER_TIME;
				if (panel > 0) osd.clear();
				panel = 0;					// switch to first panel if there is a warning
			}
			else {						// if not, we do not want the delay, so a new error shows up immediately
				if (millis() > warn_recover_timer) {		// if recover time over since last warning
					warning_active = false;			// no warning active anymore
				}
			}
		}

		osd.setPanel(first_col, first_line);
		osd.openPanel();
		osd.printf("%s", warning_string);
		osd.closePanel();
	}
}


/******************************************************************/
// Panel  : panSetup
// Needs  : Nothing, uses whole screen
// Output : The settings menu
/******************************************************************/
//void panSetup() {
//	static unsigned long setup_debounce_timer = 0;
//	static int8_t setup_menu = 0;
//	int delta = 100;
//
//	if (millis() > setup_debounce_timer) {			// RC-TX stick debouncing
//		setup_debounce_timer = millis() + SETUP_DEBOUNCE_TIME;
//
//		osd.clear();
//		osd.setPanel(5, 3);
//		osd.openPanel();
//
//		osd.printf_P(PSTR("setup screen|||"));
//
//		if (chan1_raw_middle == 0 || chan2_raw_middle == 0) {
//			chan1_raw_middle = chan1_raw;
//			chan2_raw_middle = chan2_raw;
//		}
//
//		if ((chan2_raw - PWM_OFFSET) > chan2_raw_middle) setup_menu++;
//		else if ((chan2_raw + PWM_OFFSET) < chan2_raw_middle) setup_menu--;
//
//		if (setup_menu < SETUP_LOWEST_MENU) setup_menu = SETUP_LOWEST_MENU;
//		else if (setup_menu > SETUP_HIGHEST_MENU) setup_menu = SETUP_HIGHEST_MENU;
//
//		switch (setup_menu) {
//		case 1:
//			osd.printf_P(PSTR("uavtalk "));
//			if (op_uavtalk_mode & UAVTALK_MODE_PASSIVE) {
//				osd.printf_P(PSTR("passive"));
//				if (chan1_raw < chan1_raw_middle - PWM_OFFSET)
//					op_uavtalk_mode &= ~UAVTALK_MODE_PASSIVE;
//			}
//			else {
//				osd.printf_P(PSTR("active "));
//				if (chan1_raw > chan1_raw_middle + PWM_OFFSET)
//					op_uavtalk_mode |= UAVTALK_MODE_PASSIVE;
//			}
//			break;
//		case 2:
//			osd.printf_P(PSTR("battery warning "));
//			osd.printf("%3.1f%c", float(battv) / 10.0, 0x76, 0x20);
//			battv = change_val(battv, battv_ADDR);
//			break;
//#ifdef FLIGHT_BATT_ON_MINIMOSD
//		case 5:
//			delta /= 10;
//		case 4:
//			delta /= 10;
//		case 3:
//			// volt_div_ratio
//			osd.printf_P(PSTR("calibrate||measured volt: "));
//			osd.printf("%c%5.2f%c", 0xE2, (float)osd_vbat_A, 0x8E);
//			osd.printf("||volt div ratio:  %5i", volt_div_ratio);
//			volt_div_ratio = change_int_val(volt_div_ratio, volt_div_ratio_ADDR, delta);
//			break;
//		case 8:
//			delta /= 10;
//		case 7:
//			delta /= 10;
//		case 6:
//			// curr_amp_offset
//			osd.printf_P(PSTR("calibrate||measured amp:  "));
//			osd.printf("%c%5.2f%c", 0xE2, osd_curr_A * .01, 0x8F);
//			osd.printf("||amp offset:      %5i", curr_amp_offset);
//			curr_amp_offset = change_int_val(curr_amp_offset, curr_amp_offset_ADDR, delta);
//			break;
//		case 11:
//			delta /= 10;
//		case 10:
//			delta /= 10;
//		case 9:
//			// curr_amp_per_volt
//			osd.printf_P(PSTR("calibrate||measured amp:  "));
//			osd.printf("%c%5.2f%c", 0xE2, osd_curr_A * .01, 0x8F);
//			osd.printf("||amp per volt:    %5i", curr_amp_per_volt);
//			curr_amp_per_volt = change_int_val(curr_amp_per_volt, curr_amp_per_volt_ADDR, delta);
//			break;
//#endif
//		}
//		osd.closePanel();
//	}
//}


/******* PANELS *******/


/******************************************************************/
// Panel  : panBoot
// Needs  : X, Y locations
// Output : Booting up text and empty bar after that
/******************************************************************/
void panBoot(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf_P(PSTR("booting up:\xed\xf2\xf2\xf2\xf2\xf2\xf2\xf2\xf3"));
	osd.closePanel();
}


/******************************************************************/
// Panel  : panLogo
// Needs  : X, Y locations
// Output : Startup OSD LOGO
/******************************************************************/
void panLogo() {
#ifdef USE_WITH_MINRXOSD
	osd.setPanel(5, 12);
	osd.openPanel();
	VERSION_STRING
#else
	osd.setPanel(3, 5);
	osd.openPanel();
	osd.printf_P(PSTR("\x20\x20\x20\x20\x20\xba\xbb\xbc\xbd\xbe|\x20\x20\x20\x20\x20\xca\xcb\xcc\xcd\xce|"));
	VERSION_STRING
#endif
#ifdef PACKETRXOK_ON_MINIMOSD
		osd.printf_P(PSTR(" prxok"));
#endif
#ifdef ANALOG_RSSI_ON_MINIMOSD
	osd.printf_P(PSTR(" arssi"));
#endif
#ifdef JR_SPECIALS
	osd.printf_P(PSTR(" jrs"));
#endif
	osd.closePanel();
}

/******************************************************************/
// Panel  : panRSSI
// Needs  : X, Y locations
// Output : RSSI %
/******************************************************************/
void panRSSI(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%c%3i%c", 0xE1, rssi, 0x25);
	osd.closePanel();
#ifdef REVO_ADD_ONS
	osd.setPanel(first_col - 2, first_line - 1);
	osd.openPanel();
	osd.printf("%4i%c%3i%c", oplm_rssi, 0x8B, oplm_linkquality, 0x8C);
	osd.closePanel();
#endif
}


/******************************************************************/
// Panel  : panRoll
// Needs  : X, Y locations
// Output : -+ value of current Roll from vehicle with degree symbols and roll symbol
/******************************************************************/
void panRoll(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%4i%c%c", osd_roll, 0xb0, 0xb2);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panPitch
// Needs  : X, Y locations
// Output : -+ value of current Pitch from vehicle with degree symbols and pitch symbol
/******************************************************************/
void panPitch(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%4i%c%c", osd_pitch, 0xb0, 0xb1);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panThr
// Needs  : X, Y locations
// Output : Throttle 
/******************************************************************/
void panThr(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%c%3.0i%c", 0x87, osd_throttle, 0x25);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panFlightMode 
// Needs  : X, Y locations
// Output : current flight modes
/******************************************************************/
void panFlightMode(int first_col, int first_line) {
	char* mode_str = "";

	osd.setPanel(first_col, first_line);
	osd.openPanel();
#if defined VERSION_RELEASE_12_10_1 || defined VERSION_RELEASE_12_10_2 || defined VERSION_RELEASE_13_06_1 || defined VERSION_RELEASE_13_06_2 || defined VERSION_RELEASE_14_01_1
	if (osd_mode == 0) mode_str = "man";	// MANUAL
	else if (osd_mode == 1) mode_str = "st1";	// STABILIZED1
	else if (osd_mode == 2) mode_str = "st2";	// STABILIZED2
	else if (osd_mode == 3) mode_str = "st3";	// STABILIZED3
	else if (osd_mode == 4) mode_str = "at ";	// AUTOTUNE
	else if (osd_mode == 5) mode_str = "alh";	// ALTITUDEHOLD
	else if (osd_mode == 6) mode_str = "alv";	// ALTITUDEVARIO
	else if (osd_mode == 7) mode_str = "vc ";	// VELOCITYCONTROL
	else if (osd_mode == 8) mode_str = "ph ";	// POSITIONHOLD
	else if (osd_mode == 9) mode_str = "rtb";	// RETURNTOBASE
	else if (osd_mode == 10) mode_str = "lan";	// LAND
	else if (osd_mode == 11) mode_str = "pp ";	// PATHPLANNER
	else if (osd_mode == 12) mode_str = "poi";	// POI
#endif
#if defined VERSION_RELEASE_14_06_1 || defined VERSION_RELEASE_14_10_1
	if (osd_mode == 0) mode_str = "man";	// MANUAL
	else if (osd_mode == 1) mode_str = "st1";	// STABILIZED1
	else if (osd_mode == 2) mode_str = "st2";	// STABILIZED2
	else if (osd_mode == 3) mode_str = "st3";	// STABILIZED3
	else if (osd_mode == 4) mode_str = "st4";	// STABILIZED4
	else if (osd_mode == 5) mode_str = "st5";	// STABILIZED5
	else if (osd_mode == 6) mode_str = "st6";	// STABILIZED6
	else if (osd_mode == 7) mode_str = "at ";	// AUTOTUNE
	else if (osd_mode == 8) mode_str = "ph ";	// POSITIONHOLD
	else if (osd_mode == 9) mode_str = "pvf";	// POSITIONVARIOFPV
	else if (osd_mode == 10) mode_str = "pvl";	// POSITIONVARIOLOS
	else if (osd_mode == 11) mode_str = "pvd";	// POSITIONVARIONSEW
	else if (osd_mode == 12) mode_str = "rtb";	// RETURNTOBASE
	else if (osd_mode == 13) mode_str = "lan";	// LAND
	else if (osd_mode == 14) mode_str = "pp ";	// PATHPLANNER
	else if (osd_mode == 15) mode_str = "poi";	// POI
	else if (osd_mode == 16) mode_str = "ac ";	// AUTOCRUISE
#endif
#if defined VERSION_RELEASE_15_01_1 || defined VERSION_RELEASE_15_02_1
	if (osd_mode == 0) mode_str = "man";	// MANUAL
	else if (osd_mode == 1) mode_str = "st1";	// STABILIZED1
	else if (osd_mode == 2) mode_str = "st2";	// STABILIZED2
	else if (osd_mode == 3) mode_str = "st3";	// STABILIZED3
	else if (osd_mode == 4) mode_str = "st4";	// STABILIZED4
	else if (osd_mode == 5) mode_str = "st5";	// STABILIZED5
	else if (osd_mode == 6) mode_str = "st6";	// STABILIZED6
	else if (osd_mode == 7) mode_str = "ph ";	// POSITIONHOLD
	else if (osd_mode == 8) mode_str = "cl ";	// COURSELOCK
	else if (osd_mode == 9) mode_str = "pr ";	// POSITIONROAM
	else if (osd_mode == 10) mode_str = "hl ";	// HOMELEASH
	else if (osd_mode == 11) mode_str = "pa ";	// ABSOLUTEPOSITION
	else if (osd_mode == 12) mode_str = "rtb";	// RETURNTOBASE
	else if (osd_mode == 13) mode_str = "lan";	// LAND
	else if (osd_mode == 14) mode_str = "pp ";	// PATHPLANNER
	else if (osd_mode == 15) mode_str = "poi";	// POI
	else if (osd_mode == 16) mode_str = "ac ";	// AUTOCRUISE
#endif    
#if defined VERSION_RELEASE_15_05
	if (osd_mode == 0) mode_str = "man";	// MANUAL
	else if (osd_mode == 1) mode_str = "st1";	// STABILIZED1
	else if (osd_mode == 2) mode_str = "st2";	// STABILIZED2
	else if (osd_mode == 3) mode_str = "st3";	// STABILIZED3
	else if (osd_mode == 4) mode_str = "st4";	// STABILIZED4
	else if (osd_mode == 5) mode_str = "st5";	// STABILIZED5
	else if (osd_mode == 6) mode_str = "st6";	// STABILIZED6
	else if (osd_mode == 7) mode_str = "ph ";	// POSITIONHOLD
	else if (osd_mode == 8) mode_str = "cl ";	// COURSELOCK
	else if (osd_mode == 9) mode_str = "vr ";	// VELOCITYROAM
	else if (osd_mode == 10) mode_str = "hl ";	// HOMELEASH
	else if (osd_mode == 11) mode_str = "pa ";	// ABSOLUTEPOSITION
	else if (osd_mode == 12) mode_str = "rtb";	// RETURNTOBASE
	else if (osd_mode == 13) mode_str = "lan";	// LAND
	else if (osd_mode == 14) mode_str = "pp ";	// PATHPLANNER
	else if (osd_mode == 15) mode_str = "poi";	// POI
	else if (osd_mode == 16) mode_str = "ac ";	// AUTOCRUISE
	else if (osd_mode == 17) mode_str = "at ";	// AUTOTAKEOFF
#endif
	osd.printf("%c%s", 0xE0, mode_str);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panBattery A (Voltage 1)
// Needs  : X, Y locations
// Output : Voltage value as in XX.X and symbol of over all battery status
/******************************************************************/
void panBatt_A(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%c%5.2f%c", 0xB5, (double)osd_vbat_A, 0x8E);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panCur_A
// Needs  : X, Y locations
// Output : Current
/******************************************************************/
void panCur_A(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%c%5.2f%c", 0xE4, osd_curr_A * .01, 0x8F);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panBatteryPercent
// Needs  : X, Y locations
// Output : Battery
//          (if defined FLIGHT_BATT_ON_MINIMOSD 
/******************************************************************/
void panBatteryPercent(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
#if defined FLIGHT_BATT_ON_MINIMOSD
	if (osd_total_A) {
		/* Battery current consumed */
		osd.printf("%c%5i%c", 0xB9, osd_total_A, 0x82);
	}
	else {
		/* Battery voltage as percent */
		float min_volt = battv / (10.0 * osd_ncells_A);
		float vbat_percent = 0;

		if (osd_ncells_A && osd_vbat_A) {
			vbat_percent = (100 * (osd_vbat_A - min_volt * osd_ncells_A)) /
				(BATT_VCELL_FULL * osd_ncells_A - min_volt * osd_ncells_A);
			vbat_percent = max(0, vbat_percent);
		}

		osd.printf("%c%4.1f%c", 0xB5, vbat_percent, 0x25);
	}
#else
	osd.printf("%c%3.0i%c", 0xB9, osd_battery_remaining_A, 0x25);
#endif
	osd.closePanel();
}


/******************************************************************/
// Panel  : panTxPID
// Needs  : X, Y locations
// Output : Current TxPID settings
/******************************************************************/
void panTxPID(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%1.5f", (double)osd_txpid_cur[0]);
	osd.closePanel();
	osd.setPanel(first_col, first_line + 1);
	osd.openPanel();
	osd.printf("%1.5f", (double)osd_txpid_cur[1]);
	osd.closePanel();
	osd.setPanel(first_col, first_line + 2);
	osd.openPanel();
	osd.printf("%1.5f", (double)osd_txpid_cur[2]);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panTime
// Needs  : X, Y locations
// Output : Time from bootup or start
/******************************************************************/
void panTime(int first_col, int first_line) {
	int start_time;

#ifdef START_TIMER_ON_CURRENT 
	static unsigned long engine_start_time = 0;

	if (engine_start_time == 0 && osd_curr_A > TIME_RESET_AMPERE * 100) {
		engine_start_time = millis();
	}
	start_time = (int)((millis() - engine_start_time) / 1000);
#else
	start_time = (int)(millis() / 1000);
#endif

	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf("%c%2i%c%02i", 0xB3, ((int)(start_time / 60)) % 60, 0x3A, start_time % 60);
	osd.closePanel();
}


/******************************************************************/
// Panel  : panHorizon
// Needs  : X, Y locations
// Output : artificial horizon
/******************************************************************/
void panHorizon(int first_col, int first_line) {
	osd.setPanel(first_col, first_line);
	osd.openPanel();
	osd.printf_P(PSTR("\xc8\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\xc9|"));
	osd.printf_P(PSTR("\xc8\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\xc9|"));
	osd.printf_P(PSTR("\xd8\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\xd9|"));
	osd.printf_P(PSTR("\xc8\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\xc9|"));
	osd.printf_P(PSTR("\xc8\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\xc9"));
	osd.closePanel();
	showHorizon((first_col + 1), first_line);
}

// Setup change function
int change_int_val(int value, int address, int delta) {
	int value_old = value;

	osd.printf_P(PSTR("|                   "));
	switch (delta) {
	case 100:
		osd.printf_P(PSTR("\xBF"));
		break;
	case 10:
		osd.printf_P(PSTR(" \xBF"));
		break;
	case 1:
		osd.printf_P(PSTR("  \xBF"));
		break;
	}

	if (chan1_raw > chan1_raw_middle + PWM_OFFSET) value -= delta;
	else if (chan1_raw < chan1_raw_middle - PWM_OFFSET) value += delta;

	if (value != value_old) {
		EEPROM.write(address, value & 0xff);
		EEPROM.write(address + 1, (value >> 8) & 0xff);
	}
	return value;
}


// Setup change function
int change_val(int value, int address) {
	uint8_t value_old = value;

	if (chan1_raw > chan1_raw_middle + PWM_OFFSET) value--;
	else if (chan1_raw < chan1_raw_middle - PWM_OFFSET) value++;

	if (value != value_old) EEPROM.write(address, value);
	return value;
}


// Show those fancy 2 char arrows
#define ARROW_CODE		(0x90 - 2)		// code of the first MAX7456 special arrow char -2
void showArrow(uint8_t rotate_arrow) {
	rotate_arrow = (rotate_arrow < 2) ? 2 : rotate_arrow * 2;
	osd.printf("%c%c", ARROW_CODE + rotate_arrow, ARROW_CODE + rotate_arrow + 1);
}

#ifdef AH_BETTER_RESOLUTION

// For using this, you must load a special mcm file with the new staggered artificial horizon chars!
// e.g. AH_BetterResolutionCharset002.mcm
							// with different factors we can adapt do different cam optics
#define AH_PITCH_FACTOR		0.010471976		// conversion factor for pitch
#define AH_ROLL_FACTOR		0.017453293		// conversion factor for roll
#define AH_COLS			12			// number of artificial horizon columns
#define AH_ROWS			5			// number of artificial horizon rows
#define CHAR_COLS		12			// number of MAX7456 char columns
#define CHAR_ROWS		18			// number of MAX7456 char rows
#define CHAR_SPECIAL		9			// number of MAX7456 special chars for the artificial horizon
#define AH_TOTAL_LINES		AH_ROWS * CHAR_ROWS	// helper define

#define LINE_SET_STRAIGHT__	(0x06 - 1)		// code of the first MAX7456 straight char -1
#define LINE_SET_STRAIGHT_O	(0x3B - 3)		// code of the first MAX7456 straight overflow char -3
#define LINE_SET_P___STAG_1	(0x3C - 1)		// code of the first MAX7456 positive staggered set 1 char -1
#define LINE_SET_P___STAG_2	(0x45 - 1)		// code of the first MAX7456 positive staggered set 2 char -1
#define LINE_SET_N___STAG_1	(0x4E - 1)		// code of the first MAX7456 negative staggered set 1 char -1
#define LINE_SET_N___STAG_2	(0x57 - 1)		// code of the first MAX7456 negative staggered set 2 char -1
#define LINE_SET_P_O_STAG_1	(0xD4 - 2)		// code of the first MAX7456 positive overflow staggered set 1 char -2
#define LINE_SET_P_O_STAG_2	(0xDA - 1)		// code of the first MAX7456 positive overflow staggered set 2 char -1
#define LINE_SET_N_O_STAG_1	(0xD6 - 2)		// code of the first MAX7456 negative overflow staggered set 1 char -2
#define LINE_SET_N_O_STAG_2	(0xDD - 1)		// code of the first MAX7456 negative overflow staggered set 2 char -1

#define OVERFLOW_CHAR_OFFSET	6			// offset for the overflow subvals

#define ANGLE_1			9			// angle above we switch to line set 1
#define ANGLE_2			25			// angle above we switch to line set 2

// Calculate and show artificial horizon
// used formula: y = m * x + n <=> y = tan(a) * x + n
void showHorizon(int start_col, int start_row) {
	int col, row, pitch_line, middle, hit, subval;
	int roll;
	int line_set = LINE_SET_STRAIGHT__;
	int line_set_overflow = LINE_SET_STRAIGHT_O;
	int subval_overflow = 9;

	// preset the line char attributes
	roll = osd_roll;
	if ((roll >= 0 && roll < 90) || (roll >= -179 && roll < -90)) {	// positive angle line chars
		roll = roll < 0 ? roll + 179 : roll;
		if (abs(roll) > ANGLE_2) {
			line_set = LINE_SET_P___STAG_2;
			line_set_overflow = LINE_SET_P_O_STAG_2;
			subval_overflow = 7;
		}
		else if (abs(roll) > ANGLE_1) {
			line_set = LINE_SET_P___STAG_1;
			line_set_overflow = LINE_SET_P_O_STAG_1;
			subval_overflow = 8;
		}
	}
	else {								// negative angle line chars
		roll = roll > 90 ? roll - 179 : roll;
		if (abs(roll) > ANGLE_2) {
			line_set = LINE_SET_N___STAG_2;
			line_set_overflow = LINE_SET_N_O_STAG_2;
			subval_overflow = 7;
		}
		else if (abs(roll) > ANGLE_1) {
			line_set = LINE_SET_N___STAG_1;
			line_set_overflow = LINE_SET_N_O_STAG_1;
			subval_overflow = 8;
		}
	}

	pitch_line = round(tan(-AH_PITCH_FACTOR * osd_pitch) * AH_TOTAL_LINES) + AH_TOTAL_LINES / 2;	// 90 total lines
	for (col = 1; col <= AH_COLS; col++) {
		middle = col * CHAR_COLS - (AH_COLS / 2 * CHAR_COLS) - CHAR_COLS / 2;	  // -66 to +66	center X point at middle of each column
		hit = tan(AH_ROLL_FACTOR * osd_roll) * middle + pitch_line;	          // 1 to 90	calculating hit point on Y plus offset
		if (hit >= 1 && hit <= AH_TOTAL_LINES) {
			row = (hit - 1) / CHAR_ROWS;						  // 0 to 4 bottom-up
			subval = (hit - (row * CHAR_ROWS) + 1) / (CHAR_ROWS / CHAR_SPECIAL);  // 1 to 9

			// print the line char
			osd.openSingle(start_col + col - 1, start_row + AH_ROWS - row - 1);
			osd.printf("%c", line_set + subval);

			// check if we have to print an overflow line char
			if (subval >= subval_overflow && row < 4) {	// only if it is a char which needs overflow and if it is not the upper most row
				osd.openSingle(start_col + col - 1, start_row + AH_ROWS - row - 2);
				osd.printf("%c", line_set_overflow + subval - OVERFLOW_CHAR_OFFSET);
			}
		}
	}
}

#endif // AH_BETTER_RESOLUTION


void set_converts() {
	if (EEPROM.read(measure_ADDR) == 0) {
		convert_speed = 3.6;
		convert_length = 1.0;
		convert_length_large = 1000;
		unit_speed = 0x81;
		unit_length = 0x8D;
		unit_length_large = 0xFD;
	}
	else {
		convert_speed = 2.23;
		convert_length = 3.28;
		convert_length_large = 5280;
		unit_speed = 0xfb;
		unit_length = 0x66;
		unit_length_large = 0xFA;
	}
}
