
#define UNLOADED_FILE   1
#include <idc.idc>

static main(void)
{
  Enums();              // enumerations
  Structures();         // structure types
	LowVoids(0x20);
	HighVoids(0x800000);
}

static Enums_0(id) {

	id = AddEnum(-1,"RealosTaskID",0x1100000);
	AddConstEx(id,"kTaskID_SubCPU",	0X1,	-1);
	AddConstEx(id,"kTaskID_KeyManager",	0X2,	-1);
	AddConstEx(id,"kTaskID_GuiManager",	0X3,	-1);
	AddConstEx(id,"kTaskID_DebugManager",	0X4,	-1);
	AddConstEx(id,"kTaskID_FileManager",	0X5,	-1);
	AddConstEx(id,"kTaskID_FamManager",	0X6,	-1);
	AddConstEx(id,"kTaskID_MemoryManager",	0X7,	-1);
	AddConstEx(id,"kTaskID_ImageManager",	0X8,	-1);
	AddConstEx(id,"kTaskID_UsbManager",	0X9,	-1);
	AddConstEx(id,"kTaskID_IoManager",	0XA,	-1);
	AddConstEx(id,"kTaskID_SystemManager",	0XB,	-1);
	AddConstEx(id,"kTaskID_SettingsManager",	0XC,	-1);
	AddConstEx(id,"kTaskID_MonitorManager",	0XD,	-1);
	AddConstEx(id,"kTaskID_PeripheralManager",	0XE,	-1);
	AddConstEx(id,"kTaskID_Unknown",	0XF,	-1);
	id = AddEnum(-1,"RealosSysCall",0x1100000);
	AddConstEx(id,"kSysCall_get_blf",	0X59,	-1);
	SetConstCmt(GetConstEx(id,0X59,0,-1),"(R5 [IN mempool_f_id], R6 [IN timeout], R12 [OUT err_code], R13 [OUT blk_f_start]) // Get Fixed-Sized Memory Block with Timeout",0);
	AddConstEx(id,"kSysCall_get_blk",	0X73,	-1);
	SetConstCmt(GetConstEx(id,0X73,0,-1),"(R5 [IN mempool_id], R6 [IN blk_size], R12 [OUT err_code], R13 [OUT blk_start]) // Get Variable-Sized Memory Block",0);
	AddConstEx(id,"kSysCall_prcv_msg",	0X94,	-1);
	SetConstCmt(GetConstEx(id,0X94,0,-1),"(R5 [IN mailbox_id], R12 [OUT err_code], R13 [OUT ptr_to_pk_msg]) // Receive Message from Mailbox, polling",0);
	AddConstEx(id,"kSysCall_preq_sem",	0X95,	-1);
	SetConstCmt(GetConstEx(id,0X95,0,-1),"(R4 [IN sem_id], R12 [OUT err_code]) // Poll and request Semaphore",0);
	AddConstEx(id,"kSysCall_pol_flg",	0X96,	-1);
	SetConstCmt(GetConstEx(id,0X96,0,-1),"(R5 [IN flag_id], R6 [IN wai_pattern], R7 [IN wait_flag_mode], R12 [OUT err_code], R13 [OUT flag_pattern]) // Wait for Eventflag, polling",0);
	AddConstEx(id,"kSysCall_pget_blk",	0X98,	-1);
	SetConstCmt(GetConstEx(id,0X98,0,-1),"(R5 [IN mempool_id], R6 [IN blk_size], R12 [OUT err_code], R13 [OUT blk_start]) // Get Variable-Sized Memory Block, polling",0);
	AddConstEx(id,"kSysCall_pget_blf",	0X99,	-1);
	SetConstCmt(GetConstEx(id,0X99,0,-1),"(R5 [IN mempool_f_id], R12 [OUT err_code], R13 [OUT blk_f_start]) // Poll and Get Fixed-Sized Memory Block",0);
	AddConstEx(id,"kSysCall_act_cyc",	0XA2,	-1);
	SetConstCmt(GetConstEx(id,0XA2,0,-1),"(R4 [IN cycno], R5 [IN cycact], R12 [OUT err_code]) // Activate Cyclic Handler",0);
	AddConstEx(id,"kSysCall_ret_tmr",	0XA3,	-1);
	SetConstCmt(GetConstEx(id,0XA3,0,-1),"() // Return from Timer Handler",0);
	AddConstEx(id,"kSysCall_ref_cyc",	0XA4,	-1);
	SetConstCmt(GetConstEx(id,0XA4,0,-1),"(R5 [IN ptr_to_pk_rcyc], R5 [IN cycno], R12 [OUT err_code]) // Get Cyclic Handler Status",0);
	AddConstEx(id,"kSysCall_ref_alm",	0XA5,	-1);
	SetConstCmt(GetConstEx(id,0XA5,0,-1),"(R4 [IN ptr_to_pk_ralm], R5 [IN almno], R12 [OUT err_code]) // Get Alarm Handler Status",0);
	AddConstEx(id,"kSysCall_def_cyc",	0XA6,	-1);
	SetConstCmt(GetConstEx(id,0XA6,0,-1),"(R4 [IN cycno], R5 [IN ptr_to_pk_dcyc], R12 [OUT err_code]) // Define Cyclic Handler",0);
	AddConstEx(id,"kSysCall_def_alm",	0XA7,	-1);
	SetConstCmt(GetConstEx(id,0XA7,0,-1),"(R4 [IN almno], R5 [IN ptr_to_pk_dalm], R12 [OUT err_code]) // Define Alarm Handler",0);
	AddConstEx(id,"kSysCall_dly_tsk",	0XAB,	-1);
	SetConstCmt(GetConstEx(id,0XAB,0,-1),"(R4 [IN dly_tim], R12 [OUT err_code]) // Delay Task",0);
	AddConstEx(id,"kSysCall_get_tim",	0XAC,	-1);
	SetConstCmt(GetConstEx(id,0XAC,0,-1),"(R4 [IN ptr_to_pk_tim], R12 [OUT err_code]) // Get System Clock",0);
	AddConstEx(id,"kSysCall_set_tim",	0XAD,	-1);
	SetConstCmt(GetConstEx(id,0XAD,0,-1),"(R4 [IN ptr_to_pk_tim], R12 [OUT err_code]) // Set System Clock",0);
	AddConstEx(id,"kSysCall_rel_blf",	0XB1,	-1);
	SetConstCmt(GetConstEx(id,0XB1,0,-1),"(R4 [IN mempool_f_id], R5 [OUT blk_f_start], R12 [OUT err_code]) // Release Fixed-Sized Memory Block",0);
	AddConstEx(id,"kSysCall_get_blf",	0XB3,	-1);
	SetConstCmt(GetConstEx(id,0XB3,0,-1),"(R5 [IN mempool_f_id], R12 [OUT err_code], R13 [OUT blk_f_start]) // Get Fixed-Sized Memory Block",0);
	AddConstEx(id,"kSysCall_ref_mpf",	0XB4,	-1);
	SetConstCmt(GetConstEx(id,0XB4,0,-1),"(R4 [IN ptr_to_pk_rmempool_f], R5 [IN mempool_f_id], R12 [OUT err_code]) // Get Fixed-Sized Memorypool Status",0);
	AddConstEx(id,"kSysCall_ret_int",	0XBB,	-1);
	SetConstCmt(GetConstEx(id,0XBB,0,-1),"() // Return from Interrupt Handler",0);
	AddConstEx(id,"kSysCall_ref_ilm",	0XBC,	-1);
	SetConstCmt(GetConstEx(id,0XBC,0,-1),"(R12 [OUT err_code], R13 [OUT ilmask]) // Get Interrupt Level Mask",0);
	AddConstEx(id,"kSysCall_chg_ilm",	0XBD,	-1);
	SetConstCmt(GetConstEx(id,0XBD,0,-1),"(R4 [IN ilmask], R12 [OUT err_code]) // Change Interrupt Level Mask",0);
	AddConstEx(id,"kSysCall_snd_msg",	0XC1,	-1);
	SetConstCmt(GetConstEx(id,0XC1,0,-1),"(R4 [IN mailbox_id], R5 [IN ptr_to_pk_msg], R12 [OUT err_code]) // Send Message to Mailbox",0);
	AddConstEx(id,"kSysCall_rcv_msg",	0XC3,	-1);
	SetConstCmt(GetConstEx(id,0XC3,0,-1),"(R5 [IN mailbox_id], R12 [OUT err_code], R13 [OUT ptr_to_pk_msg]) // Receive Message from Mailbox",0);
	AddConstEx(id,"kSysCall_ref_mbx",	0XC4,	-1);
	SetConstCmt(GetConstEx(id,0XC4,0,-1),"(R4 [IN ptr_to_pk_rmbx], R5 [IN mailbox_id], R12 [OUT err_code]) // Get Mailbox Status",0);
	AddConstEx(id,"kSysCall_sig_sem",	0XC9,	-1);
	SetConstCmt(GetConstEx(id,0XC9,0,-1),"(R4 [IN sem_id], R12 [OUT err_code]) // Signal Semaphore",0);
	AddConstEx(id,"kSysCall_wai_sem",	0XCB,	-1);
	SetConstCmt(GetConstEx(id,0XCB,0,-1),"(R4 [IN sem_id], R12 [OUT err_code]) // Wait on Semaphore",0);
	AddConstEx(id,"kSysCall_ref_sem",	0XCC,	-1);
	SetConstCmt(GetConstEx(id,0XCC,0,-1),"(R4 [IN ptr_to_pk_rsem], R5 [IN sem_id], R12 [OUT err_code]) // Get Semaphore Status",0);
	AddConstEx(id,"kSysCall_set_flg",	0XD0,	-1);
	SetConstCmt(GetConstEx(id,0XD0,0,-1),"(R4 [IN flag_id], R5 [IN set_pattern], R12 [OUT err_code]) // Set Eventflag",0);
	AddConstEx(id,"kSysCall_clr_flg",	0XD1,	-1);
	SetConstCmt(GetConstEx(id,0XD1,0,-1),"(R4 [IN flag_id], R5 [IN clr_pattern], R12 [OUT err_code]) // Clear Eventflag",0);
	AddConstEx(id,"kSysCall_wai_flg",	0XD2,	-1);
	SetConstCmt(GetConstEx(id,0XD2,0,-1),"(R5 [IN flag_id], R6 [IN wai_pattern], R7 [IN wait_flag_mode], R12 [OUT err_code], R13 [OUT flag_pattern]) // Wait for Eventflag",0);
	AddConstEx(id,"kSysCall_ref_flg",	0XD4,	-1);
	SetConstCmt(GetConstEx(id,0XD4,0,-1),"(R4 [IN ptr_to_pk_rflg], R5 [IN flag_id], R12 [OUT err_code]) // Get Eventflag Status",0);
	AddConstEx(id,"kSysCall_can_wup",	0XD8,	-1);
	SetConstCmt(GetConstEx(id,0XD8,0,-1),"(R4 [IN tsk_id], R12 [OUT err_code], R13 [OUT wakeup_count]) // Cancel Wakeup Request",0);
	AddConstEx(id,"kSysCall_wup_tsk",	0XD9,	-1);
	SetConstCmt(GetConstEx(id,0XD9,0,-1),"(R4 [IN tsk_id], R12 [OUT err_code]) // Wakeup Other Task",0);
	AddConstEx(id,"kSysCall_slp_tsk",	0XDA,	-1);
	SetConstCmt(GetConstEx(id,0XDA,0,-1),"(R12 [OUT err_code]) // Sleep Task",0);
	AddConstEx(id,"kSysCall_tslp_tsk",	0XDB,	-1);
	SetConstCmt(GetConstEx(id,0XDB,0,-1),"(R4 [IN timeout], R12 [OUT err_code]) // Sleep Task with Timeout",0);
	AddConstEx(id,"kSysCall_frsm_tsk",	0XDC,	-1);
	SetConstCmt(GetConstEx(id,0XDC,0,-1),"(R4 [IN tsk_id], R12 [OUT err_code]) // Forcibly Resume suspended Task",0);
	AddConstEx(id,"kSysCall_rsm_tsk",	0XDD,	-1);
	SetConstCmt(GetConstEx(id,0XDD,0,-1),"(R4 [IN tsk_id], R12 [OUT err_code]) // Resume Suspended Task",0);
	AddConstEx(id,"kSysCall_sus_tsk",	0XDF,	-1);
	SetConstCmt(GetConstEx(id,0XDF,0,-1),"(R4 [IN tsk_id], R12 [OUT err_code]) // Suspend Other Task",0);
	AddConstEx(id,"kSysCall_rel_wai",	0XE1,	-1);
	SetConstCmt(GetConstEx(id,0XE1,0,-1),"(R4 [IN tsk_id], R12 [OUT err_code]) // Release Wait of Other Task",0);
	AddConstEx(id,"kSysCall_dis_dsp",	0XE2,	-1);
	SetConstCmt(GetConstEx(id,0XE2,0,-1),"(R12 [OUT err_code]) // Disable Dispatch",0);
	AddConstEx(id,"kSysCall_ena_dsp",	0XE3,	-1);
	SetConstCmt(GetConstEx(id,0XE3,0,-1),"(R12 [OUT err_code]) // Enable Dispatch",0);
	AddConstEx(id,"kSysCall_rot_rdq",	0XE4,	-1);
	SetConstCmt(GetConstEx(id,0XE4,0,-1),"(R4 [IN tsk_prio], R12 [OUT err_code]) // Rotate Tasks on the Ready Queue",0);
	AddConstEx(id,"kSysCall_chg_pri",	0XE5,	-1);
	SetConstCmt(GetConstEx(id,0XE5,0,-1),"(R4 [IN tsk_id], R5 [IN tsk_prio], R12 [OUT err_code]) // Change Task Priority",0);
	AddConstEx(id,"kSysCall_ter_tsk",	0XE7,	-1);
	SetConstCmt(GetConstEx(id,0XE7,0,-1),"(R4 [IN tsk_id], R12 [OUT err_code]) // Terminate Other Task",0);
	AddConstEx(id,"kSysCall_get_tid",	0XE8,	-1);
	SetConstCmt(GetConstEx(id,0XE8,0,-1),"(R12 [OUT err_code], R13 [OUT tsk_id]) // Get Task Identifier",0);
	AddConstEx(id,"kSysCall_sta_tsk",	0XE9,	-1);
	SetConstCmt(GetConstEx(id,0XE9,0,-1),"(R4 [IN tsk_id], R5 [IN tsk_param], R12 [OUT err_code]) // Start Task",0);
	AddConstEx(id,"kSysCall_ext_tsk",	0XEB,	-1);
	SetConstCmt(GetConstEx(id,0XEB,0,-1),"(R12 [OUT err_code]) // Exit Issuing Task",0);
	AddConstEx(id,"kSysCall_ref_tsk",	0XEC,	-1);
	SetConstCmt(GetConstEx(id,0XEC,0,-1),"(R4 [IN ptr_to_pk_rtsk], R5 [IN tsk_id], R12 [OUT err_code]) // Get Task Status",0);
	AddConstEx(id,"kSysCall_get_ver",	0XF0,	-1);
	SetConstCmt(GetConstEx(id,0XF0,0,-1),"(R4 [IN ptr_to_pk_ver], R12 [OUT err_code]) // Get Version Information",0);
	AddConstEx(id,"kSysCall_ref_sys",	0XF4,	-1);
	SetConstCmt(GetConstEx(id,0XF4,0,-1),"(R4 [IN ptr_to_pk_rsys], R12 [OUT err_code]) // Get System Status",0);
	AddConstEx(id,"kSysCall_loc_cpu",	0XF8,	-1);
	SetConstCmt(GetConstEx(id,0XF8,0,-1),"(R12 [OUT err_code]) // Lock CPU",0);
	AddConstEx(id,"kSysCall_unl_cpu",	0XF9,	-1);
	SetConstCmt(GetConstEx(id,0XF9,0,-1),"(R12 [OUT err_code]) // Unlock CPU",0);
	AddConstEx(id,"kSysCall_trcv_msg",	0XFFFFFF54,	-1);
	SetConstCmt(GetConstEx(id,0XFFFFFF54,0,-1),"(R5 [IN mailbox_id], R6 [IN timeout], R12 [OUT err_code], R13 [OUT ptr_to_pk_msg]) // Receive Message from Mailbox, with Timeout",0);
	AddConstEx(id,"kSysCall_twai_sem",	0XFFFFFF55,	-1);
	SetConstCmt(GetConstEx(id,0XFFFFFF55,0,-1),"(R4 [IN sem_id], R5 [IN timeout], R12 [OUT err_code]) // Wait on Semaphore with Timeout",0);
	AddConstEx(id,"kSysCall_twai_flg",	0XFFFFFF56,	-1);
	SetConstCmt(GetConstEx(id,0XFFFFFF56,0,-1),"(R5 [IN flag_id], R6 [IN wai_pattern], R7 [IN wait_flag_mode], R12 [OUT err_code], R13 [IN timeout; OUT flag_pattern]) // Wait for Eventflag, with Timeout",0);
	AddConstEx(id,"kSysCall_rel_blk",	0XFFFFFF71,	-1);
	SetConstCmt(GetConstEx(id,0XFFFFFF71,0,-1),"(R4 [IN mempool_id], R5 [OUT blk_start], R12 [OUT err_code]) // Release Variable-Sized Memory Block",0);
	AddConstEx(id,"kSysCall_ref_mpl",	0XFFFFFF74,	-1);
	SetConstCmt(GetConstEx(id,0XFFFFFF74,0,-1),"(R4 [IN ptr_to_pk_rmempool], R5 [IN mempool_id], R12 [OUT err_code]) // Get Variable-Sized Memorypool Status",0);
	id = AddEnum(-1,"kControlActionType",0x1100000);
	AddConstEx(id,"kControlAction_Push",	0X1,	-1);
	AddConstEx(id,"kControlAction_Release",	0X2,	-1);
	AddConstEx(id,"kControlAction_LongPush",	0X3,	-1);
	id = AddEnum(-1,"kControlBtnType",0x1100000);
	AddConstEx(id,"kControlBtn_LV",	0,	-1);
	AddConstEx(id,"kControlBtn_PLAY",	0X1,	-1);
	AddConstEx(id,"kControlBtn_DEL",	0X2,	-1);
	AddConstEx(id,"kControlBtn_ISO",	0X3,	-1);
	AddConstEx(id,"kControlBtn_MENU",	0X4,	-1);
	AddConstEx(id,"kControlBtn_SET",	0X5,	-1);
	id = AddEnum(-1,"kControlJoystickType",0x1100000);
	AddConstEx(id,"kControlJoy_Click",	0,	-1);
	AddConstEx(id,"kControlJoy_Up",	0X1,	-1);
	AddConstEx(id,"kControlJoy_Down",	0X2,	-1);
	AddConstEx(id,"kControlJoy_Left",	0X3,	-1);
	AddConstEx(id,"kControlJoy_Right",	0X4,	-1);
	id = AddEnum(-1,"PTPCommand",0x1100000);
	AddConstEx(id,"kPTPCommand_GetDeviceInfo", 0x1001, -1);
	AddConstEx(id,"kPTPCommand_OpenSession", 0x1002, -1);
	AddConstEx(id,"kPTPCommand_CloseSession", 0x1003, -1);
	AddConstEx(id,"kPTPCommand_GetStorageID", 0x1004, -1);
	AddConstEx(id,"kPTPCommand_GetStorageInfo", 0x1005, -1);
	AddConstEx(id,"kPTPCommand_GetNumObjects", 0x1006, -1);
	AddConstEx(id,"kPTPCommand_GetObjectHandles", 0x1007, -1);
	AddConstEx(id,"kPTPCommand_GetObjectInfo", 0x1008, -1);
	AddConstEx(id,"kPTPCommand_GetObject", 0x1009, -1);
	AddConstEx(id,"kPTPCommand_GetThumb", 0x100A, -1);
	AddConstEx(id,"kPTPCommand_DeleteObject", 0x100B, -1);
	AddConstEx(id,"kPTPCommand_InitiateCapture", 0x100E, -1);
	AddConstEx(id,"kPTPCommand_GetDevicePropDesc", 0x1014, -1);
	AddConstEx(id,"kPTPCommand_GetDevicePropV", 0x1015, -1);
	AddConstEx(id,"kPTPCommand_InitiateOpenCapture", 0x101C, -1);
	AddConstEx(id,"kPTPCommandVE_SetCameraSettings", 0x9001, -1);
	AddConstEx(id,"kPTPCommandVE_GetCameraSettings", 0x9002, -1);
	AddConstEx(id,"kPTPCommandVE_GetLensParameter", 0x9003, -1);
	AddConstEx(id,"kPTPCommandVE_ReleaseStage", 0x9004, -1);
	AddConstEx(id,"kPTPCommandVE_OpenLESession", 0x9005, -1);
	AddConstEx(id,"kPTPCommandVE_CloseLESession", 0x9006, -1);
	AddConstEx(id,"kPTPCommandVE_RequestObjectTransferReady", 0x9007, -1);
	AddConstEx(id,"kPTPCommandVE_GetGeoTackingData", 0x9008, -1);
	AddConstEx(id,"kPTPCommandVE_OpenDebugSession", 0x900A, -1);
	AddConstEx(id,"kPTPCommandVE_CloseDebugSession", 0x900B, -1);
	AddConstEx(id,"kPTPCommandVE_GetDebugBuffer", 0x900C, -1);
	AddConstEx(id,"kPTPCommandVE_DebugCommandString", 0x900D, -1);
	AddConstEx(id,"kPTPCommandVE_GetDebugRoute", 0x900E, -1);
	AddConstEx(id,"kPTPCommandVE_SetIPTCData", 0x900F, -1);
	AddConstEx(id,"kPTPCommandVE_GetIPTCData", 0x9010, -1);
	AddConstEx(id,"kPTPCommandVE_Get3DAxisData", 0x9020, -1);
	AddConstEx(id,"kPTPCommandVE_OpenLiveViewSession", 0x9030, -1);
	AddConstEx(id,"kPTPCommandVE_CloseLiveViewSession", 0x9031, -1);
	AddConstEx(id,"kPTPCommandVE_9033", 0x9033, -1);
	AddConstEx(id,"kPTPCommandProd_OpenProductionSession", 0x9100, -1);
	AddConstEx(id,"kPTPCommandProd_CloseProductionSession", 0x9101, -1);
	AddConstEx(id,"kPTPCommandProd_UpdateFirmware", 0x9102, -1);
	AddConstEx(id,"kPTPCommandProd_OpenOSDSession", 0x9103, -1);
	AddConstEx(id,"kPTPCommandProd_CloseOSDSession", 0x9104, -1);
	AddConstEx(id,"kPTPCommandProd_GetOSDData", 0x9105, -1);
	AddConstEx(id,"kPTPCommandProd_GetFirmwareStruct", 0x9106, -1);
	AddConstEx(id,"kPTPCommandProd_GetDebugMenu", 0x910B, -1);
	AddConstEx(id,"kPTPCommandProd_SetDebugMenu", 0x910C, -1);
	AddConstEx(id,"kPTPCommandProd_ODINMessage", 0x910D, -1);
	AddConstEx(id,"kPTPCommandProd_GetDebugObjectHandles", 0x910E, -1);
	AddConstEx(id,"kPTPCommandProd_GetDebugObject", 0x910F, -1);
	AddConstEx(id,"kPTPCommandProd_DeleteDebugObject", 0x9110, -1);
	AddConstEx(id,"kPTPCommandProd_GetDebugObjectInfo", 0x9111, -1);
	AddConstEx(id,"kPTPCommandProd_WriteDebugObject", 0x9112, -1);
	AddConstEx(id,"kPTPCommandProd_CreateDebugObject", 0x9113, -1);
	AddConstEx(id,"kPTPCommandProd_Calibrate3Daxis", 0x9114, -1);
	AddConstEx(id,"kPTPCommandProd_MagneticCalibration", 0x9115, -1);
	AddConstEx(id,"kPTPCommandProd_GetViewfinderData", 0x9116, -1);
	id = AddEnum(-1,"PTPResponse",0x1100000);
	AddConstEx(id, "Undefined", 0x2000, -1);
	AddConstEx(id, "OK", 0x2001, -1);
	AddConstEx(id, "GeneralError", 0x2002, -1);
	AddConstEx(id, "OperationNotSupported", 0x2005, -1);
	AddConstEx(id, "IncompleteTransfer", 0x2007, -1);
	AddConstEx(id, "DeviceBusy", 0x2019, -1);	
	return id;
}

//------------------------------------------------------------------------
// Information about enum types

static Enums(void) {
        auto id;
        BeginTypeUpdating(UTP_ENUM);
	id = Enums_0(id);
        EndTypeUpdating(UTP_ENUM);
}

static Structures_0(id) {
        auto mid;

	id = AddStrucEx(-1,"UIDescType0Header",0);
	id = AddStrucEx(-1,"UIDescType0Entry",0);
	id = AddStrucEx(-1,"UIDescType1Header",0);
	id = AddStrucEx(-1,"UIDescType1Entry",0);
	id = AddStrucEx(-1,"UIDescType2",0);
	id = AddStrucEx(-1,"UIDescType3",0);
	
	id = GetStrucIdByName("UIDescType0Header");
	mid = AddStrucMember(id,"address",	0,	0x20500400,	0XFFFFFFFF,	4,	0XFFFFFFFF,	0,	0x000002);
	mid = AddStrucMember(id,"entries",	0X4,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown",	0X6,	0x10000400,	-1,	2);
	
	id = GetStrucIdByName("UIDescType0Entry");
	mid = AddStrucMember(id,"unknown1",	0,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown2",	0X2,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"address",	0X4,	0x20500400,	0XFFFFFFFF,	4,	0XFFFFFFFF,	0,	0x000002);
	
	id = GetStrucIdByName("UIDescType1Header");
	mid = AddStrucMember(id,"address",	0,	0x20500400,	0XFFFFFFFF,	4,	0XFFFFFFFF,	0,	0x000002);
	mid = AddStrucMember(id,"entries",	0X4,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown",	0X6,	0x10000400,	-1,	2);
	
	id = GetStrucIdByName("UIDescType1Entry");
	mid = AddStrucMember(id,"unknown1",	0,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown2",	0X2,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"address",	0X4,	0x20500400,	0XFFFFFFFF,	4,	0XFFFFFFFF,	0,	0x000002);
	mid = AddStrucMember(id,"objects",	0X8,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown3",	0XA,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown4",	0XC,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown5",	0XE,	0x10000400,	-1,	2);
	
	id = GetStrucIdByName("UIDescType2");
	mid = AddStrucMember(id,"reg",	0,	0x20000400,	-1,	4);
	mid = AddStrucMember(id,"address",	0X4,	0x20500400,	0XFFFFFFFF,	4,	0XFFFFFFFF,	0,	0x000002);
	mid = AddStrucMember(id,"unknown1",	0X8,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown2",	0XA,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown3",	0XC,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown4",	0XE,	0x10000400,	-1,	2);
	
	id = GetStrucIdByName("UIDescType3");
	mid = AddStrucMember(id,"unknown1",	0,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"unknown2",	0X2,	0x10000400,	-1,	2);
	mid = AddStrucMember(id,"address",	0X4,	0x20500400,	0XFFFFFFFF,	4,	0XFFFFFFFF,	0,	0x000002);
	return id;
}

//------------------------------------------------------------------------
// Information about structure types

static Structures(void) {
        auto id;
        BeginTypeUpdating(UTP_STRUCT);
	id = Structures_0(id);
        EndTypeUpdating(UTP_STRUCT);
}

// End of file.
