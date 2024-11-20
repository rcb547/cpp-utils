//Copied from 
//https://stackoverflow.com/questions/52357/what-is-the-point-of-clog?rq=1

#pragma once

#include <fstream>
#include <iostream>

class cStreamRedirecter
{
public:
	cStreamRedirecter(std::ostream& dst, std::ostream& src)
		: src(src), sbuf(src.rdbuf(dst.rdbuf())) {}

	~cStreamRedirecter() { src.rdbuf(sbuf); }
private:

	std::ostream& src;
	std::streambuf* sbuf;
	// Prevent copying.                        
	cStreamRedirecter(const cStreamRedirecter&) = delete;
	cStreamRedirecter& operator=(const cStreamRedirecter&) = delete;
};

