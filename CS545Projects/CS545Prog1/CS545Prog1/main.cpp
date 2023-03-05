/* CS 445/545 Prog 1 for Jason Carnahan

	EXTRA CREDIT - Fowls include a face and tufts of hair.
	EXTRA CREDIT - Leftward animation included when fowls hit green tower.

	- Environment Description:
		The program utilizes GLUT's event driven paradigm to achieve 
	frame-by-frame animation in the Legacy OpenGL 4.3 backwards compatibility
	environment. The program established a 480px x 480px orthographic viewing
	enviroment with singlebuffering. Animation will target 30 frames per 
	second thus allowing about 33.333 milliseconds between frames. Events are
	registered in a Queue fashion where the user or system may enqueue events
	during the `glutMainLoop`. There are 3 major events used here: display, 
	keyboard, and timer events. See Program Flow and their respective 
	handlers for more information.

	- Program Flow:
		The program starts in `main` and initializes the OpenGL/GLUT environment 
	with `glutInit` and `my_setup`. It will register two event handlers:
	1. `glutDisplayFunc` registers `displayEventHandler` for display events.
	2. `glutKeyboardFunc` registers `keyboardEventHandler` for keyboard events.
	A console prompt appears and the `glutMainLoop` begins.

		The canvas will then be created and trigger a display event. The handler 
	clears the canvas, draws the specified objects, and flushes the buffer. 
	The program will then keep redrawing this until the user triggers a 
	keyboard event.

		When the user triggers the keyboard event, the handler then disables any 
	future keyboard events (this will be reenabled later) and registers the 
	appropriate timer event for left or right motion.

		When the timer event gets triggered, the handler checks if the animation 
	should end or continue. If it should continue, the handler makes 
	modificaitons to the global position variables, reregisters another timer
	event, and then triggers a display event. This will continue until the 
	animation should end then the keyboard event handler is then 
	reregistered. This cycle repeats until the user exits the program through
	the default system window exit function.
*/
#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"
#include "stdio.h"

// Canvas Dimensions
#define CANVAS_WIDTH 480
#define CANVAS_HEIGHT 480

// Pen and Canvas Colors
#define PEN_BLACK 0.0, 0.0, 0.0
#define PEN_YELLOWISH_ORANGE 1.0, 0.60, 0.0
#define PEN_RED 1.0, 0.0, 0.0
#define PEN_GREEN 0.0, 1.0, 0.0
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Necessary globals for scene tracking
int g_fowlXOffset = 0; // x offest tracker for moving the fowls
int HALF_FOWL_LENGTH = 14; // L in the fowl math description
int NUM_OF_FOWLS = 3;
int g_fowlHorizontalSeparation = 0; // Initial value; this is set in `drawFowl`
int g_fowlStartingXPos = 50;
bool g_animateRightwards = true;

// Misc. Details
char canvas_name[] = "CS 445/545 Prog 1 for Jason Carnahan"; // Window title
int targetFramerate = 30; // 30 frames per second, ~about 33.333 ms per frame
int timerDelay = (int) 1000 / targetFramerate; // Time delay between frames draws

// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
	Draws a fowl to the canvas. Fowls are defined to have "Yellowish Orange" 
	fethers and shaped to have 5 sides; 3 major sides will be 20px long
	and in the shape of a triangle. The base of the triangle will connect to
	the 2 minor sides, each 5px long. The base must also be parrallel to the 
	canvas width. Fowls will spawn in the left, center of the canvas.

	Fowl Point Math:
	Starting at the top point of the fowl and moving CCW, the points are A, B, C,
	D, E (top, left, bottom left, bottom right, right). The origin (x, y) of the 
	fowl is at the intersection of the line segment BE and the line segment where
	A perpendicularly meets the CD line segment.
	
	- Points ABE will define the upper triangle and points BCDE will define the 
	lower trapezoid. 
	- ABE is a right triangle where A is 90 degrees. The line segment AB must be 
	20px; therefore, the height of the triangle (L) is 20sin(45) ~= 14px.
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
	
	// Flock origin is defined as the origin of the first fowl
	int flockOriginX = g_fowlStartingXPos + g_fowlXOffset;
	int flockOriginY = CANVAS_HEIGHT / 2;
	g_fowlHorizontalSeparation = distanceBetween + HALF_FOWL_LENGTH * 2;
	
	// Draw the flock
	for (int i = 0; i < flockSize; i++) {
		glColor3f(PEN_YELLOWISH_ORANGE);
		// Included to reduce the size of the vertex inputs
		int calculatedSeparation = i * g_fowlHorizontalSeparation; 

		// Draw Fowl Body in CCW order as defined by math
		glBegin(GL_LINE_LOOP);
		glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + HALF_FOWL_LENGTH); // Top
		glVertex2i(flockOriginX - HALF_FOWL_LENGTH + calculatedSeparation, flockOriginY); // Left
		glVertex2i(flockOriginX - HALF_FOWL_LENGTH + 3 + calculatedSeparation, flockOriginY - 4); // Bottom Left
		glVertex2i(flockOriginX + HALF_FOWL_LENGTH - 3 + calculatedSeparation, flockOriginY - 4); // Bottom Right
		glVertex2i(flockOriginX + HALF_FOWL_LENGTH + calculatedSeparation, flockOriginY); // Right
		glEnd();

		// EXTRA CREDIT - Draw Fowl Hair Tufts
		glBegin(GL_LINES);
		glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + HALF_FOWL_LENGTH);
		glVertex2i(flockOriginX + 4 + calculatedSeparation, flockOriginY + HALF_FOWL_LENGTH + 3); // Right tuff
		glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + HALF_FOWL_LENGTH);
		glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + HALF_FOWL_LENGTH + 5); // Vertical tuff
		glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + 14.14);
		glVertex2i(flockOriginX - 4 + calculatedSeparation, flockOriginY + HALF_FOWL_LENGTH + 3); // Left tuff
		glEnd();

		// EXTRA CREDIT - Draw Fowl Face facing to our right
		glBegin(GL_LINES);
		glVertex2i(flockOriginX - 2.5 + calculatedSeparation, flockOriginY - 3);
		glVertex2i(flockOriginX + 2.5 + calculatedSeparation, flockOriginY); // Left mouth
		glVertex2i(flockOriginX + 2.5 + calculatedSeparation, flockOriginY);
		glVertex2i(flockOriginX + 5 + calculatedSeparation, flockOriginY - 3); // Right mouth
		glVertex2i(flockOriginX - 5 + calculatedSeparation, flockOriginY + 6);
		glVertex2i(flockOriginX + calculatedSeparation, flockOriginY + 3); // Fowl's right Eye
		glVertex2i(flockOriginX + 5 + calculatedSeparation, flockOriginY + 3);
		glVertex2i(flockOriginX + 7 + calculatedSeparation, flockOriginY + 5); // Fowl's left Eye
		glEnd();
	}
}

/*
	Draws a green tower on the right side of the screen. The tower is defined 
	to be 5px wide by 300px tall and connected by line segments. It is 10 units 
	from the right side of the screen.
*/
void drawTower() {
	int height = 300;
	int width = 5;
	int margin = 10;

	// Define in CCW order
	// The bottom is drawn at 1px so we can see the line
	glColor3f(PEN_GREEN);
	glBegin(GL_LINE_LOOP);
	glVertex2i(CANVAS_WIDTH - margin - width, height); // Top left
	glVertex2i(CANVAS_WIDTH - margin - width, 1); // Bottom left
	glVertex2i(CANVAS_WIDTH - margin, 1); // Bottom right
	glVertex2i(CANVAS_WIDTH - margin, height); // Top right
	glEnd();
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
	if ((g_fowlXOffset + (HALF_FOWL_LENGTH * 4 * NUM_OF_FOWLS) >= CANVAS_WIDTH + 1) && g_animateRightwards) {
		printf("Any Key Click Will Restart\n");
		glutKeyboardFunc(keyboardEventHandler); // Reenable keyboard handling
		g_animateRightwards = false;
		return;
	}

	// Stop animating when the left fowl reaches the starting position
	if ((g_fowlXOffset <= 0) && !g_animateRightwards) {
		printf("Any Key Click Will Start\n");
		glutKeyboardFunc(keyboardEventHandler); // Reenable keyboard handling
		g_animateRightwards = true;
		return;
	}

	// Continue animation
	switch (timerId) {
	case 1: // Fowls moving right
		glutTimerFunc(timerDelay, timerEventHandler, 1);
		g_fowlXOffset += 4; // Move the fowls right 4 units
		break;
	case 2: // Fowls moving left
		glutTimerFunc(timerDelay, timerEventHandler, 2);
		g_fowlXOffset -= 4; // Move the fowls left 4 units
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
void displayEventHandler(){
	glClearColor(CANVAS_BLACK); // Define background color
	glClear(GL_COLOR_BUFFER_BIT); // Clear the background

	// Draw the scene
	drawFowl(NUM_OF_FOWLS, 15);
	drawSlingshot();
	drawTower();

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
int main(int argc, char ** argv){
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_name);

	// Register Event Handlers
	glutDisplayFunc(displayEventHandler);
	glutKeyboardFunc(keyboardEventHandler);

	// Print to the user to begin!
	printf("Any Key Click Will Start\n");

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}