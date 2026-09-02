#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void creat_directories(const char *absolute_path) {
	char temporary_buffer[1024];
	char *ptr = NULL;
	size_t path_length;

	snprintf(temporary_buffer, sizeof(temporary_buffer), "%s", absolute_path);
	path_length = strlen(temporary_buffer);
	if (temporary_buffer[path_length - 1] == '/') {
		temporary_buffer[path_length - 1] = 0; // To avoid create empty directory
	}

	for (ptr = temporary_buffer + 1; *ptr; ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			mkdir(temporary_buffer, 0755);
			*ptr = '/';
		}
	}
}

/**
 * I/O instruction function takes the relative path of the target file and read first 4 byte as a unsigned integer or crete it if not exist. The
 * default value is `0` if the file was created in this session will return `0` else will return the first 4 byte value.
 * @param relative_path const character pointer to the relative file in `HOME` directory.
 * @return `high_score` - an unsigned integer contain the value of first 4 byte int the file or `0`.      
 */
unsigned int load_high_score(const char *relative_path) {
	unsigned int high_score = 0;

	const char *home_dir = getenv("HOME");
	char absolute_path[1024];
	snprintf(absolute_path, sizeof(absolute_path), "%s/%s", home_dir, relative_path);
	creat_directories(absolute_path);
	
	FILE *file = fopen(absolute_path, "a+b");
	if (file) {
		rewind(file);
		fread(&high_score, sizeof(unsigned int), 1, file);
		fclose(file);
	}
	return high_score;
}

/**
 * Overview:
 * 
 * I/O instruction function write an unsigned integer in the file that get its relative path start from `HOME` directory.
 *
 * Warning:
 * 
 * If the relative path does not start from `HOME` directory the file will not recognized.
 * @param relative_path const character pointer for the relative path starting from `HOME` directory.
 */
void save_high_score(const char *relative_path, unsigned int new_high_score) {
	const char *home_dir = getenv("HOME");
	char absolute_path[1024];
	snprintf(absolute_path, sizeof(absolute_path), "%s/%s", home_dir, relative_path);

	FILE *file = fopen(absolute_path, "wb");
	if (file) {
		fwrite(&new_high_score, sizeof(unsigned int), 1, file);
		fclose(file);
	}
}