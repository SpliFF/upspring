#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include <list>

#include <FileSearch.h>


// NOTE: no io.h on Linux, so do conditional compilation of FindFiles()
// (Linux code copied from rts/System/Platform/Linux/UnixFileSystemHandler.cpp
// with minor modifications)

#ifdef WIN32
#include <io.h>

std::list<std::string>* FindFiles(const std::string& searchPattern, bool recursive, const std::string& path) {
    struct  _finddata_t  c_file;
    intptr_t  hFile;

	std::string searchPath;

	if (!path.empty())
		searchPath = path + "\\";

	std::list<std::string>* r = new std::list<std::string>();

	// search for directories to scan
	if (recursive)
	{
		hFile = _findfirst( (searchPath + "*").c_str(),  &c_file);
		while (hFile != -1)
		{
			if (strcmp(c_file.name, "..") &&  strcmp(c_file.name, "."))
			{
				if (c_file.attrib & _A_SUBDIR)
				{
					std::string ns = path + c_file.name;
					ns += "\\";

					std::list<std::string>* subdir = FindFiles(searchPattern, true, ns);
					r->insert(r->end(), subdir->begin(), subdir->end());
					delete subdir;
				}
			}

			if (_findnext(hFile, &c_file) != 0) 
			{
				_findclose(hFile);
				hFile = -1;
			}
		}
	}

	// search files that match the pattern
	hFile = _findfirst( (searchPath + searchPattern).c_str(),  &c_file);
	while (hFile != -1)
	{
		if (!(c_file.attrib & _A_SUBDIR))
			r->push_back(searchPath + c_file.name);

		if (_findnext(hFile, &c_file) != 0) {
			_findclose(hFile);
			hFile = -1;
		}
    }

	return r;
}



#else
#include <boost/regex.hpp>
#include <dirent.h>
#include <sys/stat.h>

/**
 * @brief quote macro
 * @param c Character to test
 * @param str string currently being built
 *
 * Given a string str that we're assembling,
 * and an upcoming character c, will append
 * an extra '\\' to quote the character if necessary.
 */
#define QUOTE(c, str)						\
	do {									\
		if (!(isalnum(c) || (c) == '_'))	\
			str += '\\';					\
		str += c;							\
} while (0)


/**
 * @brief glob to regex
 * @param glob string containing glob
 * @return string containing regex
 *
 * Converts a glob expression to a regex
 */
std::string glob_to_regex(const std::string& glob) {
	std::string regex;
	regex.reserve(glob.size() << 1);
	int braces = 0;

	for (std::string::const_iterator i = glob.begin(); i != glob.end(); ++i) {
		char c = *i;

		switch (c) {
			case '*':
				regex += ".*";
				break;
			case '?':
				regex += '.';
				break;
			case '{':
				braces++;
				regex += '(';
				break;
			case '}':
				regex += ')';
				braces--;
				break;
			case ',':
				if (braces)
					regex += '|';
				else
					QUOTE(c, regex);
				break;
			case '\\':
				++i;
				QUOTE(*i, regex);
				break;
			default:
				QUOTE(c, regex);
				break;
		}
	}

	return regex;
}


void FindFiles(std::list<std::string>* matches, const std::string& dir, const boost::regex& regexpattern, bool recursive) {
	DIR* dp;
	struct dirent* ep;

	if (!(dp = opendir(dir.c_str())))
		return;

	while ((ep = readdir(dp))) {
		// exclude hidden files
		if (ep->d_name[0] != '.') {
			// need to stat because d_type is DT_UNKNOWN on Linux
			struct stat info;

			if (stat((dir + ep->d_name).c_str(), &info) == 0) {
				// is entry a file? (just treat sockets / pipes / fifos / character&block devices as files)
				if (!S_ISDIR(info.st_mode)) {
					if (boost::regex_match(ep->d_name, regexpattern))
						matches -> push_back(dir + ep->d_name);
				}

				// entry is a directory, should we descend into it?
				else if (recursive) {
					if (boost::regex_match(ep->d_name, regexpattern))
						matches -> push_back(dir + ep->d_name);

					FindFiles(matches, dir + ep->d_name + '/', regexpattern, recursive);
				}
			}
		}
	}

	closedir(dp);
}


/**
 * @brief wrapper for recursive FindFiles()
 * @param path path in which to start looking
 * @param searchPattern glob pattern to search for
 * @param recursive whether or not to recursively search
 * @return list of std::strings containing absolute paths to the files
 *
 * Will search for a file given a particular glob pattern.
 * Starts from dirpath, descending down if recurse is true.
 */

std::list<std::string>* FindFiles(const std::string& searchPattern, bool recursive, const std::string& path) {
	std::list<std::string>* matches = new std::list<std::string>();

	// dir must end with slash so concatenation doesn't produce messed-up strings
	assert(path.size() > 0 && path[path.size() - 1] == '/');

	// turn glob (eg. "*.lua") into proper regex (eg. ".*\.lua")
	boost::regex regexpattern(glob_to_regex(searchPattern));
	FindFiles(matches, path, regexpattern, recursive);

	return matches;
}

#endif
