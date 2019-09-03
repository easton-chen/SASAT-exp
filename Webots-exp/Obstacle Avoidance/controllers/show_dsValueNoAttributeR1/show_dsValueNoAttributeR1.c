/*
 * File:         color_dsNOAGeneric
 * Date:
 * Description:  
 * Author:
 * Modifications:
 */

/*
 * You may need to add include files like <webots/distance_sensor.h> or
 * <webots/differential_wheels.h>, etc.
 */
#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/gps.h>
#include <webots/compass.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h> //用到rand()函数
#include<time.h>   //用到clock()函数
#include <webots/distance_sensor.h>
/*
 * You may want to add macros here.
 */
#define TIME_STEP 128
#define TARGET_POINTS_SIZE 1
#define DISTANCE_TOLERANCE 0.1
#define ANGLE_TOLERANCE 0.1
#define MAX_SPEED 12

enum XYZAComponents {X, Y, Z, ALPHA};
enum Sides {LEFT, RIGHT};

typedef struct _Vector {
    double u;
    double v;
} Vector;

static WbDeviceTag motors[2];
static WbDeviceTag gps;
static WbDeviceTag compass;
static bool right_obstacle;
static bool left_obstacle;
static double left_speed;
static double right_speed;


static Vector targets[TARGET_POINTS_SIZE] = {
  {0, 3.5}
};
static double wood[3][2] = {
  {0, -2}, {0, 0},  {0, 2}};
static double scale[3] = { 0.8, 1.8, 1};
static double wood_size[2] = {0.4, 0.1};

static int current_target_index = 0;
//static bool autopilot = true;

static double modulus_double(double a, double m) {
  int div_i = (int) (a/m);
  double div_d = (double) div_i;
  double r = a - div_d * m;
  if (r<0.0)
    r += m;
  return r;
}

static void robot_set_speed(double left, double right) {
    wb_motor_set_velocity(motors[0], left);
    wb_motor_set_velocity(motors[1], right);
}


static double norm(const Vector *v) {
  return sqrt(v->u*v->u + v->v*v->v);
}

// v = v/||v||
static void normalize(Vector *v) {
  double n = norm(v);
  v->u /= n;
  v->v /= n;
}

// v = v1-v2
static void minus(Vector *v, const Vector *v1, const Vector *v2) {
  v->u = v1->u - v2->u;
  v->v = v1->v - v2->v;
}

// compute the angle between two vectors
// return value: [0, 2Pi[
static double angle(const Vector *v1, const Vector *v2) {
  return modulus_double(atan2(v2->v, v2->u) - atan2(v1->v, v1->u) , 2.0*M_PI);
}
static void printMyLog(){
      printf("left_obstacle:%d right_obstacle:%d left_speed:%lf right_speed:%lf\n",
      left_obstacle, right_obstacle, left_speed, right_speed);
      
      const double *_pos3D = wb_gps_get_values(gps);
      printf("[turn]  x:%lf, z:%lf \n", _pos3D[X], _pos3D[Z]);
}
static void turn_left(double p){
      left_speed = p * MAX_SPEED;
      //printMyLog();
}
static void turn_right(double p){
      right_speed = p * MAX_SPEED;
      //printMyLog();
}

static int run_autopilot(double p) {
  // read gps position and compass values
  const double *pos3D = wb_gps_get_values(gps);
  const double *north3D = wb_compass_get_values(compass);

  // compute the 2D position of the robo and its orientation
  Vector pos   = { pos3D[X], pos3D[Z] };
  Vector north = { north3D[X], north3D[Z] };
  Vector front = { -north.u, north.v };

  // compute the direction and the distance to the target
  Vector dir;
  minus(&dir, &(targets[current_target_index]), &pos);
  double distance = norm(&dir);
  normalize(&dir);

  // compute the target angle
 // double beta = angle( &front, &dir ) - M_PI;
  double beta = angle( &front, &dir ) - M_PI;

  // a target position has been reached
  if (distance < DISTANCE_TOLERANCE) {
    printf("target reached\n");
    current_target_index++;
    current_target_index %= TARGET_POINTS_SIZE;
    robot_set_speed(0, 0);
    return 1;
  }
  // move the robot to the next target
  left_speed = MAX_SPEED;
  right_speed = MAX_SPEED;
  if (left_obstacle) {
      // turn right
      turn_right(p);
  } else if (right_obstacle) {
      turn_left(p);
  } else{
    // big turn
    if (beta > ANGLE_TOLERANCE) {
      turn_right(2.0/3);
    } else if (beta < -ANGLE_TOLERANCE) {
        turn_left(2.0/3);
    }
    // go forward with small rectifications
    else {
       left_speed = MAX_SPEED;
       right_speed = MAX_SPEED;
    }
  }
  // printf("left_obstacle:%d right_obstacle:%d beta:%lf left_speed:%lf right_speed:%lf\n",
  // left_obstacle, right_obstacle, beta, left_speed, right_speed);
  
  // set the motor speeds
  robot_set_speed(left_speed, right_speed);
  return 0;
}
/*
 * Get the distance
 */
static double getDistance(int woodId, double x, double z){
   double distance = 0;

    // get Wood z
    double woodZ = wood[woodId][1];
    double woodX = wood[woodId][0];
    double woodZz = scale[woodId] * wood_size[1]/2;
    double woodXx = scale[woodId] * wood_size[0]/2;
    double pX = 0;
    double pZ = 0;
    
    // decide left or right
   if ( x < woodX )
     pX = woodX - woodXx;
   else
     pX = woodX + woodXx;
   // front of the wood
   if ( z < woodZ - woodZz){
     pZ = woodZ - woodZz;
     if( x < woodX + woodXx && x > woodX - woodXx) // front
       pX = x;  
   }else{ // right or left
     pZ = z;
   }
   distance = sqrt((x-pX)*(x-pX) + (z-pZ)*(z-pZ));
   printf("%lf %lf, %lf %d", x, z, distance, woodId);
   return distance;

}

/*
 * This is the main program.
 * The arguments of the main function can be specified by the
 * "controllerArgs" field of the Robot node
 */
 
int main(int argc, char **argv)
{
  /* necessary to initialize webots stuff */
  wb_robot_init();

  const char *names[2] = {
    "left wheel", "right wheel",
  };

  // get motor tags
  int i;
  WbDeviceTag ps[8];
  char ps_names[6][8] = {
    "in1", "in2", "in3",
    "in4", "in5", "in6"
  };
  for (i = 0; i < 2; i++) {
    motors[i] = wb_robot_get_device(names[i]);
    wb_motor_set_position(motors[i], INFINITY);
  }
  
   for (i = 0; i < 6 ; i++) {
    ps[i] = wb_robot_get_device(ps_names[i]);
    wb_distance_sensor_enable(ps[i], TIME_STEP);
    
  }
  // get gps tag and enable
  gps = wb_robot_get_device("gps");
  wb_gps_enable(gps, TIME_STEP);

  // get compass tag and enable
  compass = wb_robot_get_device("compass");
  wb_compass_enable(compass, TIME_STEP);
// initiate the File of data
    FILE *fp = NULL;
    FILE *fpALL = NULL;
    fpALL = fopen("AttrALLData.txt", "w+");
    fp = fopen("AttrData.txt", "w+");

  // start forward motion
  robot_set_speed(0, 0);
  left_obstacle = false;
  right_obstacle = false;
  double minDistance[3] = {1000, 1000, 1000};
  int minStep[3] = {0};
  bool meetWood = false;
// begin timer
  int  stepTime;
  stepTime=0;
  // main loop
  while (wb_robot_step(TIME_STEP) != -1) {
    stepTime++;
    double ps_values[8];
    for (i = 0; i < 6 ; i++)
      ps_values[i] = wb_distance_sensor_get_value(ps[i]);

    // detect obstacles
    double Gap = 600; 
    double maxS =1024;
    const double *_pos3D = wb_gps_get_values(gps);
     // decide which wood by z,  <-1.5+0.05*0.8; <0.05*1.8  ; 1.5+0.05
   int i =0, woodId = -1;
   for( i = 0; i < 3; i++){
     if( _pos3D[Z] <= wood[i][1] + scale[i] * wood_size[1]/2){
       woodId = i;
       break;
       }
    }
    if (woodId == 2){
      Gap = 920; 
      maxS = 1024;
    }
     left_obstacle =
      (ps_values[0] > Gap ||
      ps_values[1] > Gap ||
      ps_values[2] > Gap)&&
      (ps_values[0]+ps_values[1]+ps_values[2])>(ps_values[3]+ps_values[4]+ps_values[5]);
    right_obstacle =
      (ps_values[3] > Gap ||
      ps_values[4] > Gap ||
      ps_values[5] > Gap) &&
      (ps_values[0]+ps_values[1]+ps_values[2])<=(ps_values[3]+ps_values[4]+ps_values[5]);
     double maxSensor = 0;
     if(left_obstacle){
         maxSensor = ps_values[0] > ps_values[1] ? ps_values[0] :ps_values[1] ;
         maxSensor = maxSensor > ps_values[2] ? maxSensor :ps_values[2] ;
     }else if (right_obstacle){
         maxSensor = ps_values[3] > ps_values[4] ? ps_values[3] :ps_values[4] ;
         maxSensor = maxSensor > ps_values[5] ? maxSensor :ps_values[5] ;
     }
     double maxP=1;
     double minP =0.5;
     
     double p = maxP;
     if(maxSensor > maxS){
       p = minP;
     }else if(maxSensor > Gap){
       p = maxP - (maxSensor-Gap) / (maxS-Gap) *(maxP-minP);
     }
     int ret = run_autopilot(p);
     if(ret == 1 || stepTime > 400){ // 到达目标点
           
           printf("\n\nRunning Steps: %d \n", stepTime);
           break;
     }
    
     // caculate the distance
    double distance = -1;
    
     if( right_obstacle || left_obstacle ){
      printMyLog();
      for (i = 0; i < 6 ; i++){
        printf("ds[%d]:%lf ",i, ps_values[i]);
        if(i==2) printf("\n");
        }
      printf("\n\n");
      
       distance = getDistance(woodId, _pos3D[X], _pos3D[Z]);
       if(distance < minDistance[woodId]){
         minDistance[woodId] = distance;
         minStep[woodId] = stepTime;
       }
       meetWood = true;
    }else{
         if (meetWood == false){
             distance = 2000;
         }else{
             // after meet the first wood
             if (woodId == -1){  // avoid all woods
                 distance = 2000;
             }else{
                 distance = getDistance(woodId, _pos3D[X], _pos3D[Z]);
                 if(distance < minDistance[woodId]){
                   minDistance[woodId] = distance;
                   minStep[woodId] = stepTime;
                 }
             }
         }
    }
   //stepTIme leftVelocity rightV Distance   X Z WoodZ
    if (left_speed != right_speed)
          fprintf(fp, "%d, %lf, %lf, %lf, %lf, %d \n", stepTime, p, maxSensor, distance, wood[woodId][1]-scale[woodId] * wood_size[1]/2-_pos3D[Z], woodId);
         
    fprintf(fpALL, "%d, %lf, %lf, %lf, %lf, %lf, %d \n", stepTime, left_speed, right_speed, distance, _pos3D[X], _pos3D[Z], woodId);
  }
  // minDistance
  for(int j=0; j<3; j++){
    fprintf(fpALL, "%lf, %d \n", minDistance[j], minStep[j]);
    printf("%lf, %d \n", minDistance[j], minStep[j]);
    }
  /* Enter your cleanup code here */
  fclose(fp);
  fclose(fpALL);
  /* This is necessary to cleanup webots resources */
  wb_robot_cleanup();

  return 0;
}
