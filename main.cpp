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


MeccanoPortController::PORT_CONTROLLER controllerState;

int main() {
  int posCounter = 0x18;
  int currentModule = 0;
  ser.baud(115200);
  ser.printf("Hello World!\r\n");
 
  led1 = 0;

  MeccanoPortController port1(&moduleDataOut,&moduleDataIn);
  controllerState = MeccanoPortController::MODULE_DISCOVERY;

  while(1) 
  {   	
    printAllModulesData(port1.getModulesMap());
    
    switch(controllerState)
    {
     case MeccanoPortController::MODULE_DISCOVERY:
        port1.setCommand(currentModule, MeccanoPortController::ID_NOT_ASSIGNED);
        controllerState = MeccanoPortController::GET_DISCOVERY_RESPONSE;

        break;
      case MeccanoPortController::GET_DISCOVERY_RESPONSE:
        if (port1.getModulesMap().at(currentModule).m_inputData == 249)
        {
          
            port1.setPresence(currentModule, true); 
        }     
        else{
          port1.setPresence(currentModule, false);
        }     

        if (currentModule < 3)
        {
          currentModule++;
          controllerState = MeccanoPortController::MODULE_DISCOVERY;
        }
        else
        {
          currentModule = 0;
          controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY;
          //controllerState = MeccanoPortController::MODULE_IDLE;
        }        
        break;        
      case MeccanoPortController::MODULE_TYPE_DISCOVERY:
        port1.setCommand(currentModule, MeccanoPortController::REPORT_TYPE);
        controllerState = MeccanoPortController::GET_TYPE_RESPONSE;

        break;
       case MeccanoPortController::GET_TYPE_RESPONSE: 
       
       //if (m_smartModulesMap.at(currentModule).m_isPresent)   
       //  setInputData(currentModule, receiverData);

        if (currentModule < 3)
        {
          currentModule++;
          controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY;
        }
        else
        {
          currentModule = 0;
          controllerState = MeccanoPortController::MODULE_IDLE;
        }
               
        break;       
      case MeccanoPortController::MODULE_IDLE:
        
        port1.setPosition(currentModule, posCounter);
        if (posCounter < 0xE8)
          posCounter++;
        else
          posCounter = 0x0;
        
        if (currentModule < 3)
          currentModule++;
        else
          currentModule = 0; 

        break;
      default:
        break;  
    }


  	Thread::wait(100);
  }
}
