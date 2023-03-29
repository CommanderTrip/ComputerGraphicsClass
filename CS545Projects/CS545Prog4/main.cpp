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

// Canvas Dimensions
#define CANVAS_WIDTH 640
#define CANVAS_HEIGHT 640

// Pen and Canvas Colors
#define PEN_BLACK 0.0, 0.0, 0.0
#define PEN_ORANGE 1.0, 0.66, 0.0
#define PEN_YELLOW 1.0, 1.0, 0.0
#define PEN_WHITE 1.0, 1.0, 1.0
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Global Transformations of objects
float g_fishPosition[3] = { 0.0, 0.0, -175.0 }; 
int g_fishHeading = 180; // Rotation over Z axis
float g_foodPosition[3] = { 
	(float)rand() / RAND_MAX * 200 - 100,
	(float)rand() / RAND_MAX * 200 - 100,
	-175.0
};
int g_fanRotation = 0;

// Global Game Trackers 
int g_foodSpawnTimer = 4000;
int g_fishEnlargeTimer = 0;
int g_gameScore = 0;
int g_gameTimer = 30; // Game will last 30 seconds


// Misc. Details
char canvas_name[] = "CS 445/545 Prog 4 for Jason Carnahan"; // Window title
int targetFramerate = 25; // 25 frames per second, ~about 40 ms per frame
int timerDelay = (int)1000 / targetFramerate; // Time delay between frames draws
// UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
bool collided(int x1, int y1, int x2, int y2) {
	return (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) < 2500 ? true : false;
}

/*
	Changes the internal transformation matrix to be the one defined as inputs. 
	If nothing is passed, the identity matrix is applied. This guarantees that
	developers are always applying transformations in the same way.
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

/*
	This will take any positive integer and print it as a char to the screen.
	input - The positive integer to print.
	xPos, yPos, zPos - Integers that describe where on the screen to put the int.
*/
void printAnyPositiveIntToChar(int input, int xPos, int yPos, int zPos) {
	int asciiBaseline = 48; // ASCII 48 is 0 - the first number
	char dissect[100] = "";
	if (!input) dissect[0] = '0';
	while (input) {
		float temp = (float)input / 10;
		input /= 10;
		char value[100] = { (char)(((temp - input) * 10) + asciiBaseline)};
		strcat_s(value, dissect);
		strcpy_s(dissect, value);
	}
	print(dissect, GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, xPos, yPos, zPos);
}
// END UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	The fish is to be modeled as an octahedron wire frame with a triangular 
	tail. That octahedron is 75 units wide, 25 units tall, and 25 units deep. The
	tail is 15 units wide and is attached to the body at the tip of the triangle.
*/
void drawFish() {
	int fishHalfX = 37;
	int fishHalfYZ = 12;
	float fishEnlarge = 0;
	if (g_fishEnlargeTimer) fishEnlarge = 8; // 10% of 75 rounded up 
	glColor3f(PEN_ORANGE);

	// Fish Body
	transform(
		g_fishPosition[0], g_fishPosition[1], g_fishPosition[2],
		g_fishHeading, 0, 0, 1,
		fishHalfX + fishEnlarge,
		fishHalfYZ + fishEnlarge,
		fishHalfYZ + fishEnlarge);
	glutWireOctahedron();

	// Fish Tail
	transform(g_fishPosition[0], g_fishPosition[1], g_fishPosition[2],
		g_fishHeading+180, 0, 0, 1, 
		1+fishEnlarge/32, 1+fishEnlarge/32, 1+fishEnlarge/32);
	glBegin(GL_LINE_LOOP);
	glVertex2f(fishHalfX, 0);
	glVertex2f(fishHalfX + 9, fishHalfYZ);
	glVertex2f(fishHalfX + 9, -fishHalfYZ);
	glEnd();
}

/*
	The food are white spheres of diameter 10 units. Food remains in position for
	3 seconds. At the 4th, it will spawn at a new location. It will respawn 1 
	second after being eaten.
*/
void timerEventHandler(int timerId);
void drawFood() {
	// Check for fish and food collision
	if (collided(
		g_fishPosition[0], g_fishPosition[1], 
		g_foodPosition[0], g_foodPosition[1] ))
	{ // Collision detected; begin respawn
		g_foodSpawnTimer = 999;
		glutTimerFunc(1000, timerEventHandler, 4);
		g_gameScore += 10;
	}

	if (g_foodSpawnTimer < 1000 && g_foodSpawnTimer > 0) { // Hide Food 
		g_foodPosition[0] = 500;
		g_foodPosition[1] = 500;
	}
	else if (g_foodSpawnTimer < 0) { // Respawn Food
		g_foodPosition[0] = g_fishPosition[0];
		g_foodPosition[1] = g_fishPosition[1];
		// Dont spawn new food too close to the fish
		while (collided(
			g_fishPosition[0], g_fishPosition[1],
			g_foodPosition[0], g_foodPosition[1]
		)) {
			g_foodPosition[0] = (float)rand() / RAND_MAX * 200 - 100;
			g_foodPosition[1] = (float)rand() / RAND_MAX * 200 - 100;
		}
		g_foodSpawnTimer = 4000;
	}

	glColor3f(PEN_WHITE);
	transform(g_foodPosition[0], g_foodPosition[1], g_foodPosition[2]);
	glutWireSphere(10, 5, 5);
}

/*
	The fan is 100 units in size with 6 yellow blades, each a planar triangle 
	shape 50 units long. These triangles spin about the fan’s center. The fan’s 
	center is exactly in the center of the back of the fish tank. They are 
	equally angularly positioned, and spin in synchrony at a rate of 2 seconds to
	make one spin.
*/
void drawFan() {
	glColor3f(PEN_YELLOW);
	for (int i = 0; i < 6; i++) {
		int rotation = i * 60;
		if (rotation > 360) rotation -= 360;
		transform(0, 0, -300, rotation + g_fanRotation, 0, 0, 1);
		glBegin(GL_LINE_LOOP);
		glVertex2i(0, 0);
		glVertex2i(50, 10);
		glVertex2i(50, -10);
		glEnd();
	}
}

/*
	Fish tank is a 250x250x250 axially aligned yellow wire cube with the front 
	face at z = -50.
*/
void drawTank() {
	transform(0,0,-175); // 250 / 2 = 125; 125 + 50 = 175
	glColor3f(PEN_YELLOW);
	glutWireCube(250);
}

/*
	Draws all the text to the screen.
*/
void drawUI() {
	print("SECONDS REMAINING", GLUT_BITMAP_TIMES_ROMAN_24,PEN_WHITE, 0, 350, 0);
	printAnyPositiveIntToChar(g_gameTimer, 134, 350, 0);

	print("SCORE", GLUT_BITMAP_TIMES_ROMAN_24, PEN_WHITE, -250, 300, 0);
	printAnyPositiveIntToChar(g_gameScore, -225, 280, 0);
}
// END SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	Handles display events and draws all the necessary objects to the screen.
*/
void displayEventHandler() {
	glClearColor(CANVAS_BLACK); // Define background color
	glClear(GL_COLOR_BUFFER_BIT); // Clear the background

	drawFish();
	drawTank();
	drawFan();
	drawUI();
	drawFood();

	// Execute the draw
	glutSwapBuffers();
	glFlush(); // Not including this caused issues for me
}

/*
	Handles keyboard events. The only actions to handle are moving the fish. The
	only keys to handle are 'U, N, H, J' for moving the fish.
*/
void keyboardEventHandler(unsigned char key, int x, int y) {
	switch (key) {
	case 'u': // Move Fish Up
	case 'U': 
		g_fishPosition[1] += 10;
		if (g_fishPosition[1] >= 125) g_fishPosition[1] = 125;
		break;
	case 'n': // Move Fish Down 
	case 'N':
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

/*
	Handle all timer events.
	timerId 1 - Handle countdown timer.
	timerId 2 - Handle the rotating fan.
	timerId 3 - Handle food spawn timer.
	timerId 4 - Handle fish enlargement timer.
*/
void timerEventHandler(int timerId) {
	// Game ends so stop all timers
	if (!g_gameTimer) {
		glutKeyboardFunc(NULL);
		return;
	}

	// Handle in-game timers
	switch (timerId) {
	case 1: // Game timer countdown
		g_gameTimer--;
		if (g_gameTimer < 0) {
			g_gameTimer = 0;
			return;
		}
		glutTimerFunc(1000, timerEventHandler, 1);
		break;
	case 2: // Fan spinning
		g_fanRotation += 7; // 7.2 degrees every frame@25fps meets 360 in 2 sec
		if (g_fanRotation > 360) g_fanRotation -= 360;
		glutTimerFunc(timerDelay, timerEventHandler, 2);
		break;
	case 3: // Change Food timer
		g_foodSpawnTimer -= timerDelay;
		glutTimerFunc(timerDelay, timerEventHandler, 3);
		break;
	case 4: // Enlarge Fish timer
		if (!g_fishEnlargeTimer) g_fishEnlargeTimer = 1000;
		g_fishEnlargeTimer -= timerDelay;
		if (g_fishEnlargeTimer <= 0) {
			g_fishEnlargeTimer = 0;
			return;
		}
		glutTimerFunc(timerDelay, timerEventHandler, 4);
	default:
		break;
	}
	glutPostRedisplay();
}
// END EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	This is a simple game where a user controls a fish to collect food.
*/
int main(int argc, char ** argv) {
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_name);

	glutDisplayFunc(displayEventHandler);
	glutKeyboardFunc(keyboardEventHandler);

	glutTimerFunc(1000, timerEventHandler, 1); // Start Game Timer
	glutTimerFunc(timerDelay, timerEventHandler, 2); // Start Fan Spinning
	glutTimerFunc(timerDelay, timerEventHandler, 3); // Start Food spawning

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}