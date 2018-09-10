#include <stdint.h>
#include <stddef.h>

// Compute the MODBUS RTU CRC
uint16_t ec_crc(uint8_t[] buf, int len){
	
	uint16_t crc = 0xFFFF;

	for (int pos = 0; pos < len; pos++) {
		crc ^= (uint16_t)buf[pos];
		for (int i = 8; i != 0; i--) {
			if ((crc & 0x0001) != 0){
				crc >>= 1;
				crc ^= 0xA001;
			}else{
				crc >>= 1;
			}
		}
	}
	
	return crc;
}
