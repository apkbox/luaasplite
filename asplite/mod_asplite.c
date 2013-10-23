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

#include "asplite/str.h"
#include "asplite/strlist.h"


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


static char *FindString(char *buf, size_t len,
        const char *str, size_t str_len)
{
    char *p;

    if (len < str_len)
        return NULL;

    for (p = buf; p < (buf + (len - str_len)); p++) {
        if (memcmp(p, str, str_len) == 0) {
            return p;
        }
    }

    return NULL;
}


static char *FindContent(char *buf, size_t len)
{
    return FindString(buf, len, "\r\n\r\n", 4);
}


static int ExtractContentDispositionParameters(const char *data)
{
    return 0;
}


static int ExtractContentType(const char *data)
{
    return 0;
}


static int ProcessPartHeaders(struct strlist *headers)
{
    int i;

    // Go through the headers and find Content-Disposition
    for (i = 0; i < strlist_size(headers); i++) {
        struct string *header = strlist_get(headers, i);
        char *s = string_get(header);
        if (strncmp(s, "Content-Disposition:", 20) == 0) {
            s += 20;
            while (*s == ' ')
                s++;

            ExtractContentDispositionParameters(s);
        }
        else if (strncmp(s, "Content-Type:", 12) == 0) {
            s += 12;
            while (*s == ' ')
                s++;

            ExtractContentType(s);
        }
    }

    return 0;
}


static int ProcessMultipartFormData(struct mg_connection *conn,
                             const char *boundary,
                             struct string **directory,
                             struct FormItem *form_items)
{
    static const kDefaultBufferSize = 0x4000;
    size_t buf_size;
    char *buf = NULL;
    int len = 0;
    struct FormItem *current_item = NULL;
    int form_item_index = 0;
    struct string *bd = string_new_sz("\r\n--", 4);
    bd = string_append(bd, boundary, -1);

    buf_size = string_length(bd) < kDefaultBufferSize ?
            kDefaultBufferSize : string_length(bd) + kDefaultBufferSize;

    buf = malloc(buf_size);

    // The first CRLF was consumed during request parsing,
    // so we stuff it here artificially.
    buf[0] = '\r';
    buf[1] = '\n';
    len = 2;

    while (1) {
        char *part;
        char *content;
        int read;

        read = mg_read(conn, buf + len, buf_size - len);
        if (read <= 0 && len == 0)
            break;

        len += read;

        part = FindString(buf, len, string_get(bd), string_length(bd));
        if (part != NULL) {
            if (current_item != NULL && (part - buf) > 0) {
                // TODO: current_item->Write(buf, part - buf);
                len -= part - buf;
                memmove(buf, part, len);
                continue;   // Fill the buffer.
            }

            part += string_length(bd);

            // TODO: Check that we have enough data in the buffer
            if (part[0] == '-' || part[1] == '-')
                break;   // Closing boundary marker

            if (part[0] != '\r' || part[1] != '\n')
                break;   // Unexpected.

            part += 2;   // skip CRLF

            content = FindContent(part, len - (part - buf));
            if (content == NULL)   // Too many headers
                break;

            current_item = &form_items[form_item_index++];

            if (content > part) {  // Are there headers ?
                struct strlist *part_headers;
                // terminate headers part
                *content = 0;
                part_headers = string_split_sz(part, "\r\n", 0);
                ProcessPartHeaders(part_headers);
                strlist_delete(part_headers);
            }

            content += 4;  // skip CRLF to content

            len -= content - buf;
            memmove(buf, content, len);
        }
        else {
            char *possible_boundary = FindString(buf, len, "\r\n", 2);
            if (possible_boundary == NULL) {
                // TODO: current_item->Write(buf, len);
                len = 0;
            }
            else {
                // TODO: current_item->Write(buf, possible_boundary - buf);
                len -= possible_boundary - buf;
            }
        }
    }

    string_delete(bd);
    free(buf);

    return 0;
}


// Assumes that leading and trailing whitspace were removed,
// including trailing CRLF
int ParseContentTypeHeader(const char *value,
                           struct string **content_type,
                           struct string **boundary)
{
    int i, found = 0;
    struct strlist *v = string_split_sz(value, "; ", 0);

    *content_type = NULL;
    *boundary = NULL;

    if (strlist_size(v) == 0)
        return 0;

    *content_type = strlist_remove(v, 0);

    for (i = 0; i < strlist_size(v); i++) {
        struct strlist *name_value_pair = string_split(strlist_get(v, i), "=", 0);

        if (strlist_size(name_value_pair) == 2) {
            if (strcmp(string_get(strlist_get(name_value_pair, 0)),
                    "boundary") == 0) {
                *boundary = string_unquote(
                        strlist_remove(name_value_pair, 1), '"');
                strlist_delete(name_value_pair);
                found = 1;
                break;
            }
        }

        strlist_delete(name_value_pair);
    }

    if (!found) {
        string_delete(*content_type);
        *content_type = NULL;
    }

    strlist_delete(v);
    return found;
}


static int handle_asp_request(struct mg_connection *conn, const char *path,
        struct file *filep)
{
    int i;
    void *p = NULL;
    int error = 1;

    if (strcmp(conn->request_info.request_method, "POST") == 0) {
        const char *content_type_value;
        struct string *content_type;
        struct string *boundary;
        int content_read = 0;
        char buf[1024];

        content_type_value = mg_get_header(conn, "Content-Type");
        if (content_type_value != NULL) {
            if (ParseContentTypeHeader(content_type_value, &content_type,
                    &boundary)) {
                if (strcmp(string_get(content_type),
                        "multipart/form-data") == 0) {
                    struct string *directory;
                    struct FormItem form_data[64];

                    ProcessMultipartFormData(conn, string_get(boundary),
                            &directory, form_data);
                    // TODO: Process upload
                }
                else if (strcmp(string_get(content_type),
                        "application/x-www-form-urlencoded") == 0) {
                    // TODO: Process form
                }
                else {
                    send_http_error(conn, 415, "Unsupported Media Type",
                        "%s", content_type_value);
                }

                string_delete(content_type);
                string_delete(boundary);
            }
        }

        if (!content_read) {
            while (mg_read(conn, buf, 1024) > 0)
                ;
        }
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
