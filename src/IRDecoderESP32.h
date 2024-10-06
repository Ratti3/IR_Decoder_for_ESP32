/*                                                     https://oshwlab.com/ratti3
  _|_|_|                _|      _|      _|  _|_|_|     https://youtube.com/@Ratti3
  _|    _|    _|_|_|  _|_|_|_|_|_|_|_|            _|   https://projecthub.arduino.cc/Ratti3
  _|_|_|    _|    _|    _|      _|      _|    _|_|     https://ratti3.blogspot.com
  _|    _|  _|    _|    _|      _|      _|        _|   https://hackaday.io/Ratti3
  _|    _|    _|_|_|      _|_|    _|_|  _|  _|_|_|     https://www.hackster.io/Ratti3
													   https://github.com/Ratti3

Code forked from https://github.com/Freenove/Freenove_IR_Lib_for_ESP32

This file is part of https://github.com/Ratti3/IR_Decoder_for_ESP32

IR_Decoder_for_ESP32 is free software: you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
IR_Decoder_for_ESP32 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with IR_Decoder_for_ESP32. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _IRDECODERESP32_h
#define _IRDECODERESP32_h

#include <Arduino.h>

#include "driver/rmt_rx.h"
#include "driver/rmt_encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RMT_FREQ     1000000 
#define RMT_MEM_RX   RMT_MEM_NUM_BLOCKS_3

typedef struct {
  uint16_t header_high;
  uint16_t header_low;
  uint16_t one_high;
  uint16_t one_low;
  uint16_t zero_high;
  uint16_t zero_low;
  uint16_t footer_high;
  uint8_t footer_low;
  uint16_t frequency;
  const char *name;
} ir_protocol_t;

typedef struct {
  uint8_t     ir_state;
  uint32_t    ir_data;
  uint8_t     ir_type;
} ir_recv_t;

enum ir_protocol_type {
	UNK,
	NEC,
	NEC_REP,
	SONY,
	SAM,
	RC5,
	PROTO_COUNT 
};
	
class IRDecoderESP32
{
public:
	IRDecoderESP32(uint8_t pin);
	virtual ~IRDecoderESP32(void);
	
	bool         begin(uint8_t pin);
	bool         end(uint8_t pin);
	bool         available(void);
	bool         nec_available(void);
	bool         sony_available(void);
	bool         sam_available(void);
	bool         rc5_available(void);
	const char*  protocol(void);
	uint32_t     data(void);
	
	void         task(void);
	
private:
	rmt_data_t            ir_data[64];
	int                   ir_rx_pin;
	rmt_ch_dir_t          ir_ch_dir;
	rmt_reserve_memsize_t ir_memsize;
	uint32_t              ir_frequency;
	ir_recv_t             ir_recv_data;
	const uint16_t         bitMargin = 300;

	const ir_protocol_t proto[PROTO_COUNT] = {
	  [UNK] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, "UNK" },
	  [NEC] = { 9000, 4500, 560, 1690, 560, 560, 560, 0, 38000, "NEC" },
	  [NEC_REP] = { 9000, 2250, 0, 0, 0, 0, 0, 0, 38000, "NEC_REP" },
	  [SONY] = { 2400, 600, 1200, 600, 600, 600, 0, 0, 40000, "SONY" },
	  [SAM] = { 4500, 4500, 560, 1690, 560, 560, 560, 0, 38000, "SAM" },
	  [RC5] = { 0, 0, 889, 889, 889, 889, 0, 0, 38000, "RC5" },
	};
	
	bool         check_bit(rmt_data_t &item, uint16_t high, uint16_t low);
	uint32_t     nec_rep_check(rmt_data_t *item, size_t &len);
	uint32_t     nec_check(rmt_data_t *item, size_t &len);
	uint32_t     sam_check(rmt_data_t *item, size_t &len);
	uint32_t     sony_check(rmt_data_t *item, size_t &len);
	uint32_t     rc5_check(rmt_data_t *item, size_t &len);
	bool         rc5_bit(uint32_t d, uint32_t v);
};

#endif