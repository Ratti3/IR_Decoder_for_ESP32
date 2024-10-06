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

#include "IRDecoderESP32.h"

//public

IRDecoderESP32::IRDecoderESP32(uint8_t pin)
{
	ir_rx_pin = (int)pin;
	ir_ch_dir = (rmt_ch_dir_t)RMT_RX_MODE;
	ir_memsize = (rmt_reserve_memsize_t)RMT_MEM_RX;
	ir_frequency = (uint32_t)RMT_FREQ;
	ir_recv_data.ir_state = 0;
	ir_recv_data.ir_type = UNK;
	ir_recv_data.ir_data = 0x00000000;
	begin(ir_rx_pin);
}

IRDecoderESP32::~IRDecoderESP32(void)
{	
	end(ir_rx_pin);
}

bool IRDecoderESP32::begin(uint8_t pin)
{	
	ir_rx_pin = pin;
	if(!rmtInit(ir_rx_pin, ir_ch_dir, ir_memsize, ir_frequency))
	{
		return 0;
	}
	if(!rmtSetRxMaxThreshold(ir_rx_pin, 12000))
	{
		return 0;
	}
	if(!rmtSetRxMinThreshold(ir_rx_pin, 1))
	{
		return 0;
	}
	return 1;
}

bool IRDecoderESP32::end(uint8_t pin)
{	
	return rmtDeinit(pin);
}

bool IRDecoderESP32::available(void)
{
	return ir_recv_data.ir_state;
}
bool IRDecoderESP32::nec_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)1)
		return 1;
	else
		return 0;
}
bool IRDecoderESP32::sony_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)3)
		return 1;
	else
		return 0;
}
bool IRDecoderESP32::sam_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)4)
		return 1;
	else
		return 0;
}
bool IRDecoderESP32::rc5_available(void)
{
	if(ir_recv_data.ir_state && ir_recv_data.ir_type==(ir_protocol_type)5)
		return 1;
	else
		return 0;
}

const char* IRDecoderESP32::protocol(void)
{
	return (const char*)proto[ir_recv_data.ir_type].name;
}

uint32_t IRDecoderESP32::data(void)
{
	ir_recv_data.ir_state = 0;
	return ir_recv_data.ir_data;
}


void IRDecoderESP32::task(void){
	begin(ir_rx_pin);
	size_t rx_num_symbols = 64;
	rmtRead(ir_rx_pin, ir_data, &rx_num_symbols, 1000);
	if (rmtReceiveCompleted(ir_rx_pin)) {
		rmt_data_t* rx_items = (rmt_data_t*)ir_data;
		uint32_t rcode = 0;
		ir_protocol_type rproto = UNK;
		if (rcode = nec_check(rx_items, rx_num_symbols))
		{
			rproto = NEC;
		}
		else if (rcode = nec_rep_check(rx_items, rx_num_symbols))
		{
			rproto = NEC;
		}
		else if (rcode = sam_check(rx_items, rx_num_symbols))
		{
			rproto = SAM;
		}
		else if (rcode = sony_check(rx_items, rx_num_symbols))
		{
			rproto = SONY;
		}
		else if (rcode = rc5_check(rx_items, rx_num_symbols))
		{
			rproto = RC5;
		}
		ir_recv_data.ir_state = 1;
		ir_recv_data.ir_type = rproto;
		ir_recv_data.ir_data = rcode;
	} else {
		ir_recv_data.ir_state = 0;
	}
	end(ir_rx_pin);
}

//private

bool IRDecoderESP32::check_bit(rmt_data_t &item, uint16_t high, uint16_t low) {
  return item.level0 == 0 && item.level1 != 0 && item.duration0 < (high + bitMargin) && item.duration0 > (high - bitMargin) && item.duration1 < (low + bitMargin) && item.duration1 > (low - bitMargin);
}

uint32_t IRDecoderESP32::nec_rep_check(rmt_data_t *item, size_t &len) {
  const uint8_t totalData = 2;
  if (len < totalData) {
    return 0;
  }
  const uint32_t m = 0x80000000;
  uint32_t code = 0;
  for (uint8_t i = 0; i < totalData; i++) {
    if (i == 0) {  //header
      if (check_bit(item[i], proto[NEC_REP].header_high, proto[NEC_REP].header_low)) {
        code = 0xffffffff;
      } else {
        return 0;
      }
    }
  }
  return code;
}

uint32_t IRDecoderESP32::nec_check(rmt_data_t *item, size_t &len) {
  const uint8_t totalData = 34;
  if (len < totalData) {
    return 0;
  }
  const uint32_t m = 0x80000000;
  uint32_t code = 0;
  for (uint8_t i = 0; i < totalData; i++) {
    if (i == 0) {  //header
      if (!check_bit(item[i], proto[NEC].header_high, proto[NEC].header_low)) {
        return 0;
      }
    } else if (i == 33) {  //footer
      if (!check_bit(item[i], proto[NEC].footer_high, proto[NEC].footer_low)) {
        return 0;
      }
    } else if (check_bit(item[i], proto[NEC].one_high, proto[NEC].one_low)) {
      code |= (m >> (i - 1));
    } else if (!check_bit(item[i], proto[NEC].zero_high, proto[NEC].zero_low)) {
      return 0;
    }
  }
  return code;
}

uint32_t IRDecoderESP32::sam_check(rmt_data_t *item, size_t &len){
	const uint8_t  totalData = 34;
	if(len < totalData ){
		return 0;
	}
	const uint32_t m = 0x80000000;
	uint32_t code = 0;
	for(uint8_t i = 0; i < totalData; i++){
		if(i == 0){//header
			if(!check_bit(item[i], proto[SAM].header_high, proto[SAM].header_low)){return 0;}
		}else if(i == 33){//footer
			if(!check_bit(item[i], proto[SAM].footer_high, proto[SAM].footer_low)){return 0;}
		}else if(check_bit(item[i], proto[SAM].one_high, proto[SAM].one_low)){
			code |= (m >> (i - 1) );
		}else if(!check_bit(item[i], proto[SAM].zero_high, proto[SAM].zero_low)){
			return 0;
		}
	}
	return code;
}

uint32_t IRDecoderESP32::sony_check(rmt_data_t *item, size_t &len){
	const uint8_t totalMin = 12;
	uint8_t i = 0;
	if(len < totalMin || !check_bit(item[i], proto[SONY].header_high, proto[SONY].header_low)){
		return 0;
	}
	i++;
	uint32_t m = 0x80000000;
	uint32_t code = 0;
	uint8_t maxData = 20;
	if(len < maxData){maxData = 15;}
	if(len < maxData){maxData = 12;}
	for(int j = 0; j < maxData - 1; j++){
		if(check_bit(item[i], proto[SONY].one_high, proto[SONY].one_low)){
			code |= (m >> j);
		}else if(check_bit(item[i], proto[SONY].zero_high, proto[SONY].zero_low)){
			code |= 0 & (m >> j);
		}else if(item[i].duration1 > 15000){ //space repeats
			break;
		}else{
			return 0;
		}
		i++;
	}
	code = code >> (32 - i);
	len = i+1;
	return code;
}

uint32_t IRDecoderESP32::rc5_check(rmt_data_t *item, size_t &len){
	if(len < 13 || len > 30 ){
		return 0;
	}
	const uint16_t RC5_High = proto[RC5].one_high + proto[RC5].one_low;
	uint32_t code = 0; bool c = false;
	for(uint8_t i = 0; i < len; i++){
		uint32_t d0 = item[i].duration0;
		uint32_t d1 = item[i].duration1;
		if (rc5_bit(d0, proto[RC5].one_low)) {
			code = (code << 1) | c;
			c = rc5_bit(d1, RC5_High) ? !c : c;
		} else if (rc5_bit(d0, RC5_High)) {
			code = (code << 2) | (item[i].level0 << 1) | !item[i].level0;
			c = rc5_bit(d1, proto[RC5].one_low) ? !c : c;
		}else{
			return 0;
		}
	}
	return code;
}

bool IRDecoderESP32::rc5_bit(uint32_t d, uint32_t v) {
	return (d < (v + bitMargin)) && (d > (v - bitMargin));
}


