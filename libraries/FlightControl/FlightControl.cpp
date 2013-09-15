/*
   FlightControl.cpp

   This program is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 

   This program is distributed in the hope that it will be useful, 
   but WITHOUT ANY WARRANTY; without even the implied warranty of 
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
   GNU General Public License for more details. 

   You should have received a copy of the GNU General Public License 
   along with this program. If not, see <http://www.gnu.org/licenses/>. 
 */


#include "FlightControl.h"
#include "Radio.h"



FlightControl::FlightControl(){  

}

void FlightControl::init(){

}



void FlightControl::control(float targetRoll, float targetPitch, float targetYaw, float currentRoll, float currentPitch, float currentYaw, float throttle, Motors motors, bool motors_on)
{
	float e_roll_old;
	float e_pitch_old;
	float e_yaw_old;
float multiplier=0.5;

 kp_roll= multiplier * 1;
 kp_pitch=multiplier *1;
 kp_yaw=0.1;

 kd_roll=multiplier *1;
 kd_pitch=multiplier *1;
 kd_yaw=0.1;

 ki_roll=0;
 ki_pitch=0;
 ki_yaw=0.1;

    e_roll_old = e_roll;
    e_pitch_old = e_pitch;
    e_yaw_old = e_yaw;

    e_roll = targetRoll - currentRoll;
	e_pitch = targetPitch - currentPitch;
	e_yaw = targetYaw - currentYaw;

    ed_roll = e_roll - e_roll_old;
    ed_pitch = e_pitch - e_pitch_old;
    ed_yaw = e_yaw - e_yaw_old;

if(motors_on)
{
Serial.println("MOTORS ON");
}
else
{
Serial.println("MOTOS OFF ");

}


float U1, U2, U3, U4;
float w1, w2, w3, w4;


U2 = ( kp_roll * e_roll + kd_roll *ed_roll);
U3 = -(kp_pitch * e_pitch + kd_pitch *ed_pitch);
U4 =0*( kp_yaw * e_yaw + kd_yaw *ed_yaw);

U1 = map_f(throttle, MAP_RADIO_LOW , MAP_RADIO_HIGH, -2, 2*100);


w1 = 0.5*U3+0.25*U1-0.25*U4;
w4 = - 0.5*U2+0.25*U4+0.25*U1;
w3 = 0.25*U1- 0.5*U3-0.25*U4;
w2= 0.25*U4+ 0.5*U2+0.25*U1;
//swith 2 et 4





if(w1<0)
{
w1=0;
}
else
{
w1=sqrt(w1);
}

if(w2<0)
{
w2=0;
}
else
{
w2=sqrt(w2);
}

if(w3<0)
{
w3=0;
}
else
{
w3=sqrt(w3);
}

if(w4<0)
{
w4=0;
}
else
{
w4=sqrt(w4);
}




 
Serial.print("w1: ");
Serial.println(w1);

Serial.print("w2: ");
Serial.println(w2);

Serial.print("w3: ");
Serial.println(w3);

Serial.print("w4: ");
Serial.println( w4);


motors.setMotorSpeed(1, motors_on* w1);
motors.setMotorSpeed(2,  motors_on* w2);
motors.setMotorSpeed(3,  motors_on* w3);
motors.setMotorSpeed(4,  motors_on* w4);


}







