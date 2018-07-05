#include "MeccanoPortController.h"
#include "MeccanoSmartModule.h"
#include <map>
#include "mbed.h"

//Mask that adds the start and stop bits 
//For sending bytes
#define START_STOP_BITS 0x0600
#define BIT_TIME 420

uint8_t MeccanoPortController::startByte = 0xFF;

MeccanoPortController::MeccanoPortController(Serial* a_moduleDataOut, InterruptIn* a_moduleDataIn, DigitalOut* a_portEnable)
{
    moduleDataOut = a_moduleDataOut;
    moduleDataIn = a_moduleDataIn;    
    portEnable = a_portEnable;

    *portEnable = 1;

    receiveState = START_BIT;
    sendState = START_BYTE;

    currentModule = 0;

    senderShiftCounter = 0;

    moduleCounter = 0;

    tmpData = 0;

    checkSum = 0;

    receiver;
    lowTime = 0;
    receiverShiftCounter = 0;
    receiverData = 0;

    receiver.start(); 
    moduleDataIn->fall(callback(this, &MeccanoPortController::receiveDataFall));
    moduleDataIn->rise(callback(this, &MeccanoPortController::receiveDataRise));
    

  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(0, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(1, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(2, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(3, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );

  //m_inputThread = new Thread(MeccanoPortController::threadStarter, this);
  //m_inputThread->set_priority(osPriorityRealtime);    
}

void MeccanoPortController::threadStarter(const void* arg)
{
	MeccanoPortController* instancePtr = static_cast<MeccanoPortController*>(const_cast<void*>(arg));
  instancePtr->ioControllerEngine();
}

uint8_t MeccanoPortController::calculateCheckSum(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t Data4, uint8_t moduleNum){
  uint16_t CS = Data1 + Data2 + Data3 + Data4;  // ignore overflow
  CS = CS + (CS >> 8);                  // right shift 8 places
  CS = CS + (CS << 4);                  // left shift 4 places
  CS = CS & 0xF0;                     // mask off top nibble
  CS = CS | moduleNum;

  return (CS & 0xFF);
}

void MeccanoPortController::receiveDataFall()
{
  //t0 = 0
  receiver.reset();
}

void MeccanoPortController::receiveDataRise()
{
  uint8_t bitValue = 0;

  //get t1
  lowTime = receiver.read_us(); 

  switch(receiveState)
  {
     case START_BIT:
       if (lowTime > 1900)
       {
         receiverShiftCounter = 0;
         receiverData = 0;
         receiveState = DATA_BITS; 
       }

       break;
     case DATA_BITS:       
       if (lowTime < 700)
       {
         bitValue = 1; 
       }     
       receiverData |= ((bitValue & 0x01) << receiverShiftCounter);

       if (receiverShiftCounter < 7)
       {
         ++receiverShiftCounter;
       }
       else 
       {

         std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(currentModule); 
         (it->second).m_inputData = receiverData; 
           
         receiveState = START_BIT;

         moduleDataIn->disable_irq(); 
         *portEnable = 1;
         //m_inputThread->signal_set(0x01);       
       }   
       break;
  } 
}

void MeccanoPortController::sendData()
{
  *portEnable = 1;

  checkSum = calculateCheckSum(m_smartModulesMap.at(0).m_outputData,
                          m_smartModulesMap.at(1).m_outputData,
                          m_smartModulesMap.at(2).m_outputData,
                          m_smartModulesMap.at(3).m_outputData,
                          0);

  //Send start byte
  moduleDataOut->putc(startByte);

  //Send channel data
  for (int modCount = 0; modCount < 4; ++modCount)
    moduleDataOut->putc(m_smartModulesMap.at(modCount).m_outputData);        	          	  	
  
  //Send checksum
  moduleDataOut->putc(checkSum);

  //Enable receiver
  moduleDataIn->enable_irq();  	

  wait(0.015);

  *portEnable = 0;
}

void MeccanoPortController::setPosition(int servoSlot, uint8_t position)
{
    std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
    (it->second).m_outputData = position; 

    sendData(); 
}

void MeccanoPortController::enableSendData()
{  

}

void MeccanoPortController::ioControllerEngine()
{
int posCounter = 0x18;

  setPosition(0, 0xFE);
  Thread::wait(500);
  setPosition(0, 0xFC);
  Thread::wait(500);

  while(true)
  {
    setPosition(0, posCounter);
    
    if (posCounter < 0xE8)
      posCounter++;
    else
      posCounter = 0x18;

    Thread::wait(100);    
  }
}  