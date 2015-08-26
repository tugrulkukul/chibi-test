/*
 * cca_comm_prot.c
 *
 *  Created on: Feb 10, 2015
 *      Author: efe
 */

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "cca_comm_prot.h"

/* Basic */

// Constructor
/*cca_comm_prot_protocol_basic *new_cca_comm_prot_protocol_basic(){
	cca_comm_prot_protocol_basic *self =
			(cca_comm_prot_protocol_basic *) malloc(
					sizeof(struct cca_comm_prot_protocol_basic));
	self->frame_start = SOH;
	self->frame_end = EOT;
	self->protocol_id = 0;
	return self;
}*/
//üstteki fonksiyon yerine bu gelecek, gördüğün yerde değiştir.
void init_protocol_basic(cca_comm_prot_protocol_basic *prot)
{
	prot->frame_start = SOH;
	prot->frame_end = EOT;
	prot->protocol_id = 0;
	return;
}

/* Protocol ID: 01 */

// Constructor
/*cca_comm_prot_protocol_id_01 *new_cca_comm_prot_protocol_id_01(uint32_t device_id){
	cca_comm_prot_protocol_id_01 *self =
			(cca_comm_prot_protocol_id_01 *) malloc(
					sizeof(cca_comm_prot_protocol_id_01));
	self->super = new_cca_comm_prot_protocol_basic();
	self->super->protocol_id = 1;
	self->super->self_id = device_id;

	self->mk_resp = &cca_comm_prot_protocol_id_01_mk_slave_response;
	self->get_req = &cca_comm_prot_protocol_id_01_get_master_request;
	self->del = &cca_comm_prot_protocol_id_01_del;
	self->calc_crc = &cca_comm_prot_protocol_calc_crc16;

	self->frame_size = 25;
	return self;
}*/

void init_protocol_01(cca_comm_prot_protocol_id_01 *prot, uint32_t device_id)
{
	init_protocol_basic(&(prot->super));
	prot->super.protocol_id = 1;
	prot->super.self_id = device_id;

	prot->mk_resp = &cca_comm_prot_protocol_id_01_mk_slave_response;
	prot->get_req = &cca_comm_prot_protocol_id_01_get_master_request;
	prot->del = &cca_comm_prot_protocol_id_01_del;
	prot->calc_crc = &cca_comm_prot_protocol_calc_crc16;

	prot->frame_size = 25;
	return ;
}

/*void cca_comm_prot_protocol_id_01_del(
		cca_comm_prot_protocol_id_01 *package){
	free(package->super);
	free(package);
}*/

#define BYTE_0_MASK 0xFF000000
#define BYTE_1_MASK 0x00FF0000
#define BYTE_2_MASK 0x0000FF00
#define BYTE_3_MASK 0x000000FF

#define GET_BYTE(VALUE, BYTE_INDEX) ((VALUE & (0xFF << (BYTE_INDEX * 8))) >> (BYTE_INDEX * 8))

#define MASK_WITH &


CircularBuffer * cca_comm_prot_protocol_id_01_mk_slave_response(
		cca_comm_prot_protocol_id_01 *package){

	CircularBuffer buffer;
	cbInit(&buffer,30); // max packet length of cca_comm_prot_protocol_id_01

	ElemType tmp;

	tmp.value = package->super.frame_start;
	cbWrite(&buffer, &tmp);

	tmp.value = package->super.protocol_id;
	cbWrite(&buffer, &tmp);

	// calc source and destination
	//package->dest = package->src;
	package->src = package->super.self_id;


	// Insert source
	tmp.value = GET_BYTE(package->src, 3);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->src, 2);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->src, 1);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->src, 0);
	cbWrite(&buffer, &tmp);


	// Insert destination
	tmp.value = GET_BYTE(package->dest, 3);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->dest, 2);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->dest, 1);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->dest, 0);
	cbWrite(&buffer, &tmp);

	// insert digital io
	tmp.value = GET_BYTE(package->digital_io, 3);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->digital_io, 2);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->digital_io, 1);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(package->digital_io, 0);
	cbWrite(&buffer, &tmp);

	// insert analog io
	int i;
	for(i=0; i < 4; i++)
	{
		tmp.value = GET_BYTE(package->analog_io[i], 1);
		cbWrite(&buffer, &tmp);

		tmp.value = GET_BYTE(package->analog_io[i], 0);
		cbWrite(&buffer, &tmp);
	}

	cca_comm_prot_packet_str packet_till_here = cbReadWithIndex(&buffer, 2, 0); // read only protocol data section

	crc packet_crc = package->calc_crc(packet_till_here.message, packet_till_here.lenght );

	tmp.value = GET_BYTE(packet_crc, 1);
	cbWrite(&buffer, &tmp);

	tmp.value = GET_BYTE(packet_crc, 0);
	cbWrite(&buffer, &tmp);

	// append "end" character
	tmp.value = package->super.frame_end;
	cbWrite(&buffer, &tmp);

	// cleanup
	//free(tmp); no need

	return &buffer;
}



int
cca_comm_prot_protocol_id_01_get_master_request(
		cca_comm_prot_protocol_id_01 *frame,
		CircularBuffer *buffer){

	// Algorithm:
	// ----------
	// - detect frame start
	// - copy possible frame to temporary buffer
	// - if copied frame is a real frame, erase from actual buffer
	// - return values


	// TODO: Do not make frame start removed from "buffer" if frame_not_detected
	// TODO: Do search for possible frames (search for frame start) as much as
	//   	possible (till buffer is empty, for example)

	ElemType elem_;
	ElemType *elem = &elem_;
	int status;
	int frame_not_detected = FALSE;

	if(buffer->len(buffer) < frame->frame_size){
		// there can not be a possible frame, give up
		frame_not_detected = TRUE;
	}


	// detect frame start
	while(frame_not_detected == FALSE){
		status = buffer->pop0(buffer, elem);
		if(status == CB_SUCCESS){
			if( elem->value == (typeof(elem->value)) frame->super.frame_start)
			{
				// frame start detected
				break;
			}
		}
		if(status == CB_FAIL_EMPTY){
			frame_not_detected = TRUE;
			break;
		}
	}

	// We have detected frame start
	// There should be (frame->frame_size - 1) characters in the buffer
	int frame_len_without_frame_start = frame->frame_size - 1;

	if(frame_not_detected == FALSE){
		if(buffer->len(buffer) < frame_len_without_frame_start){
			// there can not be a possible frame, give up
			frame_not_detected = TRUE;
		}
	}
	CircularBuffer tempBuffer;
	cbInit(&tempBuffer,frame->frame_size);
	CircularBuffer *tmp_buffer = &tempBuffer;//lüzumsuz düzenleme gerekli
	//CircularBuffer *tmp_buffer = new_CircularBuffer(frame->frame_size); // needs clean up

	if(frame_not_detected == FALSE){
		int tmp1 = buffer->cp_elems_to(buffer, tmp_buffer, frame_len_without_frame_start);
		if(tmp1 != frame_len_without_frame_start){
			// copy failed, give up
			frame_not_detected = TRUE;
		}
	}

	if(frame_not_detected == FALSE){
		// detect protocol id
		status = tmp_buffer->pop0(tmp_buffer, elem);
		if( status != CB_SUCCESS){
			if( elem->value != (typeof(elem->value)) frame->super.protocol_id){
				// protocol id not detected, give up
				frame_not_detected = TRUE;
			}
		}
	}

	if(frame_not_detected == FALSE){
		// we detected protocol_id, 2 characters have been cut from the
		// possible frame which is located in tmp_buffer
		tmp_buffer->read(tmp_buffer, elem, tmp_buffer->len(tmp_buffer) - 1);
		if( elem->value != (typeof(elem->value)) frame->super.frame_end) {
			frame_not_detected = TRUE;
		}
	}

	// we detected frame end, this looks like a real frame.
	// check for crc

	if(frame_not_detected == FALSE){

		cca_comm_prot_packet_str data_section = tmp_buffer->read_str(tmp_buffer, 0, tmp_buffer->len(tmp_buffer) - 3);
		crc crc_of_frame = frame->calc_crc(data_section.message, data_section.lenght);

		crc crc_from_frame = 0;
		tmp_buffer->read(tmp_buffer, elem, tmp_buffer->len(tmp_buffer) - 3);
		crc_from_frame = ((crc)elem->value) << 8;
		tmp_buffer->read(tmp_buffer, elem, tmp_buffer->len(tmp_buffer) - 2);
		crc_from_frame = crc_from_frame | ((crc)elem->value);

		if( crc_from_frame != crc_of_frame){
			// frame detection failed
			frame_not_detected = TRUE;
		}
	}

	if(frame_not_detected == FALSE){
		// we have detected frame.
		// erase examined portion from actual buffer
		buffer->erase0(buffer, frame_len_without_frame_start);

		frame->src = 0;
		frame->dest = 0;
		frame->digital_io = 0;
		frame->analog_io[0] = 0;
		frame->analog_io[1] = 0;
		frame->analog_io[2] = 0;
		frame->analog_io[3] = 0;

		// get requested values
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->src = frame->src | (((uint8_t)elem->value) << 24);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->src = frame->src | (((uint8_t)elem->value) << 16);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->src = frame->src | (((uint8_t)elem->value) << 8);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->src = frame->src | (((uint8_t)elem->value) << 0);

		tmp_buffer->pop0(tmp_buffer, elem);
		frame->dest = frame->dest | (((uint8_t)elem->value) << 24);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->dest = frame->dest | (((uint8_t)elem->value) << 16);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->dest = frame->dest | (((uint8_t)elem->value) << 8);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->dest = frame->dest | (((uint8_t)elem->value) << 0);

		tmp_buffer->pop0(tmp_buffer, elem);
		frame->digital_io = frame->digital_io | (((uint8_t)elem->value) << 24);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->digital_io = frame->digital_io | (((uint8_t)elem->value) << 16);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->digital_io = frame->digital_io | (((uint8_t)elem->value) << 8);
		tmp_buffer->pop0(tmp_buffer, elem);
		frame->digital_io = frame->digital_io | (((uint8_t)elem->value) << 0);

		int i;
		for(i = 0; i < 4; i++)
		{
			tmp_buffer->pop0(tmp_buffer, elem);
			frame->analog_io[i] = frame->analog_io[i] | (((uint8_t)elem->value) << 8);

			tmp_buffer->pop0(tmp_buffer, elem);
			frame->analog_io[i] = frame->analog_io[i] | (((uint8_t)elem->value) << 0);
		}

	}

	// finally
	//
	// clean up
	//tmp_buffer->del(tmp_buffer);


	// return success status
	if(frame_not_detected == FALSE){
		return TRUE;
	}
	else{
		return FALSE;
	}
}




crc cca_comm_prot_protocol_calc_crc16(char *message, int lenght){
	static uint8_t crc_map_initialized = 0;
	if (crc_map_initialized == 0){
		crcInit();
		crc_map_initialized = 1;
	}
	return crcFast((unsigned char *)message, lenght);
}
