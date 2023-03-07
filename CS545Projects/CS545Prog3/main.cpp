#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"
#include "stdio.h"

// Canvas Dimensions
#define CANVAS_WIDTH 600
#define CANVAS_HEIGHT 600

// Physics and Win Condition Checks
#define GRAVITY -32 // feet/sec/sec
#define PEG_X_AXIS 250
#define RING_Y_AXIS -250
#define WIN_DISTANCE 15

// Ring Start Position
#define RING_START_X -55.0
#define RING_START_Y 260.0
#define RING_START_Z -50.0

// Pen and Canvas Colors
#define PEN_BLACK 0.0, 0.0, 0.0
#define PEN_WHITE 1.0, 1.0, 1.0
#define PEN_RED 1.0, 0.0, 0.0
#define PEN_YELLOW 1.0, 1.0, 0.0
#define PEN_ORANGE 1.0, 0.60, 0.0
#define PEN_AQUA 0.0, 1.0, 1.0
#define PEN_CLOUD 0.68, 0.82, 0.83
#define PEN_DEBUG 0.61, 0.07, 0.83 // Color for testing; not for production use
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Global Game Tracking
int g_tossRound = 3; // Tracks which toss the player is on 
char g_inputVelocity[2] = {};
float g_ringXOffset = 0;
float g_ringYOffset = 0;
bool g_tossInProgress = false;

// Physics Tracking
float g_xVelocity = 0;
float g_yVelocity = 5; // negative denotes direction
float g_throwTimeElapsed = 0;

// Misc. Details
char canvas_name[] = "CS 445/545 Prog 3 for Jason Carnahan"; // Window title
int targetFramerate = 25; // 25 frames per second, ~about 40 ms per frame
int timerDelay = (int)1000 / targetFramerate; // Time delay between frames draws

// UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	Prints a desired message to the screen with font, color, and location.

	m - Character array of the message to print.
	f - Pointer to the font type.
	r - Float of the red color value from 0 - 1.
	g - Float of the green color value from 0 - 1.
	b - Float of the blue color value from 0 - 1.
	x - Int of the X position in OpenGL space of the message.
	y - Int of the Y position in OpenGL space of the message.
	z - Int of the Z position in OpenGL space of the message.
*/
void print(const char m[], void* f, float r, float g, float b, 
	int x, int y, int z) {
	glColor3f(r, g, b);
	glRasterPos3i(x, y, z);
	for (int i = 0; i < strlen(m); i++) {
		glutBitmapCharacter(f, m[i]);
	}
	glFlush(); // Messages would not appear without this
}

void printHorizontalVelocity() {
	int asciiBaseline = 48; // Int to Char follows the ASCII table

// Convert double digit number to char array using rounding and type casting
	char velocity[3];
	velocity[0] = (char)(asciiBaseline + g_xVelocity / 10);
	velocity[1] = (char)(
		(((float)g_xVelocity/10.0) - (int)(g_xVelocity/10)) *10 + asciiBaseline);
	velocity[2] = '\0';
	print(velocity, GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, -265, -235, -1);
}

/*
	Updates the velocity of the ring based on it's acceleration then updates the 
	position of the ring based on it's velocity. There is not acceleration in the 
	horizontal direction; therefore, no change in velocity in that direction. The
	vertical direction is impacted by gravity; therefore, the velocity will 
	increase.
	v = integral(a(t)) = -32 * t
	x = integral(v(t)) = -16 * t * t
*/
void updateRingMovement() {
	g_throwTimeElapsed += timerDelay/1000.0; // Update timestep in seconds

	g_yVelocity = GRAVITY * g_throwTimeElapsed; // Update vertical velocity
	// Horizontal velocity does not update

	// Update Y position offset
	g_ringYOffset = GRAVITY/2 * (g_throwTimeElapsed*g_throwTimeElapsed);

	// Update X position offset
	if (g_xVelocity) {
		g_ringXOffset = g_xVelocity/2 * (g_throwTimeElapsed*g_throwTimeElapsed);
	}
}

// END UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	The peg will consist of 4, 12-unit-sized cubes stacked vertically. The center
	of the base will be at (x = 250,y = -300,z = -50). The top and bottom are 
	parallel to the xz-plane; the front and back are rotated 30 degrees.
*/
void drawPeg() {
	int cubeSize = 12;
	glColor3f(PEN_CLOUD);

	for (int i = 0; i <= 4; i++) {
		glLoadIdentity(); // Clear previous transformations
		glTranslatef(250.0, -300.0 + (cubeSize/2 + 1) + (i * cubeSize), -50.0);
		glRotatef(30.0, 0.0, 1.0, 0.0);
		glutWireCube(cubeSize);
	}
}

/*
	Each ring is modeled by a wire torus with an inner diameter of 45 units. The
	ring width is 15 units. Three rings will be generated with the colors YELLOW,
	ORANGE, and AQUA. The Center will be at (x = -55, y = 260, z = -50). The 
	player will view the torus at its side.
*/
void drawRing() {
	glLoadIdentity();
	glTranslatef(
		RING_START_X+g_ringXOffset, RING_START_Y+g_ringYOffset, RING_START_Z);
	glRotatef(90.0, 1.0, 0.0, 0.0);

	switch (g_tossRound) // Determine ring's color
	{
	case 3:
		glColor3f(PEN_YELLOW);
		break;
	case 2:
		glColor3f(PEN_ORANGE);
		break;
	case 1:
		glColor3f(PEN_AQUA);
		break;
	default:
		glColor3f(PEN_DEBUG);
		print("GAME OVER", GLUT_BITMAP_HELVETICA_18, PEN_RED, -50, 0, 10);
		return;
		break;
	}

	glutWireTorus(15/2, 45/2+15, 10, 10);	
}

/*
	In the lower left of the screen will be a box with the text label 'VELOCITY'
	below it. The user can position their mouse in this box an type in it. The 
	box must support two-digit integers. To the right of the velocity box must be
	a box labeled GO that starts animation.
*/
void drawControls() {
	glLoadIdentity();
	glColor3f(PEN_WHITE);
	
	// Draw Velocity Box
	glBegin(GL_LINE_LOOP);
	glVertex3i(-290, -200, -1);
	glVertex3i(-290, -250, -1);
	glVertex3i(-210, -250, -1);
	glVertex3i(-210, -200, -1);
	glEnd();
	print("VELOCITY", GLUT_BITMAP_TIMES_ROMAN_10, PEN_WHITE, -290, -260, -1);

	// Draw Go Box
	glBegin(GL_LINE_LOOP);
	glVertex3i(-200, -225, -1);
	glVertex3i(-200, -250, -1);
	glVertex3i(-140, -250, -1);
	glVertex3i(-140, -225, -1);
	glEnd();
	print("GO", GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, -188, -245, -1);
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

	print("Enter Toss Velocity and then GO to Toss", GLUT_BITMAP_HELVETICA_18,
		PEN_WHITE, -150, 275, -1);
	
	drawPeg();
	drawRing();
	drawControls();
	printHorizontalVelocity();

	// Execute the draw
	glFlush(); // Not including this caused issues for me
}

void timerEventHandler(int timerId) {
	if (g_ringYOffset + RING_START_Y - 7.5 <= -300 && g_yVelocity != 0) {
		// Hit the floor
		g_yVelocity = 0;
		glutTimerFunc(1000, timerEventHandler, 1);
		return;
	} 
	if ((g_ringYOffset + RING_START_Y - WIN_DISTANCE <= RING_Y_AXIS && 
		g_ringXOffset + RING_START_X + WIN_DISTANCE >= PEG_X_AXIS) && (
			g_ringYOffset + RING_START_Y + WIN_DISTANCE >= RING_Y_AXIS &&
			g_ringXOffset + RING_START_X - WIN_DISTANCE <= PEG_X_AXIS)
		) {
		// Landed on the peg
		g_xVelocity = 0;
	}

	switch (timerId)
	{
	case 0: // Keep moving the ring
		if (!g_yVelocity) return;
		updateRingMovement();
		glutTimerFunc(timerDelay, timerEventHandler, 0);
		break;
	case 1: // Respawn the Ring
		g_tossInProgress = false;
		g_tossRound--;
		g_throwTimeElapsed = 0;
		g_xVelocity = 0;
		g_yVelocity = 5;
		g_ringXOffset = 0;
		g_ringYOffset = 0;
		break;
	default:
		break;
	}
	
	glutPostRedisplay();
}

void keyboardEventHandler(unsigned char key, int x, int y) {
	int keyAsDecimal = (int)key - 48;
	switch (key) {
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '0':
		if (g_xVelocity < 1) g_xVelocity += keyAsDecimal;
		else if (g_xVelocity >= 10) break;
		else g_xVelocity = (int)(g_xVelocity * 10) + keyAsDecimal;
		break;
	default:
		g_xVelocity /= 10;
		g_xVelocity = (int)g_xVelocity;
		break;
	}
	glutPostRedisplay();
}

void mouseClickEventHandler(int button, int state, int x, int y) {
	g_tossInProgress = true;
	glutTimerFunc(timerDelay, timerEventHandler, 0);
}

void passiveMouseEventHandler(int x, int y) {
	if (g_tossInProgress || g_tossRound <= 0) {
		glutKeyboardFunc(NULL);
		glutMouseFunc(NULL);
		return;
	}
	// If the cursor is in the bounds of the Velocity box, handle typing
	if (x >= 10 && x <= 90 &&
		y <= 550 && y >= 500) {
		glutKeyboardFunc(keyboardEventHandler);
	}
	else {
		glutKeyboardFunc(NULL);
	}

	// If the cursor is in the bounds of the Go box, handle Clicking
	if (x >= 100 && x <= 160 &&
		y <= 550 && y >= 525) {
		glutMouseFunc(mouseClickEventHandler);
	}
	else {
		glutMouseFunc(NULL);
	}
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
	glutPassiveMotionFunc(passiveMouseEventHandler);

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}