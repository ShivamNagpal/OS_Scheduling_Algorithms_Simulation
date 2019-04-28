#include <algorithm>
#include <array>
#include <iostream>
#include <list>
#include <set>

#include "Process.h"
#include "Scheduling_Output.h"

#ifndef SCHEDULING_ALGORITHM_H_
#define SCHEDULING_ALGORITHM_H_

static void sjfPreemptive(Process inputProcesses[], const int n)
{
	Process *processes = new Process[n];
	for (int i = 0; i < n; i++)
	{
		processes[i] = inputProcesses[i];
	}
	std::sort(processes, processes + n, ProcessComparator::sortSjfPreemptiveComparator);
	std::set<int> arrivalTimes;
	for (int i = 0; i < n; i++)
	{
		arrivalTimes.insert(processes[i].arrivalTime);
	}
	std::list<Process> arrivedProcesses;

	int processCursor = 0, currentTime = 0;
	bool checkForNextMinProcess = false;
	Process *minProcess = NULL;

	while (true)
	{
		if (arrivalTimes.find(currentTime) != arrivalTimes.end())
		{
			while (processCursor < n && processes[processCursor].arrivalTime <= currentTime)
			{
				arrivedProcesses.push_back(processes[processCursor]);
				processCursor += 1;
			}
			checkForNextMinProcess = true;
		}

		if (checkForNextMinProcess)
		{
			int minRemainingTime = INT_MAX;
			for (std::list<Process>::iterator iterator = arrivedProcesses.begin(); iterator != arrivedProcesses.end(); ++iterator)
			{
				if (iterator->remainingTime < minRemainingTime)
				{
					minProcess = &(iterator._Ptr->_Myval);
					minRemainingTime = iterator->remainingTime;
				}
			}
			checkForNextMinProcess = false;
		}

		for (std::list<Process>::iterator iterator = arrivedProcesses.begin(); iterator != arrivedProcesses.end(); ++iterator)
		{
			if (&(iterator._Ptr->_Myval) == minProcess)
			{
				continue;
			}
			iterator->waitingTime += 1;
		}

		if (minProcess != NULL)
		{
			minProcess->remainingTime -= 1;

			std::cout << "Current Time: " << currentTime << ", Process: " << minProcess->processId << std::endl;

			if (schedulingOutput != NULL)
			{
				delete schedulingOutput;
			}
			schedulingOutput = new SchedulingOutput(currentTime, minProcess->processId, 0, 0);

			if (minProcess->remainingTime <= 0)
			{
				arrivedProcesses.remove(*minProcess);
				checkForNextMinProcess = true;
				minProcess = NULL;
			}

			for (std::list<Process>::iterator iterator = arrivedProcesses.begin(); iterator != arrivedProcesses.end(); ++iterator)
			{
				(schedulingOutput->arrivedProcesses)->push_back(iterator->processId);
			}
			isReady = true;
			outputReady = true;
			outputTaken = false;

		}
		printf("I'm Waiting\n");
		for (long i = 0; i < 750000000; i++)
			;

		while (!outputTaken)
			;


		currentTime += 1;


		if (arrivedProcesses.size() == 0 && currentTime > processes[n - 1].arrivalTime)
		{
			printf("I'm Exiting\n");
			break;
		}
	}
	std::cout << "Current Time: " << currentTime << std::endl;
	for (long i = 0; i < 750000000; i++)
		;

	delete[n] processes;
}

#endif // !SCHEDULING_ALGORITHM_H_
