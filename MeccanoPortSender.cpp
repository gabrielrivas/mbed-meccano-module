#include "MeccanoPortSender.h"
#include "mbed.h"

//Mask that adds the start and stop bits 
//For sending bytes
MeccanoPortSender::MeccanoPortSender(Serial *a_moduleDataOut)
{
  moduleDataOut = a_moduleDataOut;     
}

void MeccanoPortSender::sendData(uint8_t data)
{
  moduleDataOut->putc(data);
}
