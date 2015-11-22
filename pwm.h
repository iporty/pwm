#ifndef PWM_H
#define PWM_H

#include <wiringPiI2C.h>
#include <errno.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <queue>


class pwm {
  public:
    pwm() : fd_(-1) {};

    void init();
    void setMin(float min) {
			_min = min;
    }
    void setMax(float max) {
    	_max = max;
    }
	  void setPWM(unsigned int channel, float frac);
    void setPWM(unsigned int channel, unsigned int on, unsigned int off);
    void setPWMFreq(float freq);
    void reset();
    void setAllPWM(unsigned int on, unsigned int off);

  private: 
    // min an max values for time for servos.
    // for 100hz 200 - 600 gives 90 degrees of travel for test servo.
    // may need to have on of these for each channel. 
    float _min;
	  float _max;

    
    int fd_;
    unsigned int  __MODE1         = 0x0;
    unsigned int  __MODE2              = 0x01;
    unsigned int  __SUBADR1            = 0x02;
    unsigned int  __SUBADR2            = 0x03;
    unsigned int  __SUBADR3            = 0x04;
    unsigned int  __PRESCALE           = 0xFE;
    unsigned int  __LED0_ON_L          = 0x06;
    unsigned int  __LED0_ON_H          = 0x07;
    unsigned int  __LED0_OFF_L         = 0x08;
    unsigned int  __LED0_OFF_H         = 0x09;
    unsigned int  __ALL_LED_ON_L       = 0xFA;
    unsigned int  __ALL_LED_ON_H       = 0xFB;
    unsigned int  __ALL_LED_OFF_L      = 0xFC;
    unsigned int  __ALL_LED_OFF_H      = 0xFD;

  // Bits
    unsigned int  __RESTART            = 0x80;
    unsigned int  __SLEEP              = 0x10;
    unsigned int  __ALLCALL            = 0x01;
    unsigned int  __INVRT              = 0x10;
    unsigned int  __OUTDRV             = 0x04;
};

#endif

