//Copied from 
//https://stackoverflow.com/questions/52357/what-is-the-point-of-clog?rq=1

#ifndef _streamredirecter_H
#define _streamredirecter_H

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
	std::streambuf* const sbuf;
	// Prevent copying.                        
	cStreamRedirecter(const cStreamRedirecter&) = delete;
	cStreamRedirecter& operator=(const cStreamRedirecter&) = delete;
};

/*
std::streambuf* open_cerr(std::ofstream& wlog, const std::string& logfilepath) {
	wlog = std::ofstream(logfilepath, std::ios::app);
	std::streambuf* cerrbuf = std::cerr.rdbuf();
	std::cerr.rdbuf(wlog.rdbuf());
	return cerrbuf;
}

void close_cerr(std::streambuf* cerrbuf) {
	std::cerr.rdbuf(cerrbuf);
}
*/

#endif
