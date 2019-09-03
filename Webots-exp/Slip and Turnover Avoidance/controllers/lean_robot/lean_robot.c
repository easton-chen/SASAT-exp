 /*
 * Copyright 1996-2018 Cyberbotics Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <webots/robot.h>

// Added a new include file
#include <webots/motor.h>
#include <webots/gyro.h>
#include <webots/accelerometer.h>
#include <webots/position_sensor.h>
#include <webots/gps.h>
#include <webots/compass.h>
#include <stdio.h>
#include <math.h>

#define TIME_STEP 32

#define MAX_SPEED 12

#define TARGET_POINTS_SIZE 1
#define DISTANCE_TOLERANCE 0.1
#define ANGLE_TOLERANCE 0.1


enum XYZAComponents {X, Y, Z, ALPHA};
enum Sides {LEFT, RIGHT};

typedef struct _Vector {
    double u;
    double v;
} Vector;

double mark[8] = {10.44, 8.80, 6.00, 2.15, -2.46, -4.14, -6.96, -9.68};

static WbDeviceTag motors[2];
static WbDeviceTag gps;
static WbDeviceTag compass;

static double left_speed;
static double right_speed;
static double set_speed;


static void robot_set_speed(double left, double right) {
    wb_motor_set_velocity(motors[0], left);
    wb_motor_set_velocity(motors[1], right);
}

int main(int argc, char **argv) {
    wb_robot_init();

    // get a handler to the motors and set target position to infinity (speed control)
    const char *names[2] = {
      "left wheel motor", "right wheel motor",
    };
    for (int i = 0; i < 2; i++) {
      motors[i] = wb_robot_get_device(names[i]);
      wb_motor_set_position(motors[i], INFINITY);
    }
  
    WbDeviceTag ps_l = wb_robot_get_device("left wheel sensor");
    WbDeviceTag ps_r = wb_robot_get_device("right wheel sensor");
    wb_position_sensor_enable(ps_l, TIME_STEP);
    wb_position_sensor_enable(ps_r, TIME_STEP);
  

    // set up the motor speeds at 10% of the MAX_SPEED.
    set_speed = MAX_SPEED;
    left_speed = set_speed;
    right_speed = set_speed;
    robot_set_speed(left_speed, right_speed);
  
    WbDeviceTag gyro = wb_robot_get_device("gyro");
    wb_gyro_enable(gyro, TIME_STEP);
    
    WbDeviceTag acc = wb_robot_get_device("accelerometer");
    wb_accelerometer_enable(acc, TIME_STEP);
    
    // get gps tag and enable
    gps = wb_robot_get_device("gps");
    wb_gps_enable(gps, TIME_STEP);

    // get compass tag and enable
    compass = wb_robot_get_device("compass");
    wb_compass_enable(compass, TIME_STEP);
  
    int sample_num = 0;
    FILE *fp = fopen("data.txt","w");
    
    double last_z = 12.98;
    while (wb_robot_step(TIME_STEP) != -1) {
        sample_num++;
        const double *vel = wb_gyro_get_values(gyro);
        const double vel_l = wb_motor_get_velocity(motors[0]);
        const double vel_r = wb_motor_get_velocity(motors[1]);
        const double pos_l = wb_position_sensor_get_value(ps_l);
        const double pos_r = wb_position_sensor_get_value(ps_r);
        const double *loc = wb_gps_get_values(gps);
        const double *a = wb_accelerometer_get_values(acc);
        double aa = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
        
        printf("rotation axes: [ x y z ] = [ %+.2f %+.2f %+.2f ]\n", vel[0], vel[1], vel[2]);
        printf("velocity left:%.2f, right:%.2f\n", vel_l, vel_r);
        printf("position left:%.2f, right:%.2f\n", pos_l, pos_r);
        printf("location: [x:%.2f y:%.2f z:%.2f]\n", loc[0], loc[1], loc[2]);
        printf("acceleter: in all: %+.2f; [x, y, z] = [%+.2f %+.2f %+.2f]\n", aa, a[0], a[1], a[2]);
        printf("==================\n");
        
        if ((last_z - loc[2] > 0.1) && (loc[2] > -14.99)) {
          fprintf(fp, "No.%d, Z:%.2f, speed:%.2f \r\n", sample_num, loc[2], (left_speed+right_speed)/2);
          last_z = loc[2];
        }
        // first flat 
        if (loc[2] > mark[0]) {
            set_speed = 12;
            robot_set_speed(set_speed, set_speed);
        }

        // first up
        if (loc[2] < mark[0] && loc[2] > mark[1]){
            set_speed = 10;
            robot_set_speed(set_speed, set_speed);
        }

        // adjsut speed after going up
        if (loc[2] < mark[1] && loc[2] > mark[2]){
            set_speed = 10;
            robot_set_speed(set_speed, set_speed);
        }
        
        // slow down when going down
        if(vel[0] < -0.5 && loc[2] < mark[2] && loc[2] > mark[3]) {
          set_speed = 9;
          robot_set_speed(set_speed,set_speed);
          //printf("sample num:%d\n",sample_num);
          printf("slow down!\n"); 
        }

        // second flat
        if(loc[2] < mark[3] && loc[2] > mark[4]){
            set_speed = 12;
            robot_set_speed(set_speed,set_speed);
        }

        // second up
        if(loc[2] < mark[4] && loc[2] > mark[5]){
            set_speed = 10;
            robot_set_speed(set_speed,set_speed);
        }        

        // second after up
        if (loc[2] < mark[5] && loc[2] > mark[6]){
            set_speed = 10;
            robot_set_speed(set_speed, set_speed);
        }
        
        // slow down when going down
        if(vel[0] < -0.5 && loc[2] < mark[6]) {
          set_speed = 8.5;
          robot_set_speed(set_speed,set_speed);
          //printf("sample num:%d\n",sample_num);
          printf("slow down!\n"); 
        } 
        
        // last flat floor
        if(loc[2] < mark[7]){
            set_speed = 12;
            robot_set_speed(set_speed,set_speed);
        }
        
        
        // adjust direction 
        if(loc[0] > 0.009) {
          left_speed = set_speed - 0.5 * loc[0];
          right_speed = set_speed + 0.5 * loc[0];
        } else if (loc[0] < -0.009){
          left_speed = set_speed - 0.5 * loc[0];
          right_speed = set_speed + 0.5 * loc[0];
        } else {
          left_speed = set_speed;
          right_speed = set_speed;
        }
        robot_set_speed(left_speed, right_speed);
        
    }
  
    wb_robot_cleanup();
    fclose(fp);
    return 0;
}
