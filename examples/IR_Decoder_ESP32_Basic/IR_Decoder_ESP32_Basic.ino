/*                                                     https://oshwlab.com/ratti3
  _|_|_|                _|      _|      _|  _|_|_|     https://youtube.com/@Ratti3
  _|    _|    _|_|_|  _|_|_|_|_|_|_|_|            _|   https://projecthub.arduino.cc/Ratti3
  _|_|_|    _|    _|    _|      _|      _|    _|_|     https://ratti3.blogspot.com
  _|    _|  _|    _|    _|      _|      _|        _|   https://hackaday.io/Ratti3
  _|    _|    _|_|_|      _|_|    _|_|  _|  _|_|_|     https://www.hackster.io/Ratti3
													   https://github.com/Ratti3

This file is part of https://github.com/Ratti3/IR_Decoder_for_ESP32

IR_Decoder_for_ESP32 is free software: you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
IR_Decoder_for_ESP32 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with IR_Decoder_for_ESP32. If not, see <https://www.gnu.org/licenses/>.
*/

#include <IRDecoderESP32.h>

#define IR_PIN 4

//Initialise library and set GPIO PIN
IRDecoderESP32 ir_recv(IR_PIN);

void setup() {
  Serial.begin(115200);
}

void loop() {
  ir_recv.task();
  if(ir_recv.nec_available())
  {
    Serial.printf("IR Protocol:%s, IR Code: %#x\r\n", ir_recv.protocol(), ir_recv.data());
  }
}
