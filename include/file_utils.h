/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _file_utils_H
#define _file_utils_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <vector>
#include <cstring>

#if defined _WIN32
#include <io.h>
#include <sys/stat.h>
#include <direct.h>
#else	
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/resource.h>
#include <unistd.h>
#define strnicmp strncasecmp
#endif

#include "general_utils.h"

inline char pathseparator()
{
#if defined _WIN32
	return '\\';
#else 
	return '/';
#endif
}

inline std::string pathseparatorstring()
{
#if defined _WIN32
	return "\\";
#else 
	return "/";
#endif
}

struct sFilePathParts {
	std::string directory;
	std::string prefix;
	std::string extension;
};

class cDirectoryAccess
{

private:
	static std::vector<std::string> getfilelist_single(const std::string& searchpattern);
	char pathsepchar;

public:

	cDirectoryAccess()
	{
		pathsepchar = pathseparator();
	}

	~cDirectoryAccess()
	{

	}


	static std::vector<std::string> getfilelist(const std::string& searchpattern);
	static bool wildcmp(std::string& wildpattern, std::string& stringpattern);

private:

};

inline void fixseparator(std::string& path)
{
#if defined _WIN32
	for (size_t i = 0; i < path.length(); i++)if (path[i] == '/')  path[i] = '\\';
#else
	for (size_t i = 0; i < path.length(); i++)if (path[i] == '\\') path[i] = '/';
#endif		
}

inline std::string fixseparator(const std::string& path)
{
	std::string result = path;
	fixseparator(result);
	return result;
}

inline void removetrailingseparator(std::string& path)
{
	if (path.size() <= 0)return;
	fixseparator(path);
	size_t len = path.length();
	if (path[len - 1] == pathseparator()) {
		std::string p = path.substr(0, len - 1);
		path = p;
	}
}

inline void addtrailingseparator(std::string& path)
{
	removetrailingseparator(path);
	path += pathseparatorstring();
}

inline bool exists(std::string path)
{
	fixseparator(path);
	removetrailingseparator(path);
#if defined _WIN32
	if (_access(path.c_str(), 0) == 0)return true;
	else return false;
#else
	if (access(path.c_str(), 0) == 0)return true;
	else return false;
#endif
}

inline bool isdirectory(std::string path)
{
	fixseparator(path);
	removetrailingseparator(path);
#if defined _WIN32
	struct _stat64i32 status;
	_stat(path.c_str(), &status);
	if (status.st_mode & _S_IFDIR)return true;
	else return false;
#else
	struct stat status;
	stat(path.c_str(), &status);
	if (status.st_mode & S_IFDIR)return true;
	else return false;
#endif        
}

inline bool isfile(std::string path)
{
	fixseparator(path);
	removetrailingseparator(path);
#if defined _WIN32
	struct _stat64i32 status;
	_stat(path.c_str(), &status);
	if (status.st_mode & _S_IFREG)return true;
	else return false;
#else
	struct stat status;
	stat(path.c_str(), &status);
	if (status.st_mode & S_IFREG)return true;
	else return false;
#endif     
}

inline bool isabsolutepath(std::string path)
{
#if defined _WIN32
	if (path[1] == ':')return true;
	else return false;
#else
	if (path[0] == pathseparator())return true;
	else return false;
#endif		
}

inline FILE* fileopen(const std::string filepath, const std::string mode)
{
	std::string path = filepath;
	fixseparator(path);
	if (mode[0] == 'w' || mode[0] == 'a') {
		std::string dirname = extractfiledirectory(path);
		removetrailingseparator(dirname);
		if (dirname.size() > 0) {
			if (exists(dirname) == false) {
				if (makedirectorydeep(dirname) == false) {
					glog.warningmsg(_SRC_, "Unable to make directory for file %s\n", path.c_str());
					return (FILE*)NULL;
				}
			}
		}
	}

	FILE* fp = fopen(path.c_str(), mode.c_str());
	if (!fp) {
		glog.warningmsg(_SRC_, "Unable to open file %s\n", path.c_str());
	}
	return fp;
}

inline std::string getcurrentdirectory()
{
	char buf[1024];
#if defined _WIN32
	char* ret = _getcwd(buf, sizeof(buf));
#else
	char* ret = getcwd(buf, sizeof(buf));
#endif
	if (ret) return std::string(buf);
	else return std::string();
}

inline std::vector<std::string> directoryheirachy(std::string dirname)
{
	fixseparator(dirname);
	addtrailingseparator(dirname);
	std::vector<std::string> v;
	size_t last = 0;
	for (size_t i = 0; i < dirname.size(); i++) {
		if (dirname[i] == pathseparator()) {
			std::string s = dirname.substr(last, i - last);
			v.push_back(s);
			last = i + 1;
		}
	}
	return v;
}

inline bool makedirectory(std::string dirname)
{
	if (dirname.size() == 0)return true;
	fixseparator(dirname);
	removetrailingseparator(dirname);
	if (exists(dirname))return true;
	int status;
#if defined _WIN32
	status = _mkdir(dirname.c_str());
	if (status != 0) {
		if (errno == EEXIST) {
			return true;
		}
		else {
			glog.warningmsg("makedirectory(): Problem creating directory because %s is an invalid path\n", dirname.c_str());
			return false;
		}
	}
	return true;
#else		
	status = mkdir(dirname.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status != 0) {
		if (errno == EEXIST) {
			return true;
		}
		else {
			glog.warningmsg("makedirectory(): Problem creating directory because %s is an invalid path\n", dirname.c_str());
			return false;
		}
	}
	return true;
#endif		
}

inline bool makedirectorydeep(std::string dirname)
{
	if (dirname.size() == 0)return true;
	if (exists(dirname))return true;
	bool status = false;
	std::vector<std::string> h = directoryheirachy(dirname);
	std::string p;
	for (size_t i = 0; i < h.size(); i++) {
		p += h[i] + pathseparatorstring();
		status = makedirectory(p);
	}
	return status;
}

inline int copyfile(std::string src, std::string dest)
{
	fixseparator(src);
	fixseparator(dest);
	int status;
	std::string cmd;
#if defined _WIN32
	cmd = "copy " + src + " " + dest;
#else		
	cmd = "cp " + src + " " + dest;;
#endif

	status = system(cmd.c_str());
	if (status != 0) {
		glog.warningmsg("copyfile(): Error copying %s to %s\n", src.c_str(), dest.c_str());
	}

	return status;
}

inline int deletefile(std::string src)
{
	fixseparator(src);
	if (exists(src) == false) return 0;

	int status;
	std::string cmd;
#if defined _WIN32
	cmd = "del " + src;
#else		
	cmd = "rm " + src;
#endif

	status = system(cmd.c_str());
	if (status != 0) {
		glog.warningmsg("deletefile(): Error deleting %s\n", src.c_str());
	}

	return status;
}

inline sFilePathParts getfilepathparts(const std::string& path)
{
	sFilePathParts fpp;
	std::string p = path;

	fixseparator(p);
	int len = (int)p.size();
	int isep = -1;
	for (auto i = 0; i < len; i++) {
		if (p[i] == pathseparator()) isep = i;
	}

	int iext = len;
	for (auto i = isep + 1; i < len; i++) {
		if (p[i] == '.') iext = i;
	}

	fpp.directory = p.substr(0, (size_t)isep + 1);
	fpp.prefix = p.substr((size_t)isep + 1, (size_t)iext - isep - 1);
	fpp.extension = p.substr(iext, (size_t)len - iext);
	return fpp;
}

inline std::string extractfiledirectory(const std::string& pathname)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	return fpp.directory;
}

inline std::string extractfilename(const std::string& pathname)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	return fpp.prefix + fpp.extension;
}

inline std::string extractfilename_noextension(const std::string& pathname)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	return fpp.prefix;
}

inline std::string extractfilepath_noextension(const std::string& pathname)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	return fpp.directory + fpp.prefix;
}

inline std::string extractfileextension(const std::string& pathname)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	return fpp.extension;
}

inline std::string insert_before_filename(const std::string& pathname, const std::string& insertion)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	return fpp.directory + insertion + fpp.prefix + fpp.extension;
}

inline std::string insert_after_filename(const std::string& pathname, const std::string& insertion)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	std::string s = fpp.directory + fpp.prefix + insertion + fpp.extension;
	return s;
}

inline std::string insert_after_extension(const std::string& pathname, const std::string& insertion)
{
	sFilePathParts fpp = getfilepathparts(pathname);
	return fpp.directory + fpp.prefix + fpp.extension + insertion;
}

inline int64_t filesize(const std::string& path)
{
#if defined _WIN32
	struct _stat64 st;
	_stat64(path.c_str(), &st);
	int64_t size = st.st_size;
	return size;
#else
	struct stat st;
	stat(path.c_str(), &st);
	int64_t size = st.st_size;
	return size;
#endif
}

inline std::vector<std::string> sortfilelistbysize(std::vector<std::string>& filelist, int sortupordown)
{
	size_t n = filelist.size();
	int* index = new int[n];
	int64_t* size = new int64_t[n];

	for (size_t i = 0; i < n; i++) {
		index[i] = (int)i;
		size[i] = filesize(filelist[i].c_str());
	}
	quicksortindex(size, index, 0, (int)n - 1, sortupordown);
	std::vector<std::string> slist;
	for (size_t i = 0; i < n; i++) {
		slist.push_back(filelist[(size_t)index[i]]);
	}
	return slist;
}

inline std::vector<std::string> getfilelist(const std::string& path, const std::string& extension)
{
	cDirectoryAccess d;
	std::vector<std::string> filelist;
	std::string p = path;
	if (p[p.size() - 1] != pathseparator()) {
		p.push_back(pathseparator());
	}
	std::string searchpattern = p + "*";
	if (extension.size() > 0) searchpattern += "." + extension;
	std::vector<std::string> list = d.getfilelist(searchpattern);
	for (size_t i = 0; i < list.size(); i++) {
		//std::string fullpath = p + list[i];
		//filelist.push_back(fullpath);		
		filelist.push_back(list[i]);
	}
	std::sort(filelist.begin(), filelist.end());
	return filelist;
}

inline void recursivefilelist(const std::string& path, const std::string& extension, FILE* outfile)
{
	std::vector<std::string> files = getfilelist(path, extension);
	for (size_t i = 0; i < files.size(); i++) {
		std::string f = files[i];
		if (outfile != NULL) {
			fprintf(outfile, "%s\n", f.c_str());
		}
		else printf("%s\n", f.c_str());
		size_t len = f.size();
		if (strcmp(&(f[len - 1]), ".") == 0)continue;
		if (strcmp(&(f[len - 2]), "..") == 0)continue;
		recursivefilelist(f, extension, outfile);
	}

}

inline void recursivefilelist(const std::string& path, const std::string& extension, std::vector<std::string>& list)
{
	std::vector<std::string> files = getfilelist(path, extension);
	for (size_t i = 0; i < files.size(); i++) {
		std::string f = files[i];
		list.push_back(f);
		size_t len = f.size();
		if (strcmp(&(f[len - 1]), ".") == 0)continue;
		if (strcmp(&(f[len - 2]), "..") == 0)continue;
		recursivefilelist(f, extension, list);
	}
}

inline std::vector<std::string> cDirectoryAccess::getfilelist(const std::string& searchpattern)
{
	std::vector<std::string> s = split(searchpattern, ';');
	std::vector<std::string> flist;
	for (size_t i = 0; i < s.size(); i++) {
		std::vector<std::string> l = cDirectoryAccess::getfilelist_single(s[i]);
		flist.insert(flist.end(), l.begin(), l.end());
	}
	return flist;
}

#if defined _WIN32	
///This version is for Microsoft Visual Studio    
inline std::vector<std::string> cDirectoryAccess::getfilelist_single(const std::string& searchpattern)
{
	std::vector<std::string> flist;
	sFilePathParts fpp = getfilepathparts(searchpattern);

	struct _finddata_t file;

	//Find First File	
	intptr_t hFile = _findfirst(searchpattern.c_str(), &file);
	if (hFile == -1) return flist;

	//Insert first file
	std::string fullpath = fpp.directory + file.name;
	flist.push_back(fullpath);

	//Find the rest of the files
	while (_findnext(hFile, &file) == 0L) {
		fullpath = fpp.directory + file.name;
		flist.push_back(fullpath);
	}
	_findclose(hFile);

	//Sort by name
	std::sort(flist.begin(), flist.end());
	return flist;
}
#else
///This version for Linux
inline std::vector<std::string> cDirectoryAccess::getfilelist_single(const std::string& searchpattern)
{

	std::vector<std::string> flist;
	struct dirent* directoryentry;
	DIR* dp;

	sFilePathParts fpp = getfilepathparts(std::string(searchpattern));
	if (fpp.directory.length() == 0)fpp.directory = ".";
	std::string directorypath = fpp.directory;

	//sp is the file search pattern (not directory)		
	std::string sp = fpp.prefix + fpp.extension;

	if ((dp = opendir(directorypath.c_str())) == NULL) {
		glog.warningmsg("getfilelist_single(): Could not open directory: %s\n", directorypath.c_str());
		return flist;
	}

	for (directoryentry = readdir(dp); directoryentry != NULL; directoryentry = readdir(dp)) {
		std::string name = std::string(directoryentry->d_name);
		if (wildcmp(sp, name) == true) {
			std::string fullpath = fpp.directory + name;
			flist.push_back(fullpath);
		}
	}
	closedir(dp);
	std::sort(flist.begin(), flist.end());
	return flist;
}
#endif

inline bool cDirectoryAccess::wildcmp(std::string& wildpattern, std::string& stringpattern)
{
	char* wild = &(wildpattern[0]);
	char* str = &(stringpattern[0]);

	while ((*str) && (*wild != '*')) {
		if ((*wild != *str) && (*wild != '?')) {
			return false;
		}
		wild++;
		str++;
	}

	char* cp = (char*)NULL;
	char* mp = (char*)NULL;
	while (*str) {
		if (*wild == '*') {
			if (!*++wild) {
				return true;
			}
			mp = wild;
			cp = str + 1;
		}
		else if ((*wild == *str) || (*wild == '?')) {
			wild++;
			str++;
		}
		else {
			wild = mp;
			str = cp++;
		}
	}

	while (*wild == '*') {
		wild++;
	}
	bool rval = (bool)!*wild;
	return rval;
}

inline bool filegetline(FILE* fp, std::string& str)
{
	size_t buflen = 8192;//buffer length
	str.clear();
	std::vector<char> buf(buflen + 1);
	while (fgets(&(buf[0]), (int)buflen, fp) != NULL) {
		str += std::string(&(buf[0]));
		size_t len = strlen(&(buf[0]));
		if (len < buflen - 1) {
			if (str[str.length() - 1] == 10) {
				//strip linefeed character
				str.resize(str.length() - 1);
			}
			return true;
		}
	}
	return false;
}

inline size_t countlines(const std::string filename)
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
}

inline size_t countlines1(const std::string filename)
{
	size_t n = 0;
	std::ifstream in(filename);
	std::string str;
	while (std::getline(in, str)) {
		++n;
	}
	return n;
}

#endif
