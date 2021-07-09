#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static fstats stats ATTRIBUTE_ALIGN(32);
#define ISFS_EEXIST -105
#define ISFS_ENOENT -105


// As we cannot install a WAD directly from the NAND via ISFS, we must move the
// forwarder WAD from it's containing folder to the SD Card.
void move_wad() {
    FILE *fp = fopen("fat:/osc-temp/forwarder.wad", "w");

    s32 fd = ISFS_Open("/title/idk.wad", ISFS_OPEN_READ);
    if (fd < 0) {
        printf("ISFS_GetFile: unable to open file (error %d)\n", fd);
    }

    // From WiiLink24 Set Personal Data Channel - helpers.c
    void *buf = NULL;
    memset(&stats, 0, sizeof(fstats));

    s32 ret = ISFS_GetFileStats(fd, &stats);
    if (ret >= 0) {
        s32 length = stats.file_length;

        // We must align our length by 32.
        // memalign itself is dreadfully broken for unknown reasons.
        s32 aligned_length = length;
        s32 remainder = aligned_length % 32;
        if (remainder != 0) {
            aligned_length += 32 - remainder;
        }

        buf = aligned_alloc(32, aligned_length);
        printf("%d", aligned_length);

        if (buf != NULL) {
            s32 tmp_size = ISFS_Read(fd, buf, length);

            if (tmp_size == length) {
                // Now that the prerequistes are out of the way, write the WAD
                // to the SD Card.
                fwrite(buf, 1, tmp_size, fp);
                fclose(fp);
            } else {
                // If positive, the file could not be fully read.
                // If negative, it is most likely an underlying /dev/fs
                // error.
                if (tmp_size >= 0) {
                    printf("ISFS_GetFile: only able to read %d out of "
                           "%d bytes!\n",
                           tmp_size, length);
                } else if (tmp_size == ISFS_ENOENT) {
                    // We ignore logging errors about files that do not exist.
                } else {
                    printf("ISFS_GetFile: ISFS_Open failed! (error %d)\n",
                           tmp_size);
                }
                free(buf);
            }
        } else {
            printf("ISFS_GetFile: failed to allocate buffer!\n");
        }
    } else {
        printf("ISFS_GetFile: unable to retrieve file stats (error %d)\n", ret);
    }
    free(buf);
    ISFS_Close(fd);
}