/*
 * circular_buffer.c
 *
 *  Created on: Feb 6, 2015
 *      Author: efe
 */

#include "circular_buffer.h"

#define CB_EXCEPTION_MEMORY_COULD_NOT_ALLOCATED 1
#define CB_EXCEPTION_NO_ERROR 0

/*ElemType *new_ElemType(int size){
	ElemType *self = (ElemType *)calloc(size, sizeof(ElemType));
	return self;
}*/

void cbInit(CircularBuffer *cb, int size)
{
	cb->exceptions = CB_EXCEPTION_NO_ERROR;

	cb->size = BUFFERSIZE;
	cb->__size  = cb->size + 1; /* include empty elem */
	cb->start = 0;
	cb->end   = 0;
	//cb->elems = (ElemType *)calloc(cb->__size, sizeof(ElemType));
	/*cb->elems = new_ElemType(cb->__size);
	if (cb->elems == NULL){
		cb->exceptions = CB_EXCEPTION_MEMORY_COULD_NOT_ALLOCATED;
	}*/
	cb->start_point_locked = CB_FALSE;
	cb->end_point_locked = CB_FALSE;

	cb->append = &cbWrite;
	cb->pop0 = &cbRead;
	//cb->del = &cbFree;
	cb->len = &cbCalcElementCount;
	cb->read_str = &cbReadWithIndex;
	cb->mv_elems_to = &cb_mv_elems_to;
	cb->cp_elems_to = &cb_cp_elems_to;
	cb->read = &cbRead_only;
	cb->erase0 = &cb_erase0;
}


/*CircularBuffer *new_CircularBuffer(int size){
	CircularBuffer *self = (CircularBuffer *) malloc(sizeof(CircularBuffer));

	if( self == NULL ){
		return NULL;
	}

	cbInit(self, size);
	return self;
}*/


/*void cbFree(CircularBuffer *cb) {
	free(cb->elems);  OK if null
	free(cb); // TODO: volatile causes problem http://stackoverflow.com/a/14152483
}*/



int cbIsFull(CircularBuffer *cb) {
	return (cb->end + 1) % cb->__size == cb->start; }

int cbIsEmpty(CircularBuffer *cb) {
	return cb->end == cb->start; }

/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking cbIsFull(). */
CBSuccessType cbWrite(CircularBuffer *cb, ElemType *elem) {
	if (cb->end_point_locked == CB_FALSE)
	{
		if(cbIsFull(cb))
		{
			return CB_FAIL_FULL;
		}
		else
		{
			// lock the buffer in order to prevent conflicts
			cb->end_point_locked = CB_TRUE;

			//copy all element struct
			cb->elems[cb->end] = *elem;
			cb->end = (cb->end + 1) % cb->__size;
			if (cb->end == cb->start)
				cb->start = (cb->start + 1) % cb->__size; /* full, overwrite */

			// release lock
			cb->end_point_locked = CB_FALSE;
			return CB_SUCCESS;
		}
	}
	else
	{
		// another process running write operation on the buffer
		return CB_FAIL_LOCKED;
	}

}

/* Read and erase oldest element.  */
CBSuccessType cbRead(CircularBuffer *cb, ElemType *elem) {
	if (cb->start_point_locked == CB_FALSE)
	{
		if(cbIsEmpty(cb))
		{
			return CB_FAIL_EMPTY;
		}
		else
		{
			// lock the buffer in order to prevent conflicts
			cb->start_point_locked = CB_TRUE;

			//copy all elem struct
			*elem = cb->elems[cb->start];
			cb_erase0(cb, 1);

			// release lock
			cb->start_point_locked = CB_FALSE;
			return CB_SUCCESS;
		}
	}
	else
	{
		// another process running write operation on the buffer
		return CB_FAIL_LOCKED;
	}
}

void cb_erase0(CircularBuffer *cb, int count){
	cb->start = (cb->start + count) % cb->__size;
}

/* Read oldest element. */
CBSuccessType cbRead_only(CircularBuffer *cb, ElemType *elem, int offset) {
	if (cb->start_point_locked == CB_FALSE)
	{
		if(cbIsEmpty(cb))
		{
			return CB_FAIL_EMPTY;
		}
		else
		{
			// lock the buffer in order to prevent conflicts
			cb->start_point_locked = CB_TRUE;

			offset = offset % cb->size;
			int read_index = (cb->start + offset) % cb->__size;
			*elem = cb->elems[read_index];

			// release lock
			cb->start_point_locked = CB_FALSE;
			return CB_SUCCESS;
		}
	}
	else
	{
		// another process running write operation on the buffer
		return CB_FAIL_LOCKED;
	}
}


int cbCalcElementCount(CircularBuffer *cb){
	int count = -1;

	if(cb->end >= cb->start){
		count = cb->end - cb->start;
	}
	else{
		count = cb->__size - (cb->start - cb->end);
	}

	return count;
}

cca_comm_prot_packet_str cbReadWithIndex(CircularBuffer *cb, int offset, int count){

	// TODO: LOCK WHILE READING

	// This function assumes that elements of the circular buffer has a
	// "value" member which has a type of char
	// count = 0 means return all elements
	int max_count = cbCalcElementCount(cb);
	if ((count <= 0) || (count > max_count)) {
		count = max_count;
	}

	cca_comm_prot_packet_str package;
	//package.message[] = "";
	package.lenght = count - offset;

	if (count > 0){

		char tmp[256] = "";
		// bellek taþmasýna neden oluyor ->  char *tmp = (char *) calloc(sizeof(char), (count + 1)); // "+1" for null at the end

		int i;
		int read_index;
		for (i = 0; i < count; i++){
			read_index = (cb->start + offset + i) % cb->__size;
			tmp[i] = cb->elems[read_index].value;
		}

		for(i=0; i < 256; i++){
			package.message[i] = tmp[i];
		}
	}


	return package;

}


int cb_mv_elems_to(CircularBuffer *src, CircularBuffer *dest, int count){
	/*
	ElemType __tmp;
	ElemType *tmp = &__tmp;

	if (dest->end_point_locked != CB_TRUE){
		dest->end_point_locked = CB_TRUE;

		if(dest->size < (dest->len(dest) + count)){

			while(i < count){
				  if( src->pop0(src, tmp) == CB_SUCCESS){
					  i = i + 1;
					  // TODO: something may go wrong here (i think)
					  dest->append(dest, tmp);
				  }else{
					  break;
				  }
			  }
		  dest->end_point_locked = CB_FALSE;
		}
	}
	 */
	int successfull_copies = cb_cp_elems_to(src, dest, count);
	cb_erase0(src, successfull_copies);

	return successfull_copies;
}

int cb_cp_elems_to(CircularBuffer *src, CircularBuffer *dest, int count){
	// Copy elements from start
	ElemType __tmp;
	ElemType *tmp = &__tmp;
	int i = 0;

	if(dest->size >= (cbCalcElementCount(dest) + count)){
		// count = -1 means "maximum possible"
		if (count == -1){
			count = cbCalcElementCount(src);
		}
		while( i < count){
			if( cbRead_only(src, tmp, i) == CB_SUCCESS){
				if(cbWrite(dest, tmp) == CB_SUCCESS){
					i = i + 1;
				}
				else {
					// can not write to destination ----- maybe error message ?
					break;
				}
			}
			else {
				// can not read from source
				break;
			}
		}
	}
	return i; // number of successfully written elements
}
