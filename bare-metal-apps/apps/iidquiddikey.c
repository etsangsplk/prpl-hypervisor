/*
Copyright (c) 2016, prpl Foundation

Permission to use, copy, modify, and/or distribute this software for any purpose with or without
fee is hereby granted, provided that the above copyright notice and this permission notice appear
in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

This code was written by Pierpaolo Bagnasco at Intrinsic-ID.

*/

/* IID Quiddikey test */

#include <pic32mz.h>
#include <libc.h>
#include <quiddikey/iid_errors.h>
#include <quiddikey/iidquiddikey.h>


volatile int32_t t2 = 0;

void irq_timer(){
 t2++;
}

#define IID_CHECK_AC_CONSTANT	0x3DDCCABC

static return_t readAndCheckAC(uint8_t * activationCode) {
	hyp_flash_read(activationCode);

	if (IID_CHECK_AC_CONSTANT == *((uint32_t *)&activationCode[QK_AC_SIZE_BYTES])) {
		return IID_SUCCESS; /* If AC exists */
	}

	return 1;/* Whatever value, if AC doesn't exist */
}

static return_t storeAC(const uint8_t * const activationCode) {
	*(uint32_t *)&activationCode[QK_AC_SIZE_BYTES] = IID_CHECK_AC_CONSTANT;

	hyp_flash_write(activationCode);

	return IID_SUCCESS;
}

char buffer[256];

extern unsigned int _puf_address;

int main() {
	uint32_t source;
	int32_t ret;
	return_t retVal;

	uint8_t * qkSram = (uint8_t *)&_puf_address;
	//PRE_HIS_ALIGN uint8_t qkSram[1024] POST_HIS_ALIGN;
	/* Last KB of SRAM.
	 * NOTE: remember to reserve it by changing the linker script!
	 * NOTE: remember to protect it by configuring the proper r/w authorizations!
	 */

	PRE_HIS_ALIGN uint8_t activationCode_buffer[1024] POST_HIS_ALIGN = { 0 }; /* This has to be 32-bits aligned! */

	uint32_t stack_top;
	asm volatile("move %0, $sp" :"=r"(stack_top) :);
	printf("stack VM %08x\n", stack_top);

	retVal = QK_Init(qkSram, 1024);

	if (IID_SUCCESS == retVal) {
		retVal = readAndCheckAC(activationCode_buffer);

		if (IID_SUCCESS != retVal) { /* If the AC doesn't exist, generate and create it */
			retVal = QK_Enroll(activationCode_buffer);
			printf("QK_Enroll executed\n");

			if (IID_SUCCESS == retVal) {
				retVal = storeAC(activationCode_buffer); /* Store the new AC */

				if (IID_SUCCESS != retVal) {
					printf("Storage of Quiddikey's Activation Code failed with error: %02X\n", retVal);
				}
			} else {
				printf("QK_Enroll failed with error: %02X\n", retVal);
			}
		}

		if (IID_SUCCESS == retVal) {
			retVal = QK_Start(activationCode_buffer);

			if (retVal != IID_SUCCESS) {
				printf("QK_Start failed with error: %02X\n", retVal);
			} else {
				printf("Quiddikey started successfully\n");
			}
		}
	} else {
		printf("QK_Init failed with error: %02X\n", retVal);
	}


	while(1) {
		ret = ReceiveMessage(&source, buffer, 1);

//		if (ret) {
//			printf("\npong VM: message from VM ID %d: \"%s\" (%d bytes)", source, buffer, ret);
//			SendMessage(source, resp, strlen(resp) + 1);
//		}
	}

    return 0;
}

