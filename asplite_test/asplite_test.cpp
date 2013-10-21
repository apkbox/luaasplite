// asplite_test.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include "asplite/asplite.h"

#include <Windows.h>


void Write(void *user_data, const char *text)
{
    std::cout << text;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;

    const char *url = "http://www.google.ca/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&ved=0CDAQFjAA&url=http%3A%2F%2Fwww.dict.cc%2Fdeutsch-englisch%2Fdurchgesetzt.html&ei=8C5gUuDMOcSwyQGu54HABQ&usg=AFQjCNEZIehzLzZFY_3KnrQ3sEcAgS-4yQ&sig2=NOxVqLROu3a3C5QBf3OS0g";
    const char *protocol = "http";
    const char *hostname = "www.google.ca";
    const char *path = "/url";
    const char *query_string = "sa=t&rct=j&q=&esrc=s&source=web&cd=1&ved=0CDAQFjAA&url=http%3A%2F%2Fwww.dict.cc%2Fdeutsch-englisch%2Fdurchgesetzt.html&ei=8C5gUuDMOcSwyQGu54HABQ&usg=AFQjCNEZIehzLzZFY_3KnrQ3sEcAgS-4yQ&sig2=NOxVqLROu3a3C5QBf3OS0g";

    lua_State *L = luaL_newstate();

    char *file;
    char full_path[MAX_PATH];
    char root_dir[MAX_PATH];

    GetFullPathNameA(argv[1], sizeof(full_path), full_path, &file);

    strncpy(root_dir, full_path, file - full_path);
    root_dir[file - full_path - 1] = 0;

    struct AspPageContext context;
    context.write_func = Write;
    context.error_func = Write;
    context.log_func = Write;
    context.request.query_string = query_string;
    context.engine_config.cache_lua = 0;
    context.engine_config.cache_luac = true;
    context.engine_config.root_path = root_dir;
    context.engine_config.cache_path = "_";

    ExeciteAspPage(L, full_path, &context);

    lua_close(L);

	return 0;
}

