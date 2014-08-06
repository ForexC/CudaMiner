
#include "cpuminer-config.h"
#include "miner.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <sph_sha2.h>
#include <sph_keccak.h>
#include <sph_haval.h>
#include <sph_tiger.h>
#include <sph_whirlpool.h>
#include <sph_ripemd.h>


static void set_one_if_zero(uint8_t *hash512) {
    for (int i = 0; i < 32; i++) {
        if (hash512[i] != 0) {
            return;
        }
    }
    hash512[0] = 1;
}

static bool fulltest_m7hash(const uint32_t *hash32, const uint32_t *target32)
{
    int i;
    bool rc = true;

    const unsigned char *hash = (const unsigned char *)hash32;
    const unsigned char *target = (const unsigned char *)target32;
    for (i = 31; i >= 0; i--) {
        if (hash[i] != target[i]) {
            rc = hash[i] < target[i];
            break;
        }
    }

    return rc;
}

uint64_t cuda_scanhash(void* ctx, void* data, void* target);

#define M7_MIDSTATE_LEN 116
int scanhash_m7hash(int thr_id, void* cuda_ctx, uint32_t *pdata, const uint32_t *ptarget,
    uint64_t max_nonce, unsigned long *hashes_done)
{
    uint32_t data[32] __attribute__((aligned(128)));
    uint32_t *data_p64 = data + (M7_MIDSTATE_LEN / sizeof(data[0]));
    uint32_t hash[8] __attribute__((aligned(32)));
    uint8_t bhash[7][64] __attribute__((aligned(32)));
    uint32_t hashtest[8] __attribute__((aligned(32)));
    uint32_t n = pdata[29] - 1;
    const uint32_t first_nonce = pdata[29];
    char data_str[245], hash_str[65], target_str[65];
    uint8_t *bdata = 0;
    int rc = 0;

    memcpy(data, pdata, 122);

    do {	
	n+=256*256*8;
        data[29] = n;


	uint64_t nonce = cuda_scanhash(cuda_ctx,data,ptarget);


        if (nonce) {

            pdata[29] = nonce >> 32;
	    n=pdata[29];

            goto out;
        }
    } while (n < max_nonce && !work_restart[thr_id].restart);

    pdata[29] = n;

out:

    *hashes_done = n - first_nonce + 1;
    free(bdata);
    return 1;
}



