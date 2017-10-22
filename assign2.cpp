/*
  CSCI 480
  Assignment 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "pic.h"

int g_iMenuId;
/* represents one control point along the spline */
struct point {
   double x;
   double y;
   double z;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
   int numControlPoints;
   struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;


int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;


  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &g_Splines[j].points[i].x, 
	   &g_Splines[j].points[i].y, 
	   &g_Splines[j].points[i].z) != EOF) {
      i++;
    }
  }

  free(cName);

  return 0;
}

void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);   // set background color
    glEnable(GL_DEPTH_TEST);            // enable depth buffering
    glShadeModel(GL_SMOOTH);            // interpolate colors during rasterization
}

void reshape(int width, int height)
{

    if (height == 0) height = 1;                // To prevent divide by 0
   GLfloat aspect = (GLfloat)width / (GLfloat)height;
 
   // Set the viewport to cover the new window
   glViewport(0, 0, width, height);
 
   // Set the aspect ratio of the clipping volume to match the viewport
   glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
   glLoadIdentity();             // Reset
   // Enable perspective projection with fovy, aspect, zNear and zFar
   gluPerspective(20.0f, aspect, 0.01f, 1000.0f);

   glMatrixMode(GL_MODELVIEW);
   //glLoadIdentity();
}

void myinit()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);   // set background color
    glEnable(GL_DEPTH_TEST);            // enable depth buffering
    glShadeModel(GL_SMOOTH); 
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

void display()
{
  /* draw 1x1 cube about origin */
  /* replace this code with your height field implementation */
  /* you may also want to precede it with your 
rotation/translation/scaling */

 
    
    // clear buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glLoadIdentity(); // reset transformation

   gluLookAt(0.0, 0.0,8.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0);
  
    //glEnd();

  glBegin(GL_POLYGON);

  glColor3f(1.0, 1.0, 1.0);
  glVertex3f(-0.5, -0.5, 0.0);
  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(-0.5, 0.5, 0.0);
  glColor3f(0.0, 0.0, 0.0);
  glVertex3f(0.5, 0.5, 0.0);
  glColor3f(1.0, 1.0, 0.0);
  glVertex3f(0.5, -0.5, 0.0);

  glEnd();

 // glPopMatrix();
    
  glutSwapBuffers(); // double buffer flush

}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
  printf ("usage: %s <trackfile>\n", argv[0]);
  exit(0);
  }

  loadSplines(argv[1]);

  glutInit(&argc,argv);
  
  /*
    create a window here..should be double buffered and use depth testing
  
    the code past here will segfault if you don't have a window set up....
    replace the exit once you add those calls.
  */
    
    // request double buffer
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    
    // set window size
    glutInitWindowSize(640, 480);
    
    // set window position
    glutInitWindowPosition(0, 0);
    
    // creates a window
    glutCreateWindow("Ahahaha!");
 //
  //exit(0);

  /* tells glut to use a particular display function to redraw */
  glutReshapeFunc(reshape);
  //glutReshapeFunc(changeSize);
  glutDisplayFunc(display);
  //glutDisplayFunc(display);
  
  /* allow the user to quit using the right mouse button menu */
  g_iMenuId = glutCreateMenu(menufunc);
  glutSetMenu(g_iMenuId);
  glutAddMenuEntry("Quit",0);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  
  /* replace with any animate code */
 // glutIdleFunc(doIdle);

 

  /* do initialization */
  myinit();

  glutMainLoop();
  return 0;
}
