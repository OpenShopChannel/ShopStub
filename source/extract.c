#include "unzip/unzip.h"
#include <fat.h>
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>

#define UNZIP_OK 0
#define UNZIP_ERROPEN 1
#define UNZIP_ERRNOTFOUND 2
#define UNZIP_ERRCREATE 3
#define UNZIP_ERRREAD 4
#define UNZIP_ERRWRITE 5

int unzipFile(char *zipfile, char *filename, char *outfile) {
    unzFile uf = 0;
    FILE *fout = 0;
    int result, bytes;
    char buffer[1024];

    uf = unzOpen(zipfile);
    if (!uf) {
        printf("Can not open file '%s'!\n", zipfile);
        return UNZIP_ERROPEN;
    }

    if (unzLocateFile(uf, filename, 0) != UNZ_OK) {
        printf("Can not find '%s' inside the zipfile!\n", filename);
        unzCloseCurrentFile(uf);
        sleep(4);
        return UNZIP_ERRNOTFOUND;
    }

    if (unzOpenCurrentFile(uf) != UNZ_OK) {
        printf("Can not open '%s' inside the zipfile!\n", filename);
        unzCloseCurrentFile(uf);
        sleep(4);
        return UNZIP_ERRNOTFOUND;
    }

    fout = fopen(outfile, "wb");
    if (!fout) {
        printf("Can not create file '%s'!\n", outfile);
        unzCloseCurrentFile(uf);
        sleep(4);
        return UNZIP_ERRCREATE;
    }

    result = UNZIP_OK;
    for (;;) {
        bytes = unzReadCurrentFile(uf, buffer, sizeof(buffer));
        if (bytes == 0)
            break; /* finish */
        if (bytes < 0) {
            printf("Error reading %s %d!\n", filename, bytes);
            result = UNZIP_ERRREAD;
            break;
        } else if (fwrite(buffer, 1, bytes, fout) != bytes) {
            printf("error in writing extracted file\n");
            result = UNZIP_ERRWRITE;
            break;
        }
    }

    fclose(fout);
    unzCloseCurrentFile(uf);

    return result;
}
