#include <stdio.h>
int copy_file(const char *to, const char *from)
{
	char buf[4096];
	ssize_t size_of_chunk;
	
	FILE *fp_from = fopen(from,"rb");
	if (fp_from == 0) {
		return -1;
	}
	
	FILE *fp_to = fopen(to,"wb");
	if (fp_to == 0) {
		fclose(fp_from);
		return -1;
	}
	
	while ((size_of_chunk = fread(buf,1,sizeof(buf),fp_from)) > 0) {
		fwrite(buf,1,size_of_chunk,fp_to);
	}
	fclose(fp_from);
	fclose(fp_to);
	
	return 0;
}