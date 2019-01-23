//
//  main.cpp
//  FRLibTool
//
//  Created by Alex Hude on 15/05/13.
//  Copyright (c) 2013 Alex Hude. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>

// -------------- MARK: Macro definitions

#define VERSION	"1.0"

//#define USE_FIRST_MATCH
#define SHOW_CLOSE_MATCH

#define	TIMESTAMP_LENGTH	14
#define	TIMESTAMP_LENGTH_NT	TIMESTAMP_LENGTH + 1	// NULL terminated timestamp
#define	MAX_NAME_LENGTH		32
#define MATCH_THRESHOLD		8

#define FR_LIB_MAGIC		0xE0003801
#define FR_OBJECTLIST_MAGIC	0xE2
#define FR_IMPORTLIST_MAGIC	0xE4

#define  BX_(x)         ((x) - (((x)>>1)&0x77777777)                    \
							 - (((x)>>2)&0x33333333)                    \
							 - (((x)>>3)&0x11111111))


#define BitCount(x)     (((BX_(x)+(BX_(x)>>4)) & 0x0F0F0F0F) % 255)

#define Swap16(x)		((uint16_t)((((uint16_t)(x) & 0xff00) >> 8) \
								| (((uint16_t)(x) & 0x00ff) << 8)))

#define Swap32(x)		((uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) \
								| (((uint32_t)(x) & 0x00ff0000) >>  8) \
								| (((uint32_t)(x) & 0x0000ff00) <<  8) \
								| (((uint32_t)(x) & 0x000000ff) << 24)))

#define Swap64(x)		((uint64_t)((((uint64_t)(x) >> 56) & 0x000000FF) \
								|	(((uint64_t)(x) >> 40) & 0x0000FF00) \
								|	(((uint64_t)(x) >> 24) & 0x00FF0000) \
								|	(((uint64_t)(x) >> 8) &  0xFF000000) \
								|	(((uint64_t)(x) & 0xFF000000) <<  8) \
								|	(((uint64_t)(x) & 0x00FF0000) << 24) \
								|	(((uint64_t)(x) & 0x0000FF00) << 40) \
								|	(((uint64_t)(x) & 0x000000FF) << 56)))

#define OffsetOf(type, field)	((uint32_t) ( (char *)&((type *)(0))->field - (char *)0 ))

#define VERBOSE(format, ...)	if (s_isVerbose) printf(format, ##__VA_ARGS__)

// -------------- MARK: FRLibTool definitions

enum ToolFlags
{
	kToolFlag_Clear			= (0),
	kToolFlag_UseScanOffset	= (1 << 0),
	kToolFlag_UseImageBase	= (1 << 1),
	kToolFlag_OutputList	= (1 << 2),
	kToolFlag_OutputPAT		= (1 << 3),
	kToolFlag_OutputIDC		= (1 << 4),
	kToolFlag_OutputPython	= (1 << 5),
	kToolFlag_OneFunciton	= (1 << 7),

	kToolFlag_DumpLib		= (1 << 30),
	kToolFlag_Verbose		= (1 << 31),
};

// -------------- MARK: FR Library definitions

struct FRLibHeader
{
	uint32_t	magic;
	char		timestamp1[TIMESTAMP_LENGTH];
	char		timestamp2[TIMESTAMP_LENGTH];
	uint32_t	objectCount;
	uint32_t	importCount;
	uint32_t	size;
	uint32_t	mcuNameLength;
	char		mcuName[16];
} __attribute__((aligned(1), packed));

struct FRLibObjectListItem
{
	char		timestamp[TIMESTAMP_LENGTH_NT];
	uint32_t	offset;
	uint32_t	size;
	uint32_t	unknown;
	uint32_t	nameLength;
	char		name[MAX_NAME_LENGTH];
} __attribute__((aligned(1), packed));
static const uint32_t kFuncListItemSize = OffsetOf(FRLibObjectListItem, name);

struct FRLibImportListItem
{
	uint32_t	index;
	uint32_t	nameLength;
	char		name[MAX_NAME_LENGTH];
} __attribute__((aligned(1), packed));
static const uint32_t kImportListItemSize = OffsetOf(FRLibImportListItem, name);

// -------------- MARK: Object definitions

enum ObjectBlockTypes
{
	kObjectHeader		= 0x81,
	kObjectDictonary	= 0x82,
	kObjectBuildInfo	= 0x84,
	kObjectImportLibs	= 0x85,
	kObjectDescriptor	= 0x90,
	kObjectDataDesc		= 0x92,
	kObjectImportTable	= 0x94,
	kObjectExportInfo	= 0x96,
	kObjectSeparator	= 0x97,
	kObjectDataPrefix	= 0xA0,
	kObjectData			= 0xA2,
	kObjectDataRefTable	= 0xA4,
	kObjectTerminator	= 0xFF
};

enum ObjectDataTypes
{
	kObjectData_Init,
	kObjectData_Code,
	kObjectData_Const,
	kObjectData_Data,
	kObjectData_Stack,
};
typedef uint8_t ObjectDataType;

enum ObjectRefTypes
{
	kObjectRef_Function	= 0x6,
	kObjectRef_Data		= 0xB,
	kObjectRef_Label	= 0xC,
};

struct FRLibObjectBlock
{
	uint8_t		type;
	uint32_t	size;
} __attribute__((aligned(1), packed));

struct FRLibObjectHeader : FRLibObjectBlock
{
	// ...
	// uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectDictonary : FRLibObjectBlock
{
	// ...
	// uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectBuildInfo : FRLibObjectBlock
{
	uint16_t	unknown1;
	char		timestamp[TIMESTAMP_LENGTH_NT];
	uint8_t		unknown2[22];
	uint32_t	frcomOffset;
	uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectImportLibs : FRLibObjectBlock
{
	uint32_t	libCount;
	uint32_t	libNameOffset;
	uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectDescriptor : FRLibObjectBlock
{
	uint8_t		unknown1;
	uint32_t	dataCount;
	uint32_t	importCount;
	uint32_t	unknown2;
	uint32_t	unknown3;
	uint32_t	fr20Offset;
	char		timestamp[TIMESTAMP_LENGTH_NT];
	uint8_t		unknown4[10];
	uint32_t	fileNameOffset;
	uint8_t		chechsum;
} __attribute__((aligned(1), packed));

struct FRLibObjectDataDesc : FRLibObjectBlock
{
	uint8_t		unknown1[6];
	uint32_t	fullDataSize;
	uint8_t		unknown2[6];
	uint32_t	dataTypeOffset;
	uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectImportTableItem
{
	uint8_t		alwaysFF;
	uint32_t	funcNameOffset;
} __attribute__((aligned(1), packed));

struct FRLibObjectImportTable : FRLibObjectBlock
{
	// FRLibObjectImportTableItem	table[];
	// uint8_t						checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectExportInfo : FRLibObjectBlock
{
	uint8_t		unknown1[9];
	uint32_t	objNameOffset;
	uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectSeparator : FRLibObjectBlock
{
	uint8_t		unknown1[4];
	uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectDataPrefix : FRLibObjectBlock
{
	uint32_t	unknown1;
	uint32_t	dataIndex;
	uint32_t	unknown2;
	uint8_t		checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectData : FRLibObjectBlock
{
	uint32_t	dataOffset;
	uint32_t	dataSize;
	// uint8_t	data[];
	// uint8_t	checksum;
} __attribute__((aligned(1), packed));

struct FRLibRefTableItem
{
	uint8_t		refSize;
	uint32_t	refOffset;
	uint8_t		unknown1[6];
	uint32_t	subItemSize;
} __attribute__((aligned(1), packed));

struct FRLibRefItemImport : FRLibRefTableItem
{
	uint8_t		importType;
	uint32_t	importIndex;
	uint8_t		alwaysFF;
} __attribute__((aligned(1), packed));

struct FRLibRefItemData : FRLibRefTableItem
{
	uint8_t		unknown2[8];
	uint8_t		offset;
	uint8_t		always20;
	uint8_t		alwaysFF;
} __attribute__((aligned(1), packed));

struct FRLibRefItemLabel : FRLibRefTableItem
{
	uint8_t		unknown2[8];
	uint16_t	offset;
	uint8_t		always20;
	uint8_t		alwaysFF;
} __attribute__((aligned(1), packed));

struct FRLibObjectDataRefTable : FRLibObjectBlock
{
	// FRLibRefTableItem	table[];
	// uint8_t				checksum;
} __attribute__((aligned(1), packed));

struct FRLibObjectTerminator : FRLibObjectBlock
{
	uint8_t		checksum;
} __attribute__((aligned(1), packed));

// -------------- MARK: FRLibTool structures

struct FRLibToolListHeader
{
	uint8_t		magic;
	uint32_t	size;
} __attribute__((aligned(1), packed));

struct FRLibToolObjectList : FRLibToolListHeader
{
	FRLibObjectListItem*	list;
} __attribute__((aligned(1), packed));

struct FRLibToolImportList : FRLibToolListHeader
{
	FRLibImportListItem*	list;
} __attribute__((aligned(1), packed));

struct FRLibToolObjectData
{
	uint32_t				size;
	uint8_t*				data;
	char*					import;
	uint32_t				offset;
	FRLibToolObjectData*	next;
} __attribute__((aligned(1), packed));

struct FRLibToolObject
{
	char*					exportName;
	uint32_t				dataSize;
	uint32_t				dataCount;
	FRLibToolObjectData*	dataFirst;
	FRLibToolObjectData*	dataLast;
} __attribute__((aligned(1), packed));

// -------------- MARK: Static definitions

static bool			s_isVerbose = false;
static uint32_t		s_toolFlags = kToolFlag_Clear;
static uint32_t		s_scanOffset = 0;
static uint32_t		s_imageBase = 0xFFFFFFFF;
static uint32_t		s_funcIndex = 0xFFFFFFFF;

// -------------- MARK: Implementation

unsigned short crc16(unsigned char *data_p, size_t length);

const char* datatype2str(ObjectDataType type)
{
	switch (type)
	{
		case kObjectData_Init:
			return "INIT";
		case kObjectData_Code:
			return "CODE";
		case kObjectData_Const:
			return "CONST";
		case kObjectData_Data:
			return "DATA";
		case kObjectData_Stack:
			return "STACK";
		default:
			return "";
	}
}

FRLibToolObjectData*	create_data_item(uint32_t size, uint8_t* data, char* import, uint32_t offset)
{
	FRLibToolObjectData* dataItem = (FRLibToolObjectData*)malloc(sizeof(FRLibToolObjectData));
	
	dataItem->size = size;
	dataItem->data = data;
	dataItem->import = import;
	dataItem->offset = offset;
	dataItem->next = NULL;
	
	return dataItem;
}

void delete_data_item(FRLibToolObjectData* dataItem)
{
	if (dataItem)
	{
		dataItem->size = NULL;
		dataItem->data = NULL;
		dataItem->import = NULL;
		dataItem->offset = 0;
		dataItem->next = NULL;
		
		free(dataItem);
	}
}

void reset_data_object(FRLibToolObject* object)
{
	if (object == NULL)
		return;
	
	FRLibToolObjectData* item = object->dataFirst;
	
	while (item)
	{
		FRLibToolObjectData* next = item->next;
		free(item);
		item = next;
	};
	
	object->dataSize = 0;
	object->dataCount = 0;
	object->dataFirst = NULL;
	object->dataLast = NULL;
}

bool parse_library_header(uint8_t* lib_buf, uint32_t lib_size, FRLibHeader* header)
{
	if (lib_size < sizeof(FRLibHeader))
		goto bail;
	
	if (header == NULL)
		goto bail;

	memcpy(header, lib_buf, sizeof(FRLibHeader));
	
	header->magic = Swap32(header->magic);
	header->objectCount = Swap32(header->objectCount);
	header->importCount = Swap32(header->importCount);
	header->size = Swap32(header->size);
	header->mcuNameLength = Swap32(header->mcuNameLength);
	
	if (header->magic != FR_LIB_MAGIC)
		goto bail;
	
	if (s_isVerbose)
	{
		char* ts;
		char fmt[32];

		printf("  magic:         0x%.8X\n", header->magic);
		
		ts = header->timestamp1;
		printf("  timestamp1:    %c%c%c%c.%c%c.%c%c %c%c:%c%c:%c%c\n",
			   ts[0], ts[1], ts[2], ts[3], ts[4], ts[5], ts[6], ts[7], ts[8], ts[9], ts[10], ts[11], ts[12], ts[13]);
		
		ts = header->timestamp2;
		printf("  timestamp2:    %c%c%c%c.%c%c.%c%c %c%c:%c%c:%c%c\n",
			   ts[0], ts[1], ts[2], ts[3], ts[4], ts[5], ts[6], ts[7], ts[8], ts[9], ts[10], ts[11], ts[12], ts[13]);
		printf("  objectCount:   %8d | 0x%.8X\n", header->objectCount, header->objectCount);
		printf("  importCount:   %8d | 0x%.8X\n", header->importCount, header->importCount);
		printf("  size:          %8d | 0x%.8X\n", header->size, header->size);
		
		sprintf(fmt, "  mcuName:       %%.%ds\n", header->mcuNameLength);
		printf("  mcuNameLength: %8d | 0x%.8X\n", header->mcuNameLength, header->mcuNameLength);
		printf(fmt, header->mcuName);
	}
	
	return true;
	
bail:
	
	return false;
}

bool parse_object_table(uint8_t* buffer, uint32_t buf_size, FRLibToolObjectList* list, uint32_t count)
{
	uint32_t offset = 0;

	if (buf_size < (sizeof(FRLibToolListHeader) + sizeof(FRLibObjectListItem)))
		goto bail;
	
	if (list == NULL || list->list == NULL)
		goto bail;

	memcpy(list, buffer, sizeof(FRLibToolListHeader));
	offset += sizeof(FRLibToolListHeader);
	
	if (list->magic != FR_OBJECTLIST_MAGIC)
		goto bail;
	
	list->size = Swap32(list->size);
	
	VERBOSE("  magic: 0x%.2X\n", list->magic);
	VERBOSE("  size:  %8d | 0x%.8X\n", list->size, list->size);
	VERBOSE("  OBJECT LIST:\n\n");
	
	for (uint32_t i = 0; (i < count) && (offset < list->size) && (offset < buf_size); i++)
	{
		uint32_t itemOffset = offset;
		uint32_t nameLength = 0;
		
		memcpy(&list->list[i], buffer + offset, kFuncListItemSize);
		
		list->list[i].offset = Swap32(list->list[i].offset);
		list->list[i].size = Swap32(list->list[i].size);
		list->list[i].unknown = Swap32(list->list[i].unknown);
		list->list[i].nameLength = Swap32(list->list[i].nameLength);
		
		offset += kFuncListItemSize;

		nameLength = list->list[i].nameLength;
		if (nameLength > MAX_NAME_LENGTH-1)
			nameLength = MAX_NAME_LENGTH-1;
		
		memcpy(list->list[i].name, buffer + offset, nameLength);

		if (((s_toolFlags & kToolFlag_OneFunciton) && i == s_funcIndex) ||
			!(s_toolFlags & kToolFlag_OneFunciton))
		{
			if (s_toolFlags & kToolFlag_DumpLib)
			{
				char* ts = NULL;
				
				printf("  [%3d] 0x%.8X: %s\n", i, itemOffset, list->list[i].name);
				
				ts = list->list[i].timestamp;
				printf("        timestamp: %c%c%c%c.%c%c.%c%c %c%c:%c%c:%c%c\n",
					   ts[0], ts[1], ts[2], ts[3], ts[4], ts[5], ts[6], ts[7], ts[8], ts[9], ts[10], ts[11], ts[12], ts[13]);
				printf("        offset:    0x%.8X\n", list->list[i].offset);
				printf("        size:      %5d | 0x%.4X\n", list->list[i].size, list->list[i].size);
				printf("        unknown:   %5d | 0x%.4X\n", list->list[i].unknown, list->list[i].unknown);
				printf("\n");
			}
			else if (s_isVerbose)
			{
				printf("  [%3d] 0x%.8X: %5d | 0x%.4X: %s\n",
					   i, list->list[i].offset, list->list[i].size, list->list[i].size, list->list[i].name);
			}
		}

		offset += list->list[i].nameLength;	// skip name
	}
	
	return true;
	
bail:
	
	return false;
}

bool parse_import_table(uint8_t* buffer, uint32_t buf_size, FRLibToolImportList* list, uint32_t count)
{
	uint32_t offset = 0;
	
	if (buf_size < (sizeof(FRLibToolListHeader)))
		goto bail;
	
	if (list == NULL || list->list == NULL)
		goto bail;
	
	memcpy(list, buffer, sizeof(FRLibToolListHeader));
	offset += sizeof(FRLibToolListHeader);
	
	if (list->magic != FR_IMPORTLIST_MAGIC)
		goto bail;
	
	list->size = Swap32(list->size);
	
	VERBOSE("  magic: 0x%.2X\n", list->magic);
	VERBOSE("  size:  %8d | 0x%.8X\n", list->size, list->size);
	VERBOSE("  IMPORT LIST:\n\n");
	
	for (uint32_t i = 0; (i < count) && (offset < buf_size); i++)
	{
		uint32_t itemOffset = offset;
		uint32_t nameLength = 0;
		
		memcpy(&list->list[i], buffer + offset, kImportListItemSize);
		
		list->list[i].index = Swap32(list->list[i].index);
		list->list[i].nameLength = Swap32(list->list[i].nameLength);
		
		offset += kImportListItemSize;
		
		nameLength = list->list[i].nameLength;
		if (nameLength > MAX_NAME_LENGTH-1)
			nameLength = MAX_NAME_LENGTH-1;
		
		memcpy(list->list[i].name, buffer + offset, nameLength);
		
		if (((s_toolFlags & kToolFlag_OneFunciton) && list->list[i].index == s_funcIndex) ||
			!(s_toolFlags & kToolFlag_OneFunciton))
		{
			printf("  [%3d] 0x%.8X: %s\n", i, itemOffset, list->list[i].name);
			printf("        index:      %5d | 0x%.4X\n", list->list[i].index, list->list[i].index);
			printf("\n");
		}
		
		offset += list->list[i].nameLength;	// skip name
	}
	
	return true;
	
bail:
	
	return false;
}

bool parse_object(uint8_t* buffer, uint32_t buf_size, FRLibToolObject* object)
{
	bool res = false;
	uint32_t offset = 0;
	uint8_t type = 0;
	uint32_t size = 0;
	
	uint8_t* dictonary = NULL;
	FRLibObjectImportTableItem* localImport = NULL;
	ObjectDataType* dataTypes = NULL;
	uint32_t currentDataIndex = 0;
	uint8_t* currentData = NULL;
	uint32_t bytesLeft = 0;
	
	if (object == NULL)
		goto bail;

	if (buffer[0] != kObjectHeader)
		goto bail;
	
	object->dataSize = 0;
	object->dataCount = 0;
	object->dataFirst = NULL;
	object->dataLast = NULL;
	
	size = Swap16(*((uint16_t*)(buffer + offset + 1)));
	offset += size;
	
	if (s_toolFlags & kToolFlag_DumpLib)
	{
		VERBOSE("          0x%.4X: Object Header\n", 0);
		VERBOSE("            type: 0x%.2X\n", buffer[0]);
		VERBOSE("            size: %4d | 0x%.4X\n\n", size, size);
	}
	
	while (offset < buf_size)
	{
		uint8_t* objStart = buffer + offset;
		FRLibObjectBlock* blockHeader = (FRLibObjectBlock*)objStart;
		
		// get block type
		type = blockHeader->type;
		// get block length
		size = Swap32(blockHeader->size);

		switch (type)
		{
			case kObjectDictonary:
			{
				dictonary = objStart + sizeof(FRLibObjectBlock);
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Dictonary\n", offset);
					VERBOSE("            type: 0x%.2X\n", type);
					VERBOSE("            size: %4d | 0x%.8X\n", size, size);

					uint8_t* value = dictonary + 1;
					for (uint32_t i=0; value < objStart + size - 1; i++)
					{
						VERBOSE("            %4d: \"%s\"\n", i, value);
						value += (strlen((char*)value)+1);
					}
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectBuildInfo:
			{
				FRLibObjectBuildInfo* block = (FRLibObjectBuildInfo*)objStart;
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					char* ts;

					VERBOSE("          0x%.4X: Object Build Info\n", offset);
					VERBOSE("            type:      0x%.2X\n", type);
					VERBOSE("            size:      %4d | 0x%.8X\n", size, size);
					
					ts = block->timestamp;
					VERBOSE("            timestamp:      | %c%c%c%c.%c%c.%c%c %c%c:%c%c:%c%c\n",
						   ts[0], ts[1], ts[2], ts[3], ts[4], ts[5], ts[6], ts[7], ts[8], ts[9], ts[10], ts[11], ts[12], ts[13]);
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectImportLibs:
			{
				FRLibObjectImportLibs block;
				memcpy(&block, objStart, sizeof(FRLibObjectImportLibs));
				
				block.libCount = Swap32(block.libCount);
				block.libNameOffset = Swap32(block.libNameOffset);
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Import Libs\n", offset);
					VERBOSE("            type:          0x%.2X\n", type);
					VERBOSE("            size:          %4d | 0x%.8X\n", size, size);
					VERBOSE("            libCount:      %4d\n", block.libCount);
					VERBOSE("            libNameOffset: %4d | 0x%.8X  (\"%s\")\n", block.libNameOffset, block.libNameOffset, (char*)(dictonary + block.libNameOffset));
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectDescriptor:
			{
				FRLibObjectDescriptor block;
				memcpy(&block, objStart, sizeof(FRLibObjectDescriptor));
				
				block.dataCount = Swap32(block.dataCount);
				block.importCount = Swap32(block.importCount);
				block.fr20Offset = Swap32(block.fr20Offset);
				block.fileNameOffset = Swap32(block.fileNameOffset);
				block.unknown2 = Swap32(block.unknown2);
				block.unknown3 = Swap32(block.unknown3);
				
				dataTypes = (ObjectDataType*)malloc(block.dataCount * sizeof(ObjectDataType));
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					char* ts;
					uint8_t* dt;
					
					VERBOSE("          0x%.4X: Object Descriptor\n", offset);
					VERBOSE("            type:           0x%.2X\n", type);
					VERBOSE("            size:           %4d | 0x%.8X\n", size, size);
					VERBOSE("            dataCount:      %4d\n", block.dataCount);
					VERBOSE("            importCount:    %4d\n", block.importCount);
					VERBOSE("            unknown2:            | 0x%.8X\n", block.unknown2);
					VERBOSE("            unknown3:            | 0x%.8X\n", block.unknown3);
					VERBOSE("            fr20Offset:     %4d | 0x%.8X (\"%s\")\n",
							block.fr20Offset, block.fr20Offset, (char*)(dictonary + block.fr20Offset));
					
					ts = block.timestamp;
					VERBOSE("            timestamp:           | %c%c%c%c.%c%c.%c%c %c%c:%c%c:%c%c\n",
							ts[0], ts[1], ts[2], ts[3], ts[4], ts[5], ts[6], ts[7], ts[8], ts[9], ts[10], ts[11], ts[12], ts[13]);
					
					dt = block.unknown4;
					VERBOSE("            unknown4:            | %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X%.2X\n",
							dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7], dt[8], dt[9]);
					VERBOSE("            fileNameOffset: %4d | 0x%.8X (\"%s\")\n",
							block.fileNameOffset, block.fileNameOffset, (char*)(dictonary + block.fileNameOffset));
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectDataDesc:
			{
				FRLibObjectDataDesc block;
				memcpy(&block, objStart, sizeof(FRLibObjectDataDesc));
				
				block.fullDataSize = Swap32(block.fullDataSize);
				block.dataTypeOffset = Swap32(block.dataTypeOffset);
				
				if (strcmp((char*)(dictonary + block.dataTypeOffset), "INIT") == 0)
					dataTypes[currentDataIndex++] = kObjectData_Init;
				else if (strcmp((char*)(dictonary + block.dataTypeOffset), "CODE") == 0)
					dataTypes[currentDataIndex++] = kObjectData_Code;
				else if (strcmp((char*)(dictonary + block.dataTypeOffset), "CONST") == 0)
					dataTypes[currentDataIndex++] = kObjectData_Const;
				else if (strcmp((char*)(dictonary + block.dataTypeOffset), "DATA") == 0)
					dataTypes[currentDataIndex++] = kObjectData_Data;
				else if (strcmp((char*)(dictonary + block.dataTypeOffset), "STACK") == 0)
					dataTypes[currentDataIndex++] = kObjectData_Stack;
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					uint8_t* dt;
					
					VERBOSE("          0x%.4X: Object Data Descriptor\n", offset);
					VERBOSE("            type:           0x%.2X\n", type);
					VERBOSE("            size:           %4d | 0x%.8X\n", size, size);
					dt = block.unknown1;
					VERBOSE("            unknown1:            | %.2X%.2X%.2X%.2X %.2X%.2X\n",
							dt[0], dt[1], dt[2], dt[3], dt[4], dt[5]);
					VERBOSE("            fullDataSize:   %4d | 0x%.8X\n",
							block.fullDataSize, block.fullDataSize);
					dt = block.unknown2;
					VERBOSE("            unknown2:            | %.2X%.2X%.2X%.2X %.2X%.2X\n",
							dt[0], dt[1], dt[2], dt[3], dt[4], dt[5]);
					VERBOSE("            dataTypeOffset: %4d | 0x%.8X (\"%s\")\n",
							block.dataTypeOffset, block.dataTypeOffset, (char*)(dictonary + block.dataTypeOffset));
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectImportTable:
			{
				uint32_t tableSize = (size - sizeof(FRLibObjectBlock) - 1);
				uint32_t itemCount = tableSize / sizeof(FRLibObjectImportTableItem);
				
				localImport = (FRLibObjectImportTableItem*)malloc(tableSize);
				memcpy(localImport, objStart + sizeof(FRLibObjectBlock), tableSize);
								
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Import Table\n", offset);
					VERBOSE("            type:   0x%.2X\n", type);
					VERBOSE("            size:   %4d | 0x%.8X\n", size, size);
				}
				
				for (uint32_t i=0; i < itemCount; i++)
				{
					localImport[i].funcNameOffset = Swap32(localImport[i].funcNameOffset);
					
					if (s_toolFlags & kToolFlag_DumpLib)
					{
						VERBOSE("            %4d:   %4d | 0x%.8X (\"%s\")\n", i,
								localImport[i].funcNameOffset, localImport[i].funcNameOffset, (char*)(dictonary + localImport[i].funcNameOffset));
					}
				}
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("\n");
				}
			}
			break;
			case kObjectExportInfo:
			{
				FRLibObjectExportInfo block;
				memcpy(&block, objStart, sizeof(FRLibObjectExportInfo));
				
				block.objNameOffset = Swap32(block.objNameOffset);
				
				object->exportName = (char*)(dictonary + block.objNameOffset);
				
				if (object->exportName[0] == '_')
					object->exportName++;
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					uint8_t* dt;
					
					VERBOSE("          0x%.4X: Object Export Info\n", offset);
					VERBOSE("            type:          0x%.2X\n", type);
					VERBOSE("            size:          %4d | 0x%.8X\n", size, size);
					dt = block.unknown1;
					VERBOSE("            unknown4:           | %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X %.2X\n",
							dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7], dt[8]);
					VERBOSE("            objNameOffset: %4d | 0x%.8X (\"%s\")\n",
							block.objNameOffset, block.objNameOffset, (char*)(dictonary + block.objNameOffset));
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectSeparator:
			{
				currentDataIndex = 0;
				
				FRLibObjectSeparator block;
				memcpy(&block, objStart, sizeof(FRLibObjectSeparator));
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Separator\n", offset);
					VERBOSE("            type:      0x%.2X\n", type);
					VERBOSE("            size:      %4d | 0x%.8X\n", size, size);
					uint8_t* dt = block.unknown1;
					VERBOSE("            unknown1:       | %.2X%.2X%.2X%.2X\n",
							dt[0], dt[1], dt[2], dt[3]);
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectDataPrefix:
			{
				FRLibObjectDataPrefix block;
				memcpy(&block, objStart, sizeof(FRLibObjectDataPrefix));
				
				block.dataIndex = Swap32(block.dataIndex);
				block.unknown1 = Swap32(block.unknown1);
				block.unknown2 = Swap32(block.unknown2);				
				
				currentDataIndex = block.dataIndex;
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Data Prefix\n", offset);
					VERBOSE("            type:      0x%.2X\n", type);
					VERBOSE("            size:      %4d | 0x%.8X\n", size, size);
					VERBOSE("            unknown1:       | 0x%.8X\n", block.unknown1);
					VERBOSE("            dataIndex: %4d | 0x%.8X (\"%s\")\n", block.dataIndex, block.dataIndex, datatype2str(dataTypes[currentDataIndex]));
					VERBOSE("            unknown2:       | 0x%.8X\n", block.unknown2);
					
					VERBOSE("\n");
				}
			}
			break;
			case kObjectData:
			{
				FRLibObjectData block;
				memcpy(&block, objStart, sizeof(FRLibObjectData));
				
				block.dataOffset = Swap32(block.dataOffset);
				block.dataSize = Swap32(block.dataSize);
				
				if (dataTypes[currentDataIndex] == kObjectData_Code)
				{
					currentData = objStart + sizeof(FRLibObjectData);
					bytesLeft = block.dataSize;
					
					if (object->dataFirst == NULL && object->dataLast == NULL)
					{
						object->dataFirst = object->dataLast = create_data_item(bytesLeft, currentData, NULL, object->dataSize);
					}
					else
					{
						object->dataLast->next = create_data_item(bytesLeft, currentData, NULL, object->dataSize);
						object->dataLast = object->dataLast->next;
					}
					
					object->dataSize += block.dataSize;
					object->dataCount++;
				}
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Data (\"%s\")\n", offset, datatype2str(dataTypes[currentDataIndex]));
					VERBOSE("            type:       0x%.2X\n", type);
					VERBOSE("            size:       %4d | 0x%.8X\n", size, size);
					VERBOSE("            dataOffset: %4d | 0x%.8X\n", block.dataOffset, block.dataOffset);
					VERBOSE("            dataSize:   %4d | 0x%.8X\n", block.dataSize, block.dataSize);
					VERBOSE("            data:  0x%.8X| ", 0);
					
					// dump data
					for (uint32_t i = 0; i < block.dataSize; i++)
					{
						if (i != 0)
						{
							if (i%16 == 0)
							{
								VERBOSE("\n");
								VERBOSE("                   0x%.8X| ", i);
							}
							else
							{
								if (dataTypes[currentDataIndex] == kObjectData_Code && i%2 == 0)
								{
									VERBOSE(" ");
								}
								else if (i%4 == 0)
								{
									VERBOSE("  ");
								}
							}
						}
						
						VERBOSE("%.2X", objStart[sizeof(FRLibObjectData) + i]);
						if (dataTypes[currentDataIndex] != kObjectData_Code)
						{
							VERBOSE(" ");
						}
					}
					
					VERBOSE("\n\n");
				}
			}
			break;
			case kObjectDataRefTable:
			{
				uint8_t* value = objStart + sizeof(FRLibObjectBlock);
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Data Ref Table\n", offset);
					VERBOSE("            type:   0x%.2X\n", type);
					VERBOSE("            size:   %4d | 0x%.8X\n", size, size);
				}

				for (uint32_t i=0; value < objStart + size - 1; i++)
				{
					FRLibRefTableItem* itemHeader = (FRLibRefTableItem*)value;
					uint32_t subItemSize = Swap32(itemHeader->subItemSize);
					
					switch (subItemSize)
					{
						case kObjectRef_Function:
						{
							char* importName;
							FRLibRefItemImport item;
							memcpy(&item, value, sizeof(FRLibRefItemImport));
							
							item.refOffset = Swap32(item.refOffset);
							item.importIndex = Swap32(item.importIndex);

							if (item.importType == 0x02)
								importName = (char*)(dictonary + localImport[item.importIndex].funcNameOffset);
							else
								importName = (char*)"";
							
							if (dataTypes[currentDataIndex] == kObjectData_Code)
							{
								// complete
								// |-------size------|
								// |ccc|ii|ccc|ii|ddd|
								// |__offset__|
								//        |___left___|
								//        |_x_|

								// fix
								// |-------size------|
								// |ccc|ii|ccc|ii|ddd|
								// |__offset__|     								
								//            |sz|
								//               |_l_|
								
								FRLibToolObjectData* next = NULL;
								
								if ((object->dataSize - item.refOffset) != object->dataLast->size)
								{
									// complete last item
									object->dataLast->size = bytesLeft - (object->dataSize - item.refOffset);
									currentData += object->dataLast->size;
									bytesLeft -= object->dataLast->size;

									// create item for variable bytes (import)
									object->dataLast->next = create_data_item(item.refSize, NULL, importName, item.refOffset);
									object->dataCount++;
									
									next = object->dataLast->next;
								}
								else
								{
									// fix last item
									object->dataLast->size = item.refSize;
									object->dataLast->data = NULL;
									object->dataLast->import = importName;
									
									next = object->dataLast;
								}
								currentData += item.refSize;
								bytesLeft -= item.refSize;
								
								// create new tail item
								next->next = create_data_item(bytesLeft, currentData, NULL, item.refOffset + item.refSize);
								object->dataCount++;
								
								// update last item pointer
								object->dataLast = next->next;
							}
							
							if (s_toolFlags & kToolFlag_DumpLib)
							{
								VERBOSE("            %4d: Import Reference (%X)\n", i, subItemSize);
								VERBOSE("                  refSize:        %d\n", item.refSize);
								VERBOSE("                  refOffset:   %4d | 0x%.8X\n", item.refOffset, item.refOffset);
								VERBOSE("                  importType:  %4d | 0x%.2X\n", item.importType, item.importType);
								VERBOSE("                  importIndex: %4d | (\"%s\")\n", item.importIndex, importName);
							}
						}
						break;
						case kObjectRef_Data:
						{
							FRLibRefItemData item;
							memcpy(&item, value, sizeof(FRLibRefItemData));
							
							item.refOffset = Swap32(item.refOffset);
							
							if (dataTypes[currentDataIndex] == kObjectData_Code)
							{
								FRLibToolObjectData* next = NULL;
								
								if ((object->dataSize - item.refOffset) != object->dataLast->size)
								{
									// complete last item
									object->dataLast->size = bytesLeft - (object->dataSize - item.refOffset);
									currentData += object->dataLast->size;
									bytesLeft -= object->dataLast->size;
									
									// create item for variable bytes (import)
									object->dataLast->next = create_data_item(item.refSize, NULL, NULL, item.refOffset);
									object->dataCount++;
									
									next = object->dataLast->next;
								}
								else
								{
									// fix last item
									object->dataLast->size = item.refSize;
									object->dataLast->data = NULL;
									object->dataLast->import = NULL;
									
									next = object->dataLast;
								}
								currentData += item.refSize;
								bytesLeft -= item.refSize;
								
								// create new tail item
								next->next = create_data_item(bytesLeft, currentData, NULL, item.refOffset + item.refSize);
								object->dataCount++;
								
								// update last item pointer
								object->dataLast = next->next;
							}
							
							if (s_toolFlags & kToolFlag_DumpLib)
							{
								uint8_t* dt = item.unknown2;
								VERBOSE("            %4d: Data Reference (%X)\n", i, subItemSize);							
								VERBOSE("                  refSize:        %d\n", item.refSize);
								VERBOSE("                  refOffset:   %4d | 0x%.8X\n", item.refOffset, item.refOffset);
								VERBOSE("                  unknown2:         | %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",
										dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7]);
								VERBOSE("                  offset:      %4d | 0x%.2X\n", item.offset, item.offset);
							}
						}
						break;
						case kObjectRef_Label:
						{
							FRLibRefItemLabel item;
							memcpy(&item, value, sizeof(FRLibRefItemLabel));
							
							item.refOffset = Swap32(item.refOffset);
							item.offset = Swap16(item.offset);
							
							if (dataTypes[currentDataIndex] == kObjectData_Code)
							{
								FRLibToolObjectData* next = NULL;
								
								if ((object->dataSize - item.refOffset) != object->dataLast->size)
								{
									// complete last item
									object->dataLast->size = bytesLeft - (object->dataSize - item.refOffset);
									currentData += object->dataLast->size;
									bytesLeft -= object->dataLast->size;
									
									// create item for variable bytes (import)
									object->dataLast->next = create_data_item(item.refSize, NULL, NULL, item.refOffset);
									object->dataCount++;
									
									next = object->dataLast->next;
								}
								else
								{
									// fix last item
									object->dataLast->size = item.refSize;
									object->dataLast->data = NULL;
									object->dataLast->import = NULL;
									
									next = object->dataLast;
								}
								currentData += item.refSize;
								bytesLeft -= item.refSize;
								
								// create new tail item
								next->next = create_data_item(bytesLeft, currentData, NULL, item.refOffset + item.refSize);
								object->dataCount++;
								
								// update last item pointer
								object->dataLast = next->next;
							}
							
							if (s_toolFlags & kToolFlag_DumpLib)
							{
								uint8_t* dt = item.unknown2;
								VERBOSE("            %4d: Label Reference (%X)\n", i, subItemSize);
								VERBOSE("                  refSize:        %d\n", item.refSize);
								VERBOSE("                  refOffset:   %4d | 0x%.8X\n", item.refOffset, item.refOffset);
								VERBOSE("                  unknown2:         | %.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",
										dt[0], dt[1], dt[2], dt[3], dt[4], dt[5], dt[6], dt[7]);
								VERBOSE("                  offset:      %4d | 0x%.4X\n", item.offset, item.offset);							
							}
						}
						break;
						default:
						{
							if (s_toolFlags & kToolFlag_DumpLib)
							{
								VERBOSE("            %4d: Unknown Reference (%X)\n", i, subItemSize);
							}
						}
						break;
					}
					
					value += (sizeof(FRLibRefTableItem) + subItemSize);
				}
				
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("\n");
				}
			}
			break;
			case kObjectTerminator:
			{
				if (s_toolFlags & kToolFlag_DumpLib)
				{
					VERBOSE("          0x%.4X: Object Terminator\n", offset);
					VERBOSE("            type:   0x%.2X\n", type);
					VERBOSE("            size:   %4d | 0x%.8X\n", size, size);
					
					VERBOSE("\n");
				}
			}
			break;
			default:
				break;
		}
		
		// move offset to the next block
		offset += size;
	}
	
	res = true;
	
bail:
	
	if (dataTypes)
		free(dataTypes);
	
	if (localImport)
		free(localImport);
	
	return res;
}

bool find_firmware_func(uint8_t* fw_buf, uint32_t fw_size, FRLibToolObject* object, uint32_t* offset, uint32_t* count)
{
	bool res = true;
	bool found = false;
	uint32_t bestOffset = 0;
	uint32_t bestBytes = 0;
	uint32_t match = 0;
	
	if (fw_buf == NULL || fw_size == 0 || object == NULL || offset == NULL || count == NULL)
		return false;
	
	*offset = 0xFFFFFFFF;
	*count = 0;
	
	for (uint32_t i=s_scanOffset; i < fw_size; i++)
	{
		match = 0;
		uint32_t j=0, m=0;
		FRLibToolObjectData* item = object->dataFirst;
		
		while (item && (item->data == NULL))
		{
			match += item->size;
			j += item->size;
			m += item->size;
			
			item = item->next;
		}
		
		for (; j < object->dataSize;)
		{
			bool fail = false;
			uint8_t* data = item->data;
			
			for (uint32_t n=0; n < item->size; n++)
			{
				if ((i+m < fw_size) && (fw_buf[i+m] == data[n]))
				{
					match++;
					m++;
					j++;
				}
				else
				{
					fail = true;
					break;
				}
			}
			
			if (fail)
				break;

			item = item->next;
			
			while (item && (item->data == NULL))
			{
				match += item->size;
				j += item->size;
				m += item->size;
				
				item = item->next;
			}
		}
		
		if (match >= bestBytes)
		{
			bestBytes = match;
			bestOffset = i;
		}
		
		if (match == object->dataSize)
		{
			found = true;
			(*count)++;
			
			if (s_toolFlags & kToolFlag_OutputList)
			{
				VERBOSE("        0x%.8X: %.2f%% MATCH (%d/%d) - FUNCTION FOUND\n", i + s_imageBase, ((float)match/(float)object->dataSize)*100, match, object->dataSize);
			}

		#ifdef USE_FIRST_MATCH
			break;
		#endif
		}
	#ifdef SHOW_CLOSE_MATCH
		else
		{
			if (match > MATCH_THRESHOLD)
			{
				if (s_toolFlags & kToolFlag_OutputList)
				{
					VERBOSE("        0x%.8X: %.2f%% MATCH (%d/%d)\n", i + s_imageBase, ((float)match/(float)object->dataSize)*100, match, object->dataSize);
				}
			}
		}
	#endif
	}
	
#ifdef SHOW_CLOSE_MATCH
	if (bestBytes <= MATCH_THRESHOLD)
#else
	if (found == false)
#endif
	{
		if (s_toolFlags & kToolFlag_OutputList)
		{
			VERBOSE("        NOT FOUND\n");
		}
	}

	if (found)
		*offset = bestOffset;
	
bail:
	
	return res;
}

bool generate_output_list(uint32_t index, uint32_t offset, char* name, uint32_t count)
{
	if (!s_isVerbose)
	{
		if (count == 1)
			printf("[%3d] %20s: 0x%.8X\n", index, name, offset + s_imageBase);
		else
		#ifdef USE_FIRST_MATCH
			printf("[%3d] %20s: 0x%.8X (first one out of %d)\n", index, name, offset + s_imageBase, count);
		#else
			printf("[%3d] %20s: 0x%.8X (last one out of %d)\n", index, name, offset + s_imageBase, count);
		#endif
	}
	
	return true;
}

bool generate_output_pat(FILE* pat_file, FRLibToolObject* object)
{
	bool res = false;
	
	uint32_t i=0;
	
	uint16_t cs = 0;
	int32_t cs_cnt = -1;
	uint8_t	cs_buf[255];

	FRLibToolObjectData* item = object->dataFirst;
	
	while (item)
	{
		uint32_t size = item->size;
		uint8_t* data = item->data;
		
		for (uint32_t j=0; j < size; j++)
		{
			// process data
			if (i < 32)
			{
				if (data != NULL)
					fprintf(pat_file, "%.2X", data[j]);
				else
					fprintf(pat_file, "..");
				
				if (i == 31)
				{
					cs_cnt = 0;
				}
			}
			else if (cs_cnt != -1)
			{
				if (data == NULL || cs_cnt == 256)
				{
					// corner case: data size = 32 + 255
					if (cs_cnt == 256)
						cs_cnt = 255;
					
					if (cs_cnt != 0)
						cs = crc16(cs_buf, cs_cnt);
					else
						cs = 0;
					
					fprintf(pat_file, " %.2X %.4X %.4X :0000 %s", cs_cnt, cs, object->dataSize, object->exportName);
					
					FRLibToolObjectData* tmpItem = object->dataFirst;
					while (tmpItem)
					{
						if (tmpItem->import)
						{
							fprintf(pat_file, " ^%.4X %s", tmpItem->offset, tmpItem->import);
						}
						
						tmpItem = tmpItem->next;
					}

					if (data != NULL)
						fprintf(pat_file, " %.2X", data[j]);
					else
						fprintf(pat_file, " ..");
					
					cs_cnt = -1;
				}
				else
				{
					cs_buf[cs_cnt++] = data[j];
				}
			}
			else
			{
				if (data != NULL)
					fprintf(pat_file, "%.2X", data[j]);
				else
					fprintf(pat_file, "..");
			}
			
			i++;
		}
		
		item = item->next;
	}
	
	// complete pat
	if ((i < 32) || (cs_cnt != -1))
	{
		if (i < 32)
		{
			for (uint32_t j=i; j < 32; j++)
			{
				fprintf(pat_file, "..");
			}

			cs_cnt = 0;
			cs = 0;
		}
		else if (cs_cnt != -1)
		{
			// corner case: data size = 32 + 255
			if (cs_cnt == 256)
				cs_cnt = 255;
			
			if (cs_cnt != 0)
				cs = crc16(cs_buf, cs_cnt);
			else
				cs = 0;
		}

		fprintf(pat_file, " %.2X %.4X %.4X :0000 %s", cs_cnt, cs, object->dataSize, object->exportName);
		
		item = object->dataFirst;
		while (item)
		{
			if (item->import)
			{
				fprintf(pat_file, " ^%.4X %s", item->offset, item->import);
			}
			
			item = item->next;
		}
	}
	
	fprintf(pat_file, "\n");
	
	res = true;
	
bail:
	
	return res;
}

bool generate_output_idc(uint32_t offset, char* name)
{
	// IDC
	// MakeName(0x0021DF7E, "name");
	// SetFunctionCmt(0x0021DF7E, "comment", 0);
	
	printf("\nMakeName(0x%.8X, \"%s\");\n", offset + s_imageBase, name);
	printf("SetFunctionCmt(0x%.8X, \"%s\", 0);\n", offset + s_imageBase, "comment");
	
	return true;
	
bail:
	
	return false;
}

bool generate_output_py(uint32_t offset, char* name)
{
	// Python
	// set_name(0x21BFF0, "name");
	// set_func_cmt(get_func(0x2d40a0), "comment", 0); refresh_idaview_anyway();

	printf("\nset_name(0x%.8X, \"%s\");\n", offset + s_imageBase, name);
	printf("set_func_cmt(get_func(0x%.8X), \"%s\", 0);\n", offset + s_imageBase, "comment");
	
	return true;
	
bail:
	
	return false;
}

void find_functions(const char* firmware, const char* library)
{
	bool res = false;
	
	uint8_t* buffer = NULL;
	uint32_t buf_size = 0;
	
	FILE* fw_file = NULL;
	FILE* lib_file = NULL;
	
	uint8_t* fw_buf = NULL;
	uint8_t* lib_buf = NULL;
	
	uint32_t fw_size = 0;
	uint32_t lib_size = 0;
	
	uint32_t firstObj = 0;
	uint32_t lastObj = 0;
	
	// PAT file
	char* dot = NULL;
	char patName[64];
	FILE* pat_file = NULL;
	
	FRLibHeader libHeader;
	FRLibToolObjectList objectList;
	FRLibToolImportList importList;
	FRLibToolObject		object;
	
	objectList.list = NULL;
	importList.list = NULL;
	
	object.dataFirst = NULL;
	object.dataLast = NULL;
	
	if (firmware != NULL)
	{
		// Load firmware file
		VERBOSE("\nOpen firmware image file: %s\n", firmware);
		fw_file = fopen(firmware, "r");
		
		fseek(fw_file, 0L, SEEK_END);
		fw_size = (uint32_t)ftell(fw_file);
		
		fw_buf = (uint8_t*)malloc(fw_size);
		
		// read packed firmware
		fseek(fw_file, 0L, SEEK_SET);
		fread(fw_buf, 1, fw_size, fw_file);
		
		fclose(fw_file);
		
		VERBOSE("  File size: %8d | 0x%.8X\n", fw_size, fw_size);
	}
	
	// Load library file
	VERBOSE("\nOpen library file: %s\n", library);
	lib_file = fopen(library, "r");
	
	fseek(lib_file, 0L, SEEK_END);
	lib_size = (uint32_t)ftell(lib_file);
	
	lib_buf = (uint8_t*)malloc(lib_size);

    buffer = lib_buf;
    buf_size = lib_size;
	
	// read packed firmware
	fseek(lib_file, 0L, SEEK_SET);
	fread(lib_buf, 1, lib_size, lib_file);
	
	fclose(lib_file);
	
	VERBOSE("  File size: %8d | 0x%.8X\n", lib_size, lib_size);

	VERBOSE("\nParse library header:\n");
	res = parse_library_header(lib_buf, lib_size, &libHeader);
	if (res == false)
		goto bail;
	
	// create function list
	objectList.list = (FRLibObjectListItem*)calloc(libHeader.objectCount, sizeof(FRLibObjectListItem));
	
	buffer += sizeof(FRLibHeader);
	buf_size -= sizeof(FRLibHeader);

	printf("\nFunctions found in library: %d\n", libHeader.objectCount);
	
	if (s_toolFlags & kToolFlag_OneFunciton)
	{
		if (s_funcIndex > libHeader.objectCount)
		{
			printf("\n Incorrect function index %d of %d", s_funcIndex, libHeader.objectCount);
			goto bail;
		}
	}
	
	VERBOSE("\nParse function table:\n");
	res = parse_object_table(buffer, buf_size, &objectList, libHeader.objectCount);
	if (res == false)
		goto bail;

	buffer += (libHeader.size - sizeof(FRLibHeader));
	buf_size -= (libHeader.size - sizeof(FRLibHeader));
	
	if ((s_toolFlags & kToolFlag_DumpLib) && libHeader.importCount != 0)
	{
		importList.list = (FRLibImportListItem*)calloc(libHeader.importCount, sizeof(FRLibImportListItem));
		
		VERBOSE("Parse import table:\n");
		res = parse_import_table(buffer, buf_size, &importList, libHeader.importCount);
		if (res == false)
			goto bail;
	}

	if (s_toolFlags & kToolFlag_DumpLib)
	{
		VERBOSE("DUMP FUNCTION OBJECTS:\n\n");
	}
	else if ((s_toolFlags & kToolFlag_OutputList) ||
			 (s_toolFlags & kToolFlag_OutputIDC) ||
			 (s_toolFlags & kToolFlag_OutputPython))
	{
		printf("\nSearch for functions in firmware...\n");
		if (!(s_toolFlags & kToolFlag_OutputList))
		{
			printf("\n---------------------------- COPY FROM HERE ----------------------------\n");
		}
	}
	else if ((s_toolFlags & kToolFlag_OutputPAT))
	{
		strcpy(patName, library);
		dot = strrchr(patName, '.');
		*dot = 0;

		printf("\nGenerating %s.pat file...\n", patName);
		
		strcat(patName, ".pat");
		
		pat_file = fopen(patName, "w");

	}

	if (s_toolFlags & kToolFlag_OneFunciton)
	{
		firstObj = s_funcIndex;
		lastObj = firstObj + 1;
	}
	else
	{
		firstObj = 0;
		lastObj = libHeader.objectCount;
	}
	
	for (uint32_t i = firstObj; i < lastObj; i++)
	{
		FRLibObjectListItem* item = &objectList.list[i];
		if (item->offset > lib_size)
			goto bail;
		
		if (s_toolFlags & kToolFlag_DumpLib)
		{
			VERBOSE("  [%3d] 0x%.8X: %s\n\n", i, item->offset, item->name);
		}
		
		res = parse_object(lib_buf + item->offset, item->size, &object);
		if (res == false)
			goto bail;
		
		if (object.dataSize == 0)
		{
			reset_data_object(&object);
			continue;
		}
		
		if (!(s_toolFlags & kToolFlag_DumpLib))
		{
			uint32_t fwOffset = 0;
			uint32_t count;
			
			if ((s_toolFlags & kToolFlag_OutputList) ||
				(s_toolFlags & kToolFlag_OutputIDC) ||
				(s_toolFlags & kToolFlag_OutputPython))
			{
				if (s_toolFlags & kToolFlag_OutputList)
				{
					VERBOSE("  [%3d] %s\n", i, item->name);
				}
				
				res = find_firmware_func(fw_buf, fw_size, &object, &fwOffset, &count);
				if (res == false)
					goto bail;
				
				if (fwOffset != 0xFFFFFFFF)
				{
					if (s_toolFlags & kToolFlag_OutputList)
					{
						res = generate_output_list(i, fwOffset, object.exportName, count);
						if (res == false)
							goto bail;
					}
					else if (s_toolFlags & kToolFlag_OutputIDC)
					{
						res = generate_output_idc(fwOffset, object.exportName);
						if (res == false)
							goto bail;
					}
					else if (s_toolFlags & kToolFlag_OutputPython)
					{
						res = generate_output_py(fwOffset, object.exportName);
						if (res == false)
							goto bail;
					}
				}
			}
			else if (s_toolFlags & kToolFlag_OutputPAT)
			{
				res = generate_output_pat(pat_file, &object);
				if (res == false)
					goto bail;
				VERBOSE("  [%3d] 0x%.8X: %20s -> done\n", i, item->offset, item->name);
			}
		}
		
		reset_data_object(&object);
	}
	
	if ((s_toolFlags & kToolFlag_OutputIDC) ||
		(s_toolFlags & kToolFlag_OutputPython))
	{
		if (s_toolFlags & kToolFlag_OutputPython)
			printf("\nrefresh_idaview_anyway();\n");

		printf("\n------------------------------ END OF CODE -----------------------------\n\n");
		
		printf("  NOTE: Function search doesn't check for collisions as well\n");
		printf("        as for function frame. Also you can have false positive\n");
		printf("        results when object body is too small. Applying this\n");
		printf("        script could break your disassembly listing.\n");
		printf("        USE IT AT YOUR OWN RISK.\n");
	}
	else if (s_toolFlags & kToolFlag_OutputPAT)
	{
		fprintf(pat_file, "---\n");
		printf("\n");
		
		strcpy(patName, library);
		dot = strrchr(patName, '.');
		*dot = 0;
		
		printf("  Use IDA FLAIR to generate FLIRT signatures:\n");
		printf("    ./sigmake -n\"Library Name\" %s.pat %s.sig\n\n", patName, patName);
		printf("  It is likely that you need to resolve collisions (read FLAIR documantation).\n");
		printf("  Then, when *.sig file is ready, copy it to the \"[IDA]/sig/fr/\" folder\n");
	}
	
bail:
	
	reset_data_object(&object);
	
	if (pat_file)
		fclose(pat_file);
	
	if (importList.list)
		free(importList.list);
	
	if (objectList.list)
		free(objectList.list);
	
	if (lib_buf)
		free(lib_buf);
	
	if (fw_buf)
		free(fw_buf);

	if (res == false)
	{
		if (s_toolFlags & kToolFlag_DumpLib)
		{
			printf("\nLibrary dump FAILED!\n");
		}
		else if ((s_toolFlags & kToolFlag_OutputIDC) ||
				 (s_toolFlags & kToolFlag_OutputPython) ||
				 (s_toolFlags & kToolFlag_OutputList))
		{
			printf("\nFunction search FAILED!\n");
		}
		else if ((s_toolFlags & kToolFlag_OutputPAT))
		{
			printf("\nPAT generation FAILED!\n");
		}
		
		if(s_isVerbose == false)
			printf("\nUse [-v] option to identify a problem.\n");
	}
	else
	{
		
		if (s_toolFlags & kToolFlag_DumpLib)
		{
			printf("\nLibrary dump complete!\n");
		}
		else if ((s_toolFlags & kToolFlag_OutputIDC) ||
				 (s_toolFlags & kToolFlag_OutputPython) ||
				 (s_toolFlags & kToolFlag_OutputList))
		{
			printf("\nFunction search complete!\n");
		}
		else if ((s_toolFlags & kToolFlag_OutputPAT))
		{
			printf("\nPAT generation complete!\n");
		}
	}
	
	return;
}

static void usage(const char* progName)
{
	printf("Fujitsu RISC Library Tool v%s\n", VERSION);
	printf("Usage: %s [-s start] [-i imagebase] [-o output] [-f index] [-dv] FIRMWARE.BIN LIBRARY.LIB\n\n", progName);
	printf("This tool will help you to find Softune REALOS library functions in FR (Fujitsu RISC) firmware.\n");
	printf("Use following arguments:\n");
	printf("    -f       Specify firmware image file\n");
	printf("    -s       Specify firmware image scan offset\n");
	printf("    -b       Specify firmware imagebase\n");
	printf("    -o       Specify output type (exclusively)\n");
	printf("       list  - list of functions\n");
	printf("       idc   - IDC script\n");
	printf("       py    - IDA python script\n");
	printf("       pat   - FLAIR pattern file\n");
	printf("    -i xxx   Specify index of particular function\n");
	printf("    -d       Dump library\n");	
	printf("    -v       Be verbose\n");
	printf("\n");
	printf("Default scan offset: 0\n");
	printf("Default imagebase: 0\n");
	printf("Default output: list\n");
	printf("\n");
	printf("NOTE: Use -v to display close matches for 'list' output. Script is generated only\n");
	printf("      for the function with full match.\n");
	printf("\n");
	printf("Examples:\n");
	printf("  Dump library:\n");
	printf("   $ FRLibTool -d LIBRARY.LIB\n");
	printf("\n");
	printf("  Dump object with index 3:\n");
	printf("   $ FRLibTool -i 3 LIBRARY.LIB\n");
	printf("\n");
	printf("  Generate *.pat file for the library:\n");
	printf("   $ FRLibTool -o pat LIBRARY.LIB\n");
	printf("\n");
	printf("  Get list of all library functions found in firmware (with close matches):\n");
	printf("   $ FRLibTool -v -f FIRMWARE.FW LIBRARY.LIB\n");
	printf("\n");
	printf("  Generate IDC script for all library functions found in firmware starting from byte 0x1000:\n");
	printf("   $ FRLibTool -o idc -s 0x1000 -f FIRMWARE.FW LIBRARY.LIB\n");
	printf("\n");
	printf("  Generate IDA python script for all library functions found in firmware (add imagebase 0x100000):\n");
	printf("   $ FRLibTool -o py -b 0x100000 -f FIRMWARE.FW LIBRARY.LIB\n");
	printf("\n");
	printf("  Usual workflow:\n");
	printf("   - To use along with IDA it is recommended to generate .pat file and then *.sig using FLAIR.\n");
	printf("   - Otherwise:\n");
	printf("     a. use list output to get all functions found in firmware with indexes\n");	
	printf("     b. generate idc/py script for particular function with index or all\n");
	printf("\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char* const argv[])
{
	int ch;
	const char* firmwareFileName = NULL;
	const char* libraryFileName = NULL;
	const char* progName = argv[0];
	
	// Set defaults
	s_isVerbose = false;
	s_toolFlags = kToolFlag_Clear;
	
	printf("\n");
	
	// Get options
	while ((ch = getopt(argc, argv, "f:s:b:o:i:vd")) != -1)
	{
		switch (ch)
		{
			case 'f':
				firmwareFileName = optarg;
				break;
			case 's':
				s_scanOffset = (uint32_t)strtol(optarg, NULL, 0);
				s_toolFlags |= kToolFlag_UseScanOffset;
				break;
			case 'b':
				s_imageBase = (uint32_t)strtol(optarg, NULL, 0);
				s_toolFlags |= kToolFlag_UseImageBase;
				break;
			case 'o':
				if (strcmp(optarg, "idc") == 0) {
					s_toolFlags |= kToolFlag_OutputIDC;
				}
				else if (strcmp(optarg, "py") == 0) {
					s_toolFlags |= kToolFlag_OutputPython;
				}
				else if (strcmp(optarg, "pat") == 0) {
					s_toolFlags |= kToolFlag_OutputPAT;
				}
				else if (strcmp(optarg, "list") == 0) {
					s_toolFlags |= kToolFlag_OutputList;
				}
				else {
					usage(progName);
				}
				break;
			case 'i':
				s_funcIndex = (uint32_t)strtol(optarg, NULL, 0);
				s_toolFlags |= kToolFlag_OneFunciton;
				break;
			case 'd':
				s_toolFlags |= kToolFlag_DumpLib;
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
	
	libraryFileName = argv[0];
	
	// Validate parameters
	if (s_toolFlags & kToolFlag_Verbose)
		s_isVerbose = true;
	
	if (s_toolFlags & kToolFlag_UseImageBase)
	{
		if (s_imageBase == 0)
		{
			printf("  Invalid imagebase.\n");
			return 0;
		}
	}
	else
	{
		s_imageBase = 0;
	}

	if ((s_toolFlags & (kToolFlag_OutputPAT |
						kToolFlag_OutputList |
						kToolFlag_OutputIDC |
						kToolFlag_OutputPython)) == 0 &&
		(s_toolFlags & kToolFlag_DumpLib) == 0)
	{
		s_toolFlags |= kToolFlag_OutputList;
	}
	
	if ((s_toolFlags & kToolFlag_OutputPAT) &&
		firmwareFileName != NULL)
	{
		printf("  You should not specify firmware image file to generate PAT.\n");
		return 0;
	}
	
	if (((s_toolFlags & kToolFlag_OutputIDC) ||
		 (s_toolFlags & kToolFlag_OutputList) ||
		 (s_toolFlags & kToolFlag_OutputPython)) &&
		firmwareFileName == NULL)
	{
		printf("  You should specify firmware image file.\n");
		return 0;
	}
	
	if ((s_toolFlags & kToolFlag_DumpLib) && firmwareFileName != NULL)
	{
		printf("  You should not specify firmware image file to dump library.\n");
		return 0;
	}

	if (BitCount(s_toolFlags & (kToolFlag_OutputPAT |
								kToolFlag_OutputList |
								kToolFlag_OutputIDC |
								kToolFlag_OutputPython)) > 1)
	{
		printf("  You should specify only one output type.\n");
		return 0;
	}

	if (s_toolFlags & kToolFlag_DumpLib)
	{
		s_toolFlags |= kToolFlag_Verbose;
		s_isVerbose = true;
	}
	
	// Display parameters
	printf("Running with options:\n");
	if ((s_toolFlags & kToolFlag_OutputIDC) ||
		(s_toolFlags & kToolFlag_OutputList) ||
		(s_toolFlags & kToolFlag_OutputPython))
	{
		printf("  + find functions from \"%s\" in firmware image file \"%s\"\n", libraryFileName, firmwareFileName);
	}
	
	if (s_toolFlags & kToolFlag_UseScanOffset)
		printf("  + use firmware start offset: %d (0x%.8X)\n", s_scanOffset, s_scanOffset);
	if (s_toolFlags & kToolFlag_UseImageBase)
		printf("  + use firmware image base: %d (0x%.8X)\n", s_imageBase, s_imageBase);
	if (s_toolFlags & kToolFlag_OutputList)
		printf("  + display output as list\n");
	if (s_toolFlags & kToolFlag_OutputIDC)
		printf("  + display output as IDC code\n");
	if (s_toolFlags & kToolFlag_OutputPython)
		printf("  + display output as IDA python code\n");
	if (s_toolFlags & kToolFlag_OutputPAT)
		printf("  + generate FLAIR *.pat file\n");
	if (s_toolFlags & kToolFlag_OneFunciton)
		printf("  + %s function with index %d (0x%X)\n", (s_toolFlags & kToolFlag_DumpLib)? "dump" : "find", s_funcIndex, s_funcIndex);
	else if(s_toolFlags & kToolFlag_DumpLib)
		printf("  + dump library\n");
	if (s_toolFlags & kToolFlag_Verbose)
		printf("  + verbose enabled\n");
	
	// Perform action
	find_functions(firmwareFileName, libraryFileName);

    return 0;
}

