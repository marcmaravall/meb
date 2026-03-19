/* ------------ meb.h ----------------------------\
* A simple logging library for C.				  |
* Author: Marc Maravall							  |
* License: MIT License							  |
* Repo: https://www.github.com/marcmaravall/meb   |
* Thanks for using my library!					  |
* ------------------------------------------------|
* TODO:											  |
* - Add support for more platforms (Linux, macOS).|
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
#include <time.h>

#ifndef MEB_NO_LOG
#define MEB_ASSERT(expr) assert(expr)
#define MEB_LOG(ctx, m) meb_log(ctx, m)
#define MEB_LOGF(ctx, fmt, ...) \
    do { \
        char buf[MEB_BUFF_SIZE]; \
        snprintf(buf, MEB_BUFF_SIZE, fmt, __VA_ARGS__); \
        meb_log(ctx, buf); \
    } while(0)
#else
#define MEB_ASSERT(expr) ((void)0)
#define MEB_LOG(ctx, m) ((void)0)
#define MEB_LOGF(ctx, fmt, ...) ((void)0)
#endif // MEB_NO_LOG

#define MEB_BUFF_SIZE 256

typedef enum {
	MEB_SECONDS,
	MEB_MILLISECONDS,
	MEB_MICROSECONDS,
	MEB_NANOSECONDS,
} MebTimeMode;

char meb_time_unit(const MebTimeMode mode);

typedef enum {
	MEB_NONE,
	MEB_INFO,
	MEB_WARNING,
	MEB_ERROR,
} MebLogLevel;

char* meb_log_level_str(const MebLogLevel level);

typedef struct {
	FILE* debug;
	double start;
	double end;
	MebTimeMode time_mode;
	MebLogLevel log_level;
} MebContext;

double meb_get_time(const MebTimeMode mode);

void meb_init	(MebContext* ctx, const char* file);

void meb_log	(MebContext* ctx, const char* message);

void meb_prof_start (MebContext* ctx);
void meb_prof_end   (MebContext* ctx);

void meb_prof_mode (MebContext* ctx, const MebTimeMode mode);
void meb_log_level (MebContext* ctx, const MebLogLevel level);

void meb_close	(MebContext* ctx);

#endif // MEB_H

// implementation ----
#ifdef MEB_IMPLEMENTATION

void meb_init(MebContext* ctx, const char* file) {
#ifndef MEB_NO_LOG
	ctx->time_mode = MEB_MICROSECONDS;
	ctx->log_level = MEB_INFO;

	MEB_ASSERT(ctx);

#ifdef MEB_LOG_TO_FILE
	ctx->debug = fopen(file, "w");

	if (!ctx->debug) {
		fprintf(stderr, "Failed to open log file: %s\n", file);
	}
#elif defined (MEB_LOG_TO_STDOUT)
	ctx->debug = stdout;
#else
#warning "WARNING: no logging method defined!"
#endif

#endif // MEB_NO_LOG
}

#ifndef MEB_LOG_IMPLEMENTATION
#define MEB_LOG_IMPLEMENTATION
void meb_log(MebContext* ctx, const char* message) {
	MEB_ASSERT(ctx);
	char* level_str = meb_log_level_str(ctx->log_level);

#ifndef MEB_NO_LOG
	
	if (ctx->debug) {
		fprintf(ctx->debug, "%s: %s\n", level_str, message);
	}
#endif // MEB_NO_LOG
}
#endif // MEB_LOG_IMPLEMENTATION

void meb_close(MebContext* ctx) {
#ifndef MEB_NO_LOG
	fclose(ctx->debug);
#endif // MEB_NO_LOG
}

void meb_prof_start(MebContext* ctx) {
	ctx->start = meb_get_time(ctx->time_mode);
	meb_log(ctx, "[MEB_PROFILING] Meb profiling started!");
}

void meb_prof_end(MebContext* ctx) {
	ctx->end = meb_get_time(ctx->time_mode);

	double elapsed = (double)(ctx->end - ctx->start);
	
	char buffer[MEB_BUFF_SIZE];
	snprintf(buffer, MEB_BUFF_SIZE, 
		"[MEB_PROFILING] Meb profiling finished! Elapsed time: %.6f%c", 
		elapsed, meb_time_unit(ctx->time_mode));

	MebLogLevel level = ctx->log_level;
	meb_log_level(ctx, MEB_INFO);
	meb_log(ctx, buffer);
	meb_log_level(ctx, level);
}

double meb_get_time(const MebTimeMode mode) {
#ifdef _WIN32
	static LARGE_INTEGER frequency;
	static int initialized = 0;

	if (!initialized) {
		QueryPerformanceFrequency(&frequency);
		initialized = 1;
	}

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	double time = (double)counter.QuadPart / frequency.QuadPart;
	switch (mode) {
	case MEB_SECONDS:      return time;
	case MEB_MILLISECONDS: return time * 1e3;
	case MEB_MICROSECONDS: return time * 1e6;
	case MEB_NANOSECONDS:  return time * 1e9;
	}
	return time;
#else
#warning "High-resolution timing is not implemented for this platform."
#endif
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
	MEB_ASSERT(0 && "Invalid case!");
	return "?";
}

void meb_prof_mode(MebContext* ctx, const MebTimeMode mode) {
	ctx->time_mode = mode;
}

void meb_log_level(MebContext* ctx, const MebLogLevel level) {
	ctx->log_level = level;
}

#endif // MEB_IMPLEMENTATION
