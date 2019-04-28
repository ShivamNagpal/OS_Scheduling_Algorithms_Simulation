#include "SchedulingAlgorithms.h"
#include "Scheduling_Output.h"
#include "VisualSection.h"
#include "Process.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
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

void drawVisualSectionLabels();
void drawOutputLabels();
void drawCPUOutputLabels();
void drawPartitions();
void init();
void idleFunction();
void renderCurrentOutput();
void renderCurrentCPUOutput();
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

}

void drawCPUOutputLabels()
{
	int xShift = visualSections[1][0]->width / 3;
	int xCurrent = visualSections[1][0]->startX + 10;
	bitmapTextRendering("Current Job", outputLabelFont, outputLabelColor, xCurrent, visualSections[1][0]->startY + visualSections[1][0]->height - 34);
	xCurrent += xShift;
	bitmapTextRendering("Current Time", outputLabelFont, outputLabelColor, xCurrent, visualSections[1][0]->startY + visualSections[1][0]->height - 34);
	xCurrent += xShift;
	bitmapTextRendering("Utilization", outputLabelFont, outputLabelColor, xCurrent, visualSections[1][0]->startY + visualSections[1][0]->height - 34);
}

void renderCurrentOutput()
{
	float color[] = { 0.0,0.0,0.0 };
	if (!isOver)
	{
		while (!outputReady)
			;
		outputReady = false;
	}
	std::stringstream ss;
	ss << schedulingOutput->processNumber;
	printf("%d %s\n", schedulingOutput->processNumber, ss.str().c_str());
	bitmapTextRendering(ss.str().c_str(), GLUT_BITMAP_TIMES_ROMAN_24, color, 200, 200);
	outputTaken = true;
}

void renderCurrentCPUOutput()
{

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
	th1.join();
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

void cleanCurrentOutput()
{
	if (schedulingOutput != NULL)
	{
		delete schedulingOutput;
	}
}

void cleanUpMemory()
{
	cleanUpVisualSections();
	cleanCurrentOutput();
}