#include <delay.h>

void delay_init()
{

}

void delay_us(unsigned long delay)
{
	unsigned long i=0;
		  SysTick->LOAD = 32 - 1; //32 000 000 = 1s so 32 = 1 us
		  SysTick->VAL = 0;
		  SysTick->CTRL = 5;

	  while(i < delay)
	  {
		  while(!((SysTick->CTRL) & 0x10000)){} //M3 Generic User Guide p. 159
		  i++;
	  }
}

void delay_ms()
{

} 