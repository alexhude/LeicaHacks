
#include "fr.hpp"
#include <math.h>

//#define FR_DEFAULT_DATA_OFFSET
#define FR_RENAME_STKVARS

static insn_t g_cmd_stack[10] = {0};
static int8_t g_cmd_sp = 0;

#define CMD_PUSH	g_cmd_stack[g_cmd_sp++] = cmd;
#define CMD_POP		cmd = g_cmd_stack[--g_cmd_sp];

inline bool is_stop (void)
{
	uint32 feature = cmd.get_canon_feature();
	return (feature & CF_STOP) != 0;
}

// Analyze an instruction
static ea_t next_insn(ea_t ea)
{
	if ( decode_insn(ea) == 0 )
		return 0;
	ea += cmd.size;
	return ea;
}

static bool get_func_desc(func_t* pfn, ushort* reg_size, uint32* var_size)
{
	CMD_PUSH
	
	bool res = false;
	
	if (reg_size)
		*reg_size = 0;
	if (var_size)
		*var_size = 0;
	
	ea_t ea = pfn->startEA;
	ea = next_insn(ea);
	while (cmd.itype == fr_stm0 ||
		   cmd.itype == fr_stm1 ||
		   (cmd.itype == fr_st &&
			cmd.Op1.type == o_reg &&
			cmd.Op2.type == o_phrase &&
			cmd.Op2.reg == rR15 &&
			cmd.Op2.specflag2 == fIGRM))
	{
		ea = next_insn(ea);
	}
	
	if ( cmd.itype == fr_enter )
	{
		// R14 is automatically pushed by fr_enter
		if (reg_size)
			*reg_size += 4;
		if (var_size)
			*var_size = uint32(cmd.Op1.value - 4);
		res = true;
	}
	
	CMD_POP
	return res;
}

static bool is_used_for_call(uint16_t reg, cref_t *ctype)
{
	CMD_PUSH;
	
	bool found = false;
	
	ea_t ea;
	ea = next_insn(cmd.ea+cmd.size);
	
	if ((cmd.itype == fr_call || cmd.itype == fr_jmp) &&
		cmd.Op1.type == o_phrase &&
		cmd.Op1.specflag2 == fIGR &&
		cmd.Op1.reg == reg)
	{
		if (cmd.itype == fr_call)
			*ctype = fl_CF; // create function
		else
			*ctype = fl_JF; // just convert to code
			
		found = true;
	}
	
	CMD_POP;
	
	return found;
}

static bool is_used_for_switch(ea_t dst_addr, uint16_t reg, int *count)
{
	bool found = false;
	
	if (get_func(cmd.ea) == NULL)
		return found;
		
	int cnt = 0;
	ea_t ea = dst_addr;
	ea_t lbl = get_32bit(ea);
	
	while (get_func(lbl) == get_func(cmd.ea) &&
		   (isUnknown(getFlags(lbl)) || isCode(getFlags(lbl))))
	{
		ea += 4;
		lbl = get_32bit(ea);
		
		cnt++;
	}

	if (cnt != 0)
	{
	#if defined(__DEBUG__)
		msg("POTENTIAL SWITCH: %.8X (%d)\n", cmd.ea, cnt);
	#endif

		CMD_PUSH;
		
		// find LD @(R13, Ri), Rj
		next_insn(cmd.ea+cmd.size);
		if (cmd.itype == fr_ld &&
			cmd.Op1.type == o_phrase &&
			cmd.Op1.specflag2 == fR13RI &&
			((reg != rR13)? (reg == cmd.Op1.reg) : true))
		{
		#if defined(__DEBUG__)
			msg("  SWITCH CONFIRMED: %.8X\n", cmd.ea);
		#endif
			found = true;
		}
		else
		{
			next_insn(cmd.ea+cmd.size);
			if (cmd.itype == fr_ld &&
				cmd.Op1.type == o_phrase &&
				cmd.Op1.specflag2 == fR13RI &&
				((reg != rR13)? (reg == cmd.Op1.reg) : true))
			{
			#if defined(__DEBUG__)
				msg("  SWITCH CONFIRMED: %.8X\n", cmd.ea);
			#endif
				found = true;
			}
		}

		CMD_POP;

		if (found && count)
			*count = cnt;
	}
	
	return found;
}

static void handle_command()
{
	switch (cmd.itype)
	{
		case fr_ld:
		case fr_lduh:
		case fr_ldub:
		{
			if (inf.af & AF_LVAR)
			{
				if (cmd.Op1.type == o_displ &&
					((cmd.Op1.specflag1 & OP_DISPL_IMM_R14) ||
					 (cmd.Op1.specflag1 & OP_DISPL_IMM_R15)))
				{
					if (may_create_stkvars() == false)
						break;

					func_t *pfn = get_func(cmd.ea);
					
					// bail if function not defined for this command
					if ( pfn == NULL )
						break;

					// bail if function is not FP based
					if ( !(pfn->flags & FUNC_FRAME) )
						break;

					// bail if SP trace is disabled for @(R15, imm)
					if ((cmd.Op1.specflag1 & OP_DISPL_IMM_R15) && (inf.af & AF_TRACE) == 0)
						break;
					
					adiff_t offset = (adiff_t)cmd.Op1.value;
					if ( ua_stkvar2(cmd.Op1, offset, STKVAR_VALID_SIZE) == true)
					{
						// convert operand to stack variable
						op_stkvar(cmd.ea, 0);
						
					#ifdef FR_RENAME_STKVARS
						char name[32] = {0};
						
						if (cmd.Op1.specflag1 & OP_DISPL_IMM_R14)
						{
							// create proper name for stack variable
							if (offset < 0)
								qsnprintf(name, 32, "var_%.3X", abs(offset));
							else
								qsnprintf(name, 32, "arg_%.3X", offset);
							
							// update stack variable
							if (cmd.itype == fr_ld)
								add_stkvar2(pfn, name, offset, dwrdflag(), NULL, 4);
							else if (cmd.itype == fr_lduh)
								add_stkvar2(pfn, name, offset, wordflag(), NULL, 2);
							else if (cmd.itype == fr_ldub)
								add_stkvar2(pfn, name, offset, byteflag(), NULL, 1);
						}
						else if (cmd.Op1.specflag1 & OP_DISPL_IMM_R15)
						{
							int delta = 0;
							uint32 locvar;
							if (get_func_desc(pfn, NULL, &locvar))
								delta = -1*locvar;
							
							// create proper name for stack variable
							qsnprintf(name, 32, "par_%.3X", abs(delta + offset));
							
							// update stack variable
							if (cmd.itype == fr_ld)
								add_stkvar2(pfn, name, delta + offset, dwrdflag(), NULL, 4);
							else if (cmd.itype == fr_lduh)
								add_stkvar2(pfn, name, delta + offset, wordflag(), NULL, 2);
							else if (cmd.itype == fr_ldub)
								add_stkvar2(pfn, name, delta + offset, byteflag(), NULL, 1);
						}
					#endif
					}
				}
			}
		}
		break;
		case fr_st:
		case fr_sth:
		case fr_stb:
		{
			if (inf.af & AF_LVAR)
			{
				if (cmd.Op2.type == o_displ &&
					((cmd.Op2.specflag1 & OP_DISPL_IMM_R14) ||
					 (cmd.Op2.specflag1 & OP_DISPL_IMM_R15)))
				{
					if (may_create_stkvars() == false)
						break;
					
					func_t *pfn = get_func(cmd.ea);
					
					// bail if function not defined for this command
					if ( pfn == NULL )
						break;
					
					// bail if function is not FP based
					if ( !(pfn->flags & FUNC_FRAME) )
						break;
					
					// bail if SP trace is disabled for @(R15, imm)
					if ((cmd.Op2.specflag1 & OP_DISPL_IMM_R15) && (inf.af & AF_TRACE) == 0)
						break;
				
					adiff_t offset = (adiff_t)cmd.Op2.value;
					if ( ua_stkvar2(cmd.Op2, offset, STKVAR_VALID_SIZE) == true)
					{
						// convert operand to stack variable
						op_stkvar(cmd.ea, 1);

					#ifdef FR_RENAME_STKVARS
						char name[32] = {0};
						
						if (cmd.Op2.specflag1 & OP_DISPL_IMM_R14)
						{
							// create proper name for stack variable
							if (offset < 0)
								qsnprintf(name, 32, "var_%.3X", abs(offset));
							else
								qsnprintf(name, 32, "arg_%.3X", offset);
							
							// update stack variable
							if (cmd.itype == fr_st)
								add_stkvar2(pfn, name, offset, dwrdflag(), NULL, 4);
							else if (cmd.itype == fr_sth)
								add_stkvar2(pfn, name, offset, wordflag(), NULL, 2);
							else if (cmd.itype == fr_stb)
								add_stkvar2(pfn, name, offset, byteflag(), NULL, 1);
						}
						else if (cmd.Op2.specflag1 & OP_DISPL_IMM_R15)
						{
							int delta = 0;
							uint32 locvar;
							if (get_func_desc(pfn, NULL, &locvar))
								delta = -1*locvar;
							
							// create proper name for stack variable
							qsnprintf(name, 32, "par_%.3X", abs(delta + offset));
							
							// update stack variable
							if (cmd.itype == fr_st)
								add_stkvar2(pfn, name, delta + offset, dwrdflag(), NULL, 4);
							else if (cmd.itype == fr_sth)
								add_stkvar2(pfn, name, delta + offset, wordflag(), NULL, 2);
							else if (cmd.itype == fr_stb)
								add_stkvar2(pfn, name, delta + offset, byteflag(), NULL, 1);
						}
					#endif
					}
				}
			}
		}
		break;
		case fr_ldi_8:
		case fr_ldi_20:
		case fr_ldi_32:
		{
			doImmd(cmd.ea); // command has immediate operand
			
			ea_t dst_addr = toEA(cmd.cs, cmd.Op1.value);
			if (isEnabled(dst_addr) == true)
			{
				cref_t ctype;
				if (is_used_for_call(cmd.Op2.reg, &ctype))
				{
					// imm value is used for CALL or JMP (IGR), create ref
					
					if (isDefArg(uFlag, 0) == false)
					{
						// covert operand to offset if undefined
						if (cmd.itype == fr_ldi_8)
							op_offset(cmd.ea, 0, REF_OFF8, dst_addr);
						else
							op_offset(cmd.ea, 0, REF_OFF32, dst_addr);
					}

					if (op_adds_xrefs(uFlag, 0) == true)
					{
						// create xref and analyse code from offset
						ua_add_cref(cmd.Op1.offb, dst_addr, ctype);
					}
					
				}
				else if (isCode(getFlags(dst_addr)) == true)
				{
					// if imm value points to code, create ref
						
					if (isDefArg(uFlag, 0) == false)
					{
						// covert operand to offset if undefined
						if (cmd.itype == fr_ldi_8)
							op_offset(cmd.ea, 0, REF_OFF8, dst_addr);
						else
							op_offset(cmd.ea, 0, REF_OFF32, dst_addr);
					}
					
					if (op_adds_xrefs(uFlag, 0) == true)
					{
						// create xref and analyse code from offset
						//ua_add_cref(cmd.Op1.offb, dst_addr, fl_JN);
						ua_add_dref(cmd.Op1.offb, dst_addr, dr_O);
					}
				}
				else if (isData(getFlags(dst_addr)) == true ||
						 isASCII(getFlags(dst_addr)) == true)
				{
					// if imm value points to data/string, create ref
					
					if (isDefArg(uFlag, 0) == false)
					{
						// covert operand to offset if undefined
						if (cmd.itype == fr_ldi_8)
							op_offset(cmd.ea, 0, REF_OFF8, dst_addr);
						else
							op_offset(cmd.ea, 0, REF_OFF32, dst_addr);
					}
					
					if (op_adds_xrefs(uFlag, 0) == true)
					{
						// create xref to data/string
						if (isData(getFlags(dst_addr)) == true)
							ua_add_off_drefs2(cmd.Op1, dr_O, 0);
						else
							ua_add_off_drefs2(cmd.Op1, dr_T, 0);
					}
				}
				else if (cmd.itype == fr_ldi_32)
				{
					if (isTail(getFlags(dst_addr)))
						break;
					
					// LDI:32  #switch_0xXXXXXX, R13
					// LSL     #2, R0
					// LD      @(R13, R0), R12
					// JMP     @12
					int count = 0;
					if (is_used_for_switch(dst_addr, cmd.Op2.reg, &count))
					{
						if (isDefArg(uFlag, 0) == false)
						{
							switch_info_ex_t si;
							
							si.clear();
							si.set_jtable_element_size(4);
							si.ncases  = count;
							si.jumps   = dst_addr;
							si.startea = cmd.ea;
							si.set_expr(cmd.Op2.reg, dt_dword);
							
							set_switch_info_ex(cmd.ea, &si);
							create_switch_table(cmd.ea, &si);

							// covert LDI:32 operand to offset if undefined
							op_offset(cmd.ea, 0, REF_OFF32, dst_addr);
							
							// fix table name
							char name[64] = {0};
							qsnprintf(name, 64, "switch_0x%X", cmd.ea);
							set_name(dst_addr, name);

							// fix table offsets
							for (int c = 0; c < count; c++)
							{
								op_offset(dst_addr + (c*4), 0, REF_OFF32);
							}
						}
						
						break;
					}

					// Create ascii string if data xref exists
					if (inf.af & AF_ASCII)
					{
						bool strFound = false;
						
						// try to make string
						if ((isUnknown(getFlags(dst_addr)) && make_ascii_string(dst_addr, get_max_ascii_length(dst_addr, ASCSTR_C), ASCSTR_C) == true) ||
							isASCII(getFlags(dst_addr)) == true)
						{
							strFound = true;
						}
						
						if (strFound)
						{
							if (isDefArg(uFlag, 0) == false)
							{
								// covert operand to offset if undefined
								op_offset(cmd.ea, 0, REF_OFF32, dst_addr);
							}
							
							if (op_adds_xrefs(uFlag, 0) == true)
							{
								// create xref to string
								ua_add_off_drefs2(cmd.Op1, dr_T, 0);
							}
							
							break;
						}
					}
					
					// Convert 32bit instruction operand to offset
					if (inf.af & AF_IMMOFF)
					{
						if (dst_addr != get_item_head(dst_addr))
							break;
						
						if (isDefArg(uFlag, 0) == false)
						{
							// covert operand to offset if undefined
							op_offset(cmd.ea, 0, REF_OFF32, dst_addr);
						}
	
						if (op_adds_xrefs(uFlag, 0) == true)
						{
							// create data
							ua_add_off_drefs2(cmd.Op1, dr_O, 0);
						#ifdef FR_DEFAULT_DATA_OFFSET
							ua_dodata2(cmd.Op1.offb, dst_addr, cmd.Op1.dtyp);
						#endif
						}
					}
				}
			}
		}
		break;
		case fr_jmp:
		{
			// do nothing, covered in fr_ldi_8/20/32
		}
		break;
		case fr_call:
		{
			if (cmd.Op1.type == o_near) // CALL label
			{
				if (isEnabled(cmd.Op1.addr))
				{
					//op_offset(cmd.ea, 0, REF_OFF32, toEA(cmd.cs, cmd.Op1.addr));
					ua_add_cref(cmd.Op1.offb, toEA(cmd.cs, cmd.Op1.addr), fl_CN);
				}
			}
		}
		break;
		case fr_stm0:
		case fr_stm1:
		case fr_ldm0:
		case fr_ldm1:
		{
			// Since FP is usually initialized during ENTER command which comes
			// after STM0/STM1, we shouldn't call add_auto_stkpnt2 here
			
			// Since STM0/STM1 commands do not change IDA SP, we should ignore
			// LDM0/LDM1 commands as well
		}
		break;
		case fr_addsp:
		case fr_enter:
		{
			if (inf.af & AF_TRACE)
			{
				if (cmd.itype == fr_addsp)
				{
					CMD_PUSH
					next_insn(cmd.ea + cmd.size);
					uint16 itype = cmd.itype;
					CMD_POP

					// ignore ADDSP at the end of the function
					if (itype == fr_ret || itype == fr_reti)
						break;
				}
				
				func_t *pfn = get_func(cmd.ea);
				if ( (pfn != NULL) && (pfn->flags & FUNC_FRAME) )
				{
					// Add SP change point for ADDSP and ENTER
					sval_t sp_delta = 0;
					
					if (cmd.itype == fr_addsp)
						sp_delta = cmd.Op1.value;
					else if (cmd.itype == fr_enter)
						sp_delta = -1 * cmd.Op1.value;
					
					add_auto_stkpnt2(pfn, cmd.ea + cmd.size, sp_delta);
				}
			}
		}
		break;
		default:
		{
			if (cmd.auxpref & INSN_BRANCH) 	// Bcc label
			{
				if (isEnabled(cmd.Op1.addr))
				{
					//op_offset(cmd.ea, 0, REF_OFF32, toEA(cmd.cs, cmd.Op1.addr));
					ua_add_cref(cmd.Op1.offb, toEA(cmd.cs, cmd.Op1.addr), fl_JN);
				}
			}
		}
		break;
	}
}

// Emulate an instruction.
int idaapi emu(void)
{
	bool cstop = false;
	bool cdelay = false;
	bool flow = false;

	cstop = is_stop();
	cdelay = cmd.auxpref & INSN_DELAY_SHOT;
	
	// handle 
	flow = (cstop == false) || (cdelay == true);
	if ( flow )
	{
		CMD_PUSH;
		if ( decode_prev_insn(cmd.ea) != BADADDR )
		{
			bool pstop = is_stop();
			bool pdelay = cmd.auxpref & INSN_DELAY_SHOT;
			flow = !(pstop && pdelay);
			
			// Fix corner case:
			// ROM:100004                 LDI:32  #loc_XXXXXX, R12
			// ROM:10000A                 JMP:D   @R12
			// ROM:10000C
			// ROM:10000C loc_10000C:                             ; CODE XREF: 0x100000
			// ROM:10000C                  MOV    R1, R2
			if (cstop == false && cdelay == false &&
				pstop == true && pdelay == true &&
				get_first_fcref_to(cmd.ea+cmd.size) != BADADDR)
			{
			#if defined(__DEBUG__)
				msg("emu: FLOW FIX %.8X\n", cmd.ea);
			#endif
				flow = true;
			}
		}
		CMD_POP;
	}

	//msg("emu: %.8X (%s)\n", cmd.ea, (flow)? "flow" : "stop");
	
	handle_command();
	
	if ( flow )
		ua_add_cref(0, cmd.ea + cmd.size, fl_F);
	
	return cmd.size;
}

// Create a function frame
bool idaapi create_func_frame(func_t *pfn)
{
	bool res = false;
	ushort reg_size = 0;
	uint32 arg_size = 0;
	uint32 var_size = 0;
	
	if (get_func_desc(pfn, &reg_size, &var_size))
	{
		pfn->flags |= FUNC_FRAME;
		res = add_frame(pfn, var_size, reg_size, arg_size);
	}
	
	return res;
}

int idaapi is_sp_based(const op_t &op)
{
	if (op.type == o_displ)
	{
		if (op.specflag1 & OP_DISPL_IMM_R14)
			return OP_SP_ADD | OP_FP_BASED;
		else if (op.specflag1 & OP_DISPL_IMM_R15)
			return OP_SP_ADD | OP_SP_BASED;
	}
		
	return 0;
}

int idaapi is_align_insn(ea_t ea)
{
	return get_byte(ea) == 0;
}
