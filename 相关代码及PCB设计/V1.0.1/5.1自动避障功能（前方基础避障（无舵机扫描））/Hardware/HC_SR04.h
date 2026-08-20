#ifndef __HC_SR04_H__
#define	__HC_SR04_H__

void HC_SR04_Init(void);
void HC_SR04_Start(void);
uint8_t HC_SR04_GetDistance(uint16_t *Distance);

#endif
