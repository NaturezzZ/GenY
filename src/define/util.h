//
// Created by Naiqian on 2021/4/24.
//

#ifndef GENY_UTIL_H
#define GENY_UTIL_H
#include <stdarg.h>
int dprintf(int fd, const char *format, ...){
    va_list arg;
    int done;
    va_start (arg, format);
    done = __vdprintf_internal (d, format, arg, 0);
    va_end (arg);
    return done;
}
#endif //GENY_UTIL_H
