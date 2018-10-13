#include "mbed.h"
#include "rtos.h"
#include <vector>
#include "MeccanoPortController.h"

#define MIN_POS 0x20
#define MAX_POS 0xE0
#define INCREMENT 1

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
  int currentModule = 0;
  int posCounter = MIN_POS;
  bool direction = false;

  moduleDataOut.baud(2398);
  moduleDataOut.format(8, SerialBase::None, 2); 
  ser.baud(115200);
  ser.printf("Hello World!\r\n");
 
  led1 = 0;

  MeccanoPortController port1(&moduleDataOut, &moduleDataIn, &portEnable);
wait(2);

  for (int j = 0; j < 4; j++)
  {
    //port1.setCurrentModule(j);
    for (int i = 0; i < 4; i++)
    {
      
      port1.sendData();     
      
    }
  }
  printAllModulesData(port1.getModulesMap());

  for (int j = 0; j < 4; j++)
  {
    //port1.setCurrentModule(j);
    for (int i = 0; i < 4; i++)
    {
      port1.sendData();     
      
    }
  }
  printAllModulesData(port1.getModulesMap());
  
  port1.setCommand(0, posCounter);
  port1.setCommand(1, posCounter);
  port1.setCommand(2, posCounter);
  port1.setCommand(3, posCounter);
  port1.sendData();
  wait(5);
  
  while(1) 
  {   	
    printAllModulesData(port1.getModulesMap());
    //ser.printf("Input data = %d \r\n", port1.getReceivedData());
    
    //port1.setCurrentModule(currentModule);     
    for (int i = 0; i < 4; i++)
    {      
      port1.setCommand(i, posCounter);     
    }
    port1.sendData();
    
    if (!direction)
    {
      if (posCounter < MAX_POS)
        posCounter+=INCREMENT;
      else
      {
        posCounter = MAX_POS;
        direction = true;
      }          
    }
    else
    {
      if (posCounter > MIN_POS)
        posCounter-=INCREMENT;
      else
      {
        posCounter = MIN_POS;
        direction = false;
      }
    }
  	wait(0.1);    
  }
}
