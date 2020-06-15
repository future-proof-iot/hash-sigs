#if !defined( CONFIG_H_ )
#define CONFIG_H_

/*
 * This file has #define's that specify how this package operates, and
 * are designed to be tweaked by the user.
 *
 * These can be adjusted to be appropriate for what the application and
 * the operating environment needs
 */

/*
 * This modifies which seed generation logic we use
 * Note that changing these parameters will change the mapping
 * between private keys.
 *
 * 0 -> We generate seeds using the process defined in Appendix A of the draft
 *      This is slightly faster
 * 1 -> We use a side channel resistant process, never using any single secret
 *      seed in more than a defined number of distinct hashes
 * 2 -> We generate seeds and secrets in a way which is compatible with ACVP
 */
#define SECRET_METHOD 2

/*
 * If we're using the side channel resistant method, this defines the max
 * number of times we'll use a single secret.  Note that this is the log2
 * of the max number of times, and so 3 means 'no more than 8 times'
 * Reducing SECRET_MAX is a bit more costly; however I don't know that if
 * it is significant
 */
#define SECRET_MAX 4  /* Never use a seed more than 16 times */

/*
 * This indicates whether we attempt to support the SHA3 (SHAKE) based
 * parameter sets
 *
 * 0 -> We don't
 * 1 -> We do.  This requires us to use a SHA3 library
 */
#define SUPPORT_SHA3 0

#endif /* CONFIG_H_ */
