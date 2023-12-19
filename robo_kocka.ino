#include <microM.h>
#include <Servo.h>
#define LED_BUILTIN 13
#define BUZZER_PIN 11
#define SPEED_INCR 10
#define SERVO_MAX 180
#define SERVO_COUNT 14
int pwm_val[14] = {0};
int drive_velocity = 0;
#define DRIVE_NOPRESS_TOLERANCE 10 // number of loop cycles. 10ms * 10 = 100ms
#define MAX_DRIVE_VELOCITY 255
int rotation_velocity = 0;
#define MAX_ROTATION_VELOCITY 200
#define TOGGLE_DEBOUNCE 500 // [ms]
#define SWING_SPEED 2 // [rad/s]
float swing_pos[SERVO_COUNT] = {0};

Servo servo[SERVO_COUNT];
int swing_state[SERVO_COUNT] = {0};
int toggle_lock[SERVO_COUNT] = {0};

void setup()
{
    microM.Setup();
    // Set the built-in LED pin (pin 13 on Arduino Micro) as an output
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(19200);
    servo[2].attach(2);
    servo[2].write(0);
    servo[3].attach(3);
    servo[3].write(0);
    servo[13].attach(13);
    servo[13].write(0);

    servo[2].writeMicroseconds(1500);
}

void pwmPlus(int pin)
{
    pwm_val[pin] += SPEED_INCR;
    pwm_val[pin] = pwm_val[pin] > SERVO_MAX ? SERVO_MAX : pwm_val[pin];
    servo[pin].write(pwm_val[pin]);
}

void pwmMinus(int pin)
{
    pwm_val[pin] -= SPEED_INCR;
    pwm_val[pin] = pwm_val[pin] < 0 ? 0 : pwm_val[pin];
    servo[pin].write(pwm_val[pin]);
}

void pwmZero(int pin)
{
    pwm_val[pin] = 0;
    servo[pin].write(pwm_val[pin]);
}

void pwmHalf(int pin)
{
    pwm_val[pin] = SERVO_MAX / 2;
    servo[pin].write(pwm_val[pin]);
}

void pwmToggle(int pin)
{
    if (millis() < toggle_lock[pin])
        return;
    toggle_lock[pin] = millis() + TOGGLE_DEBOUNCE;
    pwm_val[pin] = pwm_val[pin] < SERVO_MAX / 2 ? SERVO_MAX : 0;
    servo[pin].write(pwm_val[pin]);
}

// toggle swing mode
void pwmSwingToggle(int pin)
{
    if (millis() < toggle_lock[pin])
        return;
    toggle_lock[pin] = millis() + TOGGLE_DEBOUNCE;
    swing_state[pin] = !swing_state[pin];
}

// Calculate next swing frame for servo pin. Use sin function.
void swing(int pin)
{
    if (swing_state[pin] == 0)
        return;
    swing_pos[pin]++;
    float x = swing_pos[pin] / 100 * SWING_SPEED;
    if (x > M_PI * 2)
        swing_pos[pin] = 0; 
    int val = (sin(x) + 1) * SERVO_MAX / 2;
    servo[pin].write(val);
}

// set drive velocity with constraints. dir = 1 forward, dir = -1 backward, dir = 0 stop
void drive(int dir)
{
    if (dir == 0)
    {
        drive_velocity = 0;
        rotation_velocity = 0;
    }
    else
    {
        drive_velocity += dir * SPEED_INCR;
        drive_velocity = drive_velocity > MAX_DRIVE_VELOCITY ? MAX_DRIVE_VELOCITY : drive_velocity;
        drive_velocity = drive_velocity < -MAX_DRIVE_VELOCITY ? -MAX_DRIVE_VELOCITY : drive_velocity;
    }
    differential_drive();
}

// set rotation velocity with constraints. dir = 1 right, dir = -1 left, dir = 0 reset
void steer(int dir)
{
    if (dir == 0)
    {
        rotation_velocity = 0;
    }
    else
    {
        rotation_velocity += dir * SPEED_INCR;
        rotation_velocity = rotation_velocity > MAX_ROTATION_VELOCITY ? MAX_ROTATION_VELOCITY : rotation_velocity;
        rotation_velocity = rotation_velocity < -MAX_ROTATION_VELOCITY ? -MAX_ROTATION_VELOCITY : rotation_velocity;
    }
    differential_drive();
}

// Calculate differential drive for 2 motors based on global drive_velocity and rotation_velocity.
// Write PWM signal to microM motor driver.
void differential_drive()
{
    int left = drive_velocity - rotation_velocity;
    int right = drive_velocity + rotation_velocity;
    int max_value = max(abs(left), abs(right));
    if (max_value > 255)
    {
        left = (float)left * 255 / max_value;
        right = (float)right * 255 / max_value;
    }
    Serial.print(left);
    Serial.print(',');
    Serial.println(right);
    microM.Motors(left, right, 0, 0);
}

void playMeowingSound() {
  // Play the meowing sound for a duration
  tone(BUZZER_PIN, 1000, 150); // First meow
  delay(100);

  tone(BUZZER_PIN, 1500, 200); // Second meow
  delay(100);

  tone(BUZZER_PIN, 1200, 250); // Third meow
  delay(100);

  noTone(BUZZER_PIN);
}


void loop()
{
    delay(10);
    swing(2);
    swing(3);
    swing(13);
    int ircommand = microM.ircommand;
    microM.ircommand = 0;


    if (ircommand == 0)
        // drive(0);
        return;
    // Serial.println(ircommand);

    switch (ircommand)
    {
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
        pwmHalf(2);
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


    case 60:
        pwmSwingToggle(13);
        break;
    case 44:
        pwmToggle(11);
        break;
    case 102:
        drive(0);
        break;

    case 21:
        pwmSwingToggle(3);
        break;
    case 22:
        pwmSwingToggle(2);
        break;

    case 117:
        drive(1);
        break;
    case 118:
        drive(-1);
        break;
    case 53:
        steer(-1);
        break;
    case 52:
        steer(1);
        break;

    case 59:
        
        break;
    case 97:
        
        break;

    default:
        break;
    }

}
