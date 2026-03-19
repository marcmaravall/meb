/* ------------ meb.h --------------------------\
* A simple logging library for C.				|
* Author: Marc Maravall							|
* License: MIT License							|
* Repo: https://www.github.com/marcmaravall/meb |
* Thanks for using my library!					|
* ---------------------------------------------*/

#ifndef MEB_H
#define MEB_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <time.h>

#define MEB_ASSERT(expr) assert(expr)
#define MEB_LOG(ctx, m) meb_log(ctx, m)

#define MEB_BUFF_SIZE 256

typedef enum {
	MEB_SECONDS,
	MEB_MILLISECONDS,
	MEB_MICROSECONDS,
	MEB_NANOSECONDS,
} MebTimeMode;

char meb_time_unit(const MebTimeMode mode);

typedef struct {
	FILE* debug;
	double start;
	double end;
	MebTimeMode time_mode;
} MebContext;

double meb_get_time(const MebTimeMode mode);

void meb_init	(MebContext* ctx, const char* file);
void meb_log	(MebContext* ctx, const char* message);

void meb_prof_start (MebContext* ctx);
void meb_prof_end   (MebContext* ctx);

void meb_prof_mode(MebContext* ctx, MebTimeMode mode);

void meb_close	(MebContext* ctx);

#endif // MEB_H

// implementation ----
#ifdef MEB_IMPLEMENTATION

void meb_init(MebContext* context, const char* file) {
#ifndef MEB_NO_LOG
	context->time_mode = MEB_MICROSECONDS;
	MEB_ASSERT(context);

#ifdef MEB_LOG_TO_FILE
	context->debug = fopen(file, "w");

	if (!context->debug) {
		fprintf(stderr, "Failed to open log file: %s\n", file);
	}
#elif defined (MEB_LOG_TO_STDOUT)
	context->debug = stdout;
#else
#warning "WARNING: no logging method defined!"
#endif

#endif // MEB_NO_LOG
}

#ifndef MEB_LOG_IMPLEMENTATION
#define MEB_LOG_IMPLEMENTATION
void meb_log(MebContext* context, const char* message) {
#ifndef MEB_NO_LOG
	MEB_ASSERT(context);
	
	if (context->debug) {
		fprintf(context->debug, "%s\n", message);
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

	meb_log(ctx, buffer);
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

void meb_prof_mode(MebContext* ctx, MebTimeMode mode) {
	ctx->time_mode = mode;
}

#endif // MEB_IMPLEMENTATION
