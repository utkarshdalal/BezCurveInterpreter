#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <C:/Users/Utkarsh/Downloads/Documents/CS184/as3-2/GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>


#include "Curve.h"
#include "Point.h"
#include "Parser.h"
#include "Patch.h"
#include "PointPair.h"

using namespace std;

//Global stuff to handle
char subDivType = 'u';
bool wireFrame = false;
bool flatShade = false;
float zoom = 1;
float rotateX = 0;
float rotateZ = 0;
float transX = 0;
float transY = 0;
float transZ = 0;
float step = 0;
int numPatches = 0;
Patch * myBezierPatch;
vector<Point> points;
vector<Point> normals;

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

// Need to revise, not a general purpose setpixel function as it assumes a lot
void setPixel(int x, int y, GLfloat r, GLfloat g, GLfloat b) {
  glColor3f(r, g, b);
  glVertex2f(x + 0.5, y + 0.5);   // The 0.5 is to target pixel
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
		// toggle between filled and hidden line mode
	}
	if (key == '+') {
		zoom += .1;
	}
	if (key == '-') {
		zoom -= .1;
	}
	glutPostRedisplay();

}

//Function to handle arrow keys and shift keys
void SpecialInput(int key, int x, int y) {
	int mod = -99999999;
	switch(key) {
		case GLUT_KEY_UP:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transZ += .1;
			} else {
				rotateX += 1;
			}
			break;
		case GLUT_KEY_DOWN:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transZ -= .1;
			} else {
				rotateX -= 1;
			}
			break;
		case GLUT_KEY_LEFT:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transX += .1;
			} else {
				rotateZ += 1;
			}
			break;
		case GLUT_KEY_RIGHT:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT) {
				transX -= .1;
			} else {
				rotateZ -= 1;
			}
			break;
}

glutPostRedisplay();
}

void initScene(){
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  glClearDepth(1.0f);
  glDepthFunc(GL_LEQUAL);
  glLineWidth(1);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat amb[] = {0,0,0,0.1f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb);

}

void uniformSubdivision(Point p1, Point p2, Point p3, Point p4, float u){
	float c = u;
	while (c <= 1.0){
	    double u1,u2,u3;
		u1 = 1 - c;
		u2 = u1 * u1 * u1;
		u3 = c * c * c;

		float x = u2*p1.getX() + 3*c*u1*u1*p2.getX() + 3*c*c*u1*p3.getX() + u3*p4.getX();
		float y = u2*p1.getY() + 3*c*u1*u1*p2.getY() + 3*c*c*u1*p3.getY() + u3*p4.getY();
		float z = u2*p1.getZ() + 3*c*u1*u1*p2.getZ() + 3*c*c*u1*p3.getZ() + u3*p4.getZ();

		glVertex3f(x*0.3, y*0.3, z*0.3);

		c += u;
	}
}

/*# given the control points of a bezier curve
# and a parametric value, return the curve 
# point and derivative*/
PointPair bezcurveinterp(Curve c, float u){
	/*# first, split each of the three segments
	# to form two new ones AB and BC*/
	Point A = c.getPoint1().multiplyScalar(1.0-u).addPoint(c.getPoint2().multiplyScalar(u));
	Point B = c.getPoint2().multiplyScalar(1.0-u).addPoint(c.getPoint3().multiplyScalar(u));
	Point C = c.getPoint3().multiplyScalar(1.0-u).addPoint(c.getPoint4().multiplyScalar(u));
	//now, split AB and BC to form a new segment DE
	Point D = A.multiplyScalar(1.0-u).addPoint(B.multiplyScalar(u));
	Point E = B.multiplyScalar(1.0-u).addPoint(C.multiplyScalar(u));
	/*finally, pick the right point on DE,
	this is the point on the curve*/
	Point p = D.multiplyScalar(1.0-u).addPoint(E.multiplyScalar(u));
	//compute derivative also
	Point dPdu = (E.subPoint(D).multiplyScalar(3));
	return PointPair(p, dPdu);
}

/*# given a control patch and (u,v) values, find 
# the surface point and normal*/
PointPair bezpatchinterp(Patch p, float u, float v){
	//# build control points for a Bezier curve in v
	Curve vcurve;
	vcurve.setPoint1(bezcurveinterp(p.getCurve1(), u).getPoint1());
	vcurve.setPoint2(bezcurveinterp(p.getCurve2(), u).getPoint1());
	vcurve.setPoint3(bezcurveinterp(p.getCurve3(), u).getPoint1());
	vcurve.setPoint4(bezcurveinterp(p.getCurve4(), u).getPoint1());
	//# build control points for a Bezier curve in u
	Curve ucurve;
	ucurve.setPoint1(bezcurveinterp(p.getVertCurve1(), u).getPoint1());
	ucurve.setPoint2(bezcurveinterp(p.getVertCurve2(), u).getPoint1());
	ucurve.setPoint3(bezcurveinterp(p.getVertCurve3(), u).getPoint1());
	ucurve.setPoint4(bezcurveinterp(p.getVertCurve4(), u).getPoint1());
	//# evaluate surface and derivative for u and v
	PointPair vinterp = bezcurveinterp(vcurve, v);
	PointPair uinterp = bezcurveinterp(ucurve, u);
	Point point = uinterp.getPoint1();
	Point dPdv = uinterp.getPoint2();
	Point dPdu = uinterp.getPoint2();
	//# take cross product of partials to find normal
	Point n = dPdu.getCrossProduct(dPdv);
	n = n.normalize();
	return PointPair(point, n);
}

	//given a patch, perform uniform subdivision
void subdividepatch(Patch p, float step){
/*# compute how many subdivisions there 
# are for this step size*/
	//float numdiv = ((1 + 0.00001) / step);
	//# for each parametric value of u
	double epsilon = 0.05;        
    double numdiv = (1.0 + epsilon) / step;
    const int n = (int)numdiv;
    double equalstep = 1.0/n;
	float u;
	float v;
	Point ps [100][100];
	Point ns [100][100];
	for (int iu = 0; iu <= n; iu ++){
		u = iu * equalstep;
		//# for each parametric value of v
		for (int iv = 0; iv <= n; iv ++){
			v = iv * equalstep;
			//#evaluate surface
			PointPair pair = bezpatchinterp(p, u, v);
			ps[iu][iv] = pair.getPoint1();
			ns[iu][iv] = pair.getPoint2();
			//savesurfacepointandnormal(p,n)
		}
	}
	for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            glBegin(GL_LINE_LOOP);
            
            //glNormal3f(ns[i][j].getX(),ns[i][j].getY(),ns[i][j].getZ());
            glVertex3f(ps[i][j].getX()*0.1,ps[i][j].getY()*0.1,ps[i][j].getZ()*0.1);
            
            //glNormal3f(ns[i+1][j].getX(),ns[i+1][j].getY(),ns[i+1][j].getZ());
            glVertex3f(ps[i+1][j].getX()*0.1,ps[i+1][j].getY()*0.1,ps[i+1][j].getZ()*0.1);
            
            //glNormal3f(ns[i+1][j+1].getX(),ns[i+1][j+1].getY(),ns[i+1][j+1].getZ());
            glVertex3f(ps[i+1][j+1].getX()*0.1,ps[i+1][j+1].getY()*0.1,ps[i+1][j+1].getZ()*0.1); 
            
            //glNormal3f(ns[i][j+1].getX(),ns[i][j+1].getY(),ns[i][j+1].getZ());           
            glVertex3f(ps[i][j+1].getX()*0.1,ps[i][j+1].getY()*0.1,ps[i][j+1].getZ()*0.1);
            
            glEnd();
            
        }
    }
}

// Draw stuff
void myDisplay() {
  //clear color and depth buffer 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);                // clear the color buffer (sets everything to black)
  //glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
  //glLoadIdentity();                            // make sure transformation is "zero'd"
  glPushMatrix();

  //----------------------- code to draw objects --------------------------
  // Rectangle Code
  //glColor3f(red component, green component, blue component);
  //glColor3f(0.0f,0.0f,1.0f);                   // setting the color to pure blue 90% for the rect

//  glBegin(GL_TRIANGLES);                         // draw rectangle
//  //glVertex3f(x val, y val, z val (won't change the point because of the projection type));
//  glVertex3f(-1.0f,-0.25f,0.0f);//triangle one first vertex
//  glVertex3f(-0.5f,-0.25f,0.0f);//triangle one second vertex
//  glVertex3f(-0.75f,0.25f,0.0f);//triangle one third vertex
//  //drawing a new triangle
//  glVertex3f(0.5f,-0.25f,0.0f);//triangle two first vertex
//  glVertex3f(1.0f,-0.25f,0.0f);//triangle two second vertex
//  glVertex3f(0.75f,0.25f,0.0f);//triangle two third vertex
  //glLineWidth(2.5);
  //glPointSize(10.0f);
  for (int i = 0; i < numPatches; i++) {
	 subdividepatch(myBezierPatch[i], step);
  }
  glFlush();
  glPopMatrix();
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
		glutInit(&argc, argv);

		//This tells glut to use a double-buffered window with red, green, and blue channels and depth
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

		// Initalize theviewport size
		viewport.w = 400;
		viewport.h = 400;

		//The size and position of the window
		glutInitWindowSize(viewport.w, viewport.h);
		glutInitWindowPosition(100,100);
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
