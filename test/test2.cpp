/**
 * Test: Checks the correctness of the library when used by multiple threads.
 */
#include "../src/knarr.hpp"

#include <stdio.h>
#include <unistd.h>
#include <omp.h>


#define CHNAME "ipc:///tmp/demo.ipc"

#define ITERATIONS 10000
#define NUM_THREADS 2
#ifndef TOLERANCE
#define TOLERANCE 0.1 // Between 0 and 1
#endif
#define CUSTOM_VALUE_0 2
#define CUSTOM_VALUE_1 5

// In microseconds
#define LATENCY 3000
#define MONITORING_INTERVAL 1000000


int main(int argc, char** argv){
    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << " [0(Monitor) or 1(Application)]" << std::endl;
        return -1;
    }
    if(atoi(argv[1]) == 0){
        knarr::Monitor mon(CHNAME);
        //std::cout << "[[Monitor]]: Waiting application start." << std::endl;
        mon.waitStart();
        //std::cout << "[[Monitor]]: Application started." << std::endl;
        knarr::ApplicationSample sample;
        usleep(MONITORING_INTERVAL);
        while(mon.getSample(sample)){
            std::cout << "Received sample: " << sample << std::endl;

            double expectedLatency = LATENCY*1000; // To nanoseconds
            double expectedUtilization = ((double)LATENCY / ((double) (LATENCY))) * 100;
            if(abs(expectedLatency - sample.latency)/(double) expectedLatency > TOLERANCE){
                std::cerr << "Expected latency: " << expectedLatency <<
                             " Actual latency: " << sample.latency << std::endl;
                return -1;
            }
            if(abs(expectedUtilization - sample.loadPercentage)/(double) expectedUtilization > TOLERANCE){
                std::cerr << "Expected utilization: " << expectedUtilization <<
                             " Actual utilization: " << sample.loadPercentage << std::endl;
                return -1;
            }
            // We do not check anymore the number of task since when sampling is applied,
            // it is higly variable, i.e. we could miss a bunch of tasks.
            // Bandwidth and latency must still be correct nevertheless.
            usleep(MONITORING_INTERVAL);
        }
        uint expectedExecutionTime = ((ITERATIONS*(LATENCY))/(double)NUM_THREADS) / 1000; // Microseconds to milliseconds
        if(abs(expectedExecutionTime - mon.getExecutionTime())/expectedExecutionTime > TOLERANCE){
            std::cerr << "Expected execution time: " << expectedExecutionTime <<
                         " Actual execution time: " << mon.getExecutionTime() << std::endl;
            return -1;
        }
    }else{
        // Application. Use omp just to test the correctness when multiple
        // threads call begin/end.
        omp_set_num_threads(NUM_THREADS);
        knarr::Application app(CHNAME, NUM_THREADS);
#pragma omp parallel for
        for(size_t i = 0; i < ITERATIONS; i++){
            int threadId = omp_get_thread_num();
            app.begin(threadId);
            usleep(LATENCY);
            app.end(threadId);
        }
        app.terminate();
    }
    return 0;
}
