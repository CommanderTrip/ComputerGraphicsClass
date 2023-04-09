/* CS 445/545 Prog 4 for Jason Carnahan

	EXTRA CREDIT - Fish heading
	EXTRA CREDIT - Fish enlargement

Architecture Statement:
	There are three major event handlers in this program. `displayEventHandler`,
	`keyboardEventHandler`, and `timerEventHandler` all handle their namesake
	events. On program start, `main` will register `displayEventHandler` to begin
	drawing objects to the screen and `keyboardEventHandler` to control the
	position of the fish on "U, H, J, N" keyboard events. `main` will also
	register the `timerEventHandler` with three different timer IDs: 1 for the
	overall game timer, 2 for independant fan rotation, and 3 for independantly
	spawning the fish food. When each these timers occur, they reregister
	themselves at different intervals. When the `displayEventHandler` draws the
	fish food, it check itself to see if the fish object has collided with the
	fish food object. If it has, another `timerEventHandler` will be registered
	with ID 4: timer to enlarge the fish. Then the food will begin its respawning
	action and the user's score will be increased. These timer events will loop
	until the game timer is zero upon which the timer events will all halt and
	the `keyboardEventHandler` is unregistered to freeze the game in place.
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

void drawUah() {
	glColor3f(PEN_SILVER);
	glPushMatrix();
	glLoadIdentity();

	// Start Conditions
	glTranslatef(0, 0, -400);
	glRotatef(uahRotation, 0, 1, 0);
	
	// Draw U
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

	// Draw A
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

	// Draw H
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