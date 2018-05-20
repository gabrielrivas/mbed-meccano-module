#include "mbed.h"
#include "rtos.h"

#include "MeccanoPortController.h"

// ****************** IO
DigitalInOut moduleDataOut(D0);
InterruptIn moduleDataIn(D0);
DigitalOut led1(LED1);
Serial ser(SERIAL_TX, SERIAL_RX);

int main() {
  int posCounter = 0x18;
  
  ser.baud(115200);
  ser.printf("Hello World!\r\n");
 
  led1 = 0;

  MeccanoPortController port1(&moduleDataOut,&moduleDataIn);

  port1.communicate();
  wait(0.5);
  port1.setPosition(0, 0xFC);
  port1.communicate();
  wait(0.5);

  while(1) 
  {   	
    ser.printf("Data Received = %d\r\n", port1.getReceivedData());

    port1.setPosition(0, posCounter);
    
    port1.communicate();

    if (posCounter < 0xE8)
      posCounter++;
    else
      posCounter = 0x18;

  	wait(0.1);
  }
}
