#include "cpputil.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <random>

void out_str(const char* s)
{
    for (; *s; s++) putchar(*s);
}

void out_vfmt(const char* c, va_list ap)
{
    vprintf(c, ap);
}

void out_fmt(const char* c, ...)
{
    va_list ap;
    va_start(ap, c);
    out_vfmt(c, ap);
    va_end(ap);
}

void out_line(const char* s)
{
    out_str(s);
    putchar('\n'); fflush(stdout);
}

void out_line_fmt(const char* c, ...)
{
    va_list ap;
    va_start(ap, c);
    out_vfmt(c, ap);
    va_end(ap);
    putchar('\n'); fflush(stdout);
}

void out_char(char c)
{
    putchar(c);
}

char* int_to_str(int64_t i, char* str)
{
    #define push(c) *(str++) = c
    #define end *str = 0; return str
    if (!i) { push('0'); end; }
    char c[20]; char *s = &c[0]; *s = 0;
    if (i < 0) { i = -i; push('-'); }
    while (i)
    {
        *(++s) = '0' + (i % 10);
        i /= 10;
    }
    for (; *s; s--) push(*s);
    end;
    #undef push
    #undef end
}

void out_int(int64_t i)
{
    char c[20];
    int_to_str(i, c);
    out_str(c);
}

void in_line(char* c, int max_len)
{
    fgets(c, max_len, stdin);
}

#ifdef __WIN32
    #include <windows.h>
    #define init_seed GetTickCount()
#else
    #include <chrono>
    #define init_seed std::chrono::system_clock::now().time_since_epoch().count()
#endif // __WIN32

#undef init_seed
#define init_seed 21310510732

std::mt19937_64 rnd;

void randomize64() { rnd = std::mt19937_64(init_seed); }
int64_t random64() { return rnd(); }
int64_t random64(int64_t r) { return (r <= 0) ? 0 : rnd() % r; }
int64_t random64(int64_t l, int64_t r) { return rnd() % (r - l + 1) + l; }

#ifdef DEBUG
    void fatal_error(const char* s)
    {
        out_line_fmt("FATAL ERROR: \"%s\"\n", s);
        exit(256);
    }
#endif
