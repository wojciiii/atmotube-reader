#include "interval.h"
#include "atmotube_common.h"
#include <glib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdbool.h>

static const char* format_ld = INTERVAL_ULONG;
static const char* format_fl = INTERVAL_FLOAT;

typedef union
{
    unsigned long ul;
    double d;
} IntervalData;

typedef struct
{
    bool callback_set;
    union
    {
	ulong_callback ulong_cb;
	float_callback float_cb;
    } u;

} Callback;

typedef struct
{
    const char* label;
    const char* fmt;
} IntervalSettings;

typedef struct
{
    /* Identification. */
    const char* label;
    const char* fmt;
    /* State. */
    bool started;
    /* Time. */
    unsigned long time_interval;
    unsigned long current_ts;
    unsigned long max_ts;
    /* Contents. */
    unsigned long times;
    IntervalData current;
    Callback callback;
} Interval;

GSList* intervalList = NULL;

#define find_in_list(found, label, fmt)					\
    GSList *l;								\
    for (l = intervalList; l != NULL; l = l->next) {			\
	Interval *i = (Interval*)l->data;				\
	if ( (strcmp(i->label, label) == 0) &&				\
	     (strcmp(i->fmt, fmt) == 0) ) {				\
	    found = i;							\
	    break;							\
	}								\
    }

inline static unsigned long getTimeStamp(void)
{
    struct timeval te; 
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

int interval_add(const char *label, const char *fmt)
{
    int status = ATMOTUBE_RET_ERROR;
    
    if (strcmp(fmt, format_ld) == 0) {
	status = ATMOTUBE_RET_OK;
    }
    else if (strcmp(fmt, format_fl) == 0) {
	status = ATMOTUBE_RET_OK;
    }

    if (status == ATMOTUBE_RET_OK) {
	Interval* i = (Interval*)malloc(sizeof(Interval));
	memset(i, 0, sizeof(Interval));
	i->label=g_strdup(label);
	i->fmt=g_strdup(fmt);
	i->started = false;
	i->time_interval=1;
	i->current_ts = 0;
	i->max_ts = 0;
	i->times = 0;
	i->current.ul = 0;
	i->current.d = 0.0f;
	PRINT_DEBUG("Adding interval %s:%s\n", label, fmt);
	intervalList = g_slist_append(intervalList, i);

	PRINT_DEBUG("List len=%d\n", g_slist_length(intervalList));
    }

    return status;
}

int interval_remove_callbacks(const char *label, const char *fmt)
{
    Interval* found  = NULL;
    find_in_list(found, label, fmt);

    if (found != NULL) {
	found->callback.callback_set = false;
	found->callback.u.ulong_cb = NULL;
	found->callback.u.float_cb = NULL;
    }
    else {
	PRINT_DEBUG("Interval %s:%s not found\n", label, fmt);
    }

    return ATMOTUBE_RET_ERROR;
}

int interval_add_ulong_callback(const char *label, const char *fmt, ulong_callback callback)
{
    Interval* found  = NULL;
    find_in_list(found, label, fmt);

    if (found != NULL) {
	found->callback.u.ulong_cb = callback;
	found->callback.callback_set = true;
	PRINT_DEBUG("Interval %s:%s added ulong cb\n", label, fmt);
	return ATMOTUBE_RET_OK;
    }
    else {
	PRINT_DEBUG("Interval %s:%s not found\n", label, fmt);
    }

    return ATMOTUBE_RET_ERROR;
}

int interval_add_float_callback(const char *label, const char *fmt, float_callback callback)
{
    Interval* found  = NULL;
    find_in_list(found, label, fmt);

    if (found != NULL) {
	found->callback.u.float_cb = callback;
	found->callback.callback_set = true;
	PRINT_DEBUG("Interval %s:%s added float cb\n", label, fmt);
	return ATMOTUBE_RET_OK;
    }
    else {
	PRINT_DEBUG("Interval %s:%s not found\n", label, fmt);
    }

    return ATMOTUBE_RET_ERROR;
}

int interval_remove(const char *label, const char *fmt)
{
    Interval* found  = NULL;
    find_in_list(found, label, fmt);

    if (found != NULL) {
	free(found);
	intervalList = g_slist_remove (intervalList,
				       found);
	return ATMOTUBE_RET_OK;
    }

    return ATMOTUBE_RET_ERROR;
}

int interval_start(const char *label,
		   const char *fmt,
		   unsigned long interval_ms)
{
    Interval* found  = NULL;
    find_in_list(found, label, fmt);

    if (found != NULL) {
	PRINT_DEBUG("Interval: %lu, %lu, %lu\n", found->time_interval, found->current_ts, found->max_ts);
		
	found->time_interval = interval_ms;
	found->current_ts = getTimeStamp();
	found->max_ts     = found->current_ts + interval_ms;
	found->started    = true;
	PRINT_DEBUG("Interval %s:%s started (%ld)\n", label, fmt, interval_ms);
		
	return ATMOTUBE_RET_OK;
    }
    else {
	PRINT_DEBUG("Interval %s:%s not found\n", label, fmt);
    }

    return ATMOTUBE_RET_ERROR;
}

typedef struct 
{
    const char* label;
    const char* fmt;
    IntervalData data;
    bool last;
} intervalParams;

/*
 * Taken from: 
 * https://stackoverflow.com/questions/12636613/how-to-calculate-moving-average-without-keeping-the-count-and-data-total
 */
static double appr_rolling_average(double avg, double new_sample, unsigned long N)
{
    avg -= avg / N;
    avg += new_sample / N;

    return avg;
}

static void interval_log_impl(gpointer data,
                              gpointer user_data)
{
    Interval* i = (Interval*)data;
    intervalParams* p = (intervalParams*)user_data;

    //    PRINT_DEBUG("%s vs %s\n", p->label, i->label);
    
    if ( (strcmp(p->label, i->label) == 0) &&
	 (strcmp(p->fmt, i->fmt) == 0) ) {
	unsigned long ts = getTimeStamp();

	if (!i->started) {
	    PRINT_DEBUG("Interval %s:%s is not started\n", i->label, i->fmt);
	    return;
	}
	     
	if (ts < i->max_ts) {
	    p->last = false;
	    //PRINT_DEBUG("Logging: %lu-%lu, %s\n", internals.current_ts, internals.max_ts, label);
	}
	else {
	    p->last = true;
	    i->max_ts = ts + i->time_interval;
	    //PRINT_DEBUG("Logging: %lu-%lu, %s\n", internals.current_ts, internals.max_ts, label);
	}

	if (strcmp(p->fmt, format_ld) == 0) {
	    
	    if (i->times == 0) {
		i->current.ul = p->data.ul;
		i->times++;
	    }
	    else {
		i->times++;
		i->current.ul = (unsigned long)appr_rolling_average(i->current.ul, p->data.ul, i->times);
	    }
	    if (p->last) {
		PRINT_DEBUG("+Logging(%s): %s, %lu times, current = %lu\n", p->fmt, p->label, i->times, i->current.ul);
		i->times = 0;
		if (i->callback.callback_set) {
		    i->callback.u.ulong_cb(ts, i->current.ul);
		}
	    }
	    /*
	    else {
		PRINT_DEBUG("Logging(%s): %s, %d times, current = %lu\n", p->fmt, p->label, i->times, i->current.ul);
	    }
	    */
	    
	}
	else if (strcmp(p->fmt, format_fl) == 0) {
	    
	    if (i->times == 0) {
		i->current.d = p->data.d;
		i->times++;
	    }
	    else {
		i->times++;
		i->current.d = appr_rolling_average(i->current.d, p->data.d, i->times);
	    }
	    
	    if (p->last) {
		PRINT_DEBUG("+Logging(%s): %s, %lu times, current = %f\n", p->fmt, p->label, i->times, i->current.d);
		i->times = 0;
		if (i->callback.callback_set) {
		    i->callback.u.float_cb(ts, i->current.d);
		}
	    }
	    /*
	    else {
		PRINT_DEBUG("Logging(%s): %s, %d times, current = %f\n", p->fmt, p->label, i->times, i->current.d);
	    }
	    */
	}
    }
}

void interval_log(const char *label, const char *fmt, ...)
{
    va_list ap;
    intervalParams p = {0};

    p.label = label;
    p.fmt = fmt;

    if (strcmp(fmt, format_ld) == 0) {
	va_start(ap, fmt);
	p.fmt = format_ld;
	p.data.ul = va_arg (ap, unsigned long);
	/* PRINT_DEBUG("Got format: long -> %ld\n", p.data.ul); */
	va_end(ap);
    }
    else if (strcmp(fmt, format_fl) == 0) {
	va_start(ap, fmt);
	p.fmt = format_fl;
	p.data.d = va_arg (ap, double);
	/* PRINT_DEBUG("Got format: float -> %f\n", p.data.d); */
	va_end(ap);
    }
    else {
	PRINT_DEBUG("Invalid format: %s\n", fmt);
	return;
    }

    g_slist_foreach (intervalList, interval_log_impl, &p);
}

int interval_stop(const char *label, const char *fmt)
{
    Interval* found  = NULL;
    find_in_list(found, label, fmt);

    if (found != NULL) {
	found->time_interval = 0;
	found->current_ts = 0;
	found->max_ts     = 0;
	found->started    = false;
	PRINT_DEBUG("Interval %s:%s stopped\n", label, fmt);
		
	return ATMOTUBE_RET_OK;
    }
    else {
	PRINT_DEBUG("Interval %s:%s not found\n", label, fmt);
    }

    return ATMOTUBE_RET_ERROR;
}

static void interval_dump_impl(gpointer data,
			       gpointer unsued)
{
    Interval* i = (Interval*)data;

    printf("Interval %s:%s\n", i->label, i->fmt);
    printf("\tstarted=%u\n", i->started);
    printf("\tinterval=%lu\n", i->time_interval);
    printf("\tts=%lu\n", i->current_ts);
    printf("\tmax_ts=%lu\n", i->max_ts);
    printf("\ttimes=%lu\n", i->times);
    printf("\tul=%lu\n", i->current.ul);
    printf("\td=%f\n", i->current.d);

}

void interval_dump(void)
{
    g_slist_foreach (intervalList, interval_dump_impl, NULL);
}
