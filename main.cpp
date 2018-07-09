#include "mbed.h"
#include "rtos.h"

#include "MeccanoPortController.h"

// ****************** IO
Serial moduleDataOut(D1, D0);
InterruptIn moduleDataIn(D4);
DigitalOut led1(LED1);
DigitalOut portEnable(D3);
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

  moduleDataOut.baud(2398);
  moduleDataOut.format(8, SerialBase::None, 2); 
  ser.baud(115200);
  ser.printf("Hello World!\r\n");
 
  led1 = 0;

  MeccanoPortController port1(&moduleDataOut, &moduleDataIn, &portEnable);

/*
  port1.setCommand(0, MeccanoPortController::ID_NOT_ASSIGNED);
  wait(0.5);
  printAllModulesData(port1.getModulesMap());
  port1.setCommand(0, 0xFC);
  wait(0.5);
  */
  while(port1.getState() != MeccanoPortController::MODULE_IDLE)
  {
    //port1.setCommand(0, MeccanoPortController::ID_NOT_ASSIGNED);
    ser.printf("State = %d\r\n", port1.getState());
    ser.printf("module = %d\r\n", port1.getCurrentModule()); 
    printAllModulesData(port1.getModulesMap());
    wait(0.1);    
  }
  
  while(1) 
  {   	
    printAllModulesData(port1.getModulesMap());
    //ser.printf("Input data = %d \r\n", port1.getReceivedData());
    port1.setCommand(0, posCounter);
    
    if (posCounter < 0xE8)
      posCounter++;
    else
      posCounter = 0x18;

  	wait(0.1);    
  }
}
