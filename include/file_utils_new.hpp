/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <vector>
#include <cstring>
#include <filesystem>

#include "general_utils.hpp"

namespace fs = std::filesystem;

inline bool makedirectory(const fs::path& dirname)
{
	fs::path dpath(dirname);
	dpath.make_preferred();
	if (fs::exists(dpath)) return true;

	std::error_code ec;
	bool status = fs::create_directory(dpath, ec);
	if (status == false) {
		glog.warningmsg(_SRC_, "Could not create directory %s (%s)\n", dpath.string().c_str(), ec.message().c_str());
	}
	return status;
}

inline bool makedirectory_for(const fs::path& deeppath) {
	//deeppath can be directory or file
	fs::path p = deeppath;
	p.make_preferred();
	return makedirectory(p.parent_path());
};

inline void fileclose(FILE* fp)
{
	if (fp) fclose(fp);
}

inline FILE* fileopen(const fs::path filepath, const std::string mode)
{
	fs::path fpath(filepath);
	fpath.make_preferred();
	if (mode[0] == 'w' || mode[0] == 'a') {
		bool status = makedirectory_for(fpath);
		if (status == false) return nullptr;
	}
	else if (mode[0] == 'r') {
		if (fs::exists(fpath) == false) {
			glog.warningmsg(_SRC_, "Unable to open file %s (file does not exist)\n", fpath.string().c_str());
		}
	}

	FILE* fp = std::fopen(fpath.string().c_str(), mode.c_str());
	if (!fp) {
		glog.warningmsg(_SRC_, "Unable to open file %s\n", fpath.string().c_str());
	}
	return fp;
}

inline char pathseparator() {
	char c = fs::path::preferred_separator;
	return c;
};

inline std::string pathseparatorstring() {
	return (fs::path("") += fs::path::preferred_separator).string();
};

inline void fixseparator(std::string& path){
	fs::path p(path);
	path = p.make_preferred().string();;
}

inline std::string fixseparator(const std::string& path) {
	fs::path p(path);
	return p.make_preferred().string();
};

inline std::string getcurrentdirectory()
{
	return fs::current_path().string();
};

class FilePathParts {

public:
	std::string directory;
	std::string stem;
	std::string extension;
	
	FilePathParts() = delete;

	FilePathParts(const std::string& path) {
		fs::path p = path;
		p.make_preferred();
		directory = (p.parent_path() += fs::path::preferred_separator).string();
		stem = p.stem().string();
		extension = p.extension().string();
	};
};

inline std::vector<std::string> sortfilelistbysize(const std::vector<std::string>& filelist, int sortupordown)
{
	const size_t n = filelist.size();
	std::vector<int> index(n);
	std::vector<std::uintmax_t> size(n);

	for (size_t i = 0; i < n; i++) {
		index[i] = (int)i;
		size[i] = fs::file_size(filelist[i].c_str());
	}

	quicksortindex(size.data(), index.data(), 0, (int)n - 1, sortupordown);
	
	std::vector<std::string> slist;
	for (size_t i = 0; i < n; i++) {
		slist.push_back(filelist[(size_t)index[i]]);
	}
	return slist;
}

inline std::string extractfiledirectory_nosep(const std::string& pathname) {
	fs::path p = fs::path(pathname).make_preferred();
	return p.parent_path().string();
}

inline std::string extractfiledirectory(const std::string& pathname) {
	fs::path p = fs::path(pathname).make_preferred();
	return (p.parent_path() += fs::path::preferred_separator).string();
}

inline std::string extractfilepath_noextension(const std::string& pathname)
{
	fs::path p = fs::path(pathname).make_preferred();
	return p.replace_extension().string();
}

inline std::string extractfilename(const std::string& pathname)
{
	fs::path p = fs::path(pathname).make_preferred();
	return p.filename().string();	
}

inline std::string extractfilestem(const std::string& pathname)
{
	fs::path p = fs::path(pathname).make_preferred();
	return p.stem().string();
}

inline std::string extractfileextension(const std::string& pathname)
{
	fs::path p = fs::path(pathname).make_preferred();
	return p.filename().extension().string();
}

inline std::string insert_before_filename(const std::string& pathname, const std::string& insertion)
{
	FilePathParts fpp(pathname);
	return fpp.directory + insertion + fpp.stem + fpp.extension;
}

inline std::string insert_after_filename(const std::string& pathname, const std::string& insertion)
{
	FilePathParts fpp(pathname);
	std::string s = fpp.directory + fpp.stem + insertion + fpp.extension;
	return s;
}

inline std::string insert_after_extension(const std::string& pathname, const std::string& insertion)
{
	FilePathParts fpp(pathname);
	return fpp.directory + fpp.stem + fpp.extension + insertion;
}

