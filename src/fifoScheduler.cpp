
#include "fifoScheduler.h"
#include "Simulation.h"


FIFOScheduler::FIFOScheduler() = default;

std::vector<Job*> FIFOScheduler::fillReserved(int& running, int& runningTotal, Queue*& queue, int state, int stateCheck,double cutoffTime, double currentTime, int limitNodes, double systemTime) {

	std::vector<Job*> nextJobs; 

	//first checks if the number of nodes running for this type of job is less than its limit
	if (running < limitNodes) {
		//if so, we will try to fill the remaining reserved space on the machine for this type of job
		int n = 0;
		//while there is still space, will keep trying to fill it
		while (running < limitNodes) {
			//initially set the temp job to the first in its queue type
			Job* temp = queue->nextJob();
			//if its a nullptr, this queue is emptpy so break will take us to the end of the method
			if (temp == nullptr) break;
			//check if the current machine status is greater than or equal to the state that will require us to consider job times
			if (state >= stateCheck) {
				//if the reserved time of the first job in the queue will cause us to go over cutoffTime, this job will need to wait till next week, so in the meantime
				//we will check if any other jobs in this queue can fill the reserved space
				if (temp->getReservedTime() + currentTime > cutoffTime) {
					//we will check the queue with nextJobT for the position of the next job in line that can run in the time available
					n = queue->nextJobT(cutoffTime - currentTime);
					//if we find one, make temp Job equal to this job at position n in the queue
					if (n > 0) {
						temp = queue->getJobAt(n);
					}
					//if nextJobT returns -1, that means no job in the queue can run in the amount of time left, so we break because no jobs can fill the space this turn
					else
						break;
				}
			}
			//check the node requirements for our temp job to see if it will cause us to go over the limit for this job type
			if (temp->getNodes() + running <= limitNodes) {
				//if it dosent, we add it to our list and increment the running attribute values, then remove it from the queue
				nextJobs.push_back(temp);
				running += temp->getNodes();
				runningTotal += temp->getNodes();
				queue->removeJob(n, systemTime);
			}
			else
				break;
		}
	}
	return nextJobs;
}

std::vector<Job*> FIFOScheduler::fillReserved_TEST(int& running, int& runningTotal, Queue*& queue, int state, int stateCheck, double cutoffTime, double currentTime, int limitNodes, double systemTime){
	return fillReserved(running, runningTotal, queue, state, stateCheck, cutoffTime, currentTime, limitNodes, systemTime);		
}

Job* FIFOScheduler::oldestCheck(int& oldest, double& oldestTime, int& n, int state, int stateCheck, double cutoffTime, int queue, double currentTime) {

	Job* temp = queues[queue]->nextJob();
	bool flag = false;
	if (temp != nullptr) {
        if (state >= stateCheck) {
            if (temp->getReservedTime() + currentTime > cutoffTime) {
                //next job in this queue can't be run, so lets check for others
                n = queues[queue]->nextJobT(cutoffTime - currentTime);
                if (n > 0)
                    temp = queues[queue]->getJobAt(n);
                else
                    flag = true;//no jobs in this queue can be run at this time, so make it so it dosent check the times below
            }
        }
        if (temp->getTimeEnteredQueue() < oldestTime && !flag) {
            oldestTime = temp->getTimeEnteredQueue();
            oldest = queue;
        }
    }

	return temp;
}

Job* FIFOScheduler::oldestCheck_TEST(int& oldest, double& oldestTime, int& n, int state, int stateCheck, double cutoffTime, int queue, double currentTime) {
	return oldestCheck(oldest, oldestTime, n, state, stateCheck, cutoffTime, queue, currentTime);
}

std::vector<Job*> FIFOScheduler::getJobs(int state, std::array <int, 5>& running, int& runningTotal, double currentTime, double systemTime) {

	switch (state) {

	case 5:           //case for state 5, weekend opeteration

		return fillReserved(running[4], runningTotal, queues[4], state, 5, WEEKDAYCUTOFF, currentTime, totalNodes, systemTime);

	default:       //case for states 1-4, weekday opertaion

		std::vector<Job*> nextJobs;
		std::vector<Job*> temp;

		temp = fillReserved(running[0], runningTotal, queues[0], state, 4, WEEKENDCUTOFF, currentTime, shortMin, systemTime);

		nextJobs.insert(nextJobs.end(), temp.begin(), temp.end());

		temp = fillReserved(running[1], runningTotal, queues[1], state, 3, WEEKENDCUTOFF, currentTime, medMin, systemTime);

		nextJobs.insert(nextJobs.end(), temp.begin(), temp.end());

		temp = fillReserved(running[3], runningTotal, queues[3], state, 3, WEEKENDCUTOFF, currentTime, gpuNodes, systemTime);
		
		nextJobs.insert(nextJobs.end(), temp.begin(), temp.end());

		while (runningTotal - running[3] < totalNodes - gpuNodes) {

			double oldestTime = MAX_DOUBLE; //oldest time of next jobs in line, initialized to unreachable value
			int oldest = 3; //queues index of oldest job, initialized to unreachable value
			int nShort = 0; int nMedium = 0; int nLarge = 0;

			Job* temp0 = oldestCheck(oldest, oldestTime, nShort, state, 4, WEEKENDCUTOFF, 0, currentTime);

			Job* temp1 = oldestCheck(oldest, oldestTime, nMedium, state, 3, WEEKENDCUTOFF, 1, currentTime);

			Job* temp2 = oldestCheck(oldest, oldestTime, nLarge, state, 2, WEEKENDCUTOFF, 2, currentTime);

			if (temp0 == nullptr && temp1 == nullptr && temp2 == nullptr)
				break; //if there are no more jobs that can be run next, break out from scheduler


			if (oldest == 0) {
				if (temp0->getNodes() + running[0] + running[2] + gpuNodes + std::max(medMin, running[1]) <=
					totalNodes) {
					nextJobs.push_back(temp0);
					running[0] += temp0->getNodes();
					runningTotal += temp0->getNodes();
					queues[0]->removeJob(nShort, systemTime);
					temp0 = queues[0]->nextJob();
				}
				else
					break;
			}
			else if (oldest == 1) {
				if (temp1->getNodes() + running[1] + running[2] + gpuNodes + std::max(shortMin, running[0]) <=
					totalNodes) {
					nextJobs.push_back(temp1);
					running[1] += temp1->getNodes();
					runningTotal += temp1->getNodes();
					queues[1]->removeJob(nMedium, systemTime);
					temp1 = queues[1]->nextJob();
				}
				else
					break;
			}
			else if (oldest == 2) {
				if (temp2->getNodes() + running[2] + gpuNodes + std::max(medMin, running[1]) +
					std::max(shortMin, running[0]) <= totalNodes) {
					nextJobs.push_back(temp2);
					running[2] += temp2->getNodes();
					runningTotal += temp2->getNodes();
					queues[2]->removeJob(nLarge, systemTime);
					temp2 = queues[2]->nextJob();
				}
				else
					break;
			}

			else break;

		}
		return nextJobs;
	}
}