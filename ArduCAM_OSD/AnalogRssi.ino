/**
 ******************************************************************************
 *
 * @file       AnalogRssi.ino
 * @author     Philippe Vanhaesnedonck
 * @brief      Implements RSSI report on the Ardupilot Mega MinimOSD
 * 	       using built-in ADC reference.
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


#include "AnalogRssi.h"


void analog_rssi_read(void)
{
	//voltage divider scales to 1.1
	//analog reference is 5
	int read = analogRead(RSSI_PIN) * (5/1.1);
	osd_rssi = read * .2 + osd_rssi * .8;	// Smooth input
}
