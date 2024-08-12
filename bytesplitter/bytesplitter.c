#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#define FSEEK_SUCCESS 0
#define FOPEN_SUCCESS 0
#define byte unsigned char
#define MAX_BUFFER_LENGTH 10485760

void usage();
FILE *file_io_read_start(char *filepath);
void file_io_read_end(FILE **f);
FILE *file_io_write_start(char *filepath);
void file_io_write_end(FILE **f);
size_t copy_stream_to_buffer(FILE *f, byte *buffer);
size_t copy_buffer_to_stream(byte *buffer, FILE *f, size_t bytes_to_write);
byte *build_buffer();
void destroy_buffer(byte **buffer);
void zero_out_buffer(byte *buffer);
char * make_segment_file_path(char *file_path, int segment);
void free_segment_file_path(char **segment_file_path);
char * make_filename_segment_suffix(int segment);
char * get_filename_from_path(char * file_path);
int measure_number_length(const int number);

int main(int argc, char **argv)
{
	FILE * file_for_reading = NULL;
	FILE * file_for_writing = NULL;
    char * file_path = NULL;
    char * segment_file_path = NULL;
    byte * buffer = NULL;
    signed long long total_bytes_red = 0;
    signed long long bytes_red = 0;
    int feof_output = 0;
    int x = 0;
    int i = 0;

	if (argc < 1)
	{
		usage();
		exit(6);
	}

    file_path = argv[1];
    buffer = build_buffer();
    file_for_reading = file_io_read_start(file_path);

    while (1==1)
    {
        bytes_red = copy_stream_to_buffer(file_for_reading, buffer);

        if ((0 != (feof_output = feof(file_for_reading))) || (0 == (i % 4)))
        {
            if (0 != feof_output && bytes_red > 0)
            {
                copy_buffer_to_stream(buffer, file_for_writing, bytes_red);
            }
            if ((0 != feof_output) || (0 == (i % 4)))
            {
                file_io_write_end(&file_for_writing);
                free_segment_file_path(&segment_file_path);
            }
        	if (0 != feof_output)
        	{
        		break;
        	}
        	x = (x + 1);
        	segment_file_path = make_segment_file_path(file_path, x);
        	file_for_writing = file_io_write_start(segment_file_path);
        }
        copy_buffer_to_stream(buffer, file_for_writing, bytes_red);
        zero_out_buffer(buffer);
        i = (i + 1);
    }
    file_io_write_end(&file_for_writing);
    file_io_read_end(&file_for_reading);
    free_segment_file_path(&segment_file_path);
    destroy_buffer(&buffer);
    return;
}

void usage()
{
	printf("bytesplitter takes 1 parameter, file path");
}

char * make_segment_file_path(char * file_path, int segment)
{
    //char * segment_suffix = make_filename_segment_suffix(segment);
    //char * file_name = get_filename_from_path(file_path);
    size_t segment_filename_length = strlen(file_path) + measure_number_length(segment) + 2;
    char * segment_filename = (char *)malloc(segment_filename_length);
    sprintf_s(segment_filename, segment_filename_length, "%s_%i", file_path, segment);
    return segment_filename;
}

int measure_number_length(const int number)
{
    int number_length = 0;

    if (number < 0) 
    {
        return 0;
    }
    while (number >= (int)pow((double)10, (double)number_length))
    {
    	number_length = (number_length + 1);
    }

    return number_length;
}

char * make_filename_segment_suffix(int segment)
{
	char * segment_suffix = NULL;
	int segment_suffix_length = log10(segment);
	if (segment == pow(10, segment_suffix_length))
	{
		segment_suffix_length = segment_suffix_length + 1;
	}
	segment_suffix_length = segment_suffix_length + 2;
	segment_suffix = (char *)malloc(segment_suffix_length * sizeof(char));
    sprintf_s(segment_suffix, segment_suffix_length, "_%i", segment);

    return segment_suffix;
}

char * get_filename_from_path(char * file_path)
{
    int offset = strlen(file_path);
    if (offset <= 0)
    {
    	return file_path;
    }
    offset = (offset - 1);
    while (offset >= 0)
    {
    	if (('\\' == (char)*(file_path + offset))
    		|| ('/' == (char)*(file_path + offset)))
    	{
    		break;
    	}
    	offset = (offset - 1);
    }
    return file_path + offset;
}

FILE * file_io_write_start(char *filepath)
{
    errno_t errno_n = 0;
	FILE * f = NULL;
	int fseek_result = 0;

	if(FOPEN_SUCCESS != (errno_n = fopen_s(&f, filepath, "wbS+")))
	{
		printf("File %s could not be opened for writing", filepath);
		exit(1);
	}
	if(FSEEK_SUCCESS != (fseek_result = fseek(f, 0L, SEEK_SET)))
	{
		fclose(f);
        printf("Attempt to reset position to start of file on file %s resulted in error %i", filepath, fseek_result);
        exit(2);
	}
    return f;
}

FILE * file_io_read_start(char *filepath)
{
    errno_t errno_n = 0;
	FILE * f = NULL;
	int fseek_result = 0;

	if(FOPEN_SUCCESS != (errno_n = fopen_s(&f, filepath, "rbS")))
	{
		printf("File %s could not be opened for reading", filepath);
		exit(1);
	}
	if(FSEEK_SUCCESS != (fseek_result = fseek(f, 0L, SEEK_SET)))
	{
		fclose(f);
        printf("Attempt to reset position to start of file on file %s resulted in error %i", filepath, fseek_result);
        exit(2);
	}
    return f;
}

void file_io_write_end(FILE **f)
{
    if (NULL == *f)
    {
        return;
    }
    fclose(*f);
    *f = NULL;
}

void file_io_read_end(FILE **f)
{
    if (NULL == *f)
    {
        return;
    }
	fclose(*f);
    *f = NULL;
}

size_t copy_buffer_to_stream(byte *buffer, FILE *f, size_t bytes_to_write)
{
	size_t bytes_written = 0;

    if(NULL == buffer)
    {
    	printf("Function copystreamtobuffer parameter buffer is NULL. Exiting");
    	exit(4);
    }

    if(NULL == f)
    {
    	printf("Function copystreamtobuffer parameter FILE is NULL. Exiting");
    	exit(3);
    }

    /* write one buffer at a time */
    bytes_written = fwrite(buffer, 1, bytes_to_write, f);

    return bytes_written;
}

size_t copy_stream_to_buffer(FILE *f, byte *buffer)
{
	size_t bytes_red = 0;

    if(NULL == f)
    {
    	printf("Function copystreamtobuffer parameter FILE is NULL. Exiting");
    	exit(3);
    }

    if(NULL == buffer)
    {
    	printf("Function copystreamtobuffer parameter buffer is NULL. Exiting");
    	exit(4);
    }

    /* read one buffer at a time */
    bytes_red = fread_s(buffer, MAX_BUFFER_LENGTH, 1, MAX_BUFFER_LENGTH, f);

    return bytes_red;
}

byte * build_buffer()
{
	byte * buffer = NULL;

	if (NULL == (buffer = (byte *)malloc(MAX_BUFFER_LENGTH)))
	{
		exit(5);
	}
	memset(buffer, (byte)0, MAX_BUFFER_LENGTH);
	return buffer;
}

void zero_out_buffer(byte * buffer)
{
     memset(buffer, 0, MAX_BUFFER_LENGTH);
}

void free_segment_file_path(char **segment_file_path)
{
    if (NULL == *segment_file_path)
    {
        return;
    }
    free(*segment_file_path);
    *segment_file_path = NULL;
}

void destroy_buffer(byte **buffer)
{
	if (NULL == *buffer)
	{
		return;
	}
	free(*buffer);
	*buffer = NULL;
}