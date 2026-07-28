#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

void printUsageHelp(char * prog)
{
    printf("Usage: %s [operation] [options]\n", prog);
    printf("\n");
    printf("Operations:\n");
    printf("   -hide                         To Hide Data Inside an 8-bit Bitmap\n");
    printf("   -extract                      To Extract Hidden Data\n");
    printf("\n");
    printf("Options:\n");
    printf("   -m <message file|random>      File to Hide, \"random\" Will Randomize Hidden Message\n");
    printf("   -c <cover file>               8-bit Paletted Bitmap File To Hide In\n");
    printf("   -o <stego file>               Stego File Name to Output or Extract From (optional when hiding)\n");
    printf("   -b <bits per pixel (1-3)>     Number of Bits Hidden Per Pixel In Stego File\n");
    printf("\n");
    printf("To Hide Data Inside an 8-bit Paletted Bitmap:\n");
    printf("   %s -hide -m <message file> -c <cover file> -b <1|2|3> [-o <output file>]\n", prog);
    printf("\n");
    printf("To Extract Previously Hidden Data:\n");
    printf("   %s -extract -o <stego file> -b <1|2|3>\n", prog);
}

int main(int argc, char *argv[])
{
    // if no arguments, print help instructions
    if (argc < 2)
    {
        printUsageHelp(argv[0]);
        return 0;
    }

    char *sCoverFile = NULL;
    char *sMessageFile = NULL;
    char *sStegoFile = NULL;
    int iBitsPerPixel = 0;
    bool bIsRandomMessage = false;

    // read and parse arguments
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0) // cover file
        {
            i++;
            if (i < argc)
                sCoverFile = argv[i];
            else
            {
                printf("Error in arguments: missing cover file.\n");
                printf("   -c <cover file>\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-o") == 0) // stego file
        {
            i++;
            if (i < argc)
                sStegoFile = argv[i];
            else
            {
                printf("Error in arguments: missing stego file.\n");
                printf("   -o <stego file>\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-m") == 0) // message data (or random)
        {
            i++;
            if (i < argc)
            {
                if (strcmp(argv[i], "random") == 0)
                    bIsRandomMessage = true;
                else
                    sMessageFile = argv[i];
            }
            else
            {
                printf("Error in arguments: missing message.\n");
                printf("   -m <message file|random>\n");
                printf("       or\n");
                printf("   -m random\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-b") == 0) // bits
        {
            i++;
            if (i < argc)
                iBitsPerPixel = atoi(argv[i]);
            else
            {
                printf("Error in arguments: missing bits.\n");
                printf("   -b <1|2|3>\n");
                return 1;
            }
        }
        else
        {
            printf("Error: unrecognized argument %s\n", argv[i]);
            printUsageHelp(argv[0]);
            return 1;
        }
    }

    

    
    
    /****************************************
     H   H  III  DDDD   EEEEE
     H   H   I   D  DD  E    
     HHHHH   I   D   D  EEE  
     H   H   I   D  DD  E    
     H   H  III  DDDD   EEEEE
    ****************************************/
    if(strcmp(argv[1], "-hide") == 0)
    {
        if (sCoverFile == NULL)
        {
            printf("Error: hiding requires cover file.\n");
            printUsageHelp(argv[0]);
            return 1;
        }
        if (sMessageFile == NULL && !bIsRandomMessage)
        {
            printf("Error: hiding requires message file or \"random\" specified.\n");
            printUsageHelp(argv[0]);
            return 1;
        }
        if (iBitsPerPixel < 1 || iBitsPerPixel > 3)
        {
            printf("Error: hiding requires specified bits per pixel.\n");
            printUsageHelp(argv[0]);
            return 1;
        }

        // open cover file and check it is valid
        FILE *fpCover = fopen(sCoverFile, "rb");
        if (fpCover == NULL)
        {
            printf("Error opening cover file %s\n", sCoverFile);
            return 1;
        }
        uint8_t  ui8FileSignature[2];
        uint16_t ui16BitsPerPixel;
        uint32_t ui32Compression;
        if (fseek(fpCover, 0, SEEK_SET) != 0)
        {
            printf("Error seeking to start of cover file.\n");
            return 1;
        }
        if (fread(ui8FileSignature, 1, 2, fpCover) != 2)
        {
            printf("Error reading cover file signature.\n");
            return 1;
        }
        if (fseek(fpCover, 28, SEEK_SET) != 0)
        {
            printf("Error seeking to BitsPerPixel in cover file.\n");
            return 1;
        }
        if (fread(&ui16BitsPerPixel, sizeof(ui16BitsPerPixel), 1, fpCover) != 1)
        {
            printf("Error reading cover file BitsPerPixel.\n");
            return 1;
        }
        if (fseek(fpCover, 30, SEEK_SET) != 0)
        {
            printf("Error seeking to compression in cover file.\n");
            return 1;
        }
        if (fread(&ui32Compression, sizeof(ui32Compression), 1, fpCover) != 1)
        {
            printf("Error reading cover file compression.\n");
            return 1;
        }
        if (ui8FileSignature[0] != 'B' || ui8FileSignature[1] != 'M' || 
            ui16BitsPerPixel != 8 || ui32Compression != 0)
        {
            printf("Error: hiding requires an uncompressed 8-bit bitmap cover file.\n");
            fclose(fpCover);
            return 1;
        }

        // open message file if provided
        FILE *fpMessage;
        if (!bIsRandomMessage)
        {
            fpMessage = fopen(sMessageFile, "rb");
            if(fpMessage == NULL)
            {
                printf("Error opening message file %s\n", sMessageFile);
                return 1;
            }
        }

        // get BMP fields from cover file
        uint32_t ui32PixelOffset;
        uint32_t ui32HeaderSize;
        uint32_t ui32PaletteOffset;
        int32_t i32CoverWidth;
        int32_t i32CoverHeight;
        uint32_t ui32CoverWidth;
        uint32_t ui32CoverHeight;
        if (fseek(fpCover, 10, SEEK_SET) != 0)
        {
            printf("Error seeking to PixelOffset in cover file.\n");
            return 1;
        }
        if (fread(&ui32PixelOffset, sizeof(ui32PixelOffset), 1, fpCover) != 1)
        {
            printf("Error reading cover file PixelOffset.\n");
            return 1;
        }
        if (fseek(fpCover, 14, SEEK_SET) != 0)
        {
            printf("Error seeking to HeaderSize in cover file.\n");
            return 1;
        }
        if (fread(&ui32HeaderSize, sizeof(ui32HeaderSize), 1, fpCover) != 1)
        {
            printf("Error reading cover file HeaderSize.\n");
            return 1;
        }
        ui32PaletteOffset = 14U + ui32HeaderSize;
        if (fseek(fpCover, 18, SEEK_SET) != 0)
        {
            printf("Error seeking to Image Width in cover file.\n");
            return 1;
        }
        if (fread(&i32CoverWidth, sizeof(i32CoverWidth), 1, fpCover) != 1)
        {
            printf("Error reading cover file Image Width.\n");
            return 1;
        }
        if (i32CoverWidth <= 0)
        {
            printf("Error: cover file bitmap width must be positive.\n");
            return 1;
        }
        if (fseek(fpCover, 22, SEEK_SET) != 0)
        {
            printf("Error seeking to Image Height in cover file.\n");
            return 1;
        }
        if (fread(&i32CoverHeight, sizeof(i32CoverHeight), 1, fpCover) != 1)
        {
            printf("Error reading cover file Image Height.\n");
            return 1;
        }
        if (i32CoverHeight == 0 || i32CoverHeight == INT32_MIN)
        {
            printf("Error: invalid cover bitmap height.\n");
            return 1;
        }
        ui32CoverWidth = (uint32_t)i32CoverWidth;
        ui32CoverHeight = (uint32_t)abs(i32CoverHeight);


        // determine row padding, bitmap pixel row data must be divisible by 4 bytes.
        uint32_t ui32PaddingSize = ((ui32CoverWidth + 3U) / 4U) * 4U - ui32CoverWidth;

        bool bCoverIndexUsed[256] = {false};
        uint32_t ui32ActivePaletteSize = 0;
        uint8_t ui8PixelIndex;

        // loop through cover pixels to count used colors in the palette
        if (fseek(fpCover, ui32PixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to PixelOffset in cover file.\n");
            return 1;
        }
        for (uint32_t ui32Row = 0; ui32Row < ui32CoverHeight; ui32Row++)
        {
            for (uint32_t ui32Col = 0; ui32Col < ui32CoverWidth; ui32Col++)
            {
                if (fread(&ui8PixelIndex, sizeof(ui8PixelIndex), 1, fpCover) != 1)
                {
                    printf("Error reading bitmap pixel data.\n");
                    return 1;
                }
                if (!bCoverIndexUsed[ui8PixelIndex])
                {
                    bCoverIndexUsed[ui8PixelIndex] = true;
                    ui32ActivePaletteSize++;
                }
            }
            // at end of a row, skip row padding
            if(fseek(fpCover, ui32PaddingSize, SEEK_CUR) != 0)
            {
                printf("Error seeking past row padding in cover file.\n");
                return 1;
            }
        }

        
        // if stego filename provided, open for write. else generate name to open
        char sGeneratedStegoFileName[256];
        if (sStegoFile == NULL)
        {
            snprintf(sGeneratedStegoFileName, sizeof(sGeneratedStegoFileName), "%s_hide_%d.bmp", sCoverFile, iBitsPerPixel);
            sStegoFile = sGeneratedStegoFileName;
        }
        FILE *fpStego;
        fpStego = fopen(sStegoFile, "wb");
        if (fpStego == NULL)
        {
            printf("Error opening stego file for writing %s\n", sStegoFile);
            return 1;
        }

        // write cover header into stego file
        uint8_t *ui8Header = malloc(ui32PaletteOffset);
        if (ui8Header == NULL)
        {
            printf("Error allocating memory for bitmap header.\n");
            return 1;
        }

        if (fseek(fpCover, 0, SEEK_SET) != 0)
        {
            printf("Error seeking to start of cover file.\n");
            free (ui8Header);
            return 1;
        }
        if (fread(ui8Header, 1, ui32PaletteOffset, fpCover) != ui32PaletteOffset)
        {
            printf("Error reading cover bitmap header.\n");
            free (ui8Header);
            return 1;
        }
        if (fwrite(ui8Header, 1, ui32PaletteOffset, fpStego) != ui32PaletteOffset)
        {
            printf("Error writing bitmap header to stego file.\n");
            free (ui8Header);
            return 1;
        }
        free (ui8Header);
        ui8Header = NULL;

        // write cover palette to Stego file 2^iBitsPerPixel times
        uint8_t ui8PaletteEntry[4];
        uint8_t ui8CoverToStegoIndexRemap[8][256];
        int newIndex = 0;
        int paletteNum;
        for (paletteNum = 0; paletteNum < (1 << iBitsPerPixel); paletteNum++)
        {
            for (int i = 0; i < 256; i++)
            {
                if (bCoverIndexUsed[i])
                {
                    if (fseek(fpCover, ui32PaletteOffset + (i * 4), SEEK_SET) != 0)
                    {
                        printf("Error seeking to palette entry in cover file.\n");
                        return 1;
                    }
                    if (fread(ui8PaletteEntry, 1, 4, fpCover) != 4)
                    {
                        printf("Error reading palette entry in cover file.\n");
                        return 1;
                    }
                    /* write palette entry */
                    if (fwrite(ui8PaletteEntry, 1, 4, fpStego) != 4)
                    {
                        printf("Error writing palette entry in stego file.\n");
                        return 1;
                    }
                    ui8CoverToStegoIndexRemap[paletteNum][i] = newIndex++; // pixel index i in cover file changes to index k in stego file
                }
            }
        }


        // get total size of hidden data
        uint64_t ui64TotalHideBits = 0;
        uint64_t ui64CoverCapacityBits = (uint64_t)ui32CoverWidth * (uint64_t)ui32CoverHeight * (uint64_t)iBitsPerPixel;
        if (ui64CoverCapacityBits < 64U)
        {
            printf("Error: cover file is too small to hold size field.\n");
            return 1;
        } 
        ui64CoverCapacityBits -= 64U;

        if (!bIsRandomMessage)
        {
            if (fseek(fpMessage, 0, SEEK_END) != 0)
            {
                printf("Error seeking in message file.\n");
                return 1;
            }
            long lMessageSize = ftell(fpMessage);
            if (lMessageSize < 0)
            {
                printf("Error getting size of message file.\n");
                return 1;
            }
            uint64_t ui64MessageCapacityBits = (uint64_t)lMessageSize * 8U;
            ui64TotalHideBits = (ui64MessageCapacityBits < ui64CoverCapacityBits) ? ui64MessageCapacityBits : ui64CoverCapacityBits;
        }
        else
        {
            ui64TotalHideBits = ui64CoverCapacityBits;
        }


        uint8_t ui8MessageByte;
        int iSizeByteIndex = 0;
        int iMessageBitsRemaining = 0;
        bool bHideSizeFinished = false;
        bool bHideMessageFinished = false;

        
        if (fseek(fpCover, ui32PixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to start of pixel data in cover file.\n");
            return 1;
        }
        if (fseek(fpStego, ui32PixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to start of pixel data in stego file.\n");
            return 1;
        }
        if (!bIsRandomMessage)
            if (fseek(fpMessage, 0, SEEK_SET) != 0)
            {
                printf("Error seeking to start of message file.\n");
                return 1;
            }

        // loop through cover pixels, modify indexes to hide message and write to stego
        for (uint32_t ui32Row = 0; ui32Row < ui32CoverHeight; ui32Row++)
        {
            for (uint32_t ui32Col = 0; ui32Col < ui32CoverWidth; ui32Col++)
            {

                // get next cover pixel
                uint8_t ui8CoverPixelIndex;
                if (fread(&ui8CoverPixelIndex, sizeof(ui8CoverPixelIndex), 1, fpCover) != 1)
                {
                    printf("Error reading cover file pixel.\n");
                    return 1;
                }
                uint8_t ui8StegoPixelIndex = ui8CoverToStegoIndexRemap[0][ui8CoverPixelIndex];

                if (bIsRandomMessage)
                {
                    uint8_t ui8HiddenValue = 0;

                    for (int i = 0; i < iBitsPerPixel; i++)
                    {
                        uint8_t ui8NextBit = 0;
                        
                        if (iMessageBitsRemaining == 0)
                        {
                            if (!bHideSizeFinished)
                            {
                                // feed 8 bits of ui64TotalHideBits into ui8MessageByte
                                ui8MessageByte = (uint8_t)((ui64TotalHideBits >> (56 - (iSizeByteIndex * 8))) & 0xFFU);
                                iSizeByteIndex++;
                                iMessageBitsRemaining = 8;
                                if (iSizeByteIndex == 8)
                                    bHideSizeFinished = true;
                            }
                            else
                            {
                                // generate random data
                                ui8MessageByte = (uint8_t)(rand() % 256);
                                iMessageBitsRemaining = 8;
                            }
                        }

                        ui8NextBit = (ui8MessageByte >> (iMessageBitsRemaining - 1)) & 1U;
                        iMessageBitsRemaining--;

                        ui8HiddenValue = (uint8_t)((ui8HiddenValue << 1) | ui8NextBit);
                    }
                    
                    // assign pixel to palette matching value to hide
                    ui8StegoPixelIndex = ui8CoverToStegoIndexRemap[ui8HiddenValue][ui8CoverPixelIndex];
                    if (fwrite(&ui8StegoPixelIndex, sizeof(ui8StegoPixelIndex), 1, fpStego) != 1)
                    {
                        printf("Error writing hidden pixel to stego file.\n");
                        return 1;
                    }
                }
                else if (!bHideMessageFinished)
                {
                    // embed message into pixel data
                    uint8_t ui8HiddenValue = 0;

                    // get bits from message
                    for (int i = 0; i < iBitsPerPixel; i++)
                    {
                        uint8_t ui8NextBit = 0;

                        if (iMessageBitsRemaining == 0)
                        {
                            if (!bHideSizeFinished)
                            {
                                // feed 8 bits of ui64TotalHideBits into ui8MessageByte
                                ui8MessageByte = (uint8_t)((ui64TotalHideBits >> (56 - (iSizeByteIndex * 8))) & 0xFFU);
                                iSizeByteIndex++;
                                iMessageBitsRemaining = 8;
                                if (iSizeByteIndex == 8)
                                    bHideSizeFinished = true;
                            }
                            else
                            {
                                if (fread(&ui8MessageByte, 1, 1, fpMessage) != 1)
                                {
                                    if (ferror(fpMessage))
                                    {
                                        printf("Error reading message file.\n");
                                        return 1;
                                    }
                                    bHideMessageFinished = true;
                                }
                                else
                                    iMessageBitsRemaining = 8;
                            }
                        }

                        if (!bHideMessageFinished)
                        {
                            ui8NextBit = (ui8MessageByte >> (iMessageBitsRemaining - 1)) & 1U;
                            iMessageBitsRemaining--;
                        }

                        ui8HiddenValue = (uint8_t)((ui8HiddenValue << 1) | ui8NextBit);
                    }

                    // assign pixel to palette matching value to hide
                    ui8StegoPixelIndex = ui8CoverToStegoIndexRemap[ui8HiddenValue][ui8CoverPixelIndex];
                    if (fwrite(&ui8StegoPixelIndex, sizeof(ui8StegoPixelIndex), 1, fpStego) != 1)
                    {
                        printf("Error writing hidden pixel to stego file.\n");
                        return 1;
                    }
                }
                else
                {
                    // entire message hidden, write remaining cover to stego file
                    if (fwrite(&ui8StegoPixelIndex, 1, 1, fpStego) != 1)
                    {
                        printf("Error writing pixel to stego file.\n");
                        return 1;
                    }
                }
            }

            for (uint32_t i = 0; i < ui32PaddingSize; i++)
            {
                uint8_t ui8PaddingByte;
                if (fread(&ui8PaddingByte, 1, 1, fpCover) != 1)
                {
                    printf("Error reading padding byte from cover file.\n");
                    return 1;
                }
                if (fwrite(&ui8PaddingByte, 1, 1, fpStego) != 1)
                {
                    printf("Error writing padding byte to stego file.\n");
                    return 1;
                }
            }
        }

        fclose(fpCover);
        fclose(fpStego);
        if (!bIsRandomMessage)
            fclose(fpMessage);

        printf("Palette Duplication with %d bits performed successfully and saved at %s\n", iBitsPerPixel, sStegoFile);
    }
    /****************************************
     EEEEE X   X TTTTT RRRR   AAA   CCC  TTTTT
     E     XX XX   T   R   R A   A C   C   T  
     EEE    XXX    T   RRRR  AAAAA C       T  
     E     XX XX   T   R  R  A   A C   C   T  
     EEEEE X   X   T   R   R A   A  CCC    T  
    ****************************************/
    else if(strcmp(argv[1], "-extract") == 0)
    {
        FILE *fileStego = fopen(sStegoFile, "rb");
    }
    else
    {
        printUsageHelp(argv[0]);
    }
    return 0;
}

