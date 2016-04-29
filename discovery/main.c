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
// In Configuration diese Define hinzuf�gen :
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
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	// Enable the GPIOA clock, used by pins PA2, PA3
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// Setup the GPIO pins for Tx and Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Connect PA2 and PA3 with the USART2 Alternate Function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStructure);

	/* Enable the USART2 receive interrupt and configure
		the interrupt controller to jump to USART2_IRQHandler()
		if the USART2 receive interrupt occurs
	*/
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Finally enable the USART2 peripheral
	USART_Cmd(USART2, ENABLE);
}

void USART_puts(USART_TypeDef *USARTx, volatile char *str){
	while(*str){
		// Wait for the TC (Transmission Complete) Flag to be set
		// while(!(USARTx->SR & 0x040));
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
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
  // Hintergrund komplett mit einer Farbe f�llen
  UB_LCD_FillLayer(RGB_COL_WHITE);
  // auf Vordergrund schalten
  UB_LCD_SetLayer_2();
  // Vordergrund komplett mit einer Farbe f�llen
  UB_LCD_FillLayer(RGB_COL_WHITE);

  CurrentFrameBuffer=LCD_FRAME_BUFFER + LCD_FRAME_OFFSET;
  aktColor=LCD_COLOR_RED;

  setSysTick();
  setup_Periph(); // initialize USART1 @ 9600 baud

  USART_puts(USART2, "inicializado la discovery \r\n");

  // init und Check vom Touch
  if(UB_Touch_Init()!=SUCCESS) {
    UB_Font_DrawString(10,10,"Touch Error",&Arial_11x18,RGB_COL_WHITE,RGB_COL_RED);
    while(1);
  }

  char valores[12] = "7894561230EC";
  int i=0;
  int j=0;
  int contador = 0;
  for(i=0 ; i < 3 ; i++){
	  for(j =0; j <3 ; j++){
		  P_dibujar_cuadro((j*80), (i*80), 80, valores[contador++] );
	  }
  }
  P_dibujar_cuadro((1*80),(3*80), 80, valores[contador++] );
  P_dibujar_cuadro((0*80),(3*80), 80, valores[contador++] );
  P_dibujar_cuadro((2*80),(3*80), 80, valores[contador++] );
  //P_drawScreen();
  //LCD_DrawFullRect(0, 0, 240, 248, LCD_COLOR_BLUE2);

  char valoresSalida[6] = "    \r\n";
  int contadorPresion =0;
  while(1)
  {
    // Touch auslesen
    UB_Touch_Read();
    if(Touch_Data.status==TOUCH_PRESSED) {
    	xp=Touch_Data.xp;
    	yp=Touch_Data.yp;

    	if(contadorPresion>4){
    	    		contadorPresion =0;
    	    	}

    	if ((yp <= 80) && (yp >= 0) && (xp >= 0) && (xp <= 80))
        {
    		USART_puts(USART2, "7\r\n");
        	valoresSalida[contadorPresion++]= '7';
        }
    	else if ((yp <= 80) && (yp >= 0) && (xp >= 81) && (xp <= 160))
        {
    		USART_puts(USART2, "8\r\n");
    		valoresSalida[contadorPresion++]= '8';
        }
    	else if ((yp <= 80) && (yp >= 0) && (xp >= 161) && (xp <= 240))
        {
    		USART_puts(USART2, "9\r\n");
    		valoresSalida[contadorPresion++]= '9';
        }
        else if ((yp <= 160) && (yp >= 81) && (xp >= 0) && (xp <= 80))
        {
        	USART_puts(USART2, "4\r\n");
        	valoresSalida[contadorPresion++]= '4';
        }
        else if ((yp <= 160) && (yp >= 81) && (xp >= 81) && (xp <= 160))
		{
        	USART_puts(USART2, "5\r\n");
        	valoresSalida[contadorPresion++]= '5';
		}
        else if ((yp <= 160) && (yp >= 81) && (xp >= 161) && (xp <= 240))
		{
        	USART_puts(USART2, "6\r\n");
        	valoresSalida[contadorPresion++]= '6';
		}
        else if ((yp <= 240) && (yp >= 161) && (xp >= 0) && (xp <= 80))
        {
        	USART_puts(USART2, "1\r\n");
        	valoresSalida[contadorPresion++]= '1';
        }
        else if ((yp <= 240) && (yp >= 161) && (xp >= 81) && (xp <= 160))
        {
        	USART_puts(USART2, "2\r\n");
        	valoresSalida[contadorPresion++]= '2';
        }
        else if ((yp <= 240) && (yp >= 161) && (xp >= 161) && (xp <= 240))
        {
        	USART_puts(USART2, "3\r\n");
        	valoresSalida[contadorPresion++]= '3';
        }
        else if ((yp <= 320) && (yp >= 241) && (xp >= 0) && (xp <= 80))
		{
        	USART_puts(USART2, "E\r\n");
			USART_puts(USART2, valoresSalida);
			contadorPresion =0;
		}
		else if ((yp <= 320) && (yp >= 241) && (xp >= 81) && (xp <= 160))
		{
			USART_puts(USART2, "0\r\n");
			valoresSalida[contadorPresion++]= '0';
		}
		else if ((yp <= 320) && (yp >= 241) && (xp >= 161) && (xp <= 240))
		{
			USART_puts(USART2, "C\r\n");
			contadorPresion = 0;
		}

    	Delay(500);

    }
  }
}


//--------------------------------------------------------------
void P_drawScreen(void)
{

    LCD_DrawFullRect(5, 250, 30, 30, LCD_COLOR_BLUE2);
    LCD_DrawFullRect(40, 250, 30, 30, LCD_COLOR_CYAN);
    LCD_DrawFullRect(75, 250, 30, 30, LCD_COLOR_YELLOW);
    LCD_DrawFullRect(5, 288, 30, 30, LCD_COLOR_RED);
    LCD_DrawFullRect(40, 288, 30, 30, LCD_COLOR_BLUE);
    LCD_DrawFullRect(75, 288, 30, 30, LCD_COLOR_GREEN);
    LCD_DrawFullRect(145, 288, 30, 30, LCD_COLOR_MAGENTA);
    LCD_DrawFullRect(110, 288, 30, 30, LCD_COLOR_BLACK);
    LCD_DrawRect(180, 270, 48, 50, LCD_COLOR_BLACK);
    LCD_DrawLine(0, 248, 240, LCD_DIR_HORIZONTAL, LCD_COLOR_BLACK);
    LCD_DrawLine(0, 284, 180, LCD_DIR_HORIZONTAL, LCD_COLOR_BLACK);
    LCD_DrawLine(1, 248, 71, LCD_DIR_VERTICAL, LCD_COLOR_BLACK);
    LCD_DrawLine(37, 248, 71, LCD_DIR_VERTICAL, LCD_COLOR_BLACK);
    LCD_DrawLine(72, 248, 71, LCD_DIR_VERTICAL, LCD_COLOR_BLACK);
    LCD_DrawLine(107, 248, 71, LCD_DIR_VERTICAL, LCD_COLOR_BLACK);
    LCD_DrawLine(142, 248, 71, LCD_DIR_VERTICAL, LCD_COLOR_BLACK);
    LCD_DrawLine(0, 319, 240, LCD_DIR_HORIZONTAL, LCD_COLOR_BLACK);
    P_drawClear();

}

void P_dibujar_cuadro(int posx, int posy, int l, char letra){

	LCD_DrawRect(posx, posy, l, l, LCD_COLOR_BLACK);
	LCD_DrawFullRect(posx+3, posy+3, l-5, l-5, aktColor);
	UB_Font_DrawString(posx+35,posy+35,&letra,&Arial_11x18,RGB_COL_BLACK,RGB_COL_WHITE);

}

//--------------------------------------------------------------
void P_drawClear(void)
{
	LCD_DrawRect(180, 270, 48, 50, LCD_COLOR_BLACK);
	LCD_DrawFullRect(182, 272, 46, 46, aktColor);
	UB_Font_DrawString(190,290,"CLR",&Arial_11x18,RGB_COL_BLACK,RGB_COL_WHITE);
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


//--------------------------------------------------------------
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius, uint16_t color)
{
  int32_t  D;    /* Decision Variable */
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */

  D = 3 - (Radius << 1);

  CurX = 0;
  CurY = Radius;

  while (CurX <= CurY)
  {
    if(CurY > 0)
    {
      LCD_DrawLine(Xpos - CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL, color);
      LCD_DrawLine(Xpos + CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL, color);
    }

    if(CurX > 0)
    {
      LCD_DrawLine(Xpos - CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL, color);
      LCD_DrawLine(Xpos + CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL, color);
    }
    if (D < 0)
    {
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }

  LCD_DrawCircle(Xpos, Ypos, Radius, color);
}


//--------------------------------------------------------------
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius, uint16_t color)
{
    int x = -Radius, y = 0, err = 2-2*Radius, e2;
    do {
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_MAXX*(Ypos+y)))) = color;
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_MAXX*(Ypos+y)))) = color;
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_MAXX*(Ypos-y)))) = color;
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_MAXX*(Ypos-y)))) = color;

        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    }
    while (x <= 0);
}

