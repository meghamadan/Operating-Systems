//
//  main.cpp
//  schedulerLab
//
//  Created by Megha Madan on 3/10/16.
//  Copyright © 2016 nyu.edu. All rights reserved.

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <queue>
#include <stack>
#include <algorithm>
#include <cstring>

using namespace std;

struct process {
    
    static const int UNSTARTED = -2;
    static const int RUNNING = -1;
    static const int BLOCKED = 0;
    static const int READY = 1;
    static const int TERMINATED = 2;
    
    int processID;
    
    int A; // Arrival time
    int B; // CPU burst time
    int C; // total CPU time
    int IO; // IO Burst multiplier
    
    //state of the processor can be any of the const ints above
    int state = UNSTARTED;
    
    int cpuBurst;
    int ioBurst;
    int cpuTime; //cpuTime of process that will be counted down
    int qCpuBurst;
    
    int finishTime; // time when process finishes computing
    int totalTime; // finishTime - A (time taken from process arrival to finish)
    
    int ioTime = 0; // time spent in blocked state
    int readyTime = 0; // time spent in ready state
    
    bool onQueue = false;
    
    double r;
    double T;
    double t;
    
    
    //constructors
    process(){processID = A = B = C = IO = 0;}
    
    process(int processNum, int A, int B, int C, int IO){
        this->processID = processNum;
        this->A = A;
        this->B = B;
        this->C = C;
        this->IO = IO;
    }
};

// compare processes by arrival for sorting
bool compareArrival(const process &a, const process &b){
    return a.A < b.A;
}

// compare processes by penalty for hprn
bool comparePenalty(const process &a, const process &b){
    
    if (a.r == b.r) {
        return a.processID < b.processID;
    }
    
    return a.r > b.r;
}

//simulate a cpu
struct cpu {
    
    int time;
    process current;
    int cpuUtilization;
    int ioUtilization;
    
    cpu(){
        time = 0;
        cpuUtilization = 0;
        ioUtilization = 0;
    }
};

/*****************************************************************************/


vector<process> processes;

vector<int> randomNums;

// determine if detailed output should be printed
bool verbose = false;

int numProcess;

int randomIndex = 0;

int finishTime = 0;

const char* fileName;

const int QUANTUM = 2;


void populateRandom(){
    
    ifstream fin("random-numbers.txt");
    if(!fin.is_open()) cout << "ERROR: Random number file missing." << endl;
    string x;
    while (fin >> x) {
        int y = stoi(x);
        randomNums.push_back(y);
    }
}

int getRandomNum(int b){
    int x = randomNums[randomIndex];
    randomIndex++;
    return 1 + (x % b);
}

void resetProcessID(){
    for(int i = 0; i < processes.size(); i++){
        processes[i].processID = i+1;
    }
}

/**
 * Method name: getProcesses
 * Description: reads input of fileName and populate processes vector
 * Parameters: fileName
 */
vector<process> getProcesses(const char* fileName){
    
    processes.clear();
    
    fstream fin(fileName);
    
    if (!fin.is_open())
        cout<< "ERROR: File not found." << endl;
    
    numProcess = 0;
    
    string x;
    fin >> x;
    
    // determine how many loops to take
    int end = atoi(x.c_str());
    
    fin >> x;
    
    while (numProcess < end) {
        
        int A = atoi(x.c_str());
        
        fin >> x;
        int B = atoi(x.c_str());
        
        fin >> x;
        int C = atoi(x.c_str());
        
        fin >> x;
        int IO = atoi(x.c_str());
        
        numProcess++;
        
        process y(numProcess, A, B, C, IO);
        y.cpuTime = C;
        processes.push_back(y);
        
        fin >> x;
        
        continue;
    }
    
    // print original and sorted input
    cout << setw(24) << left << "The original input was: " << setw(3) << numProcess;
    
    for(process p : processes){
        cout << p.A << " ";
        cout << p.B << " ";
        cout << p.C << " ";
        cout << p.IO << "  ";
    }
    
    // sort according to arrival
    sort(processes.begin(), processes.end(), compareArrival);
    resetProcessID();
    
    cout << endl << setw(24) << left << "The (sorted) input is: " << setw(3) << numProcess;
    
    for(process p : processes){
        cout << p.A << " ";
        cout << p.B << " ";
        cout << p.C << " ";
        cout << p.IO << "  ";
    }
    
    cout << endl << endl;
    fin.close();
    return processes;
}

/**
 * Method name: printDetailed
 * Description: when verbose flag is true, print detailed states of all the processes
 * Parameters: the current cycle number
 */
void printDetailed(int cycleNum){
    
    if(!verbose) return;
    
    cout << setw(10) << left << "\nBefore cycle " << setw(4) << right << cycleNum << ": ";
    
    // cycle through all processes
    for(int i = 0; i < processes.size(); i++){
        process p = processes[i];
        switch(p.state){
            case process::BLOCKED: cout << setw(12) << right << "blocked " << right << setw(2) << p.ioBurst ;
            break;
            case process::READY: cout << setw(12) << right << "ready " << right << setw(2) << 0 ;
            break;
            case process::RUNNING: cout << setw(12) << right << "running " << right << setw(2) << p.cpuBurst ;
            break;
            case process::TERMINATED: cout << setw(12) << right << "terminated " << right << setw(2) << 0 ;
            break;
            case process::UNSTARTED: cout << setw(12) << right << "unstarted " << right << setw(2) << 0;
            break;
                
        }
        // to add the final period for each cycle
        if (p.processID == processes.size())
            cout << ". \t";
    }
}

/**
 * Method name: terminated
 * Description: check if all the processes are terminated
 */
bool terminated(){
    
    for(process p : processes)
        if (p.state != process::TERMINATED) return false;
    
    return true;
}

/**
 * Method name: addToQueue
 * Description: goes through the processes vector and adds all unstarted processes to ready queue
 * Parameters: processes vector, cpu object, and ready queue
 */
queue<int> addToQueue(cpu cpu, queue<int> queue){
    
    for(int i = 0; i < processes.size(); i++){
        
        if(processes[i].A <= cpu.time &&
           (processes[i].state == process::UNSTARTED || processes[i].state == process::READY) &&
            processes[i].onQueue == false){
            
            processes[i].onQueue = true;
            processes[i].state = process::READY;
            queue.push(processes[i].processID - 1);
        }
    }
    return queue;
}

/**
 * Method name: addToDeque
 * Description: goes through the processes vector and adds all unstarted processes to ready deque
 * Parameters: processes vector, cpu object, and ready deque
 */
deque<int> addToDeque(cpu cpu, deque<int> deque){
    
    // add in opposite order because it is stack and we want same order as given in input
    for(int i = (int)processes.size() - 1; i >= 0; i--){
        
        if(processes[i].A <= cpu.time &&
           (processes[i].state == process::UNSTARTED || processes[i].state == process::READY) &&
           processes[i].onQueue == false){
            
            processes[i].onQueue = true;
            processes[i].state = process::READY;
            deque.push_back(processes[i].processID - 1);
        }
    }
    return deque;
}


/**
 * Method name: addToHPRQueue
 * Description: goes through the processes vector and adds all unstarted processes to ready queue 
 * and sorts the queue by penalty ratio.
 * Parameters: processes vector, cpu object, and ready deque
 */
deque<int> addToHPRQueue(cpu cpu, deque<int> deque){
    
    vector<process> processesCopy = processes;
    sort(processesCopy.begin(), processesCopy.end(), comparePenalty);
    
    for(int i = 0; i < processesCopy.size(); i++){
        
        if(processesCopy[i].A <= cpu.time &&
           (processesCopy[i].state == process::UNSTARTED || processesCopy[i].state == process::READY) &&
           processesCopy[i].onQueue == false){
            
            int x = processesCopy[i].processID - 1;
            processes[x].onQueue = true;
            processes[x].state = process::READY;
            deque.push_back(x);
        }
    }
    return deque;
}


deque<int> orderDeque(deque<int> deque){
    
    std::deque<process> temp;
    
    for (int i : deque)
        temp.push_back(processes[i]);
    
    sort(temp.begin(), temp.end(), comparePenalty);
    
    deque.clear();
    
    for (process p : temp) {
        int x = p.processID-1;
        deque.push_back(x);
    }
    return deque;
}


void computePenalty(cpu cpu){
    
    for(int i = 0; i < processes.size(); i++){
        processes[i].t = processes[i].C - processes[i].cpuTime;
        
        if(processes[i].t < 1.0){
            processes[i].t = 1.0;
        }
        processes[i].T = cpu.time - processes[i].A;
        processes[i].r = processes[i].T / processes[i].t;
    }
}

/**
 * Method name: checkBlocked
 * Description: combs through processes vector and checks for blocked processes
 * Parameters: cpu object
 */
void checkBlocked(cpu *cpu){
    
    bool blocked = false;
    
    for(int i = 0; i < processes.size(); i++){
        
        if(processes[i].state == process:: BLOCKED){
            blocked = true;
            processes[i].ioBurst--;
            processes[i].ioTime++;
            
            
            if(processes[i].ioBurst == 0)
                processes[i].state = process::READY;
    
        } else if (processes[i].state == process::READY){
            processes[i].readyTime++;
        }
    }
    
    if(blocked)
        cpu->ioUtilization++;
}

/**
 * Method name: printSummary
 * Description: prints final statistics for each process
 */
void printSummary(cpu cpu){
    
    int totalTurnaround = 0;
    int totalReady = 0;
    
    for (int i = 0; i < processes.size(); i++) {
        
        process p = processes[i];
        totalTurnaround += p.finishTime - p.A;
        totalReady += p.readyTime;
        
        cout << "Process " << i << ":\n";
        cout << "\t(A,B,C,IO) = (" << p.A << "," << p.B << "," << p.C << "," << p.IO << ")\n";
        cout << "\tFinishing Time: " << p.finishTime << endl;
        cout << "\tTurnaround Time: " << p.finishTime - p.A << endl;
        cout << "\tI/O time: " << p.ioTime << endl;
        cout << "\tWaiting time: " << p.readyTime << endl << endl;
    }
    
    cout << "Summary Data:\n";
    cout << "\tFinishing time: " << cpu.time << endl;
    cout << "\tCPU Utilization: " << (float)cpu.cpuUtilization/(float)cpu.time << endl;
    cout << "\tIO Utilization: " << (float)cpu.ioUtilization/(float)cpu.time << endl;
    cout << "\tThroughput: " << (float)processes.size()/(float)cpu.time * 100 << " processes per hundred cycles" << endl;
    cout << "\tAverage turnaround time: " << (float)totalTurnaround/(float)processes.size() << endl;
    cout << "\tAverage waiting time: " << (float)totalReady/(float)processes.size() << endl;
}

/**
 * Method name: fcfs
 * Description: simulates first come first serve scheduling algorithm on the data provided from previous methods
 */
void fcfs(){
    
    processes = getProcesses(fileName);
    queue<int> readyQueue; // add processes ids to queue
    cpu fcfsCPU;
    
    randomIndex = 0; // reset random incrementer for testing purposes
    
    if(verbose) cout << "This detailed printout gives the state and remaining burst for each process\n";
    
    printDetailed(fcfsCPU.time);
    
    
    while ( !terminated() ) {
        
        readyQueue = addToQueue(fcfsCPU, readyQueue);
        
        if(!readyQueue.empty()){
            
            int p = readyQueue.front() ; // -1 because array index = process id - 1
            readyQueue.pop();
            processes[p].onQueue = false;
            
            // calculate cpu and io burst time
            processes[p].cpuBurst = getRandomNum(processes[p].B);
            
            // if burst time is greater than total compute time, set it to compute time
            if(processes[p].cpuBurst > processes[p].cpuTime)
                processes[p].cpuBurst = processes[p].cpuTime;
            
            
            // run the process for one cpu burst
            while (processes[p].cpuBurst > 0) {
                
                processes[p].state = process:: RUNNING;

                fcfsCPU.time++;
                fcfsCPU.cpuUtilization++;
                
                printDetailed(fcfsCPU.time);
                
                checkBlocked(&fcfsCPU);
                
                readyQueue = addToQueue(fcfsCPU, readyQueue);

                processes[p].cpuTime--;
                processes[p].cpuBurst--;
                
                if(processes[p].cpuTime == 0){
                    processes[p].finishTime = fcfsCPU.time;
                    processes[p].state = process::TERMINATED;
                    break;
                }
            }
            
            if(processes[p].state == process::RUNNING){
                processes[p].state = process::BLOCKED;
                processes[p].ioBurst = getRandomNum(processes[p].IO);
            }
    
        } else { // no processes are ready - all blocked
            
            fcfsCPU.time++;
            
            printDetailed(fcfsCPU.time);
            checkBlocked(&fcfsCPU);
            readyQueue = addToQueue(fcfsCPU, readyQueue);
        }
    }
    
    cout << "\nThe scheduling algorithm used was First Come First Served\n\n";
    
    printSummary(fcfsCPU);
    
    processes.clear();
}

/**
 * Method name: rr
 * Description: simulates round robin scheduling algorithm on the data provided from previous methods
 */
void rr(){
    
    processes = getProcesses(fileName);
    queue<int> readyQueue; // add processes ids to queue
    cpu rrCPU;
    
    randomIndex = 0; // reset random incrementer for testing purposes
    
    if(verbose) cout << "This detailed printout gives the state and remaining burst for each process\n";
    
    printDetailed(rrCPU.time);
    
    while ( !terminated() ) {
        
        readyQueue = addToQueue(rrCPU, readyQueue);
        
        if(!readyQueue.empty()){
            
            int p = readyQueue.front() ; // -1 because array index = process id - 1
            readyQueue.pop();
            processes[p].onQueue = false;
            
            processes[p].state = process:: RUNNING;
            
            // calculate cpu burst time
            if (processes[p].qCpuBurst == 0) {
                processes[p].qCpuBurst = getRandomNum(processes[p].B);
                processes[p].cpuBurst = processes[p].qCpuBurst;
            }
            
            
            // if burst time is greater than total compute time, set it to remaining computation time
            if(processes[p].qCpuBurst > QUANTUM)
                processes[p].cpuBurst = QUANTUM;
            else if(processes[p].qCpuBurst > processes[p].cpuTime)
                processes[p].cpuBurst = processes[p].cpuTime;
            
            
            int quantum = 0;
            
            // run the process for one cpu burst
            while (processes[p].qCpuBurst > 0 && quantum < QUANTUM) {
                
                quantum++;
                rrCPU.time++;
                rrCPU.cpuUtilization++;
                
                printDetailed(rrCPU.time);
                
                checkBlocked(&rrCPU); //
                
                
                processes[p].cpuTime--;
                processes[p].cpuBurst--;
                processes[p].qCpuBurst--;
                
                // first check if a process has terminated
                if(processes[p].cpuTime == 0){
                    processes[p].finishTime = rrCPU.time;
                    processes[p].state = process::TERMINATED;
                    break;
                }
                
                // only add back to queue if quantum is not yet reached
                if(quantum < QUANTUM)
                    readyQueue = addToQueue(rrCPU, readyQueue);
                
                
            }
            
            // first check if process should be bloacked
            if(processes[p].qCpuBurst == 0 && processes[p].state == process::RUNNING){
                processes[p].state = process::BLOCKED;
                processes[p].ioBurst = getRandomNum(processes[p].IO);
            }
            // then check if the process has been preempted
            else if (processes[p].qCpuBurst > 0 && processes[p].state == process::RUNNING){
                processes[p].state = process::READY;
                processes[p].cpuBurst = QUANTUM;
            }
            
            
            
        } else { // no processes are ready - all blocked
            
            rrCPU.time++;
            
            printDetailed(rrCPU.time);
            checkBlocked(&rrCPU);
            readyQueue = addToQueue(rrCPU, readyQueue);
        }
    }
    
    cout << "\nThe scheduling algorithm used was Round Robin\n\n";
    printSummary(rrCPU);
    processes.clear();
}


/**
 * Method name: lcfs
 * Description: simulates last come first serve scheduling algorithm on the data provided from previous methods
 */
void lcfs(){
    
    processes = getProcesses(fileName);
    deque<int> readyDeque; // add processes ids to deque
    cpu lcfsCPU;
    
    randomIndex = 0; // reset random incrementer for testing purposes
    
    if(verbose) cout << "This detailed printout gives the state and remaining burst for each process\n";
    
    printDetailed(lcfsCPU.time);
    
    
    while ( !terminated() ) {
        
        readyDeque = addToDeque(lcfsCPU, readyDeque);
        
        if(!readyDeque.empty()){
            
            int p = readyDeque.back() ; // -1 because array index = process id - 1
            readyDeque.pop_back();
            processes[p].onQueue = false;
            
            // calculate cpu and io burst time
            processes[p].cpuBurst = getRandomNum(processes[p].B);
            
            // if burst time is greater than total compute time, set it to compute time
            if(processes[p].cpuBurst > processes[p].cpuTime)
                processes[p].cpuBurst = processes[p].cpuTime;
            
            
            // run the process for one cpu burst
            while (processes[p].cpuBurst > 0) {
                
                processes[p].state = process:: RUNNING;
                
                lcfsCPU.time++;
                lcfsCPU.cpuUtilization++;
                
                printDetailed(lcfsCPU.time);
                
                checkBlocked(&lcfsCPU);
                
                readyDeque = addToDeque(lcfsCPU, readyDeque);
                
                processes[p].cpuTime--;
                processes[p].cpuBurst--;
                
                if(processes[p].cpuTime == 0){
                    processes[p].finishTime = lcfsCPU.time;
                    processes[p].state = process::TERMINATED;
                    break;
                }
                
            }
            
            if(processes[p].state == process::RUNNING){
                processes[p].state = process::BLOCKED;
                processes[p].ioBurst = getRandomNum(processes[p].IO);
                
            }
            
        } else { // no processes are ready - all blocked
            
            lcfsCPU.time++;
            
            printDetailed(lcfsCPU.time);
            checkBlocked(&lcfsCPU);
            readyDeque = addToDeque(lcfsCPU, readyDeque);
        }
    }
    
    cout << "\nThe scheduling algorithm used was Last Come First Served\n\n";
    
    printSummary(lcfsCPU);
    
    processes.clear();
}

/**
 * Method name: hprn
 * Description: simulates highest penalty ratio next scheduling algorithm on the data provided from previous functions
 */
void hprn(){
    
    processes = getProcesses(fileName);
    deque<int> readyDeque; // add processes ids to queue
    cpu hprnCPU;
    
    randomIndex = 0; // reset random incrementer for testing purposes
    
    if(verbose) cout << "This detailed printout gives the state and remaining burst for each process\n";
    
    printDetailed(hprnCPU.time);
    
    while ( !terminated() ) {
        
        computePenalty(hprnCPU);
        readyDeque = addToHPRQueue(hprnCPU, readyDeque);
        readyDeque = orderDeque(readyDeque);
        
        if(!readyDeque.empty()){
            
            int p = readyDeque.front() ; // -1 because array index = process id - 1
            readyDeque.pop_front();
            processes[p].onQueue = false;
            
            // calculate cpu and io burst time
            processes[p].cpuBurst = getRandomNum(processes[p].B);
            
            // if burst time is greater than total compute time, set it to compute time
            if(processes[p].cpuBurst > processes[p].cpuTime)
                processes[p].cpuBurst = processes[p].cpuTime;
            
            // run the process for one cpu burst
            while (processes[p].cpuBurst > 0) {
                
                processes[p].state = process:: RUNNING;
                
                hprnCPU.time++;
                hprnCPU.cpuUtilization++;
                
                printDetailed(hprnCPU.time);
                
                checkBlocked(&hprnCPU);
                
                computePenalty(hprnCPU);
                readyDeque = addToHPRQueue(hprnCPU, readyDeque);
                readyDeque = orderDeque(readyDeque);
                
                
                processes[p].cpuTime--;
                processes[p].cpuBurst--;
                
                if(processes[p].cpuTime == 0){
                    processes[p].finishTime = hprnCPU.time;
                    processes[p].state = process::TERMINATED;
                    break;
                }
            }
            
            if(processes[p].state == process::RUNNING){
                processes[p].state = process::BLOCKED;
                processes[p].ioBurst = getRandomNum(processes[p].IO);
            }
            
        } else { // no processes are ready - all blocked
            
            hprnCPU.time++;
            
            printDetailed(hprnCPU.time);
            checkBlocked(&hprnCPU);
            
            computePenalty(hprnCPU);
            readyDeque = addToHPRQueue(hprnCPU, readyDeque);
            readyDeque = orderDeque(readyDeque);
        }
    }
    
    cout << "\nThe scheduling algorithm used was Highest Penalty Ratio Next\n\n";
    
    printSummary(hprnCPU);
    
    processes.clear();
}

void printBarrier(){
    cout << endl <<  "****************** NEXT ALGORITHM ***********************" << endl << endl;
}

/**
 * Description: calls functions to get input data and then runs the 4 algorithms on it
 */
int main(int argc, const char * argv[]) {

    populateRandom();
    
    fileName = NULL;

    // check if any arguments were given
    if(argc > 1){
        
        // check for verbose flag and then if there is also a file name given
        if(strcmp(argv[1], "--verbose") == 0 && argc == 3){
            verbose = true;
            fileName = argv[2];
        }
        else // otherwise there should be one argument and it is the file name
            fileName = argv[1];
        
    }
    else
        cout << "ERROR: Invalid arguments." << endl;
    
    
    
    // run the algorithms with fileName input and print results
    
    fcfs();
    
    printBarrier();
    
    rr();
    
    printBarrier();
    
    lcfs();
    
    printBarrier();
    
    hprn();
    
    return 0;
}
