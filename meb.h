

#ifndef MEB_H
#define MEB_H

#include <stdio.h>
#include <assert.h>

#define MEB_ASSERT(expr) assert(expr)
#define MEB_LOG(ctx, m) meb_log(ctx, m)

typedef struct {
	FILE* debug;
} MebContext;

void meb_init	(MebContext* ctx, const char* file);
void meb_log	(MebContext* ctx, const char* message);
void meb_close	(MebContext* ctx);

#endif // MEB_H

// implementation ----
#ifdef MEB_IMPLEMENTATION

void meb_init(MebContext* context, const char* file) {
#ifndef MEB_NO_LOG

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

#endif // MEB_IMPLEMENTATION
