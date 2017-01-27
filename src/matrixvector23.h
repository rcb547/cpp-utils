/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef matrixvector23_H
#define matrixvector23_H

template<class T> class Vector2{

public:

	T e1,e2;

	Vector2(){e1=e2=0;}
	Vector2(T x, T y){e1=x; e2=y;}	  
	T dot(const Vector2<T>& a, const Vector2<T>& b);
};

template<class T> class Matrix22{

public:

	T e11,e12;
	T e21,e22;
	Matrix22(){e11=e12=e21=e22=0.0;}
	Matrix22(T _e11, T _e12, T _e21, T _e22){e11=_e11;e12=_e12;e21=_e21;e22=_e22;}


	inline Matrix22<T> operator+(const Matrix22<T>& B)
	{
		Matrix22<T> C;		
		C.e11 = e11 + B.e11;
		C.e12 = e12 + B.e12;
		C.e21 = e21 + B.e21;
		C.e22 = e22 + B.e22;	 	 
		return C;
	};


	inline Matrix22<T> operator*(const Matrix22<T>& B)
	{
		Matrix22<T> C;		
		C.e11 = e11 * B.e11 + e12 * B.e21;
		C.e12 = e11 * B.e12 + e12 * B.e22;
		C.e21 = e21 * B.e11 + e22 * B.e21;
		C.e22 = e21 * B.e12 + e22 * B.e22;	 	 
		return C;
	};

};


//3
template<class T> class Vector3{

public:

	T e1,e2,e3;	

	Vector3(){e1=0.0; e2=0.0; e3=0.0;}
	Vector3(T x, T y, T z){e1=x; e2=y; e3=z;}	    		

	inline Vector3<T> operator+(const Vector3<T>& b)
	{
		Vector3<T> e(e1+b.e1, e2+b.e2, e3+b.e3);
		return e;		
	};

	inline Vector3<T> operator-(const Vector3<T>& b)
	{
		Vector3<T> e(e1-b.e1, e2-b.e2, e3-b.e3);
		return e;		
	};

	template<class Y> T dot(const Vector3<Y>& b)
	{
		return e1*b.e1 + e2*b.e2 + e3*b.e3;
	};
	
};


template<class T> class Matrix33{

public:

	T e11,e12,e13;
	T e21,e22,e23;
	T e31,e32,e33;	
	Matrix33(){e11=e12=e13=e21=e22=e23=e31=e32=e33=0.0;}
	Matrix33(T _e11, T _e12, T _e13, T _e21, T _e22, T _e23, T _e31, T _e32, T _e33 ){ e11=_e11; e12=_e12; e13=_e13; e21=_e21; e22=_e22; e23=_e23; e31=_e31; e32=_e32; e33=_e33;}
				 
	inline Matrix33<T> operator+(const Matrix33<T>& B){
		Matrix33<T> C;
		C.e11 = e11+B.e11;
		C.e12 = e12+B.e12;
		C.e13 = e13+B.e13;
		C.e21 = e21+B.e21;  
		C.e22 = e22+B.e22;  
		C.e23 = e23+B.e23;
		C.e31 = e31+B.e31;
		C.e32 = e32+B.e32;
		C.e33 = e33*B.e33;
		return C;
	}

	inline Matrix33<T> operator*(const Matrix33<T>& B){
		Matrix33<T> C;
		C.e11 = e11*B.e11 + e12*B.e21 + e13*B.e31;
		C.e12 = e11*B.e12 + e12*B.e22 + e13*B.e32;
		C.e13 = e11*B.e13 + e12*B.e23 + e13*B.e33;
		C.e21 = e21*B.e11 + e22*B.e21 + e23*B.e31;  
		C.e22 = e21*B.e12 + e22*B.e22 + e23*B.e32;  
		C.e23 = e21*B.e13 + e22*B.e23 + e23*B.e33;
		C.e31 = e31*B.e11 + e32*B.e21 + e33*B.e31;
		C.e32 = e31*B.e12 + e32*B.e22 + e33*B.e32;
		C.e33 = e31*B.e13 + e32*B.e23 + e33*B.e33;
		return C;
	}

	template<class Y> Vector3<T> operator*(const Vector3<Y>& x)
	{
		Vector3<T> b;
		b.e1 = e11*x.e1 + e12*x.e2 + e13*x.e3;
		b.e2 = e21*x.e1 + e22*x.e2 + e23*x.e3;
		b.e3 = e31*x.e1 + e32*x.e2 + e33*x.e3;
		return b;
	}
	
	inline Matrix33<T> transpose(const Matrix33<T>& A)
	{	
		Matrix33<T> B;
		B.e11 = A.e11;
		B.e12 = A.e21;
		B.e13 = A.e31;
		B.e21 = A.e12;
		B.e22 = A.e22;
		B.e31 = A.e13;
		B.e33 = A.e33;
		return B;
	}

	inline Matrix33<T> inverse(const Matrix33<T>& A)
	{	
		Matrix33<T> B;
		double det = A.e11*A.e22*A.e33 - A.e11*A.e23*A.e32 - A.e21*A.e12*A.e33 + A.e21*A.e13*A.e32 + A.e31*A.e12*A.e23 - A.e31*A.e13*A.e22;

		B.e11 =  A.e22*A.e33 - A.e23*A.e32;
		B.e12 = -A.e12*A.e33 + A.e13*A.e32;
		B.e13 =  A.e12*A.e23 - A.e13*A.e22;

		B.e11 = -A.e21*A.e33 + A.e23*A.e31;
		B.e12 =  A.e11*A.e33 - A.e13*A.e31;
		B.e13 = -A.e11*A.e23 + A.e13*A.e21;

		B.e11 =  A.e21*A.e32 - A.e22*A.e31;
		B.e12 = -A.e11*A.e32 + A.e12*A.e31;
		B.e13 =  A.e11*A.e22 - A.e12*A.e21;
		B *= (1.0/det);

		return B;
	}
};

////////////////////////////////////////////////////////////////////////////////
#endif
