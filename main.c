#include <stdio.h>
#include <stdlib.h>

#include "hash_sig_api.h"

int main(void) {
	unsigned char sk[PRIVATE_KEY_LEN] = { 0 };
	unsigned char pk[HSS_MAX_PUBLIC_KEY_LEN] = { 0 };
	unsigned char message[17] = "This should work!";
	unsigned char signature[CRYPTO_BYTES + 17] = { 0 };
	unsigned long long length = 0;

	int key_gen_sign = keygen(sk, pk);
	printf("\nkeygen: %d\n", key_gen_sign);

	/*printf("private key:\n");
	 for (unsigned long long j = 0; j < PRIVATE_KEY_LEN; j++) {
	 printf("0x%x,", sk[j]);
	 }

	 printf("\npublic key:\n");*/
	/*for (unsigned long long j = 0; j < HSS_MAX_PUBLIC_KEY_LEN; j++) {
	 printf("0x%x,", pk[j]);

	 }*/
	for (int i = 0; i < 1000; i++) {
		printf("\n");
		int return_sign = sign(signature, &length, message, 17, sk);
		printf("return : %d\n", return_sign);
		/*	for (unsigned long long j = 0; j < length; j++) {
		 printf("0x%x, ", signature[j]);

		 }*/
		printf("\n");
		/*printf("length: %llu\n", length);*/
		printf("length: %d\n", CRYPTO_BYTES);
		printf("\n");

		int verification = verify(pk, signature, length, message, 17);
		printf("verification: %d\n", verification);
	}
}
