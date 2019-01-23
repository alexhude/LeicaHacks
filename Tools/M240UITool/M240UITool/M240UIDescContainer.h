//
//  M240UIDescContainer.h
//
//
//  Created by Alex Hude on 19/07/13.
//  Copyright (c) 2013 Alex Hude. All rights reserved.
//

#pragma once

struct UICallArguments;

struct UIContainerHeader
{
	uint32_t count;
};

struct UIStateDesc
{
	uint32_t		count;
	uint32_t		offset;
};

class M240UIDescContainer
{
	uint32_t		m_stateCount;
	uint32_t*		m_stateOffsets;

	uint8_t*		m_containerData;
	uint32_t		m_containerDataSize;
	uint32_t		m_containerMaxSize;
	
private:
	
	bool				recoverStates();
	
public:
	
	M240UIDescContainer();
	
	~M240UIDescContainer();
	
	bool				init(uint32_t stateCount=0);
	
	bool				add(UICallArguments* args, bool newState);
	
	uint32_t			getStateCount();
	UIStateDesc*		getState(uint32_t index);

	UICallArguments*	getCallArgs(uint32_t state_idx, uint32_t arg_idx);
	
	bool				readFromFile(char* filename);
	bool				writeToFile(char* filename);

};
