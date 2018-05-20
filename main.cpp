#include "mbed.h"
#include "rtos.h"
#include <map>
#include "MeccanoSmartModule.h"

//Mask that adds the start and stop bits 
//For sending bytes
#define START_STOP_BITS 0x0600
#define BIT_TIME 420

const uint8_t startByte = 0xFF;

// ****************** IO
DigitalInOut moduleDataOut(D0);
InterruptIn moduleDataIn(D0);
DigitalOut led1(LED1);
Serial ser(SERIAL_TX, SERIAL_RX);

// ****************** Sender FSM
enum SENDER_STATES
{
  START_BYTE = 0,
  DATA_BYTES
};

SENDER_STATES sendState = START_BYTE;

int senderShiftCounter = 0;
int moduleCounter = 0;
uint16_t tmpData = 0;
std::map<int, MeccanoSmartModule> m_smartModulesMap;
uint8_t checkSum = 0;

Ticker sender;

// ****************** Receiver FSM

enum RECEIVER_STATES
{
  START_BIT = 0,
  DATA_BITS   
};

RECEIVER_STATES receiveState = START_BIT;

Timer receiver;
int lowTime = 0;
int receiverShiftCounter = 0;
uint8_t receiverData = 0;

uint8_t calculateCheckSum(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t Data4, uint8_t moduleNum){
  uint16_t CS = Data1 + Data2 + Data3 + Data4;  // ignore overflow
  CS = CS + (CS >> 8);                  // right shift 8 places
  CS = CS + (CS << 4);                  // left shift 4 places
  CS = CS & 0xF0;                     // mask off top nibble
  CS = CS | moduleNum;

  return (CS & 0xFF);
}

uint16_t frameByte(uint8_t data)
{
   uint16_t tmp = data;
   
   return (tmp << 1) | START_STOP_BITS;  
}

void receiveDataFall()
{
  //t0 = 0
  receiver.reset();
}

void receiveDataRise()
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

         moduleDataIn.disable_irq();        
       }   
       break;
  }
}

void sendData()
{
    switch(sendState)
    {
    	case START_BYTE:    	  
        moduleDataOut = ((tmpData >> senderShiftCounter) & 0x01);	
    	  
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
        moduleDataOut = ((tmpData >> senderShiftCounter) & 0x01);

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
            moduleDataOut = 1;
        	  moduleCounter = 0;
        	  sendState = START_BYTE;
        
            //Detach sender interrupt while receiving data
            moduleDataIn.enable_irq();
            sender.detach();
            moduleDataOut.input();                   
        	}
          senderShiftCounter = 0;    
        }   
    	  break;  
    }	
}

void setPosition(int servoSlot, uint8_t position)
{
    std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
    (it->second).m_data = position;  
}

void communicate()
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
    
    moduleDataOut.output();      
    sender.attach_us(&sendData, BIT_TIME);
}

int main() {
  int posCounter = 0x18;

  ser.baud(115200);
  ser.printf("Hello World!\r\n");

  receiver.start(); 
  moduleDataIn.fall(&receiveDataFall);
  moduleDataIn.rise(&receiveDataRise);
  
  moduleDataOut.output();
  moduleDataOut.mode(PullNone); 
 
  //Initialize channel output	
  moduleDataOut = 1;
  led1 = 0;

  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(0, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(1, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(2, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(3, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );

  communicate();
  wait(0.5);
  setPosition(0, 0xFC);
  communicate();
  wait(0.5);

  while(1) 
  {   	
    ser.printf("Data Received = %d\r\n", receiverData);

    setPosition(0, posCounter);
    
    communicate();

    if (posCounter < 0xE8)
      posCounter++;
    else
      posCounter = 0x18;

  	wait(0.1);
  }
}
