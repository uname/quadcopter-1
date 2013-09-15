
#include <PinChangeInt.h>
#include <Servo.h>
#include <Radio.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU9150Lib.h"
#include "CalLib.h"
#include <dmpKey.h>
#include <dmpmap.h>
#include <inv_mpu.h>
#include <inv_mpu_dmp_motion_driver.h>
#include <EEPROM.h>
#include <Utils.h>
#include "Motors.h"
#include "FlightControl.h"

#define rac22 0.7071



//Radio
// true if new radio signal
volatile uint8_t bUpdateFlagsShared;
int RadioChannels[7];



//IMU
MPU9150Lib MPU;      // the MPU object
#define  ROLL_OFFSET -0.64
#define  PITCH_OFFSET 0.07
#define  YAW_OFFSET 0

float currentRoll_1;
float currentPitch_1;
float currentYaw_1;

float oldRoll;
float oldPitch;
float oldYaw;

float currentRoll;
float currentPitch;
float currentYaw;

float alpha = 0.05;
float currentYawFilter;


//Motors
Motors motors;
bool motors_on;

//FlightControl
FlightControl flightControl;


float targetRoll;
float targetPitch;
float targetYaw;
float throttle;
#define  SERIAL_PORT_SPEED  115200



//Measuring time
unsigned long previousTime = 0;
unsigned long currentTime = 0;
unsigned long deltaTime = 0;
float  freq = 0;
#define LOOP_TIME (int) (1000000.0/(float)MPU_UPDATE_RATE )


//float loop_rate = (float) MPU_UPDATE_RATE 
float loop_time =1000.0*1000.0/(float)MPU_UPDATE_RATE ;
//loop_time= 1000*1000*1/MPU_UPDATE_RATE;




unsigned long start_loop =0;

void setup()
{
	Serial.begin( SERIAL_PORT_SPEED);
	Serial.println("Start");
	Serial.print("Loop time: ");
	Serial.println( LOOP_TIME);

	//=======================================================================================
	//                                    RADIO
	//=======================================================================================
	//Power pin
	pinMode(RADIO_POWER_PIN , OUTPUT);     
	digitalWrite(RADIO_POWER_PIN , HIGH); 

	// using the PinChangeInt library, attach the interrupts to read radio signal
	PCintPort::attachInterrupt(CH1_IN_PIN, calcCh1,CHANGE);
	PCintPort::attachInterrupt(CH2_IN_PIN, calcCh2,CHANGE);
	PCintPort::attachInterrupt(CH3_IN_PIN, calcCh3,CHANGE);
	PCintPort::attachInterrupt(CH4_IN_PIN, calcCh4,CHANGE);
	PCintPort::attachInterrupt(CH5_IN_PIN, calcCh5,CHANGE);
	PCintPort::attachInterrupt(CH6_IN_PIN, calcCh6,CHANGE);




	//=======================================================================================
	//                                    IMU
	//======================================================================================= 
	Serial.print("Arduino9150 starting using device "); 
	Serial.println(DEVICE_TO_USE);
	Wire.begin();
	MPU.selectDevice(DEVICE_TO_USE);
	MPU.init(MPU_UPDATE_RATE, MPU_MAG_MIX_GYRO_AND_MAG, MAG_UPDATE_RATE, MPU_LPF_RATE);// start the MPU

	//init of the EMA
	while (!MPU.read())
	{
		currentYawFilter=MPU.m_fusedEulerPose[VEC3_Z]  - YAW_OFFSET;
	}
	//MPU.waitToBeStable();

	//=======================================================================================
	//                                    MOTORS
	//=======================================================================================   
	motors.init();





	//End of the setup phase
	Serial.print("Setup done"); 
}












void loop()
{ 
  // Measure loop rate
  //
  currentTime = micros();
  deltaTime = currentTime - previousTime;
  previousTime = currentTime;
  start_loop=micros();
freq = 1000000/deltaTime ;
	Serial.print("freq  "); 
	Serial.println(freq); 


	//=======================================================================================
	//                                    RADIO
	//=======================================================================================

	// check shared update flags to see if any channels have a new signal
	if(bUpdateFlagsShared)
	{
		noInterrupts(); // turn interrupts off quickly while we take local copies of the shared variables
		updateRadio();
		interrupts(); 
	}
	getRadio(RadioChannels);
	//Serial.print( "RADIO   " );


	motors_on = RadioChannels[5];



	//=======================================================================================
	//                                    IMU
	//=======================================================================================

	while (MPU.read())
	{                                        // get the latest data if ready yet

		//  MPU.printVector(MPU.m_calAccel);                       // print the calibrated accel data
		//  MPU.printVector(MPU.m_calMag);                         // print the calibrated mag data
		MPU.printAngles(MPU.m_fusedEulerPose);                 // print the output of the data fusion
		
        oldRoll = currentRoll_1;
        oldPitch = currentPitch_1;
		oldYaw=currentYawFilter;

		currentRoll_1= -(MPU.m_fusedEulerPose[VEC3_X] * RAD_TO_DEGREE  - ROLL_OFFSET);
		currentPitch_1= MPU.m_fusedEulerPose[VEC3_Y] * RAD_TO_DEGREE - PITCH_OFFSET;
		currentYaw= MPU.m_fusedEulerPose[VEC3_Z] * RAD_TO_DEGREE  - YAW_OFFSET;

//2kpi pb
if(abs(currentPitch_1)>100   )
{Serial.println("2kPI PBPBPBPBPBPBPBPBBPPBPBPBPBPBPBPBPBPBBPPBPBPBPBPBPBPBPBPBBPPBPBPBPBPBPBPBPBPBB");
currentPitch_1=oldPitch;
}

if(abs(currentRoll_1)>100   )
{Serial.println("2kPI PBPBPBPBPBPBPBPBBPPBPBPBPBPBPBPBPBPBBPPBPBPBPBPBPBPBPBPBBPPBPBPBPBPBPBPBPBPBB");
currentRoll_1=oldRoll;
}



//45deg rotaion
		currentRoll = rac22* currentRoll_1 + rac22*currentPitch_1;
		currentPitch = -rac22* currentRoll_1 + rac22*currentPitch_1;

/*		//Exponential moving average yaw*/
		currentYawFilter = alpha * currentYaw + (1-alpha) * oldYaw;

		Serial.print("x_c: ");
		Serial.print( currentRoll );
		Serial.print("   y_c: ");
		Serial.print( currentPitch);
		Serial.print("   z_c: ");
		Serial.print( currentYawFilter);
Serial.println("");





	}




	//=======================================================================================
	//                                    MOTORS
	//=======================================================================================
	if(1)
	{
	

		targetRoll =  map_f(RadioChannels[2], MAP_RADIO_LOW, MAP_RADIO_HIGH, -ROLL_MAX_RADIO, ROLL_MAX_RADIO);
		targetPitch =  map_f(RadioChannels[4], MAP_RADIO_LOW, MAP_RADIO_HIGH, -PITCH_MAX_RADIO, PITCH_MAX_RADIO);
		targetYaw =  map_f(RadioChannels[3], MAP_RADIO_LOW, MAP_RADIO_HIGH, -YAW_MAX_RADIO, YAW_MAX_RADIO);
		throttle = RadioChannels[1];

		flightControl.control( targetRoll, targetPitch,  targetYaw,  currentRoll,  currentPitch,  currentYaw, throttle, motors,   motors_on);

		/*if (u_motor1>100)*/
		/*{u_motor1=250;*/
		/*}*/
		/*else*/
		/*{u_motor1=50;*/
		/*}*/


		
	}
	else
	{
		motors.allStop();
		//Serial.println(RadioChannels[1]);

	}



while(micros() - start_loop < loop_time)
{

}


}





