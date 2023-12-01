#define _GNU_SOURCE /* Also supported in musl libc */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <linux/input.h>  /* For EVIOC* ioctls. */

static struct option evdev_cli_options[] = {
	{"device", required_argument, NULL, 'd'},
	{NULL, 0, NULL, 0},
};

struct evdev_protocol_version {
	int major;
	int minor;
};

int evdev_ioctl_driver_version(int file_desc, struct evdev_protocol_version *pv) {
	int ioctl_ret;
	int ioctl_arg;
	
	ioctl_ret = ioctl(file_desc, EVIOCGVERSION, (char*) &ioctl_arg);
	
	if (ioctl_ret < 0)
		return ioctl_ret;

	pv->major = ioctl_arg >> 16;
	pv->minor = ioctl_arg & ((1 << 16) - 1);

	return 0;
}

int evdev_ioctl_device_name(int file_desc, char *name) {
	int ioctl_ret;

	ioctl_ret = ioctl(file_desc, EVIOCGNAME(128), name);

	if (ioctl_ret < 0)
		return ioctl_ret;

	return 0;
}

int main(int argc, char *argv[]) {
	int file_desc, ret_val;
	int driver_version, _drvu, _drvl;
	struct evdev_protocol_version version;
	char evdev_device_name[128];
	char *evdev_device_path;
	int ch;

	while ((ch = getopt_long(argc, argv, "d:", evdev_cli_options, NULL)) != -1) {
		switch (ch) {
			case 'd':
				evdev_device_path = optarg;
				break;
			default:
				printf("Usage: evdevcli -d|--device <path>\n");
				exit(-1);
		}
	}


	file_desc = open(evdev_device_path, 0);
	if (file_desc < 0) {
		printf("Can't open device file %s\n", evdev_device_path);
		goto exit_err;
	}

	ret_val = evdev_ioctl_driver_version(file_desc, &version);
	if (ret_val < 0) {
		printf("Couldn't query evdev protocol version for device %s\n", evdev_device_path);
		goto exit_err_cleanup;
	}

	printf("evdev protocol version: %d.%d\n", version.major, version.minor);

	ret_val = evdev_ioctl_device_name(file_desc, evdev_device_name);
	if (ret_val < 0) {
		printf("Couldn't query evdev device name for device %s\n", evdev_device_path);
		goto exit_err_cleanup;
	}

	printf("\tDevice Name: %s\n", evdev_device_name);

	ret_val = 0;
exit_err_cleanup:
	close(file_desc);
exit_err:
	return ret_val;
}

