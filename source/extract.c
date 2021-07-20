#include "unzip/miniunz.h"
#include "unzip/unzip.h"
#include <fat.h>
#include <gccore.h>
#include <stdio.h>
#include <unistd.h>

// Unzip Archive
bool unzipArchive(char *zipfilepath, char *unzipfolderpath) {
    unzFile uf = unzOpen(zipfilepath);

    if (uf == NULL) {
        printf("Cannot open %s, aborting\n", zipfilepath);
        return false;
    }

    if (chdir(unzipfolderpath)) {     // can't access dir
        makedir(unzipfolderpath);     // attempt to make dir
        if (chdir(unzipfolderpath)) { // still can't access dir
            printf("Error changing into %s, aborting\n", unzipfolderpath);
            return false;
        }
    }

    extractZip(uf, 0, 1, 0);

    unzCloseCurrentFile(uf);
    return true;
}
