
#define NOMINMAX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <limits>
#include <algorithm>

#include "rom2snsf.h"
#include "PSFFile.h"
#include "cpath.h"

#ifdef WIN32
#include <direct.h>
#include <float.h>
#define getcwd _getcwd
#define chdir _chdir
#define isnan _isnan
#define strcasecmp _stricmp
#else
#include <unistd.h>
#endif

#define APP_NAME    "rom2snsf"
#define APP_VER     "[2015-04-02]"
#define APP_URL     "http://github.com/loveemu/rom2snsf"

#define SNSF_EXE_HEADER_SIZE	8

static void writeInt(uint8_t * buf, uint32_t value)
{
	buf[0] = value & 0xff;
	buf[1] = (value >> 8) & 0xff;
	buf[2] = (value >> 16) & 0xff;
	buf[3] = (value >> 24) & 0xff;
}

bool rom2snsf(const char * rom_path, const char * snsf_path, uint32_t load_offset)
{
	off_t off_rom_size = path_getfilesize(rom_path);
	if (off_rom_size == -1) {
		fprintf(stderr, "Error: File not found \"%s\"\n", rom_path);
		return false;
	}

	size_t rom_size = off_rom_size;
	if (rom_size > 0x800000) {
		fprintf(stderr, "Error: File too large \"%s\"\n", rom_path);
		return false;
	}

	uint8_t * exe = (uint8_t *)malloc(SNSF_EXE_HEADER_SIZE + rom_size);
	if (exe == NULL) {
		fprintf(stderr, "Error: Memory allocation error\n");
		return false;
	}
	writeInt(&exe[0], load_offset);
	writeInt(&exe[4], (uint32_t)rom_size);

	FILE * rom_file = fopen(rom_path, "rb");
	if (rom_file == NULL) {
		fprintf(stderr, "Error: File open error \"%s\"\n", rom_path);
		free(exe);
		return false;
	}

	if (fread(&exe[8], 1, rom_size, rom_file) != rom_size) {
		fprintf(stderr, "Error: File read error \"%s\"\n", rom_path);
		fclose(rom_file);
		free(exe);
		return false;
	}

	fclose(rom_file);

	ZlibWriter zlib_exe(Z_BEST_COMPRESSION);
	zlib_exe.write(exe, SNSF_EXE_HEADER_SIZE + rom_size);

	std::map<std::string, std::string> tags;
	if (!PSFFile::save(snsf_path, 0x23, NULL, 0, zlib_exe, tags)) {
		fprintf(stderr, "Error: File write error \"%s\"\n", snsf_path);
		free(exe);
		return false;
	}

	free(exe);

	return true;
}

static void usage(const char * progname)
{
	printf("%s %s\n", APP_NAME, APP_VER);
	printf("<%s>\n", APP_URL);
	printf("\n");
	printf("Usage\n");
	printf("-----\n");
	printf("\n");
	printf("Syntax: `%s (options) [SNES ROM files]`\n", progname);
	printf("\n");

	printf("### Options\n");
	printf("\n");
	printf("`--help`\n");
	printf("  : Show help\n");
	printf("\n");
	printf("`--load [offset]`\n");
	printf("  : Load offset of SNES executable\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	long longval;
	char *endptr = NULL;

	uint32_t load_offset = 0;
	char snsf_path[PATH_MAX];

	int argi = 1;
	while (argi < argc && argv[argi][0] == '-') {
		if (strcmp(argv[argi], "--help") == 0) {
			usage(argv[0]);
			return EXIT_FAILURE;
		}
		if (strcmp(argv[argi], "--load") == 0) {
			if (argi + 1 >= argc) {
				fprintf(stderr, "Error: Too few arguments for \"%s\"\n", argv[argi]);
				return EXIT_FAILURE;
			}

			longval = strtol(argv[argi + 1], &endptr, 16);
			if (*endptr != '\0' || errno == ERANGE)
			{
				fprintf(stderr, "Error: Number format error \"%s\"\n", argv[argi + 1]);
				return EXIT_FAILURE;
			}
			load_offset = longval;
			argi++;
		}
		else {
			fprintf(stderr, "Error: Unknown option \"%s\"\n", argv[argi]);
			return EXIT_FAILURE;
		}
		argi++;
	}

	int argnum = argc - argi;
	if (argnum == 0) {
		fprintf(stderr, "Error: Too few arguments\n");
		return EXIT_FAILURE;
	}

	int num_error = 0;
	for (; argi < argc; argi++) {
		const char * rom_path = argv[argi];
		const char * rom_ext = path_findext(rom_path);

		strcpy(snsf_path, rom_path);
		if (strcasecmp(rom_ext, ".smc") == 0) {
			path_stripext(snsf_path);
		}
		strcat(snsf_path, ".snsf");

		if (!rom2snsf(rom_path, snsf_path, load_offset)) {
			num_error++;
		}

		puts(snsf_path);
	}

	if (num_error != 0) {
		fprintf(stderr, "%d error(s)\n", num_error);
	}

	return (num_error == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}