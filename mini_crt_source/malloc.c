#include "minicrt.h"

//ʵ�ֶ�

typedef struct _heap_header
{
	enum {
		HEAP_BLOCK_FREE = 0xABABABAB,	//�ѵı�־ ����
		HEAP_BLOCK_USED = 0xCDCDCDCD,	//		   ʹ��
	}type;
	unsigned size;						//���С����ͷ��
	struct _heap_header *next;
	struct _heap_header *prev;			//�ṹ�����¾�������		
}heap_header;

//�ƶ���ַ
#define ADDR_ADD(a,o) (((char*)(a))+o)	
//�ѽṹ���С
#define HEADER_SIZE (sizeof(heap_header))


static heap_header * list_head = NULL;


void free(void* ptr)
{
	heap_header *header = (heap_header*)ADDR_ADD(ptr, -HEADER_SIZE);
	//��������û��ʹ�ã���ô�������ͷ�
	if (header->type != HEAP_BLOCK_USED)
		return;
	
	//��������ڵ�Ϊ����״̬
	header->type = HEAP_BLOCK_FREE;

	//ǰһ���ڵ㲻Ϊ�գ�ͷ�ڵ㣩��ͬʱҲ�ǿ���״̬    ������ǰ�ڵ�����ǰһ���ڵ��У�
	if (header->prev != NULL  && header->prev->type == HEAP_BLOCK_FREE) {
		//��������ڵ�
		header->prev->next = header->next;
		//����¸��ڵ��Ƿ�Ϊ�գ���Ϊ�ս�����������
		if (header->next != NULL)
			header->next->prev = header->prev;
		//�ϲ��ͷŵĴ�С��ǰһ���ڵ���
		header->prev->size += header->size;
		
		//��ֵ������Ľڵ㣨�ͷŽڵ��ǰһ���ڵ㣩
		header = header->prev;
	}
	//��һ���ڵ㲻Ϊ�գ����͵��ڿ��У����������һ�ڵ���
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
		//�ҵ�һ�����еĽڵ�
		if (header->type == HEAP_BLOCK_USED) {
			header = header->next;
			continue;
		}
		//����Ŀռ���ԭ�еĿռ��Ͻ��в��ң��������ռ��С�͸�������
		if (header->size > size + HEADER_SIZE&&
			header->size <= size + HEADER_SIZE * 2)
			//������ռ���Ϊʹ��
			header->type = HEAP_BLOCK_USED;

		//�������ռ���� ����Ŀռ��С + ����ͷ�ѿռ�ͷ����С  
		//��ô������ռ�ָ�������ռ䣬����ڵ㵱��
		if (header->size > size + HEADER_SIZE * 2) {
			//�ָ���һ���ڵ��λ��
			heap_header *next = (heap_header*)ADDR_ADD(header, size + HEADER_SIZE);
			//����һ���ڵ�� ǰ�ڵ㸳ֵ�� ����Ľڵ㣨�ָ������ѽڵ㣩��

			//next�൱�� ����ռ� ����Ľڵ�
			next->prev = header;
			next->next = header->next;
			next->type = HEAP_BLOCK_FREE;
			//next�Ĵ�С =  �ܵĴ�С - ����Ĵ�С - ��ͷ����С
			next->size = header->size - (size - HEADER_SIZE);

			//����Ķѽṹ
			header->next = next;
			header->size = size + HEADER_SIZE;
			header->type = HEAP_BLOCK_USED;
			//������οռ��λ��
			return ADDR_ADD(header, HEADER_SIZE);
		}
		//��Ѱ��һ���ڵ�
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







