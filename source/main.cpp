#include <fat.h>
#include <gccore.h>
#include <sdcard/wiisd_io.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiiuse/wpad.h>

extern "C" {
#include "extract.h"
#include "helpers.h"
#include "wad.h"
#include <libpatcher/libpatcher.h>
#include "unzip/unzip.h"
#include "unzip/miniunz.h"
}


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

    fatUnmount("fat:/");
    __io_usbstorage.shutdown();
    __io_wiisd.shutdown();
}

void error_msg() {
    printf("Please contact OSC support.\n");
    // Give user time to read the screen
    sleep(5);
    WII_ReturnToMenu();
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
        bool USB = __io_usbstorage.isInserted();
        if (USB) {
            fatMountSimple("fat", usb);
        } else {
            // No input devices were inserted OR it failed to mount either device.
            printf("Please insert either an SD Card or USB.\n");
            returnToMenu();
        }
    }
}

int main(void) {
    VIDEO_Init();

    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight,
                 rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    // Apply necessary patches
    bool success = apply_patches();
    if (!success) {
        printf("Failed to apply patches!\n");
        sleep(5);
        WII_ReturnToMenu();
    }

    WPAD_Init();
    ISFS_Initialize();
    CONF_Init();
    Init_IO();


    printf("Open the shop, we said.\n");
    char zip_path[128] = "fat:/osc-temp/hbc-app.zip";
    char extract_path[128] = "fat:/";

    makedir("fat:/osc-temp");

    // Moves WAD from containing folder to SD Card, in order to install.
    printf("Moving WAD from NAND to SD Card...\n");
    s32 wad = move_files("00000003.app", "forwarder.wad");
    if (wad > 0) {
        error_msg();
    }

    // Moves zip from containing folder to SD Card, in order to extract.
    printf("Moving ZIP from NAND to SD Card...\n");
    s32 zip = move_files("00000004.app", "hbc-app.zip");
    if (zip > 0) {
        error_msg();
    }

    // Now that our SD Card/USB has all the files, start unzip process
    printf("Extracting ZIP file contents...\n");
    if (unzipArchive(zip_path, extract_path)) {
        printf("\nSuccessfully unzipped archive\n");
    } else {
        error_msg();
    }

    // Start WAD installation
    printf("Installing forwarder WAD...\n");
    FILE *fp = fopen("fat:/osc-temp/forwarder.wad", "rb");
    // The forwarder WAD should exist, as we pack it ourselves.
    // This is just a save guard so the system doesn't PPCHALT.
    if (fp == NULL) {
        printf("Could not locate WAD file. Please contact OSC support.\n");
        sleep(5);
        WII_ReturnToMenu();
    } else {
        s32 install = install_WAD(fp);
        if (install < 0) {
            error_msg();
        }
    }

    // Remove files
    printf("Deleting temp files...");
    remove_temp_files();

    // If we got this far, we have successfully complete our task.
    printf("Installation complete!\n\nOn the Wii Menu, there will now be a forwarder channel you can use to launch\nyour homebrew app.");

    returnToMenu();

    return 0;
}