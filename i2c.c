#include "i2c.h"
#include "uart.h"
#include "stdio.h"
char data[50];
void i2c_init(void){
RCC->AHB1ENR|=RCC_AHB1ENR_GPIOBEN; //enable gpiob clock
RCC->APB1ENR|=RCC_APB1ENR_I2C1EN; //enable i2c1 clock
GPIOB->MODER|=0xA0000; //set pb8and9 to alternative function
GPIOB->AFR[1]|=0x44;
GPIOB->OTYPER|=GPIO_OTYPER_OT_8|GPIO_OTYPER_OT_9; //set pb8 and pb9 as open drain
GPIOB->PUPDR|=(1<<16)|(1<<18);
I2C1->CR1=I2C_CR1_SWRST;//reset i2c
I2C1->CR1&=~I2C_CR1_SWRST;// release reset i2c	
I2C1->CR2|=16;//set clock source to 16MHz
I2C1->CCR=80;  //based on calculation
I2C1->TRISE=17; //output max rise 
I2C1->CR1|=I2C_CR1_PE; //enable I2C
}

char i2c_readByte(char saddr,char maddr, char *data)
{
volatile int tmp;
while(I2C1->SR2&I2C_SR2_BUSY){;}
I2C1->CR1|=I2C_CR1_START;
while(!(I2C1->SR1&I2C_SR1_SB)){;}
I2C1->DR=saddr<<1;
while(!(I2C1->SR1&I2C_SR1_ADDR)){;}
tmp=I2C1->SR2;
while(!(I2C1->SR1&I2C_SR1_TXE)){;}
I2C1->DR=maddr;
while(!(I2C1->SR1&I2C_SR1_TXE)){;}
I2C1->CR1|=I2C_CR1_START;
while(!(I2C1->SR1&I2C_SR1_SB)){;}	
I2C1->DR=saddr<<1|1;
while(!(I2C1->SR1&I2C_SR1_ADDR)){;}
I2C1->CR1&=~I2C_CR1_ACK;
tmp =I2C1->SR2;
I2C1->CR1|=I2C_CR1_STOP;
while(!(I2C1->SR1&I2C_SR1_RXNE)){;}
*data=I2C1->DR;
return 0;
}


void delay(int ms)
	{
	SysTick->LOAD=16000-1;
	SysTick->VAL=0;
	SysTick->CTRL=0x5;
		for (int i=0;i<ms;i++)
		{
			while(!(SysTick->CTRL &0x10000)){}
		}
	SysTick->CTRL=0;
	
	}

void i2c_bus_scan(void)
{        
					int a=0; //address variable
         for (uint8_t i=0;i<128;i++) //go through all 127 address
   {
            I2C1->CR1 |= I2C_CR1_START; //generate start 
            while(!(I2C1->SR1 & I2C_SR1_SB)); // wait to start to be generated
            I2C1->DR=(i<<1|0); // transmit the address
            while(!(I2C1->SR1)|!(I2C1->SR2)){}; //clear status register
            I2C1->CR1 |= I2C_CR1_STOP; //generate stop condition
            delay(1);//minium wait time is 40 uS, but for sure, leave it 100 uS
            a=(I2C1->SR1&I2C_SR1_ADDR); //read the status register address set
            if (a==2)//if the address is valid
         {
					 //print the address
					 sprintf(data,"Found I2C device at adress 0x%X (hexadecimal), or %d (decimal)\r\n",i,i);
           UART_Write_String(data);
         }
     }
}

void i2c_write_byte(char saddr,char maddr, char *data)
	{
volatile int Temp;
while(I2C1->SR2&I2C_SR2_BUSY){;}          /*wait until bus not busy*/
I2C1->CR1|=I2C_CR1_START;                 /*generate start*/
while(!(I2C1->SR1&I2C_SR1_SB)){;}         /*wait until start bit is set*/
I2C1->DR = saddr<< 1;                 	 /* Send slave address*/
while(!(I2C1->SR1&I2C_SR1_ADDR)){;}      /*wait until address flag is set*/
Temp = I2C1->SR2; 											 /*clear SR2 by reading it */
while(!(I2C1->SR1&I2C_SR1_TXE)){;}       /*Wait until Data register empty*/
I2C1->DR = maddr;                        /* send memory address*/
while(!(I2C1->SR1&I2C_SR1_TXE)){;}       /*wait until data register empty*/
I2C1->DR = *data; 	
while (!(I2C1->SR1 & I2C_SR1_BTF));      /*wait until transfer finished*/
I2C1->CR1 |=I2C_CR1_STOP;								 /*Generate Stop*/
}
	
void i2c_ReadMulti(char saddr,char maddr, int n, char* data)
{
	volatile int temp;
	while (I2C1->SR2 & I2C_SR2_BUSY){;}
	I2C1->CR1|=I2C_CR1_START;
	while(!(I2C1->SR1 & I2C_SR1_SB)){;}
	I2C1->DR=saddr<<1;
	while(!(I2C1->SR1 & I2C_SR1_ADDR)){;}
	temp=I2C1->SR2;
	while(!(I2C1->SR1&I2C_SR1_TXE)){;}
	I2C1->DR = maddr;
	while(!(I2C1->SR1&I2C_SR1_TXE)){;}
	I2C1->CR1|=I2C_CR1_START;
	while(!(I2C1->SR1 & I2C_SR1_SB)){;}
	I2C1->DR=saddr<<1|1;
	while(!(I2C1->SR1 & I2C_SR1_ADDR)){;}
	temp=I2C1->SR2;
	I2C1->CR1|=I2C_CR1_ACK;
	while(n>0U)
		{
		if(n==1U)
				{
				I2C1->CR1&=~I2C_CR1_ACK;
					I2C1->CR1|=I2C_CR1_STOP;
					while(!(I2C1->SR1&I2C_SR1_RXNE)){;}
					*data++=I2C1->DR;
						break;
				}
			else
					{
					
					while(!(I2C1->SR1&I2C_SR1_RXNE)){;}
						(*data++)=I2C1->DR;
							n--;
					
					}	
				
			
		}	
		
}

void i2c_WriteMulti(char saddr,char maddr,char *buffer, uint8_t length)
{	
while (I2C1->SR2 & I2C_SR2_BUSY);           //wait until bus not busy
I2C1->CR1 |= I2C_CR1_START;                   //generate start
while (!(I2C1->SR1 & I2C_SR1_SB)){;}					//wait until start is generated
volatile int Temp;														
I2C1->DR = saddr<< 1;                 	 			// Send slave address
while (!(I2C1->SR1 & I2C_SR1_ADDR)){;}        //wait until address flag is set
Temp = I2C1->SR2; 														//Clear SR2
while (!(I2C1->SR1 & I2C_SR1_TXE));           //Wait until Data register empty
I2C1->DR = maddr;                      				// send memory address
while (!(I2C1->SR1 & I2C_SR1_TXE));           //wait until data register empty
//sending the data
for (uint8_t i=0;i<length;i++)
 { 
 I2C1->DR=buffer[i]; 													//filling buffer with command or data
	while (!(I2C1->SR1 & I2C_SR1_BTF));
 }	
                             
I2C1->CR1 |= I2C_CR1_STOP;										//wait until transfer finished

}

void lcd_write_i2c(char saddr,uint8_t *buffer, uint8_t length)
{	
while (I2C1->SR2 & I2C_SR2_BUSY);           //wait until bus not busy
I2C1->CR1 |= I2C_CR1_START;                   //generate start
while (!(I2C1->SR1 & I2C_SR1_SB)){;}					//wait until start is generated
volatile int Temp;														
I2C1->DR = saddr<< 1;                 	 			// Send slave address
while (!(I2C1->SR1 & I2C_SR1_ADDR)){;}        //wait until address flag is set
Temp = I2C1->SR2; 														//Clear SR2
//sending the data
for (uint8_t i=0;i<length;i++)
 { 
 I2C1->DR=buffer[i]; 													//filling buffer with command or data
	while (!(I2C1->SR1 & I2C_SR1_BTF));
 }	
                             
I2C1->CR1 |= I2C_CR1_STOP;										//wait until transfer finished

}