/* CS 445/545 Prog 2 for Jason Carnahan
	EXTRA CREDIT - Limited Up/Down movement with Fuel
	EXTRA CREDIT - Pause Button Implemented

	- Environment Description: The program utilizes GLUT's event driven 
	paradigm to achieve frame-by-frame animation in the Legacy OpenGL 4.3 
	backwards compatibility environment. The program establishes a 500px x 500px 
	orthographic viewing environment with single buffering. Animation will target
	25 frames per second, thus allowing about 40 milliseconds between frames. 
	Events are registered in a Queue fashion where the user or system may enqueue
	events during the `glutMainLoop`. There are 4 major events used here: 
	display, keyboard, timer, and mouse click events. See Program Flow and their 
	respective handlers for more information.

	- Program Flow: The program starts in `main` and initializes the OpenGL/GLUT 
	environment. It will then register three event handlers: 
	`displayEventHandler` for display events, `keyboardEventHandler` for keyboard
	events, and `mouseClickHandler` for mouse click events. Then, `glutMainLoop` 
	begins.

		The canvas will be created and trigger the display event with the global 
	variable `g_gameStarted` as false. The canvas will prompt the user to press 
	'B' and the keyboard event listener will accept 'B' or 'b' then switch 
	`g_gameStarted` to true and draw the initial conditions of the game. The 
	inital game will show and when the user presses 'M', the keyboard event 
	listener will accept 'M' or 'm' and animate the fowls by changing the global 
	variables `g_animatingRight` and `g_fowlsLaunched` to true.

		Each frame, the fowls will move rightwards and check for each End Game 
	conditions. If any End Game condition is met, movement will stop by changing 
	the global variable `g_gameOver` to true, and a win or loss message will be 
	printed with all other input disabled. While the game is occuring, the user 
	can press 'U' or 'N' to animate the fowls up or down. The keyboard event 
	listener will accept 'U'/'u' and 'N'/'n' and animate the vertical movement 
	over a series of intermittent steps to achieve a smooth motion. Every 
	vertical movement will deplete some fuel and when fuel reaches 0, vertical 
	movement is disabled via an IF. If the user left clicks within the region 
	defined by the pause button, `g_animatingRight` is set to false to stop the 
	fowls from moving right but vertical movement will still be enabled while 
	there is still fuel. The game will continue infinitely until the user closes 
	the window or reaches any End Game condition.
*/

#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"

// Canvas Dimensions
#define CANVAS_WIDTH 500
#define CANVAS_HEIGHT 500

// Pen and Canvas Colors
#define PEN_BLACK 0.0, 0.0, 0.0
#define PEN_WHITE 1.0, 1.0, 1.0
#define PEN_YELLOW 1.0, 1.0, 0.0
#define PEN_YELLOWISH_ORANGE 1.0, 0.60, 0.0
#define PEN_RED 1.0, 0.0, 0.0
#define PEN_BURLYWOOD 0.87, 0.72, 0.53
#define PEN_GREEN 0.0, 1.0, 0.0
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Game Constants
#define NUM_OF_FOWLS 3
#define MAX_COLLISION_DISTANCE 5 // 5px range defines the collision interaction
#define HALF_FOWL_LENGTH 18 // L in the fowl math description
#define EGG_SIZE 5

// Global Fowl tracking 
int g_fowlXOffset = 0; // x offest tracker for automated moving the fowls
int g_fowlYOffset = 0; // y offest tracker for user moving the fowls
int g_flockOriginX = 0; // Used for determining where the flock is
int g_flockOriginY = 0; // Used for determining where the flock is
int g_fowlHorizontalSeparation = 0; // Initial value; this is set in `drawFowl`
int g_fowlStartingXPos = 40;
int g_fowlUpwardsMovement = 0; // Counter for handling fowl up movement steps
int g_fowlDownwardsMovement = 0; // Counter for handling fowl down movement steps
int g_verticalMovementSteps = 4; // Fowls will move up 4 times to cover 8px
int g_fuel = 35; // Tracks how many moves left for vertical movement
bool g_fowlsLaunched = false; // Tracks if fowls have left the slingshot

// Global Game Status Tracking
bool g_gameStarted = false; // Tracks if to draw intro screen or game screen
bool g_animatingRight = false; // Tracks if the fowls are moving rightwards
bool g_gameOver = false; // Tracks to stop the game

// Global Object Position Tracking
int g_eggOriginX = 0;
int g_eggOriginY = 0;
int g_winConditionX = 0;
int g_winConditionY = 0;

// Global pause button location definition
int g_pauseButtonLeft = 10;
int g_pauseButtonRight = 90;
int g_pauseButtonTop = CANVAS_HEIGHT - 20;
int g_pauseButtonBottom = CANVAS_HEIGHT - 50;

// Misc. Details
char canvas_name[] = "CS 445/545 Prog 2 for Jason Carnahan"; // Window title
int targetFramerate = 25; // 25 frames per second, ~about 40 ms per frame
int timerDelay = (int)1000 / targetFramerate; // Time delay between frames draws

// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
	Draws a fowl to the canvas. Fowls are defined to have "Yellowish Orange"
	fethers and shaped to have 5 sides; 3 major sides will be 25px long
	and in the shape of a triangle. The base of the triangle will connect to
	the 2 minor sides, each 5px long. The base must also be parrallel to the
	canvas width. Fowls will spawn in the left, center of the canvas. These fowls
	will also consume 3D space. The leading face will be at z = -1 and the rear 
	will be at z = -6 with lines connecting the vertices.

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

	// Flock origin is defined as the origin of the left-most fowl
	g_flockOriginX = g_fowlStartingXPos + g_fowlXOffset;
	g_flockOriginY = CANVAS_HEIGHT / 2 + g_fowlYOffset;
	g_fowlHorizontalSeparation = distanceBetween + HALF_FOWL_LENGTH * 2;

	// Draw the flock
	for (int i = 0; i < flockSize; i++) {
		glColor3f(PEN_YELLOWISH_ORANGE);
		// Included to reduce the size of the vertex inputs
		int calculatedSeparation = i * g_fowlHorizontalSeparation;

		// Define Front Fowl shape vertices
		int frontTopX = g_flockOriginX + calculatedSeparation;
		int frontTopY = g_flockOriginY + HALF_FOWL_LENGTH;
		int frontLeftX = g_flockOriginX - HALF_FOWL_LENGTH + calculatedSeparation;
		int frontLeftY = g_flockOriginY;
		int frontBottomLeftX = g_flockOriginX - HALF_FOWL_LENGTH + 3 + calculatedSeparation;
		int frontBottomLeftY = g_flockOriginY - 4;
		int frontBottomRightX = g_flockOriginX + HALF_FOWL_LENGTH - 3 + calculatedSeparation;
		int frontBottomRightY = frontBottomLeftY;
		int frontRightX = g_flockOriginX + HALF_FOWL_LENGTH + calculatedSeparation;
		int frontRightY = frontLeftY;

		// Draw front Fowl Body in CCW order as defined by math
		glBegin(GL_LINE_LOOP);
		glVertex3i(frontTopX, frontTopY, frontZ); // Top
		glVertex3i(frontLeftX, g_flockOriginY, frontZ); // Left
		glVertex3i(frontBottomLeftX, frontBottomLeftY, frontZ); // Bottom Left
		glVertex3i(frontBottomRightX, frontBottomRightY, frontZ); // Bottom Right
		glVertex3i(frontRightX, frontRightY, frontZ); // Right
		glEnd();

		// Draw the visible back Fowl Body with new Z and 3D width
		glBegin(GL_LINE_STRIP);
		glVertex3i(frontTopX - width3D, frontTopY, backZ);
		glVertex3i(frontLeftX - width3D, g_flockOriginY, backZ);
		glVertex3i(frontBottomLeftX - width3D, frontBottomLeftY - 5, backZ); 
		glVertex3i(frontBottomRightX - width3D, frontBottomRightY - 5, backZ);
		glEnd();

		// Draw lines that connect the front and back polygons
		glBegin(GL_LINES);
		glVertex3i(frontTopX, frontTopY, frontZ);
		glVertex3i(frontTopX - width3D, frontTopY, backZ);
		glVertex3i(frontLeftX, frontLeftY, frontZ);
		glVertex3i(frontLeftX - width3D, frontLeftY, backZ);
		glVertex3i(frontBottomLeftX, frontBottomLeftY, frontZ);
		glVertex3i(frontBottomLeftX - width3D, frontBottomLeftY - 5, backZ);
		glVertex3i(frontBottomRightX, frontBottomRightY, frontZ); 
		glVertex3i(frontBottomRightX - width3D, frontBottomRightY - 5, backZ); 
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
	will be top-open rectangles connected to a branch below that extends from the
	right side of the screen. I will define the color of the nests and branch to 
	be a brown-ish color.
*/
void drawNests() {
	glColor3f(PEN_BURLYWOOD);
	int branchLength = CANVAS_WIDTH - 
		(HALF_FOWL_LENGTH * 2 * NUM_OF_FOWLS) - g_fowlHorizontalSeparation - 10;
	int branchY = CANVAS_HEIGHT - HALF_FOWL_LENGTH - 50;

	// Draw the branch
	glBegin(GL_LINES);
	glVertex2i(CANVAS_WIDTH, branchY);
	glVertex2i(branchLength, branchY);
	glEnd();

	// Draw a nest for each fowl
	for (int i = 0; i < NUM_OF_FOWLS; i++) {
		int nestHeight = 20;
		int nestWidth = HALF_FOWL_LENGTH * 2 + 15;
		int offset = (nestWidth) * i;
		
		// Draw nests 
		glBegin(GL_LINE_STRIP);
		glVertex2i(branchLength + 10 + offset, branchY + nestHeight);
		glVertex2i(branchLength + 10 + offset, branchY + nestHeight/2);
		glVertex2i(branchLength + nestWidth + offset, branchY + nestHeight/2);
		glVertex2i(branchLength + nestWidth + offset, branchY + nestHeight);
		glEnd();

		// Set the origin of the Egg only on initialization
		if (g_eggOriginX == 0 && i == 0) {
			g_eggOriginX = branchLength + 10 + offset;
			g_eggOriginY = branchY + nestHeight / 2;
		}

		// Set the win condition position on the third nest
		if (i == NUM_OF_FOWLS - 1) {
			g_winConditionX = branchLength + nestWidth + offset - 10;
			g_winConditionY = branchY + nestHeight;
		}

		// Draw Stems from the nest to the branch
		glBegin(GL_LINES);
		glVertex2i(branchLength + nestWidth/2 + offset, branchY + nestHeight/2);
		glVertex2i(branchLength + nestWidth/2 + offset, branchY);
		glEnd();
	}
}

/*
	Draws a white egg starting off the edge of the left nest.
*/
void drawEgg() {
	glColor3f(PEN_WHITE);
	glBegin(GL_LINE_LOOP);
	glVertex2i(g_eggOriginX, g_eggOriginY + EGG_SIZE);
	glVertex2i(g_eggOriginX - EGG_SIZE, g_eggOriginY);
	glVertex2i(g_eggOriginX - 4, g_eggOriginY - 3);
	glVertex2i(g_eggOriginX - 3, g_eggOriginY - 4);
	glVertex2i(g_eggOriginX + 3, g_eggOriginY - 4);
	glVertex2i(g_eggOriginX + 4, g_eggOriginY - 3);
	glVertex2i(g_eggOriginX + EGG_SIZE, g_eggOriginY);
	glEnd();
}
// END SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
*/
void print(const char m[], void* f, float r, float g, float b, int x, int y) {
	glColor3f(r, g, b);
	glRasterPos2i(x, y);
	for (int i = 0; i < strlen(m); i++) {
		glutBitmapCharacter(f, m[i]);
	}
	glFlush(); // Messages would not appear without this
}

/*
	Calculates the squared distance between two points and compares it against 
	the square of the max collision distance plus the sizes of both objects. This
	simulates a circular region around each object and detects if the circles 
	collide.
	
	This does not include the square root in distance calculation because square 
	root is computationally expensive and we only have ~30-40 ms between frames.

	x1 - X position of the origin for the first object.
	y1 - Y position of the origin for the first object.
	size1 - Circular radius from the first object's origin.
	x2 - X position of the origin for the second object.
	y2 - Y position of the origin for the second object.
	size2 - Circular radius from the second object's origin.
*/
bool collided(int x1, int y1, int size1, int x2, int y2, int size2) {
	return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) < 
		(MAX_COLLISION_DISTANCE + size1 + size2 ) * 
		(MAX_COLLISION_DISTANCE + size1 + size2) ? true : false;
}

/*
	Checks the game for a WIN or LOSS condition. If either WIN or LOSS is 
	detected, then the game will end.
	WIN Conditions - 1. Fowls are correctly placed in their nests.
	LOSS Conditions - 1. Fowls have reached the end of the screen.
		2. Lead fowl was struck by the egg.
*/
bool endGame() {
	// The origin is defined by the left most fowl but I need the right most one
	int fowlPositionX = g_flockOriginX + HALF_FOWL_LENGTH * 2 * NUM_OF_FOWLS;
	int fowlPositionY = g_flockOriginY;

	if (fowlPositionX >= g_winConditionX - MAX_COLLISION_DISTANCE &&
		fowlPositionX <= g_winConditionX + MAX_COLLISION_DISTANCE &&
		fowlPositionY <= g_winConditionY + MAX_COLLISION_DISTANCE &&
		fowlPositionY >= g_winConditionY - MAX_COLLISION_DISTANCE)
	{ // Win condition 1 - Defined box region in the nest
		g_gameOver = true; // End the game
		print("YOU", GLUT_BITMAP_HELVETICA_18, PEN_YELLOW, CANVAS_WIDTH / 2 - 50, CANVAS_HEIGHT / 2);
		print("WIN", GLUT_BITMAP_HELVETICA_18, PEN_GREEN, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);
	}
	else if (fowlPositionX >= CANVAS_WIDTH - MAX_COLLISION_DISTANCE * 2 ||
		collided(fowlPositionX, fowlPositionY, 10, 
			g_eggOriginX, g_eggOriginY, EGG_SIZE))
	{ // Loss condition 1 || Loss condition 2
		g_gameOver = true; // End the game
		print("BETTER LUCK NEXT TIME", GLUT_BITMAP_TIMES_ROMAN_24, PEN_RED, CANVAS_WIDTH / 2 - 125, CANVAS_HEIGHT / 2);
	}

	// Disable inputs on game over
	if (g_gameOver) {
		glutKeyboardFunc(NULL);
		glutMouseFunc(NULL);
	} 
	return g_gameOver;
}

/*
	Prints the fuel/vertical moves remaining for the user onto the screen.
*/
void printFuel() {
	int asciiBaseline = 48; // Int to Char follows the ASCII table

	// Convert double digit number to char array using rounding and type casting
	char fuel[3];
	fuel[0] = (char)(asciiBaseline + g_fuel / 10);
	fuel[1] = (char)(
		(((float)g_fuel / 10.0) - (int)(g_fuel / 10)) * 10 + asciiBaseline);
	fuel[2] = '\0';

	// Text positioning
	int titleXPos = CANVAS_WIDTH - 75;
	int valueXPose = titleXPos + 50;
	int yPos = CANVAS_HEIGHT - 20;

	// Printing Text based on fuel remaining
	switch (g_fuel) {
	case 2: // Yellow Warning Color
	case 1:
		print("Fuel: ", GLUT_BITMAP_TIMES_ROMAN_24, PEN_YELLOW, titleXPos, yPos);
		print(fuel, GLUT_BITMAP_TIMES_ROMAN_24, PEN_YELLOW, valueXPose, yPos);
		break;
	case 0: // Red Emergency Color
		print("Fuel: ", GLUT_BITMAP_TIMES_ROMAN_24, PEN_RED, titleXPos, yPos);
		print(fuel, GLUT_BITMAP_TIMES_ROMAN_24, PEN_RED, valueXPose, yPos);
		break;
	default:
		print("Fuel: ", GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, titleXPos, yPos);
		print(fuel, GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, valueXPose, yPos);
		break;
	}
}

/*
	Draws a pause button in the top left of the screen where a mouse click 
	listener listens for clicks within the region and pauses rightward movement.
*/
void drawPauseButton() {
	glColor3f(PEN_WHITE);
	glBegin(GL_LINE_LOOP);
	glVertex2i(g_pauseButtonLeft, g_pauseButtonBottom);
	glVertex2i(g_pauseButtonLeft, g_pauseButtonTop);
	glVertex2i(g_pauseButtonRight, g_pauseButtonTop);
	glVertex2i(g_pauseButtonRight, g_pauseButtonBottom);
	glEnd();
	print("Pause", GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, 20, CANVAS_HEIGHT-43);
}
// END UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/**
	Event handler for timer events. Determines how to handle moving global 
	positions and the reregistering of additional timer events for 
	Frame-by-Frame animation.
	Timer Events with ID 1 are animating fowls to the right.
	Timer Events with ID 2 are animating fowls upwards.
	Timer Events with ID 3 are animating fowls downwards.
	Timer Events with ID 4 are animating the egg downwards.

	timerId - Defines the ID of the timer event we are handling.
 */
void timerEventHandler(int timerId) {
	if (endGame()) return; // Don't do movements if the game has ended

	// This will determine how much to move the fowl vertically
	int vertialAdjustment = 8 / g_verticalMovementSteps;

	switch (timerId) {
	case 1: // Automated fowls moving right
		if (g_animatingRight) { // Clicking pause stops this
			glutTimerFunc(timerDelay, timerEventHandler, 1);
			g_fowlXOffset += 4; // Move the fowls right 4 units
		}
		break;
	case 2: // User input for fowls moving up
		if (!g_fuel) break; // Stop if we are out of fuel
		if (g_fowlUpwardsMovement >= g_verticalMovementSteps) {
			// Stop vertical movement
			g_fowlUpwardsMovement = 0;
			g_fuel--;
			break;
		}
		glutTimerFunc(timerDelay/g_verticalMovementSteps, timerEventHandler, 2);
		g_fowlUpwardsMovement++;
		g_fowlYOffset += vertialAdjustment;
		break;
	case 3: // User input for fowls moving down
		if (!g_fuel) break; // Stop if we are out of fuel
		if (g_fowlDownwardsMovement >= g_verticalMovementSteps) {
			// Stop vertical movement
			g_fowlDownwardsMovement = 0;
			g_fuel--;
			break;
		}
		glutTimerFunc(timerDelay/g_verticalMovementSteps, timerEventHandler, 3);
		g_fowlDownwardsMovement++;
		g_fowlYOffset -= vertialAdjustment;
		break;
	case 4: // Automated Egg movement
		glutTimerFunc(timerDelay, timerEventHandler, 4);
		g_eggOriginY -= 5; // Move down 5px per frame
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
	// Handle Pre-Game case
	if (!g_gameStarted) {
		switch (key){
		case 'B': // Change the game started state on B or b
		case 'b':
			g_gameStarted = true;
			glutPostRedisplay();
		default:
			break;
		}
		return;
	}

	if (!g_fowlsLaunched) {
		switch (key) {
		case 'M': // This will begin 'Movement'/Animation on any M or m press
		case 'm':
			g_animatingRight = true;
			g_fowlsLaunched = true;
			glutTimerFunc(timerDelay, timerEventHandler, 1);
			// Start egg movement .75s before end game
			glutTimerFunc(3000 - 750, timerEventHandler, 4);
			break;
		default:
			break;
		}
	}
	else {
		switch (key) {
		case 'U': // Move the Fowls up over several movement steps
		case 'u':
			glutTimerFunc(timerDelay / g_verticalMovementSteps,
				timerEventHandler, 2);
			break;
		case 'N': // Move the Fowls down over several movement steps
		case 'n':
			glutTimerFunc(timerDelay / g_verticalMovementSteps,
				timerEventHandler, 3);
			break;
		default:
			break;
		}
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

	switch (g_gameStarted) {
	case false: // Draw the Introduction screen 
		print("Press B To Begin", 
			GLUT_BITMAP_TIMES_ROMAN_24, 
			PEN_WHITE, 
			CANVAS_WIDTH / 2 - strlen("Press B To Begin") * 5, 
			CANVAS_HEIGHT / 2);
		break;
	case true: // Draw the game scene
		drawFowl(NUM_OF_FOWLS, 15);
		drawSlingshot();
		drawNests();
		drawEgg();
		drawPauseButton();
		printFuel();
		break;
	}
	
	// Execute the draw
	glFlush(); // Not including this caused issues for me
}

/*
	Event handler for mouse click events. Only operates when the user clicks down
	inside the pause box region. If the user does so, the fowls will stop moving
	right. Clicking again will resume normal operation.
*/
void mouseClickEventHandler(int button, int state, int x, int y) {
	switch (button) {
	case GLUT_LEFT_BUTTON:
		// Only within the Pause Button region and the fowls have been launched
		if (state == GLUT_DOWN && 
			x <= g_pauseButtonRight &&
			x >= g_pauseButtonLeft &&
			y >= 20 && 
			y <= 50 &&
			g_fowlsLaunched) { 
			g_animatingRight = !g_animatingRight;
			if (g_animatingRight) { // Restart right movement if turning back on
				glutTimerFunc(timerDelay, timerEventHandler, 1);
			}
		}
	default:
		break;
	}
}
// END EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	Goal of the game is to land a trio of fowls in a series of small nests on the
	upper right of the screen. If the fowls align with the nests, "YOU WIN" is 
	printed. If the fowls are struck by the falling egg or reach the right edge 
	of the screen w/o proper alignment, "BETTER LUCK NEXT TIME" is printed.
*/
int main(int argc, char ** argv) {
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_name);

	// Register Event Handlers
	glutDisplayFunc(displayEventHandler);
	glutKeyboardFunc(keyboardEventHandler);
	glutMouseFunc(mouseClickEventHandler);

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}