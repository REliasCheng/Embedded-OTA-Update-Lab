#include "USART.h"

void USART_init(){
	
#if USE_USART0
	USART0_init();
#endif

#if USE_USART1
	USART1_init();
#endif

}