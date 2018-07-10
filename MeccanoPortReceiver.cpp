#include "MeccanoPortReceiver.h"
#include "mbed.h"

#define START_BIT_LOW_US 1500
#define ZERO_BIT_US 700


MeccanoPortReceiver::MeccanoPortReceiver(InterruptIn* a_moduleDataIn)
{
  moduleDataIn = a_moduleDataIn;    
 
  receiveState = START_BIT;
 
  receiverShiftCounter = 0;
  receiverData = 0;
  dataReady = false; 
  moduleDataIn->fall(callback(this, &MeccanoPortReceiver::receiveDataFall));
  moduleDataIn->rise(callback(this, &MeccanoPortReceiver::receiveDataRise));    


  receiver.start();
}

void MeccanoPortReceiver::enableReceiver()
{
  moduleDataIn->enable_irq();
}

void MeccanoPortReceiver::disableReceiver()
{
  moduleDataIn->disable_irq();
}

void MeccanoPortReceiver::resetFSM()
{  
  receiverShiftCounter = 0;
  receiverData = 0;  
  receiveState = START_BIT;    
}

void MeccanoPortReceiver::receiveDataFall()
{
  //t0 = 0
receiver.reset();
}

void MeccanoPortReceiver::receiveDataRise()
{
    
  //uint8_t bitValue = 0;

  //get t1
 
  int lowTime = receiver.read_high_resolution_us();

  switch(receiveState)
  {
     case START_BIT:
       if (lowTime > START_BIT_LOW_US)
       {
         receiverShiftCounter = 0;
         receiveState = DATA_BITS; 
       }

       break;
     case DATA_BITS:       
       if (lowTime < ZERO_BIT_US)
       {
         receiverData |= (1 << receiverShiftCounter); 
       }     
       
       if (receiverShiftCounter < 7)
       {
         ++receiverShiftCounter;
       }
       else 
       {
         dataReady = true;
         receiveState = START_BIT;
         disableReceiver();    
       }   
       break;
  }
 
}

        uint8_t MeccanoPortReceiver::getReceivedData() {
          uint8_t dataOut = receiverData; 
          receiverData = 0;            
          return dataOut;
        }


        bool MeccanoPortReceiver::isDataReady()
        {
          bool dReady = dataReady;
          dataReady = false;  
          return dReady;
        }