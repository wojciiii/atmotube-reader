#include <confuse.h>
#include "atmotube-config.h"
#include "atmotube_common.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>

static cfg_t *cfg;

static cfg_opt_t device_opts[] = {
    CFG_STR("name", 0, CFGF_NONE),
    CFG_STR("address", 0, CFGF_NONE),
    CFG_STR("description", 0, CFGF_NONE),
    CFG_INT("resolution", 0, CFGF_NONE),
    CFG_END()
};

static int validate_output_type(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result);

static cfg_opt_t output_opts[] = {
    CFG_INT_CB("type", 0, CFGF_NONE, &validate_output_type),
    CFG_STR("source", 0, CFGF_NONE),
    CFG_STR("filename", 0, CFGF_NONE),
    CFG_END()
};

static cfg_opt_t opts[] = {
    CFG_SEC("device", device_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_SEC("output", output_opts, CFGF_MULTI | CFGF_TITLE),
    CFG_END()
};

static int numDevices = 0;
static int numOutputs = 0;

static char* configFilename;

static int validate_output(cfg_t *cfg, cfg_opt_t *opt)
{
	cfg_t *sec = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);

	if (!sec) {
	    cfg_error(cfg, "section is NULL!?");
	    return ATMOTUBE_RET_ERROR;
	}

	switch (cfg_getint(sec, "type")) {
	case OUTPUT_FILE:
	case OUTPUT_DB:
	    break;
	default:
	    cfg_error(cfg, "type option must be set for output '%s'", cfg_title(sec));
	    return ATMOTUBE_RET_ERROR;
	}
	
	
	if (cfg_getstr(sec, "filename") == 0) {
	    cfg_error(cfg, "address option must be set for output '%s'", cfg_title(sec));
	    return ATMOTUBE_RET_ERROR;
	}

	if (cfg_getstr(sec, "source") == 0) {
	    cfg_error(cfg, "source option must be set for output '%s'", cfg_title(sec));
	    return ATMOTUBE_RET_ERROR;
	}

	return ATMOTUBE_RET_OK;
}

static int validate_output_type(cfg_t *cfg, cfg_opt_t *opt, const char *value, void *result)
{
    if(strcmp(value, "file") == 0)
	*(int *)result = OUTPUT_FILE;
    else if(strcmp(value, "db") == 0)
	*(int *)result = OUTPUT_DB;
    else
	{
	    /* cfg_error(cfg, "Invalid action value '%s'", value); */
	    return -1;
	}
    return 0;
}

static int validate_device(cfg_t *cfg, cfg_opt_t *opt)
{
	cfg_t *sec = cfg_opt_getnsec(opt, cfg_opt_size(opt) - 1);

	if (!sec)
	{
		cfg_error(cfg, "section is NULL!?");
		return ATMOTUBE_RET_ERROR;
	}

	if (cfg_getstr(sec, "name") == 0)
	{
		cfg_error(cfg, "name option must be set for device '%s'", cfg_title(sec));
		return ATMOTUBE_RET_ERROR;
	}

	if (cfg_getstr(sec, "address") == 0)
	{
		cfg_error(cfg, "address option must be set for device '%s'", cfg_title(sec));
		return ATMOTUBE_RET_ERROR;
	}

	if (cfg_getstr(sec, "description") == 0)
	{
		cfg_error(cfg, "description option must be set for device '%s'", cfg_title(sec));
		return ATMOTUBE_RET_ERROR;
	}

	if (cfg_getint(sec, "resolution") == 0)
	{
		cfg_error(cfg, "resolution option must be set for device '%ld'", cfg_title(sec));
		return ATMOTUBE_RET_ERROR;
	}

	return ATMOTUBE_RET_OK;
}

void atmotube_config_start(char* fullName)
{
	if (fullName != NULL)
	{
		configFilename = malloc(strlen(fullName) + 1);
		strcpy(configFilename, fullName);
	}
	else
	{
		struct passwd *pw = getpwuid(getuid());
		const char *homedir = pw->pw_dir;
		const char *filename = ".atmotube/config";

		configFilename = malloc(snprintf(NULL, 0, "%s/%s", homedir, filename) + 1);
		sprintf(configFilename, "%s/%s", homedir, filename);
	}

	PRINT_DEBUG("Using config file: %s\n", configFilename);
}

int atmotube_config_load(deviceCB* deviceFb)
{
    int ret = 0;
    int i = 0;

    cfg = cfg_init(opts, CFGF_NOCASE);
    cfg_set_validate_func(cfg, "device", &validate_device);
    cfg_set_validate_func(cfg, "output", &validate_output);
    
    ret = cfg_parse(cfg, configFilename);
    PRINT_DEBUG("Load: cfg_parse, ret=%d\n", ret);
    
    if (ret != 0) {
	return ATMOTUBE_RET_ERROR;
    }
    
    numDevices = cfg_size(cfg, "device");
	
    PRINT_DEBUG("Load: %d device(s) present\n", numDevices);

    for (i = 0; i < numDevices; i++) {
	
    }
    
    for (i = 0; i < numDevices; i++) {
	cfg_t *device = cfg_getnsec(cfg, "device", i);
	PRINT_DEBUG("Device #%u (%s):\n", i + 1, cfg_title(device));
	PRINT_DEBUG("  name = %s\n", cfg_getstr(device, "name"));
	PRINT_DEBUG("  address = %s\n", cfg_getstr(device, "address"));
	PRINT_DEBUG("  description = %s\n", cfg_getstr(device, "description"));
	PRINT_DEBUG("  resolution = %ld\n", cfg_getint(device, "resolution"));

	/*
	ret = deviceFb(
		       0,
		       cfg_getstr(device, "name"),
		       cfg_getstr(device, "address"),
		       cfg_getstr(device, "description"),
		       cfg_getint(device, "resolution")
		       );
	*/
	PRINT_DEBUG("Added device, ret = %d\n", ret);
    }
    
    numOutputs = cfg_size(cfg, "output");
    PRINT_DEBUG("Load: %d output(s) present\n", numOutputs);
    
    for (i = 0; i < numDevices; i++) {
	cfg_t *output = cfg_getnsec(cfg, "output", i);
	PRINT_DEBUG("Output #%u (%s):\n", i + 1, cfg_title(output));
	PRINT_DEBUG("  type = %ld\n", cfg_getint(output, "type"));
	PRINT_DEBUG("  filename = %s\n", cfg_getstr(output, "filename"));
	PRINT_DEBUG("  source = %s\n", cfg_getstr(output, "source"));
    }
    
    cfg_free(cfg);
    cfg = NULL;
    
    return ATMOTUBE_RET_OK;
}

void atmotube_config_end()
{
    free(configFilename);
    configFilename = NULL;
}

