#ifndef __STROPS_H__
#define __STROPS_H__ 1

#include <stdlib.h>
// #include <string.h>

size_t stringlen(const char*);

char* strcopy(const char* src, char* dst) {
    while (1) {
        *(dst++) = *src;
        if (*src == 0) {
            return 0;
        }
        src ++;
    }
    return dst;
}

char* strmove(const char* str) {
    char* dst = (char*)malloc(stringlen(str)+1);
    if (dst == NULL) {
        return NULL;
    }
    strcopy(str, dst);
    return dst;
}

char* strjoin(const char* s1, const char* s2) {
    char* dst = (char*)malloc(stringlen(s1)+stringlen(s2)+1);
    if (dst == NULL) {
        return NULL;
    }
    strcopy(s1, dst);
    strcopy(s2, dst+stringlen(s1));
    return dst;
}

int stringcmp(const char* s1, const char* s2) {
    int c = 0;
    char c1;
    char c2;
    while ((c1 = s1[c]) != 0 && (c2 = s2[c]) != 0) {
        if (c1 != c2) {
            return 0;
        }
        c ++;
    }
    c2 = s2[c];
    if (c1 != c2) return 0;
    return 1;
}

int startswith(const char* str, const char* pre) {
    int c = 0;
    char c1;
    char c2;
    while ((c1 = str[c]) != 0 && (c2 = pre[c]) != 0) {
        if (c1 != c2) {
            return 0;
        }
        c ++;
    }
    if (c2 == 0) {
        return 1;
    }
    return 0;
}

size_t stringlen(const char* str) {
    size_t r = 0;
    while (*(str++)) {
        r ++;
    }
    return r;
}

#endif
