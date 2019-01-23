
#include "fr.hpp"

#include <entry.hpp>
#include <srarea.hpp>

// The netnode helper.
// Using this node we will save current configuration information in the
// IDA database.
static netnode helper;

// FR registers names
static const char *RegNames[] = {

    // general purpose registers :

    "R0",
    "R1",
    "R2",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15",

    // coprocessor registers :

    "CR0",
    "CR1",
    "CR2",
    "CR3",
    "CR4",
    "CR5",
    "CR6",
    "CR7",
    "CR8",
    "CR9",
    "CR10",
    "CR11",
    "CR12",
    "cR13",
    "CR14",
    "CR15",

    // dedicated registers :

    "PC",        // program counter
    "PS",        // program status
    "TBR",       // table base register
    "RP",        // return pointer
    "SSP",       // system stack pointer
    "USP",       // user stack pointer
    "MDL",       // multiplication/division register (LOW)
    "MDH",       // multiplication/division register (HIGH)

    // system use dedicated registers
    "reserved6",
    "reserved7",
    "reserved8",
    "reserved9",
    "reserved10",
    "reserved11",
    "reserved12",
    "reserved13",
    "reserved14",
    "reserved15",

    // these 2 registers are required by the IDA kernel :

    "cs",
    "ds"
};

static size_t numports = 0;
static ioport_t *ports = NULL;
char device[MAXSTR] = "";

// include IO common routines (such as set_device_name, apply_config_file, etc..)
#include <iocommon.cpp>

inline static void idaapi choose_device(TView *[] = NULL, int = 0)
{
  char cfgfile[QMAXFILE];
  get_cfg_filename(cfgfile, sizeof(cfgfile));
  if ( choose_ioport_device(cfgfile, device, sizeof(device), NULL) )
    set_device_name(device, IORESP_NONE);
}

// returns a pointer to a ioport_t object if address was found in the config file.
// otherwise, returns NULL.
const ioport_t *find_sym(ea_t address)
{
  return find_ioport(ports, numports, address);
}

// The kernel event notifications
// Here you may take desired actions upon some kernel events
static int notify(processor_t::idp_notify msgid, ...)
{
    va_list va;
    va_start(va, msgid);

    // A well behavior processor module should call invoke_callbacks()
    // in his notify() function. If this function returns 0, then
    // the processor module should process the notification itself
    // Otherwise the code should be returned to the caller:

    int code = invoke_callbacks(HT_IDP, msgid, va);
    if ( code ) return code;

    switch ( msgid ) {
        case processor_t::init:
            inf.mf = 1;
            helper.create("$ fr");
        default:
            break;

        case processor_t::term:
            free_ioports(ports, numports);
            break;

        case processor_t::newfile:
            choose_device();
            set_device_name(device, IORESP_ALL);
            break;

        case processor_t::oldfile:
            {
              char buf[MAXSTR];
              if ( helper.supval(-1, buf, sizeof(buf)) > 0 )
                set_device_name(buf, IORESP_NONE);
            }
            break;
/*
		case processor_t::add_cref:
			{
				ea_t from   = va_arg(va, ea_t);
				ea_t to		= va_arg(va, ea_t);
				int ref	= va_arg(va, int);
				
				msg("add cref %d (%X -> %X)\n", ref, from, to);
			}
			break;
 
		case processor_t::del_cref:
			{
				ea_t from   = va_arg(va, ea_t);
				ea_t to		= va_arg(va, ea_t);
				int ref	= va_arg(va, int);
				
				msg("del cref %d (%X -> %X)\n", ref, from, to);
			}
			break;
*/			
        case processor_t::closebase:
        case processor_t::savebase:
            helper.supset(-1, device);
            break;
    }

    va_end(va);

    return(1);
}

const char *idaapi set_idp_options(
    const char *keyword,
    int /*value_type*/,
    const void * /*value*/ )
{
    if ( keyword != NULL )
        return IDPOPT_BADKEY;

    char cfgfile[QMAXFILE];
    get_cfg_filename(cfgfile, sizeof(cfgfile));
    if ( !choose_ioport_device(cfgfile, device, sizeof(device), NULL)
      && strcmp(device, NONEPROC) == 0 )
    {
      warning("No devices are defined in the configuration file %s", cfgfile);
    }
    else
    {
      set_device_name(device, IORESP_NONE);
    }
    return IDPOPT_OK;
}


//
//  GNU assembler for fujitsu FR
//

// gets a function's name
static const char *fr_get_func_name(func_t *pfn, char *buf, size_t bufsize)
{
  ea_t ea = pfn->startEA;
  char *const end = buf + bufsize;
  char *ptr = tag_addr(buf, end, ea);

  if ( get_demangled_name(BADADDR, ea,
         ptr, end-ptr,
         inf.long_demnames, DEMNAM_NAME, 0) )
    return buf;
  return NULL;
}

// prints function header
static void idaapi gnu_func_header(func_t *pfn)
{
  std_gen_func_header(pfn);

  char buf[MAXSTR];
  const char *name = fr_get_func_name(pfn, buf, sizeof(buf));
  if ( name != NULL )
  {
    if ( is_public_name(pfn->startEA) && ash.a_public != NULL )
      printf_line(inf.indent, COLSTR("%s %s", SCOLOR_ASMDIR), ash.a_public, name);
    printf_line(inf.indent, COLSTR(".type %s, @function", SCOLOR_ASMDIR), name);
    printf_line(0, COLSTR("%s:", SCOLOR_ASMDIR), name);
  }
}

// prints function footer
static void idaapi gnu_func_footer(func_t *pfn)
{
  char buf[MAXSTR];
  const char *name = fr_get_func_name(pfn, buf, sizeof(buf));
  if ( name != NULL )
    printf_line(inf.indent, COLSTR(".size %s, .-%s", SCOLOR_ASMDIR), name, name);
}

static asm_t gnu_asm = {
    AS_COLON	| // create colons after data names
    ASB_BINF0	| // bin 0110b format
	ASD_DECF0	| // dec 34 format
    ASO_OCTF0	| // oct 123o format
    ASH_HEXF3	| // hex 0x123 format
	//AS_ONEDUP	| // one array definition per line
	//AS_N2CHR	| // can't have 2 byte char consts
    AS_NCMAS,	  // no commas in ascii directives
    0,
    "GNU Assembler for the Fujitsu FR Family",
    0,
    NULL,         // no headers
    NULL,         // no bad instructions
    ".org",       // origin directive
    NULL,         // end directive
    ";",          // comment string
    '"',          // string delimiter
    '\'',         // char delimiter
    "\\\"'",      // special symbols in char and string constants
    ".ascii",     // ascii string directive
    ".byte",      // byte directive
    ".word",      // word directive
    ".long",      // dword  (4 bytes)
    NULL,         // qword  (8 bytes)
    NULL,         // oword  (16 bytes)
    ".float",     // float  (4 bytes)
    ".double",    // double (8 bytes)
    NULL,         // tbyte  (10/12 bytes)
    NULL,         // packed decimal real
    NULL,         // arrays (#h,#d,#v,#s(...)
    ".und %s",    // uninited arrays
    ".equ",       // Equ
    NULL,         // seg prefix
    NULL,         // checkarg_preline()
    NULL,         // checkarg_atomprefix()
    NULL,         // checkarg_operations()
    NULL,         // translation to use in character & string constants
    "$",          // current IP (instruction pointer) symbol in assembler
    gnu_func_header,     // func_header
    gnu_func_footer,     // func_footer
    ".globl",     // public
    NULL,         // weak
    NULL,         // extrn
    NULL,         // comm
    NULL,         // get_type_name
    ".align",     // align
    '(', ')',     // lbrace, rbrace
    "%",          // mod
    "&",          // and
    "|",          // or
    "^",          // xor
    "!",          // not
    "<<",         // shl
    ">>",         // shr
    NULL,         // sizeof
    0,            // flag2 ???
    NULL,         // comment close string
    NULL,         // low8 op
    NULL,         // high8 op
    NULL,         // low16 op
    NULL          // high16 op
};

//
// Supported assemblers :
//

static asm_t *asms[] = { &gnu_asm, NULL };

//
// Short and long name for our module
//

static const char *shnames[] = {
    "fr",
    NULL
};

static const char *lnames[] = {
    "Fujitsu FR 32-Bit Family",
    NULL
};

static uchar retcode_1[] = { 0x97, 0x20 };    // ret
static uchar retcode_2[] = { 0x9F, 0x20 };    // ret with delay shot
static uchar retcode_3[] = { 0x9F, 0x30 };    // reti

static bytes_t retcodes[] = {
    { sizeof(retcode_1), retcode_1 },
    { sizeof(retcode_2), retcode_2 },
    { sizeof(retcode_3), retcode_3 },
    { 0, NULL }                            // NULL terminated array
};

//-----------------------------------------------------------------------
//      Processor Definition
//-----------------------------------------------------------------------
processor_t LPH =
{
      IDP_INTERFACE_VERSION,// version
      PLFM_FR,              // id
      PR_RNAMESOK           // can use register names for byte names
      |PR_BINMEM,           // The module creates RAM/ROM segments for binary files
                            // (the kernel shouldn't ask the user about their sizes and addresses)
      8,                    // 8 bits in a byte for code segments
      8,                    // 8 bits in a byte for other segments

      shnames,              // array of short processor names
                            // the short names are used to specify the processor
                            // with the -p command line switch)
      lnames,               // array of long processor names
                            // the long names are used to build the processor type
                            // selection menu

      asms,                 // array of target assemblers

      notify,               // the kernel event notification callback

      header,               // generate the disassembly header
      footer,               // generate the disassembly footer

      gen_segm_header,      // generate a segment declaration (start of segment)
      std_gen_segm_footer,  // generate a segment footer (end of segment)

      NULL,                 // generate 'assume' directives

      ana,                  // analyze an instruction and fill the 'cmd' structure
      emu,                  // emulate an instruction

      out,                  // generate a text representation of an instruction
      outop,                // generate a text representation of an operand
      intel_data,           // generate a text representation of a data item
      NULL,                 // compare operands
      NULL,                 // can an operand have a type?

      qnumber(RegNames),    // Number of registers
      RegNames,             // Regsiter names
      NULL,                 // get abstract register

      0,                    // Number of register files
      NULL,                 // Register file names
      NULL,                 // Register descriptions
      NULL,                 // Pointer to CPU registers

      rVcs, rVds,
      0,                    // size of a segment register
      rVcs, rVds,

      NULL,                 // No known code start sequences
      retcodes,

      0, fr_last,
      Instructions,

      NULL,                 // int  (*is_far_jump)(int icode);
      NULL,                 // Translation function for offsets
      0,                    // int tbyte_size;  -- doesn't exist
      NULL,                 // int (*realcvt)(void *m, ushort *e, ushort swt);
      { 0, 7, 15, 0 },      // char real_width[4];
                            // number of symbols after decimal point
                            // 2byte float (0-does not exist)
                            // normal float
                            // normal double
                            // long double
      NULL,            // int (*is_switch)(switch_info_t *si);
      NULL,                 // int32 (*gen_map_file)(FILE *fp);
      NULL,                 // ea_t (*extract_address)(ea_t ea,const char *string,int x);
      is_sp_based,          // int (*is_sp_based)(op_t &x);
      create_func_frame,    // int (*create_func_frame)(func_t *pfn);
      NULL,                 // int (*get_frame_retsize(func_t *pfn)
      NULL,                 // void (*gen_stkvar_def)(char *buf,const member_t *mptr,int32 v);
      gen_spcdef,           // Generate text representation of an item in a special segment
      fr_ret,               // Icode of return instruction. It is ok to give any of possible return instructions
      set_idp_options,      // const char *(*set_idp_options)(const char *keyword,int value_type,const void *value);
      is_align_insn,        // int (*is_align_insn)(ea_t ea);
      NULL                  // mvm_t *mvm;
};
