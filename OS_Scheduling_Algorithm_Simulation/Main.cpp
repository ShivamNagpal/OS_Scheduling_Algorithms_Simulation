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

const int HEIGHT = 500, WIDTH = 1000;
const int rows = 3;
int columns[] = { 1,2,2 };
VisualSection *visualSections[rows][2];
float visualSectionLabelColor[] = { 1.0,0.0,0.0 };
void *visualSectionLabelFont = GLUT_BITMAP_TIMES_ROMAN_24;
float outputLabelColor[] = { 0.0,0.0,0.0 };
void *outputLabelFont = GLUT_BITMAP_TIMES_ROMAN_10;
float outputColor[] = { 0.0,0.0,0.0 };
void *outputFont = GLUT_BITMAP_TIMES_ROMAN_24;
float processColors[][3] = { { 1,0,0 },{ 0,1,0 },{ 0,0,1 },{ 0.75,0.75,0 },{ 0,1,1 },{ 1,0,1 },{ 0.5,0.5,0.5 },{ 0.75,0.25,1 } };

float PROCESS_REPRESENTATION_TEXT_COLOR[] = { 1.0,1.0,1.0 };

int representRectangleX = 0, representRectangleY = 0;
const int REPRESENT_RECTANGLE_SEPARATION = 20;
const int REPRESENT_RECTANGLE_HEIGHT = 100;
const int REPRESENT_RECTANGLE_WIDTH = 50;

std::list<int> ganttChart;

int skipReadyQueueElements = 0;
int skipGanttChartElements = 0;

void drawVisualSectionLabels();
void drawOutputLabels();
void drawCPUOutputLabels();
void drawAverageOutputLabels();
void drawPartitions();
void init();
void idleFunction();
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

void display()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	drawPartitions();
	drawVisualSectionLabels();
	drawOutputLabels();
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
	glRasterPos2f(x + 10, y - 34);

	for (int i = 0; text[i] != '\0'; i++)
	{
		glutBitmapCharacter(font, text[i]);
	}

	glPopAttrib();
}

void drawVisualSectionLabels()
{
	bitmapTextRendering("Ready Queue", visualSectionLabelFont, visualSectionLabelColor, visualSections[2][0]->startX, visualSections[2][0]->startY + visualSections[2][0]->height);
	bitmapTextRendering("Job Pool", visualSectionLabelFont, visualSectionLabelColor, visualSections[2][1]->startX, visualSections[2][1]->startY + visualSections[2][1]->height);
	bitmapTextRendering("CPU", visualSectionLabelFont, visualSectionLabelColor, visualSections[1][0]->startX, visualSections[1][0]->startY + visualSections[1][0]->height);
	bitmapTextRendering("Average", visualSectionLabelFont, visualSectionLabelColor, visualSections[1][1]->startX, visualSections[1][1]->startY + visualSections[1][1]->height);
	bitmapTextRendering("Gantt Chart", visualSectionLabelFont, visualSectionLabelColor, visualSections[0][0]->startX, visualSections[0][0]->startY + visualSections[0][0]->height);
}

void drawOutputLabels()
{
	drawCPUOutputLabels();
	drawAverageOutputLabels();
}

void drawCPUOutputLabels()
{
	int y = visualSections[1][0]->startY + visualSections[1][0]->height - 34;
	int xShift = visualSections[1][0]->width / 3;
	int xCurrent = visualSections[1][0]->startX + 10;
	bitmapTextRendering("Current Job", outputLabelFont, outputLabelColor, xCurrent, y);
	xCurrent += xShift;
	bitmapTextRendering("Current Time", outputLabelFont, outputLabelColor, xCurrent, y);
	xCurrent += xShift;
	bitmapTextRendering("Utilization", outputLabelFont, outputLabelColor, xCurrent, y);
}

void drawAverageOutputLabels()
{
	int y = visualSections[1][1]->startY + visualSections[1][1]->height - 34;
	int xShift = visualSections[1][1]->width / 2;
	int xCurrent = visualSections[1][1]->startX + 10;
	bitmapTextRendering("Average Waiting Time", outputLabelFont, outputLabelColor, xCurrent, y);
	xCurrent += xShift;
	bitmapTextRendering("Average Turn Around Time", outputLabelFont, outputLabelColor, xCurrent, y);
}

void renderCurrentOutput()
{
	float color[] = { 0.0,0.0,0.0 };
	if (!isOver)
	{
		while (!outputReady)
			;
		outputReady = false;
		ganttChart.push_back(schedulingOutput->processNumber);
		renderCurrentCPUOutput(schedulingOutput->processNumber, schedulingOutput->currentTime);
		renderReadyQueue(schedulingOutput->arrivedProcesses);
	}
	renderCurrentAverageOutput(schedulingOutput->averageWaitingTime, schedulingOutput->averageTurnAroundTime);
	renderGanttChart();

	outputTaken = true;
}

void renderCurrentCPUOutput(int process, int currentTime)
{
	int xShift = visualSections[1][0]->width / 3;
	int xCurrent = visualSections[1][0]->startX + 10;
	int y = visualSections[1][0]->startY + visualSections[1][0]->height - 68;
	std::stringstream ss;
	ss << "Job " << process;
	bitmapTextRendering(ss.str().c_str(), outputFont, outputColor, xCurrent, y);
	ss.str("");

	xCurrent += xShift;
	ss << currentTime << " -> " << (currentTime + 1);
	bitmapTextRendering(ss.str().c_str(), outputFont, outputColor, xCurrent, y);
}

void renderCurrentAverageOutput(float waitingAverage, float turnAroundTimeAverage)
{
	int y = visualSections[1][1]->startY + visualSections[1][1]->height - 68;
	int xShift = visualSections[1][1]->width / 2;
	int xCurrent = visualSections[1][1]->startX + 10;

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

	for (int i = 0; i < skipReadyQueueElements; i++)
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

	for (int i = 0; i < skipGanttChartElements; i++)
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
	glRasterPos2f(representRectangleX + 10, representRectangleY);
	std::stringstream ss;
	ss << processNumber;
	std::string s;
	ss >> s;
	bitmapTextRendering(s.c_str(), GLUT_BITMAP_TIMES_ROMAN_24, PROCESS_REPRESENTATION_TEXT_COLOR, representRectangleX + 10, representRectangleY);

	representRectangleX += REPRESENT_RECTANGLE_WIDTH;
	glPopAttrib();
	glPopAttrib();

	if (representRectangleX + REPRESENT_RECTANGLE_SEPARATION + REPRESENT_RECTANGLE_WIDTH >= maximumWidth)
	{
		(*skipVariable)++;
	}
}

int main()
{
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Simulation");
	init();
	glutDisplayFunc(display);
	glutIdleFunc(idleFunction);

	const int n = 4;
	Process p[] = { Process(2,1,4), Process(1,0,8),  Process(3,2,9), Process(4,3,5) };
	std::thread th1(sjfPreemptive, p, n);
	printf("Did I reach\n");
	glutMainLoop();
	//th1.join();
	isOver = true;

	cleanUpMemory();
}

void idleFunction()
{
	if (isReady)
	{
		glutPostRedisplay();
		isReady = false;
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

void cleanUpMemory()
{
	cleanUpVisualSections();
	clearSchedulingOutput();
}