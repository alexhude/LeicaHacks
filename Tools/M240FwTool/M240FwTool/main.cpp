//
//  main.cpp
//  M240FwTool
//
//  Created by Alex Hude on 7/03/13.
//  Copyright (c) 2013 Alex Hude. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "md5.h"

// --------------

#define VERSION	"1.0"

#define WINDOW_SIZE		999
#define MINIMAL_BLOCK	4

#define Swap16(x)			((uint16_t)((((uint16_t)(x) & 0xff00) >> 8) \
								| (((uint16_t)(x) & 0x00ff) << 8)))

#define Swap32(x)			((uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) \
								| (((uint32_t)(x) & 0x00ff0000) >>  8) \
								| (((uint32_t)(x) & 0x0000ff00) <<  8) \
								| (((uint32_t)(x) & 0x000000ff) << 24)))

#define Swap64(x)			((uint64_t)((((uint64_t)(x) >> 56) & 0x000000FF) \
								|	(((uint64_t)(x) >> 40) & 0x0000FF00) \
								|	(((uint64_t)(x) >> 24) & 0x00FF0000) \
								|	(((uint64_t)(x) >> 8) &  0xFF000000) \
								|	(((uint64_t)(x) & 0xFF000000) <<  8) \
								|	(((uint64_t)(x) & 0x00FF0000) << 24) \
								|	(((uint64_t)(x) & 0x0000FF00) << 40) \
								|	(((uint64_t)(x) & 0x000000FF) << 56)))

#define VERBOSE(format, ...)	if (s_isVerbose) printf(format, ##__VA_ARGS__)

enum ToolFlags
{
	kToolFlag_Clear			= (0),
	kToolFlag_Extract		= (1 << 0),
	kToolFlag_Repack		= (1 << 1),
	kToolFlag_Create		= (1 << 2),
	
	kToolFlag_DumpBody		= (1 << 29),
	kToolFlag_TestPacker	= (1 << 30),
	kToolFlag_Verbose		= (1 << 31),
};

struct FWContainerHeader
{
	uint32_t	beginMagic;			// always 1E1CAF2E
	uint8_t		fwVersion[4];		// firmware version
	uint32_t	unknown2;
	uint32_t	fwPackedSize;		// compressed body size
	uint32_t	fwUnpackedSize;		// uncompressed body size
	uint8_t		fwPackedMd5[16];	// MD5 for compressed body
	uint32_t	fwBlocks;			// number of bodies
	uint32_t	fwBodyOffset;		// body offset / header size
	uint32_t	endMagic;			// always E1E350D1
};

struct FirmwareHeader
{
	uint32_t	unknown1;
	uint32_t	unknown2;
	uint32_t	unknown3;
	uint32_t	unknown4;
	uint32_t	unknown5;
	uint32_t	secCount;		// number of sections
	uint32_t	tableSize;		// section table size
	uint32_t	tableOffset;	// section table offset
	uint32_t	unknown7;
};

struct SectionHeader
{
	char		secName[20];	// section name
	uint32_t	unknown1;
	uint32_t	unknown2;
	uint8_t		secMd5[16];		// section data MD5
	uint32_t	secOffset;		// section data offset
	uint32_t	secSize;		// section data size
	uint32_t	unknown3;
	uint32_t	secImageBase;	// section image base
	uint32_t	unknown5;
	uint32_t	unknown6;
};

// --------------

static bool			s_isVerbose = false;
static uint32_t		s_toolFlags = kToolFlag_Clear;
static const char*	s_firmwareFolder = NULL;

// --------------

bool parce_container_hdr(uint8_t* in_buf, uint32_t in_size, FWContainerHeader* header)
{
	bool res = false;
	uint8_t* md5 = NULL;
	
	if (in_size < sizeof(FWContainerHeader))
		goto bail;

	if (header == NULL)
		goto bail;
	
	// Copy container header
	memcpy(header, in_buf, sizeof(FWContainerHeader));
	
	// Fix endianness
	header->fwPackedSize = Swap32(header->fwPackedSize);
	header->fwUnpackedSize = Swap32(header->fwUnpackedSize);
	header->fwBlocks = Swap32(header->fwBlocks);
	header->fwBodyOffset = Swap32(header->fwBodyOffset);
	
	md5 = header->fwPackedMd5;
	
	if (s_isVerbose)
	{
		printf("  version:       %d.%d.%d.%d\n", header->fwVersion[0], header->fwVersion[1], header->fwVersion[2], header->fwVersion[3]);
		printf("  packed size:   %8d | 0x%.8X\n", header->fwPackedSize, header->fwPackedSize);
		printf("  unpacked size: %8d | 0x%.8X\n", header->fwUnpackedSize, header->fwUnpackedSize);
		printf("  body blocks:   %8d | 0x%.8X\n", header->fwBlocks, header->fwBlocks);
		printf("  body offset:   %8d | 0x%.8X\n", header->fwBodyOffset, header->fwBodyOffset);
		printf("  MD5:           %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",
			   md5[0], md5[1], md5[2],  md5[3],  md5[4],  md5[5],  md5[6],  md5[7],
			   md5[8], md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);
	}
	
	res = check_md5(in_buf + header->fwBodyOffset, header->fwPackedSize, md5);
	if (res == false)
	{
		if(s_isVerbose)
			printf("  MD5 check:     FAILED\n");
		goto bail;
	}
	
	VERBOSE("  MD5 check:     PASSED\n");
	
	return true;
	
bail:
	
	return false;
}

bool update_container_hdr(uint8_t* in_buf, uint32_t in_size, FWContainerHeader* header)
{
	uint8_t* md5 = NULL;
	
	if (in_size < sizeof(FWContainerHeader))
		goto bail;
	
	if (header == NULL)
		goto bail;
	
	md5 = header->fwPackedMd5;
	get_md5(in_buf + header->fwBodyOffset, header->fwPackedSize, md5);
	
	if (s_isVerbose)
	{
		printf("  version:       %d.%d.%d.%d\n", header->fwVersion[0], header->fwVersion[1], header->fwVersion[2], header->fwVersion[3]);
		printf("  packed size:   %8d | 0x%.8X\n", header->fwPackedSize, header->fwPackedSize);
		printf("  unpacked size: %8d | 0x%.8X\n", header->fwUnpackedSize, header->fwUnpackedSize);
		printf("  body blocks:   %8d | 0x%.8X\n", header->fwBlocks, header->fwBlocks);
		printf("  body offset:   %8d | 0x%.8X\n", header->fwBodyOffset, header->fwBodyOffset);
		printf("  MD5:           %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",
			   md5[0], md5[1], md5[2],  md5[3],  md5[4],  md5[5],  md5[6],  md5[7],
			   md5[8], md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);
	}

	// Fix endianness back
	header->fwPackedSize = Swap32(header->fwPackedSize);
	header->fwUnpackedSize = Swap32(header->fwUnpackedSize);
	header->fwBlocks = Swap32(header->fwBlocks);
	header->fwBodyOffset = Swap32(header->fwBodyOffset);
	
	// Copy container header back
	memcpy(in_buf, header, sizeof(FWContainerHeader));

	return true;
	
bail:
	
	return false;
}

bool pack_body(uint8_t magic, uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t* out_size)
{
	uint32_t step = 0;

	uint32_t out_offset = 0;
	
	out_buf[out_offset++] = magic;
	
	for (uint32_t i=MINIMAL_BLOCK, j=0, seq=0, dif=0, acc=MINIMAL_BLOCK; i < in_size; )
	{
		if(s_isVerbose)
		{
			if (i > ((in_size/25)*step))
			{
				printf("◼"); fflush(NULL);
				step++;
			}
		}
		
		uint16_t walk_back = (i > WINDOW_SIZE)? WINDOW_SIZE : i;
		
		// search in the past
		for (j=MINIMAL_BLOCK; j <= walk_back; j++)
		{
			uint16_t match = 0;
			
			// find repeating sequence
			while ((i-j+match < i) && (i+match < in_size) && (in_buf[i-j+match] == in_buf[i+match]))
				match++;
			
			// choose biggest sequence
			if (match > seq)
			{
				seq = match;
				dif = j;
			}
		}
		
		// ignore short sequence
		uint8_t min_seq = MINIMAL_BLOCK;
		if (seq != 0)
		{
			if (seq > 0x7F)
				min_seq++;
			
			if (dif > 0x7F)
				min_seq++;
			
			if (seq < min_seq)
				seq = 0;
		}
		
		if (seq)
		{
			// copy previous data
			memcpy(&out_buf[out_offset], &in_buf[i-acc], acc);
			out_offset += acc;
			
			// put magic
			out_buf[out_offset++] = magic;
			
			// pack length
			if (seq > 0x7F)
			{
				out_buf[out_offset++] = ((seq >> 7) & 0x7) | 0x80;
			}
			out_buf[out_offset++] = seq & 0x7F;

			// pack offset
			if (dif > 0x7F)
			{
				out_buf[out_offset++] = ((dif >> 7) & 0x7) | 0x80;
			}
			out_buf[out_offset++] = dif & 0x7F;
			
			// adjust index
			i += seq;
			seq = 0;
			acc = 0;
		}
		else
		{
			if (in_buf[i] == magic)
			{
				// copy previous data
				memcpy(&out_buf[out_offset], &in_buf[i-acc], acc);
				out_offset += acc;
				
				// put magic
				out_buf[out_offset++] = magic;
				out_buf[out_offset++] = 0x00;
				
				// adjust index
				acc = 0;
				seq = 0;
			}
			else
			{
				acc++;
			}
			
			i++;
		}
		
		// put tail
		if (i == in_size && acc != 0)
		{
			// copy previous data
			memcpy(&out_buf[out_offset], &in_buf[i-acc], acc);
			out_offset += acc;
		}
	}

	*out_size = out_offset;
	
	if(s_isVerbose)
	{
		printf("\n"); fflush(NULL);
	
		printf("  %d -> %d\n", in_size, *out_size);
		printf("  Compression: DONE\n");
	}
	return true;
	
bail:
	
	VERBOSE("  Compression: FAILED\n");
		
	return false;
}

bool unpack_body(uint8_t* in_buf, uint32_t in_size, uint8_t* out_buf, uint32_t out_size)
{
	uint32_t step = 0;

	uint32_t out_offset = 0;
	uint8_t magic = in_buf[0];
	
	// unpack containter starting from byte 1 (skip 'magic')
	for (uint32_t i = 1; i < in_size;)
	{
		if(s_isVerbose)
		{
			if (i > ((in_size/25)*step))
			{
				printf("◼"); fflush(NULL);
				step++;
			}
		}
		
		if (in_buf[i] == magic)
		{
			uint8_t off_fix = 0;
			uint8_t sz = 2; // minimum length
			uint16_t len = 0;
			uint16_t offset = 0;
			
			if (in_buf[i+off_fix+1] == 0)
			{
				if (out_offset >= out_size)
					goto bail;
				
				out_buf[out_offset++] = in_buf[i];
				i += sz;
				
				continue;
			}
			
			sz++; // offset
			
			len = in_buf[i+off_fix+1];
			
			if (in_buf[i+off_fix+1] & 0x80)
			{
				sz++; // extended lendth
				
				len &= 0x7F;
				len <<= 7;
				len += in_buf[i+off_fix+2];
				off_fix++;
			}
			
			offset = in_buf[i+off_fix+2];
			
			if (in_buf[i+off_fix+2] & 0x80)
			{
				sz++; // extended offset
				offset &= 0x7F;
				offset <<= 7;
				offset += in_buf[i+off_fix+3];
			}
			
			if (out_offset >= out_size)
				goto bail;
			
			memcpy(&out_buf[out_offset], &out_buf[out_offset-offset], len);
			out_offset += len;
			i += sz;
		}
		else
		{
			if (out_offset >= out_size)
				goto bail;
			
			out_buf[out_offset++] = in_buf[i];
			i++;
		}
	}
	
	if(s_isVerbose)
	{
		printf("\n"); fflush(NULL);
		
		printf("  %d -> %d\n", in_size, out_size);
		printf("  Uncompression: DONE\n");
	}
	
	return true;
	
bail:
	
	VERBOSE("  Uncompression: FAILED\n");
	return false;
}

bool split_container(uint8_t* in_buf, uint32_t in_size)
{
	bool res = false;
	uint8_t* md5 = NULL;
	uint32_t bodyOffset = 0;
	struct stat st;
	
	FirmwareHeader fwHeader;
	SectionHeader secHeader;
	
	// Copy firmware header
	memcpy(&fwHeader, in_buf, sizeof(FirmwareHeader));

	// Fix endianness
	fwHeader.secCount = Swap32(fwHeader.secCount);
	fwHeader.tableSize = Swap32(fwHeader.tableSize);
	fwHeader.tableOffset = Swap32(fwHeader.tableOffset);
	
	if(s_isVerbose)
	{
		printf("  Number of sections:   %8d | 0x%.8X\n", fwHeader.secCount, fwHeader.secCount);
		printf("  Section table size:   %8d | 0x%.8X\n", fwHeader.tableSize, fwHeader.tableSize);
		printf("  Section table offset: %8d | 0x%.8X\n", fwHeader.tableOffset, fwHeader.tableOffset);
	}
	
	in_buf += fwHeader.tableOffset;
	
	bodyOffset += fwHeader.tableSize;
	bodyOffset += 4; // skip DA 7A DA 7A
	
	if (stat(s_firmwareFolder, &st) == -1)
		mkdir(s_firmwareFolder, 0700);
	
	// Parse section table
	for (uint32_t cnt = 0; cnt < fwHeader.secCount; cnt++)
	{
		uint8_t* section = NULL;
		uint32_t size = 0;

		char filename[256] = {0};
		FILE* out_file = NULL;

		// Copy section header
		memcpy(&secHeader, in_buf + (cnt * sizeof(SectionHeader)), sizeof(SectionHeader));

		// Fix endianness
		secHeader.secOffset = Swap32(secHeader.secOffset);
		secHeader.secSize = Swap32(secHeader.secSize);
		secHeader.secImageBase = Swap32(secHeader.secImageBase);
		
		section = in_buf + bodyOffset + secHeader.secOffset;
		size = secHeader.secSize;
		md5 = secHeader.secMd5;
		
		if(s_isVerbose)
		{
			printf("  Section %d\n", cnt+1);
			printf("    Section Name:   \"%s\"\n", secHeader.secName);
			printf("    Section offset: %8d | 0x%.8X\n", secHeader.secOffset, secHeader.secOffset);
			printf("    Section size:   %8d | 0x%.8X\n", secHeader.secSize, secHeader.secSize);
			printf("    Section base:   %8d | 0x%.8X\n", secHeader.secImageBase, secHeader.secImageBase);
			printf("    MD5:            %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",
				   md5[0], md5[1], md5[2],  md5[3],  md5[4],  md5[5],  md5[6],  md5[7],
				   md5[8], md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);
		}
		
		res = check_md5(section, size, md5);
		if (res == false)
		{
			if(s_isVerbose)
				printf("    MD5 check:      FAILED\n");
			goto bail;
		}
		if(s_isVerbose)
			printf("    MD5 check:      PASSED\n");

		strcpy(filename, s_firmwareFolder);
		strcat(filename, "/");
		if (strncmp(secHeader.secName, "[A]", 3) == 0)
			strcat(filename, secHeader.secName + 3);
		else
			strcat(filename, secHeader.secName);
		strcat(filename, ".bin");
		
		if(s_isVerbose)
			printf("    Create file:    %s\n", filename);

		// Write file to disk
		out_file = fopen(filename, "w");
		fwrite(section, 1, size, out_file);
		fclose(out_file);
	}
	
	VERBOSE("  Splitting container: DONE\n");
	
	return true;
	
bail:
	
	return false;
}

bool update_container(uint8_t* in_buf, uint32_t in_size)
{
	bool res = false;
	uint8_t* md5 = NULL;
	uint32_t bodyOffset = 0;
	
	FirmwareHeader fwHeader;
	SectionHeader secHeader;
	
	// Copy firmware header
	memcpy(&fwHeader, in_buf, sizeof(FirmwareHeader));
	
	// Fix endianness
	fwHeader.secCount = Swap32(fwHeader.secCount);
	fwHeader.tableSize = Swap32(fwHeader.tableSize);
	fwHeader.tableOffset = Swap32(fwHeader.tableOffset);
	
	if(s_isVerbose)
	{
		printf("  Number of sections:   %8d | 0x%.8X\n", fwHeader.secCount, fwHeader.secCount);
		printf("  Section table size:   %8d | 0x%.8X\n", fwHeader.tableSize, fwHeader.tableSize);
		printf("  Section table offset: %8d | 0x%.8X\n", fwHeader.tableOffset, fwHeader.tableOffset);
	}
	
	in_buf += fwHeader.tableOffset;
	
	bodyOffset += fwHeader.tableSize;
	bodyOffset += 4; // skip DA 7A DA 7A
	
	// Parse section table
	for (uint32_t cnt = 0; cnt < fwHeader.secCount; cnt++)
	{
		uint8_t* section = NULL;
		uint32_t size = 0;
		
		char filename[256] = {0};
		FILE* in_file = NULL;
		uint32_t in_file_size = 0;
		
		// Copy section header
		memcpy(&secHeader, in_buf + (cnt * sizeof(SectionHeader)), sizeof(SectionHeader));
		
		// Fix endianness
		secHeader.secOffset = Swap32(secHeader.secOffset);
		secHeader.secSize = Swap32(secHeader.secSize);
		secHeader.secImageBase = Swap32(secHeader.secImageBase);
		
		section = in_buf + bodyOffset + secHeader.secOffset;
		size = secHeader.secSize;
		md5 = secHeader.secMd5;
		
		if(s_isVerbose)
		{
			printf("  Section %d\n", cnt+1);
			printf("    Section Name:   \"%s\"\n", secHeader.secName);
		}
		
		strcpy(filename, s_firmwareFolder);
		strcat(filename, "/");
		if (strncmp(secHeader.secName, "[A]", 3) == 0)
			strcat(filename, secHeader.secName + 3);
		else
			strcat(filename, secHeader.secName);
		strcat(filename, ".bin");
		
		// Open file
		in_file = fopen(filename, "r");
		if (in_file == NULL)
		{
			if(s_isVerbose)
				printf("    SECTION FILE %s NOT FOUND!\n", filename);
			goto bail;
		}
		
		if (s_isVerbose)
		{
			printf("    Section offset: %8d | 0x%.8X\n", secHeader.secOffset, secHeader.secOffset);
			printf("    Section size:   %8d | 0x%.8X\n", secHeader.secSize, secHeader.secSize);
			printf("    Section base:   %8d | 0x%.8X\n", secHeader.secImageBase, secHeader.secImageBase);		
		}
		
		fseek(in_file, 0L, SEEK_END);
		in_file_size = (uint32_t)ftell(in_file);
		
		// Verify size
		if (in_file_size != size)
		{
			if(s_isVerbose)
				printf("    SECTION SIZE DOES NOT MATCH (old != new | %d != %d)\n", size, in_file_size);
			goto bail;
		}

		if (s_isVerbose)
		{
			printf("    old MD5:        %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",
				   md5[0], md5[1], md5[2],  md5[3],  md5[4],  md5[5],  md5[6],  md5[7],
				   md5[8], md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);
		}
		
		res = check_md5(section, size, md5);
		if (res == false)
		{
			if(s_isVerbose)
				printf("    old MD5 check:  FAILED\n");
			goto bail;
		}
		if(s_isVerbose)
			printf("    old MD5 check:  PASSED\n");
		
		// Read file from disk
		fseek(in_file, 0L, SEEK_SET);
		fread(section, 1, size, in_file);
		fclose(in_file);

		get_md5(section, size, md5); // updates secHeader.secMd5 as well
		if (s_isVerbose)
		{
			printf("    new MD5:        %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",
				   md5[0], md5[1], md5[2],  md5[3],  md5[4],  md5[5],  md5[6],  md5[7],
				   md5[8], md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);
		}

		// Fix endianness back
		secHeader.secOffset = Swap32(secHeader.secOffset);
		secHeader.secSize = Swap32(secHeader.secSize);
		secHeader.secImageBase = Swap32(secHeader.secImageBase);
		
		// Copy section header back
		memcpy(in_buf + (cnt * sizeof(SectionHeader)), &secHeader, sizeof(SectionHeader));
	}
	
	VERBOSE("  Updating container: DONE\n");
	
	return true;
	
bail:
	
	return false;
}

bool assamble_container(uint8_t* in_buf, uint32_t in_size)
{
	return true;
}

void extract_firmware(const char* filename)
{
	bool res = true;
	
	FILE* in_file = NULL;
	
	uint8_t* in_buf = NULL;
	uint8_t* up_buf = NULL;
	uint8_t* pk_buf = NULL;
	
	uint32_t in_size = 0;
	uint32_t up_size = 0;
	uint32_t pk_size = 0;
	uint32_t offset = 0;
	
	FWContainerHeader container_hdr;

	VERBOSE("\nOpen firmware file: %s\n", filename);
	in_file = fopen(filename, "r");
	
	fseek(in_file, 0L, SEEK_END);
	in_size = (uint32_t)ftell(in_file);
	
	in_buf = (uint8_t*)malloc(in_size);

	// read packed firmware
	fseek(in_file, 0L, SEEK_SET);
	fread(in_buf, 1, in_size, in_file);
	
	fclose(in_file);
	
	VERBOSE("  File size: %8d | 0x%.8X\n", in_size, in_size);
	
	VERBOSE("\nParse container header:\n");
	res = parce_container_hdr(in_buf, in_size, &container_hdr);
	if (res == false)
		goto bail;

	offset = container_hdr.fwBodyOffset;
	in_size = container_hdr.fwPackedSize;
	up_size = container_hdr.fwUnpackedSize;
	up_buf = (uint8_t*)malloc(up_size);

	if(s_isVerbose)
		{ printf("\nUncompress container body: "); fflush(NULL); }
	res = unpack_body(in_buf + offset, in_size, up_buf, up_size);
	if (res == false)
		goto bail;

	if (s_toolFlags & kToolFlag_DumpBody)
	{
		// dump to disk
		FILE* dump_file = fopen("FW_BODY_DUMP.FW", "w");
		fwrite(up_buf, 1, up_size, dump_file);
		fclose(dump_file);
	}
	
	if (s_toolFlags & kToolFlag_TestPacker)
	{
		pk_buf = (uint8_t*)malloc(up_size);
		pk_size = 0;
		
		if(s_isVerbose)
			{ printf("\nCompress  container  body: "); fflush(NULL); }
		res = pack_body(in_buf[offset], up_buf, up_size, pk_buf, &pk_size);
		if (res == false)
		{
			if(s_isVerbose)
				printf("  pack_body() failed!\n");
			goto bail;
		}

		if (pk_size != in_size)
		{
			if(s_isVerbose)
				printf("Packer test failed (%d != %d)!\n", pk_size, in_size);
			goto bail;
		}
		
		for (uint32_t i = 0; i < in_size; i++)
		{
			if (pk_buf[i] != in_buf[offset+i])
			{
				if(s_isVerbose)
					printf("Packer test FAILED buf[%d]: %.2X != %.2X)!\n", i, pk_buf[i], in_buf[offset+i]);
				goto bail;
			}
		}
		
		if(s_isVerbose)
			printf("Packer test PASSED.\n");
	}
	
	VERBOSE("\nSplit container:\n");
	res = split_container(up_buf, up_size);
	if (res == false)
		goto bail;

bail:

	if (pk_buf)
		free(pk_buf);
	
	if (up_buf)
		free(up_buf);
	
	if (in_buf)
		free(in_buf);

	if (res == false)
	{
		printf("Extraction FAILED!\n");
		
		if(s_isVerbose == false)
			printf("Use [-v] option to identify a problem.\n");
	}
	else
	{
		printf("Extraction COMPLETE!\n");
	}
	
	return;
}

void repack_firmware(const char* filename, const char* original)
{
	bool res = true;
	
	FILE* in_file = NULL;
	FILE* out_file = NULL;
	
	uint8_t* in_buf = NULL;
	uint8_t* up_buf = NULL;
	uint8_t* out_buf = NULL;
	
	uint32_t in_size = 0;
	uint32_t up_size = 0;
	uint32_t out_size = 0;
	uint32_t offset = 0;
	
	FWContainerHeader container_hdr;
	
	VERBOSE("\nOpen original file: %s\n", original);
	in_file = fopen(original, "r");
	
	fseek(in_file, 0L, SEEK_END);
	in_size = (uint32_t)ftell(in_file);
	
	in_buf = (uint8_t*)malloc(in_size);
	
	// read packed firmware
	fseek(in_file, 0L, SEEK_SET);
	fread(in_buf, 1, in_size, in_file);
	
	fclose(in_file);
	
	VERBOSE("  File size: %8d | 0x%.8X\n", in_size, in_size);
	
	VERBOSE("\nParse container header:\n");
	res = parce_container_hdr(in_buf, in_size, &container_hdr);
	if (res == false)
		goto bail;
	
	offset = container_hdr.fwBodyOffset;
	in_size = container_hdr.fwPackedSize;
	up_size = container_hdr.fwUnpackedSize;
	up_buf = (uint8_t*)malloc(up_size);
	
	if(s_isVerbose)
		{ printf("\nUncompress container body: "); fflush(NULL); }
	res = unpack_body(in_buf + offset, in_size, up_buf, up_size);
	if (res == false)
		goto bail;
	
	VERBOSE("\nUpdate container body:\n");
	res = update_container(up_buf, up_size);
	if (res == false)
		goto bail;
	
	if (s_toolFlags & kToolFlag_DumpBody)
	{
		// dump to disk
		FILE* dump_file = fopen("FW_BODY_DUMP.FW", "w");
		fwrite(up_buf, 1, up_size, dump_file);
		fclose(dump_file);
	}
	
	out_size = up_size;
	out_buf = (uint8_t*)malloc(out_size);
	
	if(s_isVerbose)
		{ printf("\nCompress  container  body: "); fflush(NULL); }
	res = pack_body(in_buf[offset], up_buf, up_size, out_buf + offset, &out_size);
	if (res == false)
		goto bail;
	
	// update packed size
	container_hdr.fwPackedSize = out_size;
	out_size += sizeof(FWContainerHeader);
	
	VERBOSE("\nUpdate container header:\n");
	res = update_container_hdr(out_buf, out_size, &container_hdr);
	if (res == false)
		goto bail;
	
	VERBOSE("\nWrite firmware file: %s\n", filename);
	out_file = fopen(filename, "w");
	fwrite(out_buf, 1, out_size, out_file);
	fclose(out_file);
	
	VERBOSE("  File size: %8d | 0x%.8X\n", out_size, out_size);

bail:
	
	if (out_buf)
		free(out_buf);
	
	if (up_buf)
		free(up_buf);
	
	if (in_buf)
		free(in_buf);
	
	if (res == false)
	{
		printf("Repacking FAILED!\n");
		
		if(s_isVerbose == false)
			printf("Use [-v] option to identify a problem.\n");
	}
	else
	{
		printf("Repacking COMPLETE!\n");
	}
	
	return;
}

void create_firmware(const char* filename)
{
	printf("Sorry, new firmware file creation isn't implemented yet, because\nnot all firmware structure fields were identified.\n");

bail:
	
	return;
}

static void usage(const char* progName)
{
	printf("Leica M (typ 240) Firmware Tool v%s\n", VERSION);	
	printf("Usage: %s [-erc] [-f folder] [-o filename] [-d] [-v] [-t] FIRMWARE.FW\n\n", progName);
	printf("With this tool you can extract, repack or create Leica M (typ 240) firmware files.\n");
	printf("Use following arguments:\n");
	printf("    -e            Unpack firmware and extract all parts into folder\n");
	printf("    -r            Repack firmware from folder using original firmware file\n");
	printf("    -c            Create firmware file from folder\n");
	printf("    -f folder     Specify folder name\n");	
	printf("    -o filename   Original firmware file for repacking\n");	
	printf("    -d            Dump unpacked firmware body to disk\n");
	printf("    -t            Test packer algorithm after unpacking firmware\n");
	printf("    -v            Be verbose\n");
	printf("\n");
	printf("Default action: [-e] (extract to default folder)\n");
	printf("Default folder: M240_FIRMWARE\\\n");
	printf("\n");
	printf("Examples:\n");
	printf("  Extract firmware to default folder:\n");
	printf("   $ M240FwTool FIRMWARE.FW\n");
	printf("\n");
	printf("  Extract firmware with log and packer test (recommended):\n");
	printf("   $ M240FwTool -e -f FOLDER -v -t FIRMWARE.FW\n");
	printf("\n");
	printf("  Repack firmware from folder:\n");
	printf("   $ M240FwTool -r -f FOLDER -o ORIGINAL.FW -v -t FIRMWARE.FW\n");
	printf("\n");
	
	exit(EXIT_FAILURE);
}

int main(int argc, char* const argv[])
{
	int ch;
	const char* firmwareFileName = NULL;
	const char* originalFileName = NULL;
	const char* progName = argv[0];
	struct stat st;
	
	// Set defaults
	s_isVerbose = false;
	s_toolFlags = kToolFlag_Clear;
	s_firmwareFolder = NULL;
	
	// Get options
	while ((ch = getopt(argc, argv, "ercf:o:dtv")) != -1)
	{
		switch (ch)
		{
			case 'e':
				s_toolFlags |= kToolFlag_Extract;
				break;
			case 'r':
				s_toolFlags |= kToolFlag_Repack;
				break;
			case 'c':
				s_toolFlags |= kToolFlag_Create;
				break;
			case 'f':
				s_firmwareFolder = optarg;
				break;
			case 'o':
				originalFileName = optarg;
				break;
			case 'd':
				s_toolFlags |= kToolFlag_DumpBody;
				break;
			case 't':
				s_toolFlags |= kToolFlag_TestPacker;
				break;
			case 'v':
				s_toolFlags |= kToolFlag_Verbose;
				break;
			default:
				usage(progName);
		}
	}

	// Get firmware file name
	argc -= optind;
	argv += optind;
	
	if (argc != 1)
		usage(progName);
	
	firmwareFileName = argv[0];

	// Validate parameters
	if (s_toolFlags & kToolFlag_Verbose)
		s_isVerbose = true;
	
	if ((s_toolFlags & 0xFF) == kToolFlag_Clear)
		s_toolFlags |= kToolFlag_Extract;
	
	if (s_firmwareFolder == NULL)
		s_firmwareFolder = "M240_FIRMWARE";
	
	if (s_firmwareFolder[0] == '-')
		usage(progName);

	if (originalFileName && originalFileName[0] == '-')
		usage(progName);
	
	if ((s_toolFlags & kToolFlag_Repack) && (s_toolFlags & kToolFlag_TestPacker))
		s_toolFlags &= ~kToolFlag_TestPacker;

	if (s_toolFlags & kToolFlag_Repack)
	{
		if (originalFileName == NULL)
		{
			printf("  Unable to repack firmware: missing original firmware file (use [-o]).\n");
			return 0;
		}
		
		if (stat(s_firmwareFolder, &st) == -1)
		{
			printf("  Unable to repack firmware: firmware folder %s\\ not found.\n", s_firmwareFolder);
			return 0;
		}
	}
		
	// Display parameters
	printf("Running with options:\n");
	printf("  + firmware folder: %s\n", s_firmwareFolder);

	if (s_toolFlags & kToolFlag_DumpBody)
		printf("  + firmware body dump enabled\n");
	if (s_toolFlags & kToolFlag_TestPacker)
		printf("  + packer test enabled\n");
	if (s_toolFlags & kToolFlag_Verbose)
		printf("  + verbose enabled\n");
	
	// Perform action
	if (s_toolFlags & kToolFlag_Extract)
	{
		extract_firmware(firmwareFileName);
	}
	else if (s_toolFlags & kToolFlag_Repack)
	{
		repack_firmware(firmwareFileName, originalFileName);
	}
	else if (s_toolFlags & kToolFlag_Create)
	{
		create_firmware(firmwareFileName);
	}

    return 0;
}

