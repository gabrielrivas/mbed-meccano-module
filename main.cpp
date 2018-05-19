#include "mbed.h"
#include "rtos.h"
#include <map>
#include "MeccanoSmartModule.h"

//0b0000 0110 0000 0000
#define START_STOP_BITS 0x0600
#define BIT_TIME 420

const uint8_t startByte = 0xFF;

enum SENDER_STATES
{
  START_BIT = 0,
  START_BYTE,
  DATA_BYTES
};

SENDER_STATES sendState = START_BYTE;

Serial ser(SERIAL_TX, SERIAL_RX);


Thread dataSendThread;
DigitalInOut moduleDataOut(D0);
InterruptIn moduleDataIn(D1);
DigitalOut led1(LED1);

Ticker sender;

int shiftCounter = 0;
int moduleCounter = 0;
uint16_t tmpData = 0;
std::map<int, MeccanoSmartModule> m_smartModulesMap;
uint8_t checkSum = 0;


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

void sendData()
{
    switch(sendState)
    {
    	case START_BYTE:    	  
        moduleDataOut = ((tmpData >> shiftCounter) & 0x01);	
    	  
    	  if (shiftCounter < 10)
    	  {
            shiftCounter++; 
        }
        else
        {
          sendState = DATA_BYTES;
          moduleCounter = 0;
          tmpData = frameByte(m_smartModulesMap.at(moduleCounter).m_data); 
          shiftCounter = 0;   
        }  
    	  break;  
    	case DATA_BYTES:    	
        moduleDataOut = ((tmpData >> shiftCounter) & 0x01);

  	    if (shiftCounter < 10)
  	    {	
          shiftCounter++; 
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
        	  //dataSendThread.signal_set(0x01);

    //Detach sender interrupt while receiving data
    sender.detach();
    moduleDataOut.input();
                   
        	}
          shiftCounter = 0;    
        }   
    	  break;  
    }	
}


void data_send_process()
{
  while (true) { 
    Thread::signal_wait(0x1);
        

    Thread::wait(10);
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
       
    shiftCounter = 0;
    moduleCounter = 0;
    
    moduleDataOut.output();  
    sender.attach_us(&sendData, BIT_TIME);
}

int main() {
  ser.baud(115200);
  ser.printf("Hello World!\r\n");

 moduleDataOut.output();
 moduleDataOut.mode(PullNone); 

  //moduleDataIn.rise(&dataRise);
  //moduleDataIn.fall(&dataFall);

  //Initialize channel output	
  moduleDataOut = 1;
  led1 = 0;

  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(0, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(1, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(2, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(3, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );


        checkSum = calculateCheckSum(m_smartModulesMap.at(0).m_data,
                          m_smartModulesMap.at(1).m_data,
                          m_smartModulesMap.at(2).m_data,
                          m_smartModulesMap.at(3).m_data,
                          0);
  

  ser.printf("Data initialized %d = \r\n", checkSum);

  dataSendThread.start(&data_send_process); 

  ser.printf("Started thread\r\n");
  ser.printf("Started ticker\r\n");
int posCounter = 0x20;
  communicate();
  wait(0.5);

  setPosition(0, 0xFC);
  communicate();
  wait(0.5);

  while(1) 
  {   	
    setPosition(0, posCounter);
    communicate();

    if (posCounter < 0xE0)
      posCounter++;
    else
      posCounter = 0x20;

  	wait(0.5);
  }
}
