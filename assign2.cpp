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
#include <math.h>  
#include <iostream>
#include <string>
#include <cstring>

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;
//char* files[6] = {"sky2.jpg","sky2.jpg","sky2.jpg","sky2.jpg","ground.jpg","sky2.jpg"};
char* files[7] = {"posz.jpg","negz.jpg","negx.jpg","posx.jpg","negy.jpg","posy.jpg","wood.jpg"};
int screenshotNum = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
typedef enum {SOLID_TRIANGLES, VERTICES, WIREFRAMES} RENDERMODE;

CONTROLSTATE g_ControlState = ROTATE;
RENDERMODE g_RenderMode = SOLID_TRIANGLES;
GLuint texture[6];
/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};
double a = 1;

int tngt_idx = 0;
int capacity = 0;
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
float b_m_3[] = {-0.5, 1.5, -1.5, 0.5};
float b_m_2[] = {1.0, -2.5, 2.0, -0.5};
float b_m_1[] = {-0.5, 0.0, 0.5, 0.0};
float b_m_0[] = {0.0, 1.0, 0.0, 0.0};


float t_m_2[] = {-1.5,4.5,-7.5,1.5};
float t_m_1[] = {2.0,-5.0,4.0,-1.0};

point* tangent;
point* binormal;
point* normal;
point* center;
point* eye;
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
point catmull (point p1, point p2, point p3, point p4, float u){
  point newPoint;
  newPoint.x = ((pow(u,3)) * (b_m_3[0]*(p1.x) + b_m_3[1]*(p2.x) + b_m_3[2]*(p3.x) + b_m_3[3]*(p4.x)))
             + ((pow(u,2)) * (b_m_2[0]*(p1.x) + b_m_2[1]*(p2.x) + b_m_2[2]*(p3.x) + b_m_2[3]*(p4.x)))
             + ((pow(u,1)) * (b_m_1[0]*(p1.x) + b_m_1[1]*(p2.x) + b_m_1[2]*(p3.x) + b_m_1[3]*(p4.x)))
             + ((pow(u,0)) * (b_m_0[0]*(p1.x) + b_m_0[1]*(p2.x) + b_m_0[2]*(p3.x) + b_m_0[3]*(p4.x)));

  newPoint.y = ((pow(u,3)) * (b_m_3[0]*(p1.y) + b_m_3[1]*(p2.y) + b_m_3[2]*(p3.y) + b_m_3[3]*(p4.y)))
             + ((pow(u,2)) * (b_m_2[0]*(p1.y) + b_m_2[1]*(p2.y) + b_m_2[2]*(p3.y) + b_m_2[3]*(p4.y)))
             + ((pow(u,1)) * (b_m_1[0]*(p1.y) + b_m_1[1]*(p2.y) + b_m_1[2]*(p3.y) + b_m_1[3]*(p4.y)))
             + ((pow(u,0)) * (b_m_0[0]*(p1.y) + b_m_0[1]*(p2.y) + b_m_0[2]*(p3.y) + b_m_0[3]*(p4.y)));

  newPoint.z = ((pow(u,3)) * (b_m_3[0]*(p1.z) + b_m_3[1]*(p2.z) + b_m_3[2]*(p3.z) + b_m_3[3]*(p4.z)))
             + ((pow(u,2)) * (b_m_2[0]*(p1.z) + b_m_2[1]*(p2.z) + b_m_2[2]*(p3.z) + b_m_2[3]*(p4.z)))
             + ((pow(u,1)) * (b_m_1[0]*(p1.z) + b_m_1[1]*(p2.z) + b_m_1[2]*(p3.z) + b_m_1[3]*(p4.z)))
             + ((pow(u,0)) * (b_m_0[0]*(p1.z) + b_m_0[1]*(p2.z) + b_m_0[2]*(p3.z) + b_m_0[3]*(p4.z)));

  return newPoint;
}


point make_unit (point p){
    float divisor = sqrt (pow(p.x,2) + pow(p.y,2) + pow(p.z,2));
    if(divisor == 0)
    {
      p.x = 0; p.y = 0; p.z = 0;
      return p;
    }
    p.x = p.x / divisor;
    p.y = p.y / divisor;
    p.z = p.z / divisor;
    return p;
  }

  point cross_product (point u, point v){
     point product;
      product.x = u.y * v.z - v.y * u.z;
      product.y = v.x * u.z - u.x * v.z;
      product.z = u.x * v.y - v.x * u.y;
      return make_unit(product);
  }

void texload(int i,char *filename)
{

   Pic* img;
   img = jpeg_read(filename, NULL);
   glBindTexture(GL_TEXTURE_2D, texture[i]);
  // std::cout << "image size = " << img->nx << " x " << img->ny << std::endl;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // std::cout << filename << std::endl;
   glTexImage2D(GL_TEXTURE_2D,
   0,
  GL_RGB,
   img->nx,
   img->ny,
   0,
  GL_RGB,
  GL_UNSIGNED_BYTE,
   &img->pix[0]);
   pic_free(img);
}

point getCenter (point tangent1, point catmull1){
  point center1;
  center1.x = catmull1.x + a*tangent1.x;
  center1.y = catmull1.y + a*tangent1.y;
  center1.z = catmull1.z + a*tangent1.z;
  return center1;
}

point getTangent2 (point p1, point p2, point p3, point p4, float u){
  point newPoint;
  newPoint.x = ((3*pow(u,2)) * (b_m_3[0]*(p1.x) + b_m_3[1]*(p2.x) + b_m_3[2]*(p3.x) + b_m_3[3]*(p4.x)))
             + ((2*pow(u,1)) * (b_m_2[0]*(p1.x) + b_m_2[1]*(p2.x) + b_m_2[2]*(p3.x) + b_m_2[3]*(p4.x)))
             + ((pow(u,0)) * (b_m_1[0]*(p1.x) + b_m_1[1]*(p2.x) + b_m_1[2]*(p3.x) + b_m_1[3]*(p4.x)))
             ;

 newPoint.y = ((3*pow(u,2)) * (b_m_3[0]*(p1.y) + b_m_3[1]*(p2.y) + b_m_3[2]*(p3.y) + b_m_3[3]*(p4.y)))
             + ((2*pow(u,1)) * (b_m_2[0]*(p1.y) + b_m_2[1]*(p2.y) + b_m_2[2]*(p3.y) + b_m_2[3]*(p4.y)))
             + ((pow(u,0)) * (b_m_1[0]*(p1.y) + b_m_1[1]*(p2.y) + b_m_1[2]*(p3.y) + b_m_1[3]*(p4.y)))
             ;
 newPoint.z = ((3*pow(u,2)) * (b_m_3[0]*(p1.z) + b_m_3[1]*(p2.z) + b_m_3[2]*(p3.z) + b_m_3[3]*(p4.z)))
             + ((2*pow(u,1)) * (b_m_2[0]*(p1.z) + b_m_2[1]*(p2.z) + b_m_2[2]*(p3.z) + b_m_2[3]*(p4.z)))
             + ((pow(u,0)) * (b_m_1[0]*(p1.z) + b_m_1[1]*(p2.z) + b_m_1[2]*(p3.z) + b_m_1[3]*(p4.z)))
             ;


  return make_unit(newPoint);
}

void DrawCube(GLfloat centerPosX, GLfloat centerPosY, GLfloat centerPosZ, GLfloat edgeLength)
{
    GLfloat halfSideLength = edgeLength * 0.5f;

    GLfloat vertices2[] =
    {
        // front face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left

        // back face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top left
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom left

        // left face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left

        // right face
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left

        // top face
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // bottom left

        // bottom face                                                                                                                                                                           // right face
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // top left
        centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // top right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
        centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left                                                                                                                                                                                                                                                                                                                                                                                                                                                  centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right                                                                                                                                                                                                                                                                                                                                                                                                                                                  centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength  // bottom left
    };


    for (int j = 0; j < 6; j++){

     // texload(j,files[j]);
      glEnable(GL_TEXTURE_2D);

      glBindTexture(GL_TEXTURE_2D, texture[j]);
       glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
       glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);
      
      int i = j*12;

       if( j == 0 || j == 3){
        glBegin(GL_POLYGON);
         glTexCoord2f(0.0, 1.0);
         glVertex3f(vertices2[i], vertices2[i+1], vertices2[i+2]);
         glTexCoord2f(1.0, 1.0);
         glVertex3f(vertices2[i+3], vertices2[i+4], vertices2[i+5]);
         glTexCoord2f(1.0, 0.0);
         glVertex3f(vertices2[i+6], vertices2[i+7], vertices2[i+8]);
         glTexCoord2f(0.0, 0.0);
         glVertex3f(vertices2[i+9], vertices2[i+10], vertices2[i+11]);
         glEnd();
        glDisable(GL_TEXTURE_2D);
       }

       else if ( j== 1 || j == 2){
        glBegin(GL_POLYGON);
       glTexCoord2f(1.0, 1.0);
       glVertex3f(vertices2[i], vertices2[i+1], vertices2[i+2]);
       glTexCoord2f(0.0, 1.0);
       glVertex3f(vertices2[i+3], vertices2[i+4], vertices2[i+5]);
       glTexCoord2f(0.0, 0.0);
       glVertex3f(vertices2[i+6], vertices2[i+7], vertices2[i+8]);
       glTexCoord2f(1.0, 0.0);
       glVertex3f(vertices2[i+9], vertices2[i+10], vertices2[i+11]);
       glEnd();
      glDisable(GL_TEXTURE_2D);
       }

       else {
        glBegin(GL_POLYGON);
          glTexCoord2f(1.0, 0.0);
       glVertex3f(vertices2[i], vertices2[i+1], vertices2[i+2]);
       glTexCoord2f(0.0, 0.0);
       glVertex3f(vertices2[i+3], vertices2[i+4], vertices2[i+5]);
       glTexCoord2f(0.0, 1.0);
       glVertex3f(vertices2[i+6], vertices2[i+7], vertices2[i+8]);
       glTexCoord2f(1.0, 1.0);
       glVertex3f(vertices2[i+9], vertices2[i+10], vertices2[i+11]);
       glEnd();
      glDisable(GL_TEXTURE_2D);
       }
       
    }


}

void drawCube2 (GLfloat vertices2[], GLuint texture1){

  
     // texload(j,files[j]);
      glEnable(GL_TEXTURE_2D);

      glBindTexture(GL_TEXTURE_2D, texture1);
       glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
       glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);
      
      

      
        glBegin(GL_POLYGON);
         glTexCoord2f(0.0, 1.0);
         glVertex3f(vertices2[0], vertices2[1], vertices2[2]);
         glTexCoord2f(1.0, 1.0);
         glVertex3f(vertices2[3], vertices2[4], vertices2[5]);
         glTexCoord2f(1.0, 0.0);
         glVertex3f(vertices2[6], vertices2[7], vertices2[8]);
         glTexCoord2f(0.0, 0.0);
         glVertex3f(vertices2[9], vertices2[10], vertices2[11]);

         glTexCoord2f(0.0, 1.0);
         glVertex3f(vertices2[12], vertices2[13], vertices2[14]);
         glTexCoord2f(1.0, 1.0);
         glVertex3f(vertices2[15], vertices2[16], vertices2[17]);
         glTexCoord2f(1.0, 0.0);
         glVertex3f(vertices2[18], vertices2[19], vertices2[20]);
         glTexCoord2f(0.0, 0.0);
         glVertex3f(vertices2[21], vertices2[22], vertices2[23]);
         glEnd();
        glDisable(GL_TEXTURE_2D);
     
      
       
 
}

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
  delete[] filename;
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
   gluPerspective(60.0f, aspect, 0.01f, 1000.0f);

   glMatrixMode(GL_MODELVIEW);
   //glLoadIdentity();
}

void fillArrays (){
  spline curr = g_Splines[0];
  int curr_controlpoints = curr.numControlPoints;

   
    for (int i = 0; i < curr_controlpoints-1; i++){
      point p1;
      point p2;
      point p3;
      point p4;

      if(i == 0)
      {
        continue;
      } 

      else if ( i + 2 >= curr_controlpoints){
        continue;
      }

      else{
        p1 = curr.points[i-1]; p4 = curr.points[i+2];
      }

      p2 = curr.points[i]; p3 = curr.points[i+1];
      for (float u = 0.0 ; u <= 1.0; u+= 0.001){
          tangent[tngt_idx] = getTangent2(p1,p2,p3,p4,u);
          if(tngt_idx == 0){
            point v; v.x = 1; v.y = 1; v.z = 1;
            v = make_unit(v);
            normal[tngt_idx] = cross_product(tangent[tngt_idx], v);
          }
          else{
            normal[tngt_idx] = cross_product(binormal[tngt_idx-1], tangent[tngt_idx]);
          }
          
          binormal[tngt_idx] = cross_product(tangent[tngt_idx], normal[tngt_idx]);
          eye[tngt_idx] = catmull(p1,p2,p3,p4,u);
          center[tngt_idx] = getCenter(tangent[tngt_idx], eye[tngt_idx]);
          tngt_idx++;
          capacity = tngt_idx;
      }
    }

    tngt_idx = 0;
}
void myinit()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);   // set background color
    glEnable(GL_DEPTH_TEST);            // enable depth buffering
    glShadeModel(GL_SMOOTH); 

    glGenTextures(7, texture);
    for (int i = 0; i < 7; i++){
      texload(i, files[i]);
    }
    tangent = new point[g_Splines[0].numControlPoints * 1001];
    binormal = new point[g_Splines[0].numControlPoints * 1001];
    normal = new point[g_Splines[0].numControlPoints * 1001];
    center = new point[g_Splines[0].numControlPoints * 1001];
    eye = new point[g_Splines[0].numControlPoints * 1001];
    /*tangent[0].x = 0; tangent[0].y = 0; tangent[0].z = 0;
    point v;
    v.x = 1; v.y = 1; v.z = 1;
    v = make_unit(v);
    normal[0] = cross_product(tangent[0],v);
    binormal[0] = cross_product(normal[0],tangent[0]);
    center[0].x = 0; center[0].y = 0; center[0].z = 0;
    eye[0].x = 0; eye[0].y = 0; eye[0].z = 0;*/
    fillArrays();
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

void printPoint (point p, char* name){
  //std::cout << name << " = {" << p.x << " , " << p.y << " , " << p.z << "}" << std::endl;
}

float* getVertices (int index){
  //float toreturn[24];
  double fa = 0.07;
  point p1 = eye[index];
  point p2 = eye[index];

  point n = normal[index];
  point b = binormal[index];
  p2.z+=2;
  GLfloat toreturn[]= {
                p1.x + fa*(n.x-b.x) , p1.y + fa*(n.y-b.y), p1.z + fa*(n.z-b.z),
                p1.x + fa*(n.x+b.x) , p1.y + fa*(n.y+b.y), p1.z + fa*(n.z+b.z),
                p1.x - fa*(n.x-b.x) , p1.y - fa*(n.y-b.y), p1.z - fa*(n.z-b.z),
                p1.x - fa*(n.x+b.x) , p1.y - fa*(n.y+b.y), p1.z - fa*(n.z+b.z),

                p2.x + fa*(n.x-b.x) , p2.y + fa*(n.y-b.y), p2.z + fa*(n.z-b.z),
                p2.x + fa*(n.x+b.x) , p2.y + fa*(n.y+b.y), p2.z + fa*(n.z+b.z),
                p2.x - fa*(n.x-b.x) , p2.y - fa*(n.y-b.y), p2.z - fa*(n.z-b.z),
                p2.x - fa*(n.x+b.x) , p2.y - fa*(n.y+b.y), p2.z - fa*(n.z+b.z)
  };
  return toreturn;
}

void drawTracks(){

  int index = 0;
  double fa = 0.07;
  spline curr = g_Splines[0];
  int curr_controlpoints =  g_Splines[0].numControlPoints;
  for (int i = 0; i < curr_controlpoints-1; i++){
      point p1;
      point p2;
      point p3;
      point p4;

      if(i == 0)
      {
        continue;
      } 

      else if ( i + 2 >= curr_controlpoints){
        continue;
      }

      else{
        p1 = curr.points[i-1]; p4 = curr.points[i+2];
      }

      p2 = curr.points[i]; p3 = curr.points[i+1];

      for (float u = 0.0 ; u <= 1.0; u+= 0.001){
          float* temp = getVertices(index);
          drawCube2(temp,texture[6]);
          index++;
      }
    }
}

void display()
{
  /* draw 1x1 cube about origin */
  /* replace this code with your height field implementation */
  /* you may also want to precede it with your 
rotation/translation/scaling */

   int index = 0;
    
    // clear buffers
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glLoadIdentity(); // reset transformation

  // gluLookAt(0.0, 0.0,32.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0);
    gluLookAt(eye[tngt_idx].x, eye[tngt_idx].y,eye[tngt_idx].z, 
             center[tngt_idx].x,center[tngt_idx].y, center[tngt_idx].z, 
             binormal[tngt_idx].x, binormal[tngt_idx].y, binormal[tngt_idx].z);
    // gluLookAt(eye[tngt_idx].x, eye[tngt_idx].y,eye[tngt_idx].z, 
    //         center[tngt_idx].x,center[tngt_idx].y, center[tngt_idx].z, 
    //         0, 0,1);
    tngt_idx+=10;
    if(tngt_idx >= capacity)
      tngt_idx = 0;
    //glRotatef(45.0, 0.0f, 0.0f, 0.0f);
     glRotatef(g_vLandRotate[0], 1.0,0.0,0.0);
     glRotatef(g_vLandRotate[1], 0.0,1.0,0.0);
     glRotatef(g_vLandRotate[2], 0.0,0.0,1.0);
     glTranslatef(g_vLandTranslate[0]*10, g_vLandTranslate[1]*10, g_vLandTranslate[2]*10);
     glScaled(g_vLandScale[0], g_vLandScale[0], g_vLandScale[0]);
   
   printPoint(eye[tngt_idx],"eye");
   printPoint(center[tngt_idx], "center");
   printPoint(normal[tngt_idx],"up");
  // tngt_idx++;
   if(g_RenderMode == SOLID_TRIANGLES)
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
   else if (g_RenderMode == VERTICES)
      glPolygonMode( GL_FRONT_AND_BACK, GL_POINT );
   else   
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
   
    // start applying the transformations
    
    
 
  DrawCube(0.0f,0.0f,0.0f,200.0f);


  for (int i = 0; i < g_iNumOfSplines; i++){
    spline curr = g_Splines[i];
    int curr_controlpoints = curr.numControlPoints;

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < curr_controlpoints-1; i++){
      point p1;
      point p2;
      point p3;
      point p4;

      if(i == 0)
      {
        continue;
      } 

      else if ( i + 2 >= curr_controlpoints){
        continue;
      }

      else{
        p1 = curr.points[i-1]; p4 = curr.points[i+2];
      }

      p2 = curr.points[i]; p3 = curr.points[i+1];

      for (float u = 0.0 ; u <= 1.0; u+= 0.001){
          //point p = catmull(p1,p2,p3,p4,u);
          point p = eye[index];
          glColor3f(1.0, 1.0, 1.0);
          glVertex3f(p.x, p.y, p.z);
          index++;
          
      }
    }

    glEnd();


  
    index = 0;
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < curr_controlpoints-1; i++){
      point p1;
      point p2;
      point p3;
      point p4;

      if(i == 0)
      {
        continue;
      } 

      else if ( i + 2 >= curr_controlpoints){
        continue;
      }

      else{
        p1 = curr.points[i-1]; p4 = curr.points[i+2];
      }

      p2 = curr.points[i]; p3 = curr.points[i+1];

      for (float u = 0.0 ; u <= 1.0; u+= 0.001){
          //point p = catmull(p1,p2,p3,p4,u);
          point p = eye[index];
          p.z += 2;
          glColor3f(1.0, 1.0, 1.0);
          glVertex3f(p.x, p.y, p.z);
          index++;
          
      }
    }

    glEnd();


    index = 0;
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < curr_controlpoints-1; i++){
      point p1;
      point p2;
      point p3;
      point p4;

      if(i == 0)
      {
        continue;
      } 

      else if ( i + 2 >= curr_controlpoints){
        continue;
      }

      else{
        p1 = curr.points[i-1]; p4 = curr.points[i+2];
      }

      p2 = curr.points[i]; p3 = curr.points[i+1];

      for (float u = 0.0 ; u <= 1.0; u+= 0.001){
          //point p = catmull(p1,p2,p3,p4,u);
          point p = eye[index];
          p.z += 2;
          glColor3f(1.0, 1.0, 1.0);
          glVertex3f(p.x, p.y, p.z);
          index++;
          
      }
    }

    glEnd();
  }

  //drawTracks();


  glutSwapBuffers(); // double buffer flus
}





void doIdle()
{
  /* do some stuff... */

  /* make the screen update */
  glutPostRedisplay();
}

/* used to change rendermode state variable*/
void processSpecialKeys(int key, int x, int y) {

  switch(key) {
    case GLUT_KEY_F1 :
        g_RenderMode = WIREFRAMES; 
        std::cout << "pressed f1 " << std::endl;
        break;
    case GLUT_KEY_F2 :
        g_RenderMode = SOLID_TRIANGLES;
        std::cout << "pressed f2 " << std::endl;
        break;
    case GLUT_KEY_F3 :
        g_RenderMode = VERTICES; 
        std::cout << "pressed f3 " << std::endl;
        break;
    case GLUT_KEY_F4 :

     std::string pi = std::to_string(screenshotNum) + ".jpg";
    char *y = new char[pi.length() + 1]; 
    std::strcpy(y, pi.c_str());
        saveScreenshot(y);
        screenshotNum++;
        break;
  }
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_CTRL:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    
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

glutIdleFunc(doIdle);

  /* callback for mouse drags */
  glutMotionFunc(mousedrag);
  /* callback for idle mouse movement */
  glutPassiveMotionFunc(mouseidle);
  /* callback for mouse button changes */
  glutMouseFunc(mousebutton);

  /**callback for special keys*/
  glutSpecialFunc(processSpecialKeys);

 

  /* do initialization */
  myinit();

  glutMainLoop();
  return 0;
}
