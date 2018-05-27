#include "MeccanoSmartModule.h"


MeccanoSmartModule::MeccanoSmartModule(TYPE_t type, uint8_t data):
m_type(type),
m_outputData(data),
m_inputData(0),
m_isPresent(false)
{	
}
