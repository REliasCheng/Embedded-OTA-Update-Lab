#include "I2C.h"

void I2C_init(){
  
#if USE_I2C0
  I2C0_init();
#endif
  
}