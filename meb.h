/* ------------ meb.h ----------------------------\
* A simple logging library for C.				  |
* Author: Marc Maravall							  |
* License: MIT License							  |
* Repo: https://www.github.com/marcmaravall/meb   |
* Thanks for using my library!					  |
* ------------------------------------------------|
* TODO:											  |
* - Add thread safety for logging.				  |
\________________________________________________*/

#ifndef MEB_H
#define MEB_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef __USE_POSIX199309
#define __USE_POSIX199309
#endif

#include <time.h>

#ifndef MEB_NO_LOG
#define MEB_INIT(file_path)     meb_init(&meb, file_path)

#define MEB_PROF_START()        meb_prof_start(&meb)
#define MEB_PROF_END()          meb_prof_end(&meb)
#define MEB_PROF_MODE(mode)     meb_prof_mode(&meb, mode)

#define MEB_ASSERT(expr)        assert(expr)
#define MEB_STATIC_ASSERT(a, b) static_assert(a, b)

#define MEB_LOG(m)              meb_log(&meb, m)

#define MEB_LOGF(fmt, ...)                              \
    do {                                                \
        char buf[MEB_BUFF_SIZE];                        \
        snprintf(buf, MEB_BUFF_SIZE, fmt, __VA_ARGS__); \
        meb_log(&meb, buf);                             \
    } while(0)

#define MEB_LOG_INFO(m)         meb_logl(&meb, MEB_INFO,    m)
#define MEB_LOG_WARNING(m)      meb_logl(&meb, MEB_WARNING, m)
#define MEB_LOG_ERROR(m)        meb_logl(&meb, MEB_ERROR,   m)

#define MEB_LOG_INFOF(fmt, ...)                                     \
    do {                                                            \
        char buf[MEB_BUFF_SIZE];                                    \
        snprintf(buf, MEB_BUFF_SIZE, fmt, __VA_ARGS__);             \
        meb_logl(&meb, MEB_INFO, buf);                              \
    } while(0)

#define MEB_LOG_WARNINGF(fmt, ...)                                  \
    do {                                                            \
        char buf[MEB_BUFF_SIZE];                                    \
        snprintf(buf, MEB_BUFF_SIZE, fmt, __VA_ARGS__);             \
        meb_logl(&meb, MEB_WARNING, buf);                           \
    } while(0)

#define MEB_LOG_ERRORF(fmt, ...)                                    \
    do {                                                            \
        char buf[MEB_BUFF_SIZE];                                    \
        snprintf(buf, MEB_BUFF_SIZE, fmt, __VA_ARGS__);             \
        meb_logl(&meb, MEB_ERROR, buf);                             \
    } while(0)

#define MEB_CLOSE()             meb_close(&meb)

#else // MEB_NO_LOG

#define MEB_INIT(file_path)         ((void)0)
#define MEB_PROF_START()            ((void)0)
#define MEB_PROF_END()              ((void)0)
#define MEB_PROF_MODE(mode)         ((void)0)
#define MEB_ASSERT(expr)            ((void)0)
#define MEB_STATIC_ASSERT(a, b)     static_assert(a, b)
#define MEB_LOG(m)                  ((void)0)
#define MEB_LOGF(fmt, ...)          ((void)0)
#define MEB_LOG_INFO(m)             ((void)0)
#define MEB_LOG_WARNING(m)          ((void)0)
#define MEB_LOG_ERROR(m)            ((void)0)
#define MEB_LOG_INFOF(fmt, ...)     ((void)0)
#define MEB_LOG_WARNINGF(fmt, ...)  ((void)0)
#define MEB_LOG_ERRORF(fmt, ...)    ((void)0)
#define MEB_CLOSE()                 ((void)0)

#endif // MEB_NO_LOG

#define MEB_BUFF_SIZE 256

typedef enum {
    MEB_SECONDS,
    MEB_MILLISECONDS,
    MEB_MICROSECONDS,
    MEB_NANOSECONDS,
} MebTimeMode;

typedef enum {
    MEB_NONE,
    MEB_INFO,
    MEB_WARNING,
    MEB_ERROR,
} MebLogLevel;

typedef struct {
    FILE* debug;
    double      start;
    double      end;
    MebTimeMode time_mode;
    MebLogLevel log_level;
} MebContext;

extern MebContext meb;

double meb_get_time(const MebTimeMode mode);
char   meb_time_unit(const MebTimeMode mode);
char*  meb_log_level_str(const MebLogLevel level);

void    meb_init(MebContext* ctx, const char* file);
void    meb_log(MebContext* ctx, const char* message);
void    meb_logl(MebContext* ctx, MebLogLevel level, const char* message);
void    meb_prof_start(MebContext* ctx);
void    meb_prof_end(MebContext* ctx);
void    meb_prof_mode(MebContext* ctx, MebTimeMode mode);
void    meb_log_level(MebContext* ctx, MebLogLevel level);
void    meb_close(MebContext* ctx);

#endif // MEB_H

#ifdef MEB_IMPLEMENTATION

MebContext meb;

void meb_init(MebContext* ctx, const char* file) {
#ifndef MEB_NO_LOG
    MEB_ASSERT(ctx);

    ctx->time_mode = MEB_MICROSECONDS;
    ctx->log_level = MEB_INFO;
    ctx->debug = NULL;

#if defined(MEB_LOG_TO_FILE)
    ctx->debug = fopen(file, "w");
    if (!ctx->debug) {
        fprintf(stderr, "meb: failed to open log file: %s\n", file);
    }
#elif defined(MEB_LOG_TO_STDOUT)
    ctx->debug = stdout;
    (void)file; 
#else
#warning "meb: no logging output defined (MEB_LOG_TO_FILE or MEB_LOG_TO_STDOUT)"
#endif

#else
    (void)ctx; (void)file;
#endif // MEB_NO_LOG
}

void meb_log(MebContext* ctx, const char* message) {
    MEB_ASSERT(ctx);
#ifndef MEB_NO_LOG
    if (!ctx->debug) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", t);

    fprintf(ctx->debug, "[%s] %s: %s\n",
        ts,
        meb_log_level_str(ctx->log_level),
        message);
#else
    (void)ctx; (void)message;
#endif
}

void meb_logl(MebContext* ctx, MebLogLevel level, const char* message) {
    MebLogLevel prev = ctx->log_level;
    meb_log_level(ctx, level);
    meb_log(ctx, message);
    meb_log_level(ctx, prev);
}

void meb_close(MebContext* ctx) {
#ifndef MEB_NO_LOG
    if (ctx->debug && ctx->debug != stdout) {
        fclose(ctx->debug);
        ctx->debug = NULL;
    }
#else
    (void)ctx;
#endif
}

void meb_prof_start(MebContext* ctx) {
    ctx->start = meb_get_time(ctx->time_mode);
    meb_logl(ctx, MEB_INFO, "[MEB_PROFILING] Profiling started.");
}

void meb_prof_end(MebContext* ctx) {
    ctx->end = meb_get_time(ctx->time_mode);
    double elapsed = ctx->end - ctx->start;

    char buffer[MEB_BUFF_SIZE];
    snprintf(buffer, MEB_BUFF_SIZE,
        "[MEB_PROFILING] Finished. Elapsed: %.6f%c",
        elapsed, meb_time_unit(ctx->time_mode));

    meb_logl(ctx, MEB_INFO, buffer);
}

double meb_get_time(const MebTimeMode mode) {
    double time;

#ifdef _WIN32
    static LARGE_INTEGER frequency;
    static int initialized = 0;
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    time = (double)counter.QuadPart / frequency.QuadPart;

#elif defined(__linux__) || defined(__APPLE__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    time = (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;

#else
#warning "meb: high-resolution timing not implemented for this platform."
    time = 0.0;
#endif

    switch (mode) {
    case MEB_SECONDS:      return time;
    case MEB_MILLISECONDS: return time * 1e3;
    case MEB_MICROSECONDS: return time * 1e6;
    case MEB_NANOSECONDS:  return time * 1e9;
    }
    return time;
}

char meb_time_unit(const MebTimeMode mode) {
    switch (mode) {
    case MEB_SECONDS:      return 's';
    case MEB_MILLISECONDS: return 'm';
    case MEB_MICROSECONDS: return 'u';
    case MEB_NANOSECONDS:  return 'n';
    }
    return '?';
}

char* meb_log_level_str(const MebLogLevel level) {
    switch (level) {
    case MEB_NONE:    return "";
    case MEB_INFO:    return "INFO";
    case MEB_WARNING: return "WARNING";
    case MEB_ERROR:   return "ERROR";
    }
    MEB_ASSERT(0 && "meb: invalid log level!");
    return "?";
}

void meb_prof_mode(MebContext* ctx, const MebTimeMode mode) {
    ctx->time_mode = mode;
}

void meb_log_level(MebContext* ctx, const MebLogLevel level) {
    ctx->log_level = level;
}

#endif // MEB_IMPLEMENTATION
