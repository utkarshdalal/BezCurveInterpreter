#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <omp.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <C:\Users\Utkarsh\Downloads\Documents\CS184\as3\glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>


#include "Curve.h"
#include "Point.h"
#include "Parser.h"
#include "Patch.h"

using namespace std;

//Global stuff to handle
char subDivType = 'u';
bool wireFrame = false;
bool flatShade = false;
bool hiddenLine = false;
float zoom = 1;
float rotateX = 0;
float rotateZ = 0;
float transX = 0;
float transY = 0;
float transZ = 0;
float step = 0;
int numPatches = 0;
Patch * myBezierPatch; //An array of all our patches
GLuint storedPN; //Stores the values computed during tessellation

class Viewport {
  public:
    int w, h; // width and height
};
Viewport	viewport;

// Reshape window if it's changed
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport (0,0,viewport.w,viewport.h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
}

//Function to do things when the keyboard is pressed.
void myKybdHndlr(unsigned char key, int x, int y){
	if (key == 's') {
		if (flatShade == false) {
			flatShade = true;
		} else {
			flatShade = false;
		}
	}
	if (key == 'w') {
		if (wireFrame == false) {
			wireFrame = true;
		} else {
			wireFrame = false;
		}
	}
	if (key == 'h') {
		if (hiddenLine == false) {
			hiddenLine = true;
		} else {
			hiddenLine = false;
		}
	}
	if (key == '+') {
		zoom -= .1;
	}
	if (key == '-') {
		zoom += .1;
	}
	glutPostRedisplay();

}

//Function to handle arrow keys and shift keys
void SpecialInput(int key, int x, int y) {
	int mod = -99999999;
	switch(key) {
		case GLUT_KEY_DOWN:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transZ += .1;
			} else {
				rotateX += .5;
			}
			break;
		case GLUT_KEY_UP:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transZ -= .1;
			} else {
				rotateX -= .5;
			}
			break;
		case GLUT_KEY_LEFT:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transX += .1;
			} else {
				rotateZ += .5;
			}
			break;
		case GLUT_KEY_RIGHT:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transX -= .1;
			} else {
				rotateZ -= .5;
			}
			break;
}

glutPostRedisplay();
}

/*# given a control patch and (u,v) values, find
# the surface point and normal*/
void tessellate(Patch* p, float u, float v, Point* x, Point* y) {
	//our vcurve
	Curve vCurve = Curve();
	//our ucurve
	Curve uCurve = Curve();
	Point tmpP1;
	Point tmpP2;

	//Build the first control points for u and v
	Curve tmpC = Curve(*(p->cPts[0][0]),*(p->cPts[0][1]),*(p->cPts[0][2]),*(p->cPts[0][3]));
	Curve tmpC2 = Curve(*(p->cPts[0][0]),*(p->cPts[1][0]),*(p->cPts[2][0]),*(p->cPts[3][0]));
	tmpC.interpolate(u);
	tmpP1 = tmpC.getPointP();
	tmpC2.interpolate(v);
	tmpP2 = tmpC2.getPointP();
	vCurve.setPoint1(tmpP1);
	uCurve.setPoint1(tmpP2);

	//Build the second control points for u and v
	tmpC = Curve(*(p->cPts[1][0]),*(p->cPts[1][1]),*(p->cPts[1][2]),*(p->cPts[1][3]));
	tmpC2 = Curve(*(p->cPts[0][1]),*(p->cPts[1][1]),*(p->cPts[2][1]),*(p->cPts[3][1]));
	tmpC.interpolate(u);
	tmpP1 = tmpC.getPointP();
	tmpC2.interpolate(v);
	tmpP2 = tmpC2.getPointP();
	vCurve.setPoint2(tmpP1);
	uCurve.setPoint2(tmpP2);

	//Build the third control points for u and v
	tmpC = Curve(*(p->cPts[2][0]),*(p->cPts[2][1]),*(p->cPts[2][2]),*(p->cPts[2][3]));
	tmpC2 = Curve(*(p->cPts[0][2]),*(p->cPts[1][2]),*(p->cPts[2][2]),*(p->cPts[3][2]));
	tmpC.interpolate(u);
	tmpP1 = tmpC.getPointP();
	tmpC2.interpolate(v);
	tmpP2 = tmpC2.getPointP();
	vCurve.setPoint3(tmpP1);
	uCurve.setPoint3(tmpP2);

	//Build the fourth control points for u and v
	tmpC = Curve(*(p->cPts[3][0]),*(p->cPts[3][1]),*(p->cPts[3][2]),*(p->cPts[3][3]));
	tmpC2 = Curve(*(p->cPts[0][3]),*(p->cPts[1][3]),*(p->cPts[2][3]),*(p->cPts[3][3]));
	tmpC.interpolate(u);
	tmpP1 = tmpC.getPointP();
	tmpC2.interpolate(v);
	tmpP2 = tmpC2.getPointP();
	vCurve.setPoint4(tmpP1);
	uCurve.setPoint4(tmpP2);

	// evaluate surface and derivative for u and v
	Point pt;
	Point dxdv;
	Point dxdu;
	vCurve.interpolate(v);
	dxdv = vCurve.getPointS();
	uCurve.interpolate(u);
	pt = uCurve.getPointP();
	dxdu = uCurve.getPointS();

	//# take cross product of partials to find normal
	*y = dxdu.getCrossProduct(dxdv);

	//solve for divide by zero nonsense in the normals
	if (y->magnitude() == 0){
		Point tempXA, tempNA;
		if ((u + step/2 <= 1) && (v + step/2 <= 1)){
			tessellate(p, u + step/2, v + step/2, &tempXA, &tempNA);
		} else if ((u + step/2 <= 1) && (v + step/2 > 1)){
			tessellate(p, u + step/2, v - step/2, &tempXA, &tempNA);
		}else if ((u  + step/2 > 1) && (v + step/2 > 1)){
			tessellate(p, u - step/2, v - step/2, &tempXA, &tempNA);
		} else {
			tessellate(p, u - step/2, v + step/2, &tempXA, &tempNA);
		}
		*y = tempNA;
	}

	//Finally normalize and return the point and the normal
	*y = y->normalize();
	*x = pt;
}


void adaptiveTessellate(Patch* p,float u1, float v1, float u2, float v2, float u3, float v3) {
	Point iMP12, n12,iMP23, n23,iMP31, n31,ep1, en1,ep2, en2,ep3, en3;
	float mp12[2] = {(u1+u2)/2, (v1+v2)/2};
	float mp23[2] = {(u2+u3)/2, (v2+v3)/2};
	float mp31[2] = {(u3+u1)/2, (v3+v1)/2};
	tessellate(p, mp12[0], mp12[1],&iMP12,&n12);
	tessellate(p, mp23[0], mp23[1],&iMP23, &n23);
	tessellate(p, mp31[0], mp31[1], &iMP31,&n31 );
	tessellate(p, u1, v1, &ep1, &en1);
	tessellate(p, u2, v2, &ep2, &en2);
	tessellate(p, u3, v3, &ep3, &en3);
	Point imp12 = ep1.addPoint(ep2).multiplyScalar(0.5);
	Point imp23 = ep2.addPoint(ep3).multiplyScalar(0.5);
	Point imp31 = ep3.addPoint(ep1).multiplyScalar(0.5);
	bool e12 = (imp12.subPoint(iMP12).magnitude()) > step;
	bool e23 = (imp23.subPoint(iMP23).magnitude()) > step;
	bool e31 = (imp31.subPoint(iMP31).magnitude()) > step;

	if (!e12 && !e23 && !e31) {
		glNormal3f(en1.getX(), en1.getY(), en1.getZ());
		glVertex3f(ep1.getX(), ep1.getY(), ep1.getZ());
		glNormal3f(en2.getX(), en2.getY(), en2.getZ());
		glVertex3f(ep2.getX(), ep2.getY(), ep2.getZ());
		glNormal3f(en3.getX(), en3.getY(), en3.getZ());
		glVertex3f(ep3.getX(), ep3.getY(), ep3.getZ());
        }
        else if (e12 && !e23 && !e31) {
        	adaptiveTessellate(p, mp12[0],mp12[1],u3,v3,u1,v1);
        	adaptiveTessellate(p,mp12[0], mp12[1],u2,v2,u3,v3);
        }
        else if (!e12 && e23 && !e31) {
        	adaptiveTessellate(p,mp23[0], mp23[1],u3,v3,u1,v1);
        	adaptiveTessellate(p,mp23[0], mp23[1],u1,v1,u2,v2);
        }
        else if (!e12 && !e23 && e31) {
        	adaptiveTessellate(p,mp31[0], mp31[1],u1,v1,u2,v2);
        	adaptiveTessellate(p, mp31[0], mp31[1],u2,v2,u3,v3);
        }
        else if (e12 && e23 && !e31) {
        	adaptiveTessellate(p,mp23[0], mp23[1],mp12[0],mp12[1],u2,v2);
        	adaptiveTessellate(p,mp23[0], mp23[1],u1,v1,mp12[0],mp12[1]);
        	adaptiveTessellate(p,mp23[0], mp23[1],u3,v3,u1,v1);
        }
        else if (!e12 && e23 && e31) {
        	adaptiveTessellate(p,mp31[0], mp31[1],mp23[0],mp23[1],u3,v3);
        	adaptiveTessellate(p,mp31[0], mp31[1],u2,v2,mp23[0],mp23[1]);
        	adaptiveTessellate(p,mp31[0], mp31[1],u1,v1,u2,v2);
        }
        else if (e12 && !e23 && e31) {
        	adaptiveTessellate(p,mp12[0], mp12[1],mp31[0],mp31[1],u1,v1);
        	adaptiveTessellate(p,mp12[0], mp12[1],u3,v3,mp31[0],mp31[1]);
        	adaptiveTessellate(p,mp12[0], mp12[1],u2,v2,u3,v3);
        }
        else if (e12 && e23 && e31) {
        	adaptiveTessellate(p,mp31[0], mp31[1],u1,v1,mp12[0],mp12[1]);
        	adaptiveTessellate(p,mp31[0], mp31[1],mp23[0],mp23[1],u3,v3);
        	adaptiveTessellate(p,mp12[0], mp12[1],u2,v2,mp23[0],mp23[1]);
        	adaptiveTessellate(p,mp31[0], mp31[1],mp12[0],mp12[1],mp23[0],mp23[1]);
        }
}

void initScene(){
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat amb[] = {0,0,0,0.1f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

}

void subDivision(Patch* p){
	Point x1,x2,x3,x4,n1,n2,n3,n4;
	int i;
	int j;
	int k;
	int l;
	int quadNum = 1/step;
	step = float(1)/quadNum;
	if (subDivType == 'a') {
	}
	for(i = 0; i <= quadNum; i++){
		float u = i*step;
		float uS = u + step;
		u = (u > 1) ? 1 : u;
		uS = (uS > 1) ? 1: uS;
		for (j = 0; j <= quadNum; j++){
			float v = j*step;
			float vS = v + step;
			v = (v > 1) ? 1 : v;
			vS = (vS > 1) ? 1: vS;
		    tessellate(p, u, v, &x1, &n1);
			tessellate(p, uS, v, &x2, &n2);
			tessellate(p, uS, vS, &x3, &n3);
			tessellate(p, u, vS, &x4, &n4);
			glNormal3f(n1.getX(),n1.getY(),n1.getZ());
			glVertex3f(x1.getX(),x1.getY(),x1.getZ());
			glNormal3f(n2.getX(),n2.getY(),n2.getZ());
			glVertex3f(x2.getX(),x2.getY(),x2.getZ());
			glNormal3f(n3.getX(),n3.getY(),n3.getZ());
			glVertex3f(x3.getX(),x3.getY(),x3.getZ());
			glNormal3f(n4.getX(),n4.getY(),n4.getZ());
			glVertex3f(x4.getX(),x4.getY(),x4.getZ());
		}
	}

}



// Draw stuff
void myDisplay() {
	glColor3f(0.0f,0.0f,1.0f);
  //clear color and depth buffer 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                // clear the color buffer (sets everything to black)

 // glMatrixMode(GL_PROJECTION);
  //glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
 // glLoadIdentity();                            // make sure transformation is "zero'd"
 // gluLookAt (0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);




  //----------------------- code to draw objects --------------------------
  // Rectangle Code
  //glColor3f(red component, green component, blue component);
                    // setting the color to pure blue 90% for the rect

//  glBegin(GL_TRIANGLES);                         // draw rectangle
//  //glVertex3f(x val, y val, z val (won't change the point because of the projection type));
//  glVertex3f(-1.0f,-0.25f,0.0f);//triangle one first vertex
//  glVertex3f(-0.5f,-0.25f,0.0f);//triangle one second vertex
//  glVertex3f(-0.75f,0.25f,0.0f);//triangle one third vertex
//  //drawing a new triangle
//  glVertex3f(0.5f,-0.25f,0.0f);//triangle two first vertex
//  glVertex3f(1.0f,-0.25f,0.0f);//triangle two second vertex
//  glVertex3f(0.75f,0.25f,0.0f);//triangle two third vertex




glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluPerspective (45*zoom, viewport.w/viewport.h, 0.01f, 200.0f);
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();
gluLookAt (0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
if(hiddenLine == true) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
glPushMatrix();
glTranslatef(-transX, transY, transZ);
glRotatef(rotateX, 1.0,0.0,0.0); //rotate about the x axis
glRotatef(rotateZ, 0.0,0.0,1.0); //rotate about the z axis
if(hiddenLine == true) {
	#pragma omp parallel for
	for (int i = 0; i < numPatches; i++) {
		if (subDivType == 'a'){
            glBegin(GL_TRIANGLES);
            adaptiveTessellate(&myBezierPatch[i],0,0,1,0,1,1);
            adaptiveTessellate(&myBezierPatch[i],0,0,1,1,0,1);
            glEnd();
		}
		else{
			glBegin(GL_QUADS);
			subDivision(&myBezierPatch[i]);
			glEnd();
		}
	}
	glPopMatrix();
	glColor3f(1.0f,0.0f,0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();
	glTranslatef(-transX, transY, transZ);
	glRotatef(rotateX, 1.0,0.0,0.0);
	glRotatef(rotateZ, 0.0,0.0,1.0);
}
if(hiddenLine == false) {
	if(wireFrame == true){
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}
if(flatShade == true){
	glShadeModel(GL_FLAT);
} else{
	glShadeModel(GL_SMOOTH);
}

#pragma omp parallel for
for (int i = 0; i < numPatches; i++) {
	if (subDivType == 'a'){
        glBegin(GL_TRIANGLES);
        adaptiveTessellate(&myBezierPatch[i],0,0,1,0,1,1);
        adaptiveTessellate(&myBezierPatch[i],0,0,1,1,0,1);
        glEnd();
	}
	else{
		glBegin(GL_QUADS);
		subDivision(&myBezierPatch[i]);
		glEnd();
	}
}

  glFlush();
  glutSwapBuffers();
}

int main(int argc, char *argv[]){
	if (argc > 1){
		std::string file = argv[1];
		step = atof(argv[2]);
		if (argc == 4) {
			std::string divType = argv[3];
			if (divType == "-a") {
				subDivType = 'a';
			};
		}
		Parser p;
		myBezierPatch = p.loadBez(file);
		numPatches = p.getNumPatches();
		storedPN = glGenLists(numPatches);
		glutInit(&argc, argv);

		//This tells glut to use a double-buffered window with red, green, and blue channels and depth
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

		// Initalize theviewport size
		viewport.w = 400;
		viewport.h = 400;

		//The size and position of the window
		glutInitWindowSize(viewport.w, viewport.h);
		glutInitWindowPosition(0,0);
		glutCreateWindow("Assignment 3");

		initScene(); 

		glutDisplayFunc(myDisplay);				// function to run when its time to draw something
		glutReshapeFunc(myReshape);				// function to run when the window gets resized
		glutKeyboardFunc(myKybdHndlr);			// function to handle keyboard inputs
		glutSpecialFunc(SpecialInput);			// function to handle special keyboard inputs

		glutMainLoop();							// infinite loop that will keep drawing and resizing

	}
	else{
		std::cout<<"Please input the name of the file."<<endl;
	}

	return 0;
}
