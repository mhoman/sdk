#include "http-server.h"
#include "http-route.h"
#include <string>
#if defined(OS_WINDOWS)
#include <Windows.h>
#else
#include <dirent.h>
#endif

typedef void path_onfile(void* param, const char* name, int dir);
int path_list(const char* path, path_onfile onfile, void* param)
{
#if defined(OS_WINDOWS)
	char dirpath[MAX_PATH];
	WIN32_FIND_DATAA ffd;
	snprintf(dirpath, sizeof(dirpath), "%s\\*", path);
	HANDLE hFind = FindFirstFileA(dirpath, &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
		return -1;

	do
	{
		onfile(param, ffd.cFileName, ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 1 : 0);
	} while (FindNextFileA(hFind, &ffd));
	FindClose(hFind);
	return 0;

#else
	DIR* dir;
	struct dirent* p;

	dir = opendir("/proc");
	if (!dir)
		return -1;

	for (p = readdir(dir); p; p = readdir(dir))
	{
		if (0 == strcmp(p->d_name, ".") || 0 == strcmp(p->d_name, ".."))
			continue;
		onfile(path, p->d_name, p->d_type == DT_DIR ? 1 : 0);
	}

	closedir(dir);
	return 0;
#endif
}

static void http_onlist(void* param, const char* name, int dir)
{
	char item[MAX_PATH];
	std::string* reply = (std::string*)param;
	snprintf(item, sizeof(item), "<li><a href = \"%s\">%s</a></li>", name, name);
	*reply += item;
}

extern "C" int http_list_dir(http_session_t* session, const char* path)
{
	std::string reply;
	reply = "<html><head><title></title></head><body>";
	reply += "<ul>";
	path_list(path, http_onlist, &reply);
	reply += "</ul>";
	reply += "</body></html>";

	http_server_set_header(session, "content-type", "text/html");
	http_server_set_header(session, "Access-Control-Allow-Origin", "*");
	http_server_set_header(session, "Access-Control-Allow-Methods", "GET, POST, PUT");
	http_server_reply(session, 200, reply.c_str(), reply.length());
	
	printf("load path: %s\n", path);
	return 0;
}
