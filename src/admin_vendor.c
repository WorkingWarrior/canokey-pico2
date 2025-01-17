

#include "pico/unique_id.h"
#include <admin.h>
#include "include/git-rev.h"

#define VENDOR_NFC_SET 0x01
#define VENDOR_NFC_GET 0x02
#define VENDOR_STACK_TEST 0x03
#define VENDOR_RDP 0x55
#define VENDOR_DFU 0x22

#define VERSION_SUFFIX "-O"
#define VERSION_SUFFIX_LENGTH 2

#define UNIQUE_ID_SIZE_BYTES 8

#define HW_VARIANT_CANOKEY_ES "CanoKey ES (PICO2)"

int admin_vendor_specific(const CAPDU *capdu, RAPDU *rapdu)
{
    UNUSED(capdu);
    UNUSED(rapdu);
    return 0;
}

int admin_vendor_version(const CAPDU *capdu, RAPDU *rapdu)
{
    UNUSED(capdu);

    const size_t git_version_length = strlen(GIT_REV);
    const size_t total_length = git_version_length + VERSION_SUFFIX_LENGTH;

    memcpy(RDATA, GIT_REV, git_version_length);
    memcpy(RDATA + git_version_length, VERSION_SUFFIX, VERSION_SUFFIX_LENGTH);
    
    LL = (total_length > LE) ? LE : total_length;

    return 0;
}

int admin_vendor_hw_variant(const CAPDU *capdu, RAPDU *rapdu)
{
    UNUSED(capdu);

    static const char *const HW_VARIANT = "CanoKey ES (PICO2)";

    size_t variant_length = strlen(HW_VARIANT);

    memcpy(RDATA, HW_VARIANT, variant_length);

    LL = (variant_length > LE) ? LE : variant_length;

    return 0;
}

int admin_vendor_hw_sn(const CAPDU *capdu, RAPDU *rapdu)
{
    UNUSED(capdu);
    pico_unique_board_id_t board_id;

    // Pobierz unique ID
    pico_get_unique_board_id(&board_id);

    // Skopiuj ID do odpowiedzi
    memcpy(RDATA, board_id.id, UNIQUE_ID_SIZE_BYTES);
    
    // Ustaw długość odpowiedzi
    LL = (UNIQUE_ID_SIZE_BYTES > LE) ? LE : UNIQUE_ID_SIZE_BYTES;

    return 0;
}