#include <list>

struct SchedulingOutput
{
	int currentTime;
	int processNumber;
	float averageWaitingTime;
	float averageTurnAroundTime;
	std::list<int> *arrivedProcesses;
};
