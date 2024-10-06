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

// This example shows how the xTaskCreatePinnedToCore can be used to run code on Core 0 on the ESP32.
// This allows the IR code which is blocking to run without slowing the main loop.
// Note: By default in Arduino main loop is run on Core 1, Wi-Fi is run on Core 0
// https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/api-reference/system/freertos_additions.html?highlight=xtaskcreatepinnedtocore#_CPPv423xTaskCreatePinnedToCore14TaskFunction_tPCKcK8uint32_tPCv11UBaseType_tPC12TaskHandle_tK10BaseType_t

#include "IRDecoderESP32.h"

#define TASK_STACK_SIZE 2048
#define TASK_PRIORITY 10
#define TASK_CORE 0
#define IR_PIN 4

//Initialise library and set GPIO PIN
IRDecoderESP32 ir_recv(IR_PIN);

BaseType_t ir_task_return;
TaskHandle_t ir_task_handle = NULL;

void setup() {
  Serial.begin(115200);
  createIRTask();
}

void loop() {
  IR_ReceiveCheck();
}

// Create the task, storing the handle.
void createIRTask(void) {
  ir_task_return = xTaskCreatePinnedToCore(
    ir_task,         // Function that implements the task.
    "ir_task",       // Text name for the task.
    TASK_STACK_SIZE, // Stack size in words, not bytes.
    (void*)1,        // Parameter passed into the task.
    TASK_PRIORITY,   // Priority at which the task is created.
    &ir_task_handle, // Used to pass out the created task's handle.
    TASK_CORE);      // Pin task to a specific core
}

// Task that calls your code like the main loop()
void ir_task(void* pvParameters) {
  configASSERT(((uint32_t)pvParameters) == 1);
  // Run forever like the main loop()
  while (1) {
    ir_recv.task();
    // This helps prevent the WDT on the ESP32 having a panic attack
    vTaskDelay(100);
  }
  if (ir_task_return == pdPASS) {
    // The task was created. Use the task's handle to delete the task.
    vTaskDelete(ir_task_handle);
  }
}

// Calls function to check if a new IR command is available
void IR_ReceiveCheck() {
  if (ir_recv.nec_available()) {
    Serial.printf("IR Protocol:%s, IR Code: %#x\r\n", ir_recv.protocol(), ir_recv.data());
  }
}