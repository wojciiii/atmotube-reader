#include <confuse.h>
#include "atmotube-config.h"
#include "atmotube_common.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdbool.h>

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

static void dumpDevice(Atmotube_Device* device)
{
    PRINT_DEBUG("Device #%u (%s):\n", device->device_id, device->device_name);
    PRINT_DEBUG("  address = %s\n", device->device_address);
    PRINT_DEBUG("  description = %s\n", device->device_description);
    PRINT_DEBUG("  resolution = %d\n", device->device_resolution);
    PRINT_DEBUG("  output type = %d\n", device->output_type);
    PRINT_DEBUG("  filename = %s\n", device->output_filename);
}

/*
 * [ struct 0 containing Atmotube_Device ]
 * [ offset                              ]
 * [ Atmotube_Device member              ]
 * [ something else                      ]
 * [                                     ] element_size
 * [ struct 1 containing Atmotube_Device ]
 * [ offset                              ]
 * [ Atmotube_Device member              ]
 * [ something else                      ]
 * [                                     ] element_size
 *
 */

static Atmotube_Device* get_ptr(void *src, int number, size_t element_size, size_t offset)
{
    void* p = src;
    p += (number * element_size);
    p += offset;
    return (Atmotube_Device*)p;
}

int atmotube_config_load(NumDevicesCB numDevicesCb, deviceCB deviceFb,
			 size_t element_size, size_t offset)
{
    int ret = 0;
    int i = 0;
    int j = 0;
    void* memory = NULL;
    Atmotube_Device* devices = NULL;
    int deviceId = 0;
    
    cfg = cfg_init(opts, CFGF_NOCASE);
    cfg_set_validate_func(cfg, "device", &validate_device);
    cfg_set_validate_func(cfg, "output", &validate_output);
    
    ret = cfg_parse(cfg, configFilename);
    PRINT_DEBUG("Load: cfg_parse, ret=%d\n", ret);
    
    if (ret != 0) {
	cfg_free(cfg);
	return ATMOTUBE_RET_ERROR;
    }
    
    numDevices = cfg_size(cfg, "device");
    PRINT_DEBUG("Load: %d device(s) present\n", numDevices);
    
    numOutputs = cfg_size(cfg, "output");
    PRINT_DEBUG("Load: %d output(s) present\n", numOutputs);

    /* Check that devices and outputs match: */
    if (numDevices != numOutputs) {
	printf("Unexpected number of devices (%d) vs outputs (%d).\n", numDevices, numOutputs);
	printf("Was expecting %d outputs\n", numDevices);
	cfg_free(cfg);
	return ATMOTUBE_RET_ERROR;
    }

    /* Get memory used to keep devices in. */
    memory = numDevicesCb(numDevices);

    deviceId = 0;
    for (i = 0; i < numDevices; i++) {
	Atmotube_Device* device = get_ptr(memory, i, element_size, offset);
	device->device_id = deviceId;
	device->device_name = NULL;

	device->device_address = NULL;
	device->device_description = NULL;
	device->device_resolution = NULL;
	device->output_type = -1;
	device->output_filename = NULL;
    }
	
    deviceId = 0;
    for (i = 0; i < numDevices; i++) {
	cfg_t *cfg_device = cfg_getnsec(cfg, "device", i);
	Atmotube_Device* device = get_ptr(memory, i, element_size, offset);

	device->device_id = deviceId;
	device->device_name = cfg_getstr(cfg_device, "name");
	device->device_address = cfg_getstr(cfg_device, "address");
	device->device_description = cfg_getstr(cfg_device, "description");
	device->device_resolution = cfg_getint(cfg_device, "resolution");

	PRINT_DEBUG("Added device %d\n", deviceId);
	deviceId++;
    }

    for (i = 0; i < numOutputs; i++) {
	cfg_t *cfg_output = cfg_getnsec(cfg, "output", i);
	char* src = cfg_getstr(cfg_output, "source");

	PRINT_DEBUG("Output %s\n", src);
			
	bool found = false;
	for (j = 0; j < numDevices; j++) {
	    Atmotube_Device* device = get_ptr(memory, j, element_size, offset);
	    if (strcmp(device->device_name, src) == 0) {
		PRINT_DEBUG("Found device %s for source %s\n", device->device_name, src);
		device->output_type = cfg_getint(cfg_output, "type");
		device->output_filename = cfg_getstr(cfg_output, "filename");
		found = true;
	    }
	}
	if (!found) {
	    printf("Unable to find source %s\n", src);
	    cfg_free(cfg);
	    return ATMOTUBE_RET_ERROR;
	}
    }

    /* Check that all inputs have corresponding outputs: */
    for (i = 0; i < numDevices; i++) {
	Atmotube_Device* device = get_ptr(memory, i, element_size, offset);

	if (device->output_type == -1) {
	    printf("Device %s needs an output. Can't continue.\n", device->device_name);
	    cfg_free(cfg);
	    return ATMOTUBE_RET_ERROR;
	}
    }

    /* The configuration is complete. */
    for (i = 0; i < numDevices; i++) {
	Atmotube_Device* device = get_ptr(memory, i, element_size, offset);
	dumpDevice(device);
	deviceFb(device);
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

