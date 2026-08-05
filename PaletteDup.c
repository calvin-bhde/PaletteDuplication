#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

// PaletteDup.c - Steganography using palette duplication in 8-bit paletted bitmaps
// by Project Group 4 - Calvin Bulger, Andrew Martin, Izhin Talamantez

void printUsageHelp(char * prog)
{
    // remove full file path from program name if present
    char *sProgramName = strrchr(prog, '\\');
    char *sForwardSlash = strrchr(prog, '/');
    if (sForwardSlash != NULL && (sProgramName == NULL || sForwardSlash > sProgramName))
        sProgramName = sForwardSlash;
    if (sProgramName != NULL)
        prog = sProgramName + 1;
    
    printf("Usage: %s [operation] [options]\n", prog);
    printf("\n");
    printf("Operations:\n");
    printf("   -hide                         To hide file data inside an 8-bit paletted bitmap\n");
    printf("   -extract                      To extract previously hidden data\n");
    printf("   -capacity                     Displays color palette capacity for hiding data with different bits per pixel\n");
    printf("\n");
    printf("Options:\n");
    printf("   -m <message file|random>      File to hide, \"random\" will randomize hidden message\n");
    printf("   -c <cover file>               8-bit paletted bitmap file to hide in\n");
    printf("   -o <output file>              Stego file name to output (optional when hiding)\n");
    printf("   -s <stego file>               Stego file for extraction\n");
    printf("   -b <bits per pixel (1-3)>     Number of bits hidden per pixel in stego file\n");
    printf("   -mod                          Modify duplicate colors in palette to reduce color duplication\n");
    printf("\n");
    printf("To hide file data inside an 8-bit paletted bitmap:\n");
    printf("   %s -hide -m <message file> -c <cover file> -b <1|2|3> [-mod] [-o <stego output file>]\n", prog);
    printf("\n");
    printf("To extract previously hidden data:\n");
    printf("   %s -extract -s <stego file> -b <1|2|3>\n", prog);
    printf("\n");
    printf("To display color palette capacity for a candidate cover file:\n");
    printf("   %s -capacity -c <cover file>\n", prog);
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
    char *sOutputFile = NULL;
    char *sStegoFile = NULL;
    int iBitsPerPixel = 0;
    bool bIsRandomMessage = false;
    bool bModifyDuplicateColors = false;

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
        else if (strcmp(argv[i], "-s") == 0) // stego file
        {
            i++;
            if (i < argc)
                sStegoFile = argv[i];
            else
            {
                printf("Error in arguments: missing stego file.\n");
                printf("   -s <stego file>\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-o") == 0) // output file
        {
            i++;
            if (i < argc)
                sOutputFile = argv[i];
            else
            {
                printf("Error in arguments: missing output file.\n");
                printf("   -o <output file>\n");
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
        else if (strcmp(argv[i], "-mod") == 0)
        {
            bModifyDuplicateColors = true;
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

        FILE *fpCover = NULL;
        FILE *fpMessage = NULL;
        FILE *fpOutput = NULL;
        uint8_t *ui8Header = NULL;
        int iHideResult = 1;

        // open cover file
        fpCover = fopen(sCoverFile, "rb");
        if (fpCover == NULL)
        {
            printf("Error opening cover file %s\n", sCoverFile);
            return 1;
        }

        // open message file if provided
        if (!bIsRandomMessage)
        {
            fpMessage = fopen(sMessageFile, "rb");
            if(fpMessage == NULL)
            {
                printf("Error opening message file %s\n", sMessageFile);
                goto hide_cleanup;
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
            goto hide_cleanup;
        }
        ui32CoverPaletteOffset = 14U + ui32CoverHeaderSize;

        // check cover file is uncompressed 8-bit bitmap and has valid dimensions
        if (ui8FileSignature[0] != 'B' || ui8FileSignature[1] != 'M' || 
            ui16CoverBitsPerPixel != 8 || ui32CoverCompression != 0 ||
            i32CoverWidth <= 0 || i32CoverHeight == 0 || i32CoverHeight == INT32_MIN)
        {
            printf("Error: hiding requires an uncompressed 8-bit bitmap cover file.\n");
            goto hide_cleanup;
        }
        

        uint32_t ui32CoverWidth = (uint32_t)i32CoverWidth;
        uint32_t ui32CoverHeight = (uint32_t)abs(i32CoverHeight);
        // determine row padding, bitmap pixel row data must be divisible by 4 bytes.
        uint32_t ui32PaddingSize = ((ui32CoverWidth + 3U) / 4U) * 4U - ui32CoverWidth;

        bool bCoverIndexUsed[256] = {false};
        uint32_t ui32ActivePaletteSize = 0;
        uint8_t ui8PixelIndex;

        const uint8_t ui8ColorModMask[8] =
        {
            0x00, // unchanged
            0x01, // flip R
            0x02, // flip G
            0x04, // flip B
            0x03, // flip R + G
            0x06, // flip G + B
            0x05, // flip R + B
            0x07  // flip R + G + B
        };

        // loop through cover pixels to count used colors in the palette
        if (fseek(fpCover, ui32CoverPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to PixelOffset in cover file.\n");
            goto hide_cleanup;
        }
        for (uint32_t ui32Row = 0; ui32Row < ui32CoverHeight; ui32Row++)
        {
            for (uint32_t ui32Col = 0; ui32Col < ui32CoverWidth; ui32Col++)
            {
                if (fread(&ui8PixelIndex, sizeof(ui8PixelIndex), 1, fpCover) != 1)
                {
                    printf("Error reading bitmap pixel data.\n");
                    goto hide_cleanup;
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
                goto hide_cleanup;
            }
        }

        uint32_t ui32PaletteGroups = 1U << iBitsPerPixel;
        uint32_t ui32StegoPaletteSize = ui32ActivePaletteSize * ui32PaletteGroups;
        uint32_t ui32StegoPixelOffset = ui32CoverPaletteOffset + (ui32StegoPaletteSize * 4U);
        if (ui32StegoPaletteSize > 256U)
        {
            printf("Error: %u active colors cannot support %d-bit hiding. The duplicated palette would require %u entries.\n", ui32ActivePaletteSize, iBitsPerPixel, ui32StegoPaletteSize);
            goto hide_cleanup;
        }
        
        // if output filename provided, open for write. else generate name to open
        char sGeneratedOutputFileName[256];
        if (sOutputFile == NULL)
        {
            snprintf(sGeneratedOutputFileName, sizeof(sGeneratedOutputFileName), "%s_hide_%d.bmp", sCoverFile, iBitsPerPixel);
            sOutputFile = sGeneratedOutputFileName;
        }
        fpOutput = fopen(sOutputFile, "wb");
        if (fpOutput == NULL)
        {
            printf("Error opening output file for writing %s\n", sOutputFile);
            goto hide_cleanup;
        }

        // write cover header into stego file
        ui8Header = malloc(ui32CoverPaletteOffset);
        if (ui8Header == NULL)
        {
            printf("Error allocating memory for bitmap header.\n");
            goto hide_cleanup;
        }

        if (fseek(fpCover, 0, SEEK_SET) != 0 ||
            fread(ui8Header, 1, ui32CoverPaletteOffset, fpCover) != ui32CoverPaletteOffset)
        {
            printf("Error copying bitmap header from start of cover file.\n");
            goto hide_cleanup;
        }
        uint32_t ui32RowSize = ui32CoverWidth + ui32PaddingSize;
        uint32_t ui32StegoImageSize = ui32RowSize * ui32CoverHeight;
        uint32_t ui32StegoFileSize = ui32StegoPixelOffset + ui32StegoImageSize;

        // store stego pixel offset, stego image size, and stego palette size in header
        memcpy(&ui8Header[2], &ui32StegoFileSize, sizeof(ui32StegoFileSize));
        memcpy(&ui8Header[10], &ui32StegoPixelOffset, sizeof(ui32StegoPixelOffset));
        memcpy(&ui8Header[34], &ui32StegoImageSize, sizeof(ui32StegoImageSize));
        memcpy(&ui8Header[46], &ui32StegoPaletteSize, sizeof(ui32StegoPaletteSize));

        if (fwrite(ui8Header, 1, ui32CoverPaletteOffset, fpOutput) != ui32CoverPaletteOffset)
        {
            printf("Error writing bitmap header to output file.\n");
            goto hide_cleanup;
        }

        // write cover palette to Stego file 2^iBitsPerPixel times (one original, 2^iBitsPerPixel - 1 duplicates)
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
                    if (fseek(fpCover, ui32CoverPaletteOffset + ((uint32_t)i * 4U), SEEK_SET) != 0 ||
                        fread(ui8PaletteEntry, 1, 4, fpCover) != 4)
                    {
                        printf("Error copying palette entry in cover file.\n");
                        goto hide_cleanup;
                    }
                    if (bModifyDuplicateColors && paletteNum > 0)
                    {
                        uint8_t ui8ModMask = ui8ColorModMask[paletteNum];
                        if ((ui8ModMask & 0x01U) != 0)
                            ui8PaletteEntry[2] ^= 0x01U; // flip R
                        if ((ui8ModMask & 0x02U) != 0)
                            ui8PaletteEntry[1] ^= 0x01U; // flip G
                        if ((ui8ModMask & 0x04U) != 0)
                            ui8PaletteEntry[0] ^= 0x01U; // flip B
                    }
                    if (fwrite(ui8PaletteEntry, 1, 4, fpOutput) != 4)
                    {
                        printf("Error writing palette entry in output file.\n");
                        goto hide_cleanup;
                    }
                    // pixel index i in cover file changes to newIndex in output file
                    ui8CoverToStegoIndexRemap[paletteNum][i] = (uint8_t)newIndex;
                    newIndex++; 
                }
            }
        }


        // get total size of hidden data
        uint64_t ui64TotalHideBits = 0;
        uint64_t ui64OriginalMessageBits = 0;
        uint64_t ui64CoverCapacityBits = (uint64_t)ui32CoverWidth * (uint64_t)ui32CoverHeight * (uint64_t)iBitsPerPixel;
        if (ui64CoverCapacityBits < 64U)
        {
            printf("Error: cover file is too small to hold size field.\n");
            goto hide_cleanup;
        } 
        ui64CoverCapacityBits -= 64U;

        if (!bIsRandomMessage)
        {
            if (fseek(fpMessage, 0, SEEK_END) != 0)
            {
                printf("Error seeking in message file.\n");
                goto hide_cleanup;
            }
            long lMessageSize = ftell(fpMessage);
            if (lMessageSize < 0)
            {
                printf("Error getting size of message file.\n");
                goto hide_cleanup;
            }
            ui64OriginalMessageBits = (uint64_t)lMessageSize * 8U;
            ui64TotalHideBits = (ui64OriginalMessageBits < ui64CoverCapacityBits) ? ui64OriginalMessageBits : ui64CoverCapacityBits;
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
            goto hide_cleanup;
        }
        if (fseek(fpOutput, ui32StegoPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to start of pixel data in output file.\n");
            goto hide_cleanup;
        }
        if (!bIsRandomMessage)
            if (fseek(fpMessage, 0, SEEK_SET) != 0)
            {
                printf("Error seeking to start of message file.\n");
                goto hide_cleanup;
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
                    goto hide_cleanup;
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
                    if (fwrite(&ui8StegoPixelIndex, sizeof(ui8StegoPixelIndex), 1, fpOutput) != 1)
                    {
                        printf("Error writing hidden pixel to output file.\n");
                        goto hide_cleanup;
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
                                        goto hide_cleanup;
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
                    if (fwrite(&ui8StegoPixelIndex, sizeof(ui8StegoPixelIndex), 1, fpOutput) != 1)
                    {
                        printf("Error writing hidden pixel to output file.\n");
                        goto hide_cleanup;
                    }
                }
                else
                {
                    // entire message hidden, write remaining cover to output file
                    if (fwrite(&ui8StegoPixelIndex, 1, 1, fpOutput) != 1)
                    {
                        printf("Error writing pixel to output file.\n");
                        goto hide_cleanup;
                    }
                }
            }

            for (uint32_t i = 0; i < ui32PaddingSize; i++)
            {
                uint8_t ui8PaddingByte;
                if (fread(&ui8PaddingByte, 1, 1, fpCover) != 1)
                {
                    printf("Error reading padding byte from cover file.\n");
                    goto hide_cleanup;
                }
                if (fwrite(&ui8PaddingByte, 1, 1, fpOutput) != 1)
                {
                    printf("Error writing padding byte to output file.\n");
                    goto hide_cleanup;
                }
            }
        }

        //printf("Palette Duplication with %d bits performed successfully and saved at %s\n", iBitsPerPixel, sStegoFile);
        // print more specific message
        if (bIsRandomMessage)
        {
            printf("Successfully hid %" PRIu64 " bits of random data in %s using %d bits per pixel.\n", ui64TotalHideBits, sOutputFile, iBitsPerPixel);
        }
        else if (ui64CoverCapacityBits < ui64OriginalMessageBits)
        {
            printf("Successfully hid %" PRIu64 " bits of data from %s in %s using %d bits per pixel. Note: cover file capacity was exceeded, only %" PRIu64 " out of %" PRIu64 " bits (%.2f%%) were hidden.\n", ui64TotalHideBits, sMessageFile, sOutputFile, iBitsPerPixel, ui64CoverCapacityBits, ui64OriginalMessageBits, (double)ui64CoverCapacityBits / ui64OriginalMessageBits * 100);
        }
        else
        {
            printf("Successfully hid %" PRIu64 " bits of data from %s in %s using %d bits per pixel.\n", ui64MessageBitsHidden, sMessageFile, sOutputFile, iBitsPerPixel);
        }

        iHideResult = 0;

        hide_cleanup:

        if (ui8Header != NULL)
        {
            free(ui8Header);
            ui8Header = NULL;
        }

        if (fpMessage != NULL)
        {
            fclose(fpMessage);
            fpMessage = NULL;
        }

        if (fpOutput != NULL)
        {
            fclose(fpOutput);
            fpOutput = NULL;
        }

        if (fpCover != NULL)
        {
            fclose(fpCover);
            fpCover = NULL;
        }

        if (iHideResult != 0)
        {
            // remove incomplete output file created before the error
            if (sOutputFile != NULL)
                remove(sOutputFile);
            return 1;
        }

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

        FILE *fpStego = NULL;
        FILE *fpExtracted = NULL;
        FILE *fpExisting = NULL;
        char *sExtractedFileName = NULL;
        int iExtractResult = 1;

        // open stego file
        fpStego = fopen(sStegoFile, "rb");
        if (fpStego == NULL)
        {
            printf("Error opening stego file %s\n", sStegoFile);
            goto extract_cleanup;
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
            goto extract_cleanup;
        }

        // check stego file is uncompressed 8-bit bitmap and has valid dimensions
        if (ui8FileSignature[0] != 'B' || ui8FileSignature[1] != 'M' ||
            ui16StegoBitsPerPixel != 8 || ui32StegoCompression != 0 ||
            i32StegoWidth <= 0 || i32StegoHeight == 0 || i32StegoHeight == INT32_MIN)
        {
            printf("Error: extraction requires a valid uncompressed 8-bit bitmap.\n");
            goto extract_cleanup;
        }

        // check that palette and pixel offsets are valid
        ui32StegoPaletteOffset = 14U + ui32StegoHeaderSize;
        if (ui32StegoPixelOffset <= ui32StegoPaletteOffset)
        {
            printf("Error: invalid palette or pixel offset in stego bitmap.\n");
            goto extract_cleanup;
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
            goto extract_cleanup;
        }

        // total palette size must divide evenly into 2^bits palette groups
        if (ui32StegoColorsUsed % ui32PaletteGroups != 0)
        {
            printf("Error: stego palette size is incompatible with %d bits per pixel.\n", iBitsPerPixel);
            goto extract_cleanup;
        }

        uint32_t ui32ActivePaletteSize = ui32StegoColorsUsed / ui32PaletteGroups;

        if (ui32ActivePaletteSize == 0)
        {
            printf("Error: invalid active palette group size.\n");
            goto extract_cleanup;
        }

        if (fseek(fpStego, ui32StegoPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to start of stego pixel data.\n");
            goto extract_cleanup;
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
                    goto extract_cleanup;
                }

                uint8_t ui8HiddenValue = (uint8_t)(ui8PixelIndex / ui32ActivePaletteSize);

                if (ui8HiddenValue >= ui32PaletteGroups)
                {
                    printf("Error: stego pixel index is outside duplicated palette groups.\n");
                    goto extract_cleanup;
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
                goto extract_cleanup;
            }
        }

        if (ui64SizeBitsRead != 64U || ui64HiddenSizeBits > ui64PixelCapacityBits - 64U)
        {
            printf("Error: invalid hidden message size.\n");
            goto extract_cleanup;
        }

        sExtractedFileName = malloc(strlen(sStegoFile) + 32U);
        if (sExtractedFileName == NULL)
        {
            printf("Error allocating extracted filename.\n");
            goto extract_cleanup;
        }

        fpExtracted = NULL;
        unsigned int uiExtractNumber;
        for (uiExtractNumber = 0; ; uiExtractNumber++)
        {
            snprintf(sExtractedFileName,
                     strlen(sStegoFile) + 32U,
                     "%s_extract_%02u",
                     sStegoFile,
                     uiExtractNumber);

            fpExisting = fopen(sExtractedFileName, "rb");
            if (fpExisting == NULL)
            {
                fpExtracted = fopen(sExtractedFileName, "wb");
                break;
            }

            fclose(fpExisting);
            fpExisting = NULL;
        }

        if (fpExtracted == NULL)
        {
            printf("Error opening extracted output file.\n");
            goto extract_cleanup;
        }

        // restart and skip exactly the first 64 hidden bits
        if (fseek(fpStego, ui32StegoPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to stego payload.\n");
            goto extract_cleanup;
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
                    goto extract_cleanup;
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
                            goto extract_cleanup;
                        }
                        ui8OutputByte = 0;
                        iOutputBits = 0;
                    }
                }
            }

            if (ui64PayloadBitsRead < ui64HiddenSizeBits && fseek(fpStego, ui32PaddingSize, SEEK_CUR) != 0)
            {
                printf("Error seeking past stego row padding.\n");
                goto extract_cleanup;
            }
        }

        if (iOutputBits > 0)
        {
            ui8OutputByte <<= (8 - iOutputBits);
            if (fwrite(&ui8OutputByte, 1, 1, fpExtracted) != 1)
            {
                printf("Error writing final extracted byte.\n");
                goto extract_cleanup;
            }
        }

        if (ui64PayloadBitsRead != ui64HiddenSizeBits)
        {
            printf("Error: stego file ended before the hidden message was complete.\n");
            goto extract_cleanup;
        }
        printf("Extracted %llu hidden bits to %s\n", (unsigned long long)ui64HiddenSizeBits, sExtractedFileName);
        
        iExtractResult = 0;
        
        extract_cleanup:

        if (fpExisting != NULL)
        {
            fclose(fpExisting);
            fpExisting = NULL;
        }
        if (fpExtracted != NULL)
        {
            fclose(fpExtracted);
            fpExtracted = NULL;
        }
        if (fpStego != NULL)
        {
            fclose(fpStego);
            fpStego = NULL;
        }
        if (iExtractResult != 0 && sExtractedFileName != NULL)
        {
            // remove incomplete output file created before the error
            remove(sExtractedFileName);
        }

        if (sExtractedFileName != NULL)
        {
            free(sExtractedFileName);
            sExtractedFileName = NULL;
        }
        if (iExtractResult != 0)
        {
            return 1;
        }

    }
    else if (strcmp(argv[1], "-capacity") == 0)
    {
        if (sCoverFile == NULL)
        {
            printf("Error: capacity requires a cover file.\n");
            printUsageHelp(argv[0]);
            return 1;
        }

        int iCapacityResult = 1;
        FILE *fpCover = fopen(sCoverFile, "rb");
        if (fpCover == NULL)
        {
            printf("Error opening cover file %s\n", sCoverFile);
            goto capacity_cleanup;
        }

        uint8_t  ui8FileSignature[2];
        uint16_t ui16CoverBitsPerPixel;
        uint32_t ui32CoverCompression;
        uint32_t ui32CoverPixelOffset;
        int32_t  i32CoverWidth;
        int32_t  i32CoverHeight;

        if (fseek(fpCover, 0, SEEK_SET) != 0 ||
            fread(ui8FileSignature, 1, 2, fpCover) != 2 ||
            fseek(fpCover, 10, SEEK_SET) != 0 ||
            fread(&ui32CoverPixelOffset, sizeof(ui32CoverPixelOffset), 1, fpCover) != 1 ||
            fseek(fpCover, 18, SEEK_SET) != 0 ||
            fread(&i32CoverWidth, sizeof(i32CoverWidth), 1, fpCover) != 1 ||
            fseek(fpCover, 22, SEEK_SET) != 0 ||
            fread(&i32CoverHeight, sizeof(i32CoverHeight), 1, fpCover) != 1 ||
            fseek(fpCover, 28, SEEK_SET) != 0 ||
            fread(&ui16CoverBitsPerPixel, sizeof(ui16CoverBitsPerPixel), 1, fpCover) != 1 ||
            fseek(fpCover, 30, SEEK_SET) != 0 ||
            fread(&ui32CoverCompression, sizeof(ui32CoverCompression), 1, fpCover) != 1)
        {
            printf("Error reading bitmap header.\n");
            goto capacity_cleanup;
        }

        if (ui8FileSignature[0] != 'B' ||
            ui8FileSignature[1] != 'M' ||
            ui16CoverBitsPerPixel != 8 ||
            ui32CoverCompression != 0 ||
            i32CoverWidth <= 0 ||
            i32CoverHeight == 0 ||
            i32CoverHeight == INT32_MIN)
        {
            printf("Error: capacity requires an uncompressed 8-bit bitmap cover file.\n");
            goto capacity_cleanup;
        }

        uint32_t ui32CoverWidth = (uint32_t)i32CoverWidth;
        uint32_t ui32CoverHeight = (uint32_t)abs(i32CoverHeight);

        uint32_t ui32PaddingSize = ((ui32CoverWidth + 3U) / 4U) * 4U - ui32CoverWidth;

        bool bCoverIndexUsed[256] = {false};
        uint32_t ui32ActivePaletteSize = 0;

        if (fseek(fpCover, ui32CoverPixelOffset, SEEK_SET) != 0)
        {
            printf("Error seeking to bitmap pixel data.\n");
            goto capacity_cleanup;
        }

        for (uint32_t ui32Row = 0;
            ui32Row < ui32CoverHeight;
            ui32Row++)
        {
            for (uint32_t ui32Col = 0;
                ui32Col < ui32CoverWidth;
                ui32Col++)
            {
                uint8_t ui8PixelIndex;

                if (fread(&ui8PixelIndex, 1, 1, fpCover) != 1)
                {
                    printf("Error reading bitmap pixel data.\n");
                    goto capacity_cleanup;
                }

                if (!bCoverIndexUsed[ui8PixelIndex])
                {
                    bCoverIndexUsed[ui8PixelIndex] = true;
                    ui32ActivePaletteSize++;
                }
            }

            if (fseek(fpCover, ui32PaddingSize, SEEK_CUR) != 0)
            {
                printf("Error seeking past bitmap row padding.\n");
                goto capacity_cleanup;
            }
        }


        int iMaxBitsPerPixel = 0;

        if (ui32ActivePaletteSize <= 32U) iMaxBitsPerPixel = 3;
        else if (ui32ActivePaletteSize <= 64U) iMaxBitsPerPixel = 2;
        else if (ui32ActivePaletteSize <= 128U) iMaxBitsPerPixel = 1;

        printf("File: %s\n", sCoverFile);

        if (iMaxBitsPerPixel == 0)
        {
            printf("Active Colors: %u (too many for palette duplication)\n", ui32ActivePaletteSize);
            iCapacityResult = 0;
            goto capacity_cleanup;
        }

        printf("Active Colors: %u (max %d-bit hiding)\n", ui32ActivePaletteSize, iMaxBitsPerPixel);

        uint64_t ui64PixelCount = (uint64_t)ui32CoverWidth * (uint64_t)ui32CoverHeight;

        for (int iBits = 1; iBits <= iMaxBitsPerPixel; iBits++)
        {
            uint64_t ui64CapacityBits =
                ui64PixelCount * (uint64_t)iBits;

            // the first 64 hidden bits store the message size
            uint64_t ui64CapacityBytes = 0;

            if (ui64CapacityBits > 64U)
                ui64CapacityBytes = (ui64CapacityBits - 64U) / 8U;

            printf("-b %d capacity: %llu bytes\n", iBits, (unsigned long long)ui64CapacityBytes);
        }

        iCapacityResult = 0;

        capacity_cleanup:
        if (fpCover != NULL)
        {
            fclose(fpCover);
            fpCover = NULL;
        }
        if (iCapacityResult != 0)
        {
            return 1;
        }
    }
    else
    {
        printUsageHelp(argv[0]);
    }
    return 0;
}

