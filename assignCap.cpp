#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <array>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdlib.h>

//exec: input: bash command, output: what prints on the cmd
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}


//input like:virsh domstats vm1 --cpu-total
//Domain: 'vm1'
//cpu.time=6173016809079111
//cpu.user=26714880000000
//cpu.system=248540680000000
float getCPUUsageTotal(std::string cmdoutput){
    std::string tmp = "cpu.time";
    std::size_t place = cmdoutput.find(tmp);
    place += 9;
    int tmp1 = 0;
    float cpuusage = 0;
    while (tmp1 == 0){
        if (isdigit(cmdoutput.at(place))){
            cpuusage *= 10;
            //printf("%c \n", cmdoutput.at(place));
            //printf("%f \n", (float) (cmdoutput.at(place)-'0'));
            cpuusage += (float) (cmdoutput.at(place)-'0');
            //printf("%f \n", cpuusage);
            place += 1;
        }
        else{
            tmp1 = 1;
        }
    }
    return(cpuusage);
}

//getCPUUsage: takes string machine_name, maybe N as input, returns cpu usage for the last N seconds
void getCPUUsage(const std::string machine_name, float second_sleep, float* usage, float* time_difference){
    int N = (int) (second_sleep*1000.0);
    std::string cmd;
    cmd = "virsh -c qemu:///system domstats  --cpu-total";
    cmd.insert(33, machine_name);
    const char* cmdchar = cmd.c_str();
    //std::string output = "Domain: 'vm1' \n cpu.time=6173016809079111 \n cpu.user=26714880000000 \n cpu.system=248540680000000";
    std::string output = exec(cmdchar);
    float tmp1 = getCPUUsageTotal(output)/1000000000; //in nanoseconds
    auto t1 = std::chrono::high_resolution_clock::now();
    //printf("%f \n", tmp1);
    std::this_thread::sleep_for (std::chrono::milliseconds(N));
    output = exec(cmdchar);
    //output = "Domain: 'vm1' \n cpu.time=6173054187484121 \n cpu.user=26714880000000 \n cpu.system=248540680000000";
    float tmp2 = getCPUUsageTotal(output)/1000000000; //in nanoseconds
    auto t2 = std::chrono::high_resolution_clock::now();
    //in milliseconds
    std::chrono::duration<double, std::milli> time_used = t2 - t1;
    *time_difference = time_used.count()/1000.0;
    //printf("time_diff=%f\n",*time_difference);
    *usage = (tmp2-tmp1)/(*time_difference);
    //return(usage, time_used.coun);
}

//pause Guest
void pauseGuest(std::string machine_name){
    std::string cmd = "virsh -c qemu:///system suspend " + machine_name;
    //printf("%s",cmd.c_str());
    exec(cmd.c_str());
}


void resumeGuest(std::string machine_name){
    std::string cmd = "virsh -c qemu:///system resume " + machine_name;
    //printf("%s",cmd.c_str());
    exec(cmd.c_str());
}

//periodically send pause signal to machine_name, then resume it in order to have cpuusage under cap (ex: 320 : if 4 processors: each work 80% of time).
void manageCPUUsage(std::string machine_name, float cap, float actualize_time = 0.1, int number_processors = 3){
    cap /= 100;
    while (1){
        float usage;
        float time_difference;
    //auto t1 = std::chrono::high_resolution_clock::now();
        getCPUUsage(machine_name, actualize_time, &usage, &time_difference);
    //auto t2 = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double, std::milli> time_used = t2 - t1;
    //float timed = (time_used.count()/1000.0 - time_difference)/time_difference;
    //printf("timediff = %f \n", timed);    
    //usage *= number_processors;
        //float usage = 320;
        //usage is sum of time used processor
        //if more than cap, we have to compensate by pausing for a set time
        if (usage > cap) {
            //if machine not paused
            //if(exec("virsh list").find("paused")==-1){
            pauseGuest(machine_name);
            //}

            float sleep_time = (time_difference * (usage - cap)) / cap;
            sleep_time *= 1000; //convert to milliseconds


            std::this_thread::sleep_for(std::chrono::milliseconds((int) sleep_time));
            //if machine not running
            //if(exec("virsh list").find("running")==-1){
            resumeGuest(machine_name);
            //}
            //sleep a bit more for safety
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
	    printf("sleep: %f \n", sleep_time);
        }
        printf("Variable usage: %f \n", usage);
        

        //printf("CPUUsage :%f\n", (usage/((float) number_processors)));

    }
}

//changes the cap of one single VM by killing the previous child and creating another one
/*
void changeCapSingleVM(float cap, std::string machine_name, float actualize_time = 0.1, pid_t pid = 0)
{
    kill(pid, SIGTERM);
    manageCPUUsage(machine_name, cap, actualize_time);
}*/


//Manages a single VM's CPU by forking : child manages and main continues its course; test_mode = 1 means test mode: see if usage should be multiplied or divided by processor number
void CPUUsageManagerSingleVM(float cap, std::string machine_name, std::string savefile = "../childpid.txt", int test_mode =1){
    if(exec("virsh list").find("running") == -1){
        resumeGuest(machine_name);
    }
    pid_t pid = fork();
    //child delete savefile, and writes the pid number on savefile to help killing it, then managesCPUUsage
    if (pid == 0){
        remove(savefile.c_str());
        std::ofstream pidfile;
        pidfile.open(savefile.c_str());
        pidfile << getpid();
        pidfile.close();
        manageCPUUsage(machine_name, cap);
    }
}

//check if pidfile is empty: if yes continue normally, else kill the previous pid
int main(int argc, char *argv[]) {
    std::string savefile = "../childpid.txt";
    char *vmname = argv[1];
    float cap = atof(argv[2]);
    int pid = -1;
    std::fstream pidfile;
    pidfile.open(savefile.c_str());
    //if file not empty
    if (!(pidfile.peek() == std::ifstream::traits_type::eof())){
        pidfile >> pid;
        //if pid exists (not artifact of previous runs): does not work occasionnaly (when other process has pid)
        if (0==kill(pid, 0)){
            kill(pid, SIGTERM);
        }
    }
    pidfile.close();
    CPUUsageManagerSingleVM(cap, vmname, savefile);
    printf("Cap of vm %s has been set as %f\n",vmname, cap);
}
