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

#ifndef ASPLITE_STR_H_C102C0E2_CB59_413C_9EBF_6CA3460389C8
#define ASPLITE_STR_H_C102C0E2_CB59_413C_9EBF_6CA3460389C8

#include <stdlib.h>

struct string {
    size_t capacity;
    size_t length;
    char str[1];
};

struct strlist;

struct string *string_new(size_t len);
struct string *string_new_sz(const char *sz, int len);
struct string *string_clone(const struct string *str);
void string_delete(struct string *str);

size_t string_length(const struct string *str);
char *string_get(struct string *str);

struct string *string_append(struct string *str, const char *sz, int len);
struct string *string_unquote(struct string *str, char quote);
struct strlist *string_split_sz(const char *sz, const char *delimiters,
        int include_empty);
struct strlist *string_split(struct string *str, const char *delimiters,
        int include_empty);

#endif // ASPLITE_STR_H_C102C0E2_CB59_413C_9EBF_6CA3460389C8
