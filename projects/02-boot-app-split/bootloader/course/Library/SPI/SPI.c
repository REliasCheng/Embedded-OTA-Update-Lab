#include "SPI.h"

void SPI_init(){

#if USE_SPI0
  
  SPI0_init();
  
#endif
  
#if USE_SPI1
  
  SPI1_init();
  
#endif
  
#if USE_SPI2
  
  SPI2_init();
  
#endif
}