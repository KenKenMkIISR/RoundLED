// 振り子回転LED WS2812B NeoPixel for PIC16F18313 by KENKEN

// 接続
// PIC　　NeoPixel
// VDD -- +5V
// VSS -- GND
// RA5 -- DIN
// ※空きピンには何も接続しない

#include "mcc_generated_files/mcc.h"

#define LED_NUM 144  //LEDの総数
#define LED_NUM_DIV3 (LED_NUM/3)

#define SPRING_CONST 1/9000 //バネ定数
#define X0 ((int32_t)LED_NUM*10*256) //初期位置

volatile uint16_t delaycounter=0;
void one_ms_interrupt(void){
	delaycounter++;
}
void delayms(uint16_t t){
	while(delaycounter<t)
		asm("SLEEP"); // Go to Idle mode
	delaycounter=0;
}

void main(void)
{
	int32_t a; //加速度
	int32_t v=0; //速度
	int32_t x=X0; //位置
	int16_t i,j,k;
	uint8_t led_r,led_g,led_b;

	// initialize the device
	SYSTEM_Initialize();
	TMR0_SetInterruptHandler(one_ms_interrupt);

	// Enable the Global Interrupts
	INTERRUPT_GlobalInterruptEnable();

	// Enable the Peripheral Interrupts
	INTERRUPT_PeripheralInterruptEnable();

	CPUDOZE=0x80; // Idle Enable

	while (1){
		//x位置からLED上の位置を計算（下位8ビットは小数点以下）
		if(x>=0) i=(x>>8)%LED_NUM;
		else{
			i=((x>>8)|0xff000000)%LED_NUM;
			if(i<0) i+=LED_NUM; // 環境により剰余結果が負数になる
		}
		//LEDテープの端から順に色相環を出力
		for(j=0;j<LED_NUM;j++){
			k=i+j;
			if(k>=LED_NUM) k-=LED_NUM;
			if(k<LED_NUM_DIV3){
				led_r=LED_NUM_DIV3-k;
				led_g=k;
				led_b=0;
			}
			else if(k<LED_NUM_DIV3*2){
				led_r=0;
				led_g=LED_NUM_DIV3*2-k;
				led_b=k-LED_NUM_DIV3;
			}
			else{
				led_r=k-LED_NUM_DIV3*2;
				led_g=0;
				led_b=LED_NUM-k;
			}
			// LEDテープにG,R,Bの順に出力
			SPI1_Exchange8bit(led_g);
			SPI1_Exchange8bit(led_r);
			SPI1_Exchange8bit(led_b);
		}
		//点の移動計算
		a=-x*SPRING_CONST;
		if(v>0 && v+a<=0){
			v=0;
			x=X0;
		}
		else{
			v+=a;
			x+=v;
		}
		delayms(20);
	}
}
