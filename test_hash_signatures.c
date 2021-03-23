#include <check.h>

#include "hash_sig_api.h"

#define MESSAGE_SIZE 17

unsigned char signature_copy[CRYPTO_BYTES + MESSAGE_SIZE];

START_TEST(key_gen) {

	unsigned char sk[PRIVATE_KEY_LEN] = { 0 };
	unsigned char pk[HSS_MAX_PUBLIC_KEY_LEN] = { 0 };

	int return_key_gen = keygen(sk, pk);
	ck_assert_int_eq(return_key_gen, 1);

}
END_TEST

START_TEST(sign_message) {

	unsigned char sk[PRIVATE_KEY_LEN] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x53, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x59, 0x68,
			0x3a, 0xca, 0xc8, 0xfe, 0x75, 0x5a, 0x30, 0x7e, 0xbf, 0x2d, 0xb4,
			0xb9, 0xee, 0xf0, 0xd9, 0xb1, 0xa9, 0x2c, 0xa6, 0x5f, 0xb2, 0x6b,
			0xf0, 0x4f, 0xed, 0x80, 0x44, 0x36, 0x1, 0x4a, 0x83, 0xbf, 0xbd,
			0x5e, 0x7f, 0xaa, 0x6f, 0xbd, 0xdc, 0x6f, 0xfb, 0xe6, 0xc, 0x38,
			0x17, 0x9d };
	/*	unsigned char pk[HSS_MAX_PUBLIC_KEY_LEN] = { 0x0, 0x0, 0x0, 0x2, 0x0, 0x0,
	 0x0, 0x5, 0x0, 0x0, 0x0, 0x3, 0x83, 0xbf, 0xbd, 0x5e, 0x7f, 0xaa,
	 0x6f, 0xbd, 0xdc, 0x6f, 0xfb, 0xe6, 0xc, 0x38, 0x17, 0x9d, 0x8f,
	 0x90, 0x1d, 0x46, 0x6f, 0x92, 0xcd, 0xe8, 0x21, 0x14, 0x91, 0xe3,
	 0xe3, 0xcc, 0x3c, 0x92, 0xf5, 0x7b, 0xc6, 0xbe, 0xb9, 0x42, 0xb5,
	 0x65, 0x5a, 0xcd, 0x8d, 0xf1, 0xc7, 0xc3, 0x7, 0x65 };*/
	unsigned char message[MESSAGE_SIZE] = "This should work!";
	unsigned char signature[CRYPTO_BYTES + MESSAGE_SIZE];
	unsigned long long length = 0;

	int return_sign = sign(signature, &length, message, MESSAGE_SIZE, sk);
	ck_assert_int_eq(return_sign, 1);
	ck_assert_int_eq(length, CRYPTO_BYTES);
	memcpy(signature_copy, signature, CRYPTO_BYTES * sizeof(unsigned char));

}
END_TEST

START_TEST(verify_signature) {
	unsigned char sk[PRIVATE_KEY_LEN] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
			0x0, 0x53, 0x53, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x59, 0x68,
			0x3a, 0xca, 0xc8, 0xfe, 0x75, 0x5a, 0x30, 0x7e, 0xbf, 0x2d, 0xb4,
			0xb9, 0xee, 0xf0, 0xd9, 0xb1, 0xa9, 0x2c, 0xa6, 0x5f, 0xb2, 0x6b,
			0xf0, 0x4f, 0xed, 0x80, 0x44, 0x36, 0x1, 0x4a, 0x83, 0xbf, 0xbd,
			0x5e, 0x7f, 0xaa, 0x6f, 0xbd, 0xdc, 0x6f, 0xfb, 0xe6, 0xc, 0x38,
			0x17, 0x9d };
	unsigned char pk[HSS_MAX_PUBLIC_KEY_LEN] = { 0x0, 0x0, 0x0, 0x2, 0x0, 0x0,
			0x0, 0x5, 0x0, 0x0, 0x0, 0x3, 0x83, 0xbf, 0xbd, 0x5e, 0x7f, 0xaa,
			0x6f, 0xbd, 0xdc, 0x6f, 0xfb, 0xe6, 0xc, 0x38, 0x17, 0x9d, 0x8f,
			0x90, 0x1d, 0x46, 0x6f, 0x92, 0xcd, 0xe8, 0x21, 0x14, 0x91, 0xe3,
			0xe3, 0xcc, 0x3c, 0x92, 0xf5, 0x7b, 0xc6, 0xbe, 0xb9, 0x42, 0xb5,
			0x65, 0x5a, 0xcd, 0x8d, 0xf1, 0xc7, 0xc3, 0x7, 0x65 };
	unsigned char message[MESSAGE_SIZE] = "This should work!";
	unsigned char signature[CRYPTO_BYTES + MESSAGE_SIZE];
	unsigned long long length = 0;

	int return_sign = sign(signature, &length, message, MESSAGE_SIZE, sk);
	ck_assert_int_eq(return_sign, 1);
	ck_assert_int_eq(length, CRYPTO_BYTES);

	int return_verify = verify(pk, signature, length, message, MESSAGE_SIZE);
	ck_assert_int_eq(return_verify, 1);

}
END_TEST

Suite* sign_suit(void) {
	Suite *s;
	TCase *tc_core;

	s = suite_create("HASH");
	tc_core = tcase_create("Core");

	tcase_add_test(tc_core, key_gen);
	tcase_add_test(tc_core, sign_message);
	tcase_add_test(tc_core, verify_signature);
	suite_add_tcase(s, tc_core);

	return s;
}

int main(void) {
	int no_failed = 0;
	Suite *s;
	SRunner *runner;

	s = sign_suit();
	runner = srunner_create(s);

	srunner_run_all(runner, CK_NORMAL);

	no_failed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return (no_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

