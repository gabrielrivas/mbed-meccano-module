#ifndef _MECCANO_SMART_MODULE_H_
#define _MECCANO_SMART_MODULE_H_

#include "mbed.h"

class MeccanoSmartModule 
{
    public:
	enum TYPE_t {
		M_NONE  = 0x00,
	    M_SERVO = 0x01,
	    M_LED   = 0x02
	};

    public:
      MeccanoSmartModule(TYPE_t type, uint8_t data);
      ~MeccanoSmartModule(){}

      TYPE_t  m_type;
      uint8_t m_data;
};

#endif