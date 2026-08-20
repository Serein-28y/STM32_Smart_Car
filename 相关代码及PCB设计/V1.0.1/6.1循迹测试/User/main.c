#include "stm32f10x.h"
#include "Delay.h"
#include "Serial.h"
#include "Car.h"
#include "Servo.h"
#include "HC_SR04.h"
#include "Avoid.h"
#include "TrackSensor.h"
#include "stdio.h"
int main()
{
	Serial_Init();
	TrackSensor_Init();

	while(1)
	{
		printf("TCRT PB10 DO = %d\r\n", TrackSensor_Read());
		Delay_ms(100);
	}





}
	
