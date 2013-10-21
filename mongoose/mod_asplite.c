// TODO: Make it more straight forward by moving here
// what PrepareAndExeciteAspPage does.

void asp_write(void *user_data, const char *text)
{
    struct AspPageContext *context = (struct AspPageContext *)user_data;
    struct mg_connection *conn = (struct mg_connection *)context->user_data;
    mg_write(conn, text, strlen(text));
}


void asp_error(void *user_data, const char *text)
{
    struct AspPageContext *context = (struct AspPageContext *)user_data;
    struct mg_connection *conn = (struct mg_connection *)context->user_data;
    cry(conn, text);
}


static int handle_asp_request(struct mg_connection *conn, const char *path,
        struct file *filep)
{
    void *p = NULL;
    int error = 1;

    // We need both mg_stat to get file size, and mg_fopen to get fd
    if (!mg_stat(conn, path, filep) || !mg_fopen(conn, path, "r", filep)) {
        send_http_error(conn, 500, http_500_error, "File [%s] not found", path);
    }
    else {
        lua_State *L;
        struct AspPageContext context;

        L = luaL_newstate();

        context.write_func = asp_write;
        context.error_func = asp_error;
        context.log_func = asp_write;
        context.user_data = conn;
        context.request.query_string = conn->request_info.query_string;
        context.engine_config.root_path = conn->ctx->config[DOCUMENT_ROOT];
        context.engine_config.cache_path = conn->ctx->config[DOCUMENT_ROOT];
        context.engine_config.cache_lua = 0;
        context.engine_config.cache_luac = 1;

        ExeciteAspPage(L, path, &context);

        lua_close(L);
    }

    if (p != NULL)
        munmap(p, filep->size);

    mg_fclose(filep);

    return error;
}
