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

static cfg_opt_t opts[] = {
		CFG_SEC("device", device_opts, CFGF_MULTI | CFGF_TITLE),
		CFG_END()
	};

static int numDevices;

static char* configFilename;

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

		const char *filename = "/.atmotube/config";
		int size = strlen(homedir)+strlen(filename)+1;

		configFilename = malloc(size);
		memset(configFilename, 0, size);
		memcpy(configFilename, homedir, strlen(homedir));
		memcpy(configFilename+strlen(filename), filename, strlen(filename));
	}

	PRINT_DEBUG("Using config file: %s\n", configFilename);
}

int atmotube_config_load(int (*fp)(char* name, char* deviceAddress, char* description, int resolution))
{
	int ret = 0;
	int i = 0;

	cfg = cfg_init(opts, CFGF_NOCASE);
	cfg_set_validate_func(cfg, "device", &validate_device);
	ret = cfg_parse(cfg, configFilename);
	PRINT_DEBUG("Load: cfg_parse, ret=%d\n", ret);

	if (ret != 0)
	{
		return ATMOTUBE_RET_ERROR;
	}

	numDevices = cfg_size(cfg, "device");

	PRINT_DEBUG("Load: %d devices present\n", numDevices);

	for (i = 0; i < numDevices; i++)
	{
		cfg_t *device = cfg_getnsec(cfg, "device", i);
		PRINT_DEBUG("device #%u (%s):\n", i + 1, cfg_title(device));
		PRINT_DEBUG("  name = %s\n", cfg_getstr(device, "name"));
		PRINT_DEBUG("  address = %s\n", cfg_getstr(device, "address"));
		PRINT_DEBUG("  description = %s\n", cfg_getstr(device, "description"));
		PRINT_DEBUG("  resolution = %ld\n", cfg_getint(device, "resolution"));

		fp(
			cfg_getstr(device, "name"),
			cfg_getstr(device, "address"),
			cfg_getstr(device, "description"),
			cfg_getint(device, "resolution")
		);
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

