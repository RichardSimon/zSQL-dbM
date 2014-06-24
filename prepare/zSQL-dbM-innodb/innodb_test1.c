#include <stdio.h>
#include <unistd.h>///=>sleep
//
#include "srv0start.h"
//-
#include "trx0types.h" //"que0que.h"已包含在"srv0start.h";


int main(void){
  printf("innodb_test1: \n");
  printf("1.create database:\n");
  int ret=innobase_start_or_create_for_mysql();
  printf("innobase_start_or_create_for_mysql ret=%d\n", ret);

  //
//#if 0
trx_t* trx = trx_allocate_for_mysql();
/*
que_eval_sql(NULL,
             " \
  CREATE TABLE SYS_FOREIGN( \
    ID CHAR, \
    FOR_NAME CHAR, \
    REF_NAME CHAR, N_COLS INT \
  );"
	     , FALSE, trx);
*/
/*2011-10-10+2011-10-11:
sql=(pars_sql)=>graph=(que_fork_start_command)=>thr ==>que_run_threads
 que_node_print_info
--
que0que.c:1410
    |
    v
 pars0grm.c:1445(gdb时指向下2行！) ->1555 =>1565(/yychar=311) =>1583=>1621
--(--2011-10-11-17-05--17-09:R;)
*/
//----2011-10-11-17-00--17-05:--17-13:[(que0que.c)“graph = pars_sql(info, sql);”=>(pars0pars.c)“yyparse();”]通过！；--15-15但是"que_fork_start_command"却选择了进入“QUE_THR_COMMAND_WAIT”状态/线程进入sleep?；--17-19记录此/R;+----2011-10-11-17-29;
//que_eval_sql(NULL,
//             " \
//PROCEDURE CREATE_FOREIGN_SYS_TABLES_PROC () IS \
//BEGIN \
//  CREATE TABLE SYS_FOREIGN( \
//    ID CHAR, \
//    FOR_NAME CHAR, \
//    REF_NAME CHAR, N_COLS INT \
//  ); \
//  CREATE INDEX FOR_IND ON SYS_FOREIGN (FOR_NAME); \
//END;"
//	     , FALSE, trx);
///*
//[root@server01 InnoSQL-5.5.8]# grep "trx_t;" ./ -r
//./storage/innobase/handler/ha_innodb.h:typedef struct trx_struct trx_t;
//./storage/innobase/include/trx0types.h:typedef struct trx_struct	trx_t;
//*/
//
que_eval_sql(NULL,
             " \
PROCEDURE CREATE_FOREIGN_SYS_TABLES_PROC () IS \
BEGIN \
  CREATE TABLE SYS_FOREIGN_( \
    ID CHAR, \
    FOR_NAME CHAR, \
    REF_NAME CHAR, N_COLS INT \
  ); \
  SELECT * FROM SYS_TABLES; \
END;"
	     , FALSE, trx);
que_eval_sql(NULL,
             " \
PROCEDURE CREATE_FOREIGN_SYS_TABLES_PROC () IS \
BEGIN \
  SELECT * FROM SYS_FOREIGN_; \
END;"
	     , FALSE, trx);
//#endif
for(;;)sleep(100);
  return 0;
}
