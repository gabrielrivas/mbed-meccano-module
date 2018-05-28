#include "mbed.h"
#include "rtos.h"

#include "MeccanoPortController.h"

// ****************** IO
DigitalInOut moduleDataOut(D0);
InterruptIn moduleDataIn(D0);
DigitalOut led1(LED1);
Serial ser(SERIAL_TX, SERIAL_RX);

void printAllModulesData(std::map<int, MeccanoSmartModule>& modulesMap)
{
  std::map<int, MeccanoSmartModule>::iterator it;
   
  for(it = modulesMap.begin(); it != modulesMap.end(); ++it)
  {
    ser.printf("Module(k,v) = %d , %d \r\n", it->first, (it->second).m_inputData); 
  }
}

int main() {
  int posCounter = 0x18;
  
  ser.baud(115200);
  ser.printf("Hello World!\r\n");
 
  led1 = 0;

  MeccanoPortController port1(&moduleDataOut,&moduleDataIn);

  port1.setPosition(0, 0xFE);
  wait(0.5);
  port1.setPosition(0, 0xFC);
  
  wait(0.5);

  while(1) 
  {   	
    printAllModulesData(port1.getModulesMap());
    
    port1.setPosition(0, posCounter);
    
    if (posCounter < 0xE8)
      posCounter++;
    else
      posCounter = 0x18;

  	wait(0.1);
  }
}
