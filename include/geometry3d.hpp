/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _geomety3d_H
#define _geomety3d_H
#include <cmath>
#include <vector>
#include <cfloat>
#include "general_constants.hpp"

class cVec;//forward declarations
class cPnt;//forward declarations
class cLine;//forward declarations
class cLineSeg;//forward declarations


class cVec{
	
public:

	double x;
	double y;
	double z;
	
	cVec(){x=0.0; y=0.0; z=0.0;}
	
	cVec(const double& xo, const double& yo, const double& zo) {
		x=xo; y=yo; z=zo;
	}	
	
	cVec(const std::vector<double>& v) {
		x = v[0]; y = v[1]; z = v[2];
	}
	
	void set(double xo, double yo, double zo){x=xo; y=yo; z=zo;}
		
	cVec& operator=(const double& a)
	{
		x=a; y=a; z=a;
		return *this;
	}		
	
	cVec& operator=(const cVec& v){
		x=v.x; y=v.y; z=v.z;
		return *this;
	}		
	
	cVec& operator=(const std::vector<double>& v){
		x = v[0]; y = v[1]; z = v[2];
		return *this;
	}
			
	cVec& operator+=(const cVec& v){
		x += v.x; y += v.y; z += v.z;
		return *this;
	}
	
	cVec& operator-=(const cVec& v){
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}		
	
	cVec& operator*=(const cVec& v){
		x *= v.x; y *= v.y; z *= v.z;
		return *this;
	}		
	
	cVec& operator/=(const cVec& v){
		x /= v.x; y /= v.y; z /= v.z;
		return *this;
	}		
	
	cVec& operator*=(const double& s){
		x*=s; y*=s; z*=s;
		return *this;
	}	
	
	cVec& operator/=(const double& s){
		x/=s; y/=s; z/=s;
		return *this;
	}
	
	cVec operator+(const cVec& a){
		return cVec(x+a.x, y+a.y, z+a.z);
	}	
	
	cVec operator-(const cVec& a){
		return cVec(x-a.x, y-a.y, z-a.z);
	}
	
	cVec operator*(const cVec& a){
		return cVec(x*a.x, y*a.y, z*a.z);
	}
	
	cVec operator/(const cVec& a){
		return cVec(x/a.x, y/a.y, z/a.z);
	}
	
	cVec operator-(){
		return cVec (-x, -y, -z);
	}	
	
	friend cVec operator+(const cVec& a, const cVec& b){return cVec(a.x+b.x, a.y+b.y, a.z+b.z);}
	
	friend cVec operator-(const cVec& a, const cVec& b){return cVec(a.x-b.x, a.y-b.y, a.z-b.z);}
	
	friend cVec operator*(const cVec& v, double s){return cVec(v.x*s, v.y*s, v.z*s);}
	
	friend cVec operator*(const double& s, const cVec& v){return cVec(v.x*s, v.y*s, v.z*s);}
	
	friend cVec operator/(const cVec& v, double s){return cVec(v.x/s, v.y/s, v.z/s);}
	
	friend cVec operator/(const double& s, const cVec& v){return cVec(v.x/s, v.y/s, v.z/s);}
	
	int operator==(const cVec& a) const
	{
		if (fabs(x - a.x) < DBL_EPSILON && fabs(y - a.y) < DBL_EPSILON && fabs(z - a.z) < DBL_EPSILON)return 1;
		else return 0;
	}
	
	int operator!=(const cVec& a) const 
	{
		if (*this == a) return 0;
		else return 1;
	}
	
	cVec rotate(const double& angle, const cVec& axis)
	{
		cVec a = axis.unit();
		double cosangle = cos(angle*D2R);
		double sinangle = sin(angle*D2R);

		double xx = a.x*a.x;   double yy = a.y*a.y;   double zz = a.z*a.z;
		double xy = a.x*a.y;   double xz = a.x*a.z;   double yz = a.y*a.z;

		double mat[9];

		double oneminuscosangle = (1.0 - cosangle);

		mat[0] = xx + cosangle * (1 - xx);
		mat[4] = yy + cosangle * (1 - yy);
		mat[8] = zz + cosangle * (1 - zz);

		mat[1] = xy * oneminuscosangle - a.z*sinangle;
		mat[3] = xy * oneminuscosangle + a.z*sinangle;

		mat[2] = xz * oneminuscosangle + a.y*sinangle;
		mat[6] = xz * oneminuscosangle - a.y*sinangle;

		mat[5] = yz * oneminuscosangle - a.x*sinangle;
		mat[7] = yz * oneminuscosangle + a.x*sinangle;

		return cVec(
			mat[0] * x + mat[1] * y + mat[2] * z,
			mat[3] * x + mat[4] * y + mat[5] * z,
			mat[6] * x + mat[7] * y + mat[8] * z
		);
	}

	double length_squared() const
	{
		return x*x + y*y + z*z;
	}

	double length() const
	{
		return std::sqrt(x*x + y*y + z*z);
	}

	double length2n(const double& power)
	{
		return std::pow(x*x + y*y + z*z,0.5*power);
	}

	void unitise(){	
		double len = std::sqrt(x*x + y*y + z*z);
		x/=len; y/=len; z/=len;
	}
	
	cVec unit() const 
	{
		cVec v(x,y,z);
		v.unitise();
		return v;
	}
	
	double dot(const cVec& v) const
	{
		return v.x*x + v.y*y + v.z*z;
	}
	
	static double dot(const cVec& a, const cVec& b)
	{
		return a.dot(b);
	}
	
	cVec cross(const cVec& b) const 
	{		
		cVec c;//c = a x b (a is this cVec)
		c.x = y*b.z - b.y*z;
		c.y = z*b.x - b.z*x;
		c.z = x*b.y - b.x*y;
		return c;
	}		
	
	static cVec cross(const cVec& a, const cVec& b)
	{
		return a.cross(b);
	}

};

class cPnt : public cVec{

	public:

	cPnt() : cVec(0.0, 0.0, 0.0) {}

	cPnt(const cVec& v) : cVec(v) {}		

	cPnt(const double& xo, const double& yo, const double& zo) : cVec(xo,yo,zo){}
		
	cPnt(const std::vector<double>& v){
		x = v[0]; y = v[1]; z = v[2];
	}

	cPnt& operator=(const std::vector<double>& v)
	{
		x = v[0]; y = v[1]; z = v[2];
		return *this;
	}

	cPnt& operator=(const cVec& v){
		x = v.x; y = v.y; z = v.z;
		return *this;	
	}

	cPnt operator+(const cVec& a){
		return cPnt(x+a.x, y+a.y, z+a.z);
	}
	
	cPnt operator-(const cVec& a){
		return cPnt(x-a.x, y-a.y, z-a.z);
	}

	double distance(const cPnt& p) const 
	{				
		const double dx = p.x - x;
		const double dy = p.y - y;
		const double dz = p.z - z;
		return std::sqrt(dx*dx + dy*dy + dz*dz);
	}

	static cVec unitnormal(const cPnt& p1, const cPnt& p2, const cPnt& p3)
	{
		//p1 p2 p3 are counter clockwise
		//about the returned normal
		cVec a = p2 - p1;
		cVec b = p3 - p2;
		cVec n = a.cross(b);
		n.unitise();
		return n;
	}

};

class cLine{

protected:

	cVec PLL;
	cPnt PNT;

public:

	cLine(const cPnt& p, const cPnt& q){
		PLL=(p-q).unit();
		PNT=p;
	}

	cLine(const cVec& v, const cPnt& p){
		PLL = v.unit();
		PNT = p;
	}
	cLine(){PLL=cVec(); PNT=cPnt();}

	const cVec pll() const { return PLL; }
	const cPnt pnt() const { return PNT; }

	void setpll(const cVec& v){PLL=v.unit();}
	void setpnt(const cPnt& p){PNT=p;}
	void set(const cVec& v, const cPnt& p){PLL=v.unit(); PNT=p;}
		
	int operator==(const cLine& m) const 
	{
		if (PLL == m.PLL) {
			cVec v = PNT - m.PNT;
			v.unitise();
			if (PLL == v || PLL == -v) return 1;
		}
		return 0;
	}

	int operator!=(const cLine& m)
	{
		if (*this == m) return 0;
		else return 1;
	}

	inline cLine operator=(const cLine& m)
	{
		PLL = m.PLL;
		PNT = m.PNT;
		return *this;
	}
	
	double distance(const cPnt& p) const
	{
		cVec v = PLL.cross(p - PNT);
		return v.length();
	}

	double distance(const cLine& n) const 
	{
		cVec w = PNT - n.PNT;
		if (w.length() < DBL_EPSILON) return 0;
		cVec v = n.PLL.cross(PLL);
		if (v.length() < DBL_EPSILON) return n.distance(PNT);
		return fabs(w.dot(v)) / v.length();
	}

	cPnt closestpointonline(const cPnt& p)
	{
		double s = PLL.dot(p - PNT);
		return PNT + s*PLL;
	}

	int online(const cPnt& p)
	{
		if (p == PNT) return 1;
		cVec a = p - PNT;
		a.unitise();
		if (PLL == a || PLL == -a) return 1;
		else return 0;
	}

	static int closestpoints(const cLine& m, const cLine& n, cPnt &p, cPnt &q)
	{
		const double AD = m.PNT.dot(n.PLL);
		const double AB = m.PNT.dot(m.PLL);
		const double BC = m.PLL.dot(n.PNT);
		const double BD = m.PLL.dot(n.PLL);
		const double CD = n.PNT.dot(n.PLL);

		if ((BD*BD - 1.0) < DBL_EPSILON)return 0;

		double t = (AD - AB * BD + BC * BD - CD) / (1.0 - BD * BD);
		q = n.PNT + t*n.PLL;
		double s = -1.0 * m.PLL.dot(m.PNT - q);
		p = m.PNT + s*m.PLL;

		return 1;
	}
};

class cLineSeg{

protected:

	cPnt P;
	cPnt Q;

public:

	cLineSeg(){
		P=cPnt();
		Q=cPnt();
	}

	cLineSeg(const cPnt& p, const cPnt& q){
		P=p;
		Q=q;
	}

	const cPnt p() const {return P;}
	const cPnt q() const {return Q;}

	void setp(const cPnt& p){P=p;}
	void setq(const cPnt& q){Q=q;}
	void set(const cPnt& p, cPnt& q){P=p; Q=q;}

	cPnt closestpoint(const cPnt& pnt)
	{
		cLine m(P,Q);
		cPnt c = m.closestpointonline(pnt);

		double dx = std::fabs(P.x - Q.x);
		double dy = std::fabs(P.y - Q.y);

		if (dx > dy) {
			if (c.x < P.x && c.x < Q.x) {}
			else if (c.x > P.x && c.x > Q.x) {}
			else return c;
		}
		else if (dx < dy) {
			if (c.y < P.y && c.y < Q.y) {}
			else if (c.y > P.y && c.y > Q.y) {}
			else return c;
		}

		double dp = pnt.distance(P);
		double dq = pnt.distance(Q);

		if (dp < dq)return P;
		else return Q;
	}
};

#endif

