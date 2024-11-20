/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _polygon_H
#define _polygon_H

#include <vector>
#include "general_types.hpp"
#include "general_utils.hpp"
#include "file_utils.hpp"

class cPolygon {
	
  public:

	  std::vector<cPoint> vertex;

	  cPolygon(){};

	  cPolygon(const std::string& filename){
		  loadfromfile(filename);
	  };
  
	  bool loadfromfile(const std::string& filename)
	  {
		  vertex.resize(0);
		  std::string str;
		  FILE* fp = fileopen(filename, "r");
		  while(filegetline(fp,str)){
			  std::vector<std::string> t = tokenize(str);
			  cPoint p;
			  p.x = atof(t[0].c_str());
			  p.y = atof(t[1].c_str());
			  vertex.push_back(p);
		  }
		  fclose(fp);

		  if (vertex[0] == vertex[vertex.size()-1]){
			  vertex.resize(vertex.size() - 1);
		  }
		  
		  return true;
	  };
	  	  
	  bool isinside(const cPoint& p) const
	  {
		  size_t nv = vertex.size();
		  //The following code is by Randolph Franklin, it returns 1 for interior points and 0 for exterior points. 
		  int c = 0;
		  size_t i, j;
		  for (i = 0, j = nv - 1; i < nv; j = i++) {
			  if ((((vertex[i].y <= p.y) && (p.y < vertex[j].y)) ||
				  ((vertex[j].y <= p.y) && (p.y < vertex[i].y))) &&
				  (p.x < (vertex[j].x - vertex[i].x) * (p.y - vertex[i].y) / (vertex[j].y - vertex[i].y) + vertex[i].x))
				  c = !c;
		  }
		  if (c == 0)return false;
		  else return true;
	  };

};
#endif
