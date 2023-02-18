/* CS 445/545 Prog 2 for Jason Carnahan

	
*/
#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"

// Canvas Dimensions and drawing constants
#define CANVAS_WIDTH 500
#define CANVAS_HEIGHT 500
#define PADDING 50

// Pen and Canvas Colors
#define PEN_BLACK 0.0, 0.0, 0.0
#define PEN_YELLOWISH_ORANGE 1.0, 0.60, 0.0
#define PEN_RED 1.0, 0.0, 0.0
#define PEN_BURLYWOOD 0.87, 0.72, 0.53
#define PEN_GREEN 0.0, 1.0, 0.0
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Necessary globals for scene tracking
int g_xOffset = 0; // x offest tracker for moving the fowls
int g_halfFowlInteriorLength = 18; // L in the fowl math description
int g_numOfFowls = 3;
int g_fowlHorizontalSeparation = 0; // Initial value; this is set in `drawFowl`
int g_fowlStartingXPos = 50;
bool g_animateRightwards = true;

// Misc. Details
char canvas_Name[] = "CS 445/545 Prog 2 for Jason Carnahan"; // Window title
int targetFramerate = 30; // 30 frames per second, ~about 33.333 ms per frame
int timerDelay = (int)1000 / targetFramerate; // Time delay between frames draws

// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
	Draws a fowl to the canvas. Fowls are defined to have "Yellowish Orange"
	fethers and shaped to have 5 sides; 3 major sides will be 25px long
	and in the shape of a triangle. The base of the triangle will connect to
	the 2 minor sides, each 5px long. The base must also be parrallel to the
	canvas width. Fowls will spawn in the left, center of the canvas.

	Change: These fowls will also consume 3D space. The leading face will be at
	z = -1 and the other will be at z = -6 with lines connecting the vertices.

	Fowl Point Math:
	Starting at the top point of the fowl and moving CCW, the points are A, B, C,
	D, E (top, left, bottom left, bottom right, right). The origin (x, y) of the
	fowl is at the intersection of the line segment BE and the line segment where
	A perpendicularly meets the CD line segment.

	- Points ABE will define the upper triangle and points BCDE will define the
	lower trapezoid.
	- ABE is a right triangle where A is 90 degrees. The line segment AB must be
	25px; therefore, the height of the triangle (L) is 25sin(45) ~= 18px.
	- The line segment BC must be 5px; therefore, a 3-4-5 triangle will be used
	to define a vertical height (H) of 4px and a horizontal base (B) of 3px.

	With this information, we can define every point for a singular fowl about
	the origin by the OpenGL Right Hand cooridinate system:
	a = (x, y + L)
	b = (x - L, y)
	c = (x - L + B, y - H)
	d = (x + L - B, y - H)
	e = (x + L, y)

	flockSize - How many fowls to spawn; typically 3
	distanceBetween - The distance between fowls, typically 10 - 15px
*/
void drawFowl(int flockSize, int distanceBetween) {

	int frontZ = -1;
	int backZ = -6;
	int width3D = (int)abs(backZ - frontZ);

	// Flock origin is defined as the origin of the first fowl
	int flockOriginX = g_fowlStartingXPos + g_xOffset;
	int flockOriginY = CANVAS_HEIGHT / 2;
	g_fowlHorizontalSeparation = distanceBetween + g_halfFowlInteriorLength * 2;

	// Draw the flock
	for (int i = 0; i < flockSize; i++) {
		glColor3f(PEN_YELLOWISH_ORANGE);
		// Included to reduce the size of the vertex inputs
		int calculatedSeparation = i * g_fowlHorizontalSeparation;

		// Define Front Fowl shape vertices
		int frontTopX = flockOriginX + calculatedSeparation;
		int frontTopY = flockOriginY + g_halfFowlInteriorLength;
		int frontLeftX = flockOriginX - g_halfFowlInteriorLength + calculatedSeparation;
		int frontLeftY = flockOriginY;
		int frontBottomLeftX = flockOriginX - g_halfFowlInteriorLength + 3 + calculatedSeparation;
		int frontBottomLeftY = flockOriginY - 4;
		int frontBottomRightX = flockOriginX + g_halfFowlInteriorLength - 3 + calculatedSeparation;
		int frontBottomRightY = frontBottomLeftY;
		int frontRightX = flockOriginX + g_halfFowlInteriorLength + calculatedSeparation;
		int frontRightY = frontLeftY;

		// Draw front Fowl Body in CCW order as defined by math
		glBegin(GL_LINE_LOOP);
		glVertex3i(frontTopX, frontTopY, frontZ); // Top
		glVertex3i(frontLeftX, flockOriginY, frontZ); // Left
		glVertex3i(frontBottomLeftX, frontBottomLeftY, frontZ); // Bottom Left
		glVertex3i(frontBottomRightX, frontBottomRightY, frontZ); // Bottom Right
		glVertex3i(frontRightX, frontRightY, frontZ); // Right
		glEnd();

		// Draw back Fowl Body same as the front but in new Z and 3D width
		glBegin(GL_LINE_LOOP);
		glVertex3i(frontTopX + width3D, frontTopY, backZ); // Top
		glVertex3i(frontLeftX + width3D, flockOriginY, backZ); // Left
		glVertex3i(frontBottomLeftX + width3D, frontBottomLeftY, backZ); // Bottom Left
		glVertex3i(frontBottomRightX + width3D, frontBottomRightY, backZ); // Bottom Right
		glVertex3i(frontRightX + width3D, frontRightY, backZ); // Right
		glEnd();

		// Draw lines that connect the front and back polygons
		glBegin(GL_LINES);
		glVertex3i(frontTopX, frontTopY, frontZ);
		glVertex3i(frontTopX + width3D, frontTopY, backZ);
		glVertex3i(frontLeftX, frontLeftY, frontZ);
		glVertex3i(frontLeftX + width3D, frontLeftY, backZ);
		glVertex3i(frontBottomLeftX, frontBottomLeftY, frontZ);
		glVertex3i(frontBottomLeftX + width3D, frontBottomLeftY, backZ);
		glVertex3i(frontBottomRightX, frontBottomRightY, frontZ); 
		glVertex3i(frontBottomRightX + width3D, frontBottomRightY, backZ); 
		glVertex3i(frontRightX, frontRightY, frontZ);
		glVertex3i(frontRightX + width3D, frontRightY, backZ);
		glEnd();
	}
}

/*
	Draws a red "Y" slingshot on the left side of the screen. The fowls will be
	"launched" from this slingshot represented with line segments. There are no
	size spcifications other than the base must be on the ground.
*/
void drawSlingshot() {
	glColor3f(PEN_RED);

	// Top left rectanlge of Y
	glBegin(GL_LINE_LOOP);
	glVertex2i(g_fowlStartingXPos - 40, CANVAS_HEIGHT / 2 + 45); // Top left
	glVertex2i(g_fowlStartingXPos + 15, CANVAS_HEIGHT / 2 - 55); // Bottom left 
	glVertex2i(g_fowlStartingXPos + 25, CANVAS_HEIGHT / 2 - 50); // Bottom right 
	glVertex2i(g_fowlStartingXPos - 30, CANVAS_HEIGHT / 2 + 50); // Top right
	glEnd();

	// Top Right rectanlge of Y
	glBegin(GL_LINE_LOOP);
	glVertex2i(g_fowlStartingXPos + 30, CANVAS_HEIGHT / 2 + 50); // Top left
	glVertex2i(g_fowlStartingXPos + 15, CANVAS_HEIGHT / 2 - 50); // Bottom left
	glVertex2i(g_fowlStartingXPos + 25, CANVAS_HEIGHT / 2 - 55); // Bottom right
	glVertex2i(g_fowlStartingXPos + 40, CANVAS_HEIGHT / 2 + 45); // Top right
	glEnd();

	// Stem of Y
	glBegin(GL_LINE_LOOP);
	glVertex2i(g_fowlStartingXPos + 15, CANVAS_HEIGHT / 2 - 50); // Top left
	glVertex2i(g_fowlStartingXPos + 15, 1); // Bottom left
	glVertex2i(g_fowlStartingXPos + 25, 1); // Bottom right
	glVertex2i(g_fowlStartingXPos + 25, CANVAS_HEIGHT / 2 - 50); // Top right
	glEnd();
}

/*
	Draws three nests at the top right of the screen to fit each fowl. The nests 
	will be top-open rectangles connected to a branch below. I will define the
	color of the nests and branch to be a brown-ish color.
*/
void drawNests() {
	glColor3f(PEN_BURLYWOOD);

	int leftBranchX = CANVAS_WIDTH - g_halfFowlInteriorLength * 2 * g_numOfFowls - g_fowlHorizontalSeparation - 10;
	int branchY = CANVAS_HEIGHT - g_halfFowlInteriorLength - PADDING;

	// Branch with stems
	glBegin(GL_LINES);
	glVertex2i(CANVAS_WIDTH, branchY);
	glVertex2i(leftBranchX, branchY);
	glEnd();

	for (int i = 0; i < g_numOfFowls; i++) {
		int nestHeight = 10;
		int nestWidth = g_halfFowlInteriorLength * 2 + 15;
		int offset = (nestWidth + 5) * i;
		
		// Draw nests 
		glBegin(GL_LINE_STRIP);
		glVertex2i(leftBranchX + 10 + offset, branchY + nestHeight);
		glVertex2i(leftBranchX + 10 + offset, branchY + nestHeight/2);
		glVertex2i(leftBranchX + nestWidth + offset, branchY + nestHeight/2);
		glVertex2i(leftBranchX + nestWidth + offset, branchY + nestHeight);
		glEnd();

		// Draw Stems
		glBegin(GL_LINES);
		glVertex2i(leftBranchX + nestWidth/2 + offset, branchY + nestHeight/2);
		glVertex2i(leftBranchX + nestWidth/2 + offset, branchY);
		glEnd();
	}
}
// END SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/* Declaring handler so timer and keyboard handlers can reference each other */
void keyboardEventHandler(unsigned char key, int x, int y);

/**
	Event handler for timer events. Determines how to handle the ending of
	animations and the reregistering of additional timer events for
	Frame-by-Frame animation.
	Timer Events with ID 1 are animating to the right.
	Timer Events with ID 2 are animating to the left.

	timerId - Defines the ID of the timer event we are handling.
 */
void timerEventHandler(int timerId) {
	// Stop animating when the right fowl encounters the tower
	if ((g_xOffset + (g_halfFowlInteriorLength * 4 * g_numOfFowls) >= CANVAS_WIDTH + 1) && g_animateRightwards) {
		glutKeyboardFunc(keyboardEventHandler); // Reenable keyboard handling
		g_animateRightwards = false;
		return;
	}

	// Stop animating when the left fowl reaches the starting position
	if ((g_xOffset <= 0) && !g_animateRightwards) {
		glutKeyboardFunc(keyboardEventHandler); // Reenable keyboard handling
		g_animateRightwards = true;
		return;
	}

	// Continue animation
	switch (timerId) {
	case 1: // Fowls moving right
		glutTimerFunc(timerDelay, timerEventHandler, 1);
		g_xOffset += 4; // Move the fowls right 4 units
		break;
	case 2: // Fowls moving left
		glutTimerFunc(timerDelay, timerEventHandler, 2);
		g_xOffset -= 4; // Move the fowls left 4 units
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

/**
	Event handler for keyboard events. The keyboard is the main source to begin
	the timer events and start animation.

	key - The key ID that was pressed
	x - The X position of the cursor on the drawing canvas
	y - The Y position of the cursor on the drawing canvas
*/
void keyboardEventHandler(unsigned char key, int x, int y) {
	glutKeyboardFunc(NULL); // While animating, disable keyboard event handling

	switch (key) {
	default: // We accept any key press to begin animation 
		if (g_animateRightwards) glutTimerFunc(timerDelay, timerEventHandler, 1);
		else glutTimerFunc(timerDelay, timerEventHandler, 2);
	}
}

/**
	Event handler for display events. Controls the drawing of each frame by
	clearing the previous frame with a new background and drawing the objects of
	the scene in new positions. The `timerEventHandler` manually triggers this
	event handler by calling `glutPostRedisplay` after making positional
	modifications.
*/
void displayEventHandler() {
	glClearColor(CANVAS_BLACK); // Define background color
	glClear(GL_COLOR_BUFFER_BIT); // Clear the background

	// Draw the scene
	drawFowl(g_numOfFowls, 15);
	drawSlingshot();
	drawNests();

	// Execute the draw
	glFlush(); // Not including this caused issues for me
}
// END EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	This program draws three yellowish-orange fowls, a red 'Y' shaped slingshot,
	and a green tower to the screen. It will then prompt the user to press a key
	to begin animation. The fowls will animate in a strctly rightward motion
	until it encounters the tower. The fowls will then stop animating, and the
	program will reprompt the user for a key stroke to begin animating in a
	strictly leftward motion until it encounters its original position. The
	program will then repeat until the user exits.
*/
int main(int argc, char ** argv) {
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_Name);

	// Register Event Handlers
	glutDisplayFunc(displayEventHandler);
	glutKeyboardFunc(keyboardEventHandler);

	// Print to the user to begin!

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}