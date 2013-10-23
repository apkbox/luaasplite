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

#include "asplite/strlist.h"

#include <string.h>

#include "asplite/str.h"


static struct strlist *strlist_alloc(struct strlist *list, size_t len)
{
    return realloc(list, sizeof(struct strlist) + sizeof(struct string *) * len);
}


static struct strlist *strlist_ensure(struct strlist *list, size_t len)
{
    if (list->capacity < len) {
        list->capacity += len;
        list = strlist_alloc(list, list->capacity);
    }

    return list;
}


struct strlist *strlist_new(size_t reserve)
{
    struct strlist *list = strlist_alloc(NULL, reserve);
    list->capacity = reserve;
    list->size = 0;
    memset(list->items, 0, sizeof(struct string *) * list->capacity);
    return list;
}


void strlist_delete(struct strlist *list)
{
    size_t i;

    for (i = 0; i < list->size; i++) {
        string_delete(list->items[i]);
    }

    free(list);
}


size_t strlist_size(const struct strlist *list)
{
    return list->size;
}


struct string *strlist_get(struct strlist *list, size_t index)
{
    if (index >= list->size)
        return NULL;

    return list->items[index];
}


struct string *strlist_clone(struct strlist *list, size_t index)
{
    if (index >= list->size)
        return NULL;

    return string_clone(list->items[index]);
}


struct strlist *strlist_append_sz(struct strlist *list, const char *sz, int len)
{
    list = strlist_ensure(list, list->size + 1);
    list->items[list->size++] = string_new_sz(sz, len);
    return list;
}


struct strlist *strlist_append(struct strlist *list, struct string *str)
{
    list = strlist_ensure(list, list->size + 1);
    list->items[list->size++] = string_clone(str);
    return list;
}


struct strlist *strlist_append_nocopy(struct strlist *list, struct string *str)
{
    list = strlist_ensure(list, list->size + 1);
    list->items[list->size++] = str;
    return list;
}


struct string *strlist_remove(struct strlist *list, size_t index)
{
    struct string *s;

    if (index >= list->size)
        return NULL;

    s = list->items[index];
    memmove(&list->items[index], &list->items[index + 1],
            sizeof(struct string *) * (list->size - index - 1));

    list->size--;

    return s;
}


size_t strlist_erase(struct strlist *list, size_t index, size_t len)
{
    int i;

    if (index >= list->size)
        return 0;

    len = len > (list->size - index) ? list->size - index : len;

    for (i = index; i < (index + len); i++) {
        string_delete(list->items[index + i]);
    }

    memmove(&list->items[index], &list->items[index + len],
            sizeof(struct string *) * (list->size - index - len));

    list->size -= len;

    return len;
}
