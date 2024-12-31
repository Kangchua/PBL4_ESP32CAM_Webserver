#include <Servo.h>
#include <Wire.h>
#include <SPI.h>
#include "RF24.h"

const byte address[6] = "20202";
RF24 radio(8, 10);
byte data[3];
//motor define
#define P1 6
#define P2 5
#define P3 9
#define P4 3
Servo E1;
Servo E2;
Servo E3;
Servo E4;
//mpu6050
#define ACCEL_XOUTH 0x3B
#define ACCEL_XOUTL 0x3C
#define ACCEL_YOUTH 0x3D
#define ACCEL_YOUTL 0x3E
#define ACCEL_ZOUTH 0x3F
#define ACCEL_ZOUTL 0x40

#define GYRO_XH 0x43
#define GYRO_XL 0x44

const int MPU = 0x68;  // MPU6050 I2C address
//float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float roll, pitch, yaw, froll, fpitch, fyaw = 0;

float elapsedTime, currentTime, previousTime;
float dt = 0.015;
bool turn_filter = false;
//---------------------------------PID------------------------------------
// ---------------- Constants ---------------------------------------
#define INTERRUPT_PIN 2

// Index locations for data and instructions /////
#define YAW 0
#define PITCH 1
#define ROLL 2
#define THROTTLE 3

#define X 0               // X axis
#define Y 1               // Y axis
#define Z 2               // Z axis
#define MPU_ADDRESS 0x68  // I2C address of the MPU-6050
//----------------

//Declaring GYRO global variables
int gyro_x, gyro_y, gyro_z;
long acc_x, acc_y, acc_z, acc_total_vector;
int temperature;
long gyro_x_cal, gyro_y_cal, gyro_z_cal;
long loop_timer;
int lcd_loop_counter;
float angle_pitch, angle_roll;
int angle_pitch_buffer, angle_roll_buffer;
boolean set_gyro_angles;
float angle_roll_acc, angle_pitch_acc;
float angle_pitch_output, angle_roll_output;
//----------------
// Status of Drone
#define STOPPED 0
#define STARTING 1
#define STARTED 2
bool started = false;
int speed1 = 1000;
int speed2 = 1000;
int speed3 = 1000;
int speed4 = 1000;
float instruction[4] = { 0, 0, 0, 1000 };
float measures[3] = { 0, 0, 0 };
// Measured errors compared to target positions in order: [yaw, pitch, roll]
float errors[3];
// Error sums used for integral component of PID in order: [yaw, pitch, roll]
float error_sum[3] = { 0, 0, 0 };
// Previous errors used for derivative component of PID in order:
// [yaw, pirch, roll]
float previous_error[3] = { 0, 0, 0 };
// float Kp[3]        = {0, .21, .21};    // P coefficients in that order : Yaw, Pitch, Roll
// float Ki[3]        = {0.00, 0.01, 0.01}; // I coefficients in that order : Yaw, Pitch, Roll
// float Kd[3]        = {0, 10, 10};
float x = 3.3;
float y = 0.001;
float z = 24.3;

float Kp[3] = { 0, x, x };  // P coefficients in that order : Yaw, Pitch, Roll
float Ki[3] = { 0, y, y };  // I coefficients in that order : Yaw, Pitch, Roll
float Kd[3] = { 0, z, z };
//------------------------------------------------------------------------
void fly() {
  E1.writeMicroseconds(constrain(speed1, 1000, 1900));
  E2.writeMicroseconds(constrain(speed2, 1000, 1900));
  E3.writeMicroseconds(constrain(speed3, 1000, 1900));
  E4.writeMicroseconds(constrain(speed4, 1000, 1900));
}
void st() {
  E1.writeMicroseconds(1000);
  E2.writeMicroseconds(1000);
  E3.writeMicroseconds(1000);
  E4.writeMicroseconds(1000);
}
void calibrate_gyro(int cal_points) {
  // keept the gyro flat and still while calibrating
  // these oare the offset values used to calculate positiion from rest.
  Serial.println("Performing Calibration....");
  for (int cal_int = 0; cal_int < cal_points; cal_int++) {  //Run this code cal_points times
    read_mpu_6050_data();                                   //Read the raw acc and gyro data from the MPU-6050
    gyro_x_cal += gyro_x;                                   //Add the gyro x-axis offset to the gyro_x_cal variable
    gyro_y_cal += gyro_y;                                   //Add the gyro y-axis offset to the gyro_y_cal variable
    gyro_z_cal += gyro_z;                                   //Add the gyro z-axis offset to the gyro_z_cal variable
    delay(3);                                               //Delay 3us to simulate the ~250Hz program loop
  }
  gyro_x_cal /= cal_points;  //Divide the gyro_x_cal variable by cal_points to get the avarage offset
  gyro_y_cal /= cal_points;  //Divide the gyro_y_cal variable by cal_points to get the avarage offset
  gyro_z_cal /= cal_points;  //Divide the gyro_z_cal variable by cal_points to get the avarage offset

  Serial.print("gyro_x_cal:");
  Serial.println(gyro_x_cal);
  Serial.print("gyro_y_cal:");
  Serial.println(gyro_y_cal);
  Serial.print("gyro_z_cal:");
  Serial.println(gyro_z_cal);
}
void setup_mpu_6050_registers() {
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);  //Start communicating with the MPU-6050
  Wire.write(0x6B);              //Send the requested starting register
  Wire.write(0x00);              //Set the requested starting register
  Wire.endTransmission();        //End the transmission
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);  //Start communicating with the MPU-6050
  Wire.write(0x1C);              //Send the requested starting register
  Wire.write(0x10);              //Set the requested starting register
  Wire.endTransmission();        //End the transmission
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);  //Start communicating with the MPU-6050
  Wire.write(0x1B);              //Send the requested starting register
  Wire.write(0x08);              //Set the requested starting register
  Wire.endTransmission();        //End the transmission
}

void read_gyro() {
  read_mpu_6050_data();  //Read the raw acc and gyro data from the MPU-6050

  gyro_x -= gyro_x_cal;  //Subtract the offset calibration value from the raw gyro_x value
  gyro_y -= gyro_y_cal;  //Subtract the offset calibration value from the raw gyro_y value
  gyro_z -= gyro_z_cal;  //Subtract the offset calibration value from the raw gyro_z value

  //Gyro angle calculations
  //0.0000611 = 1 / (250Hz / 65.5)
  angle_pitch += gyro_x * 0.0000611;  //Calculate the traveled pitch angle and add this to the angle_pitch variable
  angle_roll += gyro_y * 0.0000611;   //Calculate the traveled roll angle and add this to the angle_roll variable

  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(gyro_z * 0.000001066);  //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(gyro_z * 0.000001066);  //If the IMU has yawed transfer the pitch angle to the roll angel

  //Accelerometer angle calculations
  acc_total_vector = sqrt((acc_x * acc_x) + (acc_y * acc_y) + (acc_z * acc_z));  //Calculate the total accelerometer vector
  //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
  angle_pitch_acc = asin((float)acc_y / acc_total_vector) * 57.296;  //Calculate the pitch angle
  angle_roll_acc = asin((float)acc_x / acc_total_vector) * -57.296;  //Calculate the roll angle

  //Place the MPU-6050 spirit level and note the values in the following two lines for calibration
  angle_pitch_acc -= 0.0;  //Accelerometer calibration value for pitch
  angle_roll_acc -= 0.0;   //Accelerometer calibration value for roll

  if (set_gyro_angles) {                                            //If the IMU is already started
    angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;  //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
    angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;     //Correct the drift of the gyro roll angle with the accelerometer roll angle
  } else {                                                          //At first start
    angle_pitch = angle_pitch_acc;                                  //Set the gyro pitch angle equal to the accelerometer pitch angle
    angle_roll = angle_roll_acc;                                    //Set the gyro roll angle equal to the accelerometer roll angle
    set_gyro_angles = true;                                         //Set the IMU started flag
  }

  //To dampen the pitch and roll angles a complementary filter is used
  angle_pitch_output = angle_pitch_output * 0.9 + angle_pitch * 0.1;  //Take 90% of the output pitch value and add 10% of the raw pitch value
  angle_roll_output = angle_roll_output * 0.9 + angle_roll * 0.1;     //Take 90% of the output roll value and add 10% of the raw roll value
}

void read_mpu_6050_data() {      //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);  //Start communicating with the MPU-6050
  Wire.write(0x3B);              //Send the requested starting register
  Wire.endTransmission();        //End the transmission
  Wire.requestFrom(0x68, 14);    //Request 14 bytes from the MPU-6050
  while (Wire.available() < 14)
    ;                                            //Wait until all the bytes are received
  acc_x = Wire.read() << 8 | Wire.read();        //Add the low and high byte to the acc_x variable
  acc_y = Wire.read() << 8 | Wire.read();        //Add the low and high byte to the acc_y variable
  acc_z = Wire.read() << 8 | Wire.read();        //Add the low and high byte to the acc_z variable
  temperature = Wire.read() << 8 | Wire.read();  //Add the low and high byte to the temperature variable
  gyro_x = Wire.read() << 8 | Wire.read();       //Add the low and high byte to the gyro_x variable
  gyro_y = Wire.read() << 8 | Wire.read();       //Add the low and high byte to the gyro_y variable
  gyro_z = Wire.read() << 8 | Wire.read();       //Add the low and high byte to the gyro_z variable
}
void updateCompass() {
  unsigned long now = millis();
  while (millis() - now <= 100) {
    read_gyro();
  }
  measures[YAW] = (int)yaw / 2;
  measures[PITCH] = (int)angle_roll / 2;
  measures[ROLL] = (int)angle_pitch / 2;
}
void pidController() {
  float delta_err[3] = { 0, 0, 0 };  // Error deltas in that order   : Yaw, Pitch, Roll
  float yaw_pid = 0;
  float pitch_pid = 0;
  float roll_pid = 0;

  // Take no action if there is no throttle to the motors
  if (instruction[THROTTLE] > 1000) {
    // Calculate sum of errors : Integral coefficients
    error_sum[YAW] += errors[YAW];
    error_sum[PITCH] += errors[PITCH];
    error_sum[ROLL] += errors[ROLL];

    // Calculate error delta : Derivative coefficients
    delta_err[YAW] = errors[YAW] - previous_error[YAW];
    delta_err[PITCH] = errors[PITCH] - previous_error[PITCH];
    delta_err[ROLL] = errors[ROLL] - previous_error[ROLL];

    // Save current error as previous_error for next time
    previous_error[YAW] = errors[YAW];
    previous_error[PITCH] = errors[PITCH];
    previous_error[ROLL] = errors[ROLL];

    // PID = e.Kp + ∫e.Ki + Δe.Kd
    yaw_pid = (errors[YAW] * Kp[YAW]) + (error_sum[YAW] * Ki[YAW]) + (delta_err[YAW] * Kd[YAW]);
    pitch_pid = (errors[PITCH] * Kp[PITCH]) + (error_sum[PITCH] * Ki[PITCH]) + (delta_err[PITCH] * Kd[PITCH]);
    roll_pid = (errors[ROLL] * Kp[ROLL]) + (error_sum[ROLL] * Ki[ROLL]) + (delta_err[ROLL] * Kd[ROLL]);

    // Cauculate new target throttle for each motor
    // NOTE: These depend on setup of drone. Verify setup is propper and
    //       consider changing the plus and minuses here if issues happen.
    //       If drone is in propper setup these make sense.
    speed1 = instruction[THROTTLE] + roll_pid + pitch_pid - yaw_pid;
    speed2 = instruction[THROTTLE] - roll_pid + pitch_pid + yaw_pid;
    // back motors are way more powerful
    speed4 = instruction[THROTTLE] + roll_pid - pitch_pid + yaw_pid;
    speed3 = instruction[THROTTLE] - roll_pid - pitch_pid - yaw_pid;
  }
  // Scale values to be within acceptable range for motors
  speed1 = constrain(speed1, 1000, 1900);
  speed2 = constrain(speed2, 1000, 1900);
  speed4 = constrain(speed4, 1000, 1900);
  speed3 = constrain(speed3, 1000, 1900);
}
void calculateErrors() {
  // NOTE: currently, roll measurements are very noisy
  // consider removing from PID calculations ?
  errors[YAW] = instruction[YAW] - measures[YAW];
  errors[PITCH] = instruction[PITCH] - measures[PITCH];
  errors[ROLL] = instruction[ROLL] - measures[ROLL];
  // Serial.print(measures[ROLL]);
  // Serial.print("\t");
  // Serial.print(measures[YAW]);
  // Serial.print("\t");
  // Serial.print(measures[PITCH]);
  // Serial.print("\t");

  Serial.print(errors[ROLL]);
  Serial.print("\t");
  Serial.print(errors[YAW]);
  Serial.print("\t");
  Serial.print(errors[PITCH]);
  Serial.println("\t");
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  E1.attach(P1);
  E2.attach(P2);
  E3.attach(P3);
  E4.attach(P4);
  st();
  //mpu6050
  Wire.begin();
  //------------------------------PID----------------------------------

  //-------------------------------------------------------------------
  //nrf
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  if (radio.isChipConnected()) {
    Serial.println("Connected");
  }
  //start_seconds = millis()/1000;
  setup_mpu_6050_registers();  //Setup the registers of the MPU-6050 (500dfs / +/-8g) and start the gyro
  calibrate_gyro(2000);
}
void readControl() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));
  }

  if (data[1] == 1) {
    instruction[THROTTLE] = constrain(instruction[THROTTLE] + 5, 1000, 1900);
  }
  if (data[1] == 2) {
    instruction[THROTTLE] = constrain(instruction[THROTTLE] - 5, 1000, 1900);
  }
  if (data[2] == 1) {
    //Forward
    Kp[1] += 0.5;
    Kp[2] += 0.5;
  }
  if (data[2] == 2) {
    //right
    instruction[ROLL] += 1;
    instruction[ROLL] = constrain(instruction[ROLL], -180, 180);
  }
  if (data[2] == 3) {
    //back
    Kp[1] -= 0.5;
    Kp[2] -= 0.5;
  }
  if (data[2] == 4) {
    //right
    instruction[ROLL] -= 1;
    instruction[ROLL] = constrain(instruction[ROLL], -180, 180);
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  readControl();
  updateCompass();
  calculateErrors();
  pidController();
  fly();

  // Serial.print(speed1);
  // Serial.print("\t");
  // Serial.print(speed2);
  // Serial.print("\t");
  // Serial.print(speed3);
  // Serial.print("\t");
  // Serial.println(speed4);
}
