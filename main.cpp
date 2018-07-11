#include "mbed.h"
#include "rtos.h"
#include <vector>
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
  std::vector<int> modulesPresent;

  moduleDataOut.baud(2398);
  moduleDataOut.format(8, SerialBase::None, 2); 
  ser.baud(115200);
  ser.printf("Hello World!\r\n");
 
  led1 = 0;

  MeccanoPortController port1(&moduleDataOut, &moduleDataIn, &portEnable);

  for (int i = 0; i < 4; ++i)
  {
    port1.setCommand(i, MeccanoPortController::ID_NOT_ASSIGNED);
    uint8_t comres = port1.sendData();

    ser.printf("Comres = %d \r\n", comres);

    if (comres == 249)
    {
      ser.printf("Module %d is present !!\r\n", i);
      port1.setPresence(i, true);
      modulesPresent.push_back(i);
      wait(0.5); 
      port1.setCommand(i, 0xFC);
      port1.sendData();
      wait(0.5);
    }  
  }

  while(1) 
  {   	
    printAllModulesData(port1.getModulesMap());
    //ser.printf("Input data = %d \r\n", port1.getReceivedData());
    
    for (std::vector<int>::iterator it = modulesPresent.begin(); it != modulesPresent.end(); ++it)
    {
      port1.setCurrentModule(*it);
      port1.setCommand(*it, posCounter);
      port1.sendData();
    }

    if (posCounter < 0xE8)
      posCounter+=2;
    else
      posCounter = 0x18;

  	wait(0.1);    
  }
}
