// The MIT License (MIT)
//
// Copyright (c) 2013 Alex Kozlov
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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


struct FormItem {
    char name[256];
    char content_type[256];
    char file_name[MAX_PATH];
    char value[1024];
    int is_file;
};

#if 0

int ProcessMultipartFormData(struct mg_connection *conn,
        char *directory, struct FormItem *items)
{
    const char *content_type_header, *boundary_start;
    char buf[MG_BUF_LEN], path[PATH_MAX], fname[1024], boundary[100], *s;
    FILE *fp;
    int bl, n, i, j, headers_len, boundary_len, eof,
        len = 0, num_uploaded_files = 0;
    int item = 0;

    // Request looks like this:
    //
    // POST /upload HTTP/1.1
    // Host: 127.0.0.1:8080
    // Content-Length: 244894
    // Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryRVr
    //
    // ------WebKitFormBoundaryRVr
    // Content-Disposition: form-data; name="file"; filename="accum.png"
    // Content-Type: image/png
    //
    //  <89>PNG
    //  <PNG DATA>
    // ------WebKitFormBoundaryRVr

    // Extract boundary string from the Content-Type header
    if ((content_type_header = mg_get_header(conn, "Content-Type")) == NULL ||
            (boundary_start = mg_strcasestr(content_type_header,
            "boundary=")) == NULL ||
            (sscanf(boundary_start, "boundary=\"%99[^\"]\"", boundary) == 0 &&
            sscanf(boundary_start, "boundary=%99s", boundary) == 0) ||
            boundary[0] == '\0') {
        return num_uploaded_files;
    }

    // Create temporary directory
    mkdir(directory);

    boundary_len = strlen(boundary);
    bl = boundary_len + 4;  // \r\n--<boundary>
    for (;;) {
        // Pull in headers
        assert(len >= 0 && len <= (int) sizeof(buf));
        while ((n = mg_read(conn, buf + len, sizeof(buf)-len)) > 0) {
            len += n;
        }
        if ((headers_len = get_request_len(buf, len)) <= 0) {
            break;
        }

        // Fetch file name.
        fname[0] = '\0';
        for (i = j = 0; i < headers_len; i++) {
            if (buf[i] == '\r' && buf[i + 1] == '\n') {
                buf[i] = buf[i + 1] = '\0';
                if (strnicmp(&buf[j], "Content-Disposition:", 20) == 0) {
                    j += 20;

                    p = strstr(&buf[j], "name=");
                    if (p != NULL) {
                        p += 5

                        // Unnamed entry - ignore.
                    }

                }
                // TODO(lsm): don't expect filename to be the 3rd field,
                // parse the header properly instead.
                sscanf(&buf[j], "Content-Disposition: %*s %*s filename=\"%1023[^\"]",
                    fname);
                j = i + 2;
            }
        }

        // Give up if the headers are not what we expect
        if (fname[0] == '\0') {
            break;
        }

        // Move data to the beginning of the buffer
        assert(len >= headers_len);
        memmove(buf, &buf[headers_len], len - headers_len);
        len -= headers_len;

        // We open the file with exclusive lock held. This guarantee us
        // there is no other thread can save into the same file simultaneously.
        fp = NULL;
        // Construct destination file name. Do not allow paths to have slashes.
        if ((s = strrchr(fname, '/')) == NULL &&
            (s = strrchr(fname, '\\')) == NULL) {
            s = fname;
        }

        // Open file in binary mode. TODO: set an exclusive lock.
        snprintf(path, sizeof(path), "%s/%s", tmpdir, s);
        if ((fp = fopen(path, "wb")) == NULL) {
            break;
        }

        // Read POST data, write into file until boundary is found.
        eof = n = 0;
        do {
            len += n;
            for (i = 0; i < len - bl; i++) {
                if (!memcmp(&buf[i], "\r\n--", 4) &&
                    !memcmp(&buf[i + 4], boundary, boundary_len)) {
                    // Found boundary, that's the end of file data.
                    fwrite(buf, 1, i, fp);
                    eof = 1;
                    memmove(buf, &buf[i + bl], len - (i + bl));
                    len -= i + bl;
                    break;
                }
            }
            if (!eof && len > bl) {
                fwrite(buf, 1, len - bl, fp);
                memmove(buf, &buf[len - bl], bl);
                len = bl;
            }
        } while (!eof && (n = mg_read(conn, buf + len, sizeof(buf)-len)) > 0);
        fclose(fp);
        if (eof) {
            file_names[num_uploaded_files++] = strdup(path);
        }
    }

    return num_uploaded_files;
}

#endif

static int handle_asp_request(struct mg_connection *conn, const char *path,
        struct file *filep)
{
    int i;
    void *p = NULL;
    int error = 1;

    if (strcmp(conn->request_info.request_method, "POST") == 0) {
        const char *content_type;
        char boundary[71];   // 70 according to RFC1341
        char buf[1024];
        content_type = mg_get_header(conn, "Content-Type");
        if (content_type != NULL) {
            if (strncmp(content_type, "multipart/form-data", 19) == 0) {
                // TODO: Process upload
            }
            else if (strncmp(content_type, 
                    "application/x-www-form-urlencoded", 33) == 0) {
                // TODO: Process form
            }
            else {
                send_http_error(conn, 415, "Unsupported Media Type",
                        "%s", content_type);
            }
        }
        while (mg_read(conn, buf, 1024) > 0)
            ;
    }

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
        context.request.request_method = conn->request_info.request_method;
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
