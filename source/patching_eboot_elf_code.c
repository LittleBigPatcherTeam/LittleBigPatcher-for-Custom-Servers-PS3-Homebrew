#include <stdio.h>
#include <string.h>
#include <ppu-types.h>
/* #include <stdlib.h>
typedef unsigned char bool;
typedef unsigned char u8; */

#include "patching_eboot_elf_code.h"

#define SEARCHING_BUFFER_SIZE 4096

// any other digests shall be added here for refresh or normalise_digest
const char replace_digests[4][MAX_DIGEST_LEN_INCL_NULL] = {"!?/*hjk7duOZ1f@daX","$ghj3rLl2e5E28@~[!","9yF*A&L#5i3q@9|&*F","CustomServerDigest"};


/*https://stackoverflow.com/questions/13450809/how-to-search-a-string-in-a-char-array-in-c
#include <stdio.h>
int main()
{
    char c_to_search[5] = "asdf";
*/
bool strstr_with_nulls_in_it(const u8 *chunk_to_check, int text_size, const u8 *to_search, int c_to_search_size,
bool (*further_patching)(int,int,const char*,FILE*,int),
int buffer_start_offset, FILE *fp, const char *url,int biggest_possible_size
)
{
	bool found_a_match = 0;
	int pos_search = 0;
    int pos_text = 0;
    int len_search = c_to_search_size;
    int len_text = text_size+c_to_search_size;
    for (pos_text = 0; pos_text < len_text - len_search;++pos_text)
    {
        if(chunk_to_check[pos_text] == to_search[pos_search])
        {
            ++pos_search;
            if(pos_search == len_search)
            {
                // match	
				if (further_patching((pos_text-len_search)+1,buffer_start_offset,url,fp,biggest_possible_size)) {
					found_a_match = 1;
				}
                
            }
        }
        else
        {
           pos_text -=pos_search;
           pos_search = 0;
        }
    }

   return found_a_match;
}

/*
https://stackoverflow.com/questions/67010836/conversion-of-string-endswith-method-into-c
bool endswith( const char *s1, const char *s2 )
{
    size_t n1 = strlen( s1 );
*/
bool str_endswith( const char *s1, const char *s2 )
{
    size_t n1 = strlen( s1 );
    size_t n2 = strlen( s2 );
    
    return ( n2 == 0 ) || ( !( n1 < n2 ) && memcmp( s1 + n1 - n2, s2, n2 ) == 0 );
}


bool further_checking_after_found_http_str(int http_offset, int buffer_start_offset, const char *url, FILE *fp, int biggest_possible_size)
{
	char *null_fill;
	char checking_value[BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL];
	int i;
	int checking_value_size;
	
	fseek(fp,buffer_start_offset+http_offset,SEEK_SET);
	
	checking_value_size = fread(checking_value,1,BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL,fp);
	
	
	// assuming that checking_value will not start with a null byte

	int last_occurance_of_null = 0;
	for (i = 0; i < checking_value_size; i++) {
		if (last_occurance_of_null != 0) {
			if (checking_value[i] != 0) {
				break;
			}
		}
		if (checking_value[i] == 0) {
			last_occurance_of_null = i;
			break;
		}
	}
	if (last_occurance_of_null == 0) {
		return 0;
	}
	last_occurance_of_null++;
	
	checking_value_size = strlen(checking_value);
	// the lenght of the url is checked for in the main.c, but maybe this eboot doesnt support this long of url? - 1 for strlen not include null
	if ((strlen(url) - 1) > last_occurance_of_null) {
		return 0;
	}
	// now we can treat checking_value like a normal string
	if (str_endswith(checking_value,"/LITTLEBIGPLANETPS3_XML") || str_endswith(checking_value,"/LITTLEBIGPLANETPSP_XML")) {
		fseek(fp,buffer_start_offset+http_offset,SEEK_SET);
		
		null_fill = malloc(last_occurance_of_null);
		memset(null_fill,0,last_occurance_of_null);
		fwrite(null_fill,1,last_occurance_of_null,fp);
		free(null_fill);
		
		fseek(fp,buffer_start_offset+http_offset,SEEK_SET);
		fwrite(url,1,strlen(url),fp);
		return 1;
	}
	return 0;
}

bool further_patching_for_digest(int digest_offset, int buffer_start_offset, const char *digest, FILE *fp, int biggest_possible_size)
{
	char *null_fill;
	char checking_value[biggest_possible_size];
	int i;
	int checking_value_size;
	
	fseek(fp,buffer_start_offset+digest_offset,SEEK_SET);
	
	checking_value_size = fread(checking_value,1,biggest_possible_size,fp);
	
	// assuming that checking_value will not start with a null byte

	int last_occurance_of_null = 0;
	for (i = 0; i < checking_value_size; i++) {
		if (last_occurance_of_null != 0) {
			if (checking_value[i] != 0) {
				break;
			}
		}
		if (checking_value[i] == 0) {
			last_occurance_of_null = i;
			break;
		}
	}
	if (last_occurance_of_null == 0) {
		return 0;
	}
	last_occurance_of_null++;
	//dbglogger_log("last_occurance_of_null = %d",last_occurance_of_null);
	checking_value_size = strlen(checking_value);

	// the lenght of the digest is checked for in the main.c, but maybe this eboot doesnt support this long of digest? - 1 for strlen not include null
	if ((strlen(digest) - 1) > last_occurance_of_null) {
		return 0;
	}
	// now we can treat checking_value like a normal string
	
	fseek(fp,buffer_start_offset+digest_offset,SEEK_SET);
	
	null_fill = malloc(last_occurance_of_null);
	memset(null_fill,0,last_occurance_of_null);
	fwrite(null_fill,1,last_occurance_of_null,fp);
	free(null_fill);
	
	fseek(fp,buffer_start_offset+digest_offset,SEEK_SET);
	fwrite(digest,1,strlen(digest),fp);
	return 1;
}


int patch_eboot_elf_main_series(const char *eboot_elf_path, const char *url, const char *digest, bool normalise_digest)
{
	u8 searching_buffer[SEARCHING_BUFFER_SIZE + BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL];
	
	int buffer_start_offset;
	int buffer_size;
	int trailing_buffer_size;
	int full_buffer_trailing_buffer_size;
	bool found_a_match;
	char todo_digest[MAX_DIGEST_LEN_INCL_NULL] = {0};
	
	FILE *fp = fopen(eboot_elf_path,"rb+");
	if (fp == 0) {
		return PATCH_ERR_EBOOT_ELF_NO_EXISTS;
	}
	
	const u8 http_str[4] = "http";
	found_a_match = 0;
	while ((buffer_size = fread(searching_buffer,1,SEARCHING_BUFFER_SIZE,fp)) > 0) {
		buffer_start_offset = ftell(fp) - buffer_size;
		
		// reads extra data after the inital chunk read, so that it will find urls overlaping in the chunk sizes
		trailing_buffer_size = fread(searching_buffer+buffer_size,1,BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL,fp);	
		
		// seeks backwards so the next chunk will include this
		fseek(fp,-trailing_buffer_size,SEEK_CUR);
		
		// calcing the actual buffer size
		full_buffer_trailing_buffer_size = buffer_size + trailing_buffer_size;
		
		/* the following code is based of the regex in union patcher
		https://github.com/LBPUnion/UnionPatcher/blob/c45b9ec37eedade40490a1c000311b099ed71f31/UnionPatcher/Patcher.cs#L53
		MatchCollection urls = Regex.Matches(dataAsString, "http?[^\x00]*?LITTLEBIGPLANETPS(3|P)_XML\x00*");
		*/
		if (strstr_with_nulls_in_it(searching_buffer,
									full_buffer_trailing_buffer_size,
									http_str,
									sizeof(http_str),
									further_checking_after_found_http_str,
									buffer_start_offset,
									fp,
									url,
									BIGGEST_POSSIBLE_URL_IN_EBOOT_INCL_NULL)) {found_a_match = 1;}
	}
	fclose(fp);
	
	if (!found_a_match) {
		return PATCH_ERR_NO_URLS_FOUND;
	}


	if ((digest[0] == 0) && !normalise_digest) {
		return 0;
	}
	
	if (digest[0] != 0) {
		strcpy(todo_digest,digest);
	}
	
	else if (normalise_digest) {
		strcpy(todo_digest,"!?/*hjk7duOZ1f@daX");
	}
	
	else {
		return PATCH_ERR_THIS_SHOULD_NEVER_HAPPEN;
	}
	
	// digest patching

	u8 searching_buffer_for_digest[SEARCHING_BUFFER_SIZE + BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL];
	
	for (int i = 0; i < sizeof(replace_digests) / sizeof(replace_digests[0]); i++) {
		u8 *replace_digest = replace_digests[i];
		found_a_match = 0;
		FILE *fp_for_digest = fopen(eboot_elf_path,"rb+");
		if (fp_for_digest == 0) {
			return PATCH_ERR_EBOOT_ELF_NO_EXISTS;
		}
		while ((buffer_size = fread(searching_buffer_for_digest,1,SEARCHING_BUFFER_SIZE,fp_for_digest)) > 0) {
			buffer_start_offset = ftell(fp_for_digest) - buffer_size;
			
			// reads extra data after the inital chunk read, so that it will find urls overlaping in the chunk sizes
			trailing_buffer_size = fread(searching_buffer_for_digest+buffer_size,1,BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL,fp_for_digest);	
			
			// seeks backwards so the next chunk will include this
			fseek(fp_for_digest,-trailing_buffer_size,SEEK_CUR);
			
			// calcing the actual buffer size
			full_buffer_trailing_buffer_size = buffer_size + trailing_buffer_size;

			if (strstr_with_nulls_in_it(searching_buffer_for_digest,
										full_buffer_trailing_buffer_size,
										replace_digest,
										sizeof(replace_digests[0]),
										further_patching_for_digest,
										buffer_start_offset,
										fp_for_digest,
										todo_digest,
										BIGGEST_POSSIBLE_DIGEST_IN_EBOOT_INCL_NULL)){found_a_match = 1;}
		}
		fclose(fp_for_digest);
		
		if (found_a_match) {
			return 0; // there wont be mutiple digests in a eboot
		}
	}

	if (todo_digest[0] != 0) {
		return PATCH_ERR_NO_DIGESTS_FOUND;
	}

	return 0;
}

int patch_eboot_elf_karting(const char *eboot_elf_path, const char *url, const char *digest, bool normalise_digest)
{
	u8 searching_buffer[SEARCHING_BUFFER_SIZE + BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL];
	
	int buffer_start_offset;
	int buffer_size;
	int trailing_buffer_size;
	int full_buffer_trailing_buffer_size;
	bool found_a_match;

	const u8 replace_url[] = "lbpk.ps3.online.scea.com";
	found_a_match = 0;
	FILE *fp = fopen(eboot_elf_path,"rb+");
	if (fp == 0) {
		return PATCH_ERR_EBOOT_ELF_NO_EXISTS;
	}
	while ((buffer_size = fread(searching_buffer,1,SEARCHING_BUFFER_SIZE,fp)) > 0) {
		buffer_start_offset = ftell(fp) - buffer_size;
		
		// reads extra data after the inital chunk read, so that it will find urls overlaping in the chunk sizes
		trailing_buffer_size = fread(searching_buffer+buffer_size,1,BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL,fp);	
		
		// seeks backwards so the next chunk will include this
		fseek(fp,-trailing_buffer_size,SEEK_CUR);
		
		// calcing the actual buffer size
		full_buffer_trailing_buffer_size = buffer_size + trailing_buffer_size;

		if (strstr_with_nulls_in_it(searching_buffer,
									full_buffer_trailing_buffer_size,
									replace_url,
									sizeof(replace_url),
									further_patching_for_digest,
									buffer_start_offset,
									fp,
									url,
									BIGGEST_POSSIBLE_URL_IN_LBPK_EBOOT_INCL_NULL)){found_a_match = 1;}
	}
	fclose(fp);
	
	if (found_a_match) {
		return 0;
	}
	
	return PATCH_ERR_NO_DIGESTS_FOUND;
}