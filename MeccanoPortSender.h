#ifndef _MECCANO_PORT_SENDER_H_
#define _MECCANO_PORT_SENDER_H_

#include "mbed.h"

class MeccanoPortSender 
{
    public:
      void sendData(uint8_t data); 

    public:
      MeccanoPortSender(Serial* a_moduleDataOut);
      ~MeccanoPortSender() {}

    private:
      Serial *moduleDataOut;
};

#endif