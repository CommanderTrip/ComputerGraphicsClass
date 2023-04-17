/* CS 445/545 Prog 5 for Jason Carnahan

Architecture Statement:
	There are three major event handlers in this program. `displayEventHandler`, 
	`timerEventHandler`, and `mouseClickEventHandler` all handle their namesake 
	events. On program start, `main` will register `displayEventHandler` to begin
	establishing lighting for the environment and drawing the scene objects, and 
	`main` will register `timerEventHandler` to begin updating the 'UAH' letter 
	rotations every frame then register itself for every frame. Finally, `main` 
	will also register `mouseClickEventHandler` to determine if the user clicks 
	within the 'switch button' region of the screen. If they do, the lighting 
	shader model will toggle between Flat and Gouraud/Smooth shading. The program
	will continue infinitely until the user exits the window or terminates the 
	program manually.
*/
#include "pch.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "OpenGL445Setup.h"

// Meta Data
#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 800
#define FRAMERATE 25 // 25 frames per second, ~about 40 ms per frame

// Colors
#define SILVER 0.75, 0.75, 0.75
#define LIGHT_BLUE 0.4, 0.55, 0.75
#define GREY 0.6, 0.6, 0.6
#define WHITE 1.0, 1.0, 1.0
#define CANVAS_BLACK 0.0, 0.0, 0.0, 1.0

// Lighting Properties
float light_position[] = { 0.0, 0.0, 0.0, 1.0 }; // Homogeneous value
float light_ambient[] = { 0.1, 0.1, 0.1, 1.0 };
float light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
float light_specular[] = { 1.0, 1.0, 1.0, 1.0 };

// Material Properties
float mat_ambient_UH[] = {SILVER, 1.0};
float mat_diffuse_UH[] = {SILVER, 1.0 };
float mat_specular_UH[] = {1.0, 1.0, 1.0, 1.0}; // White Reflection
float mat_shininess_UH[] = {20.0}; // Lower -- the shinier

float mat_ambient_A[] = {LIGHT_BLUE, 1.0};
float mat_diffuse_A[] = {LIGHT_BLUE, 1.0};
float mat_specular_A[] = {0.5, 0.5, 0.5, 1.0};
float mat_shininess_A[] = {80.0}; // Higher -- the duller

float mat_ambient_spindle[] = {GREY, 1.0};
float mat_diffuse_spindle[] = {GREY, 1.0};
float mat_specular_spindle[] = {0.25, 0.25, 0.25, 1.0}; // Very dark reflection
float mat_shininess_spindle[] = {100.0};

// Misc. Details
char canvas_name[] = "CS 445/545 Prog 5 for Jason Carnahan"; // Window title
int timerDelay = (int)1000 / FRAMERATE; // Time delay between frames draws
int uahRotation = 0; // Degrees to rotate the UAH letters
bool flat_shading_model = true;
// UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	Prints a desired message to the screen with font, color, and location. Due to
	lighting being on, colors will be handled outside the function.

	m - Character array of the message to print.
	f - Pointer to the font type.
	x - Int of the X position in OpenGL space of the message.
	y - Int of the Y position in OpenGL space of the message.
	z - Int of the Z position in OpenGL space of the message.
*/
void print(const char m[], void* f,	int x, int y, int z) {
	glPushMatrix();
	glLoadIdentity();
	glRasterPos3i(x, y, z);
	for (int i = 0; i < strlen(m); i++) {
		glutBitmapCharacter(f, m[i]);
	}
	glFlush(); // Messages would not appear without this
	glPopMatrix();
}
// END UTILITY FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	Draws the individual letters to the screen. Letters are to be made up of 
	cubes that are sized 50. Each letter is 10 cubes to construct.
*/
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

/*
	Presents the combination of UAH to the screen and applies all light material
	properties to each letter.
*/
void drawUah() {
	glPushMatrix();
	glLoadIdentity();

	// Start Conditions
	glTranslatef(0.0, 0.0, -400.0);
	glRotatef(uahRotation, 0.0, 1.0, 0.0);
	
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_UH);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_UH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_UH);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_UH);
	drawLetter('u');

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_A);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_A);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_A);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_A);
	drawLetter('a');

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_UH);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_UH);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_UH);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_UH);
	drawLetter('h');

	glPopMatrix();
}

/*
	Draws a 'spindle' atop the A as a dull, grey tetrahedron.
*/
void drawSpindle() {
	glPushMatrix();
	glLoadIdentity();

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_spindle);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_spindle);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_spindle);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_spindle);
	
	glTranslatef(0.0, 18.0, -425.0);
	glRotatef(29.0, 1.0, 0.0, 0.0);	// X -- Altered for tetrahedron clarity
	glRotatef(90.0, 0.0, 1.0, 0.0); // Y
	glRotatef(132.0, 0.0, 0.0, 1.0);// Z
	glScalef(43.3, 43.3, 43.3); // sqrt(3) * 43.3 = 75
	glutSolidTetrahedron(); // Docs say it is modeled with a radius of sqrt(3)

	glPopMatrix();
}

/*
	Draws the text and switch to the bottom of the screen.
*/
void drawUi() {
	// The text is apparently subject to lighting conditions as well
	float mat_ambient_UiText[] = { WHITE, 1.0 };
	float mat_diffuse_UiText[] = { WHITE, 1.0 };
	float mat_specular_UiText[] = { 0.0, 0.0, 0.0, 0.0 };
	float mat_shininess_UiText[] = { 128.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_UiText);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_UiText);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_UiText);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess_UiText);

	// Draw Text
	float textColor[] = { 1.0, 1.0, 1.0 }; // White
	print("Flat", GLUT_BITMAP_TIMES_ROMAN_24, -3.0,-10.0,-11.0);
	print("Gouraud", GLUT_BITMAP_TIMES_ROMAN_24, 2.0,-10.0,-11.0);

	// Draw switch container as box
	glPushMatrix();
	glLoadIdentity();
	glBegin(GL_LINE_LOOP);
	glVertex3f(-1.5, -9.0, -11.0);
	glVertex3f(-1.5, -10.0, -11.0);
	glVertex3f(1.5, -10.0, -11.0);
	glVertex3f(1.5, -9.0, -11.0);
	glEnd();

	// Draw physical switch
	if (flat_shading_model) { // Draw to the left
		glBegin(GL_POLYGON);
		glVertex3f(-1.25, -9.25, -11);
		glVertex3f(-1.25, -9.75, -11);
		glVertex3f(-0.50, -9.75, -11);
		glVertex3f(-0.50, -9.25, -11);
		glEnd();
	} else { // Draw to the right 
		glBegin(GL_POLYGON);
		glVertex3f(0.50, -9.25, -11);
		glVertex3f(0.50, -9.75, -11);
		glVertex3f(1.25, -9.75, -11);
		glVertex3f(1.25, -9.25, -11);
		glEnd();
	}
	glPopMatrix();
}
// END SCENE OBJECT FUNCTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	Handles rotating the UAH on the screen. This timer is started at the 
	beginning of the program and never stops.
*/
void timerEventHandler(int timerId) {
	switch (timerId) {
	case 1: 
		uahRotation += 4; // Rotate counter clockwise
		if (uahRotation >= 360) uahRotation -= 360;
		glutTimerFunc(timerDelay/2, timerEventHandler, 1);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

/*
	Handles display events and draws all the necessary objects to the screen as 
	well as applying depth and lighting to the scene.
*/
void displayEventHandler() {
	glClearColor(CANVAS_BLACK); // Define background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the background
	glEnable(GL_DEPTH_TEST);

	// Enable Lighting
	glPushMatrix();
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glPopMatrix();

	// How specular reflection angles are computed
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); 

	// Shading Model
	if (flat_shading_model) glShadeModel(GL_FLAT);
	else glShadeModel(GL_SMOOTH);

	// Scene objects
	drawUah();
	drawSpindle();
	drawUi();

	// Execute the draw
	glutSwapBuffers();
	glFlush(); // Not including this caused issues for me
}

/*
	Handles if the user clicked their mouse down in the shading box.
*/
void mouseClickEventHandler(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		if ((x < 455 && x > 345) && (y < 765 && y > 730)) {
			flat_shading_model = !flat_shading_model;
		}
	}
}
// END EVENT HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
	This is a simple lighting demonstration program that spins the letters U-A-H 
	about a spindle. The letters and spindle demonstrate different material 
	properties and can perform under different shading models.
*/
int main(int argc, char ** argv) {
	// Setup Environment
	glutInit(&argc, argv);
	my_setup(CANVAS_WIDTH, CANVAS_HEIGHT, canvas_name);

	glutDisplayFunc(displayEventHandler);
	glutTimerFunc(timerDelay, timerEventHandler, 1);
	glutMouseFunc(mouseClickEventHandler);

	// Start
	glutMainLoop(); /* execute until killed */
	return 0;
}