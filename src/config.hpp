#pragma once

#define USE_OFFICIAL // use official npm registry
// #define USE_LOCAL // use local couchdb
// #define USE_REDIS // use local redis

#if (defined(USE_OFFICIAL) && defined(USE_LOCAL)) ||                           \
    (defined(USE_LOCAL) && defined(USE_REDIS)) ||                              \
    (defined(USE_REDIS) && defined(USE_OFFICIAL))
#error
#endif

#if !defined(USE_OFFICIAL) && !defined(USE_LOCAL) && !defined(USE_REDIS)
#error
#endif

#define PREVENT_LOOP
#define QUEUE_LOOP_LIMIT 3000
#define LOAD_LOOP_LIMIT 1000
#define PLACE_LOOP_LIMIT 100

#define DETECT_QUEUE_LOOP

#define PARSE_CONFIG_OS "linux"
#define PARSE_CONFIG_CPU "x64"
#define PARSE_CONFIG_LIBC "glibc"
