#include "tasks/include/thruster_task.h"
#include "lib/include/printfloat.h"
#include <stdio.h>

bool thruster_task_init() {
  if ( xTaskCreate(thruster_task, (const portCHAR *)"Thruster", 256, NULL,
                   tskIDLE_PRIORITY + 1, NULL) != pdTRUE) {
    return true;
  }
  return false;
}

static void thruster_task(void *params) {

#ifdef DEBUG
  UARTprintf("Starting ESC task\n");
#endif

  pca9685_begin(I2C_BUS, PCA_ADDR);
  pca9685_setPWMFreq(I2C_BUS, PWM_FREQ);

  struct Thruster_Set thruster_set;
  for (;;) {
    // wait indefinitely for something to come over the buffer
    xMessageBufferReceive(thruster_message_buffer, (void*)&thruster_set,
                          sizeof(thruster_set), portMAX_DELAY);

    blink_rgb(GREEN_LED, 1);
    // Don't know what the scale here is...
    uint16_t val = thruster_set.throttle * 4096;
    pca9685_setPWM(I2C_BUS, thruster_set.thruster_id,  val, 4096 - val);


  }
}