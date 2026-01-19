//Copied from 
//https://stackoverflow.com/questions/52357/what-is-the-point-of-clog?rq=1

#pragma once

#include <fstream>
#include <iostream>
#include <streambuf>

namespace CppUtils {
	class StreamRedirecter {
	public:
		StreamRedirecter(std::ostream& dst, std::ostream& src)
			: src(src), sbuf(src.rdbuf(dst.rdbuf())) {
		}

		~StreamRedirecter() { src.rdbuf(sbuf); }
	private:

		std::ostream& src;
		std::streambuf* sbuf;
		// Prevent copying.
		StreamRedirecter(const StreamRedirecter&) = delete;
		StreamRedirecter& operator=(const StreamRedirecter&) = delete;
	};
};
