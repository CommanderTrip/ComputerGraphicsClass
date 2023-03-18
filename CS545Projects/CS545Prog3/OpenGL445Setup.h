#pragma once
/************************************************************
OpenGL445Setup.h

	This header file contains initialization function calls and set-ups
for basic 3D CS 445/545 Open GL (MESA) programs that use the GLUT/freeglut.
The initializations involve defining a callback handler (my_reshape_function)
that sets view parameters for orthographic 3D disaply.

	TSN 01/2022 version - for OpenGL 4.3 w/legacy compatibility

*************************************************************/


/* reshape callback handler - defines viewing parameters (projection) */

void my_3d_projection(int width, int height)
{
	//For Program 3, the viewing region is described from:
	//	x = -300 : 300, y = -300 : 300, z = -1 : -100

	glViewport(0, 0, (GLfloat)width, (GLfloat)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-300.0, 300.0, -300.0, 300.0, 1, 100.0);
	glMatrixMode(GL_MODELVIEW);
}

#define STRT_X_POS 25
#define STRT_Y_POS 25

/* initialization routine */

void my_setup(int width, int height, char *window_name_str)
{
	// Allow for current OpenGL4.3 but backwards compatibility to legacy GL 4.3
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	// To get double buffering, uncomment the following line
	//glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	// below code line does single buffering - if above line is uncommented,
	// the single buffering line will have to be commented out
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(STRT_X_POS, STRT_Y_POS);

	glutCreateWindow(window_name_str);
	glewExperimental = GL_TRUE;
	glewInit();

	glutReshapeFunc(my_3d_projection);
}