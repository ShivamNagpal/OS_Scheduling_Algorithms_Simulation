#include <list>
#ifndef SCHEDULING_OUTPUT_H_
#define SCHEDULING_OUTPUT_H_
struct SchedulingOutput
{
	int currentTime;
	int processNumber;
	float averageWaitingTime;
	float averageTurnAroundTime;
	std::list<int> *arrivedProcesses = NULL;

	SchedulingOutput(int currentTime, int processNumber, float averageWaitingTime, float averageTurnAroundTime)
	{
		this->currentTime = currentTime;
		this->processNumber = processNumber;
		this->averageWaitingTime = averageWaitingTime;
		this->averageTurnAroundTime = averageTurnAroundTime;
		arrivedProcesses = new std::list<int>();
	}
};

static SchedulingOutput *schedulingOutput = NULL;
static volatile bool isReady = false;
static volatile bool outputReady = false;
static volatile bool outputTaken = true;
static volatile bool isOver = false;

static void clearSchedulingOutput()
{
	if (schedulingOutput != NULL)
	{
		if (schedulingOutput->arrivedProcesses != NULL)
		{
			delete schedulingOutput->arrivedProcesses;
		}
		delete schedulingOutput;
	}
}
#endif // !SCHEDULING_OUTPUT_H_
