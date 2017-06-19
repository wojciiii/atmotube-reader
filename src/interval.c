#include "interval.h"
#include "atmotube_common.h"
#include <glib.h>
#include <unistd.h>

static GThread *thr;
static GMutex mutex;

typedef struct
{
	unsigned long time_interval;
	int running;
} ThreadData;

static ThreadData threadData;

int interval_define(const char *label, const char *fmt)
{
	// TODO: implement this.
	return ATMOTUBE_RET_ERROR;
}

static gpointer thrfct(gpointer data)
{
	ThreadData* td = (ThreadData*)data;

	PRINT_DEBUG("thrfct started\n");

	while (td->running)
	{
		g_mutex_lock (&mutex);
		PRINT_DEBUG("Logging\n");
		// TODO: implement this.
		g_mutex_unlock (&mutex);
		usleep(100*1000);
	}

	PRINT_DEBUG("Terminate thr\n");
	return 0;
}

int interval_start(unsigned long interval_ms)
{
	threadData.time_interval = interval_ms;
	threadData.running = 1;

	PRINT_DEBUG("Interval started (%ld)\n", threadData.time_interval);
	gpointer data = &threadData;

	thr = g_thread_new ("intervallog", thrfct, data);

	return ATMOTUBE_RET_OK;
}

void interval_log(const char *label, void* p)
{
	g_mutex_lock (&mutex);
	PRINT_DEBUG("Logging: %s:%p\n", label, p);
	g_mutex_unlock (&mutex);
}

int interval_stop()
{
	threadData.running = 0;
	g_thread_join(thr);

	return ATMOTUBE_RET_OK;
}
