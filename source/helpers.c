#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static fstats stats ATTRIBUTE_ALIGN(32);
#define ISFS_EEXIST -105
#define ISFS_ENOENT -105

// As we cannot install a WAD nor unzip an archive directly from the NAND via
// ISFS, we must move the forwarder WAD and zip from it's containing folder to
// the SD Card.
int move_files(const char *in_path, const char *out_path) {
    char path[45];
    char sd_path[27];

    sprintf(path, "/title/00010001/46454f4a/content/%s", in_path);
    sprintf(sd_path, "fat:/osc-temp/%s", out_path);

    FILE *fp = fopen(sd_path, "w");

    // TODO: Change placeholder path
    s32 fd = ISFS_Open(path, ISFS_OPEN_READ);
    if (fd < 0) {
        printf("ISFS_GetFile: unable to open file (error %d)\n", fd);
        return 1;
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

        if (buf != NULL) {
            s32 tmp_size = ISFS_Read(fd, buf, length);

            if (tmp_size == length) {
                // Now that the prerequistes are out of the way, write the WAD
                // to the SD Card.
                fwrite(buf, 1, tmp_size, fp);
                free(buf);
                fclose(fp);
            } else {
                // If positive, the file could not be fully read.
                // If negative, it is most likely an underlying /dev/fs
                // error.
                if (tmp_size >= 0) {
                    printf("ISFS_GetFile: only able to read %d out of "
                           "%d bytes!\n",
                           tmp_size, length);
                    return 1;
                } else if (tmp_size == ISFS_ENOENT) {
                    // We ignore logging errors about files that do not exist.
                } else {
                    printf("ISFS_GetFile: ISFS_Open failed! (error %d)\n",
                           tmp_size);
                    return 1;
                }
                free(buf);
            }
        } else {
            printf("ISFS_GetFile: failed to allocate buffer!\n");
            return 1;
        }
    } else {
        printf("ISFS_GetFile: unable to retrieve file stats (error %d)\n", ret);
        return 1;
    }
    ISFS_Close(fd);

    return 0;
}

int remove_temp_files() {
    // Unfortunately, we cannot recursively delete a folder.
    // As such, we will delete all files then the folder.
    unlink("fat:/osc-temp/forwarder.wad");
    unlink("fat:/osc-temp/hbc-app.zip");
    unlink("fat:/osc-temp");

    // Now, to delete the files on the NAND
    ISFS_Delete("/title/00010001/46454f4a");

    return 0;
}