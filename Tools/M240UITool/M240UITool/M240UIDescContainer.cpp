//
//  M240UIDescContainer.cpp
//  
//
//  Created by Alex Hude on 19/07/13.
//  Copyright (c) 2013 Alex Hude. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "M240UIDesc.h"
#include "M240UIDescContainer.h"

M240UIDescContainer::M240UIDescContainer()
{
	m_stateCount = 0;
	m_stateOffsets = NULL;
	
	m_containerData = NULL;
	m_containerDataSize = 0;
	m_containerMaxSize = 0;
}

M240UIDescContainer::~M240UIDescContainer()
{
	if (m_containerData)
		free(m_containerData);
	m_containerData = NULL;
	
	if (m_stateOffsets)
		free(m_stateOffsets);
	m_stateOffsets = NULL;
}

bool	M240UIDescContainer::recoverStates()
{
	uint32_t offset = 0;
	UIStateDesc* state = NULL;
	m_stateCount = 0;
	
	while (offset < m_containerDataSize)
	{
		state = (UIStateDesc*)(m_containerData + offset);

		uint32_t* data = (uint32_t*)malloc((m_stateCount+1) * sizeof(uint32_t*));
		
		if (m_stateCount != 0)
			memcpy(data, m_stateOffsets, m_stateCount * sizeof(uint32_t*));
		
		free(m_stateOffsets);
		m_stateOffsets = data;
		
		m_stateOffsets[m_stateCount] = offset;
		m_stateCount++;
		
		offset += sizeof(UIStateDesc) + (state->count * sizeof(UICallArguments));
	}
	
	return true;
	
bail:
	
	return false;
}

bool	M240UIDescContainer::init(uint32_t stateCount)
{
	if (m_containerData)
		free(m_containerData);
	
	m_containerMaxSize = 0x1000;
	m_containerData = (uint8_t*)malloc(m_containerMaxSize);
	m_containerDataSize = sizeof(UIStateDesc);

	m_stateOffsets = (uint32_t*)malloc(sizeof(uint32_t*));
	
	m_stateOffsets[0] = 0;
	UIStateDesc* state = (UIStateDesc*)(m_containerData + m_stateOffsets[0]);
	
	state->count = 0;
	state->offset = sizeof(UIStateDesc);
	m_stateCount++;
	
	return true;
}

bool	M240UIDescContainer::add(UICallArguments* args, bool newState)
{
	uint32_t addSize = 0;
	
	if (args == NULL)
		return false;
	
	addSize += sizeof(UICallArguments);
	
	if (newState)
		addSize += sizeof(UIStateDesc);

	if (m_containerDataSize + addSize > m_containerMaxSize)
	{
		uint8_t* data = (uint8_t*)malloc(m_containerMaxSize*2);
		
		memcpy(data, m_containerData, m_containerDataSize);
		free(m_containerData);
		m_containerData = data;

		m_containerMaxSize *= 2;
	}
	
	if (newState && ((UIStateDesc*)(m_containerData + m_stateOffsets[0]))->count != 0)
	{
		uint32_t* data = (uint32_t*)malloc((m_stateCount+1) * sizeof(uint32_t*));
		
		memcpy(data, m_stateOffsets, m_stateCount * sizeof(uint32_t*));

		free(m_stateOffsets);
		m_stateOffsets = data;
		m_stateCount++;

		m_stateOffsets[m_stateCount-1] = m_containerDataSize;
		UIStateDesc* state = (UIStateDesc*)(m_containerData + m_stateOffsets[m_stateCount-1]);
		
		state->count = 1;
		state->offset = m_containerDataSize + sizeof(UIStateDesc);
		m_containerDataSize += sizeof(UIStateDesc);
	}
	else
	{
		UIStateDesc* state = (UIStateDesc*)(m_containerData + m_stateOffsets[m_stateCount-1]);
		state->count++;
	}
	
	memcpy(m_containerData + m_containerDataSize, args, sizeof(UICallArguments));
	m_containerDataSize += sizeof(UICallArguments);
	
	return true;
}

uint32_t	M240UIDescContainer::getStateCount()
{
	return m_stateCount;
}

UIStateDesc*	M240UIDescContainer::getState(uint32_t index)
{
	if (m_stateOffsets == NULL)
		return NULL;

	if (index >= m_stateCount)
		return NULL;
	
	return (UIStateDesc*)(m_containerData + m_stateOffsets[index]);
}

UICallArguments*	M240UIDescContainer::getCallArgs(uint32_t state_idx, uint32_t arg_idx)
{
	if (m_stateOffsets == NULL)
		return NULL;
	
	if (state_idx >= m_stateCount)
		return NULL;
	
	UIStateDesc* state = (UIStateDesc*)(m_containerData + m_stateOffsets[state_idx]);
	
	if (arg_idx >= state->count)
		return NULL;
		
	return (UICallArguments*)(m_containerData + state->offset + (arg_idx * sizeof(UICallArguments)));
}

bool	M240UIDescContainer::readFromFile(char* filename)
{
	FILE* file = NULL;

	if (m_containerData)
		free(m_containerData);
	m_containerData = NULL;
	
	if (m_stateOffsets)
		free(m_stateOffsets);
	m_stateOffsets = NULL;
	
	file = fopen(filename, "r");
	if (file == NULL)
		goto bail;
	
	fseek(file, 0L, SEEK_END);
	
	m_containerDataSize = (uint32_t)ftell(file);
	m_containerMaxSize = m_containerDataSize;
	
	m_containerData = (uint8_t*)malloc(m_containerDataSize);
	
	fseek(file, 0L, SEEK_SET);
	fread(m_containerData, 1, m_containerDataSize, file);
	
	if (file)
		fclose(file);
	
	recoverStates();
	
	return true;
	
bail:
	
	return false;
}

bool M240UIDescContainer::writeToFile(char* filename)
{
	FILE* file = NULL;
	
	file = fopen(filename, "w");
	if (file == NULL)
		goto bail;
	
	fwrite(m_containerData, 1, m_containerDataSize, file);
	
	if (file)
		fclose(file);
	
	return true;
	
bail:
	
	return false;
}
