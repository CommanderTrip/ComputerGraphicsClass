#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"

// Canvas Dimensions
#define CANVAS_WIDTH 600
#define CANVAS_HEIGHT 600

// Pen and Canvas Colors
#define PEN_BLACK 0.0, 0.0, 0.0
#define PEN_VIOLET 0.61, 0.07, 0.83
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Misc. Details
char canvas_name[] = "CS 445/545 Prog 3 for Jason Carnahan"; // Window title
int targetFramerate = 25; // 25 frames per second, ~about 40 ms per frame
int timerDelay = (int)1000 / targetFramerate; // Time delay between frames draws

// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void drawPeg() {
	glColor3f(PEN_VIOLET);
	glBegin(GL_POLYGON);
	glVertex3i(-200, -200, -1);
	glVertex3i(200, -200, -1);
	glVertex3i(200, 200, -1);
	glVertex3i(-200, 200, -1);
	glEnd();
}
// END SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
	Event handler for display events. Controls the drawing of each frame by
	clearing the previous frame with a new background and drawing the objects of
	the scene in new positions.
*/
void displayEventHandler() {
	glClearColor(CANVAS_BLACK); // Define background color
	glClear(GL_COLOR_BUFFER_BIT); // Clear the background

	drawPeg();

	// Execute the draw
	glFlush(); // Not including this caused issues for me
}
// END EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*
	Goal of the game to toss a ring-like object onto a peg. User will have 3 
	tosses and each ring that lands is 10 points.
*/
int main(int argc, char ** argv) {
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_name);

	// Register Event Handlers
	glutDisplayFunc(displayEventHandler);
	//	glutKeyboardFunc();
//	glutMouseFunc();

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}