#ifndef HTTP_H_   /* Include guard */
#define HTTP_H_

#define HTTP_YES		1
#define HTTP_NO 		0
#define HTTP_SUCCESS 	1
#define HTTP_FAILED	 	0

#define HTTP_ALLOW_SMALLER_BUFFER_SIZE 0
#define HTTP_ERROR_IF_SMALLER_BUFFER_SIZE 1

int http_init(void);

void http_end(void);

int http_download(const char* url, const char* filename, const char* local_dst);

int http_download_to_buffer(const char* url, const char* filename, void * out_buffer, uint64_t out_buffer_size, bool allow_smaller_buffer_size);


#endif // HTTP_H_