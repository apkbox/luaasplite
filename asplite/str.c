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

#include <stdlib.h>
#include <string.h>

#include "asplite/strlist.h"


static struct string *string_alloc(struct string *str, size_t len)
{
    return realloc(str, sizeof(struct string) + len + 1);
}


static struct string *string_ensure(struct string *str, size_t len)
{
    if (str->capacity < len) {
        str->capacity += len;
        str = string_alloc(str, str->capacity);
    }

    return str;
}


struct string *string_new(size_t len)
{
    struct string *str = string_alloc(NULL, len);
    str->capacity = len;
    str->length = len;
    memset(str->str, 0, len);
    return str;
}


struct string *string_new_sz(const char *sz, int len)
{
    size_t slen = len < 0 ? strlen(sz) : len;
    struct string *str = string_new(slen);
    strncpy(str->str, sz, slen);
    str->str[str->length] = 0;
    return str;
}


struct string *string_clone(const struct string *str)
{
    return string_new_sz(str->str, str->length);
}


void string_delete(struct string *str)
{
    free(str);
}


size_t string_length(const struct string *str)
{
    return str->length;
}


char *string_get(struct string *str)
{
    return str->str;
}


struct string *string_append(struct string *str, const char *sz, int len)
{
    size_t slen = len < 0 ? strlen(sz) : len;
    str = string_ensure(str, str->length + slen);
    strncat(&str->str[str->length], sz, slen);
    str->length += slen;
    return str;
}


struct string *string_unquote(struct string *str, char quote)
{
    char *p = str->str;

    if (str->str[0] != quote || str->length < 2)
        return str;

    if (str->str[0] == str->str[str->length - 1]) {
        memmove(str->str, &str->str[1], str->length - 2);
        str->length -= 2;
        str->str[str->length] = 0;
    }

    return str;
}


struct strlist *string_split_sz(const char *sz,
                                const char *delimiters,
                                int include_empty)
{
    const char *p = sz;
    struct strlist *list = strlist_new(5);

    while (*p != '\0') {
        size_t span = strcspn(p, delimiters);
        if (span == 0) {
            if (include_empty)
                strlist_append_sz(list, "", 0);
            p++;
            continue;
        }

        strlist_append_sz(list, p, span);
        p += span;
        if (*p == '\0')
            break;
        p++;
    }

    return list;
}


struct strlist *string_split(struct string *str,
                             const char *delimiters,
                             int include_empty)
{
    return string_split_sz(string_get(str), delimiters, include_empty);
}
