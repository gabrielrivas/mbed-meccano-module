#include "MeccanoPortController.h"
#include "MeccanoSmartModule.h"
#include <map>
#include "mbed.h"

//Mask that adds the start and stop bits 
//For sending bytes
#define START_STOP_BITS 0x0600
#define BIT_TIME 420

uint8_t MeccanoPortController::startByte = 0xFF;

MeccanoPortController::MeccanoPortController(DigitalInOut* a_moduleDataOut, InterruptIn* a_moduleDataIn)
{
    moduleDataOut = a_moduleDataOut;
    moduleDataIn = a_moduleDataIn;    

    moduleDataOut->output();
    moduleDataOut->mode(PullNone); 
    
    //Initialize channel output	
    *moduleDataOut = 1;

    receiveState = START_BIT;
    sendState = START_BYTE;

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
}

uint8_t MeccanoPortController::calculateCheckSum(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t Data4, uint8_t moduleNum){
  uint16_t CS = Data1 + Data2 + Data3 + Data4;  // ignore overflow
  CS = CS + (CS >> 8);                  // right shift 8 places
  CS = CS + (CS << 4);                  // left shift 4 places
  CS = CS & 0xF0;                     // mask off top nibble
  CS = CS | moduleNum;

  return (CS & 0xFF);
}

uint16_t MeccanoPortController::frameByte(uint8_t data)
{
   uint16_t tmp = data;
   
   return (tmp << 1) | START_STOP_BITS;  
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
         receiveState = START_BIT;

         moduleDataIn->disable_irq();        
       }   
       break;
  }
}

void MeccanoPortController::sendData()
{
    switch(sendState)
    {
    	case START_BYTE:    	  
        *moduleDataOut = ((tmpData >> senderShiftCounter) & 0x01);	
    	  
    	  if (senderShiftCounter < 10)
    	  {
            senderShiftCounter++; 
        }
        else
        {
          sendState = DATA_BYTES;
          moduleCounter = 0;
          tmpData = frameByte(m_smartModulesMap.at(moduleCounter).m_data); 
          senderShiftCounter = 0;   
        }  
    	  break;  
    	case DATA_BYTES:    	
        *moduleDataOut = ((tmpData >> senderShiftCounter) & 0x01);

  	    if (senderShiftCounter < 10)
  	    {	
          senderShiftCounter++; 
        }
        else
        {
          if (moduleCounter == 3 )
          {
            tmpData = frameByte(checkSum);
            moduleCounter++;   
          }
        	else if (moduleCounter < 3) 
        	{
            moduleCounter++;
        	  tmpData = frameByte(m_smartModulesMap.at(moduleCounter).m_data);        	          	  	
        	}
        	else
        	{
              *moduleDataOut = 1;
        	  moduleCounter = 0;
        	  sendState = START_BYTE;
        
              //Detach sender interrupt while receiving data
              moduleDataIn->enable_irq();
              sender.detach();
              moduleDataOut->input();                   
        	}
          senderShiftCounter = 0;    
        }   
    	  break;  
    }	
}

void MeccanoPortController::setPosition(int servoSlot, uint8_t position)
{
    std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
    (it->second).m_data = position;  
}

void MeccanoPortController::communicate()
{  
    tmpData = frameByte(startByte);
 
     checkSum = calculateCheckSum(m_smartModulesMap.at(0).m_data,
                          m_smartModulesMap.at(1).m_data,
                          m_smartModulesMap.at(2).m_data,
                          m_smartModulesMap.at(3).m_data,
                          0);
  

    sendState = START_BYTE;
       
    senderShiftCounter = 0;
    moduleCounter = 0;
    
    moduleDataOut->output();      
    sender.attach_us(callback(this, &MeccanoPortController::sendData), BIT_TIME);
}