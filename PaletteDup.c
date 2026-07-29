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

        // open cover file
        FILE *fpCover = fopen(sCoverFile, "rb");
        if (fpCover == NULL)
        {
            printf("Error opening cover file %s\n", sCoverFile);
            return 1;
        }

        // open message file if provided
        FILE *fpMessage = NULL;
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
        uint8_t  ui8FileSignature[2];
        uint16_t ui16CoverBitsPerPixel;
        uint32_t ui32CoverCompression;
        uint32_t ui32CoverPixelOffset;
        uint32_t ui32CoverHeaderSize;
        uint32_t ui32CoverPaletteOffset;
        int32_t i32CoverWidth;
        int32_t i32CoverHeight;

        if (fseek(fpCover, 0, SEEK_SET) != 0 ||
            fread(ui8FileSignature, 1, 2, fpCover) != 2 ||
            fseek(fpCover, 10, SEEK_SET) != 0 ||
            fread(&ui32CoverPixelOffset, sizeof(ui32CoverPixelOffset), 1, fpCover) != 1 ||
            fseek(fpCover, 14, SEEK_SET) != 0 ||
            fread(&ui32CoverHeaderSize, sizeof(ui32CoverHeaderSize), 1, fpCover) != 1 ||
            fseek(fpCover, 18, SEEK_SET) != 0 ||
            fread(&i32CoverWidth, sizeof(i32CoverWidth), 1, fpCover) != 1 ||
            fseek(fpCover, 22, SEEK_SET) != 0 ||
            fread(&i32CoverHeight, sizeof(i32CoverHeight), 1, fpCover) != 1 ||
            fseek(fpCover, 28, SEEK_SET) != 0 ||
            fread(&ui16CoverBitsPerPixel, sizeof(ui16CoverBitsPerPixel), 1, fpCover) != 1 ||
            fseek(fpCover, 30, SEEK_SET) != 0 ||
            fread(&ui32CoverCompression, sizeof(ui32CoverCompression), 1, fpCover) != 1)
        {
            printf("Error reading stego bitmap header.\n");
            fclose(fpCover);
            return 1;
        }
        ui32CoverPaletteOffset = 14U + ui32CoverHeaderSize;

        // check cover file is uncompressed 8-bit bitmap and has valid dimensions
        if (ui8FileSignature[0] != 'B' || ui8FileSignature[1] != 'M' || 
            ui16CoverBitsPerPixel != 8 || ui32CoverCompression != 0 ||
            i32CoverWidth <= 0 || i32CoverHeight == 0 || i32CoverHeight == INT32_MIN)
        {
            printf("Error: hiding requires an uncompressed 8-bit bitmap cover file.\n");
            fclose(fpCover);
            return 1;
        }
        

        uint32_t ui32CoverWidth = (uint32_t)i32CoverWidth;
        uint32_t ui32CoverHeight = (uint32_t)abs(i32CoverHeight);
        // determine row padding, bitmap pixel row data must be divisible by 4 bytes.
        uint32_t ui32PaddingSize = ((ui32CoverWidth + 3U) / 4U) * 4U - ui32CoverWidth;

        bool bCoverIndexUsed[256] = {false};
        uint32_t ui32ActivePaletteSize = 0;
        uint8_t ui8PixelIndex;

        // loop through cover pixels to count used colors in the palette
        if (fseek(fpCover, ui32CoverPixelOffset, SEEK_SET) != 0)
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

        uint32_t ui32PaletteGroups = 1U << iBitsPerPixel;
        uint32_t ui32StegoPaletteSize = ui32ActivePaletteSize * ui32PaletteGroups;
        
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
        uint8_t *ui8Header = malloc(ui32CoverPaletteOffset);
        if (ui8Header == NULL)
        {
            printf("Error allocating memory for bitmap header.\n");
            return 1;
        }

        if (fseek(fpCover, 0, SEEK_SET) != 0 ||
            fread(ui8Header, 1, ui32CoverPaletteOffset, fpCover) != ui32CoverPaletteOffset)
        {
            printf("Error copying bitmap header from start of cover file.\n");
            free (ui8Header);
            return 1;
        }

        // store stego palette entries in biClrUsed field of header
        memcpy(&ui8Header[46], &ui32StegoPaletteSize, sizeof(ui32StegoPaletteSize));

        if (fwrite(ui8Header, 1, ui32CoverPaletteOffset, fpStego) != ui32CoverPaletteOffset)
        {
            printf("Error writing bitmap header to stego file.\n");
            free (ui8Header);
            return 1;
        }
        free (ui8Header);
        ui8Header = NULL;

        // write cover palette to Stego file 2^iBitsPerPixel times (one original, 2^iBitsPerPixel duplicates)
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
                    if (fseek(fpCover, ui32CoverPaletteOffset + (i * 4), SEEK_SET) != 0 ||
                        fread(ui8PaletteEntry, 1, 4, fpCover) != 4)
                    {
                        printf("Error copying palette entry in cover file.\n");
                        return 1;
                    }
                    if (fwrite(ui8PaletteEntry, 1, 4, fpStego) != 4)
                    {
                        printf("Error writing palette entry in stego file.\n");
                        return 1;
                    }
                    // pixel index i in cover file changes to newIndex in stego file
                    ui8CoverToStegoIndexRemap[paletteNum][i] = newIndex++; 
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
        uint64_t ui64MessageBitsHidden = 0;
        bool bHideSizeFinished = false;
        bool bHideMessageFinished = false;

        
        if (fseek(fpCover, ui32CoverPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to start of pixel data in cover file.\n");
            return 1;
        }
        if (fseek(fpStego, ui32CoverPixelOffset, SEEK_SET) != 0)
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
                                if (ui64MessageBitsHidden >= ui64TotalHideBits)
                                {
                                    bHideMessageFinished = true;
                                }
                                else if (fread(&ui8MessageByte, 1, 1, fpMessage) != 1)
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
                            ui64MessageBitsHidden++;

                            if (ui64MessageBitsHidden >= ui64TotalHideBits)
                            {
                                bHideMessageFinished = true;
                            }
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
        if (fpMessage != NULL)
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
        if (sStegoFile == NULL)
        {
            printf("Error: extraction requires a stego file.\n");
            printUsageHelp(argv[0]);
            return 1;
        }
        if (iBitsPerPixel < 1 || iBitsPerPixel > 3)
        {
            printf("Error: extraction requires specified bits per pixel.\n");
            printUsageHelp(argv[0]);
            return 1;
        }

        // open stego file
        FILE *fpStego = fopen(sStegoFile, "rb");
        if (fpStego == NULL)
        {
            printf("Error opening stego file %s\n", sStegoFile);
            return 1;
        }

        // get BMP fields from stego file
        uint8_t ui8FileSignature[2];
        uint16_t ui16StegoBitsPerPixel;
        uint32_t ui32StegoCompression;
        uint32_t ui32StegoPixelOffset;
        uint32_t ui32StegoHeaderSize;
        uint32_t ui32StegoPaletteOffset;
        int32_t i32StegoWidth;
        int32_t i32StegoHeight;
        uint32_t ui32StegoColorsUsed;

        if (fseek(fpStego, 0, SEEK_SET) != 0 ||
            fread(ui8FileSignature, 1, 2, fpStego) != 2 ||
            fseek(fpStego, 10, SEEK_SET) != 0 ||
            fread(&ui32StegoPixelOffset, sizeof(ui32StegoPixelOffset), 1, fpStego) != 1 ||
            fseek(fpStego, 14, SEEK_SET) != 0 ||
            fread(&ui32StegoHeaderSize, sizeof(ui32StegoHeaderSize), 1, fpStego) != 1 ||
            fseek(fpStego, 18, SEEK_SET) != 0 ||
            fread(&i32StegoWidth, sizeof(i32StegoWidth), 1, fpStego) != 1 ||
            fseek(fpStego, 22, SEEK_SET) != 0 ||
            fread(&i32StegoHeight, sizeof(i32StegoHeight), 1, fpStego) != 1 ||
            fseek(fpStego, 28, SEEK_SET) != 0 ||
            fread(&ui16StegoBitsPerPixel, sizeof(ui16StegoBitsPerPixel), 1, fpStego) != 1 ||
            fseek(fpStego, 30, SEEK_SET) != 0 ||
            fread(&ui32StegoCompression, sizeof(ui32StegoCompression), 1, fpStego) != 1 ||
            fseek(fpStego, 46, SEEK_SET) != 0 ||
            fread(&ui32StegoColorsUsed, sizeof(ui32StegoColorsUsed), 1, fpStego) != 1)
        {
            printf("Error reading stego bitmap header.\n");
            fclose(fpStego);
            return 1;
        }

        // check stego file is uncompressed 8-bit bitmap and has valid dimensions
        if (ui8FileSignature[0] != 'B' || ui8FileSignature[1] != 'M' ||
            ui16StegoBitsPerPixel != 8 || ui32StegoCompression != 0 ||
            i32StegoWidth <= 0 || i32StegoHeight == 0 || i32StegoHeight == INT32_MIN)
        {
            printf("Error: extraction requires a valid uncompressed 8-bit bitmap.\n");
            fclose(fpStego);
            return 1;
        }

        // check that palette and pixel offsets are valid
        ui32StegoPaletteOffset = 14U + ui32StegoHeaderSize;
        if (ui32StegoPixelOffset <= ui32StegoPaletteOffset)
        {
            printf("Error: invalid palette or pixel offset in stego bitmap.\n");
            fclose(fpStego);
            return 1;
        }

        uint32_t ui32StegoWidth = (uint32_t)i32StegoWidth;
        uint32_t ui32StegoHeight = (uint32_t)abs(i32StegoHeight);
        // determine row padding, bitmap pixel row data must be divisible by 4 bytes.
        uint32_t ui32PaddingSize = ((ui32StegoWidth + 3U) / 4U) * 4U - ui32StegoWidth;
        
        uint32_t ui32PaletteGroups = 1U << iBitsPerPixel;

        // biClrUsed stores the total number of stego palette entries
        if (ui32StegoColorsUsed == 0 || ui32StegoColorsUsed > 256U)
        {
            printf("Error: invalid stego palette size in biClrUsed.\n");
            fclose(fpStego);
            return 1;
        }

        // total palette size must divide evenly into 2^bits palette groups
        if (ui32StegoColorsUsed % ui32PaletteGroups != 0)
        {
            printf("Error: stego palette size is incompatible with %d bits per pixel.\n", iBitsPerPixel);
            fclose(fpStego);
            return 1;
        }

        uint32_t ui32ActivePaletteSize = ui32StegoColorsUsed / ui32PaletteGroups;

        if (ui32ActivePaletteSize == 0)
        {
            printf("Error: invalid active palette group size.\n");
            fclose(fpStego);
            return 1;
        }

        if (fseek(fpStego, ui32StegoPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to start of stego pixel data.\n");
            fclose(fpStego);
            return 1;
        }

        uint64_t ui64HiddenSizeBits = 0;
        uint64_t ui64SizeBitsRead = 0;
        uint64_t ui64PayloadBitsRead = 0;
        uint64_t ui64PixelCapacityBits = (uint64_t)ui32StegoWidth * (uint64_t)ui32StegoHeight * (uint64_t)iBitsPerPixel;

        for (uint32_t ui32Row = 0; ui32Row < ui32StegoHeight && ui64SizeBitsRead < 64U; ui32Row++)
        {
            for (uint32_t ui32Col = 0; ui32Col < ui32StegoWidth && ui64SizeBitsRead < 64U; ui32Col++)
            {
                uint8_t ui8PixelIndex;
                if (fread(&ui8PixelIndex, 1, 1, fpStego) != 1)
                {
                    printf("Error reading stego size field.\n");
                    fclose(fpStego);
                    return 1;
                }

                uint8_t ui8HiddenValue = (uint8_t)(ui8PixelIndex / ui32ActivePaletteSize);

                if (ui8HiddenValue >= ui32PaletteGroups)
                {
                    printf("Error: stego pixel index is outside duplicated palette groups.\n");
                    fclose(fpStego);
                    return 1;
                }

                for (int i = iBitsPerPixel - 1; i >= 0 && ui64SizeBitsRead < 64U; i--)
                {
                    ui64HiddenSizeBits = (ui64HiddenSizeBits << 1) | ((ui8HiddenValue >> i) & 1U);
                    ui64SizeBitsRead++;
                }
            }

            if (ui64SizeBitsRead < 64U && fseek(fpStego, ui32PaddingSize, SEEK_CUR) != 0)
            {
                printf("Error seeking past stego row padding.\n");
                fclose(fpStego);
                return 1;
            }
        }

        if (ui64SizeBitsRead != 64U || ui64HiddenSizeBits > ui64PixelCapacityBits - 64U)
        {
            printf("Error: invalid hidden message size.\n");
            fclose(fpStego);
            return 1;
        }

        char *sExtractedFileName = malloc(strlen(sStegoFile) + 32U);
        if (sExtractedFileName == NULL)
        {
            printf("Error allocating extracted filename.\n");
            fclose(fpStego);
            return 1;
        }

        FILE *fpExtracted = NULL;
        unsigned int uiExtractNumber;
        for (uiExtractNumber = 0; ; uiExtractNumber++)
        {
            snprintf(sExtractedFileName,
                     strlen(sStegoFile) + 32U,
                     "%s_extract_%02u",
                     sStegoFile,
                     uiExtractNumber);

            FILE *fpExisting = fopen(sExtractedFileName, "rb");
            if (fpExisting == NULL)
            {
                fpExtracted = fopen(sExtractedFileName, "wb");
                break;
            }
            fclose(fpExisting);
        }

        if (fpExtracted == NULL)
        {
            printf("Error opening extracted output file.\n");
            free(sExtractedFileName);
            fclose(fpStego);
            return 1;
        }

        // restart and skip exactly the first 64 hidden bits
        if (fseek(fpStego, ui32StegoPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to stego payload.\n");
            fclose(fpExtracted);
            remove(sExtractedFileName);
            free(sExtractedFileName);
            fclose(fpStego);
            return 1;
        }

        uint64_t ui64HiddenBitsSkipped = 0;
        uint8_t ui8OutputByte = 0;
        int iOutputBits = 0;

        for (uint32_t ui32Row = 0; ui32Row < ui32StegoHeight && ui64PayloadBitsRead < ui64HiddenSizeBits; ui32Row++)
        {
            for (uint32_t ui32Col = 0; ui32Col < ui32StegoWidth && ui64PayloadBitsRead < ui64HiddenSizeBits; ui32Col++)
            {
                uint8_t ui8PixelIndex;
                if (fread(&ui8PixelIndex, 1, 1, fpStego) != 1)
                {
                    printf("Error reading hidden payload.\n");
                    fclose(fpExtracted);
                    remove(sExtractedFileName);
                    free(sExtractedFileName);
                    fclose(fpStego);
                    return 1;
                }

                uint8_t ui8HiddenValue = (uint8_t)(ui8PixelIndex / ui32ActivePaletteSize);

                for (int i = iBitsPerPixel - 1; i >= 0; i--)
                {
                    if (ui64HiddenBitsSkipped < 64U)
                    {
                        ui64HiddenBitsSkipped++;
                        continue;
                    }

                    if (ui64PayloadBitsRead >= ui64HiddenSizeBits)
                        break;

                    ui8OutputByte = (uint8_t)((ui8OutputByte << 1) | ((ui8HiddenValue >> i) & 1U));
                    iOutputBits++;
                    ui64PayloadBitsRead++;

                    if (iOutputBits == 8)
                    {
                        if (fwrite(&ui8OutputByte, 1, 1, fpExtracted) != 1)
                        {
                            printf("Error writing extracted file.\n");
                            fclose(fpExtracted);
                            remove(sExtractedFileName);
                            free(sExtractedFileName);
                            fclose(fpStego);
                            return 1;
                        }
                        ui8OutputByte = 0;
                        iOutputBits = 0;
                    }
                }
            }

            if (ui64PayloadBitsRead < ui64HiddenSizeBits && fseek(fpStego, ui32PaddingSize, SEEK_CUR) != 0)
            {
                printf("Error seeking past stego row padding.\n");
                fclose(fpExtracted);
                remove(sExtractedFileName);
                free(sExtractedFileName);
                fclose(fpStego);
                return 1;
            }
        }

        if (iOutputBits > 0)
        {
            ui8OutputByte <<= (8 - iOutputBits);
            if (fwrite(&ui8OutputByte, 1, 1, fpExtracted) != 1)
            {
                printf("Error writing final extracted byte.\n");
                fclose(fpExtracted);
                remove(sExtractedFileName);
                free(sExtractedFileName);
                fclose(fpStego);
                return 1;
            }
        }

        if (ui64PayloadBitsRead != ui64HiddenSizeBits)
        {
            printf("Error: stego file ended before the hidden message was complete.\n");
            fclose(fpExtracted);
            remove(sExtractedFileName);
            free(sExtractedFileName);
            fclose(fpStego);
            return 1;
        }

        fclose(fpExtracted);
        fclose(fpStego);

        printf("Extracted %llu hidden bits to %s\n", (unsigned long long)ui64HiddenSizeBits, sExtractedFileName);
        free(sExtractedFileName);
    }
    else
    {
        printUsageHelp(argv[0]);
    }
    return 0;
}

