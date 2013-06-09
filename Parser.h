#ifndef PARSERH
#define PARSERH

#include <fstream>
#include <sstream>
#include <vector>
#include "Curve.h"
#include "Point.h"
#include "Patch.h"

class Parser {
	Patch * myBezierPatch;
	int a;
	int b;
	int numPatches;
public:

	Parser();

	Patch* loadBez(std::string file) {
		std::ifstream inpfile(file.c_str());
		  if(!inpfile.is_open()) {
		    std::cout << "Unable to open file" << std::endl;
		  } else {
		    std::string line;

		    while(inpfile.good()) {
		      std::vector<std::string> splitline;
		      std::string buf;

		      std::getline(inpfile,line);
		      std::stringstream ss(line);

		      while (ss >> buf) {
		        splitline.push_back(buf);
		      }
		      //Ignore blank lines
		      if(splitline.size() == 0) {
		        continue;
		      }
		      //The line with one number shall set the size of the array containing patches
		      if(splitline.size() == 1) {
		    	  numPatches = atof(splitline[0].c_str());
		    	  myBezierPatch = new Patch[numPatches];
		      } else {
		    	  myBezierPatch[b].cPts[a][0] = new Point(atof(splitline[0].c_str()), atof(splitline[1].c_str()), atof(splitline[2].c_str()));
		    	  myBezierPatch[b].cPts[a][1] = new Point(atof(splitline[3].c_str()), atof(splitline[4].c_str()), atof(splitline[5].c_str()));
		    	  myBezierPatch[b].cPts[a][2] = new Point(atof(splitline[6].c_str()), atof(splitline[7].c_str()), atof(splitline[8].c_str()));
		    	  myBezierPatch[b].cPts[a][3] = new Point(atof(splitline[9].c_str()), atof(splitline[10].c_str()), atof(splitline[11].c_str()));
		    	  a++;
		    	  if(a == 4){
		    		  a = 0;
		    		  b++;
		    	  }
		      }

		    }
		    inpfile.close();
		    return myBezierPatch;

		  }
	}

	int getNumPatches() {
		return numPatches;
	}


};

Parser::Parser() {
	a = 0;
	b = 0;
	numPatches = 0;
	myBezierPatch = new Patch[1];
}

#endif
