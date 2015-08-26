/*
 * cca_comm_prot.h
 *
 *  Created on: Feb 10, 2015
 *      Author: efe
 */
#ifndef __CCA_COMM_PROT_H
#define __CCA_COMM_PROT_H

/*
 *
 * SOH
 * Protocol ID (HEX, UTF-8 like scheme (if frame[x] is part of Protocol_ID and frame[x] > 127 then frame[x+1] belongs to Protocol_ID  ))
 * == BEGIN PROTOCOL DATA ==
 * == END PROTOCOL DATA ==
 * EOT
 *
 *
 * Protocol ID: 0x01
 * -----------------
 * - Big Endian :
 * 		(0xA0B0C0D0 ->
 * 			Byte[0] = 0xA0,
 * 			Byte[1] = 0xB0,
 * 			Byte[2] = 0xC0,
 * 			Byte[3] = 0xD0
 * - 32 Digital Input
 * - 32 Digital Output
 * - 4 Analog Input (16 bits per channel)
 * - 4 Analog Output (16 bits per channel)
 * ------------------
 * SOURCE BYTE-0
 * SOURCE BYTE-1
 * SOURCE BYTE-2
 * SOURCE BYTE-3
 * DESTINATION BYTE-0
 * DESTINATION BYTE-1
 * DESTINATION BYTE-2
 * DESTINATION BYTE-3
 * #ifdef MASTER_PACKAGE
 * SET DIGITAL OUTPUTS 31..24
 * SET DIGITAL OUTPUTS 23..16
 * SET DIGITAL OUTPUTS 15..8
 * SET DIGITAL OUTPUTS 7..0
 * SET ANALOG OUTPUT 0 High Byte
 * SET ANALOG OUTPUT 0 Low Byte
 * SET ANALOG OUTPUT 1 High Byte
 * SET ANALOG OUTPUT 1 Low Byte
 * SET ANALOG OUTPUT 2 High Byte
 * SET ANALOG OUTPUT 2 Low Byte
 * SET ANALOG OUTPUT 3 High Byte
 * SET ANALOG OUTPUT 3 Low Byte
 * #endif // MASTER_PACKAGE
 * #ifdef SLAVE_PACKAGE
 * SEND DIGITAL INPUTS 31..24
 * SEND DIGITAL INPUTS 23..16
 * SEND DIGITAL INPUTS 15..8
 * SEND DIGITAL INPUTS 7..0
 * SEND ANALOG INPUT 0 High Byte
 * SEND ANALOG INPUT 0 Low Byte
 * SEND ANALOG INPUT 1 High Byte
 * SEND ANALOG INPUT 1 Low Byte
 * SEND ANALOG INPUT 2 High Byte
 * SEND ANALOG INPUT 2 Low Byte
 * SEND ANALOG INPUT 3 High Byte
 * SEND ANALOG INPUT 3 Low Byte
 * #endif // SLAVE_PACKAGE
 * CRC-16 OF PROTOCOL DATA SECTION High Byte
 * CRC-16 OF PROTOCOL DATA SECTION  Low Byte
 *
 *
 */
#include "circular_buffer.h"
//#include "stm32f4xx.h" gerekli mi ?
#include "crc.h"

#define SOH 0x01
#define EOT 0x04

typedef struct cca_comm_prot_protocol_basic{
	char frame_start;
	char protocol_id;
	char frame_end;

	uint32_t self_id;
} cca_comm_prot_protocol_basic;



/* Protocol 01 */
// ----------------

// Struct
// -------
typedef struct cca_comm_prot_protocol_id_01 cca_comm_prot_protocol_id_01;
struct cca_comm_prot_protocol_id_01{
	cca_comm_prot_protocol_basic super;//artık pointer değil
	uint32_t src;
	uint32_t dest;
	uint32_t digital_io;
	uint16_t analog_io[4];
	crc (*calc_crc)(char *message, int lenght);

	CircularBuffer * (*mk_resp)(cca_comm_prot_protocol_id_01 *);
	int (*get_req)(cca_comm_prot_protocol_id_01 *, CircularBuffer *);
	void (*del)(cca_comm_prot_protocol_id_01 *);

	int frame_size;
};


int
cca_comm_prot_protocol_id_01_get_master_request(
		cca_comm_prot_protocol_id_01 *,
		CircularBuffer *);

CircularBuffer*
cca_comm_prot_protocol_id_01_mk_slave_response(
		cca_comm_prot_protocol_id_01 *);

void
cca_comm_prot_protocol_id_01_del(
		cca_comm_prot_protocol_id_01 *);


crc
cca_comm_prot_protocol_calc_crc16(char *message, int lenght);

cca_comm_prot_protocol_id_01*
new_cca_comm_prot_protocol_id_01(uint32_t device_id);
#endif


