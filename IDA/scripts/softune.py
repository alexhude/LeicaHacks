import ctypes

from sys import exit
from string import find

from idc import *
from idaapi import *
from idautils import *

# ========================== LEICA M 240 ===========================

# Interrupt table
g_IntTableDesc = {
	# SKIP 0x000 - 0x2F4
	0x2F8:  [ 'int_shutdown',       'Shutdown' ],
	0x2FC:  [ 'int_realos_syscall', 'Reserved for system (REALOS)' ],
	0x300:  [ 'int_delay',          'ICR47: Delay interrupt' ],
	0x304:  [ 'int_RI46',           'ICR46: "INT: NOT IMPLEMENTED RI46 OCCURED!!!"' ],
	0x308:  [ 'int_dmac7',          'ICR45: Reserved' ],
	0x30C:  [ 'int_dmac6',          'ICR44: DMA Controller (DMAC) ch.3' ],
	0x310:  [ 'int_dmac5',          'ICR43: DMA Controller (DMAC) ch.2' ],
	0x314:  [ 'int_img_gen',        'ICR42: DMA Controller (DMAC) ch.1' ],
	0x318:  [ 'int_dmac3',          'ICR41: DMA Controller (DMAC) ch.0' ],
	0x31C:  [ 'int_dmac2',          'ICR40: Reserved' ],
	0x320:  [ 'int_dmac1',          'ICR39: USB function (End point 0 for DRQI, DRQO and each status)/USB HOST (each status)' ],
	0x324:  [ 'int_audio_per',      'ICR38: USB function (End point 1 to 3 for DRQ)' ],
	0x328:  [ 'int_RI37',           'ICR37: "INT: NOT IMPLEMENTED RI37 OCCURED!!!"' ],
	0x32C:  [ 'int_RI36',           'ICR36: Reserved' ],
	0x330:  [ 'int_RI35',           'ICR35: "INT: NOT IMPLEMENTED RI35 OCCURED!!!"' ],
	0x334:  [ 'int_audio',          'ICR34: Reserved' ],
	0x338:  [ 'int_udma2',          'ICR33: Base timer ch.3' ],
	0x33C:  [ 'int_udma1',          'ICR32: Base timer ch.2' ],
	0x340:  [ 'int_sdio',           'ICR31: Base timer ch.1' ],
	0x344:  [ 'int_RI30',           'ICR30: Base timer ch.0' ],
	0x348:  [ 'int_jpeg_enc',       'ICR29: 32-bit output compare ch.0 to ch.3' ],
	0x34C:  [ 'int_raw',            'ICR28: 32-bit input capture ch.0 to ch.3' ],
	0x350:  [ 'int_RI27',           'ICR27: "INT: NOT IMPLEMENTED RI27 OCCURED!!!"' ],
	0x354:  [ 'int_afn',            'ICR26: 10-bit A/D converter' ],
	0x358:  [ 'int_RI25',           'ICR25: "INT: NOT IMPLEMENTED RI25 OCCURED!!!"' ],
	0x35C:  [ 'int_hdmi',           'ICR24: 16-bit up/down counter ch.0' ],
	0x360:  [ 'int_img_dsp',        'ICR23: Reserved' ],
	0x364:  [ 'int_img',            'ICR22: 32-bit output compare ch.4 to ch.7' ],
	0x368:  [ 'int_RI21',           'ICR21: "INT: NOT IMPLEMENTED RI21 OCCURED!!!"' ],
	0x36C:  [ 'int_video',          'ICR20: Transmission interrupt/bus idle/status request from UART/CSIO/I2C ch.6' ],
	0x370:  [ 'int_exposure',       'ICR19: Reception interrupt request from UART/CSIO/I2C ch.6' ],
	0x374:  [ 'int_vd',             'ICR18: Reserved' ],
	0x378:  [ 'int_RI17',           'ICR17: "INT: NOT IMPLEMENTED RI17 OCCURED!!!"' ],
	0x37C:  [ 'int_equipment',      'ICR16: Reserved' ],
	0x380:  [ 'int_RI15',           'ICR15: "INT: NOT IMPLEMENTED RI15 OCCURED!!!"' ],
	0x384:  [ 'int_RI14',           'ICR14: "INT: NOT IMPLEMENTED RI14 OCCURED!!!"' ],
	0x388:  [ 'int_RI13',           'ICR13: "INT: NOT IMPLEMENTED RI13 OCCURED!!!"' ],
	0x38C:  [ 'int_RI12',           'ICR12: "INT: NOT IMPLEMENTED RI12 OCCURED!!!"' ],
	0x390:  [ 'int_uart_in',        'ICR11: Transmission interrupt/bus idle request from UART/CSIO/I2C ch.2' ],
	0x394:  [ 'int_RI10',           'ICR10: "INT: NOT IMPLEMENTED RI10 OCCURED!!!"' ],
	0x398:  [ 'int_RI09',           'ICR09: "INT: NOT IMPLEMENTED RI09 OCCURED!!!"' ],
	0x39C:  [ 'int_RI08',           'ICR08: Transmission interrupt/bus idle request from UART/CSIO/I2C ch.1' ],
	0x3A0:  [ 'int_RI07',           'ICR07: "INT: NOT IMPLEMENTED RI07 OCCURED!!!"' ],
	0x3A4:  [ 'int_RI06',           'ICR06: "INT: NOT IMPLEMENTED RI06 OCCURED!!!"' ],
	0x3A8:  [ 'int_RI05',           'ICR05: Reception interrupt request from UART/CSIO/I2C ch.0' ],
	0x3AC:  [ 'int_RI04',           'ICR04: 16-bit reload timer ch.0 to ch.2' ],
	0x3B0:  [ 'int_RI03',           'ICR03: Reserved' ],
	0x3B4:  [ 'int_RI02',           'ICR02: External interrupt request ch.16 to ch.23' ],
	0x3B8:  [ 'int_RI01',           'ICR01: Reserved' ],
	0x3BC:  [ 'int_subcpu',         'ICR00: External interrupt request ch.0 to ch.7' ],
	0x3C0:  [ 'int_shutdown',       '-' ],
	0x3C4:  [ 'int_iabt',           'Undefined instruction exception' ],
	0x3C8:  [ 'int_shutdown',       'Reserved for system' ],
	0x3CC:  [ 'int_shutdown',       'Step trace trap' ],
	0x3D0:  [ 'int_shutdown',       'Reserved for system' ],
	0x3D4:  [ 'int_shutdown',       'Reserved for system' ],
	0x3D8:  [ 'int_shutdown',       'INTE instruction' ],
	0x3DC:  [ 'int_shutdown',       'Reserved for system' ],
	0x3E0:	[ 'int_shutdown',		'Reserved for system' ],
	0x3E4:	[ 'int_shutdown',		'Reserved for system' ],
	0x3E8:	[ 'int_shutdown',		'Reserved for system' ],
	0x3EC:	[ 'int_shutdown',		'Reserved for system' ],
	0x3F0:	[ 'int_shutdown',		'Reserved for system' ],
	0x3F4:	[ 'int_shutdown',		'Reserved for system' ],
	0x3F8:	[ 'int_shutdown',		'Reserved for system' ],
	0x3FC:	[ 'int_reset', 			'Reset' ],
}

# Softune REALOS tasks
g_TaskTableDesc = [
	[ 'subcpu_task',			'Sub CPU [TID = 1]' ],
	[ 'key_manager_task',		'Key Manager [TID = 2]' ],
	[ 'gui_manager_task',		'GUI Manager [TID = 3]' ],
	[ 'debug_manager_task',		'Debug Manager [TID = 4]' ],
	[ 'file_manager_task',		'File Manager [TID = 5]' ],
	[ 'fam_manager_task',		'FAM Manager [TID = 6]' ],
	[ 'mem_manager_task',		'Memory Manager [TID = 7]' ],
	[ 'image_manager_task',		'Image Manager [TID = 8]' ],
	[ 'usb_manager_task',		'? USB Manager [TID = 9]' ],
	[ 'io_manager_task',		'IO Manager [TID = 10]' ],
	[ 'tid11_task',				'? System Manager [TID = 11]' ],
	[ 'settings_manager_task',	'Settings Manager [TID = 12]' ],
	[ 'tid13_task',				'? Monitor Manager [TID = 13]' ],
	[ 'per_manager_task',		'Peripheral Manager [TID = 14]' ],
	[ 'tid15_task',				'' ],
	[ 'tid16_task',				'' ],
]

g_SysCallTableDesc = [
	[ 0xffffffe9,	"_sta_tsk",		"(R4 [IN tsk_id], R5 [IN tsk_param], R12 [OUT err_code]) /* Start Task */" ],
	[ 0xffffffeb,	"_ext_tsk",		"(R12 [OUT err_code]) /* Exit Issuing Task */" ],
	[ 0xffffffe7,	"_ter_tsk",		"(R4 [IN tsk_id], R12 [OUT err_code]) /* Terminate Other Task */" ],
	[ 0xffffffe2,	"_dis_dsp",		"(R12 [OUT err_code] /* Disable Dispatch */)" ],
	[ 0xffffffe3,	"_ena_dsp",		"(R12 [OUT err_code] /* Enable Dispatch */)" ],
	[ 0xffffffe5,	"_chg_pri",		"(R4 [IN tsk_id], R5 [IN tsk_prio], R12 [OUT err_code] /* Change Task Priority */)" ],
	[ 0xffffffe4,	"_rot_rdq",		"(R4 [IN tsk_prio], R12 [OUT err_code] /* Rotate Tasks on the Ready Queue */)" ],
	[ 0xffffffe1,	"_rel_wai",		"(R4 [IN tsk_id], R12 [OUT err_code] /* Release Wait of Other Task */)" ],
	[ 0xffffffe8,	"_get_tid",		"(R12 [OUT err_code], R13 [OUT tsk_id] /* Get Task Identifier */)" ],
	[ 0xffffffec,	"_ref_tsk",		"(R4 [IN ptr_to_pk_rtsk], R5 [IN tsk_id], R12 [OUT err_code] /* Get Task Status */)" ],
	[ 0xffffffdf,	"_sus_tsk",		"(R4 [IN tsk_id], R12 [OUT err_code] /* Suspend Other Task */)" ],
	[ 0xffffffdd,	"_rsm_tsk",		"(R4 [IN tsk_id], R12 [OUT err_code] /* Resume Suspended Task */)" ],
	[ 0xffffffdc,	"_frsm_tsk",	"(R4 [IN tsk_id], R12 [OUT err_code] /* Forcibly Resume suspended Task */)" ],
	[ 0xffffffda,	"_slp_tsk",		"(R12 [OUT err_code] /* Sleep Task */)" ],
	[ 0xffffffdb,	"_tslp_tsk",	"(R4 [IN timeout], R12 [OUT err_code] /* Sleep Task with Timeout */)" ],
	[ 0xffffffd9,	"_wup_tsk",		"(R4 [IN tsk_id], R12 [OUT err_code] /* Wakeup Other Task */)" ],
	[ 0xffffffd8,	"_can_wup",		"(R4 [IN tsk_id], R12 [OUT err_code], R13 [OUT wakeup_count] /* Cancel Wakeup Request */)" ],
	[ 0xffffffc9,	"_sig_sem",		"(R4 [IN sem_id], R12 [OUT err_code] /* Signal Semaphore */)" ],
	[ 0xffffffcb,	"_wai_sem",		"(R4 [IN sem_id], R12 [OUT err_code] /* Wait on Semaphore */)" ],
	[ 0xffffff95,	"_preq_sem",	"(R4 [IN sem_id], R12 [OUT err_code] /* Poll and request Semaphore */)" ],
	[ 0xffffff55,	"_twai_sem",	"(R4 [IN sem_id], R5 [IN timeout], R12 [OUT err_code] /* Wait on Semaphore with Timeout */)" ],
	[ 0xffffffcc,	"_ref_sem",		"(R4 [IN ptr_to_pk_rsem], R5 [IN sem_id], R12 [OUT err_code] /* Get Semaphore Status */)" ],
	[ 0xffffffd0,	"_set_flg",		"(R4 [IN flag_id], R5 [IN set_pattern], R12 [OUT err_code] /* Set Eventflag */)" ],
	[ 0xffffffd1,	"_clr_flg",		"(R4 [IN flag_id], R5 [IN clr_pattern], R12 [OUT err_code] /* Clear Eventflag */)" ],
	[ 0xffffffd2,	"_wai_flg",		"(R5 [IN flag_id], R6 [IN wai_pattern], R7 [IN wait_flag_mode], R12 [OUT err_code], R13 [OUT flag_pattern] /* Wait for Eventflag */)" ],
	[ 0xffffff96,	"_pol_flg",		"(R5 [IN flag_id], R6 [IN wai_pattern], R7 [IN wait_flag_mode], R12 [OUT err_code], R13 [OUT flag_pattern] /* Wait for Eventflag, polling */)" ],
	[ 0xffffff56,	"_twai_flg",	"(R5 [IN flag_id], R6 [IN wai_pattern], R7 [IN wait_flag_mode], R12 [OUT err_code], R13 [IN timeout; OUT flag_pattern] /* Wait for Eventflag, with Timeout */)" ],
	[ 0xffffffd4,	"_ref_flg",		"(R4 [IN ptr_to_pk_rflg], R5 [IN flag_id], R12 [OUT err_code] /* Get Eventflag Status */)" ],
	[ 0xffffffc1,	"_snd_msg",		"(R4 [IN mailbox_id], R5 [IN ptr_to_pk_msg], R12 [OUT err_code] /* Send Message to Mailbox */)" ],
	[ 0xffffffc3,	"_rcv_msg",		"(R5 [IN mailbox_id], R12 [OUT err_code], R13 [OUT ptr_to_pk_msg] /* Receive Message from Mailbox */)" ],
	[ 0xffffff94,	"_prcv_msg",	"(R5 [IN mailbox_id], R12 [OUT err_code], R13 [OUT ptr_to_pk_msg] /* Receive Message from Mailbox, polling */)" ],
	[ 0xffffff54,	"_trcv_msg",	"(R5 [IN mailbox_id], R6 [IN timeout], R12 [OUT err_code], R13 [OUT ptr_to_pk_msg] /* Receive Message from Mailbox, with Timeout */)" ],
	[ 0xffffffc4,	"_ref_mbx",		"(R4 [IN ptr_to_pk_rmbx], R5 [IN mailbox_id], R12 [OUT err_code] /* Get Mailbox Status */)" ],
	[ 0xffffff73,	"_get_blk",		"(R5 [IN mempool_id], R6 [IN blk_size], R12 [OUT err_code], R13 [OUT blk_start] /* Get Variable-Sized Memory Block */)" ],
	[ 0xffffff98,	"_pget_blk",	"(R5 [IN mempool_id], R6 [IN blk_size], R12 [OUT err_code], R13 [OUT blk_start] /* Get Variable-Sized Memory Block, polling */)" ],
	[ 0xffffff71,	"_rel_blk",		"(R4 [IN mempool_id], R5 [OUT blk_start], R12 [OUT err_code] /* Release Variable-Sized Memory Block */)" ],
	[ 0xffffff74,	"_ref_mpl",		"(R4 [IN ptr_to_pk_rmempool], R5 [IN mempool_id], R12 [OUT err_code] /* Get Variable-Sized Memorypool Status */)" ],
	[ 0xffffffb3,	"_get_blf",		"(R5 [IN mempool_f_id], R12 [OUT err_code], R13 [OUT blk_f_start] /* Get Fixed-Sized Memory Block */)" ],
	[ 0xffffff99,	"_pget_blf",	"(R5 [IN mempool_f_id], R12 [OUT err_code], R13 [OUT blk_f_start] /* Poll and Get Fixed-Sized Memory Block */)" ],
	[ 0xffffff59,	"_tget_blf",	"(R5 [IN mempool_f_id], R6 [IN timeout], R12 [OUT err_code], R13 [OUT blk_f_start] /* Get Fixed-Sized Memory Block with Timeout */)" ],
	[ 0xffffffb1,	"_rel_blf",		"(R4 [IN mempool_f_id], R5 [OUT blk_f_start], R12 [OUT err_code] /* Release Fixed-Sized Memory Block */)" ],
	[ 0xffffffb4,	"_ref_mpf",		"(R4 [IN ptr_to_pk_rmempool_f], R5 [IN mempool_f_id], R12 [OUT err_code] /* Get Fixed-Sized Memorypool Status */)" ],
	[ 0xffffffbb,	"_ret_int",		"(/* Return from Interrupt Handler */)" ],
	[ 0xfffffff8,	"_loc_cpu",		"(R12 [OUT err_code] /* Lock CPU */)" ],
	[ 0xfffffff9,	"_unl_cpu",		"(R12 [OUT err_code] /* Unlock CPU */)" ],
	[ 0xffffffbd,	"_chg_ilm",		"(R4 [IN ilmask], R12 [OUT err_code] /* Change Interrupt Level Mask */)" ],
	[ 0xffffffbc,	"_ref_ilm",		"(R12 [OUT err_code], R13 [OUT ilmask] /* Get Interrupt Level Mask */)" ],
	[ 0xffffffad,	"_set_tim",		"(R4 [IN ptr_to_pk_tim], R12 [OUT err_code] /* Set System Clock */)" ],
	[ 0xffffffac,	"_get_tim",		"(R4 [IN ptr_to_pk_tim], R12 [OUT err_code] /* Get System Clock */)" ],
	[ 0xffffffab,	"_dly_tsk",		"(R4 [IN dly_tim], R12 [OUT err_code] /* Delay Task */)" ],
	[ 0xffffffa6,	"_def_cyc",		"(R4 [IN cycno], R5 [IN ptr_to_pk_dcyc], R12 [OUT err_code] /* Define Cyclic Handler */)" ],
	[ 0xffffffa2,	"_act_cyc",		"(R4 [IN cycno], R5 [IN cycact], R12 [OUT err_code] /* Activate Cyclic Handler */)" ],
	[ 0xffffffa4,	"_ref_cyc",		"(R5 [IN ptr_to_pk_rcyc], R5 [IN cycno], R12 [OUT err_code] /* Get Cyclic Handler Status */)" ],
	[ 0xffffffa7,	"_def_alm",		"(R4 [IN almno], R5 [IN ptr_to_pk_dalm], R12 [OUT err_code] /* Define Alarm Handler */)" ],
	[ 0xffffffa5,	"_ref_alm",		"(R4 [IN ptr_to_pk_ralm], R5 [IN almno], R12 [OUT err_code] /* Get Alarm Handler Status */)" ],
	[ 0xffffffa3,	"_ret_tmr",		"(/* Return from Timer Handler */)" ],
	[ 0xfffffff0,	"_get_ver",		"(R4 [IN ptr_to_pk_ver], R12 [OUT err_code] /* Get Version Information */)" ],
	[ 0xfffffff4,	"_ref_sys",		"(R4 [IN ptr_to_pk_rsys], R12 [OUT err_code] /* Get System Status */)" ],
]

# ============================= FIXES ==============================

def m240_fix_int_hdlrs(start):
	print ('  M240: Correct RESET function...')
	ea = start
	while (get_16bit(ea) & 0xFFF0) != 0x9700: # JMP @Rj
		ea = ea + 2	
	end = ea + 2
	func_setend(start, end)

	main = get_32bit(end - 6)
	MakeFunction(main, BADADDR)

def m240_fix_stdlib():
	ea = get_name_ea(BADADDR, "strtok")
	if ea != BADADDR:
		func = get_next_func(ea)
		MakeName(func.startEA, "strtol")
		func.flags = func.flags | FUNC_LIB
		update_func(func) 
		print ("  add 'strtol'")

	ea = get_name_ea(BADADDR, "memcmp")
	if ea != BADADDR:
		func = get_prev_func(ea)
		MakeName(func.startEA, "malloc")
		func.flags = func.flags | FUNC_LIB
		update_func(func)
		print ("  add 'malloc'")

def m240_cmt_syscalls():
	pass

def m240_prc_gui_data():
	pass

# ============================= UTILS ==============================

def prepare_code(m240fix):
	ea = MinEA()
	end = BADADDR
	while get_16bit(ea) == 0x9720: # RET
		MakeFunction(ea, BADADDR)
		Wait()
		ea = ea + 2;
	# First non RET function found should be reset
	start = ea

	MakeFunction(start, BADADDR)
	Wait()

	if m240fix == 1:
		m240_fix_int_hdlrs(start)

def apply_stdlib():
	ApplySig("softune_c_cpp")

def setStringFromOffset(value):
	inf = get_inf_structure()
	if value:
		inf.af = inf.af | AF_ASCII
	else:
		inf.af = inf.af & (~AF_ASCII)

def setOperandToOffset(value):
	inf = get_inf_structure()
	if value:
		inf.af = inf.af | AF_IMMOFF
	else:
		inf.af = inf.af & (~AF_IMMOFF)

def setFullSpAnalysis(value):
	inf = get_inf_structure()
	if value:
		inf.af2 = inf.af2 | AF2_VERSP
	else:
		inf.af2 = inf.af2 & (~AF2_VERSP)

# ============================= NAMING =============================

def name_known_ints(addr):
	int_addr = addr
	for x in xrange(256):
		key = int_addr - addr
		if key in g_IntTableDesc:
			desc = g_IntTableDesc[key]
			if desc[0] != '':
				MakeName(get_32bit(int_addr), desc[0])
			if desc[1] != '':
				MakeComm(int_addr, desc[1])
		int_addr = int_addr + 4

def name_known_tasks(addr):
	int_addr = addr
	for x in xrange(16):
		desc = g_TaskTableDesc[x]
		if desc[0] != '':
			MakeName(get_32bit(int_addr+16), desc[0])
		if desc[1] != '':
			MakeComm(int_addr+16, desc[1])
		int_addr = int_addr + 28

def name_known_syscall(idx, addr):
	MakeName(addr, g_SysCallTableDesc[idx][1]);
	if isFunc(getFlags(addr)):
		SetFunctionCmt(addr, g_SysCallTableDesc[idx][2], 0)
	else:
		MakeComm(addr, g_SysCallTableDesc[idx][2])

# ============================ PARSERS =============================

def find_int_table():
	tb_addr = 0
	ea = MinEA()
	decode_insn(ea)
	while ea != BADADDR:
		if find(GetDisasm(cmd.ea), ', TBR') != -1:
			reg = GetOpnd(ea, 0)
			ea = decode_prev_insn(ea)
			while ea != BADADDR:
				if ((GetMnem(ea) == 'LDI:32') and (GetOpnd(ea, 1) == reg)):
					tb_addr = GetOperandValue(ea, 0)
					return tb_addr
				ea = decode_prev_insn(ea)
		decode_insn(ea+idaapi.cmd.size)
		ea = cmd.ea
	return

def parse_int_table(addr):
	for x in xrange(256):
		doDwrd(addr, 4)
		op_offset(addr, 0, REF_OFF32)
		hdl_addr = get_32bit(addr)
		if isUnknown(getFlags(hdl_addr)) == False:
			if isFunc(getFlags(hdl_addr)):
				add_cref(addr, get_32bit(addr), fl_JN)
			else:
				MakeUnknown(hdl_addr, get_item_size(hdl_addr), 0)
				add_cref(addr, get_32bit(addr), fl_CF)
		else:
			add_cref(addr, get_32bit(addr), fl_CF)
		addr = addr + 4

def find_task_table():
	# FIXME: not implemented
	return 0x0

def parse_task_table(addr):
	for x in xrange(16):
		doDwrd(addr + 0x0, 4)
		doDwrd(addr + 0x4, 4)
		doDwrd(addr + 0x8, 4)
		doDwrd(addr + 0xC, 4)
		doDwrd(addr + 0x10, 4)
		op_offset(addr + 0x10, 0, REF_OFF32)
		add_cref(addr + 0x10, get_32bit(addr + 0x10), fl_CF)
		doDwrd(addr + 0x14, 4)
		doDwrd(addr + 0x18, 4)
		addr += 28

def find_syscall_table():
	ea = get_name_ea(BADADDR, "int_realos_syscall")
	if ea == BADADDR:
		return 0
	decode_insn(ea)
	while cmd.get_canon_mnem() != 'RETI':
		#print ('  %X %s %d %d %d' % (ea, GetMnem(ea), cmd.Op1.value, cmd.Op2.reg, cmd.size))
		if ((cmd.get_canon_mnem() == 'LSL') and (cmd.Op1.value == 1) and (cmd.Op2.reg == 12)):
			ea = ea + cmd.size
			decode_insn(ea)			
			if (cmd.get_canon_mnem() == 'LDI:32'):
				return cmd.Op1.value
		ea = ea + cmd.size
		decode_insn(ea)
	return 0

def parse_syscall_table(addr):
	for x in xrange(len(g_SysCallTableDesc)):
		sc_off = g_SysCallTableDesc[x][0]
		sc_off = sc_off << 1

		tb_off = ctypes.c_int32(addr + sc_off).value;
		doWord(tb_off, 2)
		MakeComm(tb_off, g_SysCallTableDesc[x][1])
		#op_offset(tb_off, 0, REF_OFF16)
		sc_func = get_16bit(tb_off) + addr
		print('  [%2d] 0x%.8X = 0x%.8X:%s' % (x, sc_func, g_SysCallTableDesc[x][0], g_SysCallTableDesc[x][1]))

		if isUnknown(getFlags(sc_func)) == False:
			if isFunc(getFlags(sc_func)):
				#print('  func')
				add_cref(tb_off, sc_func, fl_JN)
			else:
				#print('  not func')
				MakeUnknown(sc_func, get_item_size(sc_func), 0)
				add_cref(tb_off, sc_func, fl_CF)
		else:
			#print('  unknown')
			add_cref(tb_off, sc_func, fl_CF)

		Wait()
		name_known_syscall(x, sc_func)
		
def find_functions():
	count = 0
	func = get_func(MinEA())
	while func is not None:
		addr = func.endEA
		if isUnknown(getFlags(addr)) == False:
			#print ('  BAD0: 0x%X' % addr)
			func = get_next_func(func.startEA)
			continue
		if isFunc(getFlags(addr)) == True:
			#print ('  NEXT1: 0x%X' % func.startEA)
			func = get_next_func(func.startEA)
			continue
		opcode = get_16bit(addr)
		# STM0, STM1, ST Ri, @-R15
		if ((opcode & 0xFF00) == 0x8E00 or \
			(opcode & 0xFF00) == 0x8F00 or \
			(opcode & 0xFFF0) == 0x1700 or \
			(opcode & 0xFFF0) == 0x1780):
			#print('  FOUND1: 0x%.8X' % addr)
			MakeFunction(addr, BADADDR)
			Wait()
			count = count + 1;
			func = get_func(addr)
			continue
		#else:
			#print('  BAD1: 0x%.8X: %.4X' % (addr, opcode))
		addr = addr + 2
		if isFunc(getFlags(addr)) == True:
			#print ('  NEXT2: 0x%X' % func.startEA)			
			func = get_next_func(func.startEA)
			continue
		opcode = get_16bit(addr)
		if ((opcode & 0xFF00) == 0x8E00 or \
			(opcode & 0xFF00) == 0x8F00 or \
			(opcode & 0xFFF0) == 0x1700 or \
			(opcode & 0xFFF0) == 0x1780):
			#print('  FOUND2: 0x%.8X' % addr)
			MakeFunction(addr, BADADDR)
			Wait()
			count = count + 1;
			func = get_func(addr)
			continue
		#else:
			#print('  BAD2: 0x%.8X: %.4X' % (addr, opcode))
		#print ('  NEXT3: 0x%X' % func.startEA)	
		func = get_next_func(func.startEA)
	print('  New functions found: %d' % count)
	return count

# ============================== MAIN ==============================

def main():
	setStringFromOffset(False)
	setOperandToOffset(False)
	#setFullSpAnalysis(True)

	dlgM240 = AskYN(0,"Apply Leica M240 fixes?")
	
	dlgAnswer = AskYN(0,"Have you completed these steps?\n"
						"1. Perform initial code analysis\n"
						"2. Apply known enums/structures")

	if dlgAnswer == -1:
		return

	print ('---[ Softune Helper ]-------------------------------------------------------------------------------------------------')

	if dlgAnswer == 0:
		print ('Prepare code')
		print ('  Searching for RESET...')
		prepare_code(dlgM240)
		print ('')

	int_tbl_addr = find_int_table()
	if int_tbl_addr != 0:
		print ('Interrupt table found at 0x%X' % int_tbl_addr)
		MakeName(int_tbl_addr, 'int_table')
		MakeComm(int_tbl_addr, 'INTERRUPT TABLE')
		print ('  Parse interrupt table...')
		parse_int_table(int_tbl_addr)
		print ('  Name known interrupts...')
		name_known_ints(int_tbl_addr)
		Wait()
		print ('')

	task_tbl_addr = find_task_table()
	if task_tbl_addr != 0:
		print ('Task table found at 0x%X' % task_tbl_addr)
		MakeName(task_tbl_addr, 'task_table')
		MakeComm(task_tbl_addr, 'TASK TABLE')
		print ('  Parse task table...')
		parse_task_table(task_tbl_addr)
		print ('  Name known tasks...')
		name_known_tasks(task_tbl_addr)
		Wait()
		print ('')
	
	syscall_tbl_addr = find_syscall_table()
	if syscall_tbl_addr != 0:
		print ('Syscall table found at 0x%X' % syscall_tbl_addr)
		print ('  Parse syscall table...')
		parse_syscall_table(syscall_tbl_addr)
		Wait()
		print ('')
	
	dlgAnswer = AskYN(0,"Find unexplored functions?")
	if dlgAnswer == 1:
		while find_functions() != 0:
			pass

	print ('  Applying SOFTUNE C/C++ Library...')
	apply_stdlib()
	Wait()
	print ('')

	print ('All done')
	print ('----------------------------------------------------------------------------------------------------------------------')

	if dlgM240 == 1:
		print ('')
		print ('---[ Leica M240 fixes ]-----------------------------------------------------------------------------------------------')

		print ('Add missing stdlib functions')
		m240_fix_stdlib()
		Wait()
		print ('')	

		print ('Comment syscalls')
		m240_cmt_syscalls()
		Wait()
		print ('')	

		print ('Process GUI data')
		m240_prc_gui_data()
		Wait()
		print ('')	

		print ('All done')
		print ('----------------------------------------------------------------------------------------------------------------------')

	# Reanalyze everything recovering offsets and strings		
	setStringFromOffset(True)
	# setOperandToOffset(True)
	auto_mark_range(MinEA(), MaxEA(), AU_USED)
	
if __name__ == '__main__':
	main()
