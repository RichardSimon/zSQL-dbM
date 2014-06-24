#define UNIV_INTERN
#define ulint  unsigned long
//btr0btr=>data0type
UNIV_INTERN
void
innobase_get_cset_width(
/*====================*/
	ulint	cset,		/*!< in: MySQL charset-collation code */
	ulint*	mbminlen,	/*!< out: minimum length of a char (in bytes) */
	ulint*	mbmaxlen)	/*!< out: maximum length of a char (in bytes) */
{
}

#define ibool  unsigned int
//#include "trx0trx.h"  //=>"struct trx_struct"
//typedef struct trx_struct	trx_t;  //trx0types.h
//btr0btr=>
UNIV_INTERN
ibool
trx_is_interrupted(
/*===============*/
//	trx_t*	trx)	/*!< in: transaction */
	void*	trx)	/*!< in: transaction */
{
}

UNIV_INTERN
ibool
trx_is_strict(
/*==========*/
//	trx_t*	trx)	/*!< in: transaction */
	void*	trx)	/*!< in: transaction */
{
}

UNIV_INTERN
void
innobase_casedn_str(
/*================*/
	char*	a)	/*!< in/out: string to put in lower case */
{
}

UNIV_INTERN
int
innobase_strcasecmp(
/*================*/
	const char*	a,	/*!< in: first string to compare */
	const char*	b)	/*!< in: second string to compare */
{
}

UNIV_INTERN
void
innobase_convert_from_id(
/*=====================*/
	struct charset_info_st*	cs,	/*!< in: the 'from' character set */
	char*			to,	/*!< out: converted identifier */
	const char*		from,	/*!< in: identifier to convert */
	ulint			len)	/*!< in: length of 'to', in bytes */
{
}

UNIV_INTERN
void
innobase_convert_from_table_id(
/*===========================*/
//	struct charset_info_st*	cs,	/*!< in: the 'from' character set */
	void*	cs,	/*!< in: the 'from' character set */
	char*			to,	/*!< out: converted identifier */
	const char*		from,	/*!< in: identifier to convert */
	ulint			len)	/*!< in: length of 'to', in bytes */
{
}

UNIV_INTERN
struct charset_info_st*
innobase_get_charset(
/*=================*/
	void*	mysql_thd)	/*!< in: MySQL thread handle */
{
}

#define size_t  unsigned int
UNIV_INTERN
const char*
innobase_get_stmt(
/*==============*/
	void*	mysql_thd,	/*!< in: MySQL thread handle */
	size_t*	length)		/*!< out: length of the SQL statement */
{
}

/*
UNIV_INTERN
int
innobase_mysql_tmpfile(void)
/ *========================* /
{
}
*/ //----2011-10-08-16-20--16-29--更改回：！！！；；；;
//[innobase_mysql_tmpfile:
 #ifdef __cplusplus //+----2011-10-08-18-00--18-10;/因为“gcc -o $@ -x c $< -I./include -I./include2 -DSTACK_DIRECTION=1 $(CFLAGS)”！！！；；；;
 #include <strings.h>//=>bzero
 #include <string.h>//=>memset
 #include <stdlib.h>//=>malloc
 #include <pthread.h>//=>pthread_mutex_lock
 #include <unistd.h>//=>close
 #include <fcntl.h>//=>open/?unlink  //----2011-10-08-17-00;
 #endif
//extern "C"{ //----2011-10-08-17-01:还是失败！！！；；;
#include "test.c"
//}

#if defined (__WIN__) && defined (MYSQL_DYNAMIC_PLUGIN)  //__WIN__:
extern MYSQL_PLUGIN_IMPORT MY_TMPDIR mysql_tmpdir_list;
/*******************************************************************//**
Map an [OS error] to an [errno] value.
 The OS error number is stored in [_doserrno] and the mapped value is stored in [errno]. */
extern "C" void __cdecl _dosmaperr(unsigned long);/*!< in: OS error value */

/*********************************************************************//**
Creates a temporary file.
@return	temporary file descriptor, or < 0 on error */
extern "C" UNIV_INTERN
int
innobase_mysql_tmpfile(void)  //<=“defined (__WIN__) && ...”
/*========================*/
{
  HANDLE osfh; /*[OS handle] of opened file*/
  int    fd;   /*   [handle] of opened file*/
  //
  char*  tmpdir;                  /*point to the [directory] where to create file */
  TCHAR	 path_buf[MAX_PATH-14];   /*buffer for tmp [file path]. The length cannot be longer than MAX_PATH - 14, or GetTempFileName will fail. */
  char	 filename[MAX_PATH];      /*name of the [tmpfile]*/
  DWORD	 fileaccess=GENERIC_READ | GENERIC_WRITE | DELETE;                  /*OS file access*/
  DWORD	 fileshare =FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE; /*OS file sharing mode*/
  DWORD	 filecreate=CREATE_ALWAYS;                                          /*OS method of open/create*/
  DWORD	 fileattrib=FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE
                   |FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_SEQUENTIAL_SCAN;     /*OS file attribute flags*/
  uint ret;

  DBUG_ENTER("innobase_mysql_tmpfile");

  tmpdir=my_tmpdir(&mysql_tmpdir_list);//=>mysys/mf_tempdir.c
  if(!tmpdir){/*The tmpdir parameter can not be NULL for GetTempFileName.*/
    ret=GetTempPath(sizeof(path_buf), path_buf);/*Use GetTempPath to determine path for temporary files.*/ //A1.1.
    if(ret>sizeof(path_buf) || ret==0){
      _dosmaperr(GetLastError());/*map error*/
      DBUG_RETURN(-1);
    }
    tmpdir=path_buf;                            /**/
  }
//----2011-09-21-12-17;+2011-09-22-08-30--
  if(!GetTempFileName(tmpdir, "ib", 0, filename)){/* Use GetTempFileName to generate a unique filename. */ //A1.2.
    _dosmaperr(GetLastError());	/* map error */
    DBUG_RETURN(-1);
  }
  DBUG_PRINT("info", ("filename: %s", filename));

  /*Open/Create the file.*/
  osfh=CreateFile(filename, fileaccess, fileshare, NULL, filecreate, fileattrib, NULL);
  if(osfh==INVALID_HANDLE_VALUE){
    /* open/create file failed! */
    _dosmaperr(GetLastError());	/* map error */
    DBUG_RETURN(-1);
  }
  //
  do{
    /*Associates a CRT file descriptor with the OS file handle.*/
    fd = _open_osfhandle((intptr_t) osfh, 0);//=>my_winfile.c
  } while (fd == -1 && errno == EINTR);
  if(fd == -1){
    /* Open failed, close the file handle. */
    _dosmaperr(GetLastError());	/* map error */
    CloseHandle(osfh);		/* no need to check if CloseHandle fails */
  }

  DBUG_RETURN(fd);
}
#else //“#if defined (__WIN__) && defined (MYSQL_DYNAMIC_PLUGI)”
/*********************************************************************//**
Creates a temporary file.
@return	temporary file descriptor, or < 0 on error */
//extern "C" UNIV_INTERN  //-2011-10-08-18-11;/因为“gcc -o $@ -x c $< -I./include -I./include2 -DSTACK_DIRECTION=1 $(CFLAGS)”！！！；；/(“ha_innodb__.cc:185: 错误：expected identifier or ‘(’ before string constant”);
int
innobase_mysql_tmpfile(void)  //<=linux等；  //zlq
/*========================*/
{
  File	fd = mysql_tmpfile("ib");//=>"sql/sql_class.cc":mysql_tmpfile-->create_temp_file
  int	fd2= -1;

  if(fd >= 0){
    /*Copy the file descriptor, so that the additional resources allocated by create_temp_file() can be freed by invoking my_close().
      Because the file descriptor returned by this function will be passed to fdopen(),
       it will be closed by invoking [fclose(), which] in turn will invoke close() instead of my_close().
    */
#ifdef _WIN32
    /*Note that on Windows, the integer returned by mysql_tmpfile has no relation to C runtime file descriptor.
      Here, we need to call my_get_osfhandle to get the HANDLE and then [convert it to C runtime filedescriptor].
    */
    {
    HANDLE hFile = my_get_osfhandle(fd);//=>my_winfile.c
    HANDLE hDup;
    BOOL bOK = DuplicateHandle(GetCurrentProcess(), hFile, GetCurrentProcess(), &hDup, 0, FALSE, DUPLICATE_SAME_ACCESS);
    if(bOK){
      fd2 = _open_osfhandle((intptr_t)hDup,0);
    }else{
      my_osmaperr(GetLastError());
      fd2 = -1;
    }
    }
#else
    fd2 = dup(fd);
#endif
    if(fd2 < 0){
      DBUG_PRINT("error",("Got error %d on dup",fd2));
      my_errno=errno;
      my_error(EE_OUT_OF_FILERESOURCES, MYF(ME_BELL+ME_WAITTANG), "ib*", my_errno);
    }
    my_close(fd, MYF(MY_WME));
  }

  return(fd2);
}
#endif /* defined (__WIN__) && defined (MYSQL_DYNAMIC_PLUGIN) */
//innobase_mysql_tmpfile;] //zlq















#define uint  unsigned int
UNIV_INTERN
int
innobase_mysql_cmp(
/*===============*/
	int		mysql_type,	/*!< in: MySQL type */
	uint		charset_number,	/*!< in: number of the charset */
	const unsigned char* a,		/*!< in: data field */
	unsigned int	a_length,	/*!< in: data field length,
					not UNIV_SQL_NULL */
	const unsigned char* b,		/*!< in: data field */
	unsigned int	b_length)	/*!< in: data field length,
					not UNIV_SQL_NULL */
{
}

UNIV_INTERN
void
innobase_invalidate_query_cache(
/*============================*/
//	trx_t*		trx,		/*!< in: transaction which
	void*		trx,		/*!< in: transaction which
					modifies the table */
	const char*	full_name,	/*!< in: concatenation of
					database name, null char NUL,
					table name, null char NUL;
					NOTE that in Windows this is
					always in LOWER CASE! */
	ulint		full_name_len)	/*!< in: full name length where
					also the null chars count */
{
}

//innobase_rec_to_mysql

UNIV_INTERN
void
innobase_mysql_print_thd(
/*=====================*/
//	FILE*	f,		/*!< in: output stream */
	void*	f,		/*!< in: output stream */
	void*	thd,		/*!< in: pointer to a MySQL THD object */
	uint	max_query_len)	/*!< in: max query length to print, or 0 to
				   use the default max length */
{
}

UNIV_INTERN
char*
innobase_convert_name(
/*==================*/
	char*		buf,	/*!< out: buffer for converted identifier */
	ulint		buflen,	/*!< in: length of buf, in bytes */
	const char*	id,	/*!< in: identifier to convert */
	ulint		idlen,	/*!< in: length of id, in bytes */
	void*		thd,	/*!< in: MySQL connection thread, or NULL */
	ibool		table_id)/*!< in: TRUE=id is a table or database name;
				FALSE=id is an index name */
{
}

