/*
 * circular_buffer.h
 *
 *  Created on: Feb 6, 2015
 *      Author: efe
 */

// from Wikipedia
//
/* Circular buffer example, keeps one slot open */

// WARNING: DO INCLUDE SYSCALLS.C

#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H


#define CB_SUCCESS 		1
#define CB_FAIL_LOCKED  -1
#define CB_FAIL_EMPTY	-2
#define CB_FAIL_FULL 	-3

#define CB_TRUE			1
#define CB_FALSE		-1

#define CB_OVERWRITE	CB_FALSE

#define BUFFERSIZE		128

typedef int CBSuccessType;

/* Opaque buffer element type.  This would be defined by the application. */
typedef struct { char value; } ElemType;
//typedef char ElemType;

/*ElemType *new_ElemType(int size);*/

// TODO: Bu buraya olmadý ama þimdilik buraya koyduk. Baþka bir kütüphaneye taþýnmalý
typedef struct {
	char message[256];
	int lenght;
}cca_comm_prot_packet_str;


/* Circular buffer object */
typedef struct CircularBuffer CircularBuffer;
struct CircularBuffer{
	int         size;   /* maximum number of elements           */
	int 		__size; /* real size of buffer (size + 1) */
	int         start;  /* index of oldest element              */
	int         end;    /* index at which to write new element  */
	ElemType   elems[BUFFERSIZE];  /* vector of elements                   */
	int 	start_point_locked; /* using on atomic writes and reads     */
	int 	end_point_locked; /* using on atomic writes and reads     */
	int 	exceptions; /* other than zero indicates an error */

	CBSuccessType (*append)(CircularBuffer *, ElemType *); /* append to end */
	CBSuccessType (*pop0)(CircularBuffer *, ElemType *); /* pop from beginning */
	//void (*del)(CircularBuffer *); /* delete an CircularBuffer object */
	int (*len)(CircularBuffer *);
	cca_comm_prot_packet_str (*read_str)(CircularBuffer *cb, int offset, int count);

	int (*mv_elems_to)(CircularBuffer *src, CircularBuffer *dest, int count);
	int (*cp_elems_to)(CircularBuffer *src, CircularBuffer *dest, int count);


	CBSuccessType (*read)(CircularBuffer *cb, ElemType *elem, int offset);
	void (*erase0)( CircularBuffer *cb, int count);

};


void cbInit(CircularBuffer *cb, int size);
//void cbFree(CircularBuffer *cb);
int cbIsFull(CircularBuffer *cb);
int cbIsEmpty(CircularBuffer *cb);
CBSuccessType cbWrite(CircularBuffer *cb, ElemType *elem);
CBSuccessType cbRead(CircularBuffer *cb, ElemType *elem);

//CircularBuffer *new_CircularBuffer(int size);
int cbCalcElementCount(CircularBuffer *cb);
cca_comm_prot_packet_str cbReadWithIndex(CircularBuffer *, int, int);

int cb_mv_elems_to(CircularBuffer *, CircularBuffer *, int);
int cb_cp_elems_to(CircularBuffer *, CircularBuffer *, int);

CBSuccessType cbRead_only(CircularBuffer *cb, ElemType *elem, int offset);

void cb_erase0(CircularBuffer *, int);

#endif // __CIRCULAR_BUFFER_H



