/* CS 445/545 Prog 5 for Jason Carnahan


Architecture Statement:

*/
#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"
#include "stdio.h"

// Meta Data
#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 800
#define FRAMERATE 25 // 25 frames per second, ~about 40 ms per frame

// Pen and Canvas Colors
#define PEN_SILVER 0.75, 0.75, 0.75
#define PEN_ORANGE 1.0, 0.66, 0.0
#define PEN_YELLOW 1.0, 1.0, 0.0
#define PEN_WHITE 1.0, 1.0, 1.0
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

int uahRotation = 0; // Degrees to rotate the UAH letters

// Misc. Details
char canvas_name[] = "CS 445/545 Prog 5 for Jason Carnahan"; // Window title
int timerDelay = (int)1000 / FRAMERATE; // Time delay between frames draws

void drawLetter(char letter) {
	switch (letter)
	{
	case 'u':
		glTranslatef(-350, 0, 0);
		for (int i = 0; i < 4; i++) {
			glTranslatef(0, -50, 0);
			glutSolidCube(50);
		}
		for (int i = 0; i < 3; i++) {
			glTranslatef(50, 0, 0);
			glutSolidCube(50);
		}
		for (int i = 0; i < 3; i++) {
			glTranslatef(0, 50, 0);
			glutSolidCube(50);
		}
		break;
	case 'a':
		glTranslatef(75, -200, 0);
		for (int i = 0; i < 4; i++) {
			glTranslatef(25, 50, 0);
			glutSolidCube(50);
		}
		glTranslatef(25, 50, 0);
		for (int i = 0; i < 4; i++) {
			glTranslatef(25, -50, 0);
			glutSolidCube(50);
		}
		glTranslatef(-175, 50, 0);
		for (int i = 0; i < 2; i++) {
			glTranslatef(50, 0, 0);
			glutSolidCube(50);
		}
		break;
	case 'h':
		glTranslatef(175, -100, 0);
		for (int i = 0; i < 4; i++) {
			glTranslatef(0, 50, 0);
			glutSolidCube(50);
		}
		glTranslatef(150, 50, 0);
		for (int i = 0; i < 4; i++) {
			glTranslatef(0, -50, 0);
			glutSolidCube(50);
		}
		glTranslatef(0, 50, 0);
		for (int i = 0; i < 2; i++) {
			glTranslatef(-50, 0, 0);
			glutSolidCube(50);
		}
		break;
	default:
		break;
	}
}

void drawUah() {
	glColor3f(PEN_SILVER);
	glPushMatrix();
	glLoadIdentity();

	// Start Conditions
	glTranslatef(0, 0, -400);
	glRotatef(uahRotation, 0, 1, 0);
	
	drawLetter('u');
	drawLetter('a');
	drawLetter('h');

	glPopMatrix();
}


void drawSpindle() {
	glColor3f(1.0, 0.0, 0.0);
	glPushMatrix();
	glLoadIdentity();
	
	glTranslatef(0, 18, -425);
	glRotatef(30, 1, 0, 0);	// X
	glRotatef(90, 0, 1, 0); // Y
	glRotatef(132, 0, 0, 1);// Z
	glScalef(43.3, 43.3, 43.3); // sqrt(3) * 43.3 = 75
	glutSolidTetrahedron(); // Docs say it is modeled with a radius of sqrt(3)

	glPopMatrix();
}


void timerEventHandler(int timerId) {
	switch (timerId) {
	case 1: 
		uahRotation += 4;
		if (uahRotation >= 360) uahRotation - 360;
		glutTimerFunc(timerDelay, timerEventHandler, 1);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

/*
	Handles display events and draws all the necessary objects to the screen.
*/
void displayEventHandler() {
	glClearColor(CANVAS_BLACK); // Define background color
	glClear(GL_COLOR_BUFFER_BIT); // Clear the background

	drawUah();
	drawSpindle();

	// Execute the draw
	glutSwapBuffers();
	glFlush(); // Not including this caused issues for me
}

/* void keh(unsigned char key, int x, int y) {
	switch (key) {
	default:
		spindle_rotate++;
		printf("%d\n", spindle_rotate);
	}
	glutPostRedisplay();
}*/

/*
	This is a simple game where a user controls a fish to collect food.
*/
int main(int argc, char ** argv) {
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_name);

	glutDisplayFunc(displayEventHandler);
	glutTimerFunc(timerDelay, timerEventHandler, 1);
	//glutKeyboardFunc(keh);

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}