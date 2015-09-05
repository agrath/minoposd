/**
 ******************************************************************************
 *
 * @file       FlightBatt.h
 * @author     Joerg-D. Rothfuchs
 * @brief      Implements voltage and current measurement of the flight battery
 * 	       on the Ardupilot Mega MinimOSD using built-in ADC reference.
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
 /*
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  * for more details.
  *
  * You should have received a copy of the GNU General Public License along
  * with this program; if not, see <http://www.gnu.org/licenses/> or write to the
  * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  */


  // !!! For using this, you have to solder a little bit on the MinimOSD, see the wiki !!!


#ifndef FLIGHT_BATT_H_
#define FLIGHT_BATT_H_


#define VOLTAGE_PIN			0
#define CURRENT_PIN			1

#define VOLTAGE_REF_VOLTAGE			1.1			//The maximum read we expect for the configured sensor
#define CURRENT_REF_VOLTAGE			3.3			//The maximum read we expect for the configured sensor
//VREF (analog configured max input) is set to 5v (ArduCAM_OSD.ino call to analogReference)
#define ANALOG_VREF					5
#define LOW_VOLTAGE			9.6			

#define VOLT_DIV_RATIO            70.45            
#define VOLT_OFFSET               -100
#define CURR_MV_PER_AMP		18.3 * (CURRENT_REF_VOLTAGE / ANALOG_VREF)	//DATASHEET VALUE * SENSOR_MAX/VREF
#define CURR_AMPS_OFFSET		-75

#define CURRENT_VOLTAGE(x)		(((x)*VOLTAGE_REF_VOLTAGE/1024.0)-(VOLT_OFFSET/10000))*(VOLT_DIV_RATIO)
#define CURRENT_AMPS(x)			((double)(((x)*CURRENT_REF_VOLTAGE/1024.0))-(CURR_AMPS_OFFSET/10000.0)) / (CURR_MV_PER_AMP/1000)


//void flight_batt_init(void);
void flight_batt_read(void);


#endif /* FLIGHT_BATT_H_ */
