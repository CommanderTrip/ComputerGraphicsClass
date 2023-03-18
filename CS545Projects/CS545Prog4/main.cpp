#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"

// Canvas Dimensions
#define CANVAS_WIDTH 640
#define CANVAS_HEIGHT 640

// Pen and Canvas Colors
#define PEN_BLACK 0.0, 0.0, 0.0
#define PEN_ORANGE 1.0, 0.66, 0.0
#define PEN_YELLOW 1.0, 1.0, 0.0
#define PEN_WHITE 1.0, 1.0, 1.0
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Global X, Y, Z positions of objects
float g_fishPosition[3] = { 0.0, 0.0, -175.0 }; 
int g_fishHeading = 180; // Rotation over Z axis
float g_foodPosition[3] = { 0.0, 0.0, 0.0 };

// User Interface Text
int g_asciiBaseline = 48; // ASCII 48 is 0; the first number
int g_gameTimer = 30; // Game will last 30 seconds
int g_gameScore = 0;


// Misc. Details
char canvas_name[] = "CS 445/545 Prog 4 for Jason Carnahan"; // Window title
int targetFramerate = 25; // 25 frames per second, ~about 40 ms per frame
int timerDelay = (int)1000 / targetFramerate; // Time delay between frames draws
// UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	Changes the internal transformation matrix to be the one defined as inputs. 
	If nothing is passed, the identity matrix is applied.
*/
void transform(
	float translateX = 0.0, float translateY = 0.0, float translateZ = 0.0,
	float roatateTheta = 0.0,
	float rotateX = 0.0,	float rotateY = 0.0,	float rotateZ = 0.0,
	float scaleX = 1.0,		float scaleY = 1.0,		float scaleZ = 1.0
) {
	glLoadIdentity();
	glTranslatef(translateX, translateY, translateZ);
	glRotatef(roatateTheta, rotateX, rotateY, rotateZ);
	glScalef(scaleX, scaleY, scaleZ);
}

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
	glLoadIdentity();
	glColor3f(r, g, b);
	glRasterPos3i(x, y, z);
	for (int i = 0; i < strlen(m); i++) {
		glutBitmapCharacter(f, m[i]);
	}
	glFlush(); // Messages would not appear without this
}

void printIntToChar(int input, int xPos, int yPos, int zPos) {
	char dissect[3];
	dissect[0] = (char)(g_asciiBaseline + input / 10);
	dissect[1] = (char)
		( (((float)input / 10.0) - (int)(input / 10)) * 10 + g_asciiBaseline);
	dissect[2] = '\0';
	print(dissect, GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, xPos, yPos, zPos);
}
// END UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	The fish is to be modeled as an octahedron (wire frame) with a triangular 
	tail. That octahedron is 75 units wide, 25 units tall, and 25 units deep. The
	tail is 15 units wide and is attached to the body at the tip of the triangle.
*/
void drawFish() {
	int fishHalfX = 37;
	int fishHalfYZ = 12;
	glColor3f(PEN_ORANGE);

	// Fish Body
	transform(g_fishPosition[0], g_fishPosition[1], g_fishPosition[2],
		g_fishHeading, 0, 0, 1,
		fishHalfX, fishHalfYZ, fishHalfYZ);
	glutWireOctahedron();

	// Fish Tail
	transform(g_fishPosition[0], g_fishPosition[1], g_fishPosition[2],
		g_fishHeading+180, 0, 0, 1);
	glBegin(GL_LINE_LOOP);
	glVertex2f(fishHalfX, 0);
	glVertex2f(fishHalfX + 9, fishHalfYZ);
	glVertex2f(fishHalfX + 9, -fishHalfYZ);
	glEnd();
}

/*
	The food are white spheres of diameter 10 units.
*/
void drawFood() {

}

/*
	The fan is 100 units in size with 6 yellow blades, each a planar triangle 
	shape 50 units long. These triangles spin about the fan’s center. The fan’s 
	center is exactly in the center of the back of the fish tank. They are 
	equally angularly positioned, and spin in synchrony at a rate of 2 seconds to
	make one spin.
*/
void drawFan() {

}

/*
	Fish tank is a 250x250x250 axially aligned white wire cube with the front 
	face at z = -50.
*/
void drawTank() {
	transform(0,0,-175); // 250 / 2 = 125; 125 + 50 = 175
	glColor3f(PEN_WHITE);
	glutWireCube(250);
}

void drawUI() {
	print("SECONDS REMAINING:", GLUT_BITMAP_TIMES_ROMAN_24,PEN_WHITE, 0, 350, 0);
	printIntToChar(g_gameTimer, 135, 350, 0);
}
// END SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void displayEventHandler() {
	glClearColor(CANVAS_BLACK); // Define background color
	glClear(GL_COLOR_BUFFER_BIT); // Clear the background

	drawFish();
	drawTank();
	drawUI();

	// Execute the draw
	glFlush(); // Not including this caused issues for me
}

void keyboardEventHandler(unsigned char key, int x, int y) {
	switch (key) {
	case 'u': // Move Fish Up
	case 'U':
		g_fishHeading = 90;
		g_fishPosition[1] += 10;
		if (g_fishPosition[1] >= 125) g_fishPosition[1] = 125;
		break;
	case 'n': // Move Fish Down 
	case 'N':
		g_fishHeading = 270;
		g_fishPosition[1] -= 10;
		if (g_fishPosition[1] <= -125) g_fishPosition[1] = -125;
		break;
	case 'h': // Move Fish Left
	case 'H':
		g_fishHeading = 180;
		g_fishPosition[0] -= 10;
		if (g_fishPosition[0] <= -125) g_fishPosition[0] = -125;
		break;
	case 'j': // Move Fish Right
	case 'J':
		g_fishHeading = 0;
		g_fishPosition[0] += 10;
		if (g_fishPosition[0] >= 125) g_fishPosition[0] = 125;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void timerEventHandler(int timerId) {
	switch (timerId) {
	case 1: // Game timer countdown
		g_gameTimer--;
		if (g_gameTimer < 0) {
			glutKeyboardFunc(NULL);
			return;
		}
		glutTimerFunc(1000, timerEventHandler, 1);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}
// END EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	This is a simple game where a fish in a fish tank are fed.
*/
int main(int argc, char ** argv) {
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_name);

	glutDisplayFunc(displayEventHandler);
	glutKeyboardFunc(keyboardEventHandler);
	glutTimerFunc(1000, timerEventHandler, 1);

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}