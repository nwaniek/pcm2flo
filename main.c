#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <stdbool.h>


char*
strdup(const char *str)
{
	int n = strlen(str) + 1;
	char *dup = malloc(n);
	if (dup) strcpy(dup, str);
	return dup;
}


bool
test_file(char *f)
{
	struct stat sb;
	if (stat(f, &sb) == -1) {
		if (errno == ENOENT) {
			fprintf(stderr, "Warning: File %s does not exist.\n", f);
			return false;
		}
		else {
			fprintf(stderr, "Warning: Could not process file %s\n", f);
			return false;
		}
	}
	return true;
}


char*
floname(char *f)
{
	char *dup = strdup(f);
	char *bname = basename(dup);
	char *extn = NULL;
	extn = strrchr(bname, '.');
	if (extn) *extn = '\0';
	free(dup);
	char *result = strdup(bname);
	strcat(result, ".flo");
	return result;
}


void
convert(char *srcname)
{
	if (!test_file(srcname))
		return;

	// read
	FILE *f_src = fopen(srcname, "r");
	char ftag[32];
	fscanf(f_src, "%s\n", ftag);
	if (strcmp(ftag, "PC")) {
		fprintf(stderr, "Warning: %s contains an invalid PCM header\n", srcname);
		return;
	}
	int w, h;
	float m;
	fscanf(f_src, "%d %d\n", &w, &h);
	fscanf(f_src, "%f\n", &m);
	if (w <= 0 || h <= 0) {
		fprintf(stderr, "Warning: %s contains invalid (w, h) dimensions\n", srcname);
		return;
	}
	float *data = malloc(2 * w * h * sizeof(float));
	if (fread((unsigned char*)data, 1, 2 * w * h * sizeof(float), f_src) != 2 * w * h * sizeof(float)) {
		fprintf(stderr, "Warning: %s contains invalid data\n", srcname);
		free(data);
		return;
	}

	// write
	char *dstname = floname(srcname);
	FILE *f_dst= fopen(dstname, "w");
	fprintf(f_dst, "PIEH");
	fwrite(&w, sizeof(int), 1, f_dst);
	fwrite(&h, sizeof(int), 1, f_dst);
	fwrite(data, 1, 2 * w * h * sizeof(float), f_dst);

	// memory + file management
	free(data);
	free(dstname);
	fclose(f_src);
	fclose(f_dst);
}

int
main (int argc, char *argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "usage: pcm2flo file...\n");
		return EXIT_FAILURE;
	}
	while (--argc)
		convert((++argv)[0]);
	return EXIT_SUCCESS;
}
