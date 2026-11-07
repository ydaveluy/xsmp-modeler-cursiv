// `errno` is a thread-local of the server, and a library cannot reach one. So a
// WASI library reads it through a function of the server.
#pragma once
#include <errno.h>
#ifdef __cplusplus
extern "C"
#endif
    int*
    xsmp_errno_location(void);
#undef errno
#define errno (*xsmp_errno_location())
