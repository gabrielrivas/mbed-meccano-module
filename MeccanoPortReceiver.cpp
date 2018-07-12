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
  //receiver.start();
  moduleDataIn->enable_irq();
}

void MeccanoPortReceiver::disableReceiver()
{
  moduleDataIn->disable_irq();
  //receiver.stop();
}

void MeccanoPortReceiver::resetFSM()
{  
  receiverShiftCounter = 0;
  receiverData = 0;
  dataReady = false;    
  receiveState = START_BIT;    
}

void MeccanoPortReceiver::receiveDataFall()
{
  //t0 = 0
 
  int lowTime = receiver.read_us();
       
  if ((lowTime > 700) &&  (lowTime < 1000))
  {
    receiverData |= (1 << receiverShiftCounter); 
  }     
       
  ++receiverShiftCounter;
  if (receiverShiftCounter > 6)
  {
    dataReady = true;
    receiveState = START_BIT;         
  }
}

void MeccanoPortReceiver::receiveDataRise()
{
  //get t1
  receiver.reset();
}

        uint8_t MeccanoPortReceiver::getReceivedData() {
          uint8_t dataOut = receiverData; 
          receiverData = 0;            
          return dataOut;
        }


        bool MeccanoPortReceiver::isDataReady()
        {
          return dataReady;
        }