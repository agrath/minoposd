/* ******************************************************************/
/* *********************** GENERAL FUNCTIONS ********************** */

// JRChange: Flight Batt on MinimOSD:
#include "FlightBatt.h"

//Extract functions (get bits from the positioning bytes
#define ISa(panel,whichBit) getBit(panA_REG[panel], whichBit)
#define ISb(panel,whichBit) getBit(panB_REG[panel], whichBit)
#define ISc(panel,whichBit) getBit(panC_REG[panel], whichBit)
#define ISd(panel,whichBit) getBit(panD_REG[panel], whichBit)
#define ISe(panel,whichBit) getBit(panE_REG[panel], whichBit)


boolean getBit(byte Reg, byte whichBit) {
	boolean State;
	State = Reg & (1 << whichBit);
	return State;
}

byte setBit(byte &Reg, byte whichBit, boolean stat) {
	if (stat) {
		Reg = Reg | (1 << whichBit);
	}
	else {
		Reg = Reg & ~(1 << whichBit);
	}
	return Reg;
}

// EEPROM reader/writers
// Utilities for writing and reading from the EEPROM
byte readEEPROM(int address) {

	return EEPROM.read(address);
}

void writeEEPROM(byte value, int address) {
	EEPROM.write(address, value);
}


void readSettings() {
	overspeed = EEPROM.read(overspeed_ADDR);
	stall = EEPROM.read(stall_ADDR);
	battv = EEPROM.read(battv_ADDR);

	switch_mode = EEPROM.read(switch_mode_ADDR);
	//    pal_ntsc = EEPROM.read(pal_ntsc_ADDR);
	//    if (EEPROM.read(ch_toggle_ADDR) < 4 || EEPROM.read(ch_toggle_ADDR) > 8){
	//     	EEPROM.write(ch_toggle_ADDR, 5);
	//	}
	ch_toggle = EEPROM.read(ch_toggle_ADDR);
	//  battp = EEPROM.read(battp_ADDR);
//    if (EEPROM.read(panSetup_en_ADDR) != 1){
//        EEPROM.write(panSetup_en_ADDR, 1);
//    }
	rssical = EEPROM.read(OSD_RSSI_HIGH_ADDR);
	rssipersent = EEPROM.read(OSD_RSSI_LOW_ADDR);
	rssiraw_on = EEPROM.read(OSD_RSSI_RAW_ADDR);

	batt_warn_level = EEPROM.read(OSD_BATT_WARN_ADDR);
	rssi_warn_level = EEPROM.read(OSD_RSSI_WARN_ADDR);

	// JRChange: Flight Batt on MinimOSD:
	/*volt_div_ratio = EEPROM.read(volt_div_ratio_ADDR) + (EEPROM.read(volt_div_ratio_ADDR + 1) << 8);
	curr_amp_per_volt = EEPROM.read(curr_amp_per_volt_ADDR) + (EEPROM.read(curr_amp_per_volt_ADDR + 1) << 8);
	curr_amp_offset = EEPROM.read(curr_amp_offset_ADDR) + (EEPROM.read(curr_amp_offset_ADDR + 1) << 8);*/
}

void readPanelSettings() {

	//****** First set of 8 Panels ******
	uint16_t offset = OffsetBITpanel * panel;

	setBit(panA_REG[panel], Cen_BIT, readEEPROM(panCenter_en_ADDR + offset));
	panCenter_XY[0][panel] = readEEPROM(panCenter_x_ADDR + offset);
	panCenter_XY[1][panel] = checkPAL(readEEPROM(panCenter_y_ADDR + offset));

	setBit(panA_REG[panel], Bp_BIT, readEEPROM(panBatteryPercent_en_ADDR + offset));
	panBatteryPercent_XY[0][panel] = readEEPROM(panBatteryPercent_x_ADDR + offset);
	panBatteryPercent_XY[1][panel] = checkPAL(readEEPROM(panBatteryPercent_y_ADDR + offset));

	setBit(panA_REG[panel], Pit_BIT, readEEPROM(panPitch_en_ADDR + offset));
	panPitch_XY[0][panel] = readEEPROM(panPitch_x_ADDR + offset);
	panPitch_XY[1][panel] = checkPAL(readEEPROM(panPitch_y_ADDR + offset));

	setBit(panA_REG[panel], Rol_BIT, readEEPROM(panRoll_en_ADDR + offset));
	panRoll_XY[0][panel] = readEEPROM(panRoll_x_ADDR + offset);
	panRoll_XY[1][panel] = checkPAL(readEEPROM(panRoll_y_ADDR + offset));

	setBit(panA_REG[panel], BatA_BIT, readEEPROM(panBatt_A_en_ADDR + offset));
	panBatt_A_XY[0][panel] = readEEPROM(panBatt_A_x_ADDR + offset);
	panBatt_A_XY[1][panel] = checkPAL(readEEPROM(panBatt_A_y_ADDR + offset));

	//setBit(panA_REG, BatB_BIT, readEEPROM(panBatt_B_en_ADDR));
	//panBatt_B_XY[0] = readEEPROM(panBatt_B_x_ADDR);
	//panBatt_B_XY[1] = checkPAL(readEEPROM(panBatt_B_y_ADDR));

	setBit(panA_REG[panel], GPSats_BIT, readEEPROM(panGPSats_en_ADDR + offset));
	panGPSats_XY[0][panel] = readEEPROM(panGPSats_x_ADDR + offset);
	panGPSats_XY[1][panel] = checkPAL(readEEPROM(panGPSats_y_ADDR + offset));

	setBit(panA_REG[panel], GPL_BIT, readEEPROM(panGPL_en_ADDR + offset));
	panGPL_XY[0][panel] = readEEPROM(panGPL_x_ADDR + offset);
	panGPL_XY[1][panel] = checkPAL(readEEPROM(panGPL_y_ADDR + offset));

	setBit(panA_REG[panel], GPS_BIT, readEEPROM(panGPS_en_ADDR + offset));
	panGPS_XY[0][panel] = readEEPROM(panGPS_x_ADDR + offset);
	panGPS_XY[1][panel] = checkPAL(readEEPROM(panGPS_y_ADDR + offset));

	//****** Second set of 8 Panels ******

	setBit(panB_REG[panel], Rose_BIT, readEEPROM(panRose_en_ADDR + offset));
	panRose_XY[0][panel] = readEEPROM(panRose_x_ADDR + offset);
	panRose_XY[1][panel] = checkPAL(readEEPROM(panRose_y_ADDR + offset));

	setBit(panB_REG[panel], Head_BIT, readEEPROM(panHeading_en_ADDR + offset));
	panHeading_XY[0][panel] = readEEPROM(panHeading_x_ADDR + offset);
	panHeading_XY[1][panel] = checkPAL(readEEPROM(panHeading_y_ADDR + offset));

	setBit(panB_REG[panel], MavB_BIT, readEEPROM(panMavBeat_en_ADDR + offset));
	panMavBeat_XY[0][panel] = readEEPROM(panMavBeat_x_ADDR + offset);
	panMavBeat_XY[1][panel] = checkPAL(readEEPROM(panMavBeat_y_ADDR + offset));

	setBit(panB_REG[panel], HDis_BIT, readEEPROM(panHomeDis_en_ADDR + offset));
	panHomeDis_XY[0][panel] = readEEPROM(panHomeDis_x_ADDR + offset);
	panHomeDis_XY[1][panel] = checkPAL(readEEPROM(panHomeDis_y_ADDR + offset));

	setBit(panB_REG[panel], HDir_BIT, readEEPROM(panHomeDir_en_ADDR + offset));
	panHomeDir_XY[0][panel] = readEEPROM(panHomeDir_x_ADDR + offset);
	panHomeDir_XY[1][panel] = checkPAL(readEEPROM(panHomeDir_y_ADDR + offset));

	setBit(panB_REG[panel], WDir_BIT, readEEPROM(panWPDir_en_ADDR + offset));
	panWPDir_XY[0][panel] = readEEPROM(panWPDir_x_ADDR + offset);
	panWPDir_XY[1][panel] = checkPAL(readEEPROM(panWPDir_y_ADDR + offset));

	setBit(panB_REG[panel], WDis_BIT, readEEPROM(panWPDis_en_ADDR + offset));
	panWPDis_XY[0][panel] = readEEPROM(panWPDis_x_ADDR + offset);
	panWPDis_XY[1][panel] = checkPAL(readEEPROM(panWPDis_y_ADDR + offset));

	setBit(panB_REG[panel], Time_BIT, readEEPROM(panTime_en_ADDR + offset));
	panTime_XY[0][panel] = readEEPROM(panTime_x_ADDR + offset);
	panTime_XY[1][panel] = checkPAL(readEEPROM(panTime_y_ADDR + offset));

	//setBit(panB_REG, RSSI_BIT, readEEPROM(panRSSI_en_ADDR));
	//panRSSI_XY[0] = readEEPROM(panRSSI_x_ADDR);
	//panRSSI_XY[1] = checkPAL(readEEPROM(panRSSI_y_ADDR));

	//****** Third set of 8 Panels ******

	setBit(panC_REG[panel], CurA_BIT, readEEPROM(panCur_A_en_ADDR + offset));
	panCur_A_XY[0][panel] = readEEPROM(panCur_A_x_ADDR + offset);
	panCur_A_XY[1][panel] = checkPAL(readEEPROM(panCur_A_y_ADDR + offset));

	//setBit(panC_REG, CurB_BIT, readEEPROM(panCur_B_en_ADDR));
	//panCur_B_XY[0] = readEEPROM(panCur_B_x_ADDR);
	//panCur_B_XY[1] = checkPAL(readEEPROM(panCur_B_y_ADDR));

	setBit(panC_REG[panel], Alt_BIT, readEEPROM(panAlt_en_ADDR + offset));
	panAlt_XY[0][panel] = readEEPROM(panAlt_x_ADDR + offset);
	panAlt_XY[1][panel] = checkPAL(readEEPROM(panAlt_y_ADDR + offset));

	setBit(panC_REG[panel], Halt_BIT, readEEPROM(panHomeAlt_en_ADDR + offset));
	panHomeAlt_XY[0][panel] = readEEPROM(panHomeAlt_x_ADDR + offset);
	panHomeAlt_XY[1][panel] = checkPAL(readEEPROM(panHomeAlt_y_ADDR + offset));

	setBit(panC_REG[panel], As_BIT, readEEPROM(panAirSpeed_en_ADDR + offset));
	panAirSpeed_XY[0][panel] = readEEPROM(panAirSpeed_x_ADDR + offset);
	panAirSpeed_XY[1][panel] = checkPAL(readEEPROM(panAirSpeed_y_ADDR + offset));

	setBit(panC_REG[panel], Vel_BIT, readEEPROM(panVel_en_ADDR + offset));
	panVel_XY[0][panel] = readEEPROM(panVel_x_ADDR + offset);
	panVel_XY[1][panel] = checkPAL(readEEPROM(panVel_y_ADDR + offset));

	setBit(panC_REG[panel], Thr_BIT, readEEPROM(panThr_en_ADDR + offset));
	panThr_XY[0][panel] = readEEPROM(panThr_x_ADDR + offset);
	panThr_XY[1][panel] = checkPAL(readEEPROM(panThr_y_ADDR + offset));

	setBit(panC_REG[panel], FMod_BIT, readEEPROM(panFMod_en_ADDR + offset));
	panFMod_XY[0][panel] = readEEPROM(panFMod_x_ADDR + offset);
	panFMod_XY[1][panel] = checkPAL(readEEPROM(panFMod_y_ADDR + offset));

	setBit(panC_REG[panel], Hor_BIT, readEEPROM(panHorizon_en_ADDR + offset));
	panHorizon_XY[0][panel] = readEEPROM(panHorizon_x_ADDR + offset);
	panHorizon_XY[1][panel] = checkPAL(readEEPROM(panHorizon_y_ADDR + offset));

	setBit(panD_REG[panel], Warn_BIT, readEEPROM(panWarn_en_ADDR + offset));
	panWarn_XY[0][panel] = readEEPROM(panWarn_x_ADDR + offset);
	panWarn_XY[1][panel] = checkPAL(readEEPROM(panWarn_y_ADDR + offset));

	//setBit(panD_REG[panel], Off_BIT, readEEPROM(panOff_en_ADDR + offset));
	//panOff_XY[0] = readEEPROM(panOff_x_ADDR + offset);
	//panOff_XY[1] = checkPAL(readEEPROM(panOff_y_ADDR + offset));

	setBit(panD_REG[panel], WindS_BIT, readEEPROM(panWindSpeed_en_ADDR + offset));
	panWindSpeed_XY[0][panel] = readEEPROM(panWindSpeed_x_ADDR + offset);
	panWindSpeed_XY[1][panel] = checkPAL(readEEPROM(panWindSpeed_y_ADDR + offset));

	setBit(panD_REG[panel], Climb_BIT, readEEPROM(panClimb_en_ADDR + offset));
	panClimb_XY[0][panel] = readEEPROM(panClimb_x_ADDR + offset);
	panClimb_XY[1][panel] = checkPAL(readEEPROM(panClimb_y_ADDR + offset));

	setBit(panD_REG[panel], Tune_BIT, readEEPROM(panTune_en_ADDR + offset));
	panTune_XY[0][panel] = readEEPROM(panTune_x_ADDR + offset);
	panTune_XY[1][panel] = checkPAL(readEEPROM(panTune_y_ADDR + offset));

	//setBit(panD_REG[panel], Setup_BIT, readEEPROM(panSetup_en_ADDR));
	//panSetup_XY[0] = readEEPROM(panSetup_x_ADDR);
	//panSetup_XY[1] = checkPAL(readEEPROM(panSetup_y_ADDR));

	setBit(panD_REG[panel], RSSI_BIT, readEEPROM(panRSSI_en_ADDR + offset));
	panRSSI_XY[0][panel] = readEEPROM(panRSSI_x_ADDR + offset);
	panRSSI_XY[1][panel] = checkPAL(readEEPROM(panRSSI_y_ADDR + offset));

	setBit(panE_REG[panel], DIST_BIT, readEEPROM(panDistance_en_ADDR + offset));
	panDistance_XY[0][panel] = readEEPROM(panDistance_x_ADDR + offset);
	panDistance_XY[1][panel] = checkPAL(readEEPROM(panDistance_y_ADDR + offset));

	setBit(panE_REG[panel], TEMP_BIT, readEEPROM(panTemp_en_ADDR + offset));
	panTemp_XY[0][panel] = readEEPROM(panTemp_x_ADDR + offset);
	panTemp_XY[1][panel] = checkPAL(readEEPROM(panTemp_y_ADDR + offset));
}

int checkPAL(int line) {
	if (line >= osd.getCenter() && osd.getMode() == 0) {
		line -= 3;//Cutting lines offset after center if NTSC
	}
	return line;
}

void updateSettings(byte panelu, byte panel_x, byte panel_y, byte panel_s) {
	if (panel >= 1 && panel <= 32) {

		writeEEPROM(panel_s, (6 * panelu) - 6 + 0);
		if (panel_s != 0) {
			writeEEPROM(panel_x, (6 * panelu) - 6 + 2);
			writeEEPROM(panel_y, (6 * panelu) - 6 + 4);
		}
		osd.clear();
		readSettings();
		for (panel = 0; panel < npanels; panel++) readPanelSettings();
	}
}


//// JRChange: Flight Batt on MinimOSD:
//void writeBattSettings() {
//	EEPROM.write(BATT_CHK, BATT_VER);
//	EEPROM.write(volt_div_ratio_ADDR,    (int)(VOLT_DIV_RATIO*100.0)&0xff);
//	EEPROM.write(volt_div_ratio_ADDR+1, ((int)(VOLT_DIV_RATIO*100.0)>>8)&0xff);
//	EEPROM.write(curr_amp_per_volt_ADDR,    (int)(CURR_AMP_PER_VOLT*100.0)&0xff);
//	EEPROM.write(curr_amp_per_volt_ADDR+1, ((int)(CURR_AMP_PER_VOLT*100.0)>>8)&0xff);
//	EEPROM.write(curr_amp_offset_ADDR,    (int)(CURR_AMPS_OFFSET*10000.0)&0xff);
//	EEPROM.write(curr_amp_offset_ADDR+1, ((int)(CURR_AMPS_OFFSET*10000.0)>>8)&0xff);
//}
