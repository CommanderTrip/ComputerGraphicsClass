/* CS 445/545 Prog 3 for Jason Carnahan 

	Program Flow: 
		Program starts in Main and registers `displayEventHandler` and 
	`passiveMouseEventHandler`. `displayEventHandler` will clear the screen then 
	draw the UI and all scene objects to the screen on each display event. 
	`passiveMouseEventHandler` will register `keyboardEventHandler` when the user
	moves their mouse over the "VELOCITY" text box and handle keyboard input; if 
	the mouse is anywhere else, `changePerspective` is registered to only allow 
	the user to change to-and-from Orthographic and Perspecitve views with "p". 
	`passiveMouseEventHandler` will register `mouseClickEventHandler` when the 
	user moves their mouse over the "GO" button and handle mouse clicks. When the
	user clicks "GO", a timer event begins and registers the `timerEventHandler` 
	with ID 0 to begin ring movement. When it triggers, the `timerEventHandler` 
	will check 'g_ringYOffset' and 'g_yVelocity' to determine if the ring is on 
	the floor. After hitting the floor, the ring will stop moving and 
	`timerEventHandler` is registered with ID 1 to respawn the ring. After 
	checking if the ring is on the floor, `timerEventHandler` will check the ring
	position against the peg to see if we have a game winning condition and sets 
	the ring's horizontal velocity ('g_xVelocity') to 0 then continues downward 
	animation. This will continue until three tosses are made, then all handlers
	are unregistered and "GAME OVER" is printed.
*/

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
#define INITIAL_VELOCITY 5 // feet/sec
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
#define PEN_DEBUG 0.61, 0.07, 0.83
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Global Game Tracking
int g_tossRound = 3; // Tracks which toss the player is on 
float g_ringXOffset = 0;
float g_ringYOffset = 0;
bool g_tossInProgress = false;
bool g_currentView = 0; // 0 - Orthographic, 1 - Perspective 

// Physics Tracking
float g_xPosition = RING_START_X;
float g_yPosition = RING_START_Y;
float g_xVelocity = 0;
float g_yVelocity = 5;
float g_elapsedThrowTime = 0;

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
	glLoadIdentity();
	glColor3f(r, g, b);
	glRasterPos3i(x, y, z);
	for (int i = 0; i < strlen(m); i++) {
		glutBitmapCharacter(f, m[i]);
	}
	glFlush(); // Messages would not appear without this
}

/*
	Converts an integer to char using the ASCII table then uses the `print` 
	utility function to print the text to the screen.
*/
void printHorizontalVelocity() {
	int asciiBaseline = 48; // Int to Char follows the ASCII table

// Convert double digit number to char array using rounding and type casting
	char velocity[3];
	velocity[0] = (char)(asciiBaseline + g_xVelocity / 10);
	velocity[1] = (char)(
		(((float)g_xVelocity/10.0) - (int)(g_xVelocity/10)) *10 + asciiBaseline);
	velocity[2] = '\0';
	int zChange = !g_currentView ? -1 : -13;
	print(velocity, GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, -265, -235, zChange);
}

/*
	Updates the velocity of the ring based on it's acceleration then updates the 
	position of the ring based on it's velocity. There is not acceleration in the 
	horizontal direction; therefore, no change in velocity in that direction. The
	vertical direction is impacted by gravity; therefore, the velocity will 
	increase.
	v1 = integral(a(t)) = -32 * t + v0
	x = integral(v(t)) = -16 * t * t + v1 * t
*/
void updateRingMovement() {
	g_elapsedThrowTime += timerDelay/1000.0; // Update timestep in seconds

	// Update vertical velocity
	g_yVelocity = GRAVITY * g_elapsedThrowTime + INITIAL_VELOCITY;
	// Horizontal velocity does not update

	// Update Y position offset
	g_ringYOffset = g_yVelocity * g_elapsedThrowTime +
		GRAVITY/2 * (g_elapsedThrowTime*g_elapsedThrowTime);

	// Update X position offset, `if` for peg collision
	if (g_xVelocity) {
		g_ringXOffset = g_xVelocity * g_elapsedThrowTime +
			g_xVelocity/2 * (g_elapsedThrowTime*g_elapsedThrowTime);
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
		int zChange = !g_currentView ? -1 : -13;
		print("GAME OVER", GLUT_BITMAP_HELVETICA_18, PEN_RED, 
			-100, RING_START_Y-10, zChange);
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
	int zChange = !g_currentView ? -1 : -13;
	
	// Draw Velocity Box
	glBegin(GL_LINE_LOOP);
	glVertex3i(-290, -200, zChange);
	glVertex3i(-290, -250, zChange);
	glVertex3i(-210, -250, zChange);
	glVertex3i(-210, -200, zChange);
	glEnd();
	print("VELOCITY", GLUT_BITMAP_TIMES_ROMAN_10, PEN_WHITE, -290, -260, zChange);

	// Draw Go Box
	glBegin(GL_LINE_LOOP);
	glVertex3i(-200, -225, zChange);
	glVertex3i(-200, -250, zChange);
	glVertex3i(-140, -250, zChange);
	glVertex3i(-140, -225, zChange);
	glEnd();
	print("GO", GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, -188, -245, zChange);
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

	int zChange = !g_currentView ? -1 : -13;
	print("Enter Toss Velocity and then GO to Toss", GLUT_BITMAP_HELVETICA_18,
		PEN_WHITE, -150, 275, zChange);
	
	drawPeg();
	drawRing();
	drawControls();
	printHorizontalVelocity();

	// Execute the draw
	glFlush(); // Not including this caused issues for me
}

/*
	Handles timer events that are triggered for animation.
	Timer Event ID 0 - The ring is in the air; keep updating its position.
	Timer Event ID 1 - The ring is on the floor; respawn it at the start.
*/
void timerEventHandler(int timerId) {
	// Check if the ring hit the floor
	if (g_ringYOffset + RING_START_Y - 15 <= -300 && g_yVelocity != 0) {
		g_yVelocity = 0;
		glutTimerFunc(1000, timerEventHandler, 1);
		return;
	} 

	// Check if the ring hit the peg
	if ((g_ringYOffset + RING_START_Y - WIN_DISTANCE <= RING_Y_AXIS && 
		g_ringXOffset + RING_START_X + WIN_DISTANCE >= PEG_X_AXIS) && (
			g_ringYOffset + RING_START_Y + WIN_DISTANCE >= RING_Y_AXIS &&
			g_ringXOffset + RING_START_X - WIN_DISTANCE <= PEG_X_AXIS)
		) {
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
		g_elapsedThrowTime = 0;
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

/*
	Handles keyboard events. This is only registered if the user moves their 
	cursor over the "VELOCITY" text box. Then, the user can type any number into 
	the box, or any other key to 'backspace'.
*/
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
	case '0': // Adds value to text box
		if (g_xVelocity < 1) g_xVelocity += keyAsDecimal;
		else if (g_xVelocity >= 10) break;
		else g_xVelocity = (int)(g_xVelocity * 10) + keyAsDecimal;
		break;
	default: // Simulates a backspace
		g_xVelocity /= 10;
		g_xVelocity = (int)g_xVelocity;
		break;
	}
	glutPostRedisplay();
}

/*
	This is another keyboard event handler but specifically for if the user's 
	cursor is off of the "VELOCITY" text box.
*/
void changePerspective(unsigned char key, int x, int y) {
	glLoadIdentity();
	switch (key)
	{
	case 'p':
	case 'P':
		if (g_currentView == 0) { // Toggle to Perspective
			g_currentView = 1;
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(175.0, 1.0, 1.0, 100);
			glMatrixMode(GL_MODELVIEW);
		}
		else { // Toggle to Orthographic
			g_currentView = 0;
			glViewport(0, 0, (GLfloat)CANVAS_WIDTH, (GLfloat)CANVAS_HEIGHT);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(-300.0, 300.0, -300.0, 300.0, 1, 100.0);
			glMatrixMode(GL_MODELVIEW);
		}
	default:
		break;
	}
	glutPostRedisplay();
}

/*
	Handles if the user clicks their mouse in the "GO" button. When they do, 
	animation flag is set and the animation timer begins.
*/
void mouseClickEventHandler(int button, int state, int x, int y) {
	g_tossInProgress = true;
	glutTimerFunc(timerDelay, timerEventHandler, 0);
}

/*
	Handles passive mouse movement. Everytime the user moves their mouse, 
	different event handlers COULD be registered. Most of the time, 
	`changePerspective` is registered, but, when the user interacts with key 
	elements, special behaviors are registered with `keyboardEventHandler` and
	`mouseClickEventHandler`.
*/
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
		glutKeyboardFunc(changePerspective);
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