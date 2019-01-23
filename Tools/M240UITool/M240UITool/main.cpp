//
//  main.c
//  M240UITool
//
//  Created by Alex Hude on 5/07/13.
//  Copyright (c) 2013 Alex Hude. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wchar.h>
#include <locale.h>

#include "EasyBMP.h"
#include "lodepng.h"

#include "M240UIDesc.h"
#include "M240UIDescContainer.h"

// -------------- MARK: Macro definitions

#define VERSION					"1.0"
#define MENU_STRING_COUNT		42
#define PATTERN_THRESHOLD		5

#define DEFAULT_CHARMAP_SIZE	95
#define DEFAULT_FUNC_NAME		"gui_CopyImageDesc"
#define DEFAULT_DUMP_FOLDER		"UI_DUMP"
#define DEFAULT_IDC_FILENAME	"script.idc"
#define DEFAULT_CONT_FILENAME	"container.dat"
#define DEFAULT_IMAGEBASE		0x100000

#define ClipColor(x)		(((x) > 255)? 255 : ((x) < 0)? 0 : (x))

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

// -------------- MARK: M240UITool definitions

enum ToolFlags
{
	kToolFlag_Clear			= (0),
	kToolFlag_UseFuncAddr	= (1 << 0),
	kToolFlag_UseImageBase	= (1 << 1),
	kToolFlag_IDC			= (1 << 2),
	kToolFlag_Container		= (1 << 3),
	kToolFlag_LutFile		= (1 << 4),
	kToolFlag_LutAddress	= (1 << 5),
	kToolFlag_DumpBMP		= (1 << 6),
	kToolFlag_DumpPNG		= (1 << 7),
	kToolFlag_RecoverChars	= (1 << 8),
	
	kToolFlag_Verbose		= (1 << 31),	
};

// -------------- MARK: UI definitions

#define kFROpcode_LDI8			0xC000
#define kFROpcode_LDI8_RegMask	0x000F
#define kFROpcode_LDI8_ValMask	0x0FF0
#define kFROpcode_LDI8_ValShift	0x4

#define kFROpcode_LDI20			0x9B00
#define kFROpcode_LDI20_RegMask	0x000F

#define kFROpcode_LDI32			0x9F80
#define kFROpcode_LDI32_RegMask	0x000F

#define kFROpcode_LDUH			0x0500
#define kFROpcode_LDUH_RegMask	0x000F

#define kFROpcode_ST			0x1300
#define kFROpcode_ST_RegMask	0x000F

static const char* s_asciiTable[] =
{
	" ",
	"!",
	"\"",
	"#",
	"$",
	"%",
	"&",
	"'",
	"(",
	")",
	"*",
	"+",
	",",
	"-",
	".",
	"/",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	":",
	";",
	"<",
	"=",
	">",
	"?",
	"@",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"[",
	"\\",
	"]",
	"^",
	"_",
	"`",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
	"k",
	"l",
	"m",
	"n",
	"o",
	"p",
	"q",
	"r",
	"s",
	"t",
	"u",
	"v",
	"w",
	"x",
	"y",
	"z",
	"{",
	"|",
	"}",
	"~",
	"DEL",
};


static const char* s_languageString[] =
{
	"English",
	"German",
	"French",
	"Spanish",
	"Italian",
	"Russian",
	"Japanese",
	"Traditional Chinese",
	"Simplified Chinese",
};

static bool	s_languageSupport[kMaxLanguageCount] = {
	true,	// English
	false,	// German
	false,	// French
	false,	// Spanish
	false,	// Italian
	true,	// Russian
	false,	// Japanese
	false,	// Traditional Chinese
	false,	// Simplified Chinese
};

const wchar_t s_menuText[kMaxLanguageCount][MENU_STRING_COUNT][256] = {
	// English
	{
		//" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}",
		L"Lens Detection",
		L"Selftimer",
		L"Light Metering Mode",
		L"Exposure Bracketing",
		L"Flash Synch. Mode",
		L"Auto Slow Sync.",
		L"Sharpness",
		L"Saturation",
		L"Contrast",
		L"Film mode",
		L"Color space",
		L"DNG Compression",
		L"Monitor Brightness",
		L"EVF Brightness",
		L"Frameline Color",
		L"Focus Peaking",
		L"Focus Aid",
		L"Histogram",
		L"Clipping Definition",
		L"Auto Review",
		L"Copyright information",
		L"Image numbering",
		L"Horizon",
		L"Sensor cleaning",
		L"GPS",
		L"Audio",
		L"Auto Power Off",
		L"Date / Time",
		L"Acoustic Signal",
		L"Language",
		L"USB Mode",
		L"Reset",
		L"Format SD card",
		L"Firmware",
		L"ISO",
		L"White Balance",
		L"File Format",
		L"JPEG Resolution",
		L"Video Resolution",
		L"Exposure Compensation",
		L"Exposure Metering",
		L"User Profile",
	},
	// German
	{
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
	},
	// French
	{
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
	},
	// Spanish
	{
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
	},
	// Italian
	{
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
	},
	// Russian
	{
		L"распознавание объектива",
		L"автоспуск через",
		L"режим замера экспозиции",
		L"эксповилка",
		L"синхр. фотовспышки",
		L"ограничение выдержки",
		L"резкость",
		L"насыщенность",
		L"контрастность",
		L"змуляция плёнки",
		L"цветовое пространство",
		L"сжатие DNG",
		L"яркость экрана",
		L"яркость электр. видоискателя",
		L"подсветка границ кадра",
		L"наилучшая фокусировка",
		L"помощник фокусировки",
		L"гистограмма",
		L"параметры отсечения",
		L"авто. просмотр",
		L"авторские права",
		L"нумерация файлов",
		L"уровень",
		L"очистка сенсора",
		L"GPS",
		L"звук",
		L"авто. выключение",
		L"дата / время",
		L"звуковые сигналы",
		L"Language",
		L"режим USB",
		L"сбросить",
		L"форматировать SD карту",
		L"Firmware",
		L"чувствительность",
		L"баланс белого цвета",
		L"формат файлов",
		L"разрешение JPEG",
		L"разрешение видео",
		L"экспокоррекция",
		L"замер экспозиции",
		L"профиль пользователя",
	},
	// Japanese
	{
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
	},
	// Traditional Chinese
	{
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
	},
	// Simplified Chinese
	{
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
		L"not ready yet",
	},
};

static UIColorAYUV s_lut[256] = {0};

struct UIBasePattern
{
	uint32_t	size;
	uint32_t	value;
	uint32_t	pattern[256];
};

struct UIStringPattern : UIBasePattern
{
	bool				complete;
	uint32_t			address;
	uint32_t			menuIdx;
	UIStringPattern*	next;
	wchar_t				string[256];
};

struct UIStringArray
{
	uint32_t			count;
	UIStringPattern*	first;
	UIStringPattern*	last;
};

static UIBasePattern	s_menuTextPatterns[kMaxLanguageCount][MENU_STRING_COUNT] = {0};

static UIStringArray	s_stringArray[kMaxLanguageCount] = {0};

struct UICharMapItem
{
	uint32_t	address;
	wchar_t		chr;
};

static UICharMapItem	s_charMap[1024] = {0};
uint32_t				s_charMapSize = 0;

struct UICallArgumentsEx : UICallArguments
{
	uint32_t			callAddress;	// Function call address
	UICallArgumentsEx*	next;
};

struct UIArgumentsArray
{
	uint32_t			count;
	UICallArgumentsEx*	first;
	UICallArgumentsEx*	last;
};

static UIArgumentsArray s_argsArray = {0};

// -------------- MARK: Static definitions

static bool			s_isVerbose = false;
static uint32_t		s_toolFlags = kToolFlag_Clear;
static uint32_t		s_bytesToDisp = 0xFFFFFFFF;
static uint32_t		s_funcAddress = 0xFFFFFFFF;
static uint32_t		s_imageBase = 0xFFFFFFFF;
static FILE*		s_idcFile = NULL;
static char*		s_idcFileName = NULL;
static char*		s_lutFileName = NULL;
static uint32_t		s_lutAddress = 0xFFFFFFFF;
static char*		s_dumpFolder = NULL;
static char*		s_contFileName = NULL;
static uint32_t		s_languageAddress = 0xDEADBEEF;

// -------------- MARK: Implementation: Utilities

void display_bytes(uint8_t* buffer, uint32_t size, uint32_t indent)
{
	const char* spaces = "                    ";
	
	VERBOSE("%s", spaces + 20 - indent);
	
	for (uint32_t i=0; i < size; i++)
	{
		if (i != 0)
		{
			if (i % 16 == 0)
			{
				VERBOSE("\n%s", spaces + 20 - indent);
			}
			else if (i % 4 == 0)
			{
				VERBOSE("  ");
			}
		}
		VERBOSE("%.2X ", buffer[i]);
	}
	
	VERBOSE("\n");
}

void find_data_size(uint8_t* buffer, uint32_t buf_size, uint32_t* size)
{
	uint32_t num;
	uint32_t color;
	uint32_t idx = 0;
	
	uint8_t* data = buffer;
	
	uint16_t width = *((uint16_t*)data);
	width = Swap16(width);
	width <<= 1;
	data += 2;
	
	uint16_t height = *((uint16_t*)data);
	height = Swap16(height);
	height <<= 1;
	data += 2;
	
    // skip third word
    data += 2;
    
	uint32_t count = width*height;
	
	while (idx < count)
	{
		num = data[0];
		color = data[1];
		
		idx += num;
		data += 2;
	}

	*size = (uint32_t)(data - buffer);
}

bool file_exists(char* fileName)
{
	struct stat buf;
	int i = stat ( fileName, &buf );

	// file found
	if ( i == 0 )
		return true;

	return false;
}

// -------------- MARK: Implementation: Arrays

bool add_arg_to_array(UICallArgumentsEx* args)
{
	UICallArgumentsEx* newArgs= (UICallArgumentsEx*)malloc(sizeof(UICallArgumentsEx));
	memcpy(newArgs, args, sizeof(UICallArgumentsEx));

	// add argument to array
	if (s_argsArray.first != NULL)
		s_argsArray.last->next = newArgs;
	else
		s_argsArray.first = newArgs;
	
	s_argsArray.last = newArgs;
	s_argsArray.count++;
	
	return true;
}

void delete_args_array()
{
	UICallArgumentsEx* args = s_argsArray.first;
	
	while (args)
	{
		UICallArgumentsEx* toDelete = args;
		args = args->next;
		
		free(toDelete);
	}
	
	s_argsArray.count = 0;
	s_argsArray.first = NULL;
	s_argsArray.last = NULL;
}

bool get_string_from_array(uint32_t lang, uint32_t address, UIStringPattern** pattern)
{
	*pattern = NULL;
	
	UIStringPattern* tmpPattern = s_stringArray[lang].first;
	
	while (tmpPattern)
	{
		if (tmpPattern->address == address)
		{
			*pattern = tmpPattern;
			break;
		}
		
		tmpPattern = tmpPattern->next;
	}
	
	return true;
}

bool add_string_to_array(uint32_t lang, uint8_t* data, uint32_t count, UIStringPattern** pattern)
{
	uint32_t* objects = (uint32_t*)data;
	
	// create pattern
	UIStringPattern* newPattern = (UIStringPattern*)malloc(sizeof(UIStringPattern));
	memset(newPattern, 0, sizeof(UIStringPattern));
	
	// fill pattern
	newPattern->complete = false;
	newPattern->menuIdx = 0xFFFFFFFF;
	newPattern->size = count;

	for (uint32_t i=0; i < count; i++)
	{
		newPattern->pattern[i] = Swap32(objects[i]);
	}
	
	// add pattern to array
	if (s_stringArray[lang].first != NULL)
		s_stringArray[lang].last->next = newPattern;
	else
		s_stringArray[lang].first = newPattern;
	
	s_stringArray[lang].last = newPattern;
	s_stringArray[lang].count++;
	
	*pattern = newPattern;
	
	return true;
}

void delete_string_arrays()
{
	for (uint32_t lang=0; lang < kMaxLanguageCount; lang++)
	{
		UIStringPattern* pattern = s_stringArray[lang].first;
		
		while (pattern)
		{
			UIStringPattern* toDelete = pattern;
			pattern = pattern->next;
			
			free(toDelete);
		};
		
		s_stringArray[lang].count = 0;
		s_stringArray[lang].first = NULL;
		s_stringArray[lang].last = NULL;
	}
}

// -------------- MARK: Implementation: String operations

/*
 Characters address lookup:
 - find address which are related only to some particular language
 - replace these unique characters with 0x20+(adr index), for all other chars use '_'
 */

bool update_charmap(uint32_t lang, uint32_t menu_str, uint32_t index, UIBasePattern* pattern)
{
	for (uint32_t j=0; j<256; j++)
	{
		if (s_charMap[j].address == 0)
		{
			s_charMap[j].address = pattern->pattern[index];
			s_charMap[j].chr = s_menuText[lang][menu_str][index];
			
			s_charMapSize = j+1;
			
			break;
		}
		else if (s_charMap[j].address == pattern->pattern[index])
		{
			break;
		}
	}
	
	return true;
}

bool find_string_object(uint8_t lang, UIStringPattern* pattern)
{
	bool res = false;
	
	UIBasePattern tmpPattern;
	uint32_t index = 1;
	uint32_t found = 0;
	uint32_t idx = 0;

	memset(&tmpPattern, 0, sizeof(UIBasePattern));
	tmpPattern.size = pattern->size;
	tmpPattern.value = 0;
	
	// generate pattern
	for (uint32_t i=0; i < tmpPattern.size; i++)
	{
		if (tmpPattern.pattern[i] != 0)
			continue;
		
		tmpPattern.pattern[i] = index;
		
		for (uint32_t j=i+1; j < tmpPattern.size; j++)
		{
			if (pattern->pattern[i] == pattern->pattern[j])
			{
				tmpPattern.pattern[j] = index;
				tmpPattern.value++;
			}
		}
		
		index++;
	}
	
	if (tmpPattern.value < PATTERN_THRESHOLD)
		return res;
	
	// find pattern
	for (idx = 0; idx < MENU_STRING_COUNT; idx++)
	{
		if (s_menuTextPatterns[lang][idx].size != tmpPattern.size)
			continue;
		
		if (memcmp(s_menuTextPatterns[lang][idx].pattern, tmpPattern.pattern, tmpPattern.size * sizeof(uint32_t)) == 0)
		{
			found++;
			
			for (uint32_t i = 0; i < tmpPattern.size; i++)
			{
				s_menuTextPatterns[lang][idx].pattern[i] = pattern->pattern[i];
				update_charmap(lang, idx, i, &s_menuTextPatterns[lang][idx]);
			}
			res = true;
		}
	}
	
	return res;
}

bool recover_charmap()
{
	for (uint32_t lang=0; lang < kMaxLanguageCount; lang++)
	{
		if (s_languageSupport[lang] != true)
			continue;
		
		UIStringPattern* pattern = s_stringArray[lang].first;
		
		while (pattern)
		{
			bool missed = false;
			uint32_t bestMatch = 0;
			UIBasePattern tmpPattern;
			// fill pattern->string from charmap
			for (uint32_t i=0; i < pattern->size; i++)
			{
				wchar_t chr = 0;
				for (uint32_t c=0; c < s_charMapSize; c++)
				{
					if (pattern->pattern[i] == s_charMap[c].address)
					{
						chr = s_charMap[c].chr;
						break;
					}
				}
				pattern->string[i] = chr;
				if (chr == 0)
					missed = true;
			}
			
			if (missed == true)
			{
				// generate pattern
				uint32_t index = 1;
				
				memset(&tmpPattern, 0, sizeof(UIBasePattern));
				tmpPattern.size = pattern->size;
				tmpPattern.value = 0;
				
				for (uint32_t i=0; i < tmpPattern.size; i++)
				{
					if (tmpPattern.pattern[i] != 0)
						continue;
					
					tmpPattern.pattern[i] = index;
					
					for (uint32_t j=i+1; j < tmpPattern.size; j++)
					{
						if (pattern->pattern[i] == pattern->pattern[j])
						{
							tmpPattern.pattern[j] = index;
							tmpPattern.value++;
						}
					}
					
					index++;
				}
			}
			else
			{
				pattern->complete = true;
			}
			
			// find menu string
			for (uint32_t m=0; m < MENU_STRING_COUNT; m++)
			{
				if (pattern->size != s_menuTextPatterns[lang][m].size)
					continue;
				
				const wchar_t* menuString = s_menuText[lang][m];
				
				if (missed == true)
				{
					// compare pattern
					if (memcmp(s_menuTextPatterns[lang][m].pattern, tmpPattern.pattern, tmpPattern.size * sizeof(uint32_t)) != 0)
						continue;
					
					uint32_t match = 0;

					for (uint32_t i=0; i < pattern->size; i++)
					{
						if (pattern->string[i] == 0)
							continue;
						
						if (pattern->string[i] == menuString[i])
							match++;
					}
					
					if (match > bestMatch)
					{
						bestMatch = match;
						if (bestMatch > 3)
						{
							pattern->menuIdx = m;
						}
					}
				}
				else
				{
					if (wcsncasecmp(menuString, pattern->string, pattern->size) == 0)
					{
						pattern->menuIdx = m;
						break;
					}
				}
			}
			
			if (pattern->menuIdx != 0xFFFFFFFF)
			{
				// add missed chars to charmap
				if (missed)
				{
					for (uint32_t i=0; i < pattern->size; i++)
					{
						if (pattern->string[i] == 0)
						{
							update_charmap(lang, pattern->menuIdx, i, pattern);
						}
					}
				}
			}
						
			pattern = pattern->next;
		}
	}
	
	return true;
}

bool recover_incomplete_strings()
{
	for (uint32_t lang=0; lang < kMaxLanguageCount; lang++)
	{
		if (s_languageSupport[lang] != true)
			continue;
		
		UIStringPattern* pattern = s_stringArray[lang].first;
		
		while (pattern)
		{
			bool missed = false;
		
			if (pattern->complete == false)
			{
				// fill pattern->string from charmap
				for (uint32_t i=0; i < pattern->size; i++)
				{
					wchar_t chr = 0;
					for (uint32_t c=0; c < s_charMapSize; c++)
					{
						if (pattern->pattern[i] == s_charMap[c].address)
						{
							chr = s_charMap[c].chr;
							break;
						}
					}
					pattern->string[i] = chr;
					if (chr == 0)
						missed = true;
				}
				
				if (missed == false)
					pattern->complete = true;
			}
			
			pattern = pattern->next;
		}
	}
	
	return true;
}

bool dump_all_strings()
{
	wchar_t string[256] = {0};
	
	for (uint32_t lang=0; lang < kMaxLanguageCount; lang++)
	{
		if (s_languageSupport[lang] != true)
			continue;
	
		printf("--- %s ---\n", s_languageString[lang]);
		UIStringPattern* pattern = s_stringArray[lang].first;
		
		while (pattern)
		{
			if (pattern->complete == false)
			{
				memset(string, 0, 256*sizeof(wchar_t));
				wprintf(L"[%.8X|%3d]: ", pattern->address, pattern->menuIdx);
				for (uint32_t i=0; i < pattern->size; i++)
				{
					if (pattern->string[i] == 0)
						string[i] = L'_';
					else
						string[i] = pattern->string[i];
				}
				wprintf(L"%S (incomplete)\n", string);
			}
			else
			{
				wprintf(L"[%.8X|%3d]: %ls\n", pattern->address, pattern->menuIdx, pattern->string);
			}
			
			pattern = pattern->next;
		}
	}
	
	return true;
}

bool find_lang_address(uint8_t* buffer, uint32_t buf_size)
{
	uint8_t post_lang_pattern1[] = {0x38, 0x38, 0x00, 0x30, 0x30, 0x31};
	uint8_t post_lang_pattern2[] = {0x38, 0x00, 0x30, 0x30, 0x31, 0x2E};
	
	void* pat_offset = memmem(buffer, buf_size, post_lang_pattern1, 6);
	if (pat_offset == nullptr)
	{
		pat_offset = memmem(buffer, buf_size, post_lang_pattern2, 6);
		if (pat_offset == nullptr)
			return false;
	}
	
	//	ROM:100660                 LDI:32  #unk_801D4C00, R0
	//	ROM:100666                 LDI:32  #byte_58CD94, R1
	//	ROM:10066C                 LDI:32  #0x185E0, R13
	//  s_language (2 bytes) is located at right before string "88 001" inside the blob 0x58CD94 which
	//  will be moved to 0x801D4C00
	
	// 0x801D6E22 | 0x801D6F1A
	uint32_t mem_offset = Swap32(*((uint32_t*)(buffer + 0x662)));
	uint32_t blob_offset = Swap32(*((uint32_t*)(buffer + 0x668))) - s_imageBase;
	
	s_languageAddress = (uint32_t)((uintptr_t)pat_offset - (uintptr_t)buffer) - 2 + mem_offset - blob_offset;
	wprintf(L"\n");
	wprintf(L"s_language found at 0x%.8x\n", s_languageAddress);
	wprintf(L"\n");
	
	return true;
}

void ayuv_to_argb(UIColorAYUV* ayuv, UIColorARGB* argb)
{
	// http://www.fourcc.org/fccyvrgb.php
	// http://msdn.microsoft.com/en-us/library/ms893078.aspx
	
	int32_t Y, U, V;
	int32_t R,G,B;
	
	Y = ayuv->Y;
	U = ayuv->U;
	V = ayuv->V;

#if 0
	B = 1.164*(Y - 16)                   + 2.018*(U - 128);
	G = 1.164*(Y - 16) - 0.813*(V - 128) - 0.391*(U - 128);
	R = 1.164*(Y - 16) + 1.596*(V - 128);
#else
	int32_t C,D,E;

	C = Y - 16;
	D = U - 128;
	E = V - 128;
	
	R = ( 298 * C           + 409 * E + 128) >> 8;
	G = ( 298 * C - 100 * D - 208 * E + 128) >> 8;
	B = ( 298 * C + 516 * D           + 128) >> 8;
#endif
	
	argb->A = (ayuv->A * 0xFF)/0x7;
	argb->R = ClipColor(R);
	argb->G = ClipColor(G);
	argb->B = ClipColor(B);
}

bool dump_image_to_file(uint8_t* buffer, uint32_t buf_size, char* filename)
{
	if (filename == NULL)
		return false;
	
	uint32_t num;
	uint32_t color;
	uint32_t idx = 0;
	RGBApixel pixel;
	UIColorARGB argb;
	
	uint8_t* data = buffer;
	
	BMP bmp;
	uint8_t* image = NULL;
	
	uint16_t width = *((uint16_t*)data);
	width = Swap16(width);
	width <<= 1;
	data += 2;
	
	uint16_t height = *((uint16_t*)data);
	height = Swap16(height);
	height <<= 1;
	data += 2;
	
    uint16_t chr = *((uint16_t*)data);
    chr = Swap16(chr);
    data += 2;

	uint32_t count = width*height;
	
	if (s_toolFlags & kToolFlag_DumpBMP)
	{
		bmp.SetSize(width, height);
		bmp.SetBitDepth(32);
	}
	else if (s_toolFlags & kToolFlag_DumpPNG)
	{
		image = (uint8_t*)malloc(sizeof(uint32_t) * width * height);
	}
	
	while (idx < count)
	{
		num = data[0];
		color = data[1];
		
		if (num != 0)
		{
			ayuv_to_argb(&s_lut[color], &argb);
			pixel.Alpha = argb.A;
			pixel.Red = argb.R;
			pixel.Green = argb.G;
			pixel.Blue = argb.B;
			
			for (uint32_t i=0; (i<num) && (idx+i < count); i++)
			{
				if (s_toolFlags & kToolFlag_DumpBMP)
				{
					bmp.SetPixel((idx+i)%width, (idx+i)/width, pixel);
				}
				else if (s_toolFlags & kToolFlag_DumpPNG)
				{
					image[((idx+i)*4) + 0] = pixel.Red;
					image[((idx+i)*4) + 1] = pixel.Green;
					image[((idx+i)*4) + 2] = pixel.Blue;
					image[((idx+i)*4) + 3] = pixel.Alpha;
				}
			}
		}
		
		idx += num;
		data += 2;
	}
	
	if (s_toolFlags & kToolFlag_DumpBMP)
	{
		bmp.WriteToFile(filename);
	}
	else if (s_toolFlags & kToolFlag_DumpPNG)
	{
		lodepng_encode32_file(filename, image, width, height);
		
		if (image)
			free(image);
	}
	
	return true;
}

// -------------- MARK: Implementation: Firmware operations

bool parse_func_arguments(uint8_t* buffer, uint32_t buf_size, UICallArgumentsEx* args)
{
	bool res = false;
	uint32_t arg_idx = 0;
	
	args->dstAddress = 0;
	
	for (uint32_t i=0; i<buf_size; i+=2)
	{
		switch (arg_idx)
		{
			case 0:
			{
				uint16_t ldi8 = *((uint16_t*)(buffer+i));
				ldi8 = Swap16(ldi8);
				
				if ((ldi8 & ~kFROpcode_LDI8_ValMask) == (kFROpcode_LDI8 | 0x5))
				{
					args->type = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					arg_idx++;
				}
				else
				{
					VERBOSE("ARG0 - BUG");
				}
			}
			break;
			case 1:
			{
				uint16_t ldi8 = *((uint16_t*)(buffer+i));
				ldi8 = Swap16(ldi8);
				
				if ((ldi8 & ~kFROpcode_LDI8_ValMask) == (kFROpcode_LDI8 | 0x6))
				{
					args->target = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					arg_idx++;
				}
				else
				{
					VERBOSE("ARG1 - BUG");
				}
			}
			break;
			case 2:
			{
				uint16_t ldi32 = *((uint16_t*)(buffer+i));
				ldi32 = Swap16(ldi32);
			
				if (ldi32 == (kFROpcode_LDI32 | 0x7))
				{
					args->descAddress = *((uint32_t*)(buffer+i+2));
					args->descAddress = Swap32(args->descAddress);
					arg_idx++;
					
					i += sizeof(uint32_t);
				}
				else
				{
					VERBOSE("ARG2 - BUG");
				}
			}
			break;
			case 3:
			{
				uint16_t ldi8 = *((uint16_t*)(buffer+i));
				uint16_t st = *((uint16_t*)(buffer+i+2));
				
				ldi8 = Swap16(ldi8);
				st = Swap16(st);

				uint8_t reg = ldi8 & 0xF;
				
				if (((ldi8 & ~kFROpcode_LDI8_ValMask) == (kFROpcode_LDI8 | reg)) &&
					st == (kFROpcode_ST | 0x00 | reg ))
				{
					args->always0 = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					arg_idx++;
					
					i += sizeof(uint16_t);
				}
				else
				{
					VERBOSE("ARG3 - BUG");
				}
			}
			break;
			case 4:
			{
				uint16_t ldi8 = *((uint16_t*)(buffer+i));
				uint16_t st = *((uint16_t*)(buffer+i+2));
				
				ldi8 = Swap16(ldi8);
				st = Swap16(st);

				uint8_t reg = ldi8 & 0xF;
				
				if (((ldi8 & ~kFROpcode_LDI8_ValMask) == (kFROpcode_LDI8 | reg)) &&
					st == (kFROpcode_ST | 0x10 | reg))
				{
					args->index1 = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					arg_idx++;
					
					i += sizeof(uint16_t);
				}
				else
				{
					VERBOSE("ARG4 - BUG");
				}
			}
			break;
			case 5:
			{
				uint16_t ldi8 = *((uint16_t*)(buffer+i));
				uint16_t st = *((uint16_t*)(buffer+i+2));
				
				ldi8 = Swap16(ldi8);
				st = Swap16(st);

				uint8_t reg = ldi8 & 0xF;
				
				if (((ldi8 & ~kFROpcode_LDI8_ValMask) == (kFROpcode_LDI8 | reg)) &&
					st == (kFROpcode_ST | 0x20 | reg))
				{
					args->index2 = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					arg_idx++;
					
					i += sizeof(uint16_t);
				}
				else
				{
					VERBOSE("ARG5 - BUG");
				}
			}
			break;
			case 6:
			{
				uint16_t cmd = *((uint16_t*)(buffer+i));
				cmd = Swap16(cmd);
				
				if ((cmd & ~(kFROpcode_LDI8_ValMask | kFROpcode_LDI8_RegMask)) == kFROpcode_LDI8)
				{
					uint16_t ldi8 = cmd;
					uint16_t st = *((uint16_t*)(buffer+i+2));
					st = Swap16(st);
					
					uint8_t reg = ldi8 & 0xF;
					
					if (st == (kFROpcode_ST | 0x30 | reg))
					{
						args->x_offset = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
						arg_idx++;
						
						i += sizeof(uint16_t);
					}
					else
					{
						VERBOSE("ARG6 - BUG");
					}
				}
				else if ((cmd & ~kFROpcode_LDI20_RegMask) == kFROpcode_LDI20)
				{
					uint16_t ldi20 = cmd;
					uint16_t st = *((uint16_t*)(buffer+i+4));
					st = Swap16(st);
					
					uint8_t reg = ldi20 & 0xF;
					
					if (st == (kFROpcode_ST | 0x30 | reg))
					{
						args->x_offset = *((uint16_t*)(buffer+i+2));
						args->x_offset = Swap16(args->x_offset);
						arg_idx++;
						
						i += sizeof(uint16_t) * 2;
					}
					else
					{
						VERBOSE("ARG6 - BUG");
					}
				}
				else
				{
					VERBOSE("ARG6 - BUG");
				}
			}
			break;
			case 7:
			{
				uint16_t cmd = *((uint16_t*)(buffer+i));
				cmd = Swap16(cmd);
				
				if ((cmd & ~(kFROpcode_LDI8_ValMask | kFROpcode_LDI8_RegMask)) == kFROpcode_LDI8)
				{
					uint16_t ldi8 = cmd;
					uint16_t st = *((uint16_t*)(buffer+i+2));
					st = Swap16(st);
					
					uint8_t reg = ldi8 & 0xF;
					
					if (st == (kFROpcode_ST | 0x40 | reg))
					{
						args->y_offset = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
						arg_idx++;
						
						i += sizeof(uint16_t);
					}
					else
					{
						VERBOSE("ARG7 - BUG");
					}
				}
				else if ((cmd & ~kFROpcode_LDI20_RegMask) == kFROpcode_LDI20)
				{
					uint16_t ldi20 = cmd;
					uint16_t st = *((uint16_t*)(buffer+i+4));
					st = Swap16(st);
					
					uint8_t reg = ldi20 & 0xF;
					
					if (st == (kFROpcode_ST | 0x40 | reg))
					{
						args->y_offset = *((uint16_t*)(buffer+i+2));
						args->y_offset = Swap16(args->y_offset);
						arg_idx++;
						
						i += sizeof(uint16_t) * 2;
					}
					else
					{
						VERBOSE("ARG7 - BUG");
					}
				}
				else
				{
					VERBOSE("ARG7 - BUG");
				}
			}
			break;
			case 8:
			{
				uint16_t ldi8 = *((uint16_t*)(buffer+i));
				uint16_t st = *((uint16_t*)(buffer+i+2));
				
				ldi8 = Swap16(ldi8);
				st = Swap16(st);

				uint8_t reg = ldi8 & 0xF;
				
				if (((ldi8 & ~kFROpcode_LDI8_ValMask) == (kFROpcode_LDI8 | reg)) &&
					st == (kFROpcode_ST | 0x50 | reg))
				{
					args->unknown2 = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					arg_idx++;
					
					i += sizeof(uint16_t);
				}
				else
				{
					VERBOSE("ARG8 - BUG");
				}
			}
			break;
			case 9:
			{
				uint16_t cmd = *((uint16_t*)(buffer+i));
				cmd = Swap16(cmd);
				
				if ((cmd & ~(kFROpcode_LDI8_ValMask | kFROpcode_LDI8_RegMask)) == kFROpcode_LDI8)
				{
					uint8_t val = (cmd & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					uint8_t reg = cmd & 0xF;
					
					uint16_t st = 0;
					if (val == 0xFF)
						st = *((uint16_t*)(buffer+i+4));
					else
						st = *((uint16_t*)(buffer+i+2));
					st = Swap16(st);
					
					if (st == (kFROpcode_ST | 0x60 | reg))
					{
						if (val == 0xFF)
							args->language1 = 0xFFFFFFFF;
						else
							args->language1 = val;
						arg_idx++;
						
						if (val == 0xFF)
							i += sizeof(uint16_t) * 2;
						else
							i += sizeof(uint16_t);
					}
					else
					{
						VERBOSE("ARG9 - BUG");
					}
				}
				else if ((cmd & ~kFROpcode_LDI32_RegMask) == kFROpcode_LDI32)
				{
					uint16_t lduh = *((uint16_t*)(buffer+i+6));
					uint16_t st1 = *((uint16_t*)(buffer+i+10));
					uint16_t st2 = *((uint16_t*)(buffer+i+24));
					uint16_t st3 = *((uint16_t*)(buffer+i+28));
					lduh = Swap16(lduh);
					st1 = Swap16(st1);
					st2 = Swap16(st2);
					st3 = Swap16(st3);
					
					uint8_t reg = lduh & 0xF;
					
					if (st1 == (kFROpcode_ST | 0x60 | reg))
					{
						args->language1 = *((uint32_t*)(buffer+i+2));
						args->language1 = Swap32(args->language1);
						arg_idx++;
						
						i += sizeof(uint16_t) * 5;
					}
					else if (st2 == (kFROpcode_ST | 0x60 | reg))
					{
						args->language1 = *((uint32_t*)(buffer+i+2));
						args->language1 = Swap32(args->language1);
						arg_idx++;
						
						i += sizeof(uint16_t) * 12;
					}
					else if (st3 == (kFROpcode_ST | 0x60 | reg))
					{
						args->language1 = *((uint32_t*)(buffer+i+2));
						args->language1 = Swap32(args->language1);
						arg_idx++;
						
						i += sizeof(uint16_t) * 14;
					}
					else
					{
						VERBOSE("ARG9 - BUG");
					}
				}
				else
				{
					VERBOSE("ARG9 - BUG");
				}
			}
			break;
			case 10:
			{
				uint16_t cmd = *((uint16_t*)(buffer+i));
				cmd = Swap16(cmd);
				
				if ((cmd & ~(kFROpcode_LDI8_ValMask | kFROpcode_LDI8_RegMask)) == kFROpcode_LDI8)
				{
					uint8_t val = (cmd & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
					uint8_t reg = cmd & 0xF;
					
					uint16_t st = 0;
					if (val == 0xFF)
						st = *((uint16_t*)(buffer+i+4));
					else
						st = *((uint16_t*)(buffer+i+2));
					st = Swap16(st);
					
					if (st == (kFROpcode_ST | 0x70 | reg))
					{
						if (val == 0xFF)
							args->language2 = 0xFFFFFFFF;
						else
							args->language2 = val;
						arg_idx++;
						
						if (val == 0xFF)
							i += sizeof(uint16_t) * 2;
						else
							i += sizeof(uint16_t);
					}
					else
					{
						VERBOSE("ARG10 - BUG");
					}
				}
				else if ((cmd & ~kFROpcode_LDI32_RegMask) == kFROpcode_LDI32)
				{
					uint16_t lduh = *((uint16_t*)(buffer+i+6));
					uint16_t st1 = *((uint16_t*)(buffer+i+10));
					uint16_t st2 = *((uint16_t*)(buffer+i+24));
					uint16_t st3 = *((uint16_t*)(buffer+i+28));
					lduh = Swap16(lduh);
					st1 = Swap16(st1);
					st2 = Swap16(st2);
					st3 = Swap16(st3);
					
					uint8_t reg = lduh & 0xF;
					
					if (st1 == (kFROpcode_ST | 0x70 | reg))
					{
						args->language2 = *((uint32_t*)(buffer+i+2));
						args->language2 = Swap32(args->language2);
						arg_idx++;
						
						i += sizeof(uint16_t) * 5;
					}
					else if (st2 == (kFROpcode_ST | 0x70 | reg))
					{
						args->language2 = *((uint32_t*)(buffer+i+2));
						args->language2 = Swap32(args->language2);
						arg_idx++;
						
						i += sizeof(uint16_t) * 12;
					}
					else if (st3 == (kFROpcode_ST | 0x70 | reg))
					{
						args->language2 = *((uint32_t*)(buffer+i+2));
						args->language2 = Swap32(args->language2);
						arg_idx++;
						
						i += sizeof(uint16_t) * 14;
					}
					else
					{
						VERBOSE("ARG10 - BUG");
					}
				}
				else
				{
					VERBOSE("ARG10 - BUG");
				}
			}
			break;
			case 11:
			{
				uint16_t cmd = *((uint16_t*)(buffer+i));
				cmd = Swap16(cmd);
				
				if ((cmd & ~(kFROpcode_LDI8_ValMask | kFROpcode_LDI8_RegMask)) == kFROpcode_LDI8)
				{
					uint16_t ldi8 = cmd;
					uint16_t st = *((uint16_t*)(buffer+i+2));
					st = Swap16(st);
					
					uint8_t reg = ldi8 & 0xF;
					
					if (st == (kFROpcode_ST | 0x80 | reg))
					{
						args->funcAddress = (ldi8 & kFROpcode_LDI8_ValMask) >> kFROpcode_LDI8_ValShift;
						arg_idx++;
						
						res = true;
					}
					else
					{
						VERBOSE("ARG11 - BUG");
					}
				}
				else if ((cmd & ~kFROpcode_LDI32_RegMask) == kFROpcode_LDI32)
				{
					uint16_t ldi32 = cmd;
					uint16_t st = *((uint16_t*)(buffer+i+6));
					st = Swap16(st);
					
					uint8_t reg = ldi32 & 0xF;
					
					if (st == (kFROpcode_ST | 0x80 | reg))
					{
						args->funcAddress = *((uint16_t*)(buffer+i+2));
						args->funcAddress = Swap16(args->funcAddress);
						
						res = true;
					}
					else
					{
						VERBOSE("ARG11 - BUG");
					}
				}
				else
				{
					VERBOSE("ARG11 - BUG");
				}
			}
			break;
		}
		
		if (res == true)
			break;
	}
	
bail:
	
	return res;
}

bool get_all_strings(uint8_t* buffer, uint32_t buf_size)
{
	bool res = false;
	
	UICallArgumentsEx* args = s_argsArray.first;
	
	while (args)
	{
		uint8_t* descHeader = buffer + args->descAddress - s_imageBase;
		uint8_t* descEntry = NULL;
		
		if (args->type == kUIDescType_1)
		{
			bool text = (args->language1 == s_languageAddress)? true:false;
			
			if (((args->descAddress & 0xFF000000) != 0x80000000))
			{
				UIDescType1Header header;
				uint32_t items;
				
				memcpy(&header, descHeader, sizeof(UIDescType1Header));
				header.address = Swap32(header.address);
				header.entries = Swap16(header.entries);
				
				items = header.entries / kMaxLanguageCount;
				
				descEntry = buffer + header.address - s_imageBase;
				
				VERBOSE("  0x%.8X: Localization strings\n", header.address);
				
				for (uint32_t i=0; i < header.entries; i++)
				{
					UIDescType1Entry entry;
					
					memcpy(&entry, descEntry, sizeof(UIDescType1Entry));
					entry.address = Swap32(entry.address);
					entry.objects = Swap16(entry.objects);
					
					if (entry.address != 0)
					{
						uint8_t* adr_data = buffer + entry.address - s_imageBase;
						
						if (text && ((entry.address & 0xFF000000) != 0x80000000))
						{
							UIStringPattern* pattern;
							
							VERBOSE("    0x%.8X [%4d | 0x%.4X]: %s\n", entry.address, entry.objects, entry.objects, s_languageString[i/items]);
							
							add_string_to_array(i/items, adr_data, entry.objects, &pattern);
							
							pattern->address = entry.address;
							
							if (s_languageSupport[i/items])
							{
								find_string_object(i/items, pattern);
							}
						}
					}
					
					descEntry += sizeof(UIDescType1Entry);
				}
			}
		}
		
		args = args->next;
	};
	
	return res;
}

bool dump_ui_resources(uint8_t* buffer, uint32_t buf_size)
{
	bool res = false;
	uint32_t block = 0;
	struct stat st;

	if (s_toolFlags & kToolFlag_DumpBMP || s_toolFlags & kToolFlag_DumpPNG)
	{
		if (stat(s_dumpFolder, &st) == -1)
			mkdir(s_dumpFolder, 0700);
	}

	VERBOSE("\n");
	
	UICallArgumentsEx* args = s_argsArray.first;
	
	while (args)
	{
		if (args->index2 == 0 && args != s_argsArray.first)
		{
			VERBOSE("  ================================================[ END OF BLOCK %3d ]================================================\n\n", block);
			block++;
		}
		
		VERBOSE("  0x%.8X: gui_CopyImageDesc", args->callAddress);
		
#if 1
		VERBOSE("( R4, %2d, %2d, 0x%.8X, %2d, %2d, %2d, 0x%.4X, 0x%.4X, 0x%.2X, 0x%.8X, 0x%.8X, %2d) ",
				args->type,
				args->target,
				args->descAddress,
				args->always0,
				args->index1,
				args->index2,
				args->x_offset,
				args->y_offset,
				args->unknown2,
				args->language1,
				args->language2,
				args->funcAddress);
		if (args->always0 != 0)
		{
			VERBOSE(" !!!");
		}
		VERBOSE("\n");
#else
		VERBOSE(":\n");
		VERBOSE("    dstAddress  = R4\n");
		VERBOSE("    type        = %d\n", args->type);
		VERBOSE("    unknown1    = %d\n", args->unknown1);
		VERBOSE("    descAddress = 0x%.8X\n", args->descAddress);
		VERBOSE("    always0     = %d %s\n", args->always0, (args->always0 != 0)? "!!!" : "");
		VERBOSE("    index1      = %d\n", args->index1);
		VERBOSE("    index2      = %d\n", args->index2);
		VERBOSE("    x_offset    = 0x%.4X\n", args->x);
		VERBOSE("    y_offset    = 0x%.4X\n", args->y);
		VERBOSE("    unknown2    = 0x%.2X\n", args->unknown2);
		VERBOSE("    language1   = 0x%.8X\n", args->language1);
		VERBOSE("    language2   = 0x%.8X\n", args->language2);
		VERBOSE("    funcAddress = 0x%.8X\n", args->funcAddress);
#endif
		
		VERBOSE("\n");
		
		uint8_t* descHeader = buffer + args->descAddress - s_imageBase;
		uint8_t* descEntry = NULL;
		uint8_t* data = NULL;
		
		if (args->type == kUIDescType_0)
		{
			if (((args->descAddress & 0xFF000000) != 0x80000000))
			{
				UIDescType0Header header;
				
				memcpy(&header, descHeader, sizeof(UIDescType0Header));
				header.address = Swap32(header.address);
				header.entries = Swap16(header.entries);
				header.unknown = Swap16(header.unknown);
				
				VERBOSE("    0x%.8X: type 0\n", args->descAddress);
				VERBOSE("      address = %.8X %s\n", header.address, ((header.address & 0xFF000000) == 0x80000000)? "(reg)" : "" );
				VERBOSE("      entries = %.4X\n", header.entries);
				VERBOSE("      unknown = %.4X\n", header.unknown);
				VERBOSE("\n");
				
				if (s_toolFlags & kToolFlag_IDC)
				{
					fprintf(s_idcFile, "\n\tMakeUnknown(0x%.8X, sizeof(\"UIDescType0Header\"), 0);\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeStructEx(0x%.8X, -1, \"UIDescType0Header\");\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeComm(0x%.8X, \"Image Description Type 0\");\n", args->descAddress);
				}
				
				descEntry = buffer + header.address - s_imageBase;
				
				for (uint32_t i=0; i < header.entries; i++)
				{
					UIDescType0Entry entry;
					
					memcpy(&entry, descEntry, sizeof(UIDescType0Entry));
					entry.x_offset = Swap16(entry.x_offset);
					entry.y_offset = Swap16(entry.y_offset);
					entry.address = Swap32(entry.address);
					
					VERBOSE("      [%2d] 0x%.8lX:\n", i, header.address + (i * sizeof(UIDescType0Entry)));
					VERBOSE("           unknown1 = %.4X\n", entry.x_offset);
					VERBOSE("           unknown2 = %.4X\n", entry.y_offset);
					VERBOSE("           address  = %.8X\n", entry.address);
					VERBOSE("\n");
					
					if (s_toolFlags & kToolFlag_IDC)
					{
						fprintf(s_idcFile, "\t\tMakeUnknown(0x%.8lX, sizeof(\"UIDescType0Entry\"), 0);\n", header.address + (i * sizeof(UIDescType0Entry)));
						fprintf(s_idcFile, "\t\tMakeStructEx(0x%.8lX, -1, \"UIDescType0Entry\");\n", header.address + (i * sizeof(UIDescType0Entry)));
					}
					
					if (entry.address != 0)
					{
						data = buffer + entry.address - s_imageBase;
						
						uint32_t data_size = 0;
                        find_data_size(data, buf_size - (uint32_t)(data-buffer), &data_size);
                        VERBOSE("           Size: %4d | 0x%.4X\n", data_size, data_size);
                        
                        if (s_toolFlags & kToolFlag_IDC)
                        {
                            fprintf(s_idcFile, "\t\t\tMakeUnknown(0x%.8X, 0x%X, 0);\n", entry.address, data_size);
                            fprintf(s_idcFile, "\t\t\tMakeWord(0x%.8X);\n", entry.address);
                            fprintf(s_idcFile, "\t\t\tMakeArray(0x%.8X, 0x%X);\n", entry.address, data_size/2);
                        }
						
						if (s_toolFlags & kToolFlag_DumpBMP || s_toolFlags & kToolFlag_DumpPNG)
						{
							char filename[256] = {0};
							strcpy(filename, s_dumpFolder);
							strcat(filename, "/Type0/");
							
							if (stat(filename, &st) == -1)
								mkdir(filename, 0700);

							sprintf(filename,"%s/Type0/%.8X", s_dumpFolder, entry.address);
							
							if (s_toolFlags & kToolFlag_DumpBMP)
								strcat(filename, ".bmp");
							else if (s_toolFlags & kToolFlag_DumpPNG)
								strcat(filename, ".png");
							
							if (file_exists(filename) == false)
							{
								dump_image_to_file(data, buf_size - (uint32_t)(data-buffer), filename);
							}
						}
						
						if (s_bytesToDisp)
						{
							display_bytes(data, s_bytesToDisp, 11);
						}

						VERBOSE("\n");
					}
					
					descEntry += sizeof(UIDescType0Entry);
				}
			}
			else
			{
				VERBOSE("    0x%.8X: type 0 (reg)\n", args->descAddress);
				VERBOSE("\n");
			}
		}
		else if (args->type == kUIDescType_1)
		{
			bool text = (args->language1 == s_languageAddress)? true:false;
			
			if (((args->descAddress & 0xFF000000) != 0x80000000))
			{
				UIDescType1Header header;
				uint32_t items;
				
				memcpy(&header, descHeader, sizeof(UIDescType1Header));
				header.address = Swap32(header.address);
				header.entries = Swap16(header.entries);
				header.unknown = Swap16(header.unknown);
				
				items = header.entries / kMaxLanguageCount;
				
				VERBOSE("    0x%.8X: type 1\n", args->descAddress);
				VERBOSE("      address = %.8X %s\n", header.address, ((header.address & 0xFF000000) == 0x80000000)? "(reg)" : "" );
				VERBOSE("      entries = %.4X\n", header.entries);
				VERBOSE("      unknown = %.4X\n", header.unknown);
				VERBOSE("\n");
				
				if (s_toolFlags & kToolFlag_IDC)
				{
					fprintf(s_idcFile, "\n\tMakeUnknown(0x%.8X, sizeof(\"UIDescType1Header\"), 0);\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeStructEx(0x%.8X, -1, \"UIDescType1Header\");\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeComm(0x%.8X, \"Image Description Type 1\");\n", args->descAddress);
				}
				
				descEntry = buffer + header.address - s_imageBase;
				
				for (uint32_t i=0; i < header.entries; i++)
				{
					UIDescType1Entry entry;
					
					memcpy(&entry, descEntry, sizeof(UIDescType1Entry));
					entry.x_offset = Swap16(entry.x_offset);
					entry.y_offset = Swap16(entry.y_offset);
					entry.address = Swap32(entry.address);
					entry.objects = Swap16(entry.objects);
					entry.total_w = Swap16(entry.total_w);
					entry.total_h = Swap16(entry.total_h);
					entry.unknown = Swap16(entry.unknown);
					
					VERBOSE("      [%2d] 0x%.8lX (%s):\n", i, header.address + (i * sizeof(UIDescType1Entry)), s_languageString[i/items]);
					VERBOSE("           x_offset = %.4X\n", entry.x_offset);
					VERBOSE("           y_offset = %.4X\n", entry.y_offset);
					VERBOSE("           address  = %.8X %s\n", entry.address, ((entry.address & 0xFF000000) == 0x80000000)? "(reg)" : "" );
					VERBOSE("           objects  = %.4X\n", entry.objects);
					VERBOSE("           total_w  = %.4X\n", entry.total_w);
					VERBOSE("           total_h  = %.4X\n", entry.total_h);
					VERBOSE("           unknown  = %.4X\n", entry.unknown);
					VERBOSE("\n");
					
					if (s_toolFlags & kToolFlag_IDC)
					{
						fprintf(s_idcFile, "\t\tMakeUnknown(0x%.8lX, sizeof(\"UIDescType1Entry\"), 0);\n", header.address + (i * sizeof(UIDescType1Entry)));
						fprintf(s_idcFile, "\t\tMakeStructEx(0x%.8lX, -1, \"UIDescType1Entry\");\n", header.address + (i * sizeof(UIDescType1Entry)));
					}
					
					if (entry.address != 0)
					{
						uint8_t* adr_data = buffer + entry.address - s_imageBase;
						
						if (text && ((entry.address & 0xFF000000) != 0x80000000))
						{
							UIStringPattern* pattern;
							
							get_string_from_array(i/items, entry.address, &pattern);
							if (pattern && s_isVerbose)
							{
								if (pattern->complete == false)
								{
									wchar_t string[256];
									memset(string, 0, 256*sizeof(wchar_t));
									wprintf(L"           string   = \"");
									for (uint32_t j=0; j < pattern->size; j++)
									{
										if (pattern->string[j] == 0)
											string[j] = L'_';
										else
											string[j] = pattern->string[j];
									}
									wprintf(L"%S\" (incomplete)\n\n", string);
								}
								else
								{
									wprintf(L"           string   = \"%S\"\n\n", pattern->string);
								}
							}
						}

						if (s_toolFlags & kToolFlag_IDC && entry.objects != 0)
						{
							fprintf(s_idcFile, "\t\t\tMakeUnknown(0x%.8X, 0x%X, 0);\n", entry.address, entry.objects * 4);
							fprintf(s_idcFile, "\t\t\tMakeDword(0x%.8X);\n", entry.address);
							fprintf(s_idcFile, "\t\t\tMakeArray(0x%.8X, 0x%X);\n", entry.address, entry.objects);
						}
						
						for (uint32_t j=0; j < entry.objects; j++)
						{
							uint32_t objAddress = *((uint32_t*)adr_data);
							objAddress = Swap32(objAddress);
							
							data = buffer + objAddress - s_imageBase;
							
							uint32_t data_size = 0;
                            find_data_size(data, buf_size - (uint32_t)(data-buffer), &data_size);
                            VERBOSE("           [%2d] 0x%.8lX: %.8X (%4d | 0x%.4X)\n", j, entry.address + (j * sizeof(uint32_t)), objAddress, data_size, data_size);
                            
                            if (s_toolFlags & kToolFlag_IDC)
                            {
                                fprintf(s_idcFile, "\t\t\t\tMakeUnknown(0x%.8lX, 0x%lX, 0);\n", entry.address + (j * sizeof(uint32_t)), sizeof(uint32_t));
                                fprintf(s_idcFile, "\t\t\t\tMakeDword(0x%.8lX);\n", entry.address + (j * sizeof(uint32_t)));
                                fprintf(s_idcFile, "\t\t\t\tOpOffset(0x%.8lX, 0);\n", entry.address + (j * sizeof(uint32_t)));
                                fprintf(s_idcFile, "\t\t\t\tMakeUnknown(0x%.8X, 0x%X, 0);\n", objAddress, data_size);
                                fprintf(s_idcFile, "\t\t\t\tMakeWord(0x%.8X);\n", objAddress);
                                fprintf(s_idcFile, "\t\t\t\tMakeArray(0x%.8X, 0x%X);\n", objAddress, data_size/2);
                            }
							
							if (s_toolFlags & kToolFlag_DumpBMP || s_toolFlags & kToolFlag_DumpPNG)
							{
								char filename[256] = {0};
								
								strcpy(filename, s_dumpFolder);
								strcat(filename, "/Type1/");
								if (stat(filename, &st) == -1)
									mkdir(filename, 0700);
								
								strcat(filename, s_languageString[i/items]);
								strcat(filename, "/");
								if (stat(filename, &st) == -1)
									mkdir(filename, 0700);
								
								sprintf(filename,"%s/Type1/%s/%.8X", s_dumpFolder, s_languageString[i/items], objAddress);
								
								if (s_toolFlags & kToolFlag_DumpBMP)
									strcat(filename, ".bmp");
								else if (s_toolFlags & kToolFlag_DumpPNG)
									strcat(filename, ".png");
								
								if (file_exists(filename) == false)
								{
									dump_image_to_file(data, buf_size - (uint32_t)(data-buffer), filename);
								}
							}
							
							if (s_bytesToDisp)
							{
								display_bytes(data, s_bytesToDisp, 16);
								VERBOSE("\n");
							}
							
							adr_data += sizeof(uint32_t);
						}
						
						if (entry.objects && s_bytesToDisp==0)
						{
							VERBOSE("\n");
						}
					}
					
					descEntry += sizeof(UIDescType1Entry);
				}
			}
			else
			{
				VERBOSE("    0x%.8X: type 1 (reg)\n", args->descAddress);
				VERBOSE("\n");
			}
		}
		else if (args->type == kUIDescType_2)
		{
			if (((args->descAddress & 0xFF000000) != 0x80000000))
			{
				UIDescType2 header;
				
				memcpy(&header, descHeader, sizeof(UIDescType2));
				header.reg = Swap32(header.reg);
				header.address = Swap32(header.address);
				header.unknown1 = Swap16(header.unknown1);
				header.unknown2 = Swap16(header.unknown2);
				header.unknown3 = Swap16(header.unknown3);
				header.tableoff = Swap16(header.tableoff);
				
				VERBOSE("    0x%.8X: type 2\n", args->descAddress);
				VERBOSE("      register = %.8X %s\n", header.reg, ((header.reg & 0xFF000000) == 0x80000000)? "(reg)" : "" );
				VERBOSE("      address  = %.8X %s\n", header.address,((header.address & 0xFF000000) == 0x80000000)? "(reg)" : "" );
				VERBOSE("      unknown1 = %.4X\n", header.unknown1);
				VERBOSE("      unknown2 = %.4X\n", header.unknown2);
				VERBOSE("      unknown3 = %.4X\n", header.unknown3);
				VERBOSE("      tableoff = %.4X\n", header.tableoff);
				VERBOSE("\n");
				
				if (s_toolFlags & kToolFlag_IDC)
				{
					fprintf(s_idcFile, "\n\tMakeUnknown(0x%.8X, sizeof(\"UIDescType2\"), 0);\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeStructEx(0x%.8X, -1, \"UIDescType2\");\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeComm(0x%.8X, \"Image Description Type 2\");\n", args->descAddress);
				}
				
				if (header.address != 0)
				{
					uint8_t* adr_data = buffer + header.address - s_imageBase;
					
					for (uint32_t j=0; j < DEFAULT_CHARMAP_SIZE; j++)
					{
						uint32_t objAddress = *((uint32_t*)adr_data);
						objAddress = Swap32(objAddress);
						
						if (s_toolFlags & kToolFlag_IDC)
						{
							fprintf(s_idcFile, "\t\tMakeUnknown(0x%.8lX, 0x%lX, 0);\n", header.address + (j*sizeof(uint32_t)), sizeof(uint32_t));
							fprintf(s_idcFile, "\t\tMakeDword(0x%.8lX);\n", header.address + (j*sizeof(uint32_t)));
							fprintf(s_idcFile, "\t\tOpOffset(0x%.8lX, 0);\n", header.address + (j*sizeof(uint32_t)));
							if (j == 2 || j == 60) // '"' and '\'
								fprintf(s_idcFile, "\t\tMakeComm(0x%.8lX, \"'\\%s'\");\n", header.address + (j*sizeof(uint32_t)), s_asciiTable[j]);
							else
								fprintf(s_idcFile, "\t\tMakeComm(0x%.8lX, \"'%s'\");\n", header.address + (j*sizeof(uint32_t)), s_asciiTable[j]);
						}
						
						data = buffer + objAddress - s_imageBase;
						
						uint32_t data_size = 0;
						find_data_size(data, buf_size - (uint32_t)(data-buffer), &data_size);
                        VERBOSE("      0x%.8X: %.8X (%4d | 0x%.4X) = '%s'\n", header.address+(j*4), objAddress, data_size, data_size, s_asciiTable[j]);
                        
                        if (s_toolFlags & kToolFlag_IDC)
                        {
                            fprintf(s_idcFile, "\t\t\tMakeUnknown(0x%.8X, 0x%X, 0);\n", objAddress, data_size);
                            fprintf(s_idcFile, "\t\t\tMakeWord(0x%.8X);\n", objAddress);
                            fprintf(s_idcFile, "\t\t\tMakeArray(0x%.8X, 0x%X);\n", objAddress, data_size/2);
                        }
						
						if (s_toolFlags & kToolFlag_DumpBMP || s_toolFlags & kToolFlag_DumpPNG)
						{
							char filename[256] = {0};
							strcpy(filename, s_dumpFolder);
							strcat(filename, "/Type2/");
							
							if (stat(filename, &st) == -1)
								mkdir(filename, 0700);
							
							sprintf(filename,"%s/Type2/%.8X", s_dumpFolder, objAddress);
							
							if (s_toolFlags & kToolFlag_DumpBMP)
								strcat(filename, ".bmp");
							else if (s_toolFlags & kToolFlag_DumpPNG)
								strcat(filename, ".png");
							
							if (file_exists(filename) == false)
							{
								dump_image_to_file(data, buf_size - (uint32_t)(data-buffer), filename);
							}
						}
						
						if (s_bytesToDisp)
						{
							display_bytes(data, s_bytesToDisp, 6);
						}
						
						adr_data += 4;
					}
					VERBOSE("\n");
				}
			}
			else
			{
				VERBOSE("    0x%.8X: type 2 (reg)\n", args->descAddress);
				VERBOSE("\n");
			}
		}
		else if (args->type == kUIDescType_3)
		{
			if (((args->descAddress & 0xFF000000) != 0x80000000))
			{
				UIDescType3 header;
				
				memcpy(&header, descHeader, sizeof(UIDescType3));
				header.x_offset = Swap16(header.x_offset);
				header.y_offset = Swap16(header.y_offset);
				header.address = Swap32(header.address);
				
				VERBOSE("    0x%.8X: type 3\n", args->descAddress);
				VERBOSE("      x_offset = %.4X\n", header.x_offset);
				VERBOSE("      y_offset = %.4X\n", header.y_offset);
				VERBOSE("      address  = %.8X %s\n", header.address, ((header.address & 0xFF000000) == 0x80000000)? "(reg)" : "" );
				VERBOSE("\n");
				
				if (s_toolFlags & kToolFlag_IDC)
				{
					fprintf(s_idcFile, "\n\tMakeUnknown(0x%.8X, sizeof(\"UIDescType3\"), 0);\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeStructEx(0x%.8X, -1, \"UIDescType3\");\n", args->descAddress);
					fprintf(s_idcFile, "\tMakeComm(0x%.8X, \"Image Description Type 3\");\n", args->descAddress);
				}
				
				if (header.address != 0)
				{
					data = buffer + header.address - s_imageBase;
					
					uint32_t data_size = 0;
					find_data_size(data, buf_size - (uint32_t)(data-buffer), &data_size);
                    VERBOSE("      Size: %4d | 0x%.4X\n", data_size, data_size);
                    
                    if (s_toolFlags & kToolFlag_IDC)
                    {
                        //if (header.address && (header.address & 0xFF000000) != 0x80000000)
                        //	fprintf(s_idcFile, "\t\tadd_dref(0x%.8X, 0x%.8X, 0);\n", args->descAddress+4, header.address);
                        fprintf(s_idcFile, "\t\tMakeUnknown(0x%.8X, 0x%X, 0);\n", header.address, data_size);
                        fprintf(s_idcFile, "\t\tMakeWord(0x%.8X);\n", header.address);
                        fprintf(s_idcFile, "\t\tMakeArray(0x%.8X, 0x%X);\n", header.address, data_size/2);
                    }

					if (s_toolFlags & kToolFlag_DumpBMP || s_toolFlags & kToolFlag_DumpPNG)
					{
						char filename[256] = {0};
						strcpy(filename, s_dumpFolder);
						strcat(filename, "/Type3/");
						
						if (stat(filename, &st) == -1)
							mkdir(filename, 0700);
						
						sprintf(filename,"%s/Type3/%.8X", s_dumpFolder, header.address);
						
						if (s_toolFlags & kToolFlag_DumpBMP)
							strcat(filename, ".bmp");
						else if (s_toolFlags & kToolFlag_DumpPNG)
							strcat(filename, ".png");
						
						if (file_exists(filename) == false)
						{
							dump_image_to_file(data, buf_size - (uint32_t)(data-buffer), filename);
						}
					}
					
					if (s_bytesToDisp)
					{
						display_bytes(data, s_bytesToDisp, 6);
					}
					
                    VERBOSE("\n");
				}
			}
			else
			{
				VERBOSE("    0x%.8X: type 3 (reg)\n", args->descAddress);
				VERBOSE("\n");
			}
		}
		else
		{
			VERBOSE("    0x%.8X: UNKNOWN TYPE!\n", args->descAddress);
		}
		
		args = args->next;
	}
	
	res = true;
	
bail:
	
	return res;
}

void find_ui_resources(const char* firmware)
{
	bool res = false;
	
	uint8_t* buffer = NULL;
	uint32_t buf_size = 0;
	
	FILE* fw_file = NULL;
	uint8_t* fw_buf = NULL;
	uint32_t fw_size = 0;
	uint32_t lastCall = 0;
	uint32_t argCount = 0;

	UICallArgumentsEx args;
	memset(&args, 0, sizeof(UICallArgumentsEx));
	
	uint8_t call_pattern[8] = {0x9F, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x97, 0x1C};
	
	// Load firmware file
	VERBOSE("\nOpen firmware image file: %s\n", firmware);
	fw_file = fopen(firmware, "r");
		
	fseek(fw_file, 0L, SEEK_END);
	fw_size = (uint32_t)ftell(fw_file);
		
	fw_buf = (uint8_t*)malloc(fw_size);
	buffer = fw_buf;
	buf_size = fw_size;
	
	// read packed firmware
	fseek(fw_file, 0L, SEEK_SET);
	fread(fw_buf, 1, fw_size, fw_file);
		
	fclose(fw_file);
		
	VERBOSE("  File size: %8d | 0x%.8X\n", fw_size, fw_size);

	find_lang_address(fw_buf, fw_size);
	
	// load LUT from firmware
	if (s_toolFlags & kToolFlag_LutAddress)
	{
		if (s_lutAddress > fw_size-0x400)
		{
			printf("LUT ADDRESS ERROR\n");
			goto bail;
		}
		
		memcpy(s_lut, fw_buf + s_lutAddress, sizeof(s_lut));
	}
	
	// update call pattern
	memcpy(call_pattern+2, &s_funcAddress, sizeof(uint32_t));

	// search for gui_CopyImageDesc call
	VERBOSE("\nFind arguments for gui_CopyImageDesc calls:\n");
	
	if (s_toolFlags & kToolFlag_IDC)
		fprintf(s_idcFile, "\t// Image Description DREFs\n");
	
	for (uint32_t i=0; i < (fw_size - sizeof(call_pattern)); i+=2)
	{
		if (memcmp(fw_buf+i, call_pattern, sizeof(call_pattern)) == 0)
		{
			res = false;
			
			// gui_CopyImageDesc call found, find first argument offset
			for (uint32_t j = i; j > lastCall; j-=2)
			{
				uint16_t cmd = *((uint16_t*)(fw_buf+j));
				cmd = Swap16(cmd);
				if ((cmd & ~kFROpcode_LDI8_ValMask) == (kFROpcode_LDI8 | 0x5))
				{
					args.callAddress = j + s_imageBase;
					res = parse_func_arguments(fw_buf+j, i-j, &args);
					if (res == true)
					{
						lastCall = i + 8;
						args.callAddress = i + s_imageBase;
						
						VERBOSE("  0x%.8X: gui_CopyImageDesc( R4, %2d, %2d, 0x%.8X, %2d, %2d, %2d, 0x%.4X, 0x%.4X, 0x%.2X, 0x%.8X, 0x%.8X, %2d )\n",
								args.callAddress,
								args.type,
								args.target,
								args.descAddress,
								args.always0,
								args.index1,
								args.index2,
								args.x_offset,
								args.y_offset,
								args.unknown2,
								args.language1,
								args.language2,
								args.funcAddress);
						
						add_arg_to_array(&args);
						argCount++;
					}
					else
					{
						VERBOSE("  0x%.8X: ARGUMENTS SEARCH FAILED !!!", i + s_imageBase);
					}
				}
				
				if (res == true)
					break;
			}
		}
	}
	
	if (s_toolFlags & kToolFlag_RecoverChars)
	{
		VERBOSE("\nFind all strings:\n");
		get_all_strings(fw_buf, fw_size);
		
		recover_charmap();
		
		recover_incomplete_strings();
	}
	
	VERBOSE("\nDump UI resources:\n");

	if (s_toolFlags & kToolFlag_IDC)
		fprintf(s_idcFile, "\n\t// Image Descriptions\n");
	
	dump_ui_resources(fw_buf, fw_size);
	
	if (s_toolFlags & kToolFlag_Container)
	{
		VERBOSE("\nCreate UI container:\n");
		
		M240UIDescContainer* container = new M240UIDescContainer;
		container->init();
		
		UICallArgumentsEx* args = s_argsArray.first;
		uint32_t c=0;
		while (args)
		{
			container->add(args, (args->index2 == 0)? true : false);
			if (args->index2 == 0) c++;
			args = args->next;
		}

		container->writeToFile(s_contFileName);
		
		delete container;
	}

#if 0
	for (uint32_t c=0; c < s_charMapSize; c++)
	{
		printf("%.8X: %C\n", s_charMap[c].address, s_charMap[c].chr);
	}

	dump_all_strings();
#endif
    
	VERBOSE("\nDone\n");

bail:
	
	delete_string_arrays();
	
	delete_args_array();
	
	if (fw_buf)
		free(fw_buf);
}

// -------------- MARK: Implementation: Initialization

void generate_patterns()
{
	uint32_t snum = MENU_STRING_COUNT;
	const wchar_t* string = NULL;
	uint32_t slen = 0;

	for (uint32_t lang=0; lang < kMaxLanguageCount; lang++)
	{
		for (uint32_t str=0; str<snum; str++)
		{
			uint32_t index = 1;
			string = s_menuText[lang][str];
			slen = (uint32_t)wcslen(string);

			s_menuTextPatterns[lang][str].size = slen;
			s_menuTextPatterns[lang][str].value = 0;
			
			for (uint32_t i=0; i<slen; i++)
			{
				if (s_menuTextPatterns[lang][str].pattern[i] != 0)
					continue;
				
				s_menuTextPatterns[lang][str].pattern[i] = index;
				
				for (uint32_t j=i+1; j<slen; j++)
				{
					if (string[i] == string[j])
					{
						s_menuTextPatterns[lang][str].pattern[j] = index;
						s_menuTextPatterns[lang][str].value++;
					}
				}
				
				index++;
			}
		}
	}
}

void init()
{
	s_funcAddress = Swap32(s_funcAddress);
	
	generate_patterns();

	if (s_toolFlags & kToolFlag_LutFile)
	{
		FILE* lut = fopen(s_lutFileName, "r");
		fread(s_lut, sizeof(UIColorAYUV), 256, lut);
		fclose(lut);
	}
	
	if (s_toolFlags & kToolFlag_IDC)
		s_idcFile = fopen(s_idcFileName, "w");
	
	if (s_toolFlags & kToolFlag_IDC)
	{
		fprintf(s_idcFile, "#include <idc.idc>\n");
		fprintf(s_idcFile, "\n");
		fprintf(s_idcFile, "static main(void)\n");
		fprintf(s_idcFile, "{\n");
	}
}

static void usage(const char* progName)
{
	printf("Leica M (typ 240) UI Tool v%s\n", VERSION);
	printf("Usage: %s [-a address] [-i imagebase] [-s script] [-d dump] [-f folder] [-l LUT] [-rbv] FIRMWARE.BIN\n\n", progName);
	printf("This tool will help you to find UI resources in firmware.\n");
	printf("Use following arguments:\n");
	printf("    -a      Specify address of the %s function (ex. 0x2F95E0)\n", DEFAULT_FUNC_NAME);
	printf("    -i      Specify firmware imagebase\n");
	printf("    -s      Specify IDC file name\n");
	printf("    -c      Specify container file name\n");	
	printf("    -d      Specify dump image format\n");
	printf("       png  - PNG format\n");
	printf("       bmp  - BMP (ARGB) format\n");
	printf("    -f      Specify folder for dumped images\n");
	printf("    -l      Specify LUT for images (filename of address)\n");
	printf("    -b      Specify number of bytes to display in verbose mode\n");
	printf("    -r      Try to recover string characters\n");
	printf("    -v      Be verbose\n");
	printf("\n");
	printf("Default imagebase: 0x%.X\n", DEFAULT_IMAGEBASE);
	printf("Default IDC filename: %s\n", DEFAULT_IDC_FILENAME);
	printf("Default folder: %s\n", DEFAULT_DUMP_FOLDER);
	printf("\n");
	printf("Examples:\n");
	printf("  Find UI resources (verbose mode, show 16 bytes for data):\n");
	printf("   $ %s -a 0x2F95E0 -v -b 0x13 FIRMWARE.FW\n", progName);
	printf("\n");
	printf("  Generate IDC script for UI objects found in firmware (add imagebase 0x100000):\n");
	printf("   $ %s -a 0x2F95E0 -i 0x100000 -s my_script.idc FIRMWARE.FW\n", progName);
	printf("\n");
	printf("  Create UI container:\n");
	printf("   $ %s -a 0x2F95E0 -c container.dat FIRMWARE.FW\n", progName);
	printf("\n");
	printf("  Dump UI resources to PNG (use LUT address):\n");
	printf("   $ %s -a 0x2F95E0 -d png -l 0x491FF0 FIRMWARE.FW\n", progName);
	printf("\n");
	printf("  Dump UI resources to BMP (use LUT file and custom folder):\n");
	printf("   $ %s -a 0x2F95E0 -d bmp -l lut.bin FIRMWARE.FW\n", progName);
	printf("\n");
	printf("  Find UI resources and try to recover string charasters (verbose mode):\n");
	printf("   $ %s -a 0x2F95E0 -r -v FIRMWARE.FW\n", progName);
	printf("\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char* const argv[])
{
	int ch;
	const char* firmwareFileName = NULL;
	const char* progName = argv[0];
	char* lutParam = NULL;
	
	setlocale(LC_CTYPE, "en_US");

	// Set defaults
	s_isVerbose = false;
	s_toolFlags = kToolFlag_Clear;
	
	printf("\n");
	
	// Get options
	while ((ch = getopt(argc, argv, "a:i:s:c:d:f:l:b:rv")) != -1)
	{
		switch (ch)
		{
			case 'a': // function address
				s_funcAddress = (uint32_t)strtol(optarg, NULL, 0);
				s_toolFlags |= kToolFlag_UseFuncAddr;
				break;
			case 'i': // imagebase
				s_imageBase = (uint32_t)strtol(optarg, NULL, 0);
				s_toolFlags |= kToolFlag_UseImageBase;
				break;
			case 's': // create IDC
				s_idcFileName = optarg;
				s_toolFlags |= kToolFlag_IDC;
				break;
			case 'c': // create UI container
				s_contFileName = optarg;
				s_toolFlags |= kToolFlag_Container;
				break;
			case 'd': // dump img to disc
				if (strcmp(optarg, "png") == 0) {
					s_toolFlags |= kToolFlag_DumpPNG;
				}
				else if (strcmp(optarg, "bmp") == 0) {
					s_toolFlags |= kToolFlag_DumpBMP;
				}
				else {
					usage(progName);
				}
				break;
			case 'f': // dump folder
				s_dumpFolder = optarg;
				break;
			case 'l': // LUT
				lutParam = optarg;
				if (file_exists(lutParam) == true) {
					s_lutFileName = lutParam;
					s_toolFlags |= kToolFlag_LutFile;
				}
				else {
					s_lutAddress = (uint32_t)strtol(lutParam, NULL, 0);
					s_toolFlags |= kToolFlag_LutAddress;
				}
				break;
			case 'r': // recover string characters
				s_toolFlags |= kToolFlag_RecoverChars;
				break;
			case 'b': // bytes to display
				s_bytesToDisp = (uint32_t)strtol(optarg, NULL, 0);
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

	if (s_toolFlags & kToolFlag_UseFuncAddr)
	{
		if (s_funcAddress == 0)
		{
			printf("  Invalid function address.\n");
			return 0;
		}
	}
	else
	{
		printf("  Please provide function address (-a).\n");
		return 0;
	}
	
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
		s_imageBase = DEFAULT_IMAGEBASE;
	}
	
	if (s_toolFlags & kToolFlag_IDC)
	{
		if (s_idcFileName == NULL)
			s_idcFileName = (char*)DEFAULT_IDC_FILENAME;
		
		if (s_idcFileName[0] == 0)
		{
			printf("  Invalid IDC file name.\n");
			return 0;
		}
	}
	
	if (s_toolFlags & kToolFlag_Container)
	{
		if (s_contFileName == NULL)
			s_contFileName = (char*)DEFAULT_CONT_FILENAME;
		
		if (s_contFileName[0] == 0)
		{
			printf("  Invalid container file name.\n");
			return 0;
		}
	}
	
	if (s_toolFlags & kToolFlag_DumpPNG || s_toolFlags & kToolFlag_DumpBMP)
	{
		if (s_toolFlags & kToolFlag_DumpPNG && s_toolFlags & kToolFlag_DumpBMP)
		{
			printf("  please provide just one image format.\n");
			return 0;
		}
		
		if (s_dumpFolder == NULL)
			s_dumpFolder = (char*)DEFAULT_DUMP_FOLDER;

		if (s_dumpFolder[0]==0)
		{
			printf("  Invalid folder name.\n");
			return 0;
		}
		
		if ((s_toolFlags & kToolFlag_LutFile) == 0 &&
			(s_toolFlags & kToolFlag_LutAddress) == 0)
		{
			printf("  LUT not found, please provide address or file (-l).\n");
			return 0;
		}
	}
	
	if (s_bytesToDisp == 0xFFFFFFFF)
		s_bytesToDisp = 0;
	
	// Display parameters
	printf("Running with options:\n");
	if (s_toolFlags & kToolFlag_UseFuncAddr)
		printf("  + use %s function address: %d (0x%.8X)\n", DEFAULT_FUNC_NAME, s_funcAddress, s_funcAddress);
	if (s_toolFlags & kToolFlag_UseImageBase)
		printf("  + use firmware image base: %d (0x%.8X)\n", s_imageBase, s_imageBase);
	if (s_toolFlags & kToolFlag_IDC)
		printf("  + produce IDC script: %s\n", s_idcFileName);
	if (s_toolFlags & kToolFlag_Container)
		printf("  + create UI container: %s\n", s_contFileName);
	if (s_toolFlags & kToolFlag_DumpPNG)
		printf("  + dump images to disk in PNG format\n");
	if (s_toolFlags & kToolFlag_DumpBMP)
		printf("  + dump images to disk in BMP (ARGB) format\n");
	if (s_dumpFolder != NULL)
		printf("  + use following folder for images: %s\n", s_dumpFolder);
	if (s_toolFlags & kToolFlag_LutFile)
		printf("  + use LUT from file: \"%s\"\n", s_lutFileName);
	if (s_toolFlags & kToolFlag_LutAddress)
		printf("  + use LUT from firmware address: %d (0x%.8X)\n", s_lutAddress, s_lutAddress);
	if (s_toolFlags & kToolFlag_RecoverChars)
		printf("  + try to recover string characters\n");
	if (s_bytesToDisp != 0)
		printf("  + display bytes in verbose mode:  %d (0x%.8X)\n", s_bytesToDisp, s_bytesToDisp);
	if (s_toolFlags & kToolFlag_Verbose)
		printf("  + verbose enabled\n");
	
	// Perform action
	init();
	
	find_ui_resources(firmwareFileName);
	
	// Clean up
	if (s_toolFlags &= kToolFlag_IDC)
	{
		fprintf(s_idcFile, "\n\tRefresh();\n");
		fprintf(s_idcFile, "}\n");
	}
	
	if (s_idcFile)
		fclose(s_idcFile);
	
    return 0;
}

