#pragma once

#ifndef __MINI_CRT_H___
#define __MINI_CRT_H___

#ifdef __cplusplus
extern "C"{
#endif


//malloc
#ifndef NULL
#define  NULL (0)
#endif // !NULL

void free(void* ptr);
void *malloc(unsigned size);
static int brk(void *end_data_segment);
int mini_crt_heap_init();

//字符串
char * itoa(int n, char* str, int radix);
int strcmp(const char*src, const char *dst);
char *strcpy(char * dest, const char *src);
unsigned strlen(const char *str);

//文件io
typedef int FILE;
#define EOF (-1)

#ifdef WIN32
#include <windows.h>
#define  stdin  ((FILE*)(GetStdHandle(STD_INPUT_HANDLE)))
#define  stdout ((FILE*)(GetStdHandle(STD_OUTPUT_HANDLE)))
#define  stderr ((FILE*)(GetStdHandle(STD_ERROR_HANDLE)))
#else
#define  stdin		((FILE*)0)
#define  stdout		((FILE*)1)
#define  stderr		((FILE*)2)
#endif // WIN32

int mini_crt_io_init();
FILE * fopen(const char *filename, const char *mode);
int fread(char* buffer, int size, int count, FILE* stream);
int fwrite(char* buffer, int size, int count, FILE* stream);
int fclose(FILE *fp);
int fseek(FILE *fp, int offset, int set);


//format 格式控制
int fputs(const char * str, FILE *stream);
int fputc(int c, FILE *stream);
int printf(const char *format, ...);
int fprintf(FILE* Stream, char const *format, ...);


//internal
void do_global_ctors();
void mini_crt_call_exit_coutine();

//atexit
typedef void(*atexit_func_t)(void);
int atexit(atexit_func_t func);

#ifdef __cplusplus
}
#endif

#endif // !__MINI_CRT_H___
