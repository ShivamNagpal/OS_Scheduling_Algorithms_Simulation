#include "SchedulingAlgorithms.h"
#include "Scheduling_Output.h"
#include "VisualSection.h"
#include "Process.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <thread>
#include <glut.h>

const int n = 4;
Process processes[] = { Process(2,1,4), Process(1,0,8),  Process(3,2,9), Process(4,3,5) };
int jobPoolCursor = 0;

const int SCENE_INTRO = 0;
const int SCENE_ALGORITHMS = 1;
const int SCENE_SIMULATION = 2;
int currentScene = SCENE_INTRO;

const int ALGORITHM_FCFS = '1';
const int ALGORITHM_SJF_PREEMPTIVE = '2';
const int ALGORITHM_ROUND_ROBIN_PREEMPTIVE = '3';

const int HEIGHT = 500, WIDTH = 1000;
const float VIEWPORT_SCALING_FACTOR = 1.5f;
const int rows = 3;
int columns[] = { 1,2,2 };
VisualSection *visualSections[rows][2];
float visualSectionLabelColor[] = { 1.0,0.0,0.0 };
void *visualSectionLabelFont = GLUT_BITMAP_TIMES_ROMAN_24;
float outputLabelColor[] = { 0.0,0.0,0.0 };
void *outputLabelFont = GLUT_BITMAP_9_BY_15;
float outputColor[] = { 0.0,0.0,0.0 };
void *outputFont = GLUT_BITMAP_TIMES_ROMAN_24;
float processColors[][3] = { { 1,0,0 },{ 0.05f,0.9f,0.05f },{ 0,0,1 },{ 0.75f,0.75f,0 },{ 0,1,1 },{ 1,0,1 },{ 0.5f,0.5f,0.5f },{ 0.75f,0.25f,1 } };

float PROCESS_REPRESENTATION_TEXT_COLOR[] = { 1.0,1.0,1.0 };

int representRectangleX = 0, representRectangleY = 0;
SchedulingOutput *previousOutput = NULL;
int REPRESENT_RECTANGLE_SEPARATION = 5;
const int REPRESENT_RECTANGLE_HEIGHT = 80;
int REPRESENT_RECTANGLE_WIDTH = 20;

std::list<int> ganttChart;

std::thread *simulationThread = NULL;

int skipReadyQueueElements = 0;
int skipGanttChartElements = 0;

void bitmapTextRendering(const char * text, void *font, float color[], int x, int y);
void displayAlgorithmsScene();
void displayIntroScene();
void displaySimulationScene();
void drawVisualSectionLabels();
void drawOutputLabels();
void drawCPUOutputLabels();
void drawAverageOutputLabels();
void drawJobPool();
void drawPartitions();
void init();
void idleFunction();
void keyboardFunction(unsigned char key, int x, int y);
void representProcess(int processNumber, float color[], int *skipVariable, int maximumX);
void setStartingPositionForProcessRepresentation(int x, int y);
void renderCurrentOutput();
void renderCurrentCPUOutput(int process, int currentTime);
void renderCurrentAverageOutput(float waitingAverage, float turnAroundTimeAverage);
void renderReadyQueue(std::list<int> *arrivedProcesses);
void renderGanttChart();
void displayVisualSectionsAttribute();
void cleanUpMemory();
void cleanUpVisualSections();
void cleanSimulationThread();

void init()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, WIDTH, 0, HEIGHT);
	glMatrixMode(GL_MODELVIEW);
}

void displayVisualSectionsAttribute()
{
	printf("\nStart");
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns[i]; j++)
		{
			VisualSection *vs = visualSections[i][j];
			printf("%d %d %d %d %d %d \t", (i + 1), (j + 1), vs->startX, vs->startY, vs->width, vs->height);
		}
		printf("\n");
	}
	printf("End\n");
}

void displayIntroScene()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	float color[] = { 0,0,0 };
	bitmapTextRendering("Intro", GLUT_BITMAP_TIMES_ROMAN_24, color, 200, 200);
	glutSwapBuffers();
}

void displayAlgorithmsScene()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	float color[] = { 0,0,0 };
	bitmapTextRendering("Algorithm", GLUT_BITMAP_TIMES_ROMAN_24, color, 200, 200);
	glutSwapBuffers();
}

void displaySimulationScene()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	drawPartitions();
	drawVisualSectionLabels();
	drawOutputLabels();
	drawJobPool();
	renderCurrentOutput();
	glutSwapBuffers();
}

void drawPartitions()
{
	glPushAttrib(GL_LINE_WIDTH);
	glPushAttrib(GL_COLOR);
	glColor3f(0, 0, 0);
	glLineWidth(2);

	int rowHeight = HEIGHT / rows;
	int rowsStart[rows + 1];
	int currY = 0;
	rowsStart[0] = currY;
	for (int i = 1; i < rows; i++)
	{
		currY += rowHeight;
		rowsStart[i] = currY;
		glBegin(GL_LINES);
		glVertex2f(0, currY);
		glVertex2f(WIDTH, currY);
		glEnd();
	}
	rowsStart[rows] = HEIGHT;

	for (int i = 0; i < rows; i++)
	{
		if (columns[i] == 1)
		{
			visualSections[i][0] = new VisualSection(0, rowsStart[i], WIDTH, rowsStart[i + 1] - rowsStart[i]);
			continue;
		}

		int columnWidth = WIDTH / columns[i];
		int currX = 0;
		for (int j = 0; j < columns[i]; j++)
		{
			visualSections[i][j] = new VisualSection(currX, rowsStart[i], columnWidth, rowsStart[i + 1] - rowsStart[i]);
			currX += columnWidth;
			if (j == columns[i] - 1)
			{
				continue;
			}
			glBegin(GL_LINES);
			glVertex2f(currX, rowsStart[i]);
			glVertex2f(currX, rowsStart[i + 1]);
			glEnd();
		}
	}
	glPopAttrib();
	glPopAttrib();
}

void bitmapTextRendering(const char * text, void *font, float color[], int x, int y)
{
	glPushAttrib(GL_COLOR);
	glColor3fv(color);
	glRasterPos2f(x, y);

	for (int i = 0; text[i] != '\0'; i++)
	{
		glutBitmapCharacter(font, text[i]);
	}

	glPopAttrib();
}

void drawVisualSectionLabels()
{
	bitmapTextRendering("Ready Queue", visualSectionLabelFont, visualSectionLabelColor, visualSections[2][0]->startX + 10, visualSections[2][0]->startY + visualSections[2][0]->height - 34);
	bitmapTextRendering("Job Pool", visualSectionLabelFont, visualSectionLabelColor, visualSections[2][1]->startX + 10, visualSections[2][1]->startY + visualSections[2][1]->height - 34);
	bitmapTextRendering("CPU", visualSectionLabelFont, visualSectionLabelColor, visualSections[1][0]->startX + 10, visualSections[1][0]->startY + visualSections[1][0]->height - 34);
	bitmapTextRendering("Average", visualSectionLabelFont, visualSectionLabelColor, visualSections[1][1]->startX + 10, visualSections[1][1]->startY + visualSections[1][1]->height - 34);
	bitmapTextRendering("Gantt Chart", visualSectionLabelFont, visualSectionLabelColor, visualSections[0][0]->startX + 10, visualSections[0][0]->startY + visualSections[0][0]->height - 34);
}

void drawOutputLabels()
{
	drawCPUOutputLabels();
	drawAverageOutputLabels();
}

void drawCPUOutputLabels()
{
	int y = visualSections[1][0]->startY + visualSections[1][0]->height - 68;
	int xShift = visualSections[1][0]->width / 3;
	int xCurrent = visualSections[1][0]->startX + 20;
	bitmapTextRendering("Current Job", outputLabelFont, outputLabelColor, xCurrent, y);
	xCurrent += xShift;
	bitmapTextRendering("Current Time", outputLabelFont, outputLabelColor, xCurrent, y);
	xCurrent += xShift;
	bitmapTextRendering("Utilization", outputLabelFont, outputLabelColor, xCurrent, y);
}

void drawAverageOutputLabels()
{
	int y = visualSections[1][1]->startY + visualSections[1][1]->height - 68;
	int xShift = visualSections[1][1]->width / 2;
	int xCurrent = visualSections[1][1]->startX + 20;
	bitmapTextRendering("Average Waiting Time", outputLabelFont, outputLabelColor, xCurrent, y);
	xCurrent += xShift;
	bitmapTextRendering("Average Turn Around Time", outputLabelFont, outputLabelColor, xCurrent, y);
}

void drawJobPool()
{
	int x = visualSections[2][1]->startX + 20;
	int yCurrent = visualSections[2][1]->startY + visualSections[2][1]->height - 68;

	int xShift = visualSections[2][1]->width / 3;
	int xCurrent = x;
	int yShift = -24;

	bitmapTextRendering("Process Id", outputLabelFont, outputLabelColor, xCurrent, yCurrent);
	xCurrent += xShift;
	bitmapTextRendering("Burst Time", outputLabelFont, outputLabelColor, xCurrent, yCurrent);
	xCurrent += xShift;
	bitmapTextRendering("Arrival Time", outputLabelFont, outputLabelColor, xCurrent, yCurrent);

	yCurrent = yCurrent + yShift - 10;

	std::stringstream ss;
	int cursor;
	for (int i = 0; i < 3; i++)
	{
		cursor = (jobPoolCursor + i) % n;
		xCurrent = x;

		ss.str("");
		ss << processes[cursor].processId;
		bitmapTextRendering(ss.str().c_str(), outputLabelFont, outputLabelColor, xCurrent, yCurrent);
		xCurrent += xShift;

		ss.str("");
		ss << processes[cursor].burstTime;
		bitmapTextRendering(ss.str().c_str(), outputLabelFont, outputLabelColor, xCurrent, yCurrent);
		xCurrent += xShift;

		ss.str("");
		ss << processes[cursor].arrivalTime;
		bitmapTextRendering(ss.str().c_str(), outputLabelFont, outputLabelColor, xCurrent, yCurrent);
		xCurrent += xShift;

		yCurrent += yShift;
	}
	jobPoolCursor = (jobPoolCursor + 1) % n;

}

void renderCurrentOutput()
{

	if (outputReady)
	{

		clearSchedulingOutput(&previousOutput);

		previousOutput = new SchedulingOutput(*schedulingOutput);
		ganttChart.push_back(previousOutput->processNumber);
		outputReady = false;
		outputTaken = true;
	}

	if (previousOutput != NULL)
	{
		renderCurrentCPUOutput(previousOutput->processNumber, previousOutput->currentTime);
		renderReadyQueue(previousOutput->arrivedProcesses);
		renderCurrentAverageOutput(previousOutput->averageWaitingTime, previousOutput->averageTurnAroundTime);
	}

	renderGanttChart();
}

void renderCurrentCPUOutput(int process, int currentTime)
{
	int xShift = visualSections[1][0]->width / 3;
	int xCurrent = visualSections[1][0]->startX + 20;
	int y = visualSections[1][0]->startY + visualSections[1][0]->height - 102;
	std::stringstream ss;
	ss << "Job " << process;
	bitmapTextRendering(ss.str().c_str(), outputFont, outputColor, xCurrent, y);

	ss.str("");
	xCurrent += xShift;
	ss << currentTime << " -> " << (currentTime + 1);
	bitmapTextRendering(ss.str().c_str(), outputFont, outputColor, xCurrent, y);

	ss.str("");
	xCurrent += xShift;
	if (process != -1)
	{
		ss << "100%";
	}
	else
	{
		ss << "0%";
	}
	bitmapTextRendering(ss.str().c_str(), outputFont, outputColor, xCurrent, y);
}

void renderCurrentAverageOutput(float waitingAverage, float turnAroundTimeAverage)
{
	int y = visualSections[1][1]->startY + visualSections[1][1]->height - 102;
	int xShift = visualSections[1][1]->width / 2;
	int xCurrent = visualSections[1][1]->startX + 20;

	std::stringstream ss;
	ss.precision(2);
	ss << std::fixed << waitingAverage;
	bitmapTextRendering(ss.str().c_str(), outputFont, outputColor, xCurrent, y);
	ss.str("");

	xCurrent += xShift;
	ss << std::fixed << turnAroundTimeAverage;
	bitmapTextRendering(ss.str().c_str(), outputFont, outputColor, xCurrent, y);
}

void renderReadyQueue(std::list<int> *arrivedProcesses)
{
	setStartingPositionForProcessRepresentation(visualSections[2][0]->startX, visualSections[2][0]->startY + (visualSections[2][0]->height - 34) / 2);
	int maxX = visualSections[2][0]->startX + visualSections[2][0]->width;
	std::list<int>::iterator iterator = arrivedProcesses->begin();

	for (int i = 0; i < skipReadyQueueElements && iterator != arrivedProcesses->end(); i++)
	{
		iterator++;
	}

	for (; iterator != arrivedProcesses->end(); iterator++)
	{
		representProcess(*iterator, processColors[(*iterator - 1) % 8], &skipReadyQueueElements, maxX);
	}
}

void renderGanttChart()
{
	setStartingPositionForProcessRepresentation(visualSections[0][0]->startX, visualSections[0][0]->startY + (visualSections[0][0]->height - 34) / 2);
	int maxX = visualSections[0][0]->startX + visualSections[0][0]->width;
	std::list<int>::iterator iterator = ganttChart.begin();

	for (int i = 0; i < skipGanttChartElements && iterator != ganttChart.end(); i++)
	{
		iterator++;
	}

	for (; iterator != ganttChart.end(); iterator++)
	{
		representProcess(*iterator, processColors[(*iterator - 1) % 8], &skipGanttChartElements, maxX);
	}
}

void setStartingPositionForProcessRepresentation(int x, int y)
{
	representRectangleX = x;
	representRectangleY = y;
}

void representProcess(int processNumber, float color[], int *skipVariable, int maximumWidth)
{
	glPushAttrib(GL_LINE_WIDTH);
	glPushAttrib(GL_COLOR);

	glLineWidth(2.0);
	glColor3f(0, 0, 0);
	glBegin(GL_LINES);
	glVertex2f(representRectangleX, representRectangleY);
	glVertex2f(representRectangleX + REPRESENT_RECTANGLE_SEPARATION, representRectangleY);
	glEnd();

	representRectangleX += REPRESENT_RECTANGLE_SEPARATION;

	int deltaY = REPRESENT_RECTANGLE_HEIGHT / 2;
	glColor3fv(color);
	glBegin(GL_POLYGON);
	glVertex2f(representRectangleX, representRectangleY - deltaY);
	glVertex2f(representRectangleX, representRectangleY + deltaY);
	glVertex2f(representRectangleX + REPRESENT_RECTANGLE_WIDTH, representRectangleY + deltaY);
	glVertex2f(representRectangleX + REPRESENT_RECTANGLE_WIDTH, representRectangleY - deltaY);
	glEnd();

	glColor3f(0, 0, 0);
	std::stringstream ss;

	ss << processNumber;
	std::string s;
	ss >> s;
	bitmapTextRendering(s.c_str(), GLUT_BITMAP_9_BY_15, PROCESS_REPRESENTATION_TEXT_COLOR, representRectangleX + REPRESENT_RECTANGLE_WIDTH / 2 - 5, representRectangleY - deltaY + 10);

	representRectangleX += REPRESENT_RECTANGLE_WIDTH;
	glPopAttrib();
	glPopAttrib();

	if (representRectangleX + REPRESENT_RECTANGLE_SEPARATION + REPRESENT_RECTANGLE_WIDTH >= maximumWidth)
	{
		(*skipVariable)++;
	}
}

void keyboardFunction(unsigned char key, int x, int y)
{
	switch (currentScene)
	{
	case SCENE_INTRO:
		switch (key)
		{
		case 's':
		case 'S':
			glutDisplayFunc(displayAlgorithmsScene);
			currentScene = SCENE_ALGORITHMS;
			break;
		}
		break;
	case SCENE_ALGORITHMS:
		cleanSimulationThread();
		switch (key)
		{
		case ALGORITHM_FCFS:
			simulationThread = new std::thread(fcfs, processes, n);
			glutDisplayFunc(displaySimulationScene);
			currentScene = SCENE_SIMULATION;
			break;
		case ALGORITHM_SJF_PREEMPTIVE:
			simulationThread = new std::thread(sjfPreemptive, processes, n);
			glutDisplayFunc(displaySimulationScene);
			currentScene = SCENE_SIMULATION;
			break;
		case ALGORITHM_ROUND_ROBIN_PREEMPTIVE:
			simulationThread = new std::thread(roundRobinPreemptive, processes, n);
			glutDisplayFunc(displaySimulationScene);
			currentScene = SCENE_SIMULATION;
			break;
		}
		break;
	case SCENE_SIMULATION:
		break;
	}
}

int main()
{
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH * VIEWPORT_SCALING_FACTOR, HEIGHT * VIEWPORT_SCALING_FACTOR);
	glutCreateWindow("Simulation");
	init();
	glutDisplayFunc(displayIntroScene);
	glutKeyboardFunc(keyboardFunction);
	glutIdleFunc(idleFunction);

	int tempN = n;
	int multiplier = 0;
	while (tempN != 0)
	{
		multiplier++;
		tempN /= 10;
	}

	REPRESENT_RECTANGLE_WIDTH *= multiplier;
	REPRESENT_RECTANGLE_SEPARATION *= multiplier;

	/*std::thread th1(sjfPreemptive, processes, n);*/
	printf("Did I reach\n");
	glutMainLoop();

	cleanUpMemory();
}

void idleFunction()
{
	switch (currentScene)
	{
	case SCENE_SIMULATION:
		for (long i = 0; i < 200000000; i++)
			;
		glutPostRedisplay();
		break;
	default:
		glutPostRedisplay();
		break;
	}
}

void cleanUpVisualSections()
{
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns[i]; j++)
		{
			delete visualSections[i][j];
		}
	}
}

void cleanSimulationThread()
{
	if (simulationThread != NULL)
	{
		simulationThread->~thread();
		delete simulationThread;
		simulationThread = NULL;
	}
}

void cleanUpMemory()
{
	cleanUpVisualSections();
	clearSchedulingOutput(&previousOutput);
	clearSchedulingOutput(&schedulingOutput);
	cleanSimulationThread();
}
