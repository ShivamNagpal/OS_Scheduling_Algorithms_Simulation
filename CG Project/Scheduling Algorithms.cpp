#include <algorithm>
#include <array>
#include <iostream>
#include <list>
#include <set>

#include "Process.h"

void sjfPreemptive(Process processes[], int n)
{
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

			if (minProcess->remainingTime <= 0)
			{
				arrivedProcesses.remove(*minProcess);
				checkForNextMinProcess = true;
				minProcess = NULL;
			}
		}

		currentTime += 1;

		if (arrivedProcesses.size() == 0 && currentTime > processes[n - 1].arrivalTime)
		{
			break;
		}
	}
	std::cout << "Current Time: " << currentTime << std::endl;
}

int main()
{
	const int n = 4;
	Process p[] = { Process(1,0,8), Process(2,1,4), Process(3,2,9), Process(4,3,5) };
	sjfPreemptive(p, n);
	return 0;
}
