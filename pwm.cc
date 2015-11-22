#include "pwm.h"
#include <stdio.h>
#include <math.h>
#include <iostream>

void pwm::init() {
  std::cerr << "pwm1";
  fd_ = wiringPiI2CSetup(0x40);
  printf("result of i2c setup:%d\n\r", fd_);
  setAllPWM(0x00, 0x00);
  wiringPiI2CWriteReg8(fd_, __MODE2, __OUTDRV);
  wiringPiI2CWriteReg8(fd_, __MODE1, __ALLCALL);
  usleep(5000);
  int mode1 = wiringPiI2CReadReg8(fd_, __MODE1);
  mode1 = mode1 & __SLEEP;
  wiringPiI2CWriteReg8(fd_, __MODE1, mode1);
  usleep(5000);

}

void pwm::setPWM(unsigned int channel, float frac) {
	unsigned int off = static_cast<unsigned int>(_min + (_max - _min) * frac);
  setPWM(channel, 0, off);	
}

void pwm::reset() {
}

void pwm::setAllPWM(unsigned int on, unsigned int off) {
  wiringPiI2CWriteReg8(fd_, __ALL_LED_ON_L, on & 0xFF);
  wiringPiI2CWriteReg8(fd_, __ALL_LED_ON_H, (on >> 8) & 0xFF );
  wiringPiI2CWriteReg8(fd_, __ALL_LED_OFF_L, off & 0xFF);
  wiringPiI2CWriteReg8(fd_, __ALL_LED_OFF_H, (off >> 8) & 0xFF);
}

void pwm::setPWM(unsigned int channel, unsigned int on, unsigned int off) {
  wiringPiI2CWriteReg8(fd_, __LED0_ON_L + 4 * channel, on & 0xFF);
  wiringPiI2CWriteReg8(fd_, __LED0_ON_H + 4 * channel, (on >> 8) & 0xFF);
  wiringPiI2CWriteReg8(fd_, __LED0_OFF_L + 4 * channel, off & 0xFF);
  wiringPiI2CWriteReg8(fd_, __LED0_OFF_H + 4 * channel, (off >> 8) & 0xFF);
}

void pwm::setPWMFreq(float freq) {
  float prescaleval = 25000000.0;    //# 25MHz
  prescaleval /= 4096.0;       //# 12-bit
  prescaleval /= float(freq);
  prescaleval -= 1.0;
  printf("Setting PWM frequency to %f Hz\n\r", freq);
  printf("Estimated pre-scale: %f\n\r", prescaleval);
  float prescale = floor(prescaleval + 0.5);
  printf("Final pre-scale: %f\n\r", prescale);

  int oldmode = wiringPiI2CReadReg8(fd_, __MODE1);
  int newmode = (oldmode & 0x7F) | 0x10;            // # sleep
  wiringPiI2CWriteReg8(fd_, __MODE1, newmode);        //# go to sleep
  wiringPiI2CWriteReg8(fd_, __PRESCALE, static_cast<int>(floor(prescale)));
  wiringPiI2CWriteReg8(fd_, __MODE1, oldmode);
  
  usleep(5000);

  wiringPiI2CWriteReg8(fd_, __MODE1, oldmode | 0x80);
}
