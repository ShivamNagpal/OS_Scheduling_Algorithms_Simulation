struct Process
{
	int processId, arrivalTime, burstTime, remainingTime, waitingTime;
	Process()
	{

	}
	Process(int processId, int arrivalTime, int burstTime)
	{
		this->processId = processId;
		this->arrivalTime = arrivalTime;
		this->burstTime = burstTime;
		this->remainingTime = burstTime;
		this->waitingTime = 0;
	}

	bool operator==(const Process &p1)const
	{
		return this == &p1;
	}
};

namespace ProcessComparator
{
	static bool sortSjfPreemptiveComparator(const Process &p1, const Process &p2)
	{
		if (p1.arrivalTime == p2.arrivalTime)
		{
			return p1.burstTime <= p2.burstTime;
		}
		return p1.arrivalTime <= p2.arrivalTime;
	}
}