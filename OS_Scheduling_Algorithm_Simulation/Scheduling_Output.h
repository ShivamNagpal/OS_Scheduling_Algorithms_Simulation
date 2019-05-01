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

	SchedulingOutput(SchedulingOutput &output)
	{
		this->currentTime = output.currentTime;
		this->processNumber = output.processNumber;
		this->averageWaitingTime = output.averageWaitingTime;
		this->averageTurnAroundTime = output.averageTurnAroundTime;
		this->arrivedProcesses = new std::list<int>();
		for (std::list<int>::iterator iterator = output.arrivedProcesses->begin(); iterator != output.arrivedProcesses->end(); iterator++)
		{
			(this->arrivedProcesses)->push_back(*iterator);
		}
	}
};

static SchedulingOutput *schedulingOutput = NULL;
static volatile bool outputReady = false;
static volatile bool outputTaken = true;

static void clearSchedulingOutput(SchedulingOutput **output)
{
	if (*output != NULL)
	{
		if ((*output)->arrivedProcesses != NULL)
		{
			delete (*output)->arrivedProcesses;
			(*output)->arrivedProcesses = NULL;
		}
		delete (*output);
		*output = NULL;
	}
}
#endif // !SCHEDULING_OUTPUT_H_
