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
//#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <vector>
#include <cstring>
#include <filesystem>
#include <regex>
#include <functional>
#include <numeric>

#include "logger.hpp"
#include "string_utils.hpp"

namespace fs = std::filesystem;

inline char pathseparator() {
	char c = fs::path::preferred_separator;
	return c;
};

inline std::string pathseparatorstring() {
	return (fs::path("") += fs::path::preferred_separator).string();
};

inline void fixseparator(std::string& path) {
	fs::path p(path);
	path = p.make_preferred().string();;
}

inline std::string fixseparator(const std::string& path) {
	fs::path p(path);
	return p.make_preferred().string();
};

inline void remove_trailing_separator(std::string& path)
{
	if (path.size() == 0) return;
	fixseparator(path);
	size_t len = path.size();
	while(path[len - 1] == pathseparator()) {
		path.erase(len - 1, 1);
		len = path.size();
	}
};

inline void remove_trailing_separator(fs::path& path)
{
	std::string s = path.string();
	remove_trailing_separator(s);
	path = fs::path(s);
};

inline void add_trailing_separator(std::string& path)
{
	fixseparator(path);
	if (path.back() == pathseparator()) return;
	path += pathseparatorstring();
};


inline bool makedirectory(const fs::path& dirname)
{
	if (dirname.string().size() == 0) return true;
	if (fs::exists(dirname)) return true;

	std::string p = dirname.string();
	remove_trailing_separator(p);

	fs::path dpath = fs::path(p).make_preferred();
	if (fs::exists(dpath)) return true;

	std::error_code ec;
	bool status = fs::create_directories(dpath, ec);
	if (status == false) {
		glog.warningmsg(_SRC_, "Could not create directory %s (%s)\n", dpath.string().c_str(), ec.message().c_str());
	}
	return status;
}

inline bool makedirectory_for(const fs::path& deeppath) {
	//deeppath can be directory or file
	fs::path p = deeppath;
	p.make_preferred();
	bool status = makedirectory(p.parent_path());
	if (status == false) {
		glog.errormsg(_SRC_, "Unable to create directory %s (for path %s)\n", p.parent_path().string().c_str(), deeppath.string().c_str());
	}
	return status;
};

// Chexks for existence and open failure
inline std::ifstream ifstream_ex(const fs::path filepath, const std::ios_base::openmode mode = std::ios_base::in) {
	fs::path fpath = fs::path(filepath).make_preferred();
	std::ifstream ifs(fpath, mode);
	if (ifs.fail()) {
		std::string errstr = std::strerror(errno);
		glog.errormsg(_SRC_, "Unable to open file %s (%s)\n", fpath.string().c_str(), errstr.c_str());
	}
	return ifs;
};

// Chexks for existence and open failure
inline std::ifstream fileopen(const fs::path filepath, const std::ios_base::openmode mode = std::ios_base::in){
	fs::path fpath = fs::path(filepath).make_preferred();
	std::ifstream ifs(fpath,mode);
	if (ifs.fail()){
		std::string errstr = std::strerror(errno);
		glog.errormsg(_SRC_, "Unable to open file %s (%s)\n", fpath.string().c_str(), errstr.c_str());	
	}
	return ifs;
};

// Chexks for open failure
inline std::ofstream ofstream_ex(const fs::path filepath, const std::ios_base::openmode mode = std::ios_base::out) {
	fs::path fpath = fs::path(filepath).make_preferred();
	bool status = makedirectory_for(fpath);
	if (status == false) {
		glog.errormsg(_SRC_, "Unable to create directory for file %s\n", fpath.string().c_str());
	}

	std::ofstream ofs(fpath, mode);
	if (ofs.fail()) {
		std::string errstr = std::strerror(errno);
		glog.errormsg(_SRC_, "Unable to open output file %s (%s)\n", fpath.string().c_str(), errstr.c_str());
	}
	return ofs;
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
};

inline std::string extractfilestem(const std::string& pathname)
{
	fs::path p = fs::path(pathname).make_preferred();
	return p.stem().string();
};

inline std::string extractfileextension(const std::string& pathname)
{
	fs::path p = fs::path(pathname).make_preferred();
	return p.filename().extension().string();
};

inline std::string insert_before_filename(const std::string& pathname, const std::string& insertion)
{
	FilePathParts fpp(pathname);
	return fpp.directory + insertion + fpp.stem + fpp.extension;
};

inline std::string insert_after_filename(const std::string& pathname, const std::string& insertion)
{
	FilePathParts fpp(pathname);
	return fpp.directory + fpp.stem + insertion + fpp.extension;
};

inline std::string insert_after_extension(const std::string& pathname, const std::string& insertion)
{
	FilePathParts fpp(pathname);
	return fpp.directory + fpp.stem + fpp.extension + insertion;
};

class DirectoryAccess
{

private:

public:

	static std::vector<std::string> getfilelist_single_pattern(std::string single_searchpattern)
	{
		trim_inplace(single_searchpattern);
		const std::string basepathname = extractfiledirectory_nosep(single_searchpattern);
		const std::string wildcard_pattern = extractfilename(single_searchpattern);

		const std::regex star_replace(R"(\*)");
		const std::regex questionmark_replace(R"(\?)");

		// Change 
		std::string regex_pattern = wildcard_pattern;
		regex_pattern = std::regex_replace(regex_pattern, star_replace, ".*");
		regex_pattern = std::regex_replace(regex_pattern, questionmark_replace, ".");
		std::regex match_regex(regex_pattern);

		std::vector<std::string> pathlist;
		auto it = fs::directory_iterator(basepathname);
		for (fs::directory_entry const& de : it) {
			if (de.is_regular_file()) {
				const std::string filename = de.path().filename().string();
				if (std::regex_match(filename, match_regex)) {
					const std::string pathname = de.path().string();
					pathlist.push_back(pathname);
				}
			}
		}
		return pathlist;
	};

	static std::vector<std::string> getfilelist_multi_pattern(const std::string& multisearchpattern)
	{
		std::vector<std::string> s = split(multisearchpattern, ';');
		std::vector<std::string> pathlist;
		for (size_t i = 0; i < s.size(); i++) {
			std::vector<std::string> l = DirectoryAccess::getfilelist_single_pattern(s[i]);
			pathlist.insert(pathlist.end(), l.begin(), l.end());
		}
		uniquify(pathlist);
		return pathlist;
	};

	static std::vector<std::string> getfilelist(const std::string& pathname) {
		std::vector<std::string> pathlist;
		auto it = fs::directory_iterator(fs::path(pathname).make_preferred());
		for (fs::directory_entry const& de : it) {
			if (de.is_regular_file()) {
				const std::string pathname = de.path().string();
				pathlist.push_back(pathname);
			}
		}
		return pathlist;
	};

	static std::vector<std::string> getfilelist(const std::string& pathname, std::string extension) {
		if (extension[0] != '.') extension = "." + extension;
		std::vector<std::string> pathlist;
		auto it = fs::directory_iterator(fs::path(pathname).make_preferred());
		for (fs::directory_entry const& de : it) {
			if (de.is_regular_file() && de.path().has_extension()){
				const std::string e = de.path().extension().string();
				if (extension == de.path().extension().string()) {
					pathlist.push_back(de.path().string());
				}
			}
		}
		return pathlist;
	};

	static std::vector<std::string> getfilelist_recursive(const std::string& pathname) {
		std::vector<std::string> pathlist;
		fs::directory_options options = fs::directory_options::skip_permission_denied;
		auto it = fs::recursive_directory_iterator(fs::path(pathname).make_preferred(),options);
		for (auto const& de : it) {
			std::cout << de.path().string() << std::endl;
			if (de.is_regular_file()) {
				const std::string pathname = de.path().string();
				pathlist.push_back(pathname);
			}
		}
		return pathlist;
	};

	static std::vector<std::string> getfilelist_recursive(const std::string& pathname, std::string extension) {
		if (extension[0] != '.') extension = "." + extension;
		std::vector<std::string> pathlist;
		fs::directory_options options = fs::directory_options::skip_permission_denied;
		auto it = fs::recursive_directory_iterator(fs::path(pathname).make_preferred(), options);
		for (auto const& de : it) {
			std::cout << de.path().string() << std::endl;
			if (de.is_regular_file()) {
				if (extension == de.path().extension().string()) {
					pathlist.push_back(de.path().string());
				}
			}
		}
		return pathlist;
	};
};

template <typename T, typename Comparator>
std::vector<size_t> sort_indices(const std::vector<T>& v, const Comparator& comparator = std::less<T>{}) {
	//Modified from https://stackoverflow.com/questions/1577475/c-sorting-and-keeping-track-of-indexes
	std::vector<size_t> idx(v.size());
	std::iota(idx.begin(), idx.end(), 0);
	std::stable_sort(idx.begin(), idx.end(), 
		[&v,&comparator](size_t i1, size_t i2) {
			return comparator(v[i1], v[i2]);
		});

	return idx;
};

inline std::vector<std::string> sortfilelistbysize(const std::vector<std::string>& filelist, bool ascending=true)
{
	const size_t n = filelist.size();
	std::vector<std::uintmax_t> filesize(n);

	for (size_t i = 0; i < n; i++) {
		filesize[i] = fs::file_size(filelist[i].c_str());
	}
	std::vector<size_t> indices;
	if(ascending == true)indices = sort_indices(filesize, std::less<std::uintmax_t>{});
	else indices = sort_indices(filesize, std::greater<std::uintmax_t>{});

	std::vector<std::string> slist(n);
	for (size_t i = 0; i < n; i++) {
		slist[i] = filelist[indices[i]];
	}
	return slist;
}

inline bool filegetline_ifs(std::ifstream& ifs, std::string& str) {
	str.clear();
	if(std::getline(ifs, str)) return true;
	else return false;
}

inline std::FILE* fileopen(const fs::path filepath, const std::string mode)
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
};

inline void fileclose(std::FILE* fp)
{
	if (fp) fclose(fp);
};

inline size_t countlines0(const std::string filename)
{
	FILE* fp = fopen(filename.c_str(), "rb");
	size_t buffersize = 4194304;
	std::vector<char> buffer(buffersize);
	size_t n = 0;
	size_t nread = 0;
	do {
		nread = fread(buffer.data(), 1, buffersize, fp);
		for (size_t k = 0; k < nread; k++) {
			if (buffer[k] == '\n')n++;
		}
	} while (nread > 0);
	fclose(fp);
	return n;
};

inline size_t countlines1(const std::string filename)
{
	size_t n = 0;
	std::ifstream in = fileopen(filename);
	std::string str;
	while (std::getline(in, str)) {
		++n;
	}
	return n;
};