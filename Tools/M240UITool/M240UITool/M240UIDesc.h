//
//  M240UIDesc.h
//  
//
//  Created by Alex Hude on 19/07/13.
//  Copyright (c) 2013 Alex Hude. All rights reserved.
//

#pragma once

#define kLCD_Width		640
#define kLCD_Height	    480

#define kEVF_Width		800
#define kEVF_Height		600

struct UIColorAYUV
{
	uint8_t	A;
	uint8_t	Y;
	uint8_t	U;
	uint8_t	V;
};

struct UIColorARGB
{
	uint8_t	A;
	uint8_t	R;
	uint8_t	G;
	uint8_t	B;
};

enum UILanguages
{
	kLanguage_English,				// 0
	kLanguage_German,				// 1
	kLanguage_French,				// 2
	kLanguage_Spanish,				// 3
	kLanguage_Italian,				// 4
	kLanguage_Russian,				// 5
	kLanguage_Japanese,				// 6
	kLanguage_TraditionalChinese,	// 7
	kLanguage_SimplifiedChinese,	// 8
	
	kMaxLanguageCount,
	
	kLanguage_Unknown	= 0xFFFFFFFF,
};

typedef uint8_t UITarget;
enum {
	kUITarget_LCD		= 1,
	kUITarget_EVF		= 2,
};

typedef uint8_t UIDescType;
enum {
	kUIDescType_0		= 0,
	kUIDescType_1		= 1,
	kUIDescType_2		= 2,
	kUIDescType_3		= 3,
};

struct UICallArguments
{
	uint32_t			dstAddress;		// R4 - we are not filling this value
	UIDescType			type;			// R5 - description type
	UITarget			target;			// R6 - rendering target
	uint32_t			descAddress;	// R7 - description address
	uint8_t				always0;		// (SP + 0x0) - always 0
	uint8_t				index1;			// (SP + 0x4) - index 1
	uint8_t				index2;			// (SP + 0x8) - index 2
	uint16_t			x_offset;		// (SP + 0xC) - x offset
	uint16_t			y_offset;		// (SP + 0x10) - y offset
	uint16_t			unknown2;		// (SP + 0x14) -
	uint32_t			language1;		// (SP + 0x18) - language id 1
	uint32_t			language2;		// (SP + 0x1C) - language id 2
	uint32_t			funcAddress;	// (SP + 0x20) - function address
};

struct UIDescType0Header
{
	uint32_t	address;		//
	uint16_t	entries;		//
	uint16_t	unknown;		//
} __attribute__((aligned(1), packed));

struct UIDescType0Entry
{
	uint16_t	x_offset;		//
	uint16_t	y_offset;		//
	uint32_t	address;		//
} __attribute__((aligned(1), packed));

struct UIDescType1Header
{
	uint32_t	address;		//
	uint16_t	entries;		//
	uint16_t	unknown;		//
} __attribute__((aligned(1), packed));

struct UIDescType1Entry
{
	uint16_t	x_offset;		//
	uint16_t	y_offset;		//
	uint32_t	address;		//
	uint16_t	objects;		//
	uint16_t	total_w;		//
	uint16_t	total_h;		//
	uint16_t	unknown;		//
} __attribute__((aligned(1), packed));

struct UIDescType2
{
	uint32_t	reg;			//
	uint32_t	address;		//
	uint16_t	unknown1;		//
	uint16_t	unknown2;		//
	uint16_t	unknown3;		//
	uint16_t	tableoff;		//
} __attribute__((aligned(1), packed));

struct UIDescType3
{
	uint16_t	x_offset;		//
	uint16_t	y_offset;		//
	uint32_t	address;		//
} __attribute__((aligned(1), packed));
