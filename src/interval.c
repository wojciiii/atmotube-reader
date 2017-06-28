#include "interval.h"
#include "atmotube_common.h"
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>

#define FORMAT_LD "%ld"
#define FORMAT_FL "%d"

typedef struct
{
	unsigned long time_interval;
	unsigned long current_ts;
	unsigned long max_ts;
	GSList* intervalList;
} IntervalInternals;

typedef union
{
	unsigned long ul;
	double d;
} IntervalData;

typedef struct
{
	char* label;
	char* fmt;

	unsigned int times;
	IntervalData current;
} Interval;

static IntervalInternals internals;

inline static unsigned long getTimeStamp(void)
{
	struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

int interval_define(const char *label, const char *fmt)
{
	int status = ATMOTUBE_RET_ERROR;

	if (strcmp(fmt, FORMAT_LD))
	{
		status = ATMOTUBE_RET_OK;
	}
	else if (strcmp(fmt, FORMAT_FL))
	{
		status = ATMOTUBE_RET_OK;
	}

	if (status == ATMOTUBE_RET_OK)
	{
		Interval* i = (Interval*)malloc(sizeof(Interval));
		i->label=strdup(label);
		i->fmt=strdup(fmt);
		i->times = 0;
		i->current.ul = 0;
		i->current.d = 0.0f;

		internals.intervalList = g_slist_append(internals.intervalList, i);
	}

	return status;
}

int interval_start(unsigned long interval_ms)
{
	internals.time_interval = interval_ms;
	internals.current_ts = getTimeStamp();
	internals.max_ts = internals.current_ts + internals.time_interval;
	PRINT_DEBUG("Interval started (%ld)\n", internals.time_interval);

	return ATMOTUBE_RET_OK;
}

typedef struct 
{
	const char* label;
	IntervalData data;
} intervalParams;

static void interval_log_impl(gpointer data,
                              gpointer user_data)
{
	Interval* i = (Interval*)data;
	intervalParams* p = (intervalParams*)user_data;

	if (strcmp(p->label, i->label))
	{
		PRINT_DEBUG("Logging: %s\n", p->label);
	}
}

void interval_log(const char *label, const char *fmt, ...)
{
	va_list ap;
	intervalParams p;
	p.label = label;

	if (strcmp(fmt, FORMAT_LD))
	{
		va_start(ap, fmt);
		p.data.ul = va_arg (ap, unsigned long);
		va_end(ap);
	}
	else if (strcmp(fmt, FORMAT_FL))
	{
		va_start(ap, fmt);
		p.data.d = va_arg (ap, double);
		va_end(ap);
	}
	else
	{
		PRINT_DEBUG("Invalid format: %s\n", fmt);
		return;
	}

	unsigned long ts = getTimeStamp();

	if (ts < internals.max_ts)
	{
		PRINT_DEBUG("Logging: %lu-%lu, %s\n", internals.current_ts, internals.max_ts, label);
	}
	else
	{
		internals.max_ts = ts + internals.time_interval;
		PRINT_DEBUG("Logging: %lu-%lu, %s\n", internals.current_ts, internals.max_ts, label);
	}

	g_slist_foreach (internals.intervalList, interval_log_impl, &p);
}

int interval_stop()
{
	// TODO: clear the allocated memory.
	return ATMOTUBE_RET_OK;
}
