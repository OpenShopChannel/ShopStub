// NOTE: Some of this code was taken from YAWMM, not to be confused with
// Some-YAWMM-Mod.
#include <limits.h>
#include <ogcsys.h>
#include <string.h>

#include "sha1/sha1.h"

s32 Title_ZeroSignature(signed_blob *p_sig) {
    u8 *ptr = (u8 *)p_sig;

    /* Fill signature with zeroes */
    memset(ptr + 4, 0, SIGNATURE_SIZE(p_sig) - 4);

    return 0;
}

s32 Title_FakesignTik(signed_blob *p_tik) {
    tik *tik_data = NULL;
    u16 fill;

    /* Zero signature */
    Title_ZeroSignature(p_tik);

    /* Ticket data */
    tik_data = (tik *)SIGNATURE_PAYLOAD(p_tik);

    for (fill = 0; fill < USHRT_MAX; fill++) {
        sha1 hash;

        /* Modify ticket padding field */
        tik_data->padding = fill;

        /* Calculate hash */
        SHA1((u8 *)tik_data, sizeof(tik), hash);

        /* Found valid hash */
        if (!hash[0])
            return 0;
    }

    return -1;
}