//
//  main.cpp
//  pwadsplit
//
//  Created by Alexander Hude on 20/02/13.
//  Copyright © 2013 Alexander Hude. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct pwad_header
{
    uint32_t    magic;
    uint32_t    sizeHigh;
    uint32_t    sizeLow;
};

struct pwad_entry
{
    uint32_t    offset;
    uint32_t    size;
    char        name[8];
    
};

bool pwad_split(uint32_t level, char* path, uint8_t* buf, size_t size)
{
    char indent[] = "          ";
    char newpath[64] = {0};
    
    uint8_t* offset = buf;
    
    pwad_entry* entry = (pwad_entry*)(buf + sizeof(pwad_header));
    uint8_t* firstEntry = buf + entry->offset;
    
    offset += sizeof(pwad_header);
    
    for (; offset < firstEntry; )
    {
        printf("%s%s:\t0x%.8X (%8d:0x%.8X)\n", indent + (sizeof(indent)-1-level), entry->name, entry->offset, entry->size, entry->size);
        
        pwad_header* tmpHead = (pwad_header*)(buf + entry->offset);
        pwad_entry* tmpEntry = (pwad_entry*)(buf + entry->offset + sizeof(pwad_header));
        
        if ((tmpHead->magic == 'DAWP') && (((tmpHead->sizeHigh << 4) | tmpHead->sizeLow) == tmpEntry->offset))
        {
            sprintf(newpath, "%s/%s/", path, entry->name);
            mkdir(newpath, 0700);
            pwad_split(level+1, newpath, buf + entry->offset, entry->size);
        }
        else
        {
            sprintf(newpath, "%s%s.bin", path, entry->name);
            
            FILE* out_file = fopen(newpath, "w");
            
            fwrite(buf + entry->offset, 1, entry->size, out_file);
            
            fclose(out_file);
        }
        
        offset += sizeof(pwad_entry);
        entry = (pwad_entry*)offset;
    }
    
    return true;
}

int main(int argc, const char * argv[]) {
    if(argc != 3) {
        printf("Usage: %s fw_file folder\n", argv[0]);
        printf("   ex: %s ./m8-2_005.upd m8-2_005\n", argv[0]);
        return -1;
    }
    
    const char* ifilepath = argv[1];
    const char* ofolderpath = argv[2];
    FILE* ifile = fopen(ifilepath, "r");
    if (ifile == nullptr)
        return -1;

    fseek(ifile, 0L, SEEK_END);
    size_t isize = ftell(ifile);
    fseek(ifile, 0L, SEEK_SET);

    uint8_t* ibuf = (uint8_t*)malloc(isize);
    fread(ibuf, 1, isize, ifile);
    fclose(ifile);
    
    pwad_header* head = (pwad_header*)ibuf;
    pwad_entry* entry = (pwad_entry*)(ibuf + sizeof(pwad_header));
    
    // check root header
    if ((head->magic != 'DAWP') || (((head->sizeHigh << 4) | head->sizeLow) != entry->offset))
    {
        printf("invalid firmware file!\n");
        return -1;
    }

    char newpath[64] = {0};
    sprintf(newpath, "%s/", ofolderpath);
    
    mkdir(newpath, 0700);
    
    pwad_split(0, newpath, ibuf, isize);

    free(ibuf);
    return 0;
}
