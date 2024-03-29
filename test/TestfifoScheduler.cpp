#include "../unity/src/unity.h"
#include "src/Scheduler.cpp"
#include "src/fifoScheduler.h"
#include "src/Queue.cpp"
#include "src/Queue.h"
#include "src/Student.cpp"
#include "src/Job.cpp"
#include "src/User.cpp"

void setUp(){}
void tearDown(){}

void test_fillReserved(void) {
	FIFOScheduler fifo;
	Queue* q = new Queue();
	struct Curriculum curr {};
	curr.budget = 1000;
	curr.instResourceCap = 1000;
	curr.permission = { 1,1,1,0,0 };
	curr.expoParameter = 10;
	int running = 30;  int runningTotal=30;//only looking at medium queue, which we will intialize to say is runnning 30 nodes of the 40 reserved nodes
	User* dummy = new Student(curr);
	Job* first = new Job(dummy, 1, 16 * 5, 0, 8, 8);
	Job* second = new Job(dummy, 1, 16 * 4, 0, 8, 8);
	Job* third = new Job(dummy, 1, 16 * 12, 0, 8, 8);
	double currentTime = 70; //well before weekend cutoff
	std::vector<Job*> results; //our return vector
	int state = 2; //start of in a state where there is no need to check time
	results = fifo.fillReserved_TEST(running, runningTotal, q, state, 3, WEEKENDCUTOFF, currentTime, medMin, currentTime);
	TEST_ASSERT_EQUAL_INT(0, results.size()); //testing to see that it broke out of method when queue is empty and returns empty result vector
	q->insertJob(first, currentTime);
	q->insertJob(second, currentTime);
	q->insertJob(third, currentTime);
	results = fifo.fillReserved_TEST(running, runningTotal, q, state, 3, WEEKENDCUTOFF, currentTime, medMin, currentTime);
	TEST_ASSERT_EQUAL_INT(2, results.size()); //should only have taken first two jobs
	TEST_ASSERT_EQUAL_INT(1, q->getSizeQueue()); //still one guy waiting in line
	TEST_ASSERT_EQUAL_INT(39, running); //should have increased nodes running to 39
	TEST_ASSERT_EQUAL_INT(39, runningTotal); //should have increased nodes running to 39
	running =30; runningTotal=30; //reset these to run new tests here
	state=3; //now needs to check times
	currentTime= 98; //6 hours before weekend cut off
	Job* fourth = new Job(dummy, 1, 16 * 10, 0, 4, 4);
	q->insertJob(fourth, currentTime);
	results = fifo.fillReserved_TEST(running, runningTotal, q, state, 3, WEEKENDCUTOFF, currentTime, medMin, currentTime);
	TEST_ASSERT_EQUAL_INT(1, results.size()); //should have just taken the one job this time
	TEST_ASSERT_EQUAL_INT(1, q->getSizeQueue()); //still one guy waiting in line
	TEST_ASSERT_EQUAL_INT(40, running); //should have increased nodes running to 36
	TEST_ASSERT_EQUAL_INT(40, runningTotal); //should have increased nodes running to 36
	
}

void test_oldestCheck(void) {
	FIFOScheduler fifo;
	struct Curriculum curr {};
	curr.budget = 10000; curr.instResourceCap = 10000; curr.permission = {1,1,1,1,0}; curr.expoParameter = 10;
	User* dummy = new Student(curr);
	std::array<Queue*, 5> qs;
	for (int i = 0; i < 5; i++) {
		qs[i] = new Queue();
	}
	fifo.addQueues(qs);
	double t = 94; //10 hours out from weekend
	Job* short1 = new Job(dummy, 0, 16, 0, 1, 1);
	Job* lrg1 = new Job(dummy, 2, 16 * 40, 0, 16, 16);
	Job* lrg2 = new Job(dummy, 2, 16 * 40, 0, 10, 10);
	qs[0]->insertJob(short1, 93); 
	qs[2]->insertJob(lrg1, 84); qs[2]->insertJob(lrg2, 88);
	
	double oldestTime = MAX_DOUBLE; //oldest time of next jobs in line, initialized to unreachable value
	int  oldest = 3; //queues index of oldest job, initialized to unreachable value
	int nShort = 0; int nMedium = 0; int nLarge = 0;
	int state = 2; //need to check large queue for reserved times
	Job* temp;

	temp = fifo.oldestCheck_TEST(oldest, oldestTime, nShort, state, 4, WEEKENDCUTOFF, 0, t);
	
	TEST_ASSERT_EQUAL_INT(0,oldest);
	TEST_ASSERT_EQUAL_FLOAT(93, oldestTime); //this is the time the one job entered the queue
	TEST_ASSERT_EQUAL_INT(0, nShort); //should be 0, index of first job in queue
	TEST_ASSERT_EQUAL_FLOAT(93, temp->getTimeEnteredQueue());
    temp = fifo.oldestCheck_TEST(oldest, oldestTime, nMedium, state, 3, WEEKENDCUTOFF, 1, t);
	TEST_ASSERT_EQUAL_INT(0, qs[1]->getSizeQueue()); //should be no jobs in this queue
	TEST_ASSERT_EQUAL_INT(0, oldest); //should still be unchanged because no jobs in this queue
	TEST_ASSERT_EQUAL_FLOAT(93, oldestTime); //this is the time the one job entered the queue
	TEST_ASSERT_EQUAL_INT(0, nMedium); //should be unchanged
    TEST_ASSERT(temp==nullptr);
	temp = fifo.oldestCheck_TEST(oldest, oldestTime, nLarge, state, 2, WEEKENDCUTOFF, 2, t);
	TEST_ASSERT_EQUAL_INT(2, oldest); //should now be considered queue with oldest job
	TEST_ASSERT_EQUAL_FLOAT(88, oldestTime); //should be the time the second job in line entered the queue
	TEST_ASSERT_EQUAL_INT(1, nLarge); //should be index of second job in queue
    TEST_ASSERT_EQUAL_FLOAT(88, temp->getTimeEnteredQueue());

	qs[2]->removeJob(nLarge, t+1);
	nLarge = 0;
	oldest = 0; oldestTime = MAX_DOUBLE;
	temp = fifo.oldestCheck_TEST(oldest, oldestTime, nLarge, state, 2, WEEKENDCUTOFF, 2, t);
	TEST_ASSERT_EQUAL_INT(0, oldest); //should not be changed
	TEST_ASSERT(oldestTime == MAX_DOUBLE); //should not be changed
	TEST_ASSERT_EQUAL_INT(-1,nLarge); //-1 to respresent no Jobs in Queue can be run at this time
	TEST_ASSERT_EQUAL_FLOAT(84, temp->getTimeEnteredQueue()); //should be the temp

}

void test_getJobsWeekday(void) {
	FIFOScheduler fifo;
	std::array<Queue*, 5> qs;
	for (int i = 0; i < 5; i++) {
		qs[i] = new Queue();
	}
	std::array<int, 5> running = { 0,0,0,0,0 }; int runningTotal = 0;
	fifo.addQueues(qs);
	struct Curriculum curr {};
	curr.budget = 100000; curr.instResourceCap = 100000; curr.permission = { 1,1,1,1,1}; curr.expoParameter = 10;
	User* dummy = new Student(curr);
	double t = 20; int state = 1;
	Job* gpu1 = new Job(dummy, 3, 16 * 5, 1, 8, 8);
	Job* gpu2 = new Job(dummy, 3, 16 * 5, 1, 8, 8);
	Job* gpu3 = new Job(dummy, 3, 16 * 5, 1, 8, 8); //wont be able to run
	qs[3]->insertJob(gpu1, 10); qs[3]->insertJob(gpu2, 11); qs[3]->insertJob(gpu3, 12);
	std::vector<Job*> jobs;
	jobs = fifo.getJobs(state, running, runningTotal, t, t);
	TEST_ASSERT_EQUAL_INT(2, jobs.size()); //should take first two
	TEST_ASSERT_EQUAL_INT(1, qs[3]->getSizeQueue());  //leave the one
	TEST_ASSERT_EQUAL_FLOAT(12, qs[3]->getJobAt(0)->getTimeEnteredQueue());  //left the right one
	Job* lrg1 = new Job(dummy, 2, 16 * 60, 0, 10, 10);
	Job* lrg2 = new Job(dummy, 2, 16 * 20, 0, 10, 10);
	t += 10; //t=30
	qs[2]->insertJob(lrg1, 13); qs[2]->insertJob(lrg2, 15);
	jobs = fifo.getJobs(state, running, runningTotal, t, t);
	TEST_ASSERT_EQUAL_INT(1, jobs.size());//should only have taken the first large job, other one will push large jobs over 50% of nodes
	TEST_ASSERT_EQUAL_FLOAT(13, jobs[0]->getTimeEnteredQueue()); //took the first one
	TEST_ASSERT_EQUAL_INT(1, qs[2]->getSizeQueue()); //left the second one
	TEST_ASSERT_EQUAL_INT(70, runningTotal);
	qs[2]->removeJob(0, t); //remove so next tests dont wait for this guy to run
	Job* med1 = new Job(dummy, 1, 16 * 12, 0, 8, 8);
	Job* med2 = new Job(dummy, 1, 16 * 12, 0, 8, 8);
	Job* med3 = new Job(dummy, 1, 16 * 12, 0, 8, 8);
	Job* med4 = new Job(dummy, 1, 16 * 8, 0, 8, 8);
	Job* med5 = new Job(dummy, 1, 16 * 8, 0, 8, 8); //wont be taken, even though theres still room on machine, b/c reserved space for short jobs
	Job* short1 = new Job(dummy, 0, 16 * 2, 0, 1, 1); //will be taken
	qs[1]->insertJob(med1, 15); qs[1]->insertJob(med2, 16); qs[1]->insertJob(med3, 17); qs[1]->insertJob(med4, 18); qs[1]->insertJob(med5, 19);
	qs[0]->insertJob(short1, 20);
	jobs = fifo.getJobs(state, running, runningTotal, t, t);
	TEST_ASSERT_EQUAL_INT(5, jobs.size());//should only have taken the first large job, other one will push large jobs over 50% of nodes
	TEST_ASSERT_EQUAL_FLOAT(19, qs[1]->getJobAt(0)->getTimeEnteredQueue()); //left this one behind
	TEST_ASSERT_EQUAL_INT(116, runningTotal); //should be this many running total
	TEST_ASSERT_EQUAL_INT(2, running[0]);
	TEST_ASSERT_EQUAL_INT(44, running[1]);
	TEST_ASSERT_EQUAL_INT(60, running[2]);
	TEST_ASSERT_EQUAL_INT(10, running[3]);
	TEST_ASSERT_EQUAL_INT(0, running[4]); //make sure all the individual running stats are correct

}

void test_getJobsWeekend(void) {
	FIFOScheduler fifo;
	std::array<Queue*, 5> qs;
	for (int i = 0; i < 5; i++) {
		qs[i] = new Queue();
	}
	std::array<int, 5> running = { 0,0,0,0,0 }; int runningTotal = 0;
	fifo.addQueues(qs);
	struct Curriculum curr {};
	curr.budget = 100000; curr.instResourceCap = 100000; curr.permission = { 0,0,0,0,1}; curr.expoParameter = 10;
	User* dummy = new Student(curr);
	double t = 124; int state = 5;
	Job* huge1 = new Job(dummy, 4, 16 * 60, 0, 30, 30);
	Job* huge2 = new Job(dummy, 4, 16 * 40, 0, 30, 30);
	Job* huge3 = new Job(dummy, 4, 16 * 20, 0, 60, 60); //wont run because it will go over cut off time
	Job* huge4 = new Job(dummy, 4, 16 * 60, 0, 30, 30); //wont run because not enough nodes free
	qs[4]->insertJob(huge1, t); qs[4]->insertJob(huge2, t); qs[4]->insertJob(huge3, t); qs[4]->insertJob(huge4, t);
	std::vector<Job*> jobs;
	jobs = fifo.getJobs(state, running, runningTotal, t, t);
	TEST_ASSERT_EQUAL_INT(2, jobs.size()); //should only take the first two
	TEST_ASSERT_EQUAL_INT(2, qs[4]->getSizeQueue()); //should leave two in queue
	TEST_ASSERT_EQUAL_INT(100, runningTotal); //total nodes of first two jobs
	TEST_ASSERT(runningTotal == running[4]); //these should be equivalent
	t += 2; //t=126
	Job * huge5 = new Job(dummy, 4, 16 * 20, 0, 30, 30); //can run in time and nodes, but wont because huge4 is first and is waiting for nodes to be freed up
	qs[4]->insertJob(huge5, t);
	jobs = fifo.getJobs(state, running, runningTotal, t, t);
	TEST_ASSERT_EQUAL_INT(0, jobs.size()); //none should be taken this turn
	TEST_ASSERT_EQUAL_INT(3, qs[4]->getSizeQueue()); //should now have 3 jobs waiting in queue
	TEST_ASSERT_EQUAL_INT(100, runningTotal); //should not be changed
	TEST_ASSERT(runningTotal == running[4]); //should not be changed
}


int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_fillReserved);
   	RUN_TEST(test_oldestCheck);
	RUN_TEST(test_getJobsWeekday);
	RUN_TEST(test_getJobsWeekend);
    return UNITY_END();
}
