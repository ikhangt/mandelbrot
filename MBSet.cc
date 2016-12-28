// Calculate and display the Mandelbrot set
// ECE4122 final project, Fall 2016
// BY Ibrahim Khan
//4122 2016

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "complex.h"

using namespace std;

// Min and max complex plane values
Complex  minC(-2.0, -1.2);
Complex  maxC( 1.0, 1.8);
const int      maxIt = 2000;     // Max iterations for the set computations

const int width = 512, height = 512;
int windowID;
GLfloat minX = (float)minC.real;  GLfloat maxX = (float)maxC.real; GLfloat minY = (float)minC.imag; GLfloat maxY = (float)maxC.imag;
GLfloat stepX = (maxX - minX)/(GLfloat)width;
GLfloat stepY = (maxY - minY)/(GLfloat)height;

GLfloat black [] = {0.0f, 0.0f, 0.0f};
const int paletteSize = maxIt;
GLfloat palette[paletteSize][3];

const GLfloat radius = 2.0f;
bool fullScreen = false;

int activeThreads = 16;
pthread_mutex_t activeMutex;
pthread_cond_t doneCond;
GLfloat MandelbrotArr[width*height][3];

GLfloat* calculateColor(GLfloat real, GLfloat imag){
/* 
z -> z^2+C

z=x+yi
C=a+bi

z^2+C=(x+yi)*(x+yi)+a+bi
    =xx+yiyi+2xyi+a+bi
    =xx-yy+i(2xy+b)+a
    =(x^2-y^2+a)+i(2xy+b)
  //source: https://www.khanacademy.org/computer-programming/mandelbrot-set/1054272371
*/

GLfloat im = imag; GLfloat re = real; GLfloat tempRe=0.0;
 
 for(int i=0; i < maxIt; i++){
  tempRe = re*re - im*im + real;
  im = re * im * 2 + imag;
  re = tempRe;
  if( (re*re + im*im) > radius ){
   return palette[i];
  }
 }
 return  palette[maxIt-1];
}

void* Mandelbrot(void* v) {
 unsigned long threadNumber = (unsigned long)v; 
 int start = threadNumber*32;

 GLfloat starty = maxY - (threadNumber * (stepY*32));
 int end = threadNumber*32 + 512/16; //start and end condition for each threads

  int ix = 0;//index x 0-512
 for(GLfloat y = minY+start*stepY; y < minY+end*stepY;y += stepY){
  for(GLfloat x = minX; x < maxX; x += stepX){
   MandelbrotArr[start*512+ix][0] = calculateColor(x,y)[0]; MandelbrotArr[start*512+ix][1] = calculateColor(x,y)[1];MandelbrotArr[start*512+ix][2] = calculateColor(x,y)[2];//update RGB values
   ix++;

  }
 ix = 0;
 start++;
 }
 pthread_mutex_lock(&activeMutex);
 activeThreads--;
 if (activeThreads == 0) {
  pthread_cond_signal(&doneCond);
 }
 pthread_mutex_unlock(&activeMutex);
}

void display(void){ 
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
 glBegin(GL_POINTS); 

 int count = 0;
 for(int y = 0; y < height; y++) {
  for (int x = 0; x < width; x++) {
   glColor3fv(MandelbrotArr[count]);
   glVertex3f((GLfloat)x, (GLfloat)y, 0.0f); 
   count++;
 }
}
 glEnd(); 
 glutSwapBuffers(); // swap the buffers
}
void createPalette(){
 for (int i = 0; i < paletteSize-1; i++) {
  if (i < 10) {
   palette[i][0] = 0;palette[i][1] = 0;palette[i][2] = 1;
  }
  else {
   palette[i][0] = drand48(); palette[i][1] = drand48(); palette[i][2] = drand48();
  }
 }
  palette[paletteSize-1][0] = 0; palette[paletteSize-1][1] = 0; palette[paletteSize-1][2] = 0;
}
void init()
{ // Your OpenGL initialization code here
}

void reshape(int w, int h){ 
 stepX = (maxX-minX)/(GLfloat)w; 
 stepY = (maxY-minY)/(GLfloat)h; 
 glViewport (0, 0, (GLsizei)w, (GLsizei)h); 
 glutPostRedisplay(); 
}


void threadCreate(){
  pthread_mutex_init(&activeMutex, 0);
  pthread_cond_init(&doneCond, 0);
  pthread_mutex_lock(&activeMutex);
  for (int i =0; i < 16; i++) {
   pthread_t t;
   pthread_create(&t, 0, Mandelbrot, (void*)i);
  }
  pthread_cond_wait(&doneCond, &activeMutex);
  pthread_mutex_unlock(&activeMutex); 

}
void windowProperties(){
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  GLsizei windowX = (glutGet(GLUT_SCREEN_WIDTH)-width)/2;
  GLsizei windowY = (glutGet(GLUT_SCREEN_HEIGHT)-height)/2;
  glutInitWindowPosition(windowX, windowY);
  glutInitWindowSize(width, height);
  windowID = glutCreateWindow("Ibrahim Khan");

  glShadeModel(GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glViewport (0, 0, (GLsizei) width, (GLsizei) height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,width , 0, height, ((GLfloat)-1), (GLfloat)1);

}

int main(int argc, char** argv)
{

  glutInit(&argc, argv);
  createPalette();
  windowProperties();

  threadCreate();//will make a 16 threads that populate the array of pixel colors


  glutDisplayFunc(display);//will transfer pixels into the window
  glutReshapeFunc(reshape);

  glutMainLoop();
  return 0;
}
