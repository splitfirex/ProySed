//--------------------------------------------------------------
// File     : main.c
// Datum    : 03.11.2013
// Version  : 1.1
// Autor    : UB
// EMail    : mc-4u(@)t-online.de
// Web      : www.mikrocontroller-4u.de
// CPU      : STM32F429
// IDE      : CooCox CoIDE 1.7.4
// GCC      : 4.7 2012q4
// Module   : CMSIS_BOOT, M4_CMSIS_CORE
// Funktion : Mini Paint-Programm
// Hinweis  : Diese zwei Files muessen auf 8MHz stehen
//              "cmsis_boot/stm32f4xx.h"
//              "cmsis_boot/system_stm32f4xx.c"
// In Configuration diese Define hinzufügen :
// "STM32F429_439xx" , "__ASSEMBLY__" , "USE_STDPERIPH_DRIVER"
//--------------------------------------------------------------

#include "main.h"
#include "stm32_ub_lcd_ili9341.h"
#include "stm32_ub_font.h"
#include "stm32_ub_touch_stmpe811.h"
#include "stm32f4xx_dma2d.h"
#include <stdio.h>
#include "stm32f4xx.h"
//--------------------------------------------------------------
void P_drawScreen(void);
void P_drawClear(void);
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height, uint16_t color);
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction, uint16_t color);
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width, uint16_t color);
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius, uint16_t color);
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius, uint16_t color);


uint32_t CurrentFrameBuffer;
uint16_t aktColor;



volatile uint32_t msTicks; /* counts 1ms timeTicks       */
void SysTick_Handler(void) {
	msTicks++;
}

//  Delays number of Systicks (happens every 1 ms)
static void Delay(__IO uint32_t dlyTicks){
  uint32_t curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

void setSysTick(){
	// ---------- SysTick timer (1ms) -------- //
	if (SysTick_Config(SystemCoreClock / 1000)) {
		// Capture error
		while (1){};
	}
}

void setup_Periph(){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable the APB1 periph clock for USART2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	// Enable the GPIOA clock, used by pins PA2, PA3
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// Setup the GPIO pins for Tx and Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Connect PA2 and PA3 with the USART2 Alternate Function
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(UART4, &USART_InitStructure);

	/* Enable the USART2 receive interrupt and configure
		the interrupt controller to jump to USART2_IRQHandler()
		if the USART2 receive interrupt occurs
	*/
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	//NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStructure);

	// Finally enable the USART2 peripheral
	USART_Cmd(UART4, ENABLE);
}

void USART_puts(USART_TypeDef *USARTx, volatile char *str){
	while(*str){
		// Wait for the TC (Transmission Complete) Flag to be set
		// while(!(USARTx->SR & 0x040));
		while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
		USART_SendData(USARTx, *str);
		*str++;
	}
}
//--------------------------------------------------------------
int main(void)
{
  uint16_t xp,yp;

  SystemInit(); // Quarz Einstellungen aktivieren

  // Init vom LCD
  UB_LCD_Init();
  // Init der Layer
  UB_LCD_LayerInit_Fullscreen();
  // auf Hintergrund schalten
  UB_LCD_SetLayer_1();
  // Hintergrund komplett mit einer Farbe füllen
  UB_LCD_FillLayer(RGB_COL_WHITE);
  // auf Vordergrund schalten
  UB_LCD_SetLayer_2();
  // Vordergrund komplett mit einer Farbe füllen
  UB_LCD_FillLayer(RGB_COL_WHITE);

  CurrentFrameBuffer=LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;
  aktColor=LCD_COLOR_RED;

  setSysTick();
  setup_Periph(); // initialize USART1 @ 9600 baud
<<<<<<< HEAD
  USART_puts(UART4, "Inicializado discovery");
=======

>>>>>>> 777d498f11395d04b1f61a33891281dfd50d4dfc
  // init und Check vom Touch
  if(UB_Touch_Init()!=SUCCESS) {
    UB_Font_DrawString(10,10,"Touch Error",&Arial_11x18,RGB_COL_WHITE,RGB_COL_RED);
    while(1);
  }


  aktColor=RGB_COL_BLUE;
  char valores[12] = "7894561230EC";
  P_drawScreen();
  char valoresSalida[32] = "";
  int contadorPresion =0;
  while(1)
  {
    // Touch auslesen
    UB_Touch_Read();
    if(Touch_Data.status==TOUCH_PRESSED) {
    	xp=Touch_Data.xp;
    	yp=Touch_Data.yp;

    	if(contadorPresion>28){
    	    contadorPresion =0;
    	}

    	if ((yp <= 80) && (yp >= 0) && (xp >= 0) && (xp <= 80))
        {
        	valoresSalida[contadorPresion++]= '7';
  		    P_dibujar_cuadro_press(0, 0, 80, valores[0],RGB_COL_BLUE,RGB_COL_CYAN );
        }
    	else if ((yp <= 80) && (yp >= 0) && (xp >= 81) && (xp <= 160))
        {
    		valoresSalida[contadorPresion++]= '8';
    		P_dibujar_cuadro_press(80, 0, 80, valores[1],RGB_COL_BLUE,RGB_COL_CYAN );
        }
    	else if ((yp <= 80) && (yp >= 0) && (xp >= 161) && (xp <= 238))
        {
    		valoresSalida[contadorPresion++]= '9';
    		P_dibujar_cuadro_press(160, 0, 80, valores[2],RGB_COL_BLUE,RGB_COL_CYAN );
        }
        else if ((yp <= 160) && (yp >= 81) && (xp >= 0) && (xp <= 80))
        {
        	valoresSalida[contadorPresion++]= '4';
        	P_dibujar_cuadro_press(0, 80, 80, valores[3],RGB_COL_BLUE,RGB_COL_CYAN );
        }
        else if ((yp <= 160) && (yp >= 81) && (xp >= 81) && (xp <= 160))
		{
        	valoresSalida[contadorPresion++]= '5';
        	P_dibujar_cuadro_press(80, 80, 80, valores[4],RGB_COL_BLUE,RGB_COL_CYAN );
		}
        else if ((yp <= 160) && (yp >= 81) && (xp >= 161) && (xp <= 238))
		{
        	valoresSalida[contadorPresion++]= '6';
        	P_dibujar_cuadro_press(160, 80, 80, valores[5],RGB_COL_BLUE,RGB_COL_CYAN );
		}
        else if ((yp <= 240) && (yp >= 161) && (xp >= 0) && (xp <= 80))
        {
        	valoresSalida[contadorPresion++]= '1';
        	P_dibujar_cuadro_press(0, 160, 80, valores[6],RGB_COL_BLUE,RGB_COL_CYAN );
        }
        else if ((yp <= 240) && (yp >= 161) && (xp >= 81) && (xp <= 160))
        {
        	valoresSalida[contadorPresion++]= '2';
        	P_dibujar_cuadro_press(80, 160, 80, valores[7],RGB_COL_BLUE,RGB_COL_CYAN );
        }
        else if ((yp <= 240) && (yp >= 161) && (xp >= 161) && (xp <= 238))
        {
        	valoresSalida[contadorPresion++]= '3';
        	P_dibujar_cuadro_press(160, 160, 80, valores[8],RGB_COL_BLUE,RGB_COL_CYAN );
        }
        else if ((yp <= 320) && (yp >= 241) && (xp >= 0) && (xp <= 80))
		{
        	if(contadorPresion > 0){
<<<<<<< HEAD
				USART_puts(UART4, "1D");
				USART_puts(UART4, valoresSalida);
				USART_puts(UART4, "T");
=======
				USART_puts(USART2, "1D");
				USART_puts(USART2, valoresSalida);
				USART_puts(USART2, "T\r\n");
>>>>>>> 777d498f11395d04b1f61a33891281dfd50d4dfc
				memset(valoresSalida, 0, 32);
				contadorPresion = 0;
				P_dibujar_cuadro_press(0, 240, 80, valores[10],RGB_COL_RED,RGB_COL_CYAN );
        	}
		}
		else if ((yp <= 320) && (yp >= 241) && (xp >= 81) && (xp <= 160))
		{
			valoresSalida[contadorPresion++]= '0';
			P_dibujar_cuadro_press(80, 240, 80, valores[9],RGB_COL_BLUE,RGB_COL_CYAN );
		}
		else if ((yp <= 320) && (yp >= 241) && (xp >= 161) && (xp <= 238))
		{
			if(contadorPresion > 0){
				memset(valoresSalida, 0, 32);
				contadorPresion = 0;
				P_dibujar_cuadro_press(160, 240, 80, valores[11],RGB_COL_RED,RGB_COL_CYAN );
			}
		}

    	if(contadorPresion == 0){
    		  P_dibujar_cuadro((0*80),(3*80), 80, valores[10], RGB_COL_BLACK );
    		  P_dibujar_cuadro((2*80),(3*80), 80, valores[11], RGB_COL_BLACK );
    	}else{
    		P_dibujar_cuadro((0*80),(3*80), 80, valores[10], RGB_COL_RED );
    		P_dibujar_cuadro((2*80),(3*80), 80, valores[11], RGB_COL_RED );
    	}


    	Delay(100);

    }
  }
}


//--------------------------------------------------------------
void P_drawScreen(void)
{

	 char valores[12] = "7894561230EC";
	  int i=0;
	  int j=0;
	  int contador = 0;
	  for(i=0 ; i < 3 ; i++){
		  for(j =0; j <3 ; j++){
			  P_dibujar_cuadro((j*80), (i*80), 80, valores[contador++],RGB_COL_BLUE );
		  }
	  }
	  P_dibujar_cuadro((1*80),(3*80), 80, valores[contador++], RGB_COL_BLUE );
	  P_dibujar_cuadro((0*80),(3*80), 80, valores[contador++], RGB_COL_BLACK );
	  P_dibujar_cuadro((2*80),(3*80), 80, valores[contador++], RGB_COL_BLACK );

}

void P_dibujar_cuadro(int posx, int posy, int l, char* letra, uint16_t color){

	LCD_DrawRect(posx, posy, l, l, LCD_COLOR_BLACK);
	LCD_DrawFullRect(posx+3, posy+3, l-5, l-5, color);
	UB_Font_DrawString(posx+35,posy+35,&letra,&Arial_11x18,RGB_COL_WHITE,color);

}

void P_dibujar_cuadro_press(int posx, int posy, int l, char* letra, uint16_t color, uint16_t color2 ){

	P_dibujar_cuadro(posx, posy, 80, letra,color2 );
	Delay(200);
	P_dibujar_cuadro(posx, posy, 80, letra,color );

}


//--------------------------------------------------------------
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height, uint16_t color)
{
  DMA2D_InitTypeDef      DMA2D_InitStruct;

  uint32_t  Xaddress = 0;
  uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;

  Red_Value = (0xF800 & color) >> 11;
  Blue_Value = 0x001F & color;
  Green_Value = (0x07E0 & color) >> 5;

  Xaddress = CurrentFrameBuffer + 2*(LCD_MAXX*Ypos + Xpos);

  /* configure DMA2D */
  DMA2D_DeInit();
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputGreen = Green_Value;
  DMA2D_InitStruct.DMA2D_OutputBlue = Blue_Value;
  DMA2D_InitStruct.DMA2D_OutputRed = Red_Value;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;
  DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_MAXX - Width);
  DMA2D_InitStruct.DMA2D_NumberOfLine = Height;
  DMA2D_InitStruct.DMA2D_PixelPerLine = Width;
  DMA2D_Init(&DMA2D_InitStruct);

  /* Start Transfer */
  DMA2D_StartTransfer();

  /* Wait for CTC Flag activation */
  while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
  {
  }
}

//--------------------------------------------------------------
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction, uint16_t color)
{
  DMA2D_InitTypeDef      DMA2D_InitStruct;

  uint32_t  Xaddress = 0;
  uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;

  Xaddress = CurrentFrameBuffer + 2*(LCD_MAXX*Ypos + Xpos);

  Red_Value = (0xF800 & color) >> 11;
  Blue_Value = 0x001F & color;
  Green_Value = (0x07E0 & color) >> 5;

  /* Configure DMA2D */
  DMA2D_DeInit();
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
  DMA2D_InitStruct.DMA2D_OutputGreen = Green_Value;
  DMA2D_InitStruct.DMA2D_OutputBlue = Blue_Value;
  DMA2D_InitStruct.DMA2D_OutputRed = Red_Value;
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;

  if(Direction == LCD_DIR_HORIZONTAL)
  {
    DMA2D_InitStruct.DMA2D_OutputOffset = 0;
    DMA2D_InitStruct.DMA2D_NumberOfLine = 1;
    DMA2D_InitStruct.DMA2D_PixelPerLine = Length;
  }
  else
  {
    DMA2D_InitStruct.DMA2D_OutputOffset = LCD_MAXX - 1;
    DMA2D_InitStruct.DMA2D_NumberOfLine = Length;
    DMA2D_InitStruct.DMA2D_PixelPerLine = 1;
  }

  DMA2D_Init(&DMA2D_InitStruct);
  /* Start Transfer */
  DMA2D_StartTransfer();
  /* Wait for CTC Flag activation */
  while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
  {
  }

}


//--------------------------------------------------------------
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width, uint16_t color)
{
  /* draw horizontal lines */
  LCD_DrawLine(Xpos, Ypos, Width, LCD_DIR_HORIZONTAL, color);
  LCD_DrawLine(Xpos, (Ypos+ Height), Width, LCD_DIR_HORIZONTAL, color);

  /* draw vertical lines */
  LCD_DrawLine(Xpos, Ypos, Height, LCD_DIR_VERTICAL, color);
  LCD_DrawLine((Xpos + Width), Ypos, Height, LCD_DIR_VERTICAL, color);
}
