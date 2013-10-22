struct string {
    size_t capacity;
    size_t length;
    char *s;
};

struct vector {
    size_t capacity;
    size_t length;
    char *arr;
};


static struct vector *vector_alloc(struct vector *v, size_t_ len)
{
    return realloc(v, sizeof(struct vector) + sizeof(char *) * len);
}

struct vector *vector_new(size_t reserve)
{
    struct vector *v = vector_alloc(NULL, reserve);
    v->capacity = reserve;
    v->length = 0;
    memset(v->arr, 0, sizeof(char *) * len);
}


struct vector *vector_push_back(struct vector *v, char *s)
{
    if (v->capacity < v->length + 1)
        v = vector_alloc(v, v->length + 1);
        
    v->arr[v->length++] = s;
    return v;
}


void vector_delete(struct vector *v)
{
    free(v);
}


static struct string *string_alloc(struct string *s, size_t len)
{
    return realloc(s, sizeof(struct string) + len + 1);
}

struct string *string_new(size_t len)
{
    struct string *s = string_alloc(NULL, len);
    s->capacity = len;
    s->length = len;
    memset(s->s, 0, len);
    return s;
}

struct string *string_newsz(const char *sz, int len)
{
    size_t slen = len < 0 ? strlen(sz) : len;
    struct string *s = string_new(slen);
    strncpy(s->s, sz, slen);
    s->s[len] = 0;
    return s;
}


struct string *string_dup(const string *s)
{
    return string_newsz(s->s, s->length);
}

size_t string_length(const struct string *s)
{
    return s->length;
}


struct string *string_append(struct string *s, const char *sz, size_t len)
{
    size_t slen = len < 0 ? strlen(sz) : len;
    if ((s->capacity - s->size) < len) {
        s->capacity = s->size + slen;
        s = string_alloc(s, s->capacity);
    }
    
    strncat(s->s, sz, slen);
    return s;
}


void string_delete(struct string *s)
{
    free(s);
}


struct vector *str_split_inplace(char *s, const char *sep)
{
    char *p = s;
    struct vector *v = vector_new(5);
    
    while (*p != '\0') {
        size_t span = strcspn(p, sep);
        if (span == 0) {
            p++;
            continue;
        }

        *(p + span) = '\0';
        vector_push_back(v, p);
        p = p + span + 1;
    }

    return v;
}



// Assumes that leading and trailing whitspace removed
// including trailing \r\n
int parseHeaderValue(const char *value)
{
    struct string *tmp = string_newsz(value);
    struct vector *v = str_split_inplace(tmp->, "; ");
    
    // start from 1 since Content-Type: form-data;name="name";filename="filename.txt"
    for (size_t i = 1; i < v->length; i++) {
        struct vector *nv = str_split_inplace(v->arr[i], "=");
        if (nv->length < 2)
            continue;   // malformed?
        str_unquote_inplace(nv->arr[1]);
    }

}
