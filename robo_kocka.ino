#include <microM.h>
#include <Servo.h>
#define LED_BUILTIN 13
#define SPEED_INCR 10
#define SERVO_MAX 180
int pwm_val[14] = {0};

Servo servo[3];

void setup() {
  microM.Setup();
  // Set the built-in LED pin (pin 13 on Arduino Micro) as an output
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(19200);
  servo[2].attach(2);
  servo[2].write(0);
}

void pwmPlus(int pin) {
  pwm_val[pin] += SPEED_INCR;
  pwm_val[pin] = pwm_val[pin] > SERVO_MAX ? SERVO_MAX : pwm_val[pin];
  servo[pin].write(pwm_val[pin]);
}

void pwmMinus(int pin) {
  pwm_val[pin] -= SPEED_INCR;
  pwm_val[pin] = pwm_val[pin] < 0 ? 0 : pwm_val[pin];
  servo[pin].write(pwm_val[pin]);
}

void pwmZero(int pin) {
  pwm_val[pin] = 0;
  servo[pin].write(pwm_val[pin]);
}

void pwmHalf(int pin) {
    pwm_val[pin] = SERVO_MAX/2;
  servo[pin].write(pwm_val[pin]);
}


void pwmToggle(int pin) {
  pwm_val[pin] = pwm_val[pin] < SERVO_MAX/2 ? SERVO_MAX : 0; 
  servo[pin].write(pwm_val[pin]);
  delay(500);
}

void loop() {
  delay(10);

  if (microM.ircommand == 0)
    return;
  
  switch (microM.ircommand) {
      case 1:
        pwmPlus(2);
        break;
      case 2:
        pwmPlus(2);
        break;
      case 3:
        pwmPlus(3);
        break;
      case 4:
        pwmHalf(2);
        break;
      case 5:
        pwmZero(2);
        break;
      case 6:
        pwmZero(3);
        break;
      case 7:
        pwmMinus(2);
        break;
      case 8:
        pwmMinus(2);
        break;
      case 9:
        pwmMinus(3);
        break;

      case 30:
        pwmPlus(2);
        break;
      case 57:
        pwmPlus(11);
        break;
      case 31:
        pwmPlus(12);
        break;
      case 10:
        pwmZero(2);
        break;
      case 80:
        pwmMinus(2);
        break;
      case 64:
        pwmMinus(11);
        break;
      case 38:
        pwmMinus(12);
        break;
      
      case 19:
        pwmPlus(12);
        break;
      case 17:
        pwmPlus(13);
        break;
      case 20:
        pwmMinus(12);
        break;
      case 18:
        pwmMinus(13);
        break;

      case 22:
        pwmToggle(1);
        break;
      case 21:
        pwmToggle(2);
        break;
      case 60:
        pwmToggle(3);
        break;
      case 44:
        pwmToggle(11);
        break;
      case 102:
        pwmToggle(13);
        break;


      default:
        Serial.println("You can't follow directions");
        break;
      
  }
  Serial.println(servo[2].read());
  

  microM.ircommand = 0;
}
