#include <fat.h>
#include <gccore.h>
#include <sdcard/wiisd_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>

#include "extract.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

const DISC_INTERFACE *sd_slot = &__io_wiisd;
// TODO: Handle USB drive access
// const DISC_INTERFACE* usb = &__io_usbstorage;

void SDCard_Init() {
    sd_slot->startup();
    fatMountSimple("sd", &__io_wiisd);
}

int main(int argc, char **argv) {
    VIDEO_Init();
    WPAD_Init();
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight,
                 rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    printf("Open the shop, we said.\n");

    ISFS_Initialize();
    CONF_Init();
    SDCard_Init();
    s32 unzip = unzipFile("sd:/osc-temp/wiilauncher.zip",
                          "apps/wiilauncher/boot.dol", "sd:/moment.dol");
    if (unzip > 0) {
        printf("Epic fail");
    } else {
        printf("success");
    }

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_HOME)
            exit(0);

        VIDEO_WaitVSync();
    }

    return 0;
}
