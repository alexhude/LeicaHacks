//
//  main.cpp
//  xortool
//
//  Created by Alexander Hude on 25/02/13.
//  Copyright © 2013 Alexander Hude. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define kMaxKeySize 1024

void dump_key(uint8_t* key, size_t size)
{
    printf("{\n//\t0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F\n\n\t");
    for (uint32_t i = 0; i < size; ++i)
    {
        printf("0x%02X, ", key[i]);
        if ((i % 16) == 15) printf(" // 0x%.3X (%4d)\n\t", i - 15, i - 15);
    }
    printf("\n\n//\t0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F\n}\n");
}

uint32_t get_key_length(uint8_t* buf, size_t size)
{
    uint32_t index = 0;
    uint32_t ncmp = 0;
    uint32_t offset = 0;
    
    const size_t maxkey = kMaxKeySize;
    uint32_t length[kMaxKeySize*2][2] = {{0},{0}};
    
    for(uint32_t start = 0; start < size; )
    {
        index = start + 1;
        size_t bound = start + (maxkey*2);
        if (bound > size)
            bound = size;
        
        // find repeating patterns
        while (index < bound)
        {
            if (buf[start] == buf[index])
            {
                ncmp = 1;
                
                while (((index + ncmp) < bound) && (buf[start + ncmp] == buf[index + ncmp]))
                {
                    ncmp++;
                }
                
                if (ncmp > 2)
                {
                    length[index - start][0]++;
                    if (length[index - start][1] < ncmp)
                        length[index - start][1] = ncmp;
                    
                    if (ncmp >= maxkey && offset == 0)
                        offset = start;
                }
            }
            
            index++;
        }
        
        start++;
    }
    
    uint32_t cnt = 0, res = 0;
    for (uint32_t i = 0; i < maxkey; i++)
    {
        if (length[i][0] > cnt)
        {
            res = i;
            cnt = length[i][0];
        }
    }
    
    printf("possible length: %d (0x%X)\n", res, res);
    return res;
}

typedef uint32_t keystat_t[kMaxKeySize][256];

void get_key(uint8_t* buf, size_t size, uint8_t* key, uint32_t ksize)
{
    // brutforce all key lengths if ksize is zero
    uint32_t keysize = (ksize)? ksize : 128;
    uint32_t maxkey = kMaxKeySize;
    keystat_t* keystat = (keystat_t*)malloc(sizeof(keystat_t));
    
    for (; keysize <= maxkey; keysize++)
    {
        memset((*keystat), 0, sizeof(keystat_t));
        
        // get statistics
        // how many times byte appears in current posittion of the key (most popular key bytes)
        for (size_t start = 0; start < size; )
        {
            size_t read = keysize;
            if (read > (size - start))
                read = (size - start);
            
            for (uint32_t i = 0; i < read; i++)
            {
                (*keystat)[i][buf[start + i]]++;
            }
            
            start += keysize;
        }
        
        // analyse
        uint32_t bad = 0;
        for (uint32_t i = 0; i < keysize; i++)
        {
            uint32_t pmax = 0;
            uint32_t max = 0;
            
            // get max
            for (uint32_t j = 0; j < 256; j++)
            {
                if ((*keystat)[i][j] > max)
                {
                    max = (*keystat)[i][j];
                }
            }
            
            // get previous max
            for (uint32_t j = 0; j < 256; j++)
            {
                if ((*keystat)[i][j] == max)
                    continue;
                
                if ((*keystat)[i][j] > pmax)
                {
                    pmax = (*keystat)[i][j];
                }
            }
            
            // get difference
            float diff = (float)pmax/(float)max;
            if (diff > 0.5f)
                bad++;
            
            if (bad == 10)
                break;
        }
        
        if (bad == 0)
        {
            printf("[%4d|0x%.3X] possible key:\n", keysize, keysize);
            
            // retrieve key
            for (uint32_t i = 0; i < keysize; i++)
            {
                uint8_t v = 0;
                uint32_t max = 0;
                
                for (uint32_t j = 0; j < 256; j++)
                {
                    if ((*keystat)[i][j] > max)
                    {
                        max = (*keystat)[i][j];
                        v = j;
                    }
                }
                
                key[i] = v; // create key from most popular chars
            }

            dump_key(key, keysize);
            if (ksize)
                break;
        }
        else if (bad < 10)
        {
            printf("[%4d|0x%.3X] possible key (badness = %d):\n", keysize, keysize, bad);
            
            // retrieve key
            uint8_t* k = (uint8_t*)malloc(keysize);
            memset(k, 0, keysize);
            
            for (uint32_t i = 0; i < keysize; i++)
            {
                uint8_t v = 0;
                uint32_t max = 0;
                
                for (uint32_t j = 0; j < 256; j++)
                {
                    if ((*keystat)[i][j] > max)
                    {
                        max = (*keystat)[i][j];
                        v = j;
                    }
                }
                
                k[i] = v;
            }
            
            dump_key(k, keysize);
            
            free(k);
        }
        else
        {
            printf("[%4d|0x%.3X] nothing\n", keysize, keysize);
        }
    }
    
    FILE* ofile = fopen("key.bin", "w");
    fwrite(key, ksize, 1, ofile);
    fclose(ofile);
    
    free(keystat);
}


int main(int argc, const char * argv[]) {
    if(argc != 2) {
        printf("Usage: %s fw_file\n", argv[0]);
        printf("   ex: %s ./m9-1_196_dec.upd\n", argv[0]);
        return -1;
    }
    
    const char* ifilepath = argv[1];
    FILE* ifile = fopen(ifilepath, "r");
    if (ifile == nullptr)
        return -1;
    
    fseek(ifile, 0L, SEEK_END);
    size_t isize = ftell(ifile);
    fseek(ifile, 0L, SEEK_SET);
    
    uint8_t* ibuf = (uint8_t*)malloc(isize);
    fread(ibuf, 1, isize, ifile);
    fclose(ifile);

    // get key
    printf("getting key length (this may take a while)...\n");
    uint8_t key[kMaxKeySize];
    uint32_t ksize = get_key_length(ibuf, isize);
    
    // extract key and decrypt firmware
    if (ksize)
    {
        printf("getting key (this may also take a while)...\n");
        get_key(ibuf, isize, key, ksize);

        printf("decrypting firmware...\n");
        char* ofilepath = strdup(ifilepath);
        strcat(ofilepath, ".dec");
        
        FILE* ofile = fopen(ofilepath, "w");
        
        size_t bytes = 0;
        for (uint32_t i = 0; i < isize; i += bytes)
        {
            bytes = ksize;
            if (isize - i < ksize)
                bytes = isize - i;
            
            for (uint32_t j = 0; j < bytes; j++)
            {
                ibuf[j] ^= key[j];
            }
            
            fwrite(ibuf, 1, bytes, ofile);
        }
        
        fclose(ofile);
        free(ofilepath);
        
        printf("done\n");
    }
    else
    {
        printf("invalid key length!\n");
    }

    free(ibuf);
    return 0;
}
