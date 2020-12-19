// vim:ts=4:shiftwidth=4:expandtab:smartindent

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "tiffio.h"

int main(int argc, char *argv[]) {
    TIFF *tif;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.tiff\n", argv[0]);
    }
    
    tif = TIFFOpen(argv[1], "r");

    if (tif) {
        uint32 w = 0;
        uint32 h = 0;

        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH,  &w);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

        uint32 *raster = _TIFFmalloc(w * h * sizeof(uint32));
        
        int success = TIFFReadRGBAImageOriented(tif, w, h, raster, ORIENTATION_TOPLEFT, 1);
        assert(success==1);
      
        for (uint32 y=0; y<h; y++) {
            for (uint32 x=0; x<w; x++) {
                if (TIFFGetR(raster[y*w + x]) < 127 || TIFFGetG(raster[y*w + x]) < 127 || TIFFGetB(raster[y*w + x]) < 127) {
                    printf("%d %d %d\n", x, y, 0);
                }
            }
        }
        
    }
    else {
        fprintf(stderr, "Could not open '%s'.", argv[1]);
        exit(1);
    }
    return 0;
}
