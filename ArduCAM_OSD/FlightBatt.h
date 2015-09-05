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

#define REF_VOLTAGE			5			
#define LOW_VOLTAGE			9.6			// filter start value for 3s LiPo

//if you change these don't forget to alter #define BATT_VER 1 to force an eeprom write as the runtime values only come from eeprom
#define VOLT_DIV_RATIO            15.55            
#define VOLT_OFFSET               -100
#define CURR_AMP_PER_VOLT		10.98 //attopilot 180 is 18.3mv/A at 3.3 vref but we use 5 vref so 18.3 * 3/5	
#define CURR_AMPS_OFFSET		0.0000	

#define CURRENT_VOLTAGE(x)		(((x)*REF_VOLTAGE/1024.0)-(VOLT_OFFSET/10000.0))*(volt_div_ratio/100.0)
#define CURRENT_AMPS(x)			(((x)*REF_VOLTAGE/1024.0)-(curr_amp_offset/10000.0))*(curr_amp_per_volt/100.0)


void flight_batt_init(void);
void flight_batt_read(void);


#endif /* FLIGHT_BATT_H_ */
