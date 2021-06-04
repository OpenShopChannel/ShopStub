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
const DISC_INTERFACE* usb = &__io_usbstorage;

void returnToMenu() {
    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_HOME)
            exit(0);

        VIDEO_WaitVSync();
    }
}

void Init_IO() {
    // Initialize IO
    usb->startup();
    sd_slot->startup();

    // Check if the SD Card is inserted
    bool isInserted = __io_wiisd.isInserted();

    // Try to mount the SD Card before the USB
    if (isInserted) {
        fatMountSimple("fat", sd_slot);
    } else {
        // Since the SD Card is not inserted, we will attempt to mount the USB.
        bool idk = __io_usbstorage.isInserted();
        if (idk) {
            fatMountSimple("fat", usb);
        } else {
            // No input devices were inserted OR it failed to mount either device.
            printf("Please insert either an SD Card or USB.\n");
            returnToMenu();
        }
    }
}

void DeInit_IO() {
    fatUnmount("fat:/");
    __io_usbstorage.shutdown();
    __io_wiisd.shutdown();
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
    Init_IO();

    s32 unzip = unzipFile("fat:/osc-temp/wiilauncher.zip",
                          "apps/wiilauncher/boot.dol", "fat:/moment.dol");
    if (unzip > 0) {
        printf("Epic fail");
    } else {
        printf("success");
    }

    DeInit_IO();

    returnToMenu();

    return 0;
}
