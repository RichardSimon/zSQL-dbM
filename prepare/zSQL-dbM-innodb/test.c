//gcc test.c -I./include2 -DSTACK_DIRECTION=1
//gcc test.c -I./include2 -DSTACK_DIRECTION=1 -w
//gcc test.c -I./include2 -DSTACK_DIRECTION=1 -w -g
//include <stdio.h> //-2011-10-08-18-23移到main函数前！！！；；；;

#define HAVE_MKSTEMP  1 //stdlib.h
//
#define SIZEOF_INT  4 //下面的my_global.h用到;
#include "my_global.h" //=>File等;
#include "my_dbug.h"
//InnoSQL-5.5.8/dbug/dbug.c:
void _db_enter_(const char *_func_, const char *_file_,
                uint _line_, struct _db_stack_frame_ *_stack_frame_)
{
}
void _db_pargs_(uint _line_, const char *keyword)
{
}
#include <stdarg.h>
void _db_doprnt_(const char *format,...)
{
}
//include2/m_string.h //InnoSQL-5.5.8/strings/strmov.c:
char *strmov(register char *dst, register const char *src)
{
  while ((*dst++ = *src++)) ;
  return dst-1;
}
//
//extern int my_errno;  /* Last error in mysys */ //include2/my_sys.h //InnoSQL-5.5.8/mysys/my_static.c
int		my_errno=0; //InnoSQL-5.5.8/mysys/my_static.c
//
#define MY_WME		16	/* Write message on error */ //include2/my_sys.h
#define ME_NOINPUT	128	/* Dont use the input libary */ //include2/my_sys.h
enum file_type
{
  UNOPEN = 0, FILE_BY_OPEN, FILE_BY_CREATE, STREAM_BY_FOPEN, STREAM_BY_FDOPEN,
  FILE_BY_MKSTEMP, FILE_BY_DUP
}; //include2/my_sys.h
//
#include "mysys_err.h" //EE_CANTCREATEFILE
//
//extern ulong	my_file_opened,my_stream_opened, my_tmp_file_created; //include2/my_sys.h
ulong		my_stream_opened=0,my_file_opened=0, my_tmp_file_created=0;  //InnoSQL-5.5.8/mysys/my_static.c  //(15-22。。(douban))----2011-09-22-15-55--16-03;
// #include "my_sys.h" //同时包含了"my_global.h"  //=>导致太多的编译错误！！；----2011-09-22-13-25;
//+(链接错从而需要增加:)
char *strnmov(register char *dst, register const char *src, size_t n)
{
  while (n-- != 0) {
    if (!(*dst++ = *src++)) {
      return (char*) dst-1;
    }
  }
  return dst;
} //InnoSQL-5.5.8/strings/strnmov.c
//
#define TRACE_ON        ((uint)1 << 31)  /* Trace enabled. MUST be the highest bit!*/
//-
/*
 *      The user may specify a list of functions to trace or
 *      debug.  These lists are kept in a linear linked list,
 *      a very simple implementation.
 */
struct link {
    struct link *next_link;   /* Pointer to the next link */
    char   flags;
    char   str[1];            /* Pointer to link's contents */
};
/*
 *      Debugging settings can be pushed or popped off of a
 *      stack which is implemented as a linked list.  Note
 *      that the head of the list is the current settings and the
 *      stack is pushed by adding a new settings to the head of the
 *      list or popped by removing the first link.
 *
 *      Note: if out_file is NULL, the other fields are not initialized at all!
 */
struct settings {
  uint flags;                   /* Current settings flags               */
  uint maxdepth;                /* Current maximum trace depth          */
  uint delay;                   /* Delay after each output line         */
  uint sub_level;               /* Sub this from code_state->level      */
  FILE *out_file;               /* Current output stream                */
  FILE *prof_file;              /* Current profiling stream             */
  char name[FN_REFLEN];         /* Name of output file                  */
  struct link *functions;       /* List of functions                    */
  struct link *p_functions;     /* List of profiled functions           */
  struct link *keywords;        /* List of debug keywords               */
  struct link *processes;       /* List of process names                */
  struct settings *next;        /* Next settings in the list            */
};
typedef struct _db_code_state_ {
  const char *process;          /* Pointer to process name; usually argv[0] */
  const char *func;             /* Name of current user function            */
  const char *file;             /* Name of current user file                */
  struct _db_stack_frame_ *framep; /* Pointer to current frame              */  //=>my_dbug.h
  struct settings *stack;       /* debugging settings                       */
  const char *jmpfunc;          /* Remember current function for setjmp     */
  const char *jmpfile;          /* Remember current file for setjmp         */
  int lineno;                   /* Current debugger output line number      */
  uint level;                   /* Current function nesting level           */
  int jmplevel;                 /* Remember nesting level at setjmp()       */

/*
 *      The following variables are used to hold the state information
 *      between the call to _db_pargs_() and _db_doprnt_(), during
 *      expansion of the DBUG_PRINT macro.  This is the only macro
 *      that currently uses these variables.
 *
 *      These variables are currently used only by _db_pargs_() and
 *      _db_doprnt_().
 */

  uint u_line;                  /* User source code line number */
  int  locked;                  /* If locked with _db_lock_file_ */
  const char *u_keyword;        /* Keyword for current macro */
} CODE_STATE;
//-
#define BOOLEAN my_bool
static BOOLEAN init_done= FALSE; /* Set to TRUE when initialization done */
static struct settings init_settings;
#define OPEN_APPEND     (1 << 11)  /* Open for append      */
//#ifdef THREAD
//#include <my_pthread.h>
//static pthread_mutex_t THR_LOCK_dbug;
//
//static CODE_STATE *code_state(void)
//{
//  CODE_STATE *cs, **cs_ptr;
//
//  /*
//    _dbug_on_ is reset if we don't plan to use any debug commands at all and
//    we want to run on maximum speed
//   */
//  if (!_dbug_on_)
//    return 0;
//
//  if (!init_done)
//  {
//    init_done=TRUE;
//    pthread_mutex_init(&THR_LOCK_dbug, NULL);
//    bzero(&init_settings, sizeof(init_settings));
//    init_settings.out_file=stderr;
//    init_settings.flags=OPEN_APPEND;
//  }
//
//  if (!(cs_ptr= (CODE_STATE**) my_thread_var_dbug()))
//    return 0;                                   /* Thread not initialised */
//  if (!(cs= *cs_ptr))
//  {
//    cs=(CODE_STATE*) DbugMalloc(sizeof(*cs));
//    bzero((uchar*) cs,sizeof(*cs));
//    cs->process= db_process ? db_process : "dbug";
//    cs->func="?func";
//    cs->file="?file";
//    cs->stack=&init_settings;
//    *cs_ptr= cs;
//  }
//  return cs;
//}
//
//#else /* !THREAD */
//
static CODE_STATE static_code_state=
{
  "dbug", "?func", "?file", NULL, &init_settings,
  NullS, NullS, 0,0,0,0,0,NullS
};

static CODE_STATE *code_state(void)
{
  if (!init_done)
  {
    bzero(&init_settings, sizeof(init_settings));
    init_settings.out_file=stderr;
    init_settings.flags=OPEN_APPEND;
    init_done=TRUE;
  }
  return &static_code_state;
}
//
//#define pthread_mutex_lock(A) {}
//#define pthread_mutex_unlock(A) {}
//#endif
#define get_code_state_or_return if (!((cs=code_state()))) return
//-
#define ERR_MISSING_RETURN "missing DBUG_RETURN or DBUG_VOID_RETURN macro in function \"%s\"\n"
#define PROF_XFMT       "X\t%ld\t%s\n"
#define DO_TRACE        1
#define TRACING (cs->stack->flags & TRACE_ON)
static pthread_mutex_t THR_LOCK_dbug;
//-
//#include "ctype-latin1.c" //InnoSQL-5.5.8/strings/ctype-latin1.c  //zlq
#include "m_ctype.h" //"} CHARSET_INFO;"
//[
static uchar ctype_latin1[] = {
    0,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 40, 40, 40, 32, 32,
   32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
   72, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
  132,132,132,132,132,132,132,132,132,132, 16, 16, 16, 16, 16, 16,
   16,129,129,129,129,129,129,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 16, 16, 16, 16, 16,
   16,130,130,130,130,130,130,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 16, 16, 16, 16, 32,
   16,  0, 16,  2, 16, 16, 16, 16, 16, 16,  1, 16,  1,  0,  1,  0,
    0, 16, 16, 16, 16, 16, 16, 16, 16, 16,  2, 16,  2,  0,  2,  1,
   72, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
   16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1, 16,  1,  1,  1,  1,  1,  1,  1,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2, 16,  2,  2,  2,  2,  2,  2,  2,  2
};
static uchar to_lower_latin1[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   64, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122, 91, 92, 93, 94, 95,
   96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
  112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
  224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
  240,241,242,243,244,245,246,215,248,249,250,251,252,253,254,223,
  224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
  240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};
static uchar to_upper_latin1[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
   96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
  208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
  192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
  208,209,210,211,212,213,214,247,216,217,218,219,220,221,222,255
};
static uchar sort_order_latin1[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
   32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
   48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
   64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
   96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
   80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
  128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
  144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
  160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
  176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
   65, 65, 65, 65, 92, 91, 92, 67, 69, 69, 69, 69, 73, 73, 73, 73,
   68, 78, 79, 79, 79, 79, 93,215,216, 85, 85, 85, 89, 89,222,223,
   65, 65, 65, 65, 92, 91, 92, 67, 69, 69, 69, 69, 73, 73, 73, 73,
   68, 78, 79, 79, 79, 79, 93,247,216, 85, 85, 85, 89, 89,222,255
};
unsigned short cs_to_uni[256]={
0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,
0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,
0x0018,0x0019,0x001A,0x001B,0x001C,0x001D,0x001E,0x001F,
0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,
0x0028,0x0029,0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,
0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,
0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,
0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,
0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,
0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,
0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,
0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,
0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,
0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,
0x0078,0x0079,0x007A,0x007B,0x007C,0x007D,0x007E,0x007F,
0x20AC,0x0081,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,
0x02C6,0x2030,0x0160,0x2039,0x0152,0x008D,0x017D,0x008F,
0x0090,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
0x02DC,0x2122,0x0161,0x203A,0x0153,0x009D,0x017E,0x0178,
0x00A0,0x00A1,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,
0x00A8,0x00A9,0x00AA,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,
0x00B8,0x00B9,0x00BA,0x00BB,0x00BC,0x00BD,0x00BE,0x00BF,
0x00C0,0x00C1,0x00C2,0x00C3,0x00C4,0x00C5,0x00C6,0x00C7,
0x00C8,0x00C9,0x00CA,0x00CB,0x00CC,0x00CD,0x00CE,0x00CF,
0x00D0,0x00D1,0x00D2,0x00D3,0x00D4,0x00D5,0x00D6,0x00D7,
0x00D8,0x00D9,0x00DA,0x00DB,0x00DC,0x00DD,0x00DE,0x00DF,
0x00E0,0x00E1,0x00E2,0x00E3,0x00E4,0x00E5,0x00E6,0x00E7,
0x00E8,0x00E9,0x00EA,0x00EB,0x00EC,0x00ED,0x00EE,0x00EF,
0x00F0,0x00F1,0x00F2,0x00F3,0x00F4,0x00F5,0x00F6,0x00F7,
0x00F8,0x00F9,0x00FA,0x00FB,0x00FC,0x00FD,0x00FE,0x00FF
};
//[
uchar pl00[256]={
0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x00,0x81,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x8D,0x00,0x8F,
0x90,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x9D,0x00,0x00,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF
};
uchar pl01[256]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x8C,0x9C,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x8A,0x9A,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x9F,0x00,0x00,0x00,0x00,0x8E,0x9E,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x83,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
uchar pl02[256]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x88,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x98,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
uchar pl20[256]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x96,0x97,0x00,0x00,0x00,
0x91,0x92,0x82,0x00,0x93,0x94,0x84,0x00,
0x86,0x87,0x95,0x00,0x00,0x00,0x85,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x89,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x8B,0x9B,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
uchar pl21[256]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x99,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
uchar *uni_to_cs[256]={
pl00,pl01,pl02,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
pl20,pl21,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL
};
static
int my_mb_wc_latin1(CHARSET_INFO *cs  __attribute__((unused)),
		    my_wc_t *wc,
		    const uchar *str,
		    const uchar *end __attribute__((unused)))
{
  if (str >= end)
    return MY_CS_TOOSMALL;

  *wc=cs_to_uni[*str];
  return (!wc[0] && str[0]) ? -1 : 1;
}
static
int my_wc_mb_latin1(CHARSET_INFO *cs  __attribute__((unused)),
		    my_wc_t wc,
		    uchar *str,
		    uchar *end __attribute__((unused)))
{
  uchar *pl;

  if (str >= end)
    return MY_CS_TOOSMALL;

  pl= uni_to_cs[(wc>>8) & 0xFF];
  str[0]= pl ? pl[wc & 0xFF] : '\0';
  return (!str[0] && wc) ? MY_CS_ILUNI : 1;
}
//[[
uint my_mbcharlen_8bit(CHARSET_INFO *cs __attribute__((unused)),
                      uint c __attribute__((unused)))
{
  return 1;
} //=>InnoSQL-5.5.8/strings/ctype-bin.c  //+----2011-09-22-16-04--;
//[[[
size_t my_numchars_8bit(CHARSET_INFO *cs __attribute__((unused)),
		      const char *b, const char *e)
{
  return (size_t) (e - b);
}
size_t my_charpos_8bit(CHARSET_INFO *cs __attribute__((unused)),
                       const char *b  __attribute__((unused)),
                       const char *e  __attribute__((unused)),
                       size_t pos)
{
  return pos;
}
size_t my_well_formed_len_8bit(CHARSET_INFO *cs __attribute__((unused)),
                               const char *start, const char *end,
                               size_t nchars, int *error)
{
  size_t nbytes= (size_t) (end-start);
  *error= 0;
  return min(nbytes, nchars);
}
//[
/* SPACE_INT is a word that contains only spaces */
#if SIZEOF_INT == 4
#define SPACE_INT 0x20202020
#elif SIZEOF_INT == 8
#define SPACE_INT 0x2020202020202020
#else
#error define the appropriate constant for a word full of spaces
#endif
static inline const uchar *skip_trailing_space(const uchar *ptr,size_t len)
{
  const uchar *end= ptr + len;

  if (len > 20)
  {
    const uchar *end_words= (const uchar *)(intptr)
      (((ulonglong)(intptr)end) / SIZEOF_INT * SIZEOF_INT);
    const uchar *start_words= (const uchar *)(intptr)
       ((((ulonglong)(intptr)ptr) + SIZEOF_INT - 1) / SIZEOF_INT * SIZEOF_INT);

    DBUG_ASSERT(((ulonglong)(intptr)ptr) >= SIZEOF_INT);
    if (end_words > ptr)
    {
      while (end > end_words && end[-1] == 0x20)
        end--;
      if (end[-1] == 0x20 && start_words < end_words)
        while (end > start_words && ((unsigned *)end)[-1] == SPACE_INT)
          end -= SIZEOF_INT;
    }
  }
  while (end > ptr && end[-1] == 0x20)
    end--;
  return (end);
}
//]//=>m_string.h
size_t my_lengthsp_8bit(CHARSET_INFO *cs __attribute__((unused)),
                        const char *ptr, size_t length)
{
  const char *end;
  end= (const char *) skip_trailing_space((const uchar *)ptr, length);
  return (size_t) (end-ptr);
}
size_t my_numcells_8bit(CHARSET_INFO *cs __attribute__((unused)),
                        const char *b, const char *e)
{
  return (size_t) (e - b);
}
int my_mb_wc_8bit(CHARSET_INFO *cs,my_wc_t *wc,
		  const uchar *str,
		  const uchar *end __attribute__((unused)))
{
  if (str >= end)
    return MY_CS_TOOSMALL;

  *wc=cs->tab_to_uni[*str];
  return (!wc[0] && str[0]) ? -1 : 1;
}
int my_wc_mb_8bit(CHARSET_INFO *cs,my_wc_t wc,
		  uchar *str,
		  uchar *end)
{
  MY_UNI_IDX *idx;

  if (str >= end)
    return MY_CS_TOOSMALL;

  for (idx=cs->tab_from_uni; idx->tab ; idx++)
  {
    if (idx->from <= wc && idx->to >= wc)
    {
      str[0]= idx->tab[wc - idx->from];
      return (!str[0] && wc) ? MY_CS_ILUNI : 1;
    }
  }
  return MY_CS_ILUNI;
}
int my_mb_ctype_8bit(CHARSET_INFO *cs, int *ctype,
                   const uchar *s, const uchar *e)
{
  if (s >= e)
  {
    *ctype= 0;
    return MY_CS_TOOSMALL;
  }
  *ctype= cs->ctype[*s + 1];
  return 1;
}
size_t my_caseup_str_8bit(CHARSET_INFO * cs,char *str)
{
  register uchar *map= cs->to_upper;
  char *str_orig= str;
  while ((*str= (char) map[(uchar) *str]) != 0)
    str++;
  return (size_t) (str - str_orig);
}
size_t my_casedn_str_8bit(CHARSET_INFO * cs,char *str)
{
  register uchar *map= cs->to_lower;
  char *str_orig= str;
  while ((*str= (char) map[(uchar) *str]) != 0)
    str++;
  return (size_t) (str - str_orig);
}
size_t my_caseup_8bit(CHARSET_INFO * cs, char *src, size_t srclen,
                      char *dst __attribute__((unused)),
                      size_t dstlen __attribute__((unused)))
{
  char *end= src + srclen;
  register uchar *map= cs->to_upper;
  DBUG_ASSERT(src == dst && srclen == dstlen);
  for ( ; src != end ; src++)
    *src= (char) map[(uchar) *src];
  return srclen;
}
size_t my_casedn_8bit(CHARSET_INFO * cs, char *src, size_t srclen,
                      char *dst __attribute__((unused)),
                      size_t dstlen __attribute__((unused)))
{
  char *end= src + srclen;
  register uchar *map=cs->to_lower;
  DBUG_ASSERT(src == dst && srclen == dstlen);
  for ( ; src != end ; src++)
    *src= (char) map[(uchar) *src];
  return srclen;
}
size_t my_vsnprintf(char *to, size_t n, const char* fmt, va_list ap); //z+  //----2011-09-22-16-49; //zlq
size_t my_snprintf_8bit(CHARSET_INFO *cs  __attribute__((unused)),
                        char* to, size_t n  __attribute__((unused)),
		     const char* fmt, ...)
{
  va_list args;
  int result;
  va_start(args,fmt);
  result= my_vsnprintf(to, n, fmt, args);
  va_end(args);
  return result;
}
size_t my_long10_to_str_8bit(CHARSET_INFO *cs __attribute__((unused)),
                             char *dst, size_t len, int radix, long int val)
{
  char buffer[66];
  register char *p, *e;
  long int new_val;
  uint sign=0;
  unsigned long int uval = (unsigned long int) val;

  e = p = &buffer[sizeof(buffer)-1];
  *p= 0;

  if (radix < 0)
  {
    if (val < 0)
    {
      /* Avoid integer overflow in (-val) for LONGLONG_MIN (BUG#31799). */
      uval= (unsigned long int)0 - uval;
      *dst++= '-';
      len--;
      sign= 1;
    }
  }

  new_val = (long) (uval / 10);
  *--p    = '0'+ (char) (uval - (unsigned long) new_val * 10);
  val     = new_val;

  while (val != 0)
  {
    new_val=val/10;
    *--p = '0' + (char) (val-new_val*10);
    val= new_val;
  }

  len= min(len, (size_t) (e-p));
  memcpy(dst, p, len);
  return len+sign;
}
//[//z+  //2011-10-08
#define LONG_MIN	((long) 0x80000000L)
#define LONG_MAX	((long) 0x7FFFFFFFL)
//]
//[my_global.h:  //2011-10-08
/* This is from the old m-machine.h file */

#if SIZEOF_LONG_LONG > 4
#define HAVE_LONG_LONG 1
#endif

/*
  Some pre-ANSI-C99 systems like AIX 5.1 and Linux/GCC 2.95 define
  ULONGLONG_MAX, LONGLONG_MIN, LONGLONG_MAX; we use them if they're defined.
*/

#if defined(HAVE_LONG_LONG) && !defined(LONGLONG_MIN)
#define LONGLONG_MIN	((long long) 0x8000000000000000LL)
#define LONGLONG_MAX	((long long) 0x7FFFFFFFFFFFFFFFLL)
#endif

#if defined(HAVE_LONG_LONG) && !defined(ULONGLONG_MAX)
/* First check for ANSI C99 definition: */
#ifdef ULLONG_MAX
#define ULONGLONG_MAX  ULLONG_MAX
#else
#define ULONGLONG_MAX ((unsigned long long)(~0ULL))
#endif
#endif /* defined (HAVE_LONG_LONG) && !defined(ULONGLONG_MAX)*/

#define INT_MIN64       (~0x7FFFFFFFFFFFFFFFLL)
#define INT_MAX64       0x7FFFFFFFFFFFFFFFLL
#define INT_MIN32       (~0x7FFFFFFFL)
#define INT_MAX32       0x7FFFFFFFL
#define UINT_MAX32      0xFFFFFFFFL
#define INT_MIN24       (~0x007FFFFF)
#define INT_MAX24       0x007FFFFF
#define UINT_MAX24      0x00FFFFFF
#define INT_MIN16       (~0x7FFF)
#define INT_MAX16       0x7FFF
#define UINT_MAX16      0xFFFF
#define INT_MIN8        (~0x7F)
#define INT_MAX8        0x7F
#define UINT_MAX8       0xFF

/* From limits.h instead */
#ifndef DBL_MIN
#define DBL_MIN		4.94065645841246544e-324
#define FLT_MIN		((float)1.40129846432481707e-45)
#endif
#ifndef DBL_MAX
#define DBL_MAX		1.79769313486231470e+308
#define FLT_MAX		((float)3.40282346638528860e+38)
#endif
#ifndef SIZE_T_MAX
#define SIZE_T_MAX      (~((size_t) 0))
#endif

#ifndef isfinite
#ifdef HAVE_FINITE
#define isfinite(x) finite(x)
#else
#define finite(x) (1.0 / fabs(x) > 0.0)
#endif /* HAVE_FINITE */
#endif /* isfinite */

#ifndef HAVE_ISNAN
#define isnan(x) ((x) != (x))
#endif

#ifdef HAVE_ISINF
/* Check if C compiler is affected by GCC bug #39228 */
#if !defined(__cplusplus) && defined(HAVE_BROKEN_ISINF)
/* Force store/reload of the argument to/from a 64-bit double */
static inline double my_isinf(double x)
{
  volatile double t= x;
  return isinf(t);
}
#else
/* System-provided isinf() is available and safe to use */
#define my_isinf(X) isinf(X)
#endif
#else /* !HAVE_ISINF */
#define my_isinf(X) (!finite(X) && !isnan(X))
#endif

/* Define missing math constants. */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.7182818284590452354
#endif
#ifndef M_LN2
#define M_LN2 0.69314718055994530942
#endif
//]
size_t my_longlong10_to_str_8bit(CHARSET_INFO *cs __attribute__((unused)),
                                 char *dst, size_t len, int radix,
                                 longlong val)
{
  char buffer[65];
  register char *p, *e;
  long long_val;
  uint sign= 0;
  ulonglong uval = (ulonglong)val;

  if (radix < 0)
  {
    if (val < 0)
    {
      /* Avoid integer overflow in (-val) for LONGLONG_MIN (BUG#31799). */
      uval = (ulonglong)0 - uval;
      *dst++= '-';
      len--;
      sign= 1;
    }
  }

  e = p = &buffer[sizeof(buffer)-1];
  *p= 0;

  if (uval == 0)
  {
    *--p= '0';
    len= 1;
    goto cnv;
  }

  while (uval > (ulonglong) LONG_MAX)
  {
    ulonglong quo= uval/(uint) 10;
    uint rem= (uint) (uval- quo* (uint) 10);
    *--p = '0' + rem;
    uval= quo;
  }

  long_val= (long) uval;
  while (long_val != 0)
  {
    long quo= long_val/10;
    *--p = (char) ('0' + (long_val - quo*10));
    long_val= quo;
  }

  len= min(len, (size_t) (e-p));
cnv:
  memcpy(dst, p, len);
  return len+sign;
}
//[m_string.h:  //2011-10-08
//./configure.cmake:CHECK_FUNCTION_EXISTS (bfill HAVE_BFILL)
#if !defined(HAVE_BFILL)
# define bfill(A,B,C)           memset((A),(C),(B))
#endif
//]
void my_fill_8bit(CHARSET_INFO *cs __attribute__((unused)),
		   char *s, size_t l, int fill)
{
  bfill((uchar*) s,l,fill);
}
long my_strntol_8bit(CHARSET_INFO *cs,
		     const char *nptr, size_t l, int base,
		     char **endptr, int *err)
{
  int negative;
  register uint32 cutoff;
  register uint cutlim;
  register uint32 i;
  register const char *s;
  register uchar c;
  const char *save, *e;
  int overflow;

  *err= 0;				/* Initialize error indicator */

  s = nptr;
  e = nptr+l;

  for ( ; s<e && my_isspace(cs, *s) ; s++);

  if (s == e)
  {
    goto noconv;
  }

  /* Check for a sign.	*/
  if (*s == '-')
  {
    negative = 1;
    ++s;
  }
  else if (*s == '+')
  {
    negative = 0;
    ++s;
  }
  else
    negative = 0;

  save = s;
  cutoff = ((uint32)~0L) / (uint32) base;
  cutlim = (uint) (((uint32)~0L) % (uint32) base);

  overflow = 0;
  i = 0;
  for (c = *s; s != e; c = *++s)
  {
    if (c>='0' && c<='9')
      c -= '0';
    else if (c>='A' && c<='Z')
      c = c - 'A' + 10;
    else if (c>='a' && c<='z')
      c = c - 'a' + 10;
    else
      break;
    if (c >= base)
      break;
    if (i > cutoff || (i == cutoff && c > cutlim))
      overflow = 1;
    else
    {
      i *= (uint32) base;
      i += c;
    }
  }

  if (s == save)
    goto noconv;

  if (endptr != NULL)
    *endptr = (char *) s;

  if (negative)
  {
    if (i  > (uint32) INT_MIN32)
      overflow = 1;
  }
  else if (i > INT_MAX32)
    overflow = 1;

  if (overflow)
  {
    err[0]= ERANGE;
    return negative ? INT_MIN32 : INT_MAX32;
  }

  return (negative ? -((long) i) : (long) i);

noconv:
  err[0]= EDOM;
  if (endptr != NULL)
    *endptr = (char *) nptr;
  return 0L;
}
ulong my_strntoul_8bit(CHARSET_INFO *cs,
		       const char *nptr, size_t l, int base,
		       char **endptr, int *err)
{
  int negative;
  register uint32 cutoff;
  register uint cutlim;
  register uint32 i;
  register const char *s;
  register uchar c;
  const char *save, *e;
  int overflow;

  *err= 0;				/* Initialize error indicator */

  s = nptr;
  e = nptr+l;

  for( ; s<e && my_isspace(cs, *s); s++);

  if (s==e)
  {
    goto noconv;
  }

  if (*s == '-')
  {
    negative = 1;
    ++s;
  }
  else if (*s == '+')
  {
    negative = 0;
    ++s;
  }
  else
    negative = 0;

  save = s;
  cutoff = ((uint32)~0L) / (uint32) base;
  cutlim = (uint) (((uint32)~0L) % (uint32) base);
  overflow = 0;
  i = 0;

  for (c = *s; s != e; c = *++s)
  {
    if (c>='0' && c<='9')
      c -= '0';
    else if (c>='A' && c<='Z')
      c = c - 'A' + 10;
    else if (c>='a' && c<='z')
      c = c - 'a' + 10;
    else
      break;
    if (c >= base)
      break;
    if (i > cutoff || (i == cutoff && c > cutlim))
      overflow = 1;
    else
    {
      i *= (uint32) base;
      i += c;
    }
  }

  if (s == save)
    goto noconv;

  if (endptr != NULL)
    *endptr = (char *) s;

  if (overflow)
  {
    err[0]= ERANGE;
    return (~(uint32) 0);
  }

  return (negative ? -((long) i) : (long) i);

noconv:
  err[0]= EDOM;
  if (endptr != NULL)
    *endptr = (char *) nptr;
  return 0L;
}
#define LONGLONG_MIN	((long long) 0x8000000000000000LL) //=>my_global.h
#define LONGLONG_MAX	((long long) 0x7FFFFFFFFFFFFFFFLL) //=>my_global.h
#define ULONGLONG_MAX ((unsigned long long)(~0ULL)) //=>my_global.h
longlong my_strntoll_8bit(CHARSET_INFO *cs __attribute__((unused)),
			  const char *nptr, size_t l, int base,
			  char **endptr,int *err)
{
  int negative;
  register ulonglong cutoff;
  register uint cutlim;
  register ulonglong i;
  register const char *s, *e;
  const char *save;
  int overflow;

  *err= 0;				/* Initialize error indicator */

  s = nptr;
  e = nptr+l;

  for(; s<e && my_isspace(cs,*s); s++);

  if (s == e)
  {
    goto noconv;
  }

  if (*s == '-')
  {
    negative = 1;
    ++s;
  }
  else if (*s == '+')
  {
    negative = 0;
    ++s;
  }
  else
    negative = 0;

  save = s;

  cutoff = (~(ulonglong) 0) / (unsigned long int) base;
  cutlim = (uint) ((~(ulonglong) 0) % (unsigned long int) base);

  overflow = 0;
  i = 0;
  for ( ; s != e; s++)
  {
    register uchar c= *s;
    if (c>='0' && c<='9')
      c -= '0';
    else if (c>='A' && c<='Z')
      c = c - 'A' + 10;
    else if (c>='a' && c<='z')
      c = c - 'a' + 10;
    else
      break;
    if (c >= base)
      break;
    if (i > cutoff || (i == cutoff && c > cutlim))
      overflow = 1;
    else
    {
      i *= (ulonglong) base;
      i += c;
    }
  }

  if (s == save)
    goto noconv;

  if (endptr != NULL)
    *endptr = (char *) s;

  if (negative)
  {
    if (i  > (ulonglong) LONGLONG_MIN)
      overflow = 1;
  }
  else if (i > (ulonglong) LONGLONG_MAX)
    overflow = 1;

  if (overflow)
  {
    err[0]= ERANGE;
    return negative ? LONGLONG_MIN : LONGLONG_MAX;
  }

  return (negative ? -((longlong) i) : (longlong) i);

noconv:
  err[0]= EDOM;
  if (endptr != NULL)
    *endptr = (char *) nptr;
  return 0L;
}
ulonglong my_strntoull_8bit(CHARSET_INFO *cs,
			   const char *nptr, size_t l, int base,
			   char **endptr, int *err)
{
  int negative;
  register ulonglong cutoff;
  register uint cutlim;
  register ulonglong i;
  register const char *s, *e;
  const char *save;
  int overflow;

  *err= 0;				/* Initialize error indicator */

  s = nptr;
  e = nptr+l;

  for(; s<e && my_isspace(cs,*s); s++);

  if (s == e)
  {
    goto noconv;
  }

  if (*s == '-')
  {
    negative = 1;
    ++s;
  }
  else if (*s == '+')
  {
    negative = 0;
    ++s;
  }
  else
    negative = 0;

  save = s;

  cutoff = (~(ulonglong) 0) / (unsigned long int) base;
  cutlim = (uint) ((~(ulonglong) 0) % (unsigned long int) base);

  overflow = 0;
  i = 0;
  for ( ; s != e; s++)
  {
    register uchar c= *s;

    if (c>='0' && c<='9')
      c -= '0';
    else if (c>='A' && c<='Z')
      c = c - 'A' + 10;
    else if (c>='a' && c<='z')
      c = c - 'a' + 10;
    else
      break;
    if (c >= base)
      break;
    if (i > cutoff || (i == cutoff && c > cutlim))
      overflow = 1;
    else
    {
      i *= (ulonglong) base;
      i += c;
    }
  }

  if (s == save)
    goto noconv;

  if (endptr != NULL)
    *endptr = (char *) s;

  if (overflow)
  {
    err[0]= ERANGE;
    return (~(ulonglong) 0);
  }

  return (negative ? -((longlong) i) : (longlong) i);

noconv:
  err[0]= EDOM;
  if (endptr != NULL)
    *endptr = (char *) nptr;
  return 0L;
}
//[strings/dtoa.c:  //2011-10-08
//my_fcvt: Converts a given floating point number to a zero-terminated string representation using the 'f' format.
////[[strings/t_ctype.h:
//#define TOT_LEVELS 5
//enum level_symbols {
//    L_UPRUPR = TOT_LEVELS,
//    L_UPPER,
//    L_MIDDLE,
//    L_LOWER
//};
////]]
////[[strings/ctype-tis620.c:  //2011-10-08
//#define M  L_MIDDLE
//#define U  L_UPPER
//#define L  L_LOWER
//#define UU L_UPRUPR
//#define X  L_MIDDLE
////]]
typedef int32 Long;
typedef uint32 ULong;
typedef int64 LLong;
typedef uint64 ULLong;
typedef union { double d; ULong L[2]; } U;
typedef struct Bigint
{
  union {
    ULong *x;              /* points right after this Bigint object */
    struct Bigint *next;   /* to maintain free lists */
  } p;
  int k;                   /* 2^k = maxwds */
  int maxwds;              /* maximum length in 32-bit words */
  int sign;                /* not zero if number is negative */
  int wds;                 /* current length in 32-bit words */
} Bigint;
#define Kmax 15
/* A simple stack-memory based allocator for Bigints */
typedef struct Stack_alloc
{
  char *begin;
  char *free;
  char *end;
  /*
    Having list of free blocks lets us reduce maximum required amount
    of memory from ~4000 bytes to < 1680 (tested on x86).
  */
  Bigint *freelist[Kmax+1];
} Stack_alloc;

#define dval(x) (x)->d

#include <float.h>//=>DBL_DIG  //z+

static const double tens[] =
{
  1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
  1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
  1e20, 1e21, 1e22
};
static const double bigtens[]= { 1e16, 1e32, 1e64, 1e128, 1e256 };
static const double tinytens[]=
{ 1e-16, 1e-32, 1e-64, 1e-128,
  9007199254740992.*9007199254740992.e-256 /* = 2^106 * 1e-53 */
};
/*
  The factor of 2^53 in tinytens[4] helps us avoid setting the underflow
  flag unnecessarily.  It leads to a song and dance at the end of strtod.
*/
#define Scale_Bit 0x10
#define n_bigtens 5

//[//=>Flt_Rounds/Exp_mask/...
#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Bias 1023
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny1 1
#define Quick_max 14
#define Int_max 14

#ifndef Flt_Rounds
#ifdef FLT_ROUNDS
#define Flt_Rounds FLT_ROUNDS
#else
#define Flt_Rounds 1
#endif
#endif /*Flt_Rounds*/

#ifdef Honor_FLT_ROUNDS
#define Rounding rounding
#undef Check_FLT_ROUNDS
#define Check_FLT_ROUNDS
#else
#define Rounding Flt_Rounds
#endif

#define rounded_product(a,b) a*= b
#define rounded_quotient(a,b) a/= b

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff
#define FFFFFFFF 0xffffffffUL
//]

#if defined(WORDS_BIGENDIAN) || (defined(__FLOAT_WORD_ORDER) &&        \
                                 (__FLOAT_WORD_ORDER == __BIG_ENDIAN))
#define word0(x) (x)->L[0]
#define word1(x) (x)->L[1]
#else
#define word0(x) (x)->L[1]
#define word1(x) (x)->L[0]
#endif

#define DTOA_BUFF_SIZE (420 * sizeof(void *))

//[[my_global.h://=>SIZEOF_CHARP
/*
  The macros below are used to allow build of Universal/fat binaries of
  MySQL and MySQL applications under darwin.
*/
//#if defined(__APPLE__) && defined(__MACH__) //z-
#  undef SIZEOF_CHARP
#  undef SIZEOF_SHORT
#  undef SIZEOF_INT
#  undef SIZEOF_LONG
#  undef SIZEOF_LONG_LONG
#  undef SIZEOF_OFF_T
#  undef WORDS_BIGENDIAN
#  define SIZEOF_SHORT 2
#  define SIZEOF_INT 4
#  define SIZEOF_LONG_LONG 8
#  define SIZEOF_OFF_T 8
#  if defined(__i386__) || defined(__ppc__)
#    define SIZEOF_CHARP 4
#    define SIZEOF_LONG 4
#  elif defined(__x86_64__) || defined(__ppc64__)
#    define SIZEOF_CHARP 8
#    define SIZEOF_LONG 8
#  else
#    error Building FAT binary for an unknown architecture.
#  endif
#  if defined(__ppc__) || defined(__ppc64__)
#    define WORDS_BIGENDIAN
#  endif
//#endif /* defined(__APPLE__) && defined(__MACH__) */ //z-

//./config.h.cmake:  #define SIZEOF_CHARP   SIZEOF_LONG
//]]
/*
  Try to allocate object on stack, and resort to malloc if all
  stack memory is used. Ensure allocated objects to be aligned by the pointer
  size in order to not break the alignment rules when storing a pointer to a
  Bigint.
*/
static Bigint *Balloc(int k, Stack_alloc *alloc)
{
  Bigint *rv;
  if (k <= Kmax &&  alloc->freelist[k])
  {
    rv= alloc->freelist[k];
    alloc->freelist[k]= rv->p.next;
  }
  else
  {
    int x, len;

    x= 1 << k;
    len= MY_ALIGN(sizeof(Bigint) + x * sizeof(ULong), SIZEOF_CHARP);

    if (alloc->free + len <= alloc->end)
    {
      rv= (Bigint*) alloc->free;
      alloc->free+= len;
    }
    else
      rv= (Bigint*) malloc(len);

    rv->k= k;
    rv->maxwds= x;
  }
  rv->sign= rv->wds= 0;
  rv->p.x= (ULong*) (rv + 1);
  return rv;
}
//-
#define Bcopy(x,y) memcpy((char *)&x->sign, (char *)&y->sign,   \
                          2*sizeof(int) + y->wds*sizeof(ULong))
static void Bfree(Bigint *v, Stack_alloc *alloc){
/*
  If object was allocated on stack, try putting it to the free
  list. Otherwise call free().
*/
  char *gptr= (char*) v;                       /* generic pointer */
  if (gptr < alloc->begin || gptr >= alloc->end)
    free(gptr);
  else if (v->k <= Kmax)
  {
    /*
      Maintain free lists only for stack objects: this way we don't
      have to bother with freeing lists in the end of dtoa;
      heap should not be used normally anyway.
    */
    v->p.next= alloc->freelist[v->k];
    alloc->freelist[v->k]= v;
  }
}
static Bigint *multadd(Bigint *b, int m, int a, Stack_alloc *alloc)
{
  int i, wds;
  ULong *x;
  ULLong carry, y;
  Bigint *b1;

  wds= b->wds;
  x= b->p.x;
  i= 0;
  carry= a;
  do
  {
    y= *x * (ULLong)m + carry;
    carry= y >> 32;
    *x++= (ULong)(y & FFFFFFFF);
  }
  while (++i < wds);
  if (carry)
  {
    if (wds >= b->maxwds)
    {
      b1= Balloc(b->k+1, alloc);
      Bcopy(b1, b);
      Bfree(b, alloc);
      b= b1;
    }
    b->p.x[wds++]= (ULong) carry;
    b->wds= wds;
  }
  return b;
}
static Bigint *s2b(const char *s, int nd0, int nd, ULong y9, Stack_alloc *alloc)
{
  Bigint *b;
  int i, k;
  Long x, y;

  x= (nd + 8) / 9;
  for (k= 0, y= 1; x > y; y <<= 1, k++) ;
  b= Balloc(k, alloc);
  b->p.x[0]= y9;
  b->wds= 1;

  i= 9;
  if (9 < nd0)
  {
    s+= 9;
    do
      b= multadd(b, 10, *s++ - '0', alloc);
    while (++i < nd0);
    s++;
  }
  else
    s+= 10;
  for(; i < nd; i++)
    b= multadd(b, 10, *s++ - '0', alloc);
  return b;
}

static int lo0bits(ULong *y)
{
  register int k;
  register ULong x= *y;

  if (x & 7)
  {
    if (x & 1)
      return 0;
    if (x & 2)
    {
      *y= x >> 1;
      return 1;
    }
    *y= x >> 2;
    return 2;
  }
  k= 0;
  if (!(x & 0xffff))
  {
    k= 16;
    x>>= 16;
  }
  if (!(x & 0xff))
  {
    k+= 8;
    x>>= 8;
  }
  if (!(x & 0xf))
  {
    k+= 4;
    x>>= 4;
  }
  if (!(x & 0x3))
  {
    k+= 2;
    x>>= 2;
  }
  if (!(x & 1))
  {
    k++;
    x>>= 1;
    if (!x)
      return 32;
  }
  *y= x;
  return k;
}
static int hi0bits(register ULong x)
{
  register int k= 0;

  if (!(x & 0xffff0000))
  {
    k= 16;
    x<<= 16;
  }
  if (!(x & 0xff000000))
  {
    k+= 8;
    x<<= 8;
  }
  if (!(x & 0xf0000000))
  {
    k+= 4;
    x<<= 4;
  }
  if (!(x & 0xc0000000))
  {
    k+= 2;
    x<<= 2;
  }
  if (!(x & 0x80000000))
  {
    k++;
    if (!(x & 0x40000000))
      return 32;
  }
  return k;
}
static Bigint *d2b(U *d, int *e, int *bits, Stack_alloc *alloc)
{
  Bigint *b;
  int de, k;
  ULong *x, y, z;
  int i;
#define d0 word0(d)
#define d1 word1(d)

  b= Balloc(1, alloc);
  x= b->p.x;

  z= d0 & Frac_mask;
  d0 &= 0x7fffffff;       /* clear sign bit, which we ignore */
  if ((de= (int)(d0 >> Exp_shift)))
    z|= Exp_msk1;
  if ((y= d1))
  {
    if ((k= lo0bits(&y)))
    {
      x[0]= y | z << (32 - k);
      z>>= k;
    }
    else
      x[0]= y;
    i= b->wds= (x[1]= z) ? 2 : 1;
  }
  else
  {
    k= lo0bits(&z);
    x[0]= z;
    i= b->wds= 1;
    k+= 32;
  }
  if (de)
  {
    *e= de - Bias - (P-1) + k;
    *bits= P - k;
  }
  else
  {
    *e= de - Bias - (P-1) + 1 + k;
    *bits= 32*i - hi0bits(x[i-1]);
  }
  return b;
#undef d0
#undef d1
}

/* Convert integer to Bigint number */
static Bigint *i2b(int i, Stack_alloc *alloc)
{
  Bigint *b;

  b= Balloc(1, alloc);
  b->p.x[0]= i;
  b->wds= 1;
  return b;
}

/*
  Precalculated array of powers of 5: tested to be enough for
  vasting majority of dtoa_r cases.
*/
static ULong powers5[]=
{
  625UL,

  390625UL,

  2264035265UL, 35UL,

  2242703233UL, 762134875UL,  1262UL,

  3211403009UL, 1849224548UL, 3668416493UL, 3913284084UL, 1593091UL,

  781532673UL,  64985353UL,   253049085UL,  594863151UL,  3553621484UL,
  3288652808UL, 3167596762UL, 2788392729UL, 3911132675UL, 590UL,

  2553183233UL, 3201533787UL, 3638140786UL, 303378311UL, 1809731782UL,
  3477761648UL, 3583367183UL, 649228654UL, 2915460784UL, 487929380UL,
  1011012442UL, 1677677582UL, 3428152256UL, 1710878487UL, 1438394610UL,
  2161952759UL, 4100910556UL, 1608314830UL, 349175UL
};
static Bigint p5_a[]=
{
  /*  { x } - k - maxwds - sign - wds */
  { { powers5 }, 1, 1, 0, 1 },
  { { powers5 + 1 }, 1, 1, 0, 1 },
  { { powers5 + 2 }, 1, 2, 0, 2 },
  { { powers5 + 4 }, 2, 3, 0, 3 },
  { { powers5 + 7 }, 3, 5, 0, 5 },
  { { powers5 + 12 }, 4, 10, 0, 10 },
  { { powers5 + 22 }, 5, 19, 0, 19 }
};
#define P5A_MAX (sizeof(p5_a)/sizeof(*p5_a) - 1)
static Bigint *mult(Bigint *a, Bigint *b, Stack_alloc *alloc)
{
  Bigint *c;
  int k, wa, wb, wc;
  ULong *x, *xa, *xae, *xb, *xbe, *xc, *xc0;
  ULong y;
  ULLong carry, z;

  if (a->wds < b->wds)
  {
    c= a;
    a= b;
    b= c;
  }
  k= a->k;
  wa= a->wds;
  wb= b->wds;
  wc= wa + wb;
  if (wc > a->maxwds)
    k++;
  c= Balloc(k, alloc);
  for (x= c->p.x, xa= x + wc; x < xa; x++)
    *x= 0;
  xa= a->p.x;
  xae= xa + wa;
  xb= b->p.x;
  xbe= xb + wb;
  xc0= c->p.x;
  for (; xb < xbe; xc0++)
  {
    if ((y= *xb++))
    {
      x= xa;
      xc= xc0;
      carry= 0;
      do
      {
        z= *x++ * (ULLong)y + *xc + carry;
        carry= z >> 32;
        *xc++= (ULong) (z & FFFFFFFF);
      }
      while (x < xae);
      *xc= (ULong) carry;
    }
  }
  for (xc0= c->p.x, xc= xc0 + wc; wc > 0 && !*--xc; --wc) ;
  c->wds= wc;
  return c;
}
static Bigint *pow5mult(Bigint *b, int k, Stack_alloc *alloc)
{
  Bigint *b1, *p5, *p51;
  int i;
  static int p05[3]= { 5, 25, 125 };

  if ((i= k & 3))
    b= multadd(b, p05[i-1], 0, alloc);

  if (!(k>>= 2))
    return b;
  p5= p5_a;
  for (;;)
  {
    if (k & 1)
    {
      b1= mult(b, p5, alloc);
      Bfree(b, alloc);
      b= b1;
    }
    if (!(k>>= 1))
      break;
    /* Calculate next power of 5 */
    if (p5 < p5_a + P5A_MAX)
      ++p5;
    else if (p5 == p5_a + P5A_MAX)
      p5= mult(p5, p5, alloc);
    else
    {
      p51= mult(p5, p5, alloc);
      Bfree(p5, alloc);
      p5= p51;
    }
  }
  return b;
}

static Bigint *lshift(Bigint *b, int k, Stack_alloc *alloc)
{
  int i, k1, n, n1;
  Bigint *b1;
  ULong *x, *x1, *xe, z;

  n= k >> 5;
  k1= b->k;
  n1= n + b->wds + 1;
  for (i= b->maxwds; n1 > i; i<<= 1)
    k1++;
  b1= Balloc(k1, alloc);
  x1= b1->p.x;
  for (i= 0; i < n; i++)
    *x1++= 0;
  x= b->p.x;
  xe= x + b->wds;
  if (k&= 0x1f)
  {
    k1= 32 - k;
    z= 0;
    do
    {
      *x1++= *x << k | z;
      z= *x++ >> k1;
    }
    while (x < xe);
    if ((*x1= z))
      ++n1;
  }
  else
    do
      *x1++= *x++;
    while (x < xe);
  b1->wds= n1 - 1;
  Bfree(b, alloc);
  return b1;
}

static int cmp(Bigint *a, Bigint *b)
{
  ULong *xa, *xa0, *xb, *xb0;
  int i, j;

  i= a->wds;
  j= b->wds;
  if (i-= j)
    return i;
  xa0= a->p.x;
  xa= xa0 + j;
  xb0= b->p.x;
  xb= xb0 + j;
  for (;;)
  {
    if (*--xa != *--xb)
      return *xa < *xb ? -1 : 1;
    if (xa <= xa0)
      break;
  }
  return 0;
}
static Bigint *diff(Bigint *a, Bigint *b, Stack_alloc *alloc)
{
  Bigint *c;
  int i, wa, wb;
  ULong *xa, *xae, *xb, *xbe, *xc;
  ULLong borrow, y;

  i= cmp(a,b);
  if (!i)
  {
    c= Balloc(0, alloc);
    c->wds= 1;
    c->p.x[0]= 0;
    return c;
  }
  if (i < 0)
  {
    c= a;
    a= b;
    b= c;
    i= 1;
  }
  else
    i= 0;
  c= Balloc(a->k, alloc);
  c->sign= i;
  wa= a->wds;
  xa= a->p.x;
  xae= xa + wa;
  wb= b->wds;
  xb= b->p.x;
  xbe= xb + wb;
  xc= c->p.x;
  borrow= 0;
  do
  {
    y= (ULLong)*xa++ - *xb++ - borrow;
    borrow= y >> 32 & (ULong)1;
    *xc++= (ULong) (y & FFFFFFFF);
  }
  while (xb < xbe);
  while (xa < xae)
  {
    y= *xa++ - borrow;
    borrow= y >> 32 & (ULong)1;
    *xc++= (ULong) (y & FFFFFFFF);
  }
  while (!*--xc)
    wa--;
  c->wds= wa;
  return c;
}

static double ulp(U *x)
{
  register Long L;
  U u;

  L= (word0(x) & Exp_mask) - (P - 1)*Exp_msk1;
  word0(&u) = L;
  word1(&u) = 0;
  return dval(&u);
}

static double b2d(Bigint *a, int *e)
{
  ULong *xa, *xa0, w, y, z;
  int k;
  U d;
#define d0 word0(&d)
#define d1 word1(&d)

  xa0= a->p.x;
  xa= xa0 + a->wds;
  y= *--xa;
  k= hi0bits(y);
  *e= 32 - k;
  if (k < Ebits)
  {
    d0= Exp_1 | y >> (Ebits - k);
    w= xa > xa0 ? *--xa : 0;
    d1= y << ((32-Ebits) + k) | w >> (Ebits - k);
    goto ret_d;
  }
  z= xa > xa0 ? *--xa : 0;
  if (k-= Ebits)
  {
    d0= Exp_1 | y << k | z >> (32 - k);
    y= xa > xa0 ? *--xa : 0;
    d1= z << k | y >> (32 - k);
  }
  else
  {
    d0= Exp_1 | y;
    d1= z;
  }
 ret_d:
#undef d0
#undef d1
  return dval(&d);
}
static double ratio(Bigint *a, Bigint *b)
{
  U da, db;
  int k, ka, kb;

  dval(&da)= b2d(a, &ka);
  dval(&db)= b2d(b, &kb);
  k= ka - kb + 32*(a->wds - b->wds);
  if (k > 0)
    word0(&da)+= k*Exp_msk1;
  else
  {
    k= -k;
    word0(&db)+= k*Exp_msk1;
  }
  return dval(&da) / dval(&db);
}

//=>://zlq
/*
  strtod for IEEE--arithmetic machines.

  This strtod returns a nearest machine number to the input decimal
  string (or sets errno to EOVERFLOW). Ties are broken by the IEEE round-even
  rule.

  Inspired loosely by William D. Clinger's paper "How to Read Floating
  Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].

  Modifications:

   1. We only require IEEE (not IEEE double-extended).
   2. We get by with floating-point arithmetic in a case that
     Clinger missed -- when we're computing d * 10^n
     for a small integer d and the integer n is not too
     much larger than 22 (the maximum integer k for which
     we can represent 10^k exactly), we may be able to
     compute (d*10^k) * 10^(e-k) with just one roundoff.
   3. Rather than a bit-at-a-time adjustment of the binary
     result in the hard case, we use floating-point
     arithmetic to determine the adjustment to within
     one bit; only in really hard cases do we need to
     compute a second residual.
   4. Because of 3., we don't need a large table of powers of 10
     for ten-to-e (just some small tables, e.g. of 10^k
     for 0 <= k <= 22).
*/
static double my_strtod_int(const char *s00, char **se, int *error, char *buf, size_t buf_size)
{
  int scale;
  int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, UNINIT_VAR(c), dsign,
     e, e1, esign, i, j, k, nd, nd0, nf, nz, nz0, sign;
  const char *s, *s0, *s1, *end = *se;
  double aadj, aadj1;
  U aadj2, adj, rv, rv0;
  Long L;
  ULong y, z;
  Bigint *bb, *bb1, *bd, *bd0, *bs, *delta;
#ifdef SET_INEXACT
  int inexact, oldinexact;
#endif
#ifdef Honor_FLT_ROUNDS
  int rounding;
#endif
  Stack_alloc alloc;

  *error= 0;

  alloc.begin= alloc.free= buf;
  alloc.end= buf + buf_size;
  memset(alloc.freelist, 0, sizeof(alloc.freelist));

  sign= nz0= nz= 0;
  dval(&rv)= 0.;
  for (s= s00; s < end; s++)
    switch (*s) {
    case '-':
      sign= 1;
      /* no break */
    case '+':
      s++;
      goto break2;
    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r':
    case ' ':
      continue;
    default:
      goto break2;
    }
 break2:
  if (s >= end)
    goto ret0;

  if (*s == '0')
  {
    nz0= 1;
    while (++s < end && *s == '0') ;
    if (s >= end)
      goto ret;
  }
  s0= s;
  y= z= 0;
  for (nd= nf= 0; s < end && (c= *s) >= '0' && c <= '9'; nd++, s++)
    if (nd < 9)
      y= 10*y + c - '0';
    else if (nd < 16)
      z= 10*z + c - '0';
  nd0= nd;
  if (s < end - 1 && c == '.')
  {
    c= *++s;
    if (!nd)
    {
      for (; s < end && c == '0'; c= *++s)
        nz++;
      if (s < end && c > '0' && c <= '9')
      {
        s0= s;
        nf+= nz;
        nz= 0;
        goto have_dig;
      }
      goto dig_done;
    }
    for (; s < end && c >= '0' && c <= '9'; c = *++s)
    {
 have_dig:
      nz++;
      if (c-= '0')
      {
        nf+= nz;
        for (i= 1; i < nz; i++)
          if (nd++ < 9)
            y*= 10;
          else if (nd <= DBL_DIG + 1)
            z*= 10;
        if (nd++ < 9)
          y= 10*y + c;
        else if (nd <= DBL_DIG + 1)
          z= 10*z + c;
        nz= 0;
      }
    }
  }
 dig_done:
  e= 0;
  if (s < end && (c == 'e' || c == 'E'))
  {
    if (!nd && !nz && !nz0)
      goto ret0;
    s00= s;
    esign= 0;
    if (++s < end)
      switch (c= *s) {
      case '-':
        esign= 1;
      case '+':
        c= *++s;
      }
    if (s < end && c >= '0' && c <= '9')
    {
      while (s < end && c == '0')
        c= *++s;
      if (s < end && c > '0' && c <= '9') {
        L= c - '0';
        s1= s;
        while (++s < end && (c= *s) >= '0' && c <= '9')
          L= 10*L + c - '0';
        if (s - s1 > 8 || L > 19999)
          /* Avoid confusion from exponents
           * so large that e might overflow.
           */
          e= 19999; /* safe for 16 bit ints */
        else
          e= (int)L;
        if (esign)
          e= -e;
      }
      else
        e= 0;
    }
    else
      s= s00;
  }
  if (!nd)
  {
    if (!nz && !nz0)
    {
 ret0:
      s= s00;
      sign= 0;
    }
    goto ret;
  }
  e1= e -= nf;

  /*
    Now we have nd0 digits, starting at s0, followed by a
    decimal point, followed by nd-nd0 digits.  The number we're
    after is the integer represented by those digits times
    10**e
   */

  if (!nd0)
    nd0= nd;
  k= nd < DBL_DIG + 1 ? nd : DBL_DIG + 1;
  dval(&rv)= y;
  if (k > 9)
  {
#ifdef SET_INEXACT
    if (k > DBL_DIG)
      oldinexact = get_inexact();
#endif
    dval(&rv)= tens[k - 9] * dval(&rv) + z;
  }
  bd0= 0;
  if (nd <= DBL_DIG
#ifndef Honor_FLT_ROUNDS
    && Flt_Rounds == 1
#endif
      )
  {
    if (!e)
      goto ret;
    if (e > 0)
    {
      if (e <= Ten_pmax)
      {
#ifdef Honor_FLT_ROUNDS
        /* round correctly FLT_ROUNDS = 2 or 3 */
        if (sign)
        {
          rv.d= -rv.d;
          sign= 0;
        }
#endif
        /* rv = */ rounded_product(dval(&rv), tens[e]);
        goto ret;
      }
      i= DBL_DIG - nd;
      if (e <= Ten_pmax + i)
      {
        /*
          A fancier test would sometimes let us do
          this for larger i values.
        */
#ifdef Honor_FLT_ROUNDS
        /* round correctly FLT_ROUNDS = 2 or 3 */
        if (sign)
        {
          rv.d= -rv.d;
          sign= 0;
        }
#endif
        e-= i;
        dval(&rv)*= tens[i];
        /* rv = */ rounded_product(dval(&rv), tens[e]);
        goto ret;
      }
    }
#ifndef Inaccurate_Divide
    else if (e >= -Ten_pmax)
    {
#ifdef Honor_FLT_ROUNDS
      /* round correctly FLT_ROUNDS = 2 or 3 */
      if (sign)
      {
        rv.d= -rv.d;
        sign= 0;
      }
#endif
      /* rv = */ rounded_quotient(dval(&rv), tens[-e]);
      goto ret;
    }
#endif
  }
  e1+= nd - k;

#ifdef SET_INEXACT
  inexact= 1;
  if (k <= DBL_DIG)
    oldinexact= get_inexact();
#endif
  scale= 0;
#ifdef Honor_FLT_ROUNDS
  if ((rounding= Flt_Rounds) >= 2)
  {
    if (sign)
      rounding= rounding == 2 ? 0 : 2;
    else
      if (rounding != 2)
        rounding= 0;
  }
#endif

  /* Get starting approximation = rv * 10**e1 */

  if (e1 > 0)
  {
    if ((i= e1 & 15))
      dval(&rv)*= tens[i];
    if (e1&= ~15)
    {
      if (e1 > DBL_MAX_10_EXP)
      {
 ovfl:
        *error= EOVERFLOW;
        /* Can't trust HUGE_VAL */
#ifdef Honor_FLT_ROUNDS
        switch (rounding)
        {
        case 0: /* toward 0 */
        case 3: /* toward -infinity */
          word0(&rv)= Big0;
          word1(&rv)= Big1;
          break;
        default:
          word0(&rv)= Exp_mask;
          word1(&rv)= 0;
        }
#else /*Honor_FLT_ROUNDS*/
        word0(&rv)= Exp_mask;
        word1(&rv)= 0;
#endif /*Honor_FLT_ROUNDS*/
#ifdef SET_INEXACT
        /* set overflow bit */
        dval(&rv0)= 1e300;
        dval(&rv0)*= dval(&rv0);
#endif
        if (bd0)
          goto retfree;
        goto ret;
      }
      e1>>= 4;
      for(j= 0; e1 > 1; j++, e1>>= 1)
        if (e1 & 1)
          dval(&rv)*= bigtens[j];
    /* The last multiplication could overflow. */
      word0(&rv)-= P*Exp_msk1;
      dval(&rv)*= bigtens[j];
      if ((z= word0(&rv) & Exp_mask) > Exp_msk1 * (DBL_MAX_EXP + Bias - P))
        goto ovfl;
      if (z > Exp_msk1 * (DBL_MAX_EXP + Bias - 1 - P))
      {
        /* set to largest number (Can't trust DBL_MAX) */
        word0(&rv)= Big0;
        word1(&rv)= Big1;
      }
      else
        word0(&rv)+= P*Exp_msk1;
    }
  }
  else if (e1 < 0)
  {
    e1= -e1;
    if ((i= e1 & 15))
      dval(&rv)/= tens[i];
    if ((e1>>= 4))
    {
      if (e1 >= 1 << n_bigtens)
        goto undfl;
      if (e1 & Scale_Bit)
        scale= 2 * P;
      for(j= 0; e1 > 0; j++, e1>>= 1)
        if (e1 & 1)
          dval(&rv)*= tinytens[j];
      if (scale && (j = 2 * P + 1 - ((word0(&rv) & Exp_mask) >> Exp_shift)) > 0)
      {
        /* scaled rv is denormal; zap j low bits */
        if (j >= 32)
        {
          word1(&rv)= 0;
          if (j >= 53)
            word0(&rv)= (P + 2) * Exp_msk1;
          else
            word0(&rv)&= 0xffffffff << (j - 32);
        }
        else
          word1(&rv)&= 0xffffffff << j;
      }
      if (!dval(&rv))
      {
 undfl:
          dval(&rv)= 0.;
          if (bd0)
            goto retfree;
          goto ret;
      }
    }
  }

  /* Now the hard part -- adjusting rv to the correct value.*/

  /* Put digits into bd: true value = bd * 10^e */

  bd0= s2b(s0, nd0, nd, y, &alloc);

  for(;;)
  {
    bd= Balloc(bd0->k, &alloc);
    Bcopy(bd, bd0);
    bb= d2b(&rv, &bbe, &bbbits, &alloc);  /* rv = bb * 2^bbe */
    bs= i2b(1, &alloc);

    if (e >= 0)
    {
      bb2= bb5= 0;
      bd2= bd5= e;
    }
    else
    {
      bb2= bb5= -e;
      bd2= bd5= 0;
    }
    if (bbe >= 0)
      bb2+= bbe;
    else
      bd2-= bbe;
    bs2= bb2;
#ifdef Honor_FLT_ROUNDS
    if (rounding != 1)
      bs2++;
#endif
    j= bbe - scale;
    i= j + bbbits - 1;  /* logb(rv) */
    if (i < Emin)  /* denormal */
      j+= P - Emin;
    else
      j= P + 1 - bbbits;
    bb2+= j;
    bd2+= j;
    bd2+= scale;
    i= bb2 < bd2 ? bb2 : bd2;
    if (i > bs2)
      i= bs2;
    if (i > 0)
    {
      bb2-= i;
      bd2-= i;
      bs2-= i;
    }
    if (bb5 > 0)
    {
      bs= pow5mult(bs, bb5, &alloc);
      bb1= mult(bs, bb, &alloc);
      Bfree(bb, &alloc);
      bb= bb1;
    }
    if (bb2 > 0)
      bb= lshift(bb, bb2, &alloc);
    if (bd5 > 0)
      bd= pow5mult(bd, bd5, &alloc);
    if (bd2 > 0)
      bd= lshift(bd, bd2, &alloc);
    if (bs2 > 0)
      bs= lshift(bs, bs2, &alloc);
    delta= diff(bb, bd, &alloc);
    dsign= delta->sign;
    delta->sign= 0;
    i= cmp(delta, bs);
#ifdef Honor_FLT_ROUNDS
    if (rounding != 1)
    {
      if (i < 0)
      {
        /* Error is less than an ulp */
        if (!delta->p.x[0] && delta->wds <= 1)
        {
          /* exact */
#ifdef SET_INEXACT
          inexact= 0;
#endif
          break;
        }
        if (rounding)
        {
          if (dsign)
          {
            adj.d= 1.;
            goto apply_adj;
          }
        }
        else if (!dsign)
        {
          adj.d= -1.;
          if (!word1(&rv) && !(word0(&rv) & Frac_mask))
          {
            y= word0(&rv) & Exp_mask;
            if (!scale || y > 2*P*Exp_msk1)
            {
              delta= lshift(delta, Log2P, &alloc);
              if (cmp(delta, bs) <= 0)
              adj.d= -0.5;
            }
          }
 apply_adj:
          if (scale && (y= word0(&rv) & Exp_mask) <= 2 * P * Exp_msk1)
            word0(&adj)+= (2 * P + 1) * Exp_msk1 - y;
          dval(&rv)+= adj.d * ulp(&rv);
        }
        break;
      }
      adj.d= ratio(delta, bs);
      if (adj.d < 1.)
        adj.d= 1.;
      if (adj.d <= 0x7ffffffe)
      {
        /* adj = rounding ? ceil(adj) : floor(adj); */
        y= adj.d;
        if (y != adj.d)
        {
          if (!((rounding >> 1) ^ dsign))
            y++;
          adj.d= y;
        }
      }
      if (scale && (y= word0(&rv) & Exp_mask) <= 2 * P * Exp_msk1)
        word0(&adj)+= (2 * P + 1) * Exp_msk1 - y;
      adj.d*= ulp(&rv);
      if (dsign)
        dval(&rv)+= adj.d;
      else
        dval(&rv)-= adj.d;
      goto cont;
    }
#endif /*Honor_FLT_ROUNDS*/

    if (i < 0)
    {
      /*
        Error is less than half an ulp -- check for special case of mantissa
        a power of two.
      */
      if (dsign || word1(&rv) || word0(&rv) & Bndry_mask ||
          (word0(&rv) & Exp_mask) <= (2 * P + 1) * Exp_msk1)
      {
#ifdef SET_INEXACT
        if (!delta->x[0] && delta->wds <= 1)
          inexact= 0;
#endif
        break;
      }
      if (!delta->p.x[0] && delta->wds <= 1)
      {
        /* exact result */
#ifdef SET_INEXACT
        inexact= 0;
#endif
        break;
      }
      delta= lshift(delta, Log2P, &alloc);
      if (cmp(delta, bs) > 0)
        goto drop_down;
      break;
    }
    if (i == 0)
    {
      /* exactly half-way between */
      if (dsign)
      {
        if ((word0(&rv) & Bndry_mask1) == Bndry_mask1 &&
            word1(&rv) ==
            ((scale && (y = word0(&rv) & Exp_mask) <= 2 * P * Exp_msk1) ?
             (0xffffffff & (0xffffffff << (2*P+1-(y>>Exp_shift)))) :
             0xffffffff))
        {
          /*boundary case -- increment exponent*/
          word0(&rv)= (word0(&rv) & Exp_mask) + Exp_msk1;
          word1(&rv) = 0;
          dsign = 0;
          break;
        }
      }
      else if (!(word0(&rv) & Bndry_mask) && !word1(&rv))
      {
 drop_down:
        /* boundary case -- decrement exponent */
        if (scale)
        {
          L= word0(&rv) & Exp_mask;
          if (L <= (2 *P + 1) * Exp_msk1)
          {
            if (L > (P + 2) * Exp_msk1)
              /* round even ==> accept rv */
              break;
            /* rv = smallest denormal */
            goto undfl;
          }
        }
        L= (word0(&rv) & Exp_mask) - Exp_msk1;
        word0(&rv)= L | Bndry_mask1;
        word1(&rv)= 0xffffffff;
        break;
      }
      if (!(word1(&rv) & LSB))
        break;
      if (dsign)
        dval(&rv)+= ulp(&rv);
      else
      {
        dval(&rv)-= ulp(&rv);
        if (!dval(&rv))
          goto undfl;
      }
      dsign= 1 - dsign;
      break;
    }
    if ((aadj= ratio(delta, bs)) <= 2.)
    {
      if (dsign)
        aadj= aadj1= 1.;
      else if (word1(&rv) || word0(&rv) & Bndry_mask)
      {
        if (word1(&rv) == Tiny1 && !word0(&rv))
          goto undfl;
        aadj= 1.;
        aadj1= -1.;
      }
      else
      {
        /* special case -- power of FLT_RADIX to be rounded down... */
        if (aadj < 2. / FLT_RADIX)
          aadj= 1. / FLT_RADIX;
        else
          aadj*= 0.5;
        aadj1= -aadj;
      }
    }
    else
    {
      aadj*= 0.5;
      aadj1= dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
      switch (Rounding)
      {
      case 2: /* towards +infinity */
        aadj1-= 0.5;
        break;
      case 0: /* towards 0 */
      case 3: /* towards -infinity */
        aadj1+= 0.5;
      }
#else
      if (Flt_Rounds == 0)
        aadj1+= 0.5;
#endif /*Check_FLT_ROUNDS*/
    }
    y= word0(&rv) & Exp_mask;

    /* Check for overflow */

    if (y == Exp_msk1 * (DBL_MAX_EXP + Bias - 1))
    {
      dval(&rv0)= dval(&rv);
      word0(&rv)-= P * Exp_msk1;
      adj.d= aadj1 * ulp(&rv);
      dval(&rv)+= adj.d;
      if ((word0(&rv) & Exp_mask) >= Exp_msk1 * (DBL_MAX_EXP + Bias - P))
      {
        if (word0(&rv0) == Big0 && word1(&rv0) == Big1)
          goto ovfl;
        word0(&rv)= Big0;
        word1(&rv)= Big1;
        goto cont;
      }
      else
        word0(&rv)+= P * Exp_msk1;
    }
    else
    {
      if (scale && y <= 2 * P * Exp_msk1)
      {
        if (aadj <= 0x7fffffff)
        {
          if ((z= (ULong) aadj) <= 0)
            z= 1;
          aadj= z;
          aadj1= dsign ? aadj : -aadj;
        }
        dval(&aadj2) = aadj1;
        word0(&aadj2)+= (2 * P + 1) * Exp_msk1 - y;
        aadj1= dval(&aadj2);
        adj.d= aadj1 * ulp(&rv);
        dval(&rv)+= adj.d;
        if (rv.d == 0.)
          goto undfl;
      }
      else
      {
        adj.d= aadj1 * ulp(&rv);
        dval(&rv)+= adj.d;
      }
    }
    z= word0(&rv) & Exp_mask;
#ifndef SET_INEXACT
    if (!scale)
      if (y == z)
      {
        /* Can we stop now? */
        L= (Long)aadj;
        aadj-= L;
        /* The tolerances below are conservative. */
        if (dsign || word1(&rv) || word0(&rv) & Bndry_mask)
        {
          if (aadj < .4999999 || aadj > .5000001)
            break;
        }
        else if (aadj < .4999999 / FLT_RADIX)
          break;
      }
#endif
 cont:
    Bfree(bb, &alloc);
    Bfree(bd, &alloc);
    Bfree(bs, &alloc);
    Bfree(delta, &alloc);
  }
#ifdef SET_INEXACT
  if (inexact)
  {
    if (!oldinexact)
    {
      word0(&rv0)= Exp_1 + (70 << Exp_shift);
      word1(&rv0)= 0;
      dval(&rv0)+= 1.;
    }
  }
  else if (!oldinexact)
    clear_inexact();
#endif
  if (scale)
  {
    word0(&rv0)= Exp_1 - 2 * P * Exp_msk1;
    word1(&rv0)= 0;
    dval(&rv)*= dval(&rv0);
  }
#ifdef SET_INEXACT
  if (inexact && !(word0(&rv) & Exp_mask))
  {
    /* set underflow bit */
    dval(&rv0)= 1e-300;
    dval(&rv0)*= dval(&rv0);
  }
#endif
 retfree:
  Bfree(bb, &alloc);
  Bfree(bd, &alloc);
  Bfree(bs, &alloc);
  Bfree(bd0, &alloc);
  Bfree(delta, &alloc);
 ret:
  *se= (char *)s;
  return sign ? -dval(&rv) : dval(&rv);
}
/**
   @brief
   Converts string to double (string does not have to be zero-terminated)

   @details
   This is a wrapper around dtoa's version of strtod().

   @param str     input string
   @param end     address of a pointer to the first character after the input
                  string. Upon return the pointer is set to point to the first
                  rejected character.
   @param error   Upon return is set to EOVERFLOW in case of underflow or
                  overflow.

   @return        The resulting double value. In case of underflow, 0.0 is
                  returned. In case overflow, signed DBL_MAX is returned.
*/
double my_strtod(const char *str, char **end, int *error)
{
  char buf[DTOA_BUFF_SIZE];
  double res;
  DBUG_ASSERT(end != NULL && ((str != NULL && *end != NULL) ||
                              (str == NULL && *end == NULL)) &&
              error != NULL);

  res= my_strtod_int(str, end, error, buf, sizeof(buf));
  return (*error == 0) ? res : (res < 0 ? -DBL_MAX : DBL_MAX);
}
//]
double my_strntod_8bit(CHARSET_INFO *cs __attribute__((unused)),
		       char *str, size_t length,
		       char **end, int *err)
{
  if (length == INT_MAX32)
    length= 65535;                          /* Should be big enough */
  *end= str + length;
  return my_strtod(str, end, err);
}
//[strings/my_strtoll10.c:  //2011-10-08-13-53;
//#include <my_global.h>
//#include <my_sys.h>            /* Needed for MY_ERRNO_ERANGE */
//#include <m_string.h>
//
#define MAX_NEGATIVE_NUMBER	((ulonglong) LL(0x8000000000000000))
#define INIT_CNT  9
#define LFACTOR   ULL(1000000000)
#define LFACTOR1  ULL(10000000000)
#define LFACTOR2  ULL(100000000000)

static unsigned long lfactor[9]=
{
  1L, 10L, 100L, 1000L, 10000L, 100000L, 1000000L, 10000000L, 100000000L
};

//[[my_sys.h:
/* Internal error numbers (for assembler functions) */
#define MY_ERRNO_EDOM		33
#define MY_ERRNO_ERANGE		34
//]]
/*
  Convert a string to an to unsigned long long integer value

  SYNOPSYS
    my_strtoll10()
      nptr     in       pointer to the string to be converted
      endptr   in/out   pointer to the end of the string/
                        pointer to the stop character
      error    out      returned error code

  DESCRIPTION
    This function takes the decimal representation of integer number
    from string nptr and converts it to an signed or unsigned
    long long integer value.
    Space characters and tab are ignored.
    A sign character might precede the digit characters. The number
    may have any number of pre-zero digits.

    The function stops reading the string nptr at the first character
    that is not a decimal digit. If endptr is not NULL then the function
    will not read characters after *endptr.

  RETURN VALUES
    Value of string as a signed/unsigned longlong integer

    if no error and endptr != NULL, it will be set to point at the character
    after the number

    The error parameter contains information how things went:
    -1		Number was an ok negative number
    0	 	ok
    ERANGE	If the the value of the converted number exceeded the
	        maximum negative/unsigned long long integer.
		In this case the return value is ~0 if value was
		positive and LONGLONG_MIN if value was negative.
    EDOM	If the string didn't contain any digits. In this case
    		the return value is 0.

    If endptr is not NULL the function will store the end pointer to
    the stop character here.
*/
longlong my_strtoll10(const char *nptr, char **endptr, int *error)
{
  const char *s, *end, *start, *n_end, *true_end;
  char *dummy;
  uchar c;
  unsigned long i, j, k;
  ulonglong li;
  int negative;
  ulong cutoff, cutoff2, cutoff3;

  s= nptr;
  /* If fixed length string */
  if (endptr)
  {
    end= *endptr;
    while (s != end && (*s == ' ' || *s == '\t'))
      s++;
    if (s == end)
      goto no_conv;
  }
  else
  {
    endptr= &dummy;				/* Easier end test */
    while (*s == ' ' || *s == '\t')
      s++;
    if (!*s)
      goto no_conv;
    /* This number must be big to guard against a lot of pre-zeros */
    end= s+65535;				/* Can't be longer than this */
  }

  /* Check for a sign.	*/
  negative= 0;
  if (*s == '-')
  {
    *error= -1;					/* Mark as negative number */
    negative= 1;
    if (++s == end)
      goto no_conv;
    cutoff=  MAX_NEGATIVE_NUMBER / LFACTOR2;
    cutoff2= (MAX_NEGATIVE_NUMBER % LFACTOR2) / 100;
    cutoff3=  MAX_NEGATIVE_NUMBER % 100;
  }
  else
  {
    *error= 0;
    if (*s == '+')
    {
      if (++s == end)
	goto no_conv;
    }
    cutoff=  ULONGLONG_MAX / LFACTOR2;
    cutoff2= ULONGLONG_MAX % LFACTOR2 / 100;
    cutoff3=  ULONGLONG_MAX % 100;
  }

  /* Handle case where we have a lot of pre-zero */
  if (*s == '0')
  {
    i= 0;
    do
    {
      if (++s == end)
	goto end_i;				/* Return 0 */
    }
    while (*s == '0');
    n_end= s+ INIT_CNT;
  }
  else
  {
    /* Read first digit to check that it's a valid number */
    if ((c= (*s-'0')) > 9)
      goto no_conv;
    i= c;
    n_end= ++s+ INIT_CNT-1;
  }

  /* Handle first 9 digits and store them in i */
  if (n_end > end)
    n_end= end;
  for (; s != n_end ; s++)
  {
    if ((c= (*s-'0')) > 9)
      goto end_i;
    i= i*10+c;
  }
  if (s == end)
    goto end_i;

  /* Handle next 9 digits and store them in j */
  j= 0;
  start= s;				/* Used to know how much to shift i */
  n_end= true_end= s + INIT_CNT;
  if (n_end > end)
    n_end= end;
  do
  {
    if ((c= (*s-'0')) > 9)
      goto end_i_and_j;
    j= j*10+c;
  } while (++s != n_end);
  if (s == end)
  {
    if (s != true_end)
      goto end_i_and_j;
    goto end3;
  }
  if ((c= (*s-'0')) > 9)
    goto end3;

  /* Handle the next 1 or 2 digits and store them in k */
  k=c;
  if (++s == end || (c= (*s-'0')) > 9)
    goto end4;
  k= k*10+c;
  *endptr= (char*) ++s;

  /* number string should have ended here */
  if (s != end && (c= (*s-'0')) <= 9)
    goto overflow;

  /* Check that we didn't get an overflow with the last digit */
  if (i > cutoff || (i == cutoff && ((j > cutoff2 || j == cutoff2) &&
                                     k > cutoff3)))
    goto overflow;
  li=i*LFACTOR2+ (ulonglong) j*100 + k;
  return (longlong) li;

overflow:					/* *endptr is set here */
  *error= MY_ERRNO_ERANGE;
  return negative ? LONGLONG_MIN : (longlong) ULONGLONG_MAX;

end_i:
  *endptr= (char*) s;
  return (negative ? ((longlong) -(long) i) : (longlong) i);

end_i_and_j:
  li= (ulonglong) i * lfactor[(uint) (s-start)] + j;
  *endptr= (char*) s;
  return (negative ? -((longlong) li) : (longlong) li);

end3:
  li=(ulonglong) i*LFACTOR+ (ulonglong) j;
  *endptr= (char*) s;
  return (negative ? -((longlong) li) : (longlong) li);

end4:
  li=(ulonglong) i*LFACTOR1+ (ulonglong) j * 10 + k;
  *endptr= (char*) s;
  if (negative)
  {
   if (li > MAX_NEGATIVE_NUMBER)
     goto overflow;
   return -((longlong) li);
  }
  return (longlong) li;

no_conv:
  /* There was no number to convert.  */
  *error= MY_ERRNO_EDOM;
  *endptr= (char *) nptr;
  return 0;
}
//]
longlong my_strtoll10_8bit(CHARSET_INFO *cs __attribute__((unused)),
                           const char *nptr, char **endptr, int *error)
{
  return my_strtoll10(nptr, endptr, error);
}
#define MY_ERRNO_EDOM		33  //=>my_sys.h
#define MY_ERRNO_ERANGE		34  //=>my_sys.h
#define CUTOFF  (ULONGLONG_MAX / 10)  //=>InnoSQL-5.5.8/strings/ctype-simple.c
#define CUTLIM  (ULONGLONG_MAX % 10)  //=>InnoSQL-5.5.8/strings/ctype-simple.c
#define DIGITS_IN_ULONGLONG 20        //=>InnoSQL-5.5.8/strings/ctype-simple.c
static ulonglong d10[DIGITS_IN_ULONGLONG]=
{
  1,
  10,
  100,
  1000,
  10000,
  100000,
  1000000,
  10000000,
  100000000,
  1000000000,
  10000000000ULL,
  100000000000ULL,
  1000000000000ULL,
  10000000000000ULL,
  100000000000000ULL,
  1000000000000000ULL,
  10000000000000000ULL,
  100000000000000000ULL,
  1000000000000000000ULL,
  10000000000000000000ULL
}; //=>InnoSQL-5.5.8/strings/ctype-simple.c //2011-10-08
ulonglong
my_strntoull10rnd_8bit(CHARSET_INFO *cs __attribute__((unused)),
                       const char *str, size_t length, int unsigned_flag,
                       char **endptr, int *error) //=>InnoSQL-5.5.8/strings/ctype-simple.c
{
  const char *dot, *end9, *beg, *end= str + length;
  ulonglong ull;
  ulong ul;
  uchar ch;
  int shift= 0, digits= 0, negative, addon;

  /* Skip leading spaces and tabs */
  for ( ; str < end && (*str == ' ' || *str == '\t') ; str++);

  if (str >= end)
    goto ret_edom;

  if ((negative= (*str == '-')) || *str=='+') /* optional sign */
  {
    if (++str == end)
      goto ret_edom;
  }

  beg= str;
  end9= (str + 9) > end ? end : (str + 9);
  /* Accumulate small number into ulong, for performance purposes */
  for (ul= 0 ; str < end9 && (ch= (uchar) (*str - '0')) < 10; str++)
  {
    ul= ul * 10 + ch;
  }

  if (str >= end) /* Small number without dots and expanents */
  {
    *endptr= (char*) str;
    if (negative)
    {
      if (unsigned_flag)
      {
        *error= ul ? MY_ERRNO_ERANGE : 0;
        return 0;
      }
      else
      {
        *error= 0;
        return (ulonglong) (longlong) -(long) ul;
      }
    }
    else
    {
      *error=0;
      return (ulonglong) ul;
    }
  }

  digits= str - beg;

  /* Continue to accumulate into ulonglong */
  for (dot= NULL, ull= ul; str < end; str++)
  {
    if ((ch= (uchar) (*str - '0')) < 10)
    {
      if (ull < CUTOFF || (ull == CUTOFF && ch <= CUTLIM))
      {
        ull= ull * 10 + ch;
        digits++;
        continue;
	}
      /*
        Adding the next digit would overflow.
        Remember the next digit in "addon", for rounding.
        Scan all digits with an optional single dot.
      */
      if (ull == CUTOFF)
      {
        ull= ULONGLONG_MAX;
        addon= 1;
        str++;
      }
      else
        addon= (*str >= '5');

      if (!dot)
      {
        for ( ; str < end && (ch= (uchar) (*str - '0')) < 10; shift++, str++);
        if (str < end && *str == '.')
        {
          str++;
          for ( ; str < end && (ch= (uchar) (*str - '0')) < 10; str++);
        }
      }
      else
      {
        shift= dot - str;
        for ( ; str < end && (ch= (uchar) (*str - '0')) < 10; str++);
      }
      goto exp;
    }

    if (*str == '.')
    {
      if (dot)
      {
        /* The second dot character */
        addon= 0;
        goto exp;
      }
      else
      {
        dot= str + 1;
      }
      continue;
    }

    /* Unknown character, exit the loop */
    break;
  }
  shift= dot ? dot - str : 0; /* Right shift */
  addon= 0;

exp:    /* [ E [ <sign> ] <unsigned integer> ] */

  if (!digits)
  {
    str= beg;
    goto ret_edom;
  }

  if (str < end && (*str == 'e' || *str == 'E'))
  {
    str++;
    if (str < end)
    {
      int negative_exp, exponent;
      if ((negative_exp= (*str == '-')) || *str=='+')
      {
        if (++str == end)
          goto ret_sign;
      }
      for (exponent= 0 ;
           str < end && (ch= (uchar) (*str - '0')) < 10;
           str++)
      {
        exponent= exponent * 10 + ch;
      }
      shift+= negative_exp ? -exponent : exponent;
    }
  }

  if (shift == 0) /* No shift, check addon digit */
  {
    if (addon)
    {
      if (ull == ULONGLONG_MAX)
        goto ret_too_big;
      ull++;
    }
    goto ret_sign;
  }

  if (shift < 0) /* Right shift */
  {
    ulonglong d, r;

    if (-shift >= DIGITS_IN_ULONGLONG)
      goto ret_zero; /* Exponent is a big negative number, return 0 */

    d= d10[-shift];
    r= (ull % d) * 2;
    ull /= d;
    if (r >= d)
      ull++;
    goto ret_sign;
  }

  if (shift > DIGITS_IN_ULONGLONG) /* Huge left shift */
  {
    if (!ull)
      goto ret_sign;
    goto ret_too_big;
  }

  for ( ; shift > 0; shift--, ull*= 10) /* Left shift */
  {
    if (ull > CUTOFF)
      goto ret_too_big; /* Overflow, number too big */
  }

ret_sign:
  *endptr= (char*) str;

  if (!unsigned_flag)
  {
    if (negative)
    {
      if (ull > (ulonglong) LONGLONG_MIN)
      {
        *error= MY_ERRNO_ERANGE;
        return (ulonglong) LONGLONG_MIN;
      }
      *error= 0;
      return (ulonglong) -(longlong) ull;
    }
    else
    {
      if (ull > (ulonglong) LONGLONG_MAX)
      {
        *error= MY_ERRNO_ERANGE;
        return (ulonglong) LONGLONG_MAX;
      }
      *error= 0;
      return ull;
    }
  }

  /* Unsigned number */
  if (negative && ull)
  {
    *error= MY_ERRNO_ERANGE;
    return 0;
  }
  *error= 0;
  return ull;

ret_zero:
  *endptr= (char*) str;
  *error= 0;
  return 0;

ret_edom:
  *endptr= (char*) str;
  *error= MY_ERRNO_EDOM;
  return 0;

ret_too_big:
  *endptr= (char*) str;
  *error= MY_ERRNO_ERANGE;
  return unsigned_flag ?
         ULONGLONG_MAX :
         negative ? (ulonglong) LONGLONG_MIN : (ulonglong) LONGLONG_MAX;
}
size_t my_scan_8bit(CHARSET_INFO *cs, const char *str, const char *end, int sq)
{
  const char *str0= str;
  switch (sq)
  {
  case MY_SEQ_INTTAIL:
    if (*str == '.')
    {
      for(str++ ; str != end && *str == '0' ; str++);
      return (size_t) (str - str0);
    }
    return 0;

  case MY_SEQ_SPACES:
    for ( ; str < end ; str++)
    {
      if (!my_isspace(cs,*str))
        break;
    }
    return (size_t) (str - str0);
  default:
    return 0;
  }
}
//]]] //=>InnoSQL-5.5.8/strings/ctype-simple.c //----2011-09-22-16-20;
//]]
static MY_CHARSET_HANDLER my_charset_handler=
{
    NULL,			/* init */
    NULL,
    my_mbcharlen_8bit,
    my_numchars_8bit,
    my_charpos_8bit,
    my_well_formed_len_8bit,
    my_lengthsp_8bit,
    my_numcells_8bit,
    my_mb_wc_latin1,
    my_wc_mb_latin1,
    my_mb_ctype_8bit,
    my_caseup_str_8bit,
    my_casedn_str_8bit,
    my_caseup_8bit,
    my_casedn_8bit,
    my_snprintf_8bit,
    my_long10_to_str_8bit,
    my_longlong10_to_str_8bit,
    my_fill_8bit,
    my_strntol_8bit,
    my_strntoul_8bit,
    my_strntoll_8bit,
    my_strntoull_8bit,
    my_strntod_8bit,
    my_strtoll10_8bit,
    my_strntoull10rnd_8bit,
    my_scan_8bit
};
//]
//[
//#include "ctype-utf8.c"
/*=>错误：
In file included from ./include2/mysql/service_my_snprintf.h:75,
                 from ./include2/mysql/services.h:21,
                 from ./include2/mysql/plugin.h:51,
                 from ./include2/m_string.h:206,
                 from ctype-utf8.c:22,
                 from test.c:1475:
*/
//] //z  //----2011-09-22-17-03; //zlq
CHARSET_INFO my_charset_latin1=
{
    8,0,0,				/* number    */
    MY_CS_COMPILED | MY_CS_PRIMARY,	/* state     */
    "latin1",				/* cs name    */
    "latin1_swedish_ci",		/* name      */
    "",					/* comment   */
    NULL,				/* tailoring */
    ctype_latin1,
    to_lower_latin1,
    to_upper_latin1,
    sort_order_latin1,
    NULL,		/* contractions */
    NULL,		/* sort_order_big*/
    cs_to_uni,		/* tab_to_uni   */
    NULL,		/* tab_from_uni */
//    my_unicase_default, /* caseinfo     */  //z- //zlq
NULL,
    NULL,		/* state_map    */
    NULL,		/* ident_map    */
    1,			/* strxfrm_multiply */
    1,                  /* caseup_multiply  */
    1,                  /* casedn_multiply  */
    1,			/* mbminlen   */
    1,			/* mbmaxlen  */
    0,			/* min_sort_char */
    255,		/* max_sort_char */
    ' ',                /* pad char      */
    0,                  /* escape_with_backslash_is_dangerous */
    &my_charset_handler,
//  &my_collation_8bit_simple_ci_handler  //z- //zlq  //----2011-09-22-17-05注释掉！！/编译通过；;
NULL
};
//]//=>InnoSQL-5.5.8/strings/ctype-latin1.c
#define ESCAPED_ARG    8
#define PREZERO_ARG    4
#define LENGTH_ARG     1
#define WIDTH_ARG      2
/**
  Calculates print length or index of positional argument

  @param fmt         processed string
  @param length      print length or index of positional argument
  @param pre_zero    returns flags with PREZERO_ARG set if necessary

  @retval
    string position right after length digits
*/
static const char *get_length(const char *fmt, size_t *length, uint *pre_zero)
{
  for (; my_isdigit(&my_charset_latin1, *fmt); fmt++)
  {
    *length= *length * 10 + (uint)(*fmt - '0');
    if (!*length)
      *pre_zero|= PREZERO_ARG;                  /* first digit was 0 */
  }
  return fmt;
}
struct pos_arg_info
{
  char arg_type;                              /* argument type */
  uint have_longlong;                         /* used from integer values */
  char *str_arg;                              /* string value of the arg */
  longlong longlong_arg;                      /* integer value of the arg */
  double double_arg;                          /* double value of the arg */
};
typedef struct pos_arg_info ARGS_INFO;
struct print_info
{
  char arg_type;                              /* argument type */
  size_t arg_idx;                             /* index of the positional arg */
  size_t length;                              /* print width or arg index */
  size_t width;                               /* print width or arg index */
  uint flags;
  const char *begin;                          /**/
  const char *end;                            /**/
};
typedef struct print_info PRINT_INFO;
#define MAX_ARGS 32                           /* max positional args count*/
#define MAX_PRINT_INFO 32                     /* max print position count */
static const char *get_width(const char *fmt, size_t *width); //z+ //否则导致“test.c:864: 错误：与‘get_width’类型冲突 .. test.c:696: 附注：‘get_width’的上一个隐式声明在此”错误 //----2011-09-22-14-55--14-58！; //zlq
static const char *check_longlong(const char *fmt, uint *have_longlong);
static char *process_str_arg(CHARSET_INFO *cs, char *to, char *end, size_t width, char *par, uint print_type);
static char *process_bin_arg(char *to, char *end, size_t width, char *par);
static char *process_dbl_arg(char *to, char *end, size_t width, double par, char arg_type);
static char *process_int_arg(char *to, char *end, size_t length, longlong par, char arg_type, uint print_type);
/**
  Procesed positional arguments.

  @param cs         string charset
  @param to         buffer where processed string will be place
  @param end        end of buffer
  @param par        format string
  @param arg_index  arg index of the first occurrence of positional arg
  @param ap         list of parameters

  @retval
    end of buffer where processed string is placed
*/
static char *process_args(CHARSET_INFO *cs, char *to, char *end,
                          const char* fmt, size_t arg_index, va_list ap)
{
  ARGS_INFO args_arr[MAX_ARGS];
  PRINT_INFO print_arr[MAX_PRINT_INFO];
  uint idx= 0, arg_count= arg_index;

start:
  /* Here we are at the beginning of positional argument, right after $ */
  arg_index--;
  print_arr[idx].flags= 0;
  if (*fmt == '`')
  {
    print_arr[idx].flags|= ESCAPED_ARG;
    fmt++;
  }
  if (*fmt == '-')
    fmt++;
  print_arr[idx].length= print_arr[idx].width= 0;
  /* Get print length */
  if (*fmt == '*')
  {
    fmt++;
    fmt= get_length(fmt, &print_arr[idx].length, &print_arr[idx].flags);
    print_arr[idx].length--;
    DBUG_ASSERT(*fmt == '$' && print_arr[idx].length < MAX_ARGS);
    args_arr[print_arr[idx].length].arg_type= 'd';
    print_arr[idx].flags|= LENGTH_ARG;
    arg_count= max(arg_count, print_arr[idx].length + 1);
    fmt++;
  }
  else
    fmt= get_length(fmt, &print_arr[idx].length, &print_arr[idx].flags);

  if (*fmt == '.')
  {
    fmt++;
    /* Get print width */
    if (*fmt == '*')
    {
      fmt++;
      fmt= get_width(fmt, &print_arr[idx].width);
      print_arr[idx].width--;
      DBUG_ASSERT(*fmt == '$' && print_arr[idx].width < MAX_ARGS);
      args_arr[print_arr[idx].width].arg_type= 'd';
      print_arr[idx].flags|= WIDTH_ARG;
      arg_count= max(arg_count, print_arr[idx].width + 1);
      fmt++;
    }
    else
      fmt= get_width(fmt, &print_arr[idx].width);
  }
  else
    print_arr[idx].width= SIZE_T_MAX;

  fmt= check_longlong(fmt, &args_arr[arg_index].have_longlong);
  if (*fmt == 'p')
    args_arr[arg_index].have_longlong= (sizeof(void *) == sizeof(longlong));
  args_arr[arg_index].arg_type= print_arr[idx].arg_type= *fmt;

  print_arr[idx].arg_idx= arg_index;
  print_arr[idx].begin= ++fmt;

  while (*fmt && *fmt != '%')
    fmt++;

  if (!*fmt)                                  /* End of format string */
  {
    uint i;
    print_arr[idx].end= fmt;
    /* Obtain parameters from the list */
    for (i= 0 ; i < arg_count; i++)
    {
      switch (args_arr[i].arg_type) {
      case 's':
      case 'b':
        args_arr[i].str_arg= va_arg(ap, char *);
        break;
      case 'f':
      case 'g':
        args_arr[i].double_arg= va_arg(ap, double);
        break;
      case 'd':
      case 'i':
      case 'u':
      case 'x':
      case 'X':
      case 'o':
      case 'p':
        if (args_arr[i].have_longlong)
          args_arr[i].longlong_arg= va_arg(ap,longlong);
        else if (args_arr[i].arg_type == 'd' || args_arr[i].arg_type == 'i')
          args_arr[i].longlong_arg= va_arg(ap, int);
        else
          args_arr[i].longlong_arg= va_arg(ap, uint);
        break;
      case 'c':
        args_arr[i].longlong_arg= va_arg(ap, int);
        break;
      default:
        DBUG_ASSERT(0);
      }
    }
    /* Print result string */
    for (i= 0; i <= idx; i++)
    {
      size_t width= 0, length= 0;
      switch (print_arr[i].arg_type) {
      case 's':
      {
        char *par= args_arr[print_arr[i].arg_idx].str_arg;
        width= (print_arr[i].flags & WIDTH_ARG)
          ? (size_t)args_arr[print_arr[i].width].longlong_arg
          : print_arr[i].width;
        to= process_str_arg(cs, to, end, width, par, print_arr[i].flags);
        break;
      }
      case 'b':
      {
        char *par = args_arr[print_arr[i].arg_idx].str_arg;
        width= (print_arr[i].flags & WIDTH_ARG)
          ? (size_t)args_arr[print_arr[i].width].longlong_arg
          : print_arr[i].width;
        to= process_bin_arg(to, end, width, par);
        break;
      }
      case 'c':
      {
        if (to == end)
          break;
        *to++= (char) args_arr[print_arr[i].arg_idx].longlong_arg;
        break;
      }
      case 'f':
      case 'g':
      {
        double d= args_arr[print_arr[i].arg_idx].double_arg;
        width= (print_arr[i].flags & WIDTH_ARG) ?
          (uint)args_arr[print_arr[i].width].longlong_arg : print_arr[i].width;
        to= process_dbl_arg(to, end, width, d, print_arr[i].arg_type);
        break;
      }
      case 'd':
      case 'i':
      case 'u':
      case 'x':
      case 'X':
      case 'o':
      case 'p':
      {
        /* Integer parameter */
        longlong larg;
        length= (print_arr[i].flags & LENGTH_ARG)
          ? (size_t)args_arr[print_arr[i].length].longlong_arg
          : print_arr[i].length;

        if (args_arr[print_arr[i].arg_idx].have_longlong)
          larg = args_arr[print_arr[i].arg_idx].longlong_arg;
        else if (print_arr[i].arg_type == 'd' || print_arr[i].arg_type == 'i' )
          larg = (int) args_arr[print_arr[i].arg_idx].longlong_arg;
        else
          larg= (uint) args_arr[print_arr[i].arg_idx].longlong_arg;

        to= process_int_arg(to, end, length, larg, print_arr[i].arg_type,
                            print_arr[i].flags);
        break;
      }
      default:
        break;
      }

      if (to == end)
        break;

      length= min(end - to , print_arr[i].end - print_arr[i].begin);
      if (to + length < end)
        length++;
      to= strnmov(to, print_arr[i].begin, length);
    }
    DBUG_ASSERT(to <= end);
    *to='\0';				/* End of errmessage */
    return to;
  }
  else
  {
    /* Process next positional argument*/
    DBUG_ASSERT(*fmt == '%');
    print_arr[idx].end= fmt - 1;
    idx++;
    fmt++;
    arg_index= 0;
    fmt= get_width(fmt, &arg_index);
    DBUG_ASSERT(*fmt == '$');
    fmt++;
    arg_count= max(arg_count, arg_index);
    goto start;
  }

  return 0;
}
/**
  Calculates print width or index of positional argument

  @param fmt         processed string
  @param width       print width or index of positional argument

  @retval
    string position right after width digits
*/
static const char *get_width(const char *fmt, size_t *width)
{
  for (; my_isdigit(&my_charset_latin1, *fmt); fmt++)
  {
    *width= *width * 10 + (uint)(*fmt - '0');
  }
  return fmt;
}
/**
  Calculates print width or index of positional argument

  @param fmt            processed string
  @param have_longlong  TRUE if longlong is required

  @retval
    string position right after modifier symbol
*/
static const char *check_longlong(const char *fmt, uint *have_longlong)
{
  *have_longlong= 0;
  if (*fmt == 'l')
  {
    fmt++;
    if (*fmt != 'l')
      *have_longlong= (sizeof(long) == sizeof(longlong));
    else
    {
      fmt++;
      *have_longlong= 1;
    }
  }
  else if (*fmt == 'z')
  {
    fmt++;
    *have_longlong= (sizeof(size_t) == sizeof(longlong));
  }
  return fmt;
}
//[
/**
  Returns escaped string

  @param cs         string charset
  @param to         buffer where escaped string will be placed
  @param end        end of buffer
  @param par        string to escape
  @param par_len    string length
  @param quote_char character for quoting

  @retval
    position in buffer which points on the end of escaped string
*/

static char *backtick_string(CHARSET_INFO *cs, char *to, char *end,
                             char *par, size_t par_len, char quote_char)
{
  uint char_len;
  char *start= to;
  char *par_end= par + par_len;
  size_t buff_length= (size_t) (end - to);

  if (buff_length <= par_len)
    goto err;
  *start++= quote_char;

  for ( ; par < par_end; par+= char_len)
  {
    uchar c= *(uchar *) par;
    if (!(char_len= my_mbcharlen(cs, c)))
      char_len= 1;
    if (char_len == 1 && c == (uchar) quote_char )
    {
      if (start + 1 >= end)
        goto err;
      *start++= quote_char;
    }
    if (start + char_len >= end)
      goto err;
    start= strnmov(start, par, char_len);
  }

  if (start + 1 >= end)
    goto err;
  *start++= quote_char;
  return start;

err:
    *to='\0';
  return to;
}
/**
  Prints string argument
*/
static char *process_str_arg(CHARSET_INFO *cs, char *to, char *end,
                             size_t width, char *par, uint print_type)
{
  int well_formed_error;
  size_t plen, left_len= (size_t) (end - to) + 1;
  if (!par)
    par = (char*) "(null)";

  plen= strnlen(par, width);
  if (left_len <= plen)
    plen = left_len - 1;
  plen= cs->cset->well_formed_len(cs, par, par + plen,
                                  width, &well_formed_error);
  if (print_type & ESCAPED_ARG)
    to= backtick_string(cs, to, end, par, plen, '`');
  else
    to= strnmov(to,par,plen);
  return to;
}
//]
/**
  Prints binary argument
*/
static char *process_bin_arg(char *to, char *end, size_t width, char *par)
{
  DBUG_ASSERT(to <= end);
  if (to + width + 1 > end)
    width= end - to - 1;  /* sign doesn't matter */
  memmove(to, par, width);
  to+= width;
  return to;
}
//#include "dtoa.c" //z+  //=>my_fcvt/my_gcvt //zlq
//[
//[[strings/dtoa.c:  //2011-10-08
//#include <m_string.h>  /* for memcpy and NOT_FIXED_DEC */
#define NOT_FIXED_DEC 31
#define DTOA_OVERFLOW 9999 /* Magic value returned by dtoa() to indicate overflow */
//-
/*
  This is to place return value of dtoa in: tries to use stack
  as well, but passes by free lists management and just aligns len by
  the pointer size in order to not break the alignment rules when storing a
  pointer to a Bigint.
*/
static char *dtoa_alloc(int i, Stack_alloc *alloc)
{
  char *rv;
  int aligned_size= MY_ALIGN(i, SIZEOF_CHARP);
  if(alloc->free + aligned_size <= alloc->end){
    rv= alloc->free;
    alloc->free+= aligned_size;
  }else{
    //rv= malloc(i); //----2011-10-08-17-06更改为：
    rv=(char*)malloc(i);
  }
  return rv;
}
static int quorem(Bigint *b, Bigint *S)
{
  int n;
  ULong *bx, *bxe, q, *sx, *sxe;
  ULLong borrow, carry, y, ys;

  n= S->wds;
  if (b->wds < n)
    return 0;
  sx= S->p.x;
  sxe= sx + --n;
  bx= b->p.x;
  bxe= bx + n;
  q= *bxe / (*sxe + 1);  /* ensure q <= true quotient */
  if (q)
  {
    borrow= 0;
    carry= 0;
    do
    {
      ys= *sx++ * (ULLong)q + carry;
      carry= ys >> 32;
      y= *bx - (ys & FFFFFFFF) - borrow;
      borrow= y >> 32 & (ULong)1;
      *bx++= (ULong) (y & FFFFFFFF);
    }
    while (sx <= sxe);
    if (!*bxe)
    {
      bx= b->p.x;
      while (--bxe > bx && !*bxe)
        --n;
      b->wds= n;
    }
  }
  if (cmp(b, S) >= 0)
  {
    q++;
    borrow= 0;
    carry= 0;
    bx= b->p.x;
    sx= S->p.x;
    do
    {
      ys= *sx++ + carry;
      carry= ys >> 32;
      y= *bx - (ys & FFFFFFFF) - borrow;
      borrow= y >> 32 & (ULong)1;
      *bx++= (ULong) (y & FFFFFFFF);
    }
    while (sx <= sxe);
    bx= b->p.x;
    bxe= bx + n;
    if (!*bxe)
    {
      while (--bxe > bx && !*bxe)
        --n;
      b->wds= n;
    }
  }
  return q;
}
/*
   dtoa for IEEE arithmetic (dmg): convert double to ASCII string.

   Inspired by "How to Print Floating-Point Numbers Accurately" by
   Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 112-126].

   Modifications:
        1. Rather than iterating, we use a simple numeric overestimate
           to determine k= floor(log10(d)).  We scale relevant
           quantities using O(log2(k)) rather than O(k) multiplications.
        2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
           try to generate digits strictly left to right.  Instead, we
           compute with fewer bits and propagate the carry if necessary
           when rounding the final digit up.  This is often faster.
        3. Under the assumption that input will be rounded nearest,
           mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
           That is, we allow equality in stopping tests when the
           round-nearest rule will give the same floating-point value
           as would satisfaction of the stopping test with strict
           inequality.
        4. We remove common factors of powers of 2 from relevant
           quantities.
        5. When converting floating-point integers less than 1e16,
           we use floating-point arithmetic rather than resorting
           to multiple-precision integers.
        6. When asked to produce fewer than 15 digits, we first try
           to get by with floating-point arithmetic; we resort to
           multiple-precision integer arithmetic only if we cannot
           guarantee that the floating-point calculation has given
           the correctly rounded result.  For k requested digits and
           "uniformly" distributed input, the probability is
           something like 10^(k-15) that we must resort to the Long
           calculation.
 */
static char *dtoa(double dd, int mode, int ndigits, int *decpt, int *sign,
                  char **rve, char *buf, size_t buf_size)
{
  /*
    Arguments ndigits, decpt, sign are similar to those
    of ecvt and fcvt; trailing zeros are suppressed from
    the returned string.  If not null, *rve is set to point
    to the end of the return value.  If d is +-Infinity or NaN,
    then *decpt is set to DTOA_OVERFLOW.

    mode:
          0 ==> shortest string that yields d when read in
                and rounded to nearest.
          1 ==> like 0, but with Steele & White stopping rule;
                e.g. with IEEE P754 arithmetic , mode 0 gives
                1e23 whereas mode 1 gives 9.999999999999999e22.
          2 ==> max(1,ndigits) significant digits.  This gives a
                return value similar to that of ecvt, except
                that trailing zeros are suppressed.
          3 ==> through ndigits past the decimal point.  This
                gives a return value similar to that from fcvt,
                except that trailing zeros are suppressed, and
                ndigits can be negative.
          4,5 ==> similar to 2 and 3, respectively, but (in
                round-nearest mode) with the tests of mode 0 to
                possibly return a shorter string that rounds to d.
                With IEEE arithmetic and compilation with
                -DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
                as modes 2 and 3 when FLT_ROUNDS != 1.
          6-9 ==> Debugging modes similar to mode - 4:  don't try
                fast floating-point estimate (if applicable).

      Values of mode other than 0-9 are treated as mode 0.

    Sufficient space is allocated to the return value
    to hold the suppressed trailing zeros.
  */

  int bbits, b2, b5, be, dig, i, ieps, UNINIT_VAR(ilim), ilim0,
    UNINIT_VAR(ilim1), j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
    spec_case, try_quick;
  Long L;
  int denorm;
  ULong x;
  Bigint *b, *b1, *delta, *mlo, *mhi, *S;
  U d2, eps, u;
  double ds;
  char *s, *s0;
#ifdef Honor_FLT_ROUNDS
  int rounding;
#endif
  Stack_alloc alloc;

  alloc.begin= alloc.free= buf;
  alloc.end= buf + buf_size;
  memset(alloc.freelist, 0, sizeof(alloc.freelist));

  u.d= dd;
  if (word0(&u) & Sign_bit)
  {
    /* set sign for everything, including 0's and NaNs */
    *sign= 1;
    word0(&u) &= ~Sign_bit;  /* clear sign bit */
  }
  else
    *sign= 0;

  /* If infinity, set decpt to DTOA_OVERFLOW, if 0 set it to 1 */
  if (((word0(&u) & Exp_mask) == Exp_mask && (*decpt= DTOA_OVERFLOW)) ||
      (!dval(&u) && (*decpt= 1)))
  {
    /* Infinity, NaN, 0 */
    char *res= (char*) dtoa_alloc(2, &alloc);
    res[0]= '0';
    res[1]= '\0';
    if (rve)
      *rve= res + 1;
    return res;
  }

#ifdef Honor_FLT_ROUNDS
  if ((rounding= Flt_Rounds) >= 2)
  {
    if (*sign)
      rounding= rounding == 2 ? 0 : 2;
    else
      if (rounding != 2)
        rounding= 0;
  }
#endif

  b= d2b(&u, &be, &bbits, &alloc);
  if ((i= (int)(word0(&u) >> Exp_shift1 & (Exp_mask>>Exp_shift1))))
  {
    dval(&d2)= dval(&u);
    word0(&d2) &= Frac_mask1;
    word0(&d2) |= Exp_11;

    /*
      log(x)       ~=~ log(1.5) + (x-1.5)/1.5
      log10(x)      =  log(x) / log(10)
                   ~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
      log10(d)= (i-Bias)*log(2)/log(10) + log10(d2)

      This suggests computing an approximation k to log10(d) by

      k= (i - Bias)*0.301029995663981
           + ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );

      We want k to be too large rather than too small.
      The error in the first-order Taylor series approximation
      is in our favor, so we just round up the constant enough
      to compensate for any error in the multiplication of
      (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
      and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
      adding 1e-13 to the constant term more than suffices.
      Hence we adjust the constant term to 0.1760912590558.
      (We could get a more accurate k by invoking log10,
       but this is probably not worthwhile.)
    */

    i-= Bias;
    denorm= 0;
  }
  else
  {
    /* d is denormalized */

    i= bbits + be + (Bias + (P-1) - 1);
    x= i > 32  ? word0(&u) << (64 - i) | word1(&u) >> (i - 32)
      : word1(&u) << (32 - i);
    dval(&d2)= x;
    word0(&d2)-= 31*Exp_msk1; /* adjust exponent */
    i-= (Bias + (P-1) - 1) + 1;
    denorm= 1;
  }
  ds= (dval(&d2)-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
  k= (int)ds;
  if (ds < 0. && ds != k)
    k--;    /* want k= floor(ds) */
  k_check= 1;
  if (k >= 0 && k <= Ten_pmax)
  {
    if (dval(&u) < tens[k])
      k--;
    k_check= 0;
  }
  j= bbits - i - 1;
  if (j >= 0)
  {
    b2= 0;
    s2= j;
  }
  else
  {
    b2= -j;
    s2= 0;
  }
  if (k >= 0)
  {
    b5= 0;
    s5= k;
    s2+= k;
  }
  else
  {
    b2-= k;
    b5= -k;
    s5= 0;
  }
  if (mode < 0 || mode > 9)
    mode= 0;

#ifdef Check_FLT_ROUNDS
  try_quick= Rounding == 1;
#else
  try_quick= 1;
#endif

  if (mode > 5)
  {
    mode-= 4;
    try_quick= 0;
  }
  leftright= 1;
  switch (mode) {
  case 0:
  case 1:
    ilim= ilim1= -1;
    i= 18;
    ndigits= 0;
    break;
  case 2:
    leftright= 0;
    /* no break */
  case 4:
    if (ndigits <= 0)
      ndigits= 1;
    ilim= ilim1= i= ndigits;
    break;
  case 3:
    leftright= 0;
    /* no break */
  case 5:
    i= ndigits + k + 1;
    ilim= i;
    ilim1= i - 1;
    if (i <= 0)
      i= 1;
  }
  s= s0= dtoa_alloc(i, &alloc);

#ifdef Honor_FLT_ROUNDS
  if (mode > 1 && rounding != 1)
    leftright= 0;
#endif

  if (ilim >= 0 && ilim <= Quick_max && try_quick)
  {
    /* Try to get by with floating-point arithmetic. */
    i= 0;
    dval(&d2)= dval(&u);
    k0= k;
    ilim0= ilim;
    ieps= 2; /* conservative */
    if (k > 0)
    {
      ds= tens[k&0xf];
      j= k >> 4;
      if (j & Bletch)
      {
        /* prevent overflows */
        j&= Bletch - 1;
        dval(&u)/= bigtens[n_bigtens-1];
        ieps++;
      }
      for (; j; j>>= 1, i++)
      {
        if (j & 1)
        {
          ieps++;
          ds*= bigtens[i];
        }
      }
      dval(&u)/= ds;
    }
    else if ((j1= -k))
    {
      dval(&u)*= tens[j1 & 0xf];
      for (j= j1 >> 4; j; j>>= 1, i++)
      {
        if (j & 1)
        {
          ieps++;
          dval(&u)*= bigtens[i];
        }
      }
    }
    if (k_check && dval(&u) < 1. && ilim > 0)
    {
      if (ilim1 <= 0)
        goto fast_failed;
      ilim= ilim1;
      k--;
      dval(&u)*= 10.;
      ieps++;
    }
    dval(&eps)= ieps*dval(&u) + 7.;
    word0(&eps)-= (P-1)*Exp_msk1;
    if (ilim == 0)
    {
      S= mhi= 0;
      dval(&u)-= 5.;
      if (dval(&u) > dval(&eps))
        goto one_digit;
      if (dval(&u) < -dval(&eps))
        goto no_digits;
      goto fast_failed;
    }
    if (leftright)
    {
      /* Use Steele & White method of only generating digits needed. */
      dval(&eps)= 0.5/tens[ilim-1] - dval(&eps);
      for (i= 0;;)
      {
        L= (Long) dval(&u);
        dval(&u)-= L;
        *s++= '0' + (int)L;
        if (dval(&u) < dval(&eps))
          goto ret1;
        if (1. - dval(&u) < dval(&eps))
          goto bump_up;
        if (++i >= ilim)
          break;
        dval(&eps)*= 10.;
        dval(&u)*= 10.;
      }
    }
    else
    {
      /* Generate ilim digits, then fix them up. */
      dval(&eps)*= tens[ilim-1];
      for (i= 1;; i++, dval(&u)*= 10.)
      {
        L= (Long)(dval(&u));
        if (!(dval(&u)-= L))
          ilim= i;
        *s++= '0' + (int)L;
        if (i == ilim)
        {
          if (dval(&u) > 0.5 + dval(&eps))
            goto bump_up;
          else if (dval(&u) < 0.5 - dval(&eps))
          {
            while (*--s == '0');
            s++;
            goto ret1;
          }
          break;
        }
      }
    }
  fast_failed:
    s= s0;
    dval(&u)= dval(&d2);
    k= k0;
    ilim= ilim0;
  }

  /* Do we have a "small" integer? */

  if (be >= 0 && k <= Int_max)
  {
    /* Yes. */
    ds= tens[k];
    if (ndigits < 0 && ilim <= 0)
    {
      S= mhi= 0;
      if (ilim < 0 || dval(&u) <= 5*ds)
        goto no_digits;
      goto one_digit;
    }
    for (i= 1;; i++, dval(&u)*= 10.)
    {
      L= (Long)(dval(&u) / ds);
      dval(&u)-= L*ds;
#ifdef Check_FLT_ROUNDS
      /* If FLT_ROUNDS == 2, L will usually be high by 1 */
      if (dval(&u) < 0)
      {
        L--;
        dval(&u)+= ds;
      }
#endif
      *s++= '0' + (int)L;
      if (!dval(&u))
      {
        break;
      }
      if (i == ilim)
      {
#ifdef Honor_FLT_ROUNDS
        if (mode > 1)
        {
          switch (rounding) {
          case 0: goto ret1;
          case 2: goto bump_up;
          }
        }
#endif
        dval(&u)+= dval(&u);
        if (dval(&u) > ds || (dval(&u) == ds && L & 1))
        {
bump_up:
          while (*--s == '9')
            if (s == s0)
            {
              k++;
              *s= '0';
              break;
            }
          ++*s++;
        }
        break;
      }
    }
    goto ret1;
  }

  m2= b2;
  m5= b5;
  mhi= mlo= 0;
  if (leftright)
  {
    i = denorm ? be + (Bias + (P-1) - 1 + 1) : 1 + P - bbits;
    b2+= i;
    s2+= i;
    mhi= i2b(1, &alloc);
  }
  if (m2 > 0 && s2 > 0)
  {
    i= m2 < s2 ? m2 : s2;
    b2-= i;
    m2-= i;
    s2-= i;
  }
  if (b5 > 0)
  {
    if (leftright)
    {
      if (m5 > 0)
      {
        mhi= pow5mult(mhi, m5, &alloc);
        b1= mult(mhi, b, &alloc);
        Bfree(b, &alloc);
        b= b1;
      }
      if ((j= b5 - m5))
        b= pow5mult(b, j, &alloc);
    }
    else
      b= pow5mult(b, b5, &alloc);
  }
  S= i2b(1, &alloc);
  if (s5 > 0)
    S= pow5mult(S, s5, &alloc);

  /* Check for special case that d is a normalized power of 2. */

  spec_case= 0;
  if ((mode < 2 || leftright)
#ifdef Honor_FLT_ROUNDS
      && rounding == 1
#endif
     )
  {
    if (!word1(&u) && !(word0(&u) & Bndry_mask) &&
        word0(&u) & (Exp_mask & ~Exp_msk1)
       )
    {
      /* The special case */
      b2+= Log2P;
      s2+= Log2P;
      spec_case= 1;
    }
  }

  /*
    Arrange for convenient computation of quotients:
    shift left if necessary so divisor has 4 leading 0 bits.

    Perhaps we should just compute leading 28 bits of S once
    a nd for all and pass them and a shift to quorem, so it
    can do shifts and ors to compute the numerator for q.
  */
  if ((i= ((s5 ? 32 - hi0bits(S->p.x[S->wds-1]) : 1) + s2) & 0x1f))
    i= 32 - i;
  if (i > 4)
  {
    i-= 4;
    b2+= i;
    m2+= i;
    s2+= i;
  }
  else if (i < 4)
  {
    i+= 28;
    b2+= i;
    m2+= i;
    s2+= i;
  }
  if (b2 > 0)
    b= lshift(b, b2, &alloc);
  if (s2 > 0)
    S= lshift(S, s2, &alloc);
  if (k_check)
  {
    if (cmp(b,S) < 0)
    {
      k--;
      /* we botched the k estimate */
      b= multadd(b, 10, 0, &alloc);
      if (leftright)
        mhi= multadd(mhi, 10, 0, &alloc);
      ilim= ilim1;
    }
  }
  if (ilim <= 0 && (mode == 3 || mode == 5))
  {
    if (ilim < 0 || cmp(b,S= multadd(S,5,0, &alloc)) <= 0)
    {
      /* no digits, fcvt style */
no_digits:
      k= -1 - ndigits;
      goto ret;
    }
one_digit:
    *s++= '1';
    k++;
    goto ret;
  }
  if (leftright)
  {
    if (m2 > 0)
      mhi= lshift(mhi, m2, &alloc);

    /*
      Compute mlo -- check for special case that d is a normalized power of 2.
    */

    mlo= mhi;
    if (spec_case)
    {
      mhi= Balloc(mhi->k, &alloc);
      Bcopy(mhi, mlo);
      mhi= lshift(mhi, Log2P, &alloc);
    }

    for (i= 1;;i++)
    {
      dig= quorem(b,S) + '0';
      /* Do we yet have the shortest decimal string that will round to d? */
      j= cmp(b, mlo);
      delta= diff(S, mhi, &alloc);
      j1= delta->sign ? 1 : cmp(b, delta);
      Bfree(delta, &alloc);
      if (j1 == 0 && mode != 1 && !(word1(&u) & 1)
#ifdef Honor_FLT_ROUNDS
          && rounding >= 1
#endif
         )
      {
        if (dig == '9')
          goto round_9_up;
        if (j > 0)
          dig++;
        *s++= dig;
        goto ret;
      }
      if (j < 0 || (j == 0 && mode != 1 && !(word1(&u) & 1)))
      {
        if (!b->p.x[0] && b->wds <= 1)
        {
          goto accept_dig;
        }
#ifdef Honor_FLT_ROUNDS
        if (mode > 1)
          switch (rounding) {
          case 0: goto accept_dig;
          case 2: goto keep_dig;
          }
#endif /*Honor_FLT_ROUNDS*/
        if (j1 > 0)
        {
          b= lshift(b, 1, &alloc);
          j1= cmp(b, S);
          if ((j1 > 0 || (j1 == 0 && dig & 1))
              && dig++ == '9')
            goto round_9_up;
        }
accept_dig:
        *s++= dig;
        goto ret;
      }
      if (j1 > 0)
      {
#ifdef Honor_FLT_ROUNDS
        if (!rounding)
          goto accept_dig;
#endif
        if (dig == '9')
        { /* possible if i == 1 */
round_9_up:
          *s++= '9';
          goto roundoff;
        }
        *s++= dig + 1;
        goto ret;
      }
#ifdef Honor_FLT_ROUNDS
keep_dig:
#endif
      *s++= dig;
      if (i == ilim)
        break;
      b= multadd(b, 10, 0, &alloc);
      if (mlo == mhi)
        mlo= mhi= multadd(mhi, 10, 0, &alloc);
      else
      {
        mlo= multadd(mlo, 10, 0, &alloc);
        mhi= multadd(mhi, 10, 0, &alloc);
      }
    }
  }
  else
    for (i= 1;; i++)
    {
      *s++= dig= quorem(b,S) + '0';
      if (!b->p.x[0] && b->wds <= 1)
      {
        goto ret;
      }
      if (i >= ilim)
        break;
      b= multadd(b, 10, 0, &alloc);
    }

  /* Round off last digit */

#ifdef Honor_FLT_ROUNDS
  switch (rounding) {
  case 0: goto trimzeros;
  case 2: goto roundoff;
  }
#endif
  b= lshift(b, 1, &alloc);
  j= cmp(b, S);
  if (j > 0 || (j == 0 && dig & 1))
  {
roundoff:
    while (*--s == '9')
      if (s == s0)
      {
        k++;
        *s++= '1';
        goto ret;
      }
    ++*s++;
  }
  else
  {
#ifdef Honor_FLT_ROUNDS
trimzeros:
#endif
    while (*--s == '0');
    s++;
  }
ret:
  Bfree(S, &alloc);
  if (mhi)
  {
    if (mlo && mlo != mhi)
      Bfree(mlo, &alloc);
    Bfree(mhi, &alloc);
  }
ret1:
  Bfree(b, &alloc);
  *s= 0;
  *decpt= k + 1;
  if (rve)
    *rve= s;
  return s0;
}
/*
  dtoa_free() must be used to free values s returned by dtoa()
  This is the counterpart of dtoa_alloc()
*/
static void dtoa_free(char *gptr, char *buf, size_t buf_size)
{
  if (gptr < buf || gptr >= buf + buf_size)
    free(gptr);
}
/**
   @brief
   Converts a given floating point number to a zero-terminated string
   representation using the 'f' format.

   @details
   This function is a wrapper around dtoa() to do the same as
   sprintf(to, "%-.*f", precision, x), though the conversion is usually more
   precise. The only difference is in handling [-,+]infinity and nan values,
   in which case we print '0\0' to the output string and indicate an overflow.

   @param x           the input floating point number.
   @param precision   the number of digits after the decimal point.
                      All properties of sprintf() apply:
                      - if the number of significant digits after the decimal
                        point is less than precision, the resulting string is
                        right-padded with zeros
                      - if the precision is 0, no decimal point appears
                      - if a decimal point appears, at least one digit appears
                        before it
   @param to          pointer to the output buffer. The longest string which
                      my_fcvt() can return is FLOATING_POINT_BUFFER bytes
                      (including the terminating '\0').
   @param error       if not NULL, points to a location where the status of
                      conversion is stored upon return.
                      FALSE  successful conversion
                      TRUE   the input number is [-,+]infinity or nan.
                             The output string in this case is always '0'.
   @return            number of written characters (excluding terminating '\0')
*/
size_t my_fcvt(double x, int precision, char *to, my_bool *error)
{
  int decpt, sign, len, i;
  char *res, *src, *end, *dst= to;
  char buf[DTOA_BUFF_SIZE];
  DBUG_ASSERT(precision >= 0 && precision < NOT_FIXED_DEC && to != NULL);

  res= dtoa(x, 5, precision, &decpt, &sign, &end, buf, sizeof(buf));

  if (decpt == DTOA_OVERFLOW)
  {
    dtoa_free(res, buf, sizeof(buf));
    *to++= '0';
    *to= '\0';
    if (error != NULL)
      *error= TRUE;
    return 1;
  }

  src= res;
  len= end - src;

  if (sign)
    *dst++= '-';

  if (decpt <= 0)
  {
    *dst++= '0';
    *dst++= '.';
    for (i= decpt; i < 0; i++)
      *dst++= '0';
  }

  for (i= 1; i <= len; i++)
  {
    *dst++= *src++;
    if (i == decpt && i < len)
      *dst++= '.';
  }
  while (i++ <= decpt)
    *dst++= '0';

  if (precision > 0)
  {
    if (len <= decpt)
      *dst++= '.';

    for (i= precision - max(0, (len - decpt)); i > 0; i--)
      *dst++= '0';
  }

  *dst= '\0';
  if (error != NULL)
    *error= FALSE;

  dtoa_free(res, buf, sizeof(buf));

  return dst - to;
}

//[[m_string.h:
typedef enum {
  MY_GCVT_ARG_FLOAT,
  MY_GCVT_ARG_DOUBLE
} my_gcvt_arg_type;
#define MAX_DECPT_FOR_F_FORMAT DBL_DIG
//]
/**
   @brief
   Converts a given floating point number to a zero-terminated string
   representation with a given field width using the 'e' format
   (aka scientific notation) or the 'f' one.

   @details
   The format is chosen automatically to provide the most number of significant
   digits (and thus, precision) with a given field width. In many cases, the
   result is similar to that of sprintf(to, "%g", x) with a few notable
   differences:
   - the conversion is usually more precise than C library functions.
   - there is no 'precision' argument. instead, we specify the number of
     characters available for conversion (i.e. a field width).
   - the result never exceeds the specified field width. If the field is too
     short to contain even a rounded decimal representation, my_gcvt()
     indicates overflow and truncates the output string to the specified width.
   - float-type arguments are handled differently than double ones. For a
     float input number (i.e. when the 'type' argument is MY_GCVT_ARG_FLOAT)
     we deliberately limit the precision of conversion by FLT_DIG digits to
     avoid garbage past the significant digits.
   - unlike sprintf(), in cases where the 'e' format is preferred,  we don't
     zero-pad the exponent to save space for significant digits. The '+' sign
     for a positive exponent does not appear for the same reason.

   @param x           the input floating point number.
   @param type        is either MY_GCVT_ARG_FLOAT or MY_GCVT_ARG_DOUBLE.
                      Specifies the type of the input number (see notes above).
   @param width       field width in characters. The minimal field width to
                      hold any number representation (albeit rounded) is 7
                      characters ("-Ne-NNN").
   @param to          pointer to the output buffer. The result is always
                      zero-terminated, and the longest returned string is thus
                      'width + 1' bytes.
   @param error       if not NULL, points to a location where the status of
                      conversion is stored upon return.
                      FALSE  successful conversion
                      TRUE   the input number is [-,+]infinity or nan.
                             The output string in this case is always '0'.
   @return            number of written characters (excluding terminating '\0')

   @todo
   Check if it is possible and  makes sense to do our own rounding on top of
   dtoa() instead of calling dtoa() twice in (rare) cases when the resulting
   string representation does not fit in the specified field width and we want
   to re-round the input number with fewer significant digits. Examples:

     my_gcvt(-9e-3, ..., 4, ...);
     my_gcvt(-9e-3, ..., 2, ...);
     my_gcvt(1.87e-3, ..., 4, ...);
     my_gcvt(55, ..., 1, ...);

   We do our best to minimize such cases by:

   - passing to dtoa() the field width as the number of significant digits

   - removing the sign of the number early (and decreasing the width before
     passing it to dtoa())

   - choosing the proper format to preserve the most number of significant
     digits.
*/
size_t my_gcvt(double x, my_gcvt_arg_type type, int width, char *to,
               my_bool *error)
{
  int decpt, sign, len, exp_len;
  char *res, *src, *end, *dst= to, *dend= dst + width;
  char buf[DTOA_BUFF_SIZE];
  my_bool have_space, force_e_format;
  DBUG_ASSERT(width > 0 && to != NULL);

  /* We want to remove '-' from equations early */
  if (x < 0.)
    width--;

  res= dtoa(x, 4, type == MY_GCVT_ARG_DOUBLE ? width : min(width, FLT_DIG),
            &decpt, &sign, &end, buf, sizeof(buf));
  if (decpt == DTOA_OVERFLOW)
  {
    dtoa_free(res, buf, sizeof(buf));
    *to++= '0';
    *to= '\0';
    if (error != NULL)
      *error= TRUE;
    return 1;
  }

  if (error != NULL)
    *error= FALSE;

  src= res;
  len= end - res;

  /*
    Number of digits in the exponent from the 'e' conversion.
     The sign of the exponent is taken into account separetely, we don't need
     to count it here.
   */
  exp_len= 1 + (decpt >= 101 || decpt <= -99) + (decpt >= 11 || decpt <= -9);

  /*
     Do we have enough space for all digits in the 'f' format?
     Let 'len' be the number of significant digits returned by dtoa,
     and F be the length of the resulting decimal representation.
     Consider the following cases:
     1. decpt <= 0, i.e. we have "0.NNN" => F = len - decpt + 2
     2. 0 < decpt < len, i.e. we have "NNN.NNN" => F = len + 1
     3. len <= decpt, i.e. we have "NNN00" => F = decpt
  */
  have_space= (decpt <= 0 ? len - decpt + 2 :
               decpt > 0 && decpt < len ? len + 1 :
               decpt) <= width;
  /*
    The following is true when no significant digits can be placed with the
    specified field width using the 'f' format, and the 'e' format
    will not be truncated.
  */
  force_e_format= (decpt <= 0 && width <= 2 - decpt && width >= 3 + exp_len);
  /*
    Assume that we don't have enough space to place all significant digits in
    the 'f' format. We have to choose between the 'e' format and the 'f' one
    to keep as many significant digits as possible.
    Let E and F be the lengths of decimal representaion in the 'e' and 'f'
    formats, respectively. We want to use the 'f' format if, and only if F <= E.
    Consider the following cases:
    1. decpt <= 0.
       F = len - decpt + 2 (see above)
       E = len + (len > 1) + 1 + 1 (decpt <= -99) + (decpt <= -9) + 1
       ("N.NNe-MMM")
       (F <= E) <=> (len == 1 && decpt >= -1) || (len > 1 && decpt >= -2)
       We also need to ensure that if the 'f' format is chosen,
       the field width allows us to place at least one significant digit
       (i.e. width > 2 - decpt). If not, we prefer the 'e' format.
    2. 0 < decpt < len
       F = len + 1 (see above)
       E = len + 1 + 1 + ... ("N.NNeMMM")
       F is always less than E.
    3. len <= decpt <= width
       In this case we have enough space to represent the number in the 'f'
       format, so we prefer it with some exceptions.
    4. width < decpt
       The number cannot be represented in the 'f' format at all, always use
       the 'e' 'one.
  */
  if ((have_space ||
      /*
        Not enough space, let's see if the 'f' format provides the most number
        of significant digits.
      */
       ((decpt <= width && (decpt >= -1 || (decpt == -2 &&
                                            (len > 1 || !force_e_format)))) &&
         !force_e_format)) &&

       /*
         Use the 'e' format in some cases even if we have enough space for the
         'f' one. See comment for MAX_DECPT_FOR_F_FORMAT.
       */
      (!have_space || (decpt >= -MAX_DECPT_FOR_F_FORMAT + 1 &&
                       (decpt <= MAX_DECPT_FOR_F_FORMAT || len > decpt))))
  {
    /* 'f' format */
    int i;

    width-= (decpt < len) + (decpt <= 0 ? 1 - decpt : 0);

    /* Do we have to truncate any digits? */
    if (width < len)
    {
      if (width < decpt)
      {
        if (error != NULL)
          *error= TRUE;
        width= decpt;
      }

      /*
        We want to truncate (len - width) least significant digits after the
        decimal point. For this we are calling dtoa with mode=5, passing the
        number of significant digits = (len-decpt) - (len-width) = width-decpt
      */
      dtoa_free(res, buf, sizeof(buf));
      res= dtoa(x, 5, width - decpt, &decpt, &sign, &end, buf, sizeof(buf));
      src= res;
      len= end - res;
    }

    if (len == 0)
    {
      /* Underflow. Just print '0' and exit */
      *dst++= '0';
      goto end;
    }

    /*
      At this point we are sure we have enough space to put all digits
      returned by dtoa
    */
    if (sign && dst < dend)
      *dst++= '-';
    if (decpt <= 0)
    {
      if (dst < dend)
        *dst++= '0';
      if (len > 0 && dst < dend)
        *dst++= '.';
      for (; decpt < 0 && dst < dend; decpt++)
        *dst++= '0';
    }

    for (i= 1; i <= len && dst < dend; i++)
    {
      *dst++= *src++;
      if (i == decpt && i < len && dst < dend)
        *dst++= '.';
    }
    while (i++ <= decpt && dst < dend)
      *dst++= '0';
  }
  else
  {
    /* 'e' format */
    int decpt_sign= 0;

    if (--decpt < 0)
    {
      decpt= -decpt;
      width--;
      decpt_sign= 1;
    }
    width-= 1 + exp_len; /* eNNN */

    if (len > 1)
      width--;

    if (width <= 0)
    {
      /* Overflow */
      if (error != NULL)
        *error= TRUE;
      width= 0;
    }

    /* Do we have to truncate any digits? */
    if (width < len)
    {
      /* Yes, re-convert with a smaller width */
      dtoa_free(res, buf, sizeof(buf));
      res= dtoa(x, 4, width, &decpt, &sign, &end, buf, sizeof(buf));
      src= res;
      len= end - res;
      if (--decpt < 0)
        decpt= -decpt;
    }
    /*
      At this point we are sure we have enough space to put all digits
      returned by dtoa
    */
    if (sign && dst < dend)
      *dst++= '-';
    if (dst < dend)
      *dst++= *src++;
    if (len > 1 && dst < dend)
    {
      *dst++= '.';
      while (src < end && dst < dend)
        *dst++= *src++;
    }
    if (dst < dend)
      *dst++= 'e';
    if (decpt_sign && dst < dend)
      *dst++= '-';

    if (decpt >= 100 && dst < dend)
    {
      *dst++= decpt / 100 + '0';
      decpt%= 100;
      if (dst < dend)
        *dst++= decpt / 10 + '0';
    }
    else if (decpt >= 10 && dst < dend)
      *dst++= decpt / 10 + '0';
    if (dst < dend)
      *dst++= decpt % 10 + '0';

  }

end:
  dtoa_free(res, buf, sizeof(buf));
  *dst= '\0';

  return dst - to;
}
//]]
//]
/**
  Prints double or float argument
*/
static char *process_dbl_arg(char *to, char *end, size_t width,
                             double par, char arg_type)
{
  if (width == SIZE_T_MAX)
    width= FLT_DIG; /* width not set, use default */
  else if (width >= NOT_FIXED_DEC)
    width= NOT_FIXED_DEC - 1; /* max.precision for my_fcvt() */
  width= min(width, (size_t)(end-to) - 1);

  if (arg_type == 'f')
    to+= my_fcvt(par, (int)width , to, NULL);
  else
    to+= my_gcvt(par, MY_GCVT_ARG_DOUBLE, (int) width , to, NULL);
  return to;
}
//[strings/longlong102str.c:  //2011-10-08
//[[strings/int2str.c:
/*
  _dig_vec arrays are public because they are used in several outer places.
*/
char _dig_vec_upper[] =
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char _dig_vec_lower[] =
  "0123456789abcdefghijklmnopqrstuvwxyz";
//]]
#ifndef ll2str
/*
  This assumes that longlong multiplication is faster than longlong division.
*/
char *ll2str(longlong val,char *dst,int radix, int upcase)
{
  char buffer[65];
  register char *p;
  long long_val;
  char *dig_vec= upcase ? _dig_vec_upper : _dig_vec_lower;
  ulonglong uval= (ulonglong) val;

  if (radix < 0)
  {
    if (radix < -36 || radix > -2) return (char*) 0;
    if (val < 0) {
      *dst++ = '-';
      /* Avoid integer overflow in (-val) for LONGLONG_MIN (BUG#31799). */
      uval = (ulonglong)0 - uval;
    }
    radix = -radix;
  }
  else
  {
    if (radix > 36 || radix < 2) return (char*) 0;
  }
  if (uval == 0)
  {
    *dst++='0';
    *dst='\0';
    return dst;
  }
  p = &buffer[sizeof(buffer)-1];
  *p = '\0';

  while (uval > (ulonglong) LONG_MAX)
  {
    ulonglong quo= uval/(uint) radix;
    uint rem= (uint) (uval- quo* (uint) radix);
    *--p= dig_vec[rem];
    uval= quo;
  }
  long_val= (long) uval;
  while (long_val != 0)
  {
    long quo= long_val/radix;
    *--p= dig_vec[(uchar) (long_val - quo*radix)];
    long_val= quo;
  }
  while ((*dst++ = *p++) != 0) ;
  return dst-1;
}
#endif

#ifndef longlong10_to_str
char *longlong10_to_str(longlong val,char *dst,int radix)
{
  char buffer[65];
  register char *p;
  long long_val;
  ulonglong uval= (ulonglong) val;

  if (radix < 0)
  {
    if (val < 0)
    {
      *dst++ = '-';
      /* Avoid integer overflow in (-val) for LONGLONG_MIN (BUG#31799). */
      uval = (ulonglong)0 - uval;
    }
  }

  if (uval == 0)
  {
    *dst++='0';
    *dst='\0';
    return dst;
  }
  p = &buffer[sizeof(buffer)-1];
  *p = '\0';

  while (uval > (ulonglong) LONG_MAX)
  {
    ulonglong quo= uval/(uint) 10;
    uint rem= (uint) (uval- quo* (uint) 10);
    *--p = _dig_vec_upper[rem];
    uval= quo;
  }
  long_val= (long) uval;
  while (long_val != 0)
  {
    long quo= long_val/10;
    *--p = _dig_vec_upper[(uchar) (long_val - quo*10)];
    long_val= quo;
  }
  while ((*dst++ = *p++) != 0) ;
  return dst-1;
}
#endif
//]
//[m_string.h:  //2011-10-08
/*  This is needed for the definitions of bzero... on solaris */
#if defined(HAVE_STRINGS_H)
#include <strings.h>
#endif

/*  This is needed for the definitions of memcpy... on solaris */
#if defined(HAVE_MEMORY_H) && !defined(__cplusplus)
#include <memory.h>
#endif

//#if !defined(HAVE_MEMCPY) && !defined(HAVE_MEMMOVE)  //z- //zlq
//# define memcpy(d, s, n)	bcopy ((s), (d), (n))
//# define memset(A,C,B)		bfill((A),(B),(C))
//# define memmove(d, s, n)	bmove ((d), (s), (n))
//#elif defined(HAVE_MEMMOVE)
# define bmove(d, s, n)		memmove((d), (s), (n))
//#endif  //z- //zlq
//]
/**
  Prints integer argument
*/
static char *process_int_arg(char *to, char *end, size_t length,
                             longlong par, char arg_type, uint print_type)
{
  size_t res_length, to_length;
  char *store_start= to, *store_end;
  char buff[32];

  if ((to_length= (size_t) (end-to)) < 16 || length)
    store_start= buff;

  if (arg_type == 'd' || arg_type == 'i')
    store_end= longlong10_to_str(par, store_start, -10);
  else if (arg_type == 'u')
    store_end= longlong10_to_str(par, store_start, 10);
  else if (arg_type == 'p')
  {
    store_start[0]= '0';
    store_start[1]= 'x';
    store_end= ll2str(par, store_start + 2, 16, 0);
  }
  else if (arg_type == 'o')
  {
    store_end= ll2str(par, store_start, 8, 0);
  }
  else
  {
    DBUG_ASSERT(arg_type == 'X' || arg_type =='x');
    store_end= ll2str(par, store_start, 16, (arg_type == 'X'));
  }

  if ((res_length= (size_t) (store_end - store_start)) > to_length)
    return to;                           /* num doesn't fit in output */
  /* If %#d syntax was used, we have to pre-zero/pre-space the string */
  if (store_start == buff)
  {
    length= min(length, to_length);
    if (res_length < length)
    {
      size_t diff= (length- res_length);
      bfill(to, diff, (print_type & PREZERO_ARG) ? '0' : ' ');
      if (arg_type == 'p' && print_type & PREZERO_ARG)
      {
        if (diff > 1)
          to[1]= 'x';
        else
          store_start[0]= 'x';
        store_start[1]= '0';
      }
      to+= diff;
    }
    bmove(to, store_start, res_length);
  }
  to+= res_length;
  return to;
}
/**
  Produces output string according to a format string

  See the detailed documentation around my_snprintf_service_st

  @param cs         string charset
  @param to         buffer where processed string will be place
  @param n          size of buffer
  @param par        format string
  @param ap         list of parameters

  @retval
    length of result string
*/
size_t my_vsnprintf_ex(CHARSET_INFO *cs, char *to, size_t n,
                       const char* fmt, va_list ap)
{
  char *start=to, *end=to+n-1;
  size_t length, width;
  uint print_type, have_longlong;

  for (; *fmt ; fmt++)
  {
    if (*fmt != '%')
    {
      if (to == end)                            /* End of buffer */
	break;
      *to++= *fmt;                            /* Copy ordinary char */
      continue;
    }
    fmt++;					/* skip '%' */

    length= width= 0;
    print_type= 0;

    /* Read max fill size (only used with %d and %u) */
    if (my_isdigit(&my_charset_latin1, *fmt))
    {
      fmt= get_length(fmt, &length, &print_type);
      if (*fmt == '$')
      {
        to= process_args(cs, to, end, (fmt+1), length, ap);
        return (size_t) (to - start);
      }
    }
    else
    {
      if (*fmt == '`')
      {
        print_type|= ESCAPED_ARG;
        fmt++;
      }
      if (*fmt == '-')
        fmt++;
      if (*fmt == '*')
      {
        fmt++;
        length= va_arg(ap, int);
      }
      else
        fmt= get_length(fmt, &length, &print_type);
    }

    if (*fmt == '.')
    {
      fmt++;
      if (*fmt == '*')
      {
        fmt++;
        width= va_arg(ap, int);
      }
      else
        fmt= get_width(fmt, &width);
    }
    else
      width= SIZE_T_MAX;

    fmt= check_longlong(fmt, &have_longlong);

    if (*fmt == 's')				/* String parameter */
    {
      reg2 char *par= va_arg(ap, char *);
      to= process_str_arg(cs, to, end, width, par, print_type);
      continue;
    }
    else if (*fmt == 'b')				/* Buffer parameter */
    {
      char *par = va_arg(ap, char *);
      to= process_bin_arg(to, end, width, par);
      continue;
    }
    else if (*fmt == 'f' || *fmt == 'g')
    {
      double d= va_arg(ap, double);
      to= process_dbl_arg(to, end, width, d, *fmt);
      continue;
    }
    else if (*fmt == 'd' || *fmt == 'i' || *fmt == 'u' || *fmt == 'x' ||
             *fmt == 'X' || *fmt == 'p' || *fmt == 'o')
    {
      /* Integer parameter */
      longlong larg;
      if (*fmt == 'p')
        have_longlong= (sizeof(void *) == sizeof(longlong));

      if (have_longlong)
        larg = va_arg(ap,longlong);
      else if (*fmt == 'd' || *fmt == 'i')
        larg = va_arg(ap, int);
      else
        larg= va_arg(ap, uint);

      to= process_int_arg(to, end, length, larg, *fmt, print_type);
      continue;
    }
    else if (*fmt == 'c')                       /* Character parameter */
    {
      register int larg;
      if (to == end)
        break;
      larg = va_arg(ap, int);
      *to++= (char) larg;
      continue;
    }

    /* We come here on '%%', unknown code or too long parameter */
    if (to == end)
      break;
    *to++='%';				/* % used as % or unknown code */
  }
  DBUG_ASSERT(to <= end);
  *to='\0';				/* End of errmessage */
  return (size_t) (to - start);
} //=>InnoSQL-5.5.8/strings/my_vsnprintf.c
size_t my_vsnprintf(char *to, size_t n, const char* fmt, va_list ap)
{
  return my_vsnprintf_ex(&my_charset_latin1, to, n, fmt, ap);
} //=>InnoSQL-5.5.8/strings/my_vsnprintf.c
size_t my_snprintf(char* to, size_t n, const char* fmt, ...)
{
  size_t result;
  va_list args;
  va_start(args,fmt);
  result= my_vsnprintf(to, n, fmt, args);
  va_end(args);
  return result;
} //=>InnoSQL-5.5.8/strings/my_vsnprintf.c
//[
//[[dbug/dbug.c:  //2011-10-08
//#define ERR_MISSING_RETURN "missing DBUG_RETURN or DBUG_VOID_RETURN macro in function \"%s\"\n"  //----2011-10-08-15-07与上面的重复！！！；；;
#define ERR_OPEN "%s: can't open debug output stream \"%s\": "
#define ERR_CLOSE "%s: can't close debug file: "
#define ERR_ABORT "%s: debugger aborting because %s\n"

void _db_flush_()
{
  CODE_STATE *cs= NULL;
  get_code_state_or_return;
  (void) fflush(cs->stack->out_file);
}
//]]
#define DBUG_ABORT()                    (_db_flush_(), abort())  //=>my_dbug.h
static void DbugExit(const char *why)
{
  CODE_STATE *cs=code_state();
  (void) fprintf(stderr, ERR_ABORT, cs ? cs->process : "(null)", why);
  (void) fflush(stderr);
  DBUG_ABORT();
}
static BOOLEAN DoProfile(CODE_STATE *cs)
{
/*  return PROFILING &&
         cs->level <= cs->stack->maxdepth &&
         InList(cs->stack->p_functions, cs->func) & (INCLUDE|MATCHED) &&
         InList(cs->stack->processes, cs->process) & (INCLUDE|MATCHED);*/  //z- //zlq
}
static unsigned long Clock()
{
/*    struct rusage ru;

    (void) getrusage(RUSAGE_SELF, &ru);
    return ru.ru_utime.tv_sec*1000 + ru.ru_utime.tv_usec/1000;*/  //z- //zlq
}
static int DoTrace(CODE_STATE *cs)
{
/*  if ((cs->stack->maxdepth == 0 || cs->level <= cs->stack->maxdepth) &&
      InList(cs->stack->processes, cs->process) & (MATCHED|INCLUDE))
    switch(InList(cs->stack->functions, cs->func)) {
    case INCLUDE|SUBDIR:  return ENABLE_TRACE;
    case INCLUDE:         return DO_TRACE;
    case MATCHED|SUBDIR:
    case NOT_MATCHED|SUBDIR:
    case MATCHED:         return framep_trace_flag(cs, cs->framep) ?
                                           DO_TRACE : DONT_TRACE;
    case EXCLUDE:
    case NOT_MATCHED:     return DONT_TRACE;
    case EXCLUDE|SUBDIR:  return DISABLE_TRACE;
    }
  return DONT_TRACE;*/  //z- //zlq
}
static void DoPrefix(CODE_STATE *cs, uint _line_)
{
  //...  //z- //zlq
}
static void DbugFlush(CODE_STATE *cs)
{
/*  if (cs->stack->flags & FLUSH_ON_WRITE)
  {
    (void) fflush(cs->stack->out_file);
    if (cs->stack->delay)
      (void) Delay(cs->stack->delay);
  }
  if (!cs->locked)
  pthread_mutex_unlock(&THR_LOCK_dbug);*/  //z- //zlq
} /* DbugFlush */
static void Indent(CODE_STATE *cs, int indent)
{
/*  REGISTER int count;

  indent= max(indent-1-cs->stack->sub_level,0)*INDENT;
  for (count= 0; count < indent ; count++)
  {
    if ((count % INDENT) == 0)
      fputc('|',cs->stack->out_file);
    else
      fputc(' ',cs->stack->out_file);
  }*/  //z- //zlq
}
//]//InnoSQL-5.5.8/dbug/dbug.c
/*
 *  FUNCTION
 *
 *      _db_return_    process exit from user function
 *
 *  SYNOPSIS
 *
 *      VOID _db_return_(_line_, _stack_frame_)
 *      int _line_;             current source line number
 *      struct _db_stack_frame_ allocated on the caller's stack
 *
 *  DESCRIPTION
 *
 *      Called just before user function executes an explicit or implicit
 *      return.  Prints a trace line if trace is enabled, decrements
 *      the current nesting level, and restores the current function and
 *      file names from the defunct function's stack.
 *
 */
void _db_return_(uint _line_, struct _db_stack_frame_ *_stack_frame_)
{
  int save_errno=errno;
  //
  uint _slevel_= _stack_frame_->level & ~TRACE_ON;
  CODE_STATE *cs;
  get_code_state_or_return;

/*
  if (cs->framep != _stack_frame_)
  {
    char buf[512];
    my_snprintf(buf, sizeof(buf), ERR_MISSING_RETURN, cs->func);
    DbugExit(buf);
  }
#ifndef THREAD
  if (DoProfile(cs))
    (void) fprintf(cs->stack->prof_file, PROF_XFMT, Clock(), cs->func); //=>此函数中定义了“CODE_STATE *cs;” //zlq
#endif
  if (DoTrace(cs) & DO_TRACE)
  {
    if (TRACING)  //=>"#define TRACING (cs->stack->flags & TRACE_ON)" //zlq
    {
      if (!cs->locked)
        pthread_mutex_lock(&THR_LOCK_dbug);
      DoPrefix(cs, _line_);
      Indent(cs, cs->level);
      (void) fprintf(cs->stack->out_file, "<%s\n", cs->func);
      DbugFlush(cs);
    }
  }
  / *
    Check to not set level < 0. This can happen if DBUG was disabled when
    function was entered and enabled in function.
  * /
  cs->level= _slevel_ != 0 ? _slevel_ - 1 : 0;
  cs->func= _stack_frame_->func;
  cs->file= _stack_frame_->file;
  if (cs->framep != NULL)
    cs->framep= cs->framep->prev;
*/ //----2011-10-08-15-18注释！----15-33补充记录；+2011-10-xx-xx:解决此/去掉注释！！；；;
  //
  errno=save_errno;
} //InnoSQL-5.5.8/dbug/dbug.c
//[
//[[my_global.h:  //2011-10-08-11-56==12-00--12-02;
///*
//  MY_FILE_MIN is  Windows speciality and is used to quickly detect
//  the mismatch of CRT and mysys file IO usage on Windows at runtime.
//  CRT file descriptors can be in the range 0-2047, whereas descriptors returned
//  by my_open() will start with 2048. If a file descriptor with value less then
//  MY_FILE_MIN is passed to mysys IO function, chances are it stemms from
//  open()/fileno() and not my_open()/my_fileno.
//
//  For Posix,  mysys functions are light wrappers around libc, and MY_FILE_MIN
//  is logically 0.
//*/
//#ifdef _WIN32
//#define MY_FILE_MIN  2048
//#else
//#define MY_FILE_MIN  0
//#endif
//见文件头部的“"my_global.h"”
//]]
///*
//  MY_NFILE is the default size of my_file_info array.
//  It is larger on Windows, because it all file handles are stored in my_file_info
//  Default size is 16384 and this should be enough for most cases.If it is not
//  enough, --max-open-files with larger value can be used.
//  For Posix , my_file_info array is only used to store filenames for
//  error reporting and its size is not a limitation for number of open files.
//*/
//#ifdef _WIN32
//#define MY_NFILE (16384 + MY_FILE_MIN)
//#else
//#define MY_NFILE 64
//#endif  //my_global.h;
//#ifndef OS_FILE_LIMIT
//#define OS_FILE_LIMIT	UINT_MAX
//#endif  //my_global.h;
uint   my_file_limit= MY_NFILE;  //mysys/my_static.c //2011-10-08
//[my_sys.h://=>MY_FFNF/ME_BELL  //2011-10-08
/* General bitmaps for my_func's */
#define MY_FFNF		1	/* Fatal if file not found */
#define MY_FNABP	2	/* Fatal if not all bytes read/writen */
#define MY_NABP		4	/* Error if not all bytes read/writen */
#define MY_FAE		8	/* Fatal if any error */
#define MY_WME		16	/* Write message on error */
#define MY_WAIT_IF_FULL 32	/* Wait and try again if disk full error */
#define MY_IGNORE_BADFD 32      /* my_sync: ignore 'bad descriptor' errors */
#define MY_SYNC_DIR     8192    /* my_create/delete/rename: sync directory */
#define MY_UNUSED       64      /* Unused (was support for RAID) */
#define MY_FULL_IO     512      /* For my_read - loop intil I/O is complete */
#define MY_DONT_CHECK_FILESIZE 128 /* Option to init_io_cache() */
#define MY_LINK_WARNING 32	/* my_redel() gives warning if links */
#define MY_COPYTIME	64	/* my_redel() copys time */
#define MY_DELETE_OLD	256	/* my_create_with_symlink() */
#define MY_RESOLVE_LINK 128	/* my_realpath(); Only resolve links */
#define MY_HOLD_ORIGINAL_MODES 128  /* my_copy() holds to file modes */
#define MY_REDEL_MAKE_BACKUP 256
#define MY_SEEK_NOT_DONE 32	/* my_lock may have to do a seek */
#define MY_DONT_WAIT	64	/* my_lock() don't wait if can't lock */
#define MY_ZEROFILL	32	/* my_malloc(), fill array with zero */
#define MY_ALLOW_ZERO_PTR 64	/* my_realloc() ; zero ptr -> malloc */
#define MY_FREE_ON_ERROR 128	/* my_realloc() ; Free old ptr on error */
#define MY_HOLD_ON_ERROR 256	/* my_realloc() ; Return old ptr on error */
#define MY_DONT_OVERWRITE_FILE 1024	/* my_copy: Don't overwrite file */
#define MY_THREADSAFE 2048      /* my_seek(): lock fd mutex */
#define MY_SYNC       4096      /* my_copy(): sync dst file */
//z+/z-
//#define MY_CHECK_ERROR	1	/* Params to my_end; Check open-close */
//#define MY_GIVE_INFO	2	/* Give time info about process*/
//#define MY_DONT_FREE_DBUG 4     /* Do not call DBUG_END() in my_end() */
//z+/z-
#define ME_HIGHBYTE	8	/* Shift for colours */
#define ME_NOCUR	1	/* Don't use curses message */
#define ME_OLDWIN	2	/* Use old window */
#define ME_BELL		4	/* Ring bell then printing message */
#define ME_HOLDTANG	8	/* Don't delete last keys */
#define ME_WAITTOT	16	/* Wait for errtime secs of for a action */
#define ME_WAITTANG	32	/* Wait for a user action  */
#define ME_NOREFRESH	64	/* Dont refresh screen */
#define ME_NOINPUT	128	/* Dont use the input libary */
#define ME_COLOUR1	((1 << ME_HIGHBYTE))	/* Possibly error-colours */
#define ME_COLOUR2	((2 << ME_HIGHBYTE))
#define ME_COLOUR3	((3 << ME_HIGHBYTE))
#define ME_FATALERROR   1024    /* Fatal statement error */
//]
//[mysys/my_error.c:
/* Max length of a error message. Should be kept in sync with MYSQL_ERRMSG_SIZE. */
#define ERRMSGSIZE      (512)
//[[mysys/error.c:
#ifndef SHARED_LIBRARY

const char *globerrs[GLOBERRS]=
{
  "Can't create/write to file '%s' (Errcode: %d)",
  "Error reading file '%s' (Errcode: %d)",
  "Error writing file '%s' (Errcode: %d)",
  "Error on close of '%s' (Errcode: %d)",
  "Out of memory (Needed %u bytes)",
  "Error on delete of '%s' (Errcode: %d)",
  "Error on rename of '%s' to '%s' (Errcode: %d)",
  "",
  "Unexpected eof found when reading file '%s' (Errcode: %d)",
  "Can't lock file (Errcode: %d)",
  "Can't unlock file (Errcode: %d)",
  "Can't read dir of '%s' (Errcode: %d)",
  "Can't get stat of '%s' (Errcode: %d)",
  "Can't change size of file (Errcode: %d)",
  "Can't open stream from handle (Errcode: %d)",
  "Can't get working dirctory (Errcode: %d)",
  "Can't change dir to '%s' (Errcode: %d)",
  "Warning: '%s' had %d links",
  "Warning: %d files and %d streams is left open\n",
  "Disk is full writing '%s' (Errcode: %d). Waiting for someone to free space... (Expect up to %d secs delay for server to continue after freeing disk space)",
  "Can't create directory '%s' (Errcode: %d)",
  "Character set '%s' is not a compiled character set and is not specified in the '%s' file",
  "Out of resources when opening file '%s' (Errcode: %d)",
  "Can't read value for symlink '%s' (Error %d)",
  "Can't create symlink '%s' pointing at '%s' (Error %d)",
  "Error on realpath() on '%s' (Error %d)",
  "Can't sync file '%s' to disk (Errcode: %d)",
  "Collation '%s' is not a compiled collation and is not specified in the '%s' file",
  "File '%s' not found (Errcode: %d)",
  "File '%s' (fileno: %d) was not closed",
  "Can't change ownership of the file '%s' (Errcode: %d)",
  "Can't change permissions of the file '%s' (Errcode: %d)",
};

void init_glob_errs(void)
{
  /* This is now done statically. */
}

#else

void init_glob_errs()
{
  EE(EE_CANTCREATEFILE) = "Can't create/write to file '%s' (Errcode: %d)";
  EE(EE_READ)		= "Error reading file '%s' (Errcode: %d)";
  EE(EE_WRITE)		= "Error writing file '%s' (Errcode: %d)";
  EE(EE_BADCLOSE)	= "Error on close of '%'s (Errcode: %d)";
  EE(EE_OUTOFMEMORY)	= "Out of memory (Needed %u bytes)";
  EE(EE_DELETE)		= "Error on delete of '%s' (Errcode: %d)";
  EE(EE_LINK)		= "Error on rename of '%s' to '%s' (Errcode: %d)";
  EE(EE_EOFERR)		= "Unexpected eof found when reading file '%s' (Errcode: %d)";
  EE(EE_CANTLOCK)	= "Can't lock file (Errcode: %d)";
  EE(EE_CANTUNLOCK)	= "Can't unlock file (Errcode: %d)";
  EE(EE_DIR)		= "Can't read dir of '%s' (Errcode: %d)";
  EE(EE_STAT)		= "Can't get stat of '%s' (Errcode: %d)";
  EE(EE_CANT_CHSIZE)	= "Can't change size of file (Errcode: %d)";
  EE(EE_CANT_OPEN_STREAM)= "Can't open stream from handle (Errcode: %d)";
  EE(EE_GETWD)		= "Can't get working directory (Errcode: %d)";
  EE(EE_SETWD)		= "Can't change dir to '%s' (Errcode: %d)";
  EE(EE_LINK_WARNING)	= "Warning: '%s' had %d links";
  EE(EE_OPEN_WARNING)	= "Warning: %d files and %d streams is left open\n";
  EE(EE_DISK_FULL)	= "Disk is full writing '%s'. Waiting for someone to free space...";
  EE(EE_CANT_MKDIR)	="Can't create directory '%s' (Errcode: %d)";
  EE(EE_UNKNOWN_CHARSET)= "Character set '%s' is not a compiled character set and is not specified in the %s file";
  EE(EE_OUT_OF_FILERESOURCES)="Out of resources when opening file '%s' (Errcode: %d)";
  EE(EE_CANT_READLINK)=	"Can't read value for symlink '%s' (Error %d)";
  EE(EE_CANT_SYMLINK)=	"Can't create symlink '%s' pointing at '%s' (Error %d)";
  EE(EE_REALPATH)=	"Error on realpath() on '%s' (Error %d)";
  EE(EE_SYNC)=		"Can't sync file '%s' to disk (Errcode: %d)";
  EE(EE_UNKNOWN_COLLATION)= "Collation '%s' is not a compiled collation and is not specified in the %s file";
  EE(EE_FILENOTFOUND)	= "File '%s' not found (Errcode: %d)";
  EE(EE_FILE_NOT_CLOSED) = "File '%s' (fileno: %d) was not closed";
  EE(EE_CHANGE_OWNERSHIP)   = "Can't change ownership of the file '%s' (Errcode: %d)";
  EE(EE_CHANGE_PERMISSIONS) = "Can't change permissions of the file '%s' (Errcode: %d)";
}
#endif
const char **get_global_errmsgs()
{
  return globerrs;
}
//]]
/*
  Message texts are registered into a linked list of 'my_err_head' structs.
  Each struct contains (1.) an array of pointers to C character strings with
  '\0' termination, (2.) the error number for the first message in the array
  (array index 0) and (3.) the error number for the last message in the array
  (array index (last - first)).
  The array may contain gaps with NULL pointers and pointers to empty strings.
  Both kinds of gaps will be translated to "Unknown error %d.", if my_error()
  is called with a respective error number.
  The list of header structs is sorted in increasing order of error numbers.
  Negative error numbers are allowed. Overlap of error numbers is not allowed.
  Not registered error numbers will be translated to "Unknown error %d.".
*/
static struct my_err_head
{
  struct my_err_head    *meh_next;         /* chain link */
  const char**          (*get_errmsgs) (); /* returns error message format */
  int                   meh_first;       /* error number matching array slot 0 */
  int                   meh_last;          /* error number matching last slot */
} my_errmsgs_globerrs = {NULL, get_global_errmsgs, EE_ERROR_FIRST, EE_ERROR_LAST};
static struct my_err_head *my_errmsgs_list= &my_errmsgs_globerrs;
//[[mysys/my_mess.c:
//#include "mysys_priv.h"
//[[[mysys/my_static.c:
/* from my_init */
char *	home_dir=0;
const char      *my_progname=0;
char		curr_dir[FN_REFLEN]= {0},
		home_dir_buff[FN_REFLEN]= {0};
//ulong		my_stream_opened=0,my_file_opened=0, my_tmp_file_created=0;  //重复定义 //zlq  //2011-10-08-12-39;
ulong           my_file_total_opened= 0;
int		my_umask=0664, my_umask_dir=0777;
//#ifndef THREAD
//int		my_errno=0;  //重复定义 //zlq  //2011-10-08-12-39;
//#endif
//[[[[include/my_sys.h:  //2011-10-08-13-15;
struct st_my_file_info
{
  char  *name;
#ifdef _WIN32
  HANDLE fhandle;   /* win32 file handle */
  int    oflag;     /* open flags, e.g O_APPEND */
#endif
  enum   file_type	type;
#if defined(THREAD) && !defined(HAVE_PREAD) && !defined(_WIN32)
  mysql_mutex_t mutex;
#endif
};
//]]]]
struct st_my_file_info my_file_info_default[MY_NFILE];
//uint   my_file_limit= MY_NFILE;  //重复定义 //zlq  //2011-10-08-13-16;
struct st_my_file_info *my_file_info= my_file_info_default;
//]]]
#include <stdlib.h> //=>exit
//
void my_message_stderr(uint error __attribute__((unused)),
                       const char *str, myf MyFlags)
{
  DBUG_ENTER("my_message_stderr");
  DBUG_PRINT("enter",("message: %s",str));
  (void) fflush(stdout);
  if (MyFlags & ME_BELL)
    (void) fputc('\007', stderr);
  if (my_progname)
  {
    (void)fputs(my_progname,stderr); (void)fputs(": ",stderr);
  }
  (void)fputs(str,stderr);
  (void)fputc('\n',stderr);
  (void)fflush(stderr);
  DBUG_VOID_RETURN;
}
//]]
//[[mysys/my_static.c:
void (*my_abort_hook)(int) = (void(*)(int)) exit;
void (*error_handler_hook)(uint error, const char *str, myf MyFlags)=
  my_message_stderr;
void (*fatal_error_handler_hook)(uint error, const char *str, myf MyFlags)=
  my_message_stderr;
//]]
/*
   Error message to user

   SYNOPSIS
     my_error()
       nr	Errno
       MyFlags	Flags
       ...	variable list

*/
void my_error(int nr, myf MyFlags, ...)
{
  const char *format;
  struct my_err_head *meh_p;
  va_list args;
  char ebuff[ERRMSGSIZE];
  DBUG_ENTER("my_error");
  DBUG_PRINT("my", ("nr: %d  MyFlags: %d  errno: %d", nr, MyFlags, errno));

  /* Search for the error messages array, which could contain the message. */
  for (meh_p= my_errmsgs_list; meh_p; meh_p= meh_p->meh_next)
    if (nr <= meh_p->meh_last)
      break;

  /* get the error message string. Default, if NULL or empty string (""). */
  if (! (format= (meh_p && (nr >= meh_p->meh_first)) ?
                  meh_p->get_errmsgs()[nr - meh_p->meh_first] : NULL) || ! *format)
    (void) my_snprintf (ebuff, sizeof(ebuff), "Unknown error %d", nr);
  else
  {
    va_start(args,MyFlags);
//    (void) my_vsnprintf_ex(&my_charset_utf8_general_ci, ebuff,
//                           sizeof(ebuff), format, args);  //z- //zlq
    va_end(args);
  }
  (*error_handler_hook)(nr, ebuff, MyFlags);
  DBUG_VOID_RETURN;
}
/*
  WARNING!
  my_error family functions have to be used according following rules:
  - if message have not parameters use my_message(ER_CODE, ER(ER_CODE), MYF(N))
  - if message registered use my_error(ER_CODE, MYF(N), ...).
  - With some special text of errror message use:
  my_printf_error(ER_CODE, format, MYF(N), ...)
*/
//]
//[include/my_pthread.h:
//#include "my_pthread.h"
//]
//[include/mysql/psi/mysql_thread.h:
/**
  An instrumented mutex structure.
  @sa mysql_mutex_t
*/
struct st_mysql_mutex
{
  /** The real mutex. */
  pthread_mutex_t m_mutex;
  /**
   The instrumentation hook.
    Note that this hook is not conditionally defined,
    for binary compatibility of the @c mysql_mutex_t interface.
  */
  struct PSI_mutex *m_psi;
};
/**
  Type of an instrumented mutex.
  @c mysql_mutex_t is a drop-in replacement for @c pthread_mutex_t.
  @sa mysql_mutex_assert_owner
  @sa mysql_mutex_assert_not_owner
  @sa mysql_mutex_init
  @sa mysql_mutex_lock
  @sa mysql_mutex_unlock
  @sa mysql_mutex_destroy
*/
typedef struct st_mysql_mutex mysql_mutex_t;
//]
//[mysys/my_thr_init.c:
//#ifdef THREAD  //z-//zlqlxm
//...
mysql_mutex_t THR_LOCK_malloc, THR_LOCK_open,
              THR_LOCK_lock, THR_LOCK_isam, THR_LOCK_myisam, THR_LOCK_heap,
              THR_LOCK_net, THR_LOCK_charset, THR_LOCK_threads, THR_LOCK_time,
              THR_LOCK_myisam_mmap;
//...
//#endif
//]
//[include/mysql/psi/mysql_thread.h:
static inline int inline_mysql_mutex_lock(
  mysql_mutex_t *that
#if defined(SAFE_MUTEX) || defined (HAVE_PSI_INTERFACE)
  , const char *src_file, uint src_line
#endif
  )
{
  int result;
#ifdef HAVE_PSI_INTERFACE
  struct PSI_mutex_locker *locker= NULL;
  PSI_mutex_locker_state state;
  if (likely(PSI_server && that->m_psi))
  {
    locker= PSI_server->get_thread_mutex_locker(&state, that->m_psi, PSI_MUTEX_LOCK);
    if (likely(locker != NULL))
      PSI_server->start_mutex_wait(locker, src_file, src_line);
  }
#endif
#ifdef SAFE_MUTEX
  result= safe_mutex_lock(&that->m_mutex, FALSE, src_file, src_line);
#else
  result= pthread_mutex_lock(&that->m_mutex);
#endif
#ifdef HAVE_PSI_INTERFACE
  if (likely(locker != NULL))
    PSI_server->end_mutex_wait(locker, result);
#endif
  return result;
}
/**
  @def mysql_mutex_lock(M)
  Instrumented mutex_lock.
  @c mysql_mutex_lock is a drop-in replacement for @c pthread_mutex_lock.
  @param M The mutex to lock
*/
#if defined(SAFE_MUTEX) || defined (HAVE_PSI_INTERFACE)
  #define mysql_mutex_lock(M) \
    inline_mysql_mutex_lock(M, __FILE__, __LINE__)
#else
  #define mysql_mutex_lock(M) \
    inline_mysql_mutex_lock(M)
#endif

static inline int inline_mysql_mutex_unlock(
  mysql_mutex_t *that
#ifdef SAFE_MUTEX
  , const char *src_file, uint src_line
#endif
  )
{
  int result;
#ifdef HAVE_PSI_INTERFACE
  if (likely(PSI_server && that->m_psi))
    PSI_server->unlock_mutex(that->m_psi);
#endif
#ifdef SAFE_MUTEX
  result= safe_mutex_unlock(&that->m_mutex, src_file, src_line);
#else
  result= pthread_mutex_unlock(&that->m_mutex);
#endif
  return result;
}
/**
  @def mysql_mutex_unlock(M)
  Instrumented mutex_unlock.
  @c mysql_mutex_unlock is a drop-in replacement for @c pthread_mutex_unlock.
*/
#ifdef SAFE_MUTEX
  #define mysql_mutex_unlock(M) \
    inline_mysql_mutex_unlock(M, __FILE__, __LINE__)
#else
  #define mysql_mutex_unlock(M) \
    inline_mysql_mutex_unlock(M)
#endif
//]/include/mysql/psi/mysql_thread.h;
//./include/my_no_pthread.h:#define mysql_mutex_lock(A) do {} while (0)  //zlqlxm
//[mysys/my_malloc.c:
//[[dbug/dbug.c:
/*
 *  FUNCTION
 *
 *      _db_keyword_    test keyword for member of keyword list
 *
 *  DESCRIPTION
 *
 *      Test a keyword to determine if it is in the currently active
 *      keyword list.  If strict=0, a keyword is accepted
 *      if the list is null, otherwise it must match one of the list
 *      members.  When debugging is not on, no keywords are accepted.
 *      After the maximum trace level is exceeded, no keywords are
 *      accepted (this behavior subject to change).  Additionally,
 *      the current function and process must be accepted based on
 *      their respective lists.
 *
 *      Returns TRUE if keyword accepted, FALSE otherwise.
 *
 */
BOOLEAN _db_keyword_(CODE_STATE *cs, const char *keyword, int strict)
{
/*  get_code_state_if_not_set_or_return FALSE;
  strict=strict ? INCLUDE : INCLUDE|MATCHED;

  return DEBUGGING && DoTrace(cs) & DO_TRACE &&
         InList(cs->stack->keywords, keyword) & strict;
*/  //z- //zlq
return FALSE; //z+ //zlq  //<-DBUG_EXECUTE_IF<-my_malloc //----2011-10-08-16-02--16-06！！！；；;
/*
[root@server01 InnoSQL-5.5.8]# grep "_db_keyword_" ./  -r
./dbug/dbug.c:  if (_db_keyword_(cs, cs->u_keyword, 0))
./dbug/dbug.c:  if (_db_keyword_(cs, keyword, 0))
./dbug/dbug.c: *      _db_keyword_    test keyword for member of keyword list
./dbug/dbug.c:BOOLEAN _db_keyword_(CODE_STATE *cs, const char *keyword, int strict)
./dbug/dbug_long.h:    extern int _db_keyword_ ();		/ * Accept/reject keyword * /
./dbug/dbug_long.h:	      {if (_db_on_) {if (_db_keyword_ (keyword)) { a1 }}}
./include/my_dbug.h:extern  my_bool _db_keyword_(struct _db_code_state_ *, const char *, int);
./include/my_dbug.h:        do {if (_db_keyword_(0, (keyword), 0)) { a1 }} while(0)
./include/my_dbug.h:        do {if (_db_keyword_(0, (keyword), 1)) { a1 }} while(0)
./include/my_dbug.h:        (_db_keyword_(0,(keyword), 0) ? (a1) : (a2))
./include/my_dbug.h:        (_db_keyword_(0,(keyword), 1) ? (a1) : (a2))
[root@server01 InnoSQL-5.5.8]#
*/
}
//]]
//[[my_dbug.h:
#include "my_dbug.h"//=>DBUG_EXECUTE_IF/DBUG_SET
//]]
//[[dbug/dbug.c:
/*
 *  FUNCTION
 *
 *      _db_set_       set current debugger settings
 *
 *  SYNOPSIS
 *
 *      VOID _db_set_(control)
 *      char *control;
 *
 *  DESCRIPTION
 *
 *      Given pointer to a debug control string in "control",
 *      parses the control string, and sets up a current debug
 *      settings. Pushes a new debug settings if the current is
 *      set to the initial debugger settings.
 *
 */
void _db_set_(const char *control)
{
  CODE_STATE *cs;
  uint old_fflags;
  get_code_state_or_return;
/*
  old_fflags=fflags(cs);
  if (cs->stack == &init_settings)
    PushState(cs);
  if (DbugParse(cs, control))
    FixTraceFlags(old_fflags, cs);
*/ //z- //zlq
}
//]]
/**
  Allocate a sized block of memory.

  @param size   The size of the memory block in bytes.
  @param flags  Failure action modifiers (bitmasks).

  @return A pointer to the allocated memory block, or NULL on failure.
*/
void *my_malloc(size_t size, myf my_flags)
{
  void* point;
  DBUG_ENTER("my_malloc");
  DBUG_PRINT("my",("size: %lu  my_flags: %d", (ulong) size, my_flags));

  if(!size)size=1;/* Safety */
  point= malloc(size);
  //
  DBUG_EXECUTE_IF("simulate_out_of_memory",
                  {
                    free(point);
                    point=NULL;
		  });
//#define DBUG_EXECUTE_IF(keyword,a1) \
//        do {if (_db_keyword_(0, (keyword), 1)) { a1 }} while(0)
//_db_keyword_:"Test a keyword to determine if it is in the currently active keyword list." //----2011-10-08-16-02;
  //
  if(NULL==point){
    my_errno=errno;
    if (my_flags & MY_FAE)
      error_handler_hook=fatal_error_handler_hook;
    if (my_flags & (MY_FAE+MY_WME))
      my_error(EE_OUTOFMEMORY, MYF(ME_BELL+ME_WAITTANG+ME_NOREFRESH),size);
    DBUG_EXECUTE_IF("simulate_out_of_memory",
                    DBUG_SET("-d,simulate_out_of_memory");
		    );
//(my_dbug.h:)“#define DBUG_SET(a1) _db_set_ (a1)”
    if (my_flags & MY_FAE)
      exit(1);
  }else if (my_flags & MY_ZEROFILL){
    bzero(point, size);
  }
  DBUG_PRINT("exit",("ptr: %p", point));

  DBUG_RETURN(point);
}

/**
  Free memory allocated with my_malloc.

  @remark Relies on free being able to handle a NULL argument.

  @param ptr Pointer to the memory allocated by my_malloc.
*/
void my_free(void *ptr)
{
  DBUG_ENTER("my_free");
  DBUG_PRINT("my",("ptr: %p", ptr));
  free(ptr);
  DBUG_VOID_RETURN;
}

char *my_strdup(const char *from, myf my_flags)
{
  char *ptr;
  size_t length= strlen(from)+1;
  if ((ptr= (char*) my_malloc(length, my_flags)))
    memcpy(ptr, from, length);
  return ptr;
}
//]
//[mysys/my_open.c:
//[[mysys/my_div.c:
//#include "mysys_priv.h"
//
/*
  Get filename of file

  SYNOPSIS
    my_filename()
      fd	File descriptor
*/
char * my_filename(File fd)
{
  DBUG_ENTER("my_filename");
  if ((uint) fd >= (uint) my_file_limit)
    DBUG_RETURN((char*) "UNKNOWN");
  if (fd >= 0 && my_file_info[fd].type != UNOPEN)
  {
    DBUG_RETURN(my_file_info[fd].name);
  }
  else
    DBUG_RETURN((char*) "UNOPENED");	/* Debug message */
}
//]]
/*
  Close a file

  SYNOPSIS
    my_close()
      fd	File sescriptor
      myf	Special Flags

*/
int my_close(File fd, myf MyFlags)
{
  int err;
  DBUG_ENTER("my_close");
  DBUG_PRINT("my",("fd: %d  MyFlags: %d",fd, MyFlags));

  mysql_mutex_lock(&THR_LOCK_open);
#ifndef _WIN32
  do
  {
    err= close(fd);
  } while (err == -1 && errno == EINTR);
#else
  err= my_win_close(fd);
#endif
  if (err)
  {
    DBUG_PRINT("error",("Got error %d on close",err));
    my_errno=errno;
    if (MyFlags & (MY_FAE | MY_WME))
      my_error(EE_BADCLOSE, MYF(ME_BELL+ME_WAITTANG),my_filename(fd),errno);
  }
  if ((uint) fd < my_file_limit && my_file_info[fd].type != UNOPEN)
  {
    my_free(my_file_info[fd].name);
#if defined(THREAD) && !defined(HAVE_PREAD) && !defined(_WIN32)
    mysql_mutex_destroy(&my_file_info[fd].mutex);
#endif
    my_file_info[fd].type = UNOPEN;
  }
  my_file_opened--;
  mysql_mutex_unlock(&THR_LOCK_open);
  DBUG_RETURN(err);
} /* my_close */
//]
/*
  Register file in my_file_info[]

  SYNOPSIS
    my_register_filename()
    fd			   File number opened, -1 if error on open
    FileName		   File name
    type_file_type	   How file was created
    error_message_number   Error message number if caller got error (fd == -1)
    MyFlags		   Flags for my_close()

  RETURN
    -1   error
     #   Filenumber

*/
File my_register_filename(File fd, const char *FileName, enum file_type
			  type_of_file, uint error_message_number, myf MyFlags) //mysys/my_open.c  //zlq
{
  DBUG_ENTER("my_register_filename");
  if((int)fd>=MY_FILE_MIN){
    if ((uint) fd >= my_file_limit)
    {
#if defined(THREAD) && !defined(HAVE_PREAD)
      my_errno= EMFILE;
#else
      thread_safe_increment(my_file_opened,&THR_LOCK_open);
      DBUG_RETURN(fd);				/* safeguard */
#endif
    }
    else
    {
      mysql_mutex_lock(&THR_LOCK_open);
      if ((my_file_info[fd].name = (char*) my_strdup(FileName,MyFlags)))  //zlqlxm //这一步导致"create_temp_file"函数执行失败！！；----2011-10-08-15-26--15-30！==2011-10-08-15-36；;
      {
        my_file_opened++;
        my_file_total_opened++;
        my_file_info[fd].type = type_of_file;
#if defined(THREAD) && !defined(HAVE_PREAD) && !defined(_WIN32)
        mysql_mutex_init(key_my_file_info_mutex, &my_file_info[fd].mutex,
                         MY_MUTEX_INIT_FAST);
#endif
        mysql_mutex_unlock(&THR_LOCK_open);
        DBUG_PRINT("exit",("fd: %d",fd));
        DBUG_RETURN(fd);
      }
      mysql_mutex_unlock(&THR_LOCK_open);
      my_errno= ENOMEM;                                                   //zlqlxm
    }
    (void) my_close(fd, MyFlags);
  }else{
    my_errno=errno;
  }

  DBUG_PRINT("error",("Got error %d on open", my_errno));
  if(MyFlags & (MY_FFNF | MY_FAE | MY_WME)){
    if (my_errno == EMFILE)
      error_message_number= EE_OUT_OF_FILERESOURCES;
    DBUG_PRINT("error",("print err: %d",error_message_number));
    my_error(error_message_number, MYF(ME_BELL+ME_WAITTANG),
             FileName, my_errno);
  }

  DBUG_RETURN(-1);
}
//]//InnoSQL-5.5.8/mysys/my_open.c  //zlq
//[
//[[mysys/my_sync.c:
//#include "mysys_priv.h"
//#include "mysys_err.h"
//#include <errno.h>
//
//#define HAVE_FSYNC  //z+ //zlq
#define HAVE_FDATASYNC
#define HAVE_DECL_FDATASYNC 1  //z+ //zlq  //----2011-10-08-14-10;
//#define NEED_EXPLICIT_SYNC_DIR  //z+ //zlq  //----2011-10-08-14-17;  //z-/zlq
/*
  Sync data in file to disk

  SYNOPSIS
    my_sync()
    fd			File descritor to sync
    my_flags		Flags (now only MY_WME is supported)

  NOTE
    If file system supports its, only file data is synced, not inode data.

    MY_IGNORE_BADFD is useful when fd is "volatile" - not protected by a
    mutex. In this case by the time of fsync(), fd may be already closed by
    another thread, or even reassigned to a different file. With this flag -
    MY_IGNORE_BADFD - such a situation will not be considered an error.
    (which is correct behaviour, if we know that the other thread synced the
    file before closing)

  RETURN
    0 ok
    -1 error
*/
int my_sync(File fd, myf my_flags)
{
  int res;
  DBUG_ENTER("my_sync");
  DBUG_PRINT("my",("Fd: %d  my_flags: %d", fd, my_flags));

  do
  {
#if defined(F_FULLFSYNC)
    /*
      In Mac OS X >= 10.3 this call is safer than fsync() (it forces the
      disk's cache and guarantees ordered writes).
    */
    if (!(res= fcntl(fd, F_FULLFSYNC, 0)))
      break; /* ok */
    /* Some file systems don't support F_FULLFSYNC and fail above: */
    DBUG_PRINT("info",("fcntl(F_FULLFSYNC) failed, falling back"));
#endif
#if defined(HAVE_FDATASYNC) && HAVE_DECL_FDATASYNC
    res= fdatasync(fd);
#elif defined(HAVE_FSYNC)
    res= fsync(fd);
#elif defined(_WIN32)
    res= my_win_fsync(fd);
#else
#error Cannot find a way to sync a file, durability in danger
//zlqlxm
    res= 0;					/* No sync (strange OS) */
#endif
  } while (res == -1 && errno == EINTR);

  if (res)
  {
    int er= errno;
    if (!(my_errno= er))
      my_errno= -1;                             /* Unknown error */
    if ((my_flags & MY_IGNORE_BADFD) &&
        (er == EBADF || er == EINVAL || er == EROFS))
    {
      DBUG_PRINT("info", ("ignoring errno %d", er));
      res= 0;
    }
    else if (my_flags & MY_WME)
      my_error(EE_SYNC, MYF(ME_BELL+ME_WAITTANG), my_filename(fd), my_errno);
  }
  DBUG_RETURN(res);
} /* my_sync */

static const char cur_dir_name[]= {FN_CURLIB, 0};

//[mysys/my_open.c: //----2011-10-08-14-39;
/*
  Open a file

  SYNOPSIS
    my_open()
      FileName	Fully qualified file name
      Flags	Read | write
      MyFlags	Special flags

  RETURN VALUE
    File descriptor
*/
File my_open(const char *FileName, int Flags, myf MyFlags)
				/* Path-name of file */
				/* Read | write .. */
				/* Special flags */
{
  File fd;
  DBUG_ENTER("my_open");
  DBUG_PRINT("my",("Name: '%s'  Flags: %d  MyFlags: %d",
		   FileName, Flags, MyFlags));
#if defined(_WIN32)
  fd= my_win_open(FileName, Flags);
#elif !defined(NO_OPEN_3)
  fd = open(FileName, Flags, my_umask);	/* Normal unix */
#else
  fd = open((char *) FileName, Flags);
#endif

  DBUG_RETURN(my_register_filename(fd, FileName, FILE_BY_OPEN,
				   EE_FILENOTFOUND, MyFlags));
} /* my_open */
//]
/*
  Force directory information to disk.

  SYNOPSIS
    my_sync_dir()
    dir_name             the name of the directory
    my_flags             flags (MY_WME etc)

  RETURN
    0 if ok, !=0 if error
*/
#ifdef NEED_EXPLICIT_SYNC_DIR
int my_sync_dir(const char *dir_name, myf my_flags)
{
  File dir_fd;
  int res= 0;
  const char *correct_dir_name;
  DBUG_ENTER("my_sync_dir");
  DBUG_PRINT("my",("Dir: '%s'  my_flags: %d", dir_name, my_flags));
  /* Sometimes the path does not contain an explicit directory */
  correct_dir_name= (dir_name[0] == 0) ? cur_dir_name : dir_name;
  /*
    Syncing a dir may give EINVAL on tmpfs on Linux, which is ok.
    EIO on the other hand is very important. Hence MY_IGNORE_BADFD.
  */
  if ((dir_fd= my_open(correct_dir_name, O_RDONLY, MYF(my_flags))) >= 0)
  {
    if (my_sync(dir_fd, MYF(my_flags | MY_IGNORE_BADFD)))
      res= 2;
    if (my_close(dir_fd, MYF(my_flags)))
      res= 3;
  }
  else
    res= 1;
  DBUG_RETURN(res);
}
#else /* NEED_EXPLICIT_SYNC_DIR */
int my_sync_dir(const char *dir_name __attribute__((unused)),
                myf my_flags __attribute__((unused)))
{
  return 0;
}
#endif /* NEED_EXPLICIT_SYNC_DIR */

//[[[my_global.h://=>FN_LIBCHAR
///* General constants */
//#define FN_LEN		256	/* Max file name len */
//#define FN_HEADLEN	253	/* Max length of filepart of file name */
//#define FN_EXTLEN	20	/* Max length of extension (part of FN_LEN) */
//#define FN_REFLEN	512	/* Max length of full path-name */
//#define FN_EXTCHAR	'.'
//#define FN_HOMELIB	'~'	/* ~/ is used as abbrev for home dir */
//#define FN_CURLIB	'.'	/* ./ is used as abbrev for current dir */
//#define FN_PARENTDIR	".."	/* Parent directory; Must be a string */
//
//#ifdef _WIN32
//#define FN_LIBCHAR	'\\'
//#define FN_LIBCHAR2	'/'
//#define FN_ROOTDIR	"\\"
//#define FN_DEVCHAR	':'
//#define FN_NETWORK_DRIVES	/* Uses \\ to indicate network drives */
//#define FN_NO_CASE_SENCE	/* Files are not case-sensitive */
//#else
//#define FN_LIBCHAR	'/'
//#define FN_LIBCHAR2	'/'
//#define FN_ROOTDIR	"/"
//#endif
//]]]
//[[[mysys/mf_dirname.c: /同时"convert_dirname"函数用于后面的"create_temp_file";//----2011-10-08-14-21;
//#include "mysys_priv.h"
//#include <m_string.h>
//
size_t dirname_length(const char *name)
{
  register char *pos, *gpos;
#ifdef BASKSLASH_MBTAIL
  CHARSET_INFO *fs= fs_character_set();
#endif
#ifdef FN_DEVCHAR
  if ((pos=(char*)strrchr(name,FN_DEVCHAR)) == 0)
#endif
    pos=(char*) name-1;

  gpos= pos++;
  for ( ; *pos ; pos++)				/* Find last FN_LIBCHAR */
  {
#ifdef BASKSLASH_MBTAIL
    uint l;
    if (use_mb(fs) && (l= my_ismbchar(fs, pos, pos + 3)))
    {
      pos+= l - 1;
      continue;
    }
#endif
    if (*pos == FN_LIBCHAR || *pos == '/')
      gpos=pos;
  }
  return (size_t) (gpos+1-(char*) name);
}
//-
//[[[[strings/strmake.c:
//#include <my_global.h>
//#include "m_string.h"
//
char *strmake(register char *dst, register const char *src, size_t length)
{
#ifdef EXTRA_DEBUG
  /*
    'length' is the maximum length of the string; the buffer needs
    to be one character larger to accomodate the terminating '\0'.
    This is easy to get wrong, so we make sure we write to the
    entire length of the buffer to identify incorrect buffer-sizes.
    We only initialise the "unused" part of the buffer here, a) for
    efficiency, and b) because dst==src is allowed, so initialising
    the entire buffer would overwrite the source-string. Also, we
    write a character rather than '\0' as this makes spotting these
    problems in the results easier.
  */
  uint n= 0;
  while (n < length && src[n++]);
  memset(dst + n, (int) 'Z', length - n + 1);
#endif

  while (length--)
    if (! (*dst++ = *src++))
      return dst-1;
  *dst=0;
  return dst;
}
//]]]]
/*
  Convert directory name to use under this system

  SYNPOSIS
    convert_dirname()
    to				Store result here. Must be at least of size
    				min(FN_REFLEN, strlen(from) + 1) to make room
    				for adding FN_LIBCHAR at the end.
    from			Original filename. May be == to
    from_end			Pointer at end of filename (normally end \0)

  IMPLEMENTATION
    If Windows converts '/' to '\'
    Adds a FN_LIBCHAR to end if the result string if there isn't one
    and the last isn't dev_char.
    Copies data from 'from' until ASCII(0) for until from == from_end
    If you want to use the whole 'from' string, just send NullS as the
    last argument.

    If the result string is larger than FN_REFLEN -1, then it's cut.

  RETURN
   Returns pointer to end \0 in to
*/
#ifndef FN_DEVCHAR
#define FN_DEVCHAR '\0'				/* For easier code */
#endif
char *convert_dirname(char *to, const char *from, const char *from_end)
{
  char *to_org=to;
#ifdef BACKSLASH_MBTAIL
  CHARSET_INFO *fs= fs_character_set();
#endif
  DBUG_ENTER("convert_dirname");

  /* We use -2 here, becasue we need place for the last FN_LIBCHAR */
  if (!from_end || (from_end - from) > FN_REFLEN-2)
    from_end=from+FN_REFLEN -2;

#if FN_LIBCHAR != '/'
  {
    for (; from != from_end && *from ; from++)
    {
      if (*from == '/')
	*to++= FN_LIBCHAR;
      else
      {
#ifdef BACKSLASH_MBTAIL
        uint l;
        if (use_mb(fs) && (l= my_ismbchar(fs, from, from + 3)))
        {
          memmove(to, from, l);
          to+= l;
          from+= l - 1;
          to_org= to; /* Don't look inside mbchar */
        }
        else
#endif //“#ifdef BACKSLASH_MBTAIL”
        {
          *to++= *from;
        }
      }
    }
    *to=0;
  }
#else //“#if FN_LIBCHAR != '/'”
  /* This is ok even if to == from, becasue we need to cut the string */
  to= strmake(to, from, (size_t) (from_end-from));
#endif

  /* Add FN_LIBCHAR to the end of directory path */
  if (to != to_org && (to[-1] != FN_LIBCHAR && to[-1] != FN_DEVCHAR))
  {
    *to++=FN_LIBCHAR;
    *to=0;
  }
  DBUG_RETURN(to);                              /* Pointer to end of dir */
} /* convert_dirname */
/*
  Gives directory part of filename. Directory ends with '/'

  SYNOPSIS
    dirname_part()
    to		Store directory name here
    name	Original name
    to_length	Store length of 'to' here

  RETURN
   #  Length of directory part in 'name'
*/
size_t dirname_part(char *to, const char *name, size_t *to_res_length)
{
  size_t length;
  DBUG_ENTER("dirname_part");
  DBUG_PRINT("enter",("'%s'",name));

  length=dirname_length(name);
  *to_res_length= (size_t) (convert_dirname(to, name, name+length) - to);
  DBUG_RETURN(length);
} /* dirname */
//]]]
/*
  Force directory information to disk.

  SYNOPSIS
    my_sync_dir_by_file()
    file_name            the name of a file in the directory
    my_flags             flags (MY_WME etc)

  RETURN
    0 if ok, !=0 if error
*/
#ifdef NEED_EXPLICIT_SYNC_DIR
int my_sync_dir_by_file(const char *file_name, myf my_flags)
{
  char dir_name[FN_REFLEN];
  size_t dir_name_length;
  dirname_part(dir_name, file_name, &dir_name_length);
  return my_sync_dir(dir_name, my_flags);
}
#else /* NEED_EXPLICIT_SYNC_DIR */
int my_sync_dir_by_file(const char *file_name __attribute__((unused)),
                        myf my_flags __attribute__((unused)))
{
  return 0;
}
#endif /* NEED_EXPLICIT_SYNC_DIR */
//]]
int my_delete(const char *name, myf MyFlags)
{
  int err;
  DBUG_ENTER("my_delete");
  DBUG_PRINT("my",("name %s MyFlags %d", name, MyFlags));

  if ((err = unlink(name)) == -1)
  {
    my_errno=errno;
    if (MyFlags & (MY_FAE+MY_WME))
      my_error(EE_DELETE,MYF(ME_BELL+ME_WAITTANG+(MyFlags & ME_NOINPUT)),
	       name,errno);
  }
  else if ((MyFlags & MY_SYNC_DIR) &&
           my_sync_dir_by_file(name, MyFlags))
    err= -1;

  DBUG_RETURN(err);
}
//]//InnoSQL-5.5.8/mysys/my_delete.c  //zlq
//xxx //zlq
/*（原始的应该是包含：
/
#include "mysys_priv.h"
#include <m_string.h>
#include "my_static.h"
#include "mysys_err.h"
#include <errno.h>
#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
*
//mf_tempdir.c: init_tmpdir/my_tmpdir/free_tmpdir
  ）
*/ //----2011-09-22-17-07; //zlqlxm
//#include <fcntl.h> //=>O_RDWR等...//----2011-10-08-14-55;//与上面的“fd = open(FileName, Flags, my_umask);”重复和冲突！！！；----2011-10-08-14-56；；;
//#include <sys/types.h>
//#include <sys/stat.h>
#ifndef O_RDWR //+----2011-10-08-16-23加此句/因为本文件被包含到“os0file.c”会导致重复定义！！；;--2011-10-08-16-25:R;/被“ha_innodb__.cc”使用！！！--2011-10-08-16-32；；;
#include <asm-generic/fcntl.h>
#endif
File create_temp_file(char* to,
                      const char* dir, const char* prefix,
		      int mode __attribute__((unused)),
		      myf MyFlags __attribute__((unused)))
{
  File file= -1;
#ifdef __WIN__
  TCHAR path_buf[MAX_PATH-14];
#endif

  DBUG_ENTER("create_temp_file");//=>_db_enter_

  DBUG_PRINT("enter", ("dir: %s, prefix: %s", dir, prefix));//=>_db_pargs_ ... _db_doprnt_
#if defined (__WIN__)
{
  /*Use GetTempPath to [determine path] for temporary files.
    This is because the documentation for GetTempFileName
    has the following to say about this parameter:
    "If this parameter is NULL, the function fails."
  */
  if(!dir){
    if(GetTempPath(sizeof(path_buf), path_buf) > 0){
      dir = path_buf;
    }
  }
  /*Use GetTempFileName to [generate a unique filename],
     create the file and release it's handle - uses up to the first three letters from prefix
  */
  if(GetTempFileName(dir, prefix, 0, to) == 0){
    DBUG_RETURN(-1);
  }
  DBUG_PRINT("info", ("name: %s", to));

  /*Open the file without the "open only if file doesn't already exist"
    since the file has already been created by GetTempFileName
  */
  if((file=my_open(to,  (mode & ~O_EXCL), MyFlags)) < 0){//=>my_sys.h(:include拷贝成include2) //mysys/my_open.c
    /*Open failed, remove the file created by GetTempFileName*/
    int tmp=my_errno;
    (void)my_delete(to, MYF(0));
    my_errno=tmp;
  }
}
#elif defined(HAVE_MKSTEMP) //"#if defined (__WIN__)"
{
  char prefix_buff[30];
  uint pfx_len;
  File org_file;

  if(!dir && !(dir=getenv("TMPDIR"))){
    dir=P_tmpdir;
  }
  pfx_len=(uint)(strmov(strnmov(prefix_buff, prefix?prefix:"tmp.", sizeof(prefix_buff)-7),
                        "XXXXXX"
                       ) - prefix_buff
		 ); //strnmov=>m_string.h //InnoSQL-5.5.8/strings/strnmov.c
  if(strlen(dir)+pfx_len > FN_REFLEN-2){
    errno=my_errno= ENAMETOOLONG;
    DBUG_RETURN(file);
  }
  strmov(convert_dirname(to,dir,NullS), prefix_buff);

  org_file=mkstemp(to); //zlq //stdlib.h
  //
  if(mode & O_TEMPORARY){
    (void)my_delete(to, MYF(MY_WME|ME_NOINPUT));
  }
  file=my_register_filename(org_file, to, FILE_BY_MKSTEMP, EE_CANTCREATEFILE, MyFlags);
  //
  if(org_file>=0 && file<0){/* If we didn't manage to register the name, remove the temp file */
    int tmp=my_errno;
    close(org_file);
    (void)my_delete(to, MYF(MY_WME|ME_NOINPUT));
    my_errno=tmp;
  }
}
#elif defined(HAVE_TEMPNAM)
{
  extern char** environ;
  char* res;
  char** old_env;
  char* temp_env[1];

  if(dir && !dir[0]){/* Change empty string to current dir */
      to[0]= FN_CURLIB;
      to[1]= 0;
      dir=to;
  }

  old_env= (char**) environ;
  if (dir){				/* Don't use TMPDIR if dir is given */
      environ=(const char**) temp_env;
      temp_env[0]=0;
  }

  if((res=tempnam((char*) dir, (char*) prefix))){ //zlq

HAVE_PSI_INTERFACE      strmake(to,res,FN_REFLEN-1);
      (*free)(res);
      file=my_create(to,0,
		     (int) (O_RDWR | O_BINARY | O_TRUNC | O_EXCL | O_NOFOLLOW |
			    O_TEMPORARY | O_SHORT_LIVED),
		     MYF(MY_WME));

  }else{
      DBUG_PRINT("error",("Got error: %d from tempnam",errno));
  }

  environ=(const char**) old_env;
}
#else
{
  #error No implementation found for create_temp_file
}
#endif

  if(file >= 0)
    thread_safe_increment(my_tmp_file_created, &THR_LOCK_open);

  DBUG_RETURN(file);
} //=>mysys/mf_tempfile.c  //zlq


//[sql/sql_class.cc:
//[[sql/mysqld.h: //----2011-10-08-16-45;
//#define mysql_tmpdir (my_tmpdir(&mysql_tmpdir_list))
#define mysql_tmpdir "/tmp"
//]]
/*
  The following functions form part of the C plugin API
*/
int mysql_tmpfile(const char *prefix)
{
  char filename[FN_REFLEN];
  File fd = create_temp_file(filename, mysql_tmpdir, prefix,
#ifdef __WIN__
                             O_BINARY | O_TRUNC | O_SEQUENTIAL |
                             O_SHORT_LIVED |
#endif /* __WIN__ */
                             O_CREAT | O_EXCL | O_RDWR | O_TEMPORARY,
                             MYF(MY_WME));
  if (fd >= 0) {
#ifndef __WIN__
    /*
      This can be removed once the following bug is fixed:
      Bug #28903  create_temp_file() doesn't honor O_TEMPORARY option
                  (file not removed) (Unix)
    */
    unlink(filename);
#endif /* !__WIN__ */
  }

  return fd;
}
//]




#if defined(TESTONLY)
#include <stdio.h>
//
int main(void){
  File f;
#ifdef __WIN__
  //printf("win\n");
#else
  //printf("no win\n");
  char buf[256];
  /*
  f=create_temp_file(buf,
                     "/tmp", "innodb",
                     O_CREAT | O_EXCL | O_RDWR | O_TEMPORARY,  //->mode==>my_open的Flags--->open的flags/“./asm-generic/fcntl.h:#define O_EXCL		00000200	/ * not fcntl * / ... ./asm-generic/fcntl.h:#define O_RDWR		00000002” //----2011-10-08-14-55;
                     MYF(MY_WME));
  */
  f=mysql_tmpfile("innodb");//----2011-10-08-16-39;
  printf("ret=%d\n", f); //----2011-09-22-17-17; //zlq

#endif


}
#endif
//gcc -o test test.c -I./include -I./include2 -DSTACK_DIRECTION=1 -w -g -DTESTONLY=1  //(for: os0file.c的“os_file_create_tmpfile”)----2011-10-08-16-22;



//?HAVE_PSI_INTERFACE
/*
[root@server01 innobase_]# ./innodb
innodb_test1:
1.create database:
InnoDB: The InnoDB memory heap is disabled
InnoDB: Mutexes and rw_locks use InnoDB's own implementation
InnoDB: Compressed tables use zlib 1.2.3
InnoDB: Warning: innodb_file_io_threads is deprecated. Please use innodb_read_io_threads and innodb_write_io_threads instead
111008 15:01:03  InnoDB: Assertion failure in thread 3077454640 in file srv0start.c line 1648
InnoDB: Failing assertion: srv_n_file_io_threads <= SRV_MAX_N_IO_THREADS
InnoDB: We intentionally generate a memory trap.
InnoDB: Submit a detailed bug report to http://bugs.mysql.com.
InnoDB: If you get repeated assertion failures or crashes, even
InnoDB: immediately after the mysqld startup, there may be
InnoDB: corruption in the InnoDB tablespace. Please refer to
InnoDB: http://dev.mysql.com/doc/refman/5.1/en/forcing-recovery.html
InnoDB: about forcing recovery.
已放弃 (core dumped)
[root@server01 innobase_]#
*/

