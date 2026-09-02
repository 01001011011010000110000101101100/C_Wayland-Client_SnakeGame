#ifndef IO
#define IO "io.file"

extern void save_high_score(const char *relative_path, unsigned int new_high_score);
extern unsigned int load_high_score(const char *relative_path);

#endif