#include "../unity/src/unity.h"
#include "src/Machine.h"
#include "MockScheduler.h"
#include "MockScheduler.cpp"
#include "Scheduler.cpp"
#include "src/Job.cpp"
#include "src/Student.h"
#include "src/Queue.cpp"

void setUp(){}

void tearDown(){}

void test_initialization(void){
    Machine m = Machine();
    Scheduler* s;
    m.addScheduler(s);
    TEST_ASSERT(m.getScheduler() == s);
    std::array<Queue*,5> queues;
    Queue* q4 = new Queue();
    queues[4] = q4;
    for (int i =0; i<4;i++){
        Queue* q = new Queue();
        queues[i] = q;
    }
    m.addQueues(queues);
    TEST_ASSERT(m.getQueues().at(4)==q4);
}



void test_getJobsFromScheduler(void){
    Machine m = Machine();
    Scheduler* s = new MockScheduler();
    m.addScheduler(s);
    std::array<Queue*,5> queues;
    for (int i =0; i<5;i++){
        Queue* q = new Queue();
        queues[i] = q;
    }
    m.addQueues(queues);
    s->addQueues(queues);
    Student* student;
    Job* j1 = new Job(student,0,5,0,10.2,15.3);
    Job* j3 = new Job(student,0,5,0,10.2,15.3);
    Job* j5 = new Job(student,0,5,0,10.2,15.3);
    queues[0]->insertJob(j1);
    queues[2]->insertJob(j3);
    queues[4]->insertJob(j5);
    m.getJobsFromScheduler(0.1);
    TEST_ASSERT_EQUAL_FLOAT(0.1,j1->getTimeLeftQueue());
    TEST_ASSERT(m.getJobsRunning()[0] == j1);
    TEST_ASSERT(m.getJobsRunning()[1] == j3);
    TEST_ASSERT(m.getJobsRunning()[2] == j5);
}

void test_checkJobsRunning(void){
    Machine m = Machine();
    Scheduler* s = new MockScheduler();
    m.addScheduler(s);
    std::array<Queue*,5> queues;
    for (int i =0; i<5;i++){
        Queue* q = new Queue();
        queues[i] = q;
    }
    m.addQueues(queues);
    s->addQueues(queues);
    Student* student;
    Job* j1 = new Job(student,0,5,0,1.2,1.3);
    Job* j3 = new Job(student,2,5,0,1.2,1.3);
    Job* j5 = new Job(student,4,5,0,1.2,1.3);
    queues[0]->insertJob(j1);
    m.getJobsFromScheduler(0);
    queues[2]->insertJob(j3);
    m.getJobsFromScheduler(5);
    queues[4]->insertJob(j5);
    m.getJobsFromScheduler(10);
    m.checkJobsRunning(4);
    TEST_ASSERT_EQUAL_INT(2,m.getJobsRunning().size());
    TEST_ASSERT(m.getJobsRunning()[0] == j3);
    TEST_ASSERT(m.getJobsRunning()[1] == j5);


    m.checkJobsRunning(6.2);
    TEST_ASSERT_EQUAL_INT(1,m.getJobsRunning().size());
    TEST_ASSERT(m.getJobsRunning()[0] == j5);
    TEST_ASSERT_EQUAL_FLOAT(5,j3->getWaitTime());
    TEST_ASSERT_EQUAL_FLOAT(5,m.getWaitTimeByQueue()[2]);
}

void test_setMachineStatus(void){
    Machine m = Machine();
    m.setMachineStatus(STATE5);
    TEST_ASSERT_EQUAL_INT(5, m.getMachineStatus());
    m.setMachineStatus(STATE4);
    TEST_ASSERT_EQUAL_INT(4, m.getMachineStatus());
    m.setMachineStatus(STATE3);
    TEST_ASSERT_EQUAL_INT(3, m.getMachineStatus());
    m.setMachineStatus(STATE2);
    TEST_ASSERT_EQUAL_INT(2, m.getMachineStatus());
    m.setMachineStatus(STATE1);
    TEST_ASSERT_EQUAL_INT(1, m.getMachineStatus());
}

void test_collector(void){
    Machine m = Machine();
    Scheduler* s = new MockScheduler();
    m.addScheduler(s);
    std::array<Queue*,5> queues;
    for (int i =0; i<5;i++){
        Queue* q = new Queue();
        queues[i] = q;
    }
    m.addQueues(queues);
    s->addQueues(queues);
    Student* student;
    Job* j1 = new Job(student,0,5,0,1.2,1.3);
    Job* j2 = new Job(student,3,6,1,4,5);
    queues[0]->insertJob(j1);
    m.getJobsFromScheduler(0);
    queues[3]->insertJob(j2);
    m.getJobsFromScheduler(5);
    TEST_ASSERT_EQUAL_INT(2,m.getRunningTotal());
    TEST_ASSERT_EQUAL_INT(1,m.getRunning()[0]);
    m.collector(j1);
    TEST_ASSERT_EQUAL_INT(1,m.getRunningTotal());
    TEST_ASSERT_EQUAL_INT(0,m.getRunning()[0]);
    m.collector(j2);
    TEST_ASSERT_EQUAL_INT(0,m.getRunningTotal());
    TEST_ASSERT_EQUAL_INT(1,m.getProcessedByQueue()[0]);
    TEST_ASSERT_EQUAL_INT(0,m.getProcessedByQueue()[1]);
    TEST_ASSERT_EQUAL_INT(0,m.getProcessedByQueue()[2]);
    TEST_ASSERT_EQUAL_INT(1,m.getProcessedByQueue()[3]);
    TEST_ASSERT_EQUAL_INT(0,m.getProcessedByQueue()[4]);

    TEST_ASSERT_EQUAL_FLOAT(0,m.getWaitTimeByQueue()[0]);
    TEST_ASSERT_EQUAL_FLOAT(0,m.getWaitTimeByQueue()[1]);
    TEST_ASSERT_EQUAL_FLOAT(0,m.getWaitTimeByQueue()[2]);
    TEST_ASSERT_EQUAL_FLOAT(5,m.getWaitTimeByQueue()[3]);
    TEST_ASSERT_EQUAL_FLOAT(0,m.getWaitTimeByQueue()[4]);

    TEST_ASSERT_EQUAL_FLOAT(3.25,m.getTurnaroundRatioSummed());
    TEST_ASSERT_EQUAL_FLOAT(5.2,m.getMachineHoursConsumed());
    TEST_ASSERT_EQUAL_FLOAT(4*MACHINE_COST_GPU+1.2*MACHINE_COST, m.getPricePaid());
}

void test_resetMetrics(void){
    Machine m = Machine();
    Scheduler* s = new MockScheduler();
    m.addScheduler(s);
    std::array<Queue*,5> queues;
    for (int i =0; i<5;i++){
        Queue* q = new Queue();
        queues[i] = q;
    }
    m.addQueues(queues);
    s->addQueues(queues);
    Student* student;
    Job* j1 = new Job(student,0,5,0,1.2,1.3);
    queues[0]->insertJob(j1);
    m.getJobsFromScheduler(5);
    m.checkJobsRunning(10);
    m.resetMetrics();

    TEST_ASSERT_EQUAL_INT(0,m.getProcessedByQueue()[0]);
    TEST_ASSERT_EQUAL_FLOAT(0,m.getWaitTimeByQueue()[0]);
    TEST_ASSERT_EQUAL_FLOAT(0,m.getTurnaroundRatioSummed());
    TEST_ASSERT_EQUAL_FLOAT(0,m.getMachineHoursConsumed());
    TEST_ASSERT_EQUAL_FLOAT(0,m.getPricePaid());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_initialization);
    RUN_TEST(test_getJobsFromScheduler);
    RUN_TEST(test_checkJobsRunning);
    RUN_TEST(test_setMachineStatus);
    RUN_TEST(test_collector);
    RUN_TEST(test_resetMetrics);
    return UNITY_END();
}