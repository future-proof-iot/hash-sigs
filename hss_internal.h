#if !defined( HSS_INTERNAL_H_ )
#define HSS_INTERNAL_H_

#include <stdlib.h>
#include "common_defs.h"
#include "hss.h"
#include "config.h"

/*
 * This is the central internal include file for the functions that make up
 * this subsystem.  It should not be used by applications
 */

#define PARAM_SET_COMPRESS_LEN 1  /* We assume that we can compress the */
                                  /* lm_type and the lm_ots type for a */
                                  /* single level into 1 byte */

/*
 * The internal structure of a private key
 */
#define PRIVATE_KEY_FORMAT 0    /* 4 byte description of the key format */
#define PRIVATE_KEY_FORMAT_NUM_LEVEL 3
#define PRIVATE_KEY_FORMAT_LEN 4
#define PRIVATE_KEY_INDEX (PRIVATE_KEY_FORMAT + PRIVATE_KEY_FORMAT_LEN)
#define PRIVATE_KEY_INDEX_LEN 8  /* 2**64 signatures should be enough for */
                                 /* everyone */
#define PRIVATE_KEY_CHECKSUM (PRIVATE_KEY_INDEX + PRIVATE_KEY_INDEX_LEN)
#define PRIVATE_KEY_CHECKSUM_LEN 8
#if FAULT_CACHE_SIG
#define PRIVATE_KEY_SIG_CACHE (PRIVATE_KEY_CHECKSUM + PRIVATE_KEY_CHECKSUM_LEN)
#define PRIVATE_KEY_SIG_CACHE_LEN(levels) ((levels-1) * FAULT_CACHE_LEN)
#define PRIVATE_KEY_END_WRITABLE(levels) (PRIVATE_KEY_SIG_CACHE + \
                                    PRIVATE_KEY_SIG_CACHE_LEN(levels)) 
#else
#define PRIVATE_KEY_END_WRITABLE(levels) (PRIVATE_KEY_CHECKSUM + \
                PRIVATE_KEY_CHECKSUM_LEN ) 
#endif
/* PRIVATE_KEY_END_WRITABLE is the end of the part of the private key */
/* that is dynamically written as the key is used */
#define PRIVATE_KEY_MAX(levels)   PRIVATE_KEY_END_WRITABLE(levels)
#define PRIVATE_KEY_MAX_LEN 8
#define PRIVATE_KEY_PARAM_SET(levels) (PRIVATE_KEY_MAX(levels) + \
                                          PRIVATE_KEY_MAX_LEN)
#define PRIVATE_KEY_PARAM_SET_LEN(levels) (PARAM_SET_COMPRESS_LEN * levels)
#define PRIVATE_KEY_SEED(levels) (PRIVATE_KEY_PARAM_SET(levels) + \
                                  PRIVATE_KEY_PARAM_SET_LEN(levels))
#define PRIVATE_KEY_SEED_LEN SEED_LEN
#define PRIVATE_KEY_LEN(levels) (PRIVATE_KEY_SEED(levels) + \
                        PRIVATE_KEY_SEED_LEN) /* That's 60 bytes, plus */
                                        /* FAULT_CACHE_LEN+1 per level */

/*
 * Routines to read/update the private key
 */
enum hss_error_code hss_read_private_key(unsigned char *private_key,
            struct hss_working_key *w);
enum hss_error_code hss_write_private_key(unsigned char *private_key,
            struct hss_working_key *w, int num_cache_sig);
enum hss_error_code hss_write_private_key_no_w(
            unsigned char *private_key, size_t len,
            bool (*read_private_key)(unsigned char *private_key,
                                    size_t len_private_key, void *context),
            bool (*update_private_key)(unsigned char *private_key,
                                    size_t len_private_key, void *context),
            void *context);
bool hss_check_private_key(const unsigned char *private_key);
void hss_set_private_key_format(unsigned char *private_key, int levels);

struct merkle_level;
struct hss_working_key {
    unsigned levels;
    enum hss_error_code status;   /* What is the status of this key */
                                  /* hss_error_none if everything looks ok */
                                  /* Otherwise, the error code we report if */
                                  /* we try to use this key to sign */
    sequence_t reserve_count;     /* The value written to the private key */
                                  /* Will be higher than the 'current count' */
                                  /* if some signaures are 'reserved' */
    sequence_t max_count;         /* The maximum count we can ever have */
                                  /* (from the parameter set) */
    unsigned autoreserve;         /* How many signatures to attempt to */
                                  /* reserve if the signing process hits */
                                  /* the end of the current reservation */

    bool (*read_private_key)(     /* Function to read the private key */
            unsigned char *private_key,
            size_t len_private_key, void *context);
    bool (*update_private_key)(   /* Function to write the private key */
            unsigned char *private_key,
            size_t len_private_key, void *context);
    void *context;                /* Context pointer for the above two */

    size_t signature_len;         /* The length of the HSS signature */

    unsigned char *stack;         /* The stack memory used by the subtrees */

        /* The private key (in its entirety) */
    unsigned char private_key[PRIVATE_KEY_LEN(MAX_HSS_LEVELS)];
        /* The pointer to the seed (contained within the private key) */
        /* Warning: nonsyntaxic macro; need to be careful how we use this */
#define working_key_seed private_key + PRIVATE_KEY_SEED(w->levels)

    size_t siglen[MAX_HSS_LEVELS]; /* The lengths of the signatures */
                                  /* generated by the various levels */
    size_t signed_pk_len[MAX_HSS_LEVELS]; /* The lengths of the signed */
                                  /* public keys for the various levels */
    unsigned char *signed_pk[MAX_HSS_LEVELS]; /* The current signed public */
                                  /* keys for the nontop levels */
                                  /* Each array element is that level's */
                                  /* current root value, signed by the */
                                  /* previous level.  Unused for the */
                                  /* topmost level */
    struct merkle_level *tree[FAULT_RECOMPUTE+1][MAX_HSS_LEVELS]; /* The */
                                  /* structures that manage each individual */
                                  /* level.  The [1] versions are redundant */
                                  /* copies used to double check */
                                  /* Note: tree[1][0] == tree[0][0] */
                                  /* Because errors in the top level tree */
                                  /* don't allow forgeries */
};

#define MIN_SUBTREE    2  /* All subtrees (other than the root subtree) have */
                          /* at least 2 levels */
#define MAX_SUBLEVELS  ((MAX_MERKLE_HEIGHT + MIN_SUBTREE - 1) / MIN_SUBTREE)
#if MAX_SUBLEVELS > (1 << (MIN_MERKLE_HEIGHT-1)) - 2
#error We need to rethink our parent tree update logic, as there is a
#error possibility we do not give the tree enough updates between signatures
/* One possible fix would be to increase the subtree size for extremely */
/* tall trees */
#endif 

struct merkle_level {
    unsigned level;               /* Total number of levels */
    unsigned h, hash_size;        /* Hash function, width */
    param_set_t lm_type;
    param_set_t lm_ots_type;      /* OTS parameter */
    merkle_index_t current_index; /* The number of signatures this tree has */
                                  /* generated so far */
    merkle_index_t max_index;     /* 1<<level - 1 */
    unsigned sublevels;           /* Total number of levels of subtrees */
    unsigned subtree_size;        /* Height up each subtree */
    unsigned top_subtree_size;    /* The top subtree might be shorter */

    unsigned update_count;
        /* For nonbottom trees, we get a chance to update ourselves during */
        /* (some) signature operations.  This flag indicates what we do */
        /* with that chance */
#define UPDATE_NEXT     0    /* We update our NEXT_TREE */
#define UPDATE_PARENT   1    /* We kick the update to our parent tree */
#define UPDATE_BUILDING 2    /* For 2 .. 2 + sublevels - 2, we update the */
                             /* BUILDING_TREE subtree i-1 */
#define UPDATE_DONE     255  /* We've done all the updates we need to until */
                             /* the next signature operation */

        /* For every subtree level, we have up to 3 different subtrees: */
        /* ACTIVE_TREE; this is the subtree that the current authentication */
        /*     path goes through.  It is always fully populated */
        /* BUILDING_TREE; this is the subtree that is the next in line */
        /*     within the current Merkle tree.  We gradually populate it as */
        /*     we generate signatures for the current subtree.  When the */
        /*     authentication path shifts to this, this subtree will be */
        /*     fully populated, and we'll swap ACTIVE and BUILDING subtrees */
        /* NEXT_TREE; this is the first subtree in the next Merkle tree. */
        /*     We gradually populate the NEXT_TREE's as we generate */
        /*     signatures for the current Merkle tree.  When the current */
        /*     Merkle tree runs out, the NEXT_TREE's will be fully */
        /*     populated, and we'll swap ACTIVE and NEXT subtrees */
#define ACTIVE_TREE 0    /* The tree in the active path */
#define BUILDING_TREE 1  /* The tree in the next active path */
#define NEXT_TREE 2      /* The tree at the start of the next Merkle tree */
#define NUM_SUBTREE 3    /* Maximum number of subtrees we have at each level */
    struct subtree *subtree[MAX_SUBLEVELS][NUM_SUBTREE];

       /* The I values for the current Merkle tree, and the next one */
    unsigned char I[I_LEN], I_next[I_LEN];

        /* The seed values for the current Merkle tree, and the next one */
    unsigned char seed[SEED_LEN], seed_next[SEED_LEN];
};

/*
 * This stands for a single subtree */
struct subtree {
    merkle_index_t current_index; /* For BUILDING_TREE, the number of OTS */
                                  /*  public key's we've generated so far */
                                  /* For NEXT_TREE, 0 if haven't started on */
                                  /*  this subtree */ 
                                  /*  MAX_SUBINDEX if we've finished it */
                                  /*  Else number of OTS public keys so far */
                                  /* For ACTIVE_TREE, unused; we'll use */
                                  /*  merkle_tree::current_index to */
                                  /*  determine where the authentication */
                                  /*  path currently is */
#define MAX_SUBINDEX (~(merkle_index_t)0)

    merkle_index_t left_leaf;     /* The index of the leftmost leaf below */
                                  /* this subtree (0..2**tree->levels) */
    unsigned level;               /* The level that the root of this subtree */
                                  /* is within the larger Merkle tree */
    unsigned levels_below;        /* The number of levels below this subtree */
                                  /* in the Merkle tree */
    unsigned char *stack;         /* Pointer to the stack used when */
                                  /* generating nodes; will be a pointer */
                                  /* into the hss_working_key::stack array */
                                  /* Used to incrementally compute bottom */
                                  /* node values */
    unsigned char nodes[1];       /* The actual subtree node values */
                                  /* 2*(1<<subtree_size) - 1 of them */
};

/* Internal function to compress a list of parameters into a short format */
/* that we use internally */
bool hss_compress_param_set( unsigned char *compressed,
                   int levels, 
                   const param_set_t *lm_type,
                   const param_set_t *lm_ots_type,
                   size_t len_compressed );

/* Internal function to compute the maximum number of seqno for a parameter set */
sequence_t hss_get_max_seqno( int levels, const param_set_t *lm_type );

/* Internal function to generate the root seed, I value (based on the */
/* private seed).  We do this (rather than selecting them  at random) so */
/* that we don't need to store them in our private key; we can recompute */
void hss_generate_root_seed_I_value(unsigned char *seed, unsigned char *I,
                                     const unsigned char *master_seed);

/* Internal function to generate the seed, I value for a child Merkle tree */
/* (based on the seed, I value of the parent.  We do this (rather than */
/* selecting them at random) so we have consistent values between reboots */
void hss_generate_child_seed_I_value( unsigned char *seed, unsigned char *I,
                   const unsigned char *parent_seed,
                   const unsigned char *parent_I, merkle_index_t index,
                   param_set_t parent_lm, param_set_t parent_ots,
                   int child_level );

/* Combine two internal nodes */
void hss_combine_internal_nodes( unsigned char *dest,
        const unsigned char *left_node, const unsigned char *right_node,
        int h, const unsigned char *I, unsigned hash_size,
        merkle_index_t node_num);

bool hss_create_signed_public_key(unsigned char *signed_key,
                                    size_t len_signature,
                                    struct merkle_level *tree,
                                    struct merkle_level *parent,
                                    struct hss_working_key *w);
#if FAULT_RECOMPUTE
bool hss_doublecheck_signed_public_key(const unsigned char *signed_key,
                                    size_t len_signature,
                                    struct merkle_level *tree,
                                    struct merkle_level *parent,
                                    struct hss_working_key *w);
#endif
/* This needs to be called after we've generated a signature */
void hss_step_tree(struct merkle_level *tree);

/* Used to generate the bottom nodes of a subtree in parallel */
struct intermed_tree_detail {
    unsigned char *dest;
    merkle_index_t node_num;
    const unsigned char *seed;
    param_set_t lm_type;
    param_set_t lm_ots_type;
    unsigned h;        /* Hash function */
    unsigned tree_height;
    const unsigned char *I;
    unsigned node_count;
    enum hss_error_code *got_error;
    int level;         /* Which Merkle tree within the hypertree */
};
struct thread_collection;
void hss_gen_intermediate_tree(const void *data,
                               struct thread_collection *col);
#include "hss_zeroize.h"

/*
 * Given a tree height (and whether it's the top level, and the hash size),
 * this returns the subtree size that yields the least amount of memory
 */
int hss_smallest_subtree_size(int tree_height, int i, int n);

/*
 * Here's the routine that a thread runs to verify its part of the signature
 */
struct verify_detail {
    enum hss_error_code *got_error;
    const unsigned char *public_key;
    const void *message;
    size_t message_len;
    const unsigned char *signature;
    size_t signature_len;
    int tree_level;
};
void validate_internal_sig(const void *data,
                               struct thread_collection *col);

struct seed_derive;
void lm_ots_generate_randomizer(unsigned char *c, unsigned n,
                                struct seed_derive *seed);

bool hss_all_zero( unsigned char *s, size_t len);
bool hss_compute_hash_for_cache( unsigned char *hash_output,
                                 const unsigned char *sig, size_t sig_len );

#endif /* HSS_INTERNAL_H_ */
