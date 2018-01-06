#include "minicrt.h"

//实现堆

typedef struct _heap_header
{
	enum {
		HEAP_BLOCK_FREE = 0xABABABAB,	//堆的标志 空闲
		HEAP_BLOCK_USED = 0xCDCDCDCD,	//		   使用
	}type;
	unsigned size;						//块大小包括头部
	struct _heap_header *next;
	struct _heap_header *prev;			//结构体往下就是数据		
}heap_header;

//移动地址
#define ADDR_ADD(a,o) (((char*)(a))+o)	
//堆结构体大小
#define HEADER_SIZE (sizeof(heap_header))


static heap_header * list_head = NULL;


void free(void* ptr)
{
	heap_header *header = (heap_header*)ADDR_ADD(ptr, -HEADER_SIZE);
	//如果这个堆没有使用，那么不进行释放
	if (header->type != HEAP_BLOCK_USED)
		return;
	
	//设置这个节点为空闲状态
	header->type = HEAP_BLOCK_FREE;

	//前一个节点不为空（头节点），同时也是空闲状态    （将当前节点融入前一个节点中）
	if (header->prev != NULL  && header->prev->type == HEAP_BLOCK_FREE) {
		//跳过这个节点
		header->prev->next = header->next;
		//检查下个节点是否为空，不为空将链表链接上
		if (header->next != NULL)
			header->next->prev = header->prev;
		//合并释放的大小到前一个节点中
		header->prev->size += header->size;
		
		//赋值新融入的节点（释放节点的前一个节点）
		header = header->prev;
	}
	//后一个节点不为空，类型等于空闲，可以融入后一节点中
	if (header->next != NULL&&header->next->type == HEAP_BLOCK_FREE) {
		header->size += header->next->size;
		header->next = header->next->next;
	}
}

void *malloc(unsigned size)
{
	heap_header *header;
	if (size == 0)
		return NULL;

	header = list_head;
	while (header != 0) {
		//找到一个空闲的节点
		if (header->type == HEAP_BLOCK_USED) {
			header = header->next;
			continue;
		}
		//申请的空间在原有的空间上进行查找，如果满足空间大小就给它申请
		if (header->size > size + HEADER_SIZE&&
			header->size <= size + HEADER_SIZE * 2)
			//将这个空间置为使用
			header->type = HEAP_BLOCK_USED;

		//如果这个空间大于 申请的空间大小 + 两个头堆空间头部大小  
		//那么将这个空间分割成两个空间，链入节点当中
		if (header->size > size + HEADER_SIZE * 2) {
			//分割下一个节点的位置
			heap_header *next = (heap_header*)ADDR_ADD(header, size + HEADER_SIZE);
			//将下一个节点的 前节点赋值成 申请的节点（分割后连入堆节点））

			//next相当于 申请空间 后面的节点
			next->prev = header;
			next->next = header->next;
			next->type = HEAP_BLOCK_FREE;
			//next的大小 =  总的大小 - 申请的大小 - 堆头部大小
			next->size = header->size - (size - HEADER_SIZE);

			//申请的堆结构
			header->next = next;
			header->size = size + HEADER_SIZE;
			header->type = HEAP_BLOCK_USED;
			//返回这段空间的位置
			return ADDR_ADD(header, HEADER_SIZE);
		}
		//搜寻下一个节点
		header = header->next;
	}
		return NULL;
}

#ifndef WIN32
	//Linux brk system call
	static int brk(void *end_data_segment) {
		int ret = 0;
		//brk system call number 45
		//in /usr/include/asm-i386/unistd.h
		//#define _NR_brk 45
		asm("movl $45, %%eax \n\t"
			"movl %1, %%ebx		\n\t"
			"int $0x80			\n\t"
			"movl %%eax,%0		\n\t"
			:"=r"(ret) : "m"(end_data_segment));
	}

#endif // !WIN32 brk Linux

#ifdef WIN32
#include <windows.h>
#endif // WIN32  virtualAlloc

int mini_crt_heap_init()
{
	void * base = NULL;
	heap_header * header = NULL;

	//32MB heap size
	unsigned heap_size = 1024 * 1024 * 32;

#ifdef WIN32
	base = VirtualAlloc(0, heap_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (base == NULL)
		return 0;

#else
	base = (void*)brk(0);
	void* end = ADDR_ADD(base, heap_size);
	end = (void*)brk(end);
	if (!end)
		return 0;
#endif // WIN32
	header = (heap_header *)base;
	header->size = heap_size;
	header->type = HEAP_BLOCK_FREE;
	header->next = NULL;
	header->prev = NULL;
	
	list_head = header;
	return 1;
}







