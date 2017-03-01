/**
 * resize.c
 *
 * Computer Science 50
 * Problem Set 4
 *
 * Copies a BMP piece by piece, and resizes it by a factor supplied 
 * when the program is called.
 */
       
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

int main(int argc, char* argv[])
{
    // ensure proper usage
    if (argc != 4)
    {
        printf("Usage: ./resize n infile outfile\n");
        return 1;
    }

    // remember filenames
    int n = atoi(argv[1]); // Convert string to an int.
    char* infile = argv[2];
    char* outfile = argv[3];
    
    // ensure n is in the right range
    if (n < 1 || n > 100)
    {
        printf("n must be a positive number less than or equal to 100.\n");
    }

    // open input file 
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return 2;
    }

    // open output file
    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return 3;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf, bf_o;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi, bi_o;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 || 
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return 4;
    }
    // update header info
    bf_o = bf;
    bi_o = bi;
    bi_o.biWidth = n * bi.biWidth;
    bi_o.biHeight = n * bi.biHeight;
    int padding_o = (4 - (bi_o.biWidth * sizeof(RGBTRIPLE)) % 4) %4; // outfile padding
    bi_o.biSizeImage = (bi_o.biWidth * abs(bi_o.biHeight)) * 3 + (padding_o * abs(bi_o.biHeight));
    bf_o.bfSize = bi_o.biSizeImage + bf.bfOffBits;

    
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf_o, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi_o, sizeof(BITMAPINFOHEADER), 1, outptr);

    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        //Write each line n times
        for (int m = 0; m < n; m++)
        {
            // iterate over pixels in scanline
            for (int j = 0; j < bi.biWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;

                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                // write each RGB triple to outfile n times (per line)
                for (int l = 0; l < n; l++)
                {
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }
            }
        
            // skip over any padding in infile
            fseek(inptr, padding, SEEK_CUR);

            // add padding to outfile
            for (int k = 0; k < padding_o; k++)
            {
                fputc(0x00, outptr);
            }
                
            // Move cursor back across the width of one line from current position
            fseek(inptr, -(bi.biWidth * 3 + padding), SEEK_CUR);
        }
        // Move cursor across the width of one line from current position
        fseek(inptr, bi.biWidth * 3 + padding, SEEK_CUR);
     }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // that's all folks
    return 0;
}
