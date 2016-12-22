/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _csvfile_H
#define _csvfile_H

#include "general_utils.h"
#include "file_utils.h"

class cCSVFile{

	public:
		std::vector<std::string> header;
		std::vector<std::vector<std::string>> records;

		cCSVFile(){ }

		cCSVFile(const std::string csvfile){

			FILE* fp = fileopen(csvfile, "r");

			std::string str;
			std::vector<std::string> tokens;

			//Read header line
			filegetline(fp, str);
			split(str, ',', tokens);
			header = tokens;
			size_t nfields = header.size();			

			//Read remaining lines
			size_t k = 1;
			while (filegetline(fp, str)){
				k++;
				tokens.resize(0);
				split(str, ',', tokens);
				if (tokens.size() == nfields - 1){
					tokens.push_back("");
				}

				if (tokens.size() != nfields){
					std::printf("Error: On line %lu of file %s\n", k, csvfile.c_str());
					std::printf("Error: The number of header items (%lu) does not match the number of data items (%lu)\n", nfields, tokens.size());
				}

				for (size_t i = 0; i < tokens.size(); i++){
					tokens[i] = trim(tokens[i]);
					tokens[i] = stripquotes(tokens[i]);
				}
				records.push_back(tokens);
			}
			fclose(fp);
		}

		bool addfield(const std::string fname){
			header.push_back(fname);
			for (size_t i = 0; i < records.size(); i++){
				const std::string empty;
				records[i].push_back(empty);
			}
			return true;
		}

		bool setfield(const std::string fname, const std::string value, const size_t recindex){
			int k = findkeyindex(fname);
			if (k < 0)return false;
			records[recindex][(size_t)k] = value;
			return true;
		}

		bool setfield(const std::string fname, const std::string value){
			int k = findkeyindex(fname);
			if (k < 0)return false;
			if (records.size() == 0){
				records.resize(1);
				records[0].resize(header.size());
			}

			for (size_t i = 0; i < records.size(); i++){
				records[i][(size_t)k] = value;
			}
			return true;
		}

		int findkeyindex(const std::string& fname){
			for (size_t i = 0; i < header.size(); i++){
				if (header[i] == fname){
					return (int)i;
				}
			}
			return -1;
		}

		std::vector<size_t> findmatchingrecords(const size_t keyindex, const int value){
			std::vector<size_t> indices;
			for (size_t i = 0; i < records.size(); i++){
				int n = atoi(records[i][keyindex].c_str());
				if (n == value){
					indices.push_back(i);
				}
			}
			return indices;
		}

		std::vector<size_t> findmatchingrecords(const std::string key, const int value)	{
			int keyindex = findkeyindex(key);
			if (keyindex < 0){
				std::vector<size_t> indices;
				return indices;
			}
			return findmatchingrecords((size_t)keyindex, value);
		}

		std::vector<size_t> findmatchingrecords(const size_t keyindex, const std::string value){
			std::vector<size_t> indices;
			for (size_t i = 0; i < records.size(); i++){
				if (!strcasecmp(records[i][keyindex],value)){
					indices.push_back(i);
				}
			}
			return indices;
		}

		std::vector<size_t> findmatchingrecords(const std::string key, const std::string value)	{
			int keyindex = findkeyindex(key);
			if (keyindex < 0){
				std::vector<size_t> indices;
				return indices;
			}
			return findmatchingrecords((size_t)keyindex, value);
		}

		void printrecord(const size_t n){
			for (size_t mi = 0; mi < header.size(); mi++){
				std::printf("%s: %s\n", header[mi].c_str(), records[n][mi].c_str());
			}
			std::printf("\n");
		}
};

#endif

