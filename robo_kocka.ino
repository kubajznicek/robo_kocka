#include <microM.h>
#include <Servo.h>
#define LED_BUILTIN 13
#define SPEED_INCR 10
#define SERVO_MAX 180
int pwm_val[14] = {0};
int drive_velocity = 0;
int drive_nopress_count = 0;
#define DRIVE_NOPRESS_TOLERANCE 10 // number of loop cycles. 10ms * 10 = 100ms
int max_drive_velocity = 100;  // can be changed by 59/97  (-,+) buttons info/exit
int rotation_velocity = 0;
#define MAX_ROTATION_VELOCITY 80
#define TOGGLE_DEBOUNCE 100 // [ms]

Servo servo[3];
int swing_state[3] = {0};

void setup()
{
    microM.Setup();
    // Set the built-in LED pin (pin 13 on Arduino Micro) as an output
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(19200);
    servo[2].attach(2);
    servo[2].write(0);
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
    pwm_val[pin] = pwm_val[pin] < SERVO_MAX / 2 ? SERVO_MAX : 0;
    servo[pin].write(pwm_val[pin]);
    delay(TOGGLE_DEBOUNCE);
}

// set max drive velocity with constraints
void set_max_drive_velocity(int val)
{
    max_drive_velocity += val;
    max_drive_velocity = max_drive_velocity > 255 ? 255 : max_drive_velocity;
    max_drive_velocity = max_drive_velocity < 60 ? 60 : max_drive_velocity;
}

// toggle swing mode
void pwmSwingToggle(int pin)
{
    swing_state[pin] = !swing_state[pin];
    delay(TOGGLE_DEBOUNCE);
}

// Calculate next swing frame for servo pin. Use sin function.
void swing(int pin)
{
    if (swing_state[pin] == 0)
        return;
    int val = (sin(millis() / 1000.0) + 1) * SERVO_MAX / 2;
    servo[pin].write(val);
}

// set drive velocity with constraints. dir = 1 forward, dir = -1 backward, dir = 0 stop
void drive(int dir)
{
    if (dir == 0)
    {
        drive_nopress_count++;
        if (drive_nopress_count > DRIVE_NOPRESS_TOLERANCE)
        {
            drive_velocity = 0;
        }
    }
    else
    {
        drive_nopress_count = 0;
        drive_velocity += dir * SPEED_INCR;
        drive_velocity = drive_velocity > max_drive_velocity ? max_drive_velocity : drive_velocity;
        drive_velocity = drive_velocity < -max_drive_velocity ? -max_drive_velocity : drive_velocity;    }
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
        rotation_velocity = dir * MAX_ROTATION_VELOCITY;
        rotation_velocity = rotation_velocity > 255 ? 255 : rotation_velocity;
        rotation_velocity = rotation_velocity < -255 ? -255 : rotation_velocity;
    }
    drive_nopress_count = 0;
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
        left = left * 255 / max_value;
        right = right * 255 / max_value;
    }
    microM.motor(1, left);
    microM.motor(2, right);
}


void loop()
{
    delay(10);
    swing(2);
    swing(13);
    int ircommand = microM.ircommand;
    microM.ircommand = 0;

    if (ircommand == 0)
        drive(0);
        return;

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
        set_max_drive_velocity(-50);
        break;
    case 97:
        set_max_drive_velocity(50);
        break;

    default:
        break;
    }

    Serial.println(servo[2].read());
}
