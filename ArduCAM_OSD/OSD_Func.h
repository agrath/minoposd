

//------------------ Battery Remaining Picture ----------------------------------

char setBatteryPic(uint16_t bat_level)
{
	if (bat_level <= 100) {
		return 0xb4;
	}
	else if (bat_level <= 300) {
		return 0xb5;
	}
	else if (bat_level <= 400) {
		return 0xb6;
	}
	else if (bat_level <= 500) {
		return 0xb7;
	}
	else if (bat_level <= 800) {
		return 0xb8;
	}
	else return 0xb9;
}
