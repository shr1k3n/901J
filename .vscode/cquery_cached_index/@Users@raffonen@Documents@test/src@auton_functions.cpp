#include "main.h"
#include "cmath"
#include "config.hpp"
#include "auton_functions.h"
//UNITS ARE INCHES!!!!
const float ENCODER_WHEEL_RADIUS = 1.625;
const double WHEEL_RADIUS = 2.0;
const double CIRCUMFERENCE = 2*M_PI*WHEEL_RADIUS;
const double ENCODER_CIRCUMFERENCE = 2*M_PI*ENCODER_WHEEL_RADIUS;
const float ENCODERTICKSPERREVOLUTION = 360.0;
const int DEFAULTSLEWRATEINCREMENT = 20;
const int ARMGEARRATIO = 7;

void puncherDelay(int target){
  while (!((puncher.get_position() < target+15) && (puncher.get_position() > target-12))) {
   // Continue running this loop as long as the motor is not within +-5 units of its goal
   pros::delay(5);
 }
}
void setpuncher(){
  puncher.tare_position();
  puncher.move_absolute(200,200);
  puncherDelay(200);
}
void shootpuncher(){
  puncher.tare_position();
  puncher.move_absolute(200,200);
  puncherDelay(180);
  pros::delay(100);
  puncher.tare_position();
  puncher.move_absolute(210,200);
  puncherDelay(200);
}

void doublePunch(){
  puncher.tare_position();
  puncher.move_absolute(180,200);
  puncherDelay(180);
  angler.move_velocity(-170);
  intake.move_velocity(200);
  puncher.tare_position();
  puncher.move_absolute(180,200);
  puncherDelay(180);
  pros::delay(200);
  puncher.tare_position();
  puncher.move_absolute(200,200);
  puncherDelay(200);
  pros::delay(100);
  puncher.tare_position();
  puncher.move_absolute(200,200);
  puncherDelay(200);
}

void brakeMotors(){//brake the base motors
  left_wheel.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  right_wheel.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  left_chain.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  right_chain.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  left_wheel.move_velocity(0);
  left_chain.move_velocity(0);
  right_wheel.move_velocity(0);
  right_chain.move_velocity(0);
}
void unBrakeMotors(){
  left_wheel.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  right_wheel.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  left_chain.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
  right_chain.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
}

void slewRateControl(pros::Motor *motor, int targetVelocity, int increment){
  int currentVelocity = motor->get_target_velocity();
  if (targetVelocity != 0){
    if (currentVelocity != targetVelocity){
      if (targetVelocity > currentVelocity){
        currentVelocity += increment;
      } else if (targetVelocity < currentVelocity){
        currentVelocity -= increment;
      }
      if (std::abs(currentVelocity) > std::abs(targetVelocity)){
        currentVelocity = targetVelocity;
      }
    }
  } else {
    currentVelocity = targetVelocity;
  }
  motor->move_velocity(currentVelocity);
}

void move_PID(float targetDistance, int maxVelocity, int multiTask){

  const double degreeGoal = (targetDistance/ENCODER_CIRCUMFERENCE)*ENCODERTICKSPERREVOLUTION;
  bool goalMet = false;
  int targetVelocity = 0;
  double currentPosition = 0;
  double error = 0;
  double previous_error = degreeGoal;
  double kP = 0.46;
  double kI = 0.0008;
  double kD = 0.02;
  double integral = 0;
  double derivative = 0;

  if (targetDistance < 0) {maxVelocity *= -1;}
  mainEncoder.reset();

  if(multiTask == 1){//setting multitask
    intake.move_velocity(-200); //intake out
  } else if (multiTask == 2){
    intake.move_velocity(200); //intake in
  } else if (multiTask == 3){
    angler.move_velocity(-200);
  } else if (multiTask == 4){
    angler.move_velocity(200);
  }

  while(!goalMet){
    currentPosition = mainEncoder.get_value();
    error = degreeGoal - currentPosition;

    if (std::abs(error) < 600){
      integral += error;
    }

    derivative = error - previous_error;
    previous_error = error;

    targetVelocity = kP*error + kI*integral + kD*derivative;

    if (targetVelocity > maxVelocity){
      targetVelocity = maxVelocity;
    }

    slewRateControl(&left_wheel, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&left_chain, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_wheel, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_chain, targetVelocity, DEFAULTSLEWRATEINCREMENT);

    if (std::abs(error) < 10){
      goalMet = true;
    }
    pros::delay(10);
  }
  brakeMotors();
}

void park_PID(float targetDistance, int maxVelocity, int multiTask){ //BACK WHEELS

  const double degreeGoal = (targetDistance/CIRCUMFERENCE)*ENCODERTICKSPERREVOLUTION;
  bool goalMet = false;
  int targetVelocity = 0;
  double currentPosition = 0;
  double error = 0;
  double previous_error = degreeGoal;
  double kP = 0.42;
  double kI = 0.0004;
  double kD = 0.05;
  double integral = 0;
  double derivative = 0;

  if (targetDistance < 0) {maxVelocity *= -1;}
  right_chain.tare_position();
  left_chain.tare_position();

  if(multiTask == 1){//setting multitask
    intake.move_velocity(-200); //intake out
  } else if (multiTask == 2){
    intake.move_velocity(200); //intake in
  } else if (multiTask == 3){
    angler.move_velocity(-200);
  } else if (multiTask == 4){
    angler.move_velocity(200);
  }

  while(!goalMet){
    currentPosition = (right_chain.get_position() + left_chain.get_position())/2.0;
    error = degreeGoal - currentPosition;

    if (std::abs(error) < 600){
      integral += error;
    }

    derivative = error - previous_error;
    previous_error = error;

    targetVelocity = kP*error + kI*integral + kD*derivative;

    if (targetVelocity > maxVelocity){
      targetVelocity = maxVelocity;
    }

    slewRateControl(&left_wheel, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&left_chain, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_wheel, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_chain, targetVelocity, DEFAULTSLEWRATEINCREMENT);

    if (std::abs(error) < 10){
      goalMet = true;
    }
    pros::delay(10);
  }
  brakeMotors();
}

void move_align(float targetDistance, int velocity){
   const double degreeGoal = (targetDistance/ENCODER_CIRCUMFERENCE)*ENCODERTICKSPERREVOLUTION;
   if (targetDistance < 0){
     velocity *= -1;
   }
   left_wheel.move_velocity(velocity);
   left_chain.move_velocity(velocity);
   right_wheel.move_velocity(velocity);
   right_chain.move_velocity(velocity);

  while (std::abs(mainEncoder.get_value()) < degreeGoal) {
    pros::delay(5);
  }
}

void turn_PID(float targetDegree, int maxVelocity){

  const double degreeGoal = targetDegree*10;
  bool goalMet = false;
  int targetVelocity = 0;
  int leftTarget = 0;
  int rightTarget = 0;
  double currentPosition = 0;
  double error = 0;
  double previous_error = degreeGoal;
  double kP = 0.15;
  double kI = 0.0002;
  double kD = 0.00;
  double integral = 0;
  double derivative = 0;
  if(targetDegree<0){maxVelocity *= -1;}
  gyro.reset();
  gyro2.reset();


  while(!goalMet){
    currentPosition = (gyro.get_value()+gyro2.get_value())/2;
    error = degreeGoal - currentPosition;
    printf("%f\r\n",currentPosition);
    if (std::abs(error) < 450){
      integral += error;
    }
    printf("gyro avg %f\r\n",currentPosition);
    currentPosition = gyro.get_value();
    printf("gyro 1 %f\r\n",currentPosition);
    currentPosition = gyro2.get_value();
    printf("gyro 2 %f\r\n",currentPosition);
    derivative = error - previous_error;
    previous_error = error;

    targetVelocity = kP*error + kI*integral + kD*derivative;

    if (std::abs(targetVelocity) > std::abs(maxVelocity)){
      targetVelocity = maxVelocity;
    }


      leftTarget = targetVelocity;
      rightTarget = -1*targetVelocity;


    slewRateControl(&left_wheel, leftTarget, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&left_chain, leftTarget, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_wheel, rightTarget, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_chain, rightTarget, DEFAULTSLEWRATEINCREMENT);

    if (std::abs(error) < 6){
      goalMet = true;
    }

    pros::delay(10);
  }
  brakeMotors();
}

void arm_PID(float targetDegree, int maxVelocity){
  arm.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);

  const double degreeGoal = (targetDegree*ARMGEARRATIO);
  bool goalMet = false;
  bool limitStart = false;
  int targetVelocity = 0;
  double currentPosition = 0;
  double error = 0;
  double previous_error = degreeGoal;
  double kP = 0.7;
  double kI = 0.001;
  double kD = 0.03;
  double integral = 0;
  double derivative = 0;

  if (targetDegree < 0) {maxVelocity *= -1;}
  if (armLimitSwitch.get_value() == 1){
    limitStart = true;
  }
  arm.tare_position();


  while(!goalMet){
    currentPosition = arm.get_position();
    error = degreeGoal - currentPosition;

    if (std::abs(error) < 100){
      integral += error;
    }

    derivative = error - previous_error;
    previous_error = error;

    targetVelocity = kP*error + kI*integral + kD*derivative;

    if (targetVelocity > maxVelocity){
      targetVelocity = maxVelocity;
    }

    slewRateControl(&arm, targetVelocity, DEFAULTSLEWRATEINCREMENT);

    if (std::abs(error) < 1){
      goalMet = true;
    } else if (armLimitSwitch.get_value() == 1 && !limitStart){ //resets encoder
      arm.tare_position();
      goalMet = true;
    }

    pros::delay(10);
  }
  arm.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
  arm.move_velocity(0);
}
void arm_stack_task(void* param){
  arm_PID(-180.0,200);
}

void move_ultrasonic(float targetDistance, int maxVelocity, int multiTask){

  const double degreeGoal = targetDistance*10;
  bool goalMet = false;
  int targetVelocity = 0;
  double currentPosition = 0.0;
  double error = 0;
  double previous_error = degreeGoal;
  double kP = 0.8;
  double kI = 0.0006;
  double kD = 0.01;
  double integral = 0;
  double derivative = 0;




  if(multiTask == 1){//setting multitask
    intake.move_velocity(-200); //intake out
  } else if (multiTask == 2){
    intake.move_velocity(200); //intake in
  } else if (multiTask == 3){
    angler.move_velocity(-200);
  } else if (multiTask == 4){
    angler.move_velocity(200);
  } else if (multiTask == 5){
    std::string text("hello");
    pros::Task armStacker(arm_stack_task,&text);
  }

  while(!goalMet){
    currentPosition = (ultrasonic.get_value()/2.54);
    error = currentPosition - degreeGoal;

    if (std::abs(error) < degreeGoal*10){
      integral += error;
    }

    derivative = error - previous_error;
    previous_error = error;

    targetVelocity = kP*error + kI*integral + kD*derivative;

    if (targetVelocity > maxVelocity){
      targetVelocity = maxVelocity;
    }
    printf("%f\r\n",(ultrasonic.get_value()/2.54));

    slewRateControl(&left_wheel, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&left_chain, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_wheel, targetVelocity, DEFAULTSLEWRATEINCREMENT);
    slewRateControl(&right_chain, targetVelocity, DEFAULTSLEWRATEINCREMENT);

    if (std::abs(error) < 5){
      goalMet = true;
    }
    pros::delay(10);
  }
  brakeMotors();
}
