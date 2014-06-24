#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>//=>wchar_t
#include <sys/stat.h>
//
#include "engine.h"
#include "ZSQL_InnoDB.h"

#define TRIMREDUCTIONS  0 /* 0=off, 1=on */
#define DEBUG           1 /* 0=off, 1=on */


/* Struct for transporting data between rules. 
   Add whatever you need.
   Note: you could also use [global variables] to store stuff, 
         but using a struct like this [makes the interpreter thread-safe].
*/
struct ContextStruct{
  wchar_t* ReturnValue; /* In this template all rules return a string. */
  int Indent;           /* For printing debug messages. */
  //
  int Debug;            /* 0=off, 1=on */
};

/* Forward definition of the RuleJumpTable. It will be filled with a link to a subroutine for every rule later on. */
void (*RuleJumpTable[])(struct TokenStruct *Token, struct ContextStruct *Context);


/***** Helper subroutines ***************************************************/
/* Make [a readable copy] of a string. 
   All characters outside 32...127 are displayed as a HEX number in square brackets, for example "[0A]". 
*/
void ReadableString(wchar_t* Input, wchar_t* Output, long Width) {
  char s1[BUFSIZ];
  long i1;
  long i2;
  
  /* Sanity check. */
  if ((Output == NULL) || (Width < 1)) return;
  Output[0] = 0;
  if (Input == NULL) return;
  
  i1 = 0;
  i2 = 0;
  while ((i2 < Width - 1) && (Input[i1] != 0)) {
    if ((Input[i1] >= 32) && (Input[i1] <= 127)) {
      Output[i2++] = Input[i1];
    } else {
      if (Width - i2 > 4) {
        sprintf(s1,"%02X",Input[i1]);
        Output[i2++] = '[';
        Output[i2++] = s1[0];
        Output[i2++] = s1[1];
        Output[i2++] = ']';
      }
    }
    i1++;
  }
  Output[i2] = 0;
}

void ShowIndent(int Indent) {
  int i;
  for(i=0; i<Indent; i++){
    fprintf(stdout,"  ");
  }
}


/***** Rule subroutine template *********************************************/
/* This subroutine is a template of things that can happen in the subroutine of a rule. 
   It shows how to access the value of symbols and how to call rules, 
    and how to transport results and stuff via the Context.
   For example the rule:
     <Increment> ::= <Expression> '+' Number
   Has 3 sub-tokens:
     Token->Tokens[0] = token for <Expression>, a rule
     Token->Tokens[1] = token for '+', a symbol
     Token->Tokens[2] = token for Number, a symbol

   We know Token->Tokens[0] is a rule, because that's what the grammar says. 
   We may assume that the engine has fully [populated the sub-tokens] and don't have to perform any checks. 
   We can immediately call the subroutine of the rule, like this:
     RuleJumpTable[Token->Tokens[0]->ReductionRule]( Token->Tokens[0], Context );

   The subroutine should [hand back it's results] via the Context. 
   Here is an example of how to [store a result in the context]:
     Context->ReturnValue = (wchar_t *)wcsdup(....);
   Symbols are literal strings from the input (that was parsed by the engine), [stored in the sub-token]. 
   We can get the value of the "Number" symbol like this:
     Value = (wchar_t *)wcsdup( Token->Tokens[2]->Data );

   Further reading:
   - See "engine.h" for the definition of the TokenStruct.
   - See "readme.txt" for a short discussion on how to use the content
     of a Token.
   - See "example4.c" for a working template example.
*/
void RuleTemplate(struct TokenStruct *Token, struct ContextStruct *Context) {
  int i;
  
  /* Debugging: show the description of the rule. */
  if(Context->Debug > 0){
    ShowIndent(Context->Indent);
    fprintf(stdout,"Executing rule: %s\n",Grammar.RuleArray[Token->ReductionRule].Description);
  }
  /* For all [the sub-Tokens]. */
  for(i=0; i < Grammar.RuleArray[Token->ReductionRule].SymbolsCount; i++){
    if(Token->Tokens[i]->ReductionRule < 0){/* See if the Token is a Symbol or a Rule. */
      /* It's a Symbol. Make a copy of the Data. 
         Most symbols are grammar, for example '+', 'function', 'while', and such, 
          and you won't need to look at the Data. 
         Other symbols are literals from the input script, for example numbers, strings, variable names, and such.
      */
      if (Context->ReturnValue != NULL) free(Context->ReturnValue);
      Context->ReturnValue = (wchar_t *)wcsdup(Token->Tokens[i]->Data);
  /* Debugging: show a description of the Symbol, and it's value. */
  if(Context->Debug > 0){
    ShowIndent(Context->Indent + 1);
    fprintf(stdout, "Token[%u] = Symbol('%s') = '%s'\n",  i, Grammar.SymbolArray[Token->Tokens[i]->Symbol].Name,  Context->ReturnValue);
  }
    }else{                                  /* It's a rule. */
  /* Debugging: show a description of the rule. */
  if(Context->Debug > 0) {
    ShowIndent(Context->Indent + 1);
    fprintf(stdout,"Token[%u] = Rule = %s\n",  i,  Grammar.RuleArray[Token->Tokens[i]->ReductionRule].Description);
  }
      /* Call the [rule's subroutine] via the RuleJumpTable. */
      Context->Indent = Context->Indent + 1;
      RuleJumpTable[Token->Tokens[i]->ReductionRule]( Token->Tokens[i], Context );
      Context->Indent = Context->Indent - 1;
      
      /* At this point you will probably want to save the Context->ReturnValue somewhere. */
      
  /* Debugging: show the value that was returned by the rule's subroutine. */
  if (Context->Debug > 0) {
    ShowIndent(Context->Indent + 2);
    fprintf(stdout,"Result value = %s\n",Context->ReturnValue);
  }
    }
  }/*--“for(i=0; i < Grammar.RuleArray[Token->ReductionRule].SymbolsCount; i++){”*/
  
  /* Do whatever processing is needed by the rule. Remember to free() the Values you have saved. */
  
}
//
//
/***** Rule subroutines *****************************************************/
/* <SQLs> ::= <SQL> <SQLs> */
void Rule_SQLs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLs> ::= <SQL> */
void Rule_SQLs2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQL> ::= <procedure_definition> ';' */
void Rule_SQL_Semi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQL> ::= <statement> */
void Rule_SQL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <procedure_definition> ::= PROCEDURE Id '(' <parameter_declaration_list> ')' IS <variable_declaration_list> <declaration_list> BEGIN <statement_list> END */
void Rule_procedure_definition_PROCEDURE_Id_LParan_RParan_IS_BEGIN_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <parameter_declaration_list> ::= <parameter_declaration_list> ',' <parameter_declaration> */
void Rule_parameter_declaration_list_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <parameter_declaration_list> ::= <parameter_declaration> */
void Rule_parameter_declaration_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <parameter_declaration_list> ::=  */
void Rule_parameter_declaration_list2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <parameter_declaration> ::= Id IN <type_name> */
void Rule_parameter_declaration_Id_IN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <parameter_declaration> ::= Id OUT <type_name> */
void Rule_parameter_declaration_Id_OUT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <type_name> ::= INT */
void Rule_type_name_INT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <type_name> ::= INTEGER */
void Rule_type_name_INTEGER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <type_name> ::= CHAR */
void Rule_type_name_CHAR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <type_name> ::= BINARY */
void Rule_type_name_BINARY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <type_name> ::= BLOB */
void Rule_type_name_BLOB(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <variable_declaration_list> ::= <variable_declaration_list> <variable_declaration> */
void Rule_variable_declaration_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <variable_declaration_list> ::= <variable_declaration> */
void Rule_variable_declaration_list2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <variable_declaration_list> ::=  */
void Rule_variable_declaration_list3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <variable_declaration> ::= Id <type_name> ';' */
void Rule_variable_declaration_Id_Semi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <declaration_list> ::= <declaration_list> <declaration> */
void Rule_declaration_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <declaration_list> ::= <declaration> */
void Rule_declaration_list2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <declaration_list> ::=  */
void Rule_declaration_list3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <declaration> ::= <cursor_declaration> */
void Rule_declaration(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <declaration> ::= <function_declaration> */
void Rule_declaration2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <cursor_declaration> ::= DECLARE CURSOR Id IS <select_statement> ';' */
void Rule_cursor_declaration_DECLARE_CURSOR_Id_IS_Semi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_declaration> ::= DECLARE FUNCTION Id ';' */
void Rule_function_declaration_DECLARE_FUNCTION_Id_Semi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement_list> ::= <statement_list> <statement> */
void Rule_statement_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement_list> ::= <statement> */
void Rule_statement_list2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <stored_procedure_call> */
void Rule_statement(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <predefined_procedure_call> ';' */
void Rule_statement_Semi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <while_statement> ';' */
void Rule_statement_Semi2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <for_statement> ';' */
void Rule_statement_Semi3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <exit_statement> ';' */
void Rule_statement_Semi4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <if_statement> ';' */
void Rule_statement_Semi5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <return_statement> ';' */
void Rule_statement_Semi6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <assignment_statement> ';' */
void Rule_statement_Semi7(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <select_statement> ';' */
void Rule_statement_Semi8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <insert_statement> ';' */
void Rule_statement_Semi9(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <row_printf_statement> ';' */
void Rule_statement_Semi10(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <delete_statement_searched> ';' */
void Rule_statement_Semi11(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <delete_statement_positioned> ';' */
void Rule_statement_Semi12(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <update_statement_searched> ';' */
void Rule_statement_Semi13(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <update_statement_positioned> ';' */
void Rule_statement_Semi14(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <open_cursor_statement> ';' */
void Rule_statement_Semi15(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <fetch_statement> ';' */
void Rule_statement_Semi16(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <close_cursor_statement> ';' */
void Rule_statement_Semi17(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <commit_statement> ';' */
void Rule_statement_Semi18(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <rollback_statement> ';' */
void Rule_statement_Semi19(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <create_table> ';' */
void Rule_statement_Semi20(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <statement> ::= <create_index> ';' */
void Rule_statement_Semi21(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= Id */
void Rule_exp_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <function_name> '(' <exp_list> ')' */
void Rule_exp_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= IntegerLiteral */
void Rule_exp_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= RealLiteral */
void Rule_exp_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= StringLiteral */
void Rule_exp_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= 'PARS_FIXBINARY_LIT' */
void Rule_exp_PARS_FIXBINARY_LIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= 'PARS_BLOB_LIT' */
void Rule_exp_PARS_BLOB_LIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= NULL */
void Rule_exp_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= SQL */
void Rule_exp_SQL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '+' <exp> */
void Rule_exp_Plus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '-' <exp> */
void Rule_exp_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '*' <exp> */
void Rule_exp_Times(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '/' <exp> */
void Rule_exp_Div(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= '(' <exp> ')' */
void Rule_exp_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '=' <exp> */
void Rule_exp_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '<' <exp> */
void Rule_exp_Lt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '>' <exp> */
void Rule_exp_Gt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '>=' <exp> */
void Rule_exp_GtEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '<=' <exp> */
void Rule_exp_LtEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> '<>' <exp> */
void Rule_exp_LtGt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> AND <exp> */
void Rule_exp_AND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= <exp> OR <exp> */
void Rule_exp_OR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= NOT <exp> */
void Rule_exp_NOT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= Id '%' NOTFOUND */
void Rule_exp_Id_Percent_NOTFOUND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp> ::= SQL '%' NOTFOUND */
void Rule_exp_SQL_Percent_NOTFOUND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= 'to_char' */
void Rule_function_name_to_char(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= 'to_number' */
void Rule_function_name_to_number(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= 'to_binary' */
void Rule_function_name_to_binary(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= 'binary_to_number' */
void Rule_function_name_binary_to_number(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= substr */
void Rule_function_name_substr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= concat */
void Rule_function_name_concat(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= instr */
void Rule_function_name_instr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= length */
void Rule_function_name_length(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= sysdate */
void Rule_function_name_sysdate(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= rnd */
void Rule_function_name_rnd(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <function_name> ::= 'rnd_str' */
void Rule_function_name_rnd_str(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp_list> ::= <exp_list> ',' <exp> */
void Rule_exp_list_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp_list> ::= <exp> */
void Rule_exp_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exp_list> ::=  */
void Rule_exp_list2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <stored_procedure_call> ::= '{' Id '(' <question_mark_list> ')' '}' */
void Rule_stored_procedure_call_LBrace_Id_LParan_RParan_RBrace(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <question_mark_list> ::= <question_mark_list> ',' '?' */
void Rule_question_mark_list_Comma_Question(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <question_mark_list> ::= '?' */
void Rule_question_mark_list_Question(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <question_mark_list> ::=  */
void Rule_question_mark_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <predefined_procedure_call> ::= <predefined_procedure_name> '(' <exp_list> ')' */
void Rule_predefined_procedure_call_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <predefined_procedure_name> ::= REPLSTR */
void Rule_predefined_procedure_name_REPLSTR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <predefined_procedure_name> ::= PRINTF */
void Rule_predefined_procedure_name_PRINTF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <predefined_procedure_name> ::= ASSERT */
void Rule_predefined_procedure_name_ASSERT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <while_statement> ::= WHILE <exp> LOOP <statement_list> END LOOP */
void Rule_while_statement_WHILE_LOOP_END_LOOP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <for_statement> ::= FOR Id IN <exp> '..' <exp> LOOP <statement_list> END LOOP */
void Rule_for_statement_FOR_Id_IN_DotDot_LOOP_END_LOOP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <exit_statement> ::= EXIT */
void Rule_exit_statement_EXIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <if_statement> ::= IF <exp> THEN <statement_list> <else_part> END IF */
void Rule_if_statement_IF_THEN_END_IF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <else_part> ::= <elsif_list> */
void Rule_else_part(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <else_part> ::= ELSE <statement_list> */
void Rule_else_part_ELSE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <else_part> ::=  */
void Rule_else_part2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <elsif_list> ::= <elsif_list> <elsif_element> */
void Rule_elsif_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <elsif_list> ::= <elsif_element> */
void Rule_elsif_list2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <elsif_element> ::= ELSIF */
void Rule_elsif_element_ELSIF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <elsif_element> ::= <exp> THEN <statement_list> */
void Rule_elsif_element_THEN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <return_statement> ::= RETURN */
void Rule_return_statement_RETURN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <assignment_statement> ::= Id ':=' <exp> */
void Rule_assignment_statement_Id_ColonEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_statement> ::= SELECT <select_list> FROM <table_list> <search_condition> <for_update_clause> <lock_shared_clause> <order_by_clause> */
void Rule_select_statement_SELECT_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_list> ::= <select_item_list> INTO <variable_list> */
void Rule_select_list_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_list> ::= <select_item_list> */
void Rule_select_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_list> ::= '*' */
void Rule_select_list_Times(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_item_list> ::= <select_item_list> ',' <select_item> */
void Rule_select_item_list_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_item_list> ::= <select_item> */
void Rule_select_item_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_item_list> ::=  */
void Rule_select_item_list2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_item> ::= <exp> */
void Rule_select_item(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_item> ::= count '(' '*' ')' */
void Rule_select_item_count_LParan_Times_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_item> ::= count '(' DISTINCT Id ')' */
void Rule_select_item_count_LParan_DISTINCT_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <select_item> ::= sum '(' <exp> ')' */
void Rule_select_item_sum_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <variable_list> ::= <variable_list> ',' Id */
void Rule_variable_list_Comma_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <variable_list> ::= Id */
void Rule_variable_list_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <variable_list> ::=  */
void Rule_variable_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_list> ::= <table_list> ',' Id */
void Rule_table_list_Comma_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_list> ::= Id */
void Rule_table_list_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <search_condition> ::= WHERE <exp> */
void Rule_search_condition_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <search_condition> ::=  */
void Rule_search_condition(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <for_update_clause> ::= FOR UPDATE */
void Rule_for_update_clause_FOR_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <for_update_clause> ::=  */
void Rule_for_update_clause(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <lock_shared_clause> ::= LOCK IN SHARE MODE */
void Rule_lock_shared_clause_LOCK_IN_SHARE_MODE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <lock_shared_clause> ::=  */
void Rule_lock_shared_clause(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <order_by_clause> ::= ORDER BY Id <order_direction> */
void Rule_order_by_clause_ORDER_BY_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <order_by_clause> ::=  */
void Rule_order_by_clause(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <order_direction> ::= ASC */
void Rule_order_direction_ASC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <order_direction> ::= DESC */
void Rule_order_direction_DESC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <order_direction> ::=  */
void Rule_order_direction(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <insert_statement> ::= <insert_statement_start> VALUES '(' <exp_list> ')' */
void Rule_insert_statement_VALUES_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <insert_statement> ::= <insert_statement_start> <select_statement> */
void Rule_insert_statement(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <insert_statement_start> ::= INSERT INTO Id */
void Rule_insert_statement_start_INSERT_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <row_printf_statement> ::= 'ROW_PRINTF' <select_statement> */
void Rule_row_printf_statement_ROW_PRINTF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <delete_statement_searched> ::= <delete_statement_start> <search_condition> */
void Rule_delete_statement_searched(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <delete_statement_start> ::= DELETE FROM Id */
void Rule_delete_statement_start_DELETE_FROM_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <delete_statement_positioned> ::= <delete_statement_start> <cursor_positioned> */
void Rule_delete_statement_positioned(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <cursor_positioned> ::= WHERE CURRENT OF Id */
void Rule_cursor_positioned_WHERE_CURRENT_OF_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <update_statement_searched> ::= <update_statement_start> <search_condition> */
void Rule_update_statement_searched(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <update_statement_start> ::= UPDATE Id SET <column_assignment_list> */
void Rule_update_statement_start_UPDATE_Id_SET(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_assignment_list> ::= <column_assignment_list> ',' <column_assignment> */
void Rule_column_assignment_list_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_assignment_list> ::= <column_assignment> */
void Rule_column_assignment_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_assignment> ::= Id '=' <exp> */
void Rule_column_assignment_Id_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <update_statement_positioned> ::= <update_statement_start> <cursor_positioned> */
void Rule_update_statement_positioned(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <open_cursor_statement> ::= OPEN Id */
void Rule_open_cursor_statement_OPEN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fetch_statement> ::= FETCH Id INTO <variable_list> */
void Rule_fetch_statement_FETCH_Id_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fetch_statement> ::= FETCH Id INTO <user_function_call> */
void Rule_fetch_statement_FETCH_Id_INTO2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <user_function_call> ::= Id '(' ')' */
void Rule_user_function_call_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <close_cursor_statement> ::= CLOSE Id */
void Rule_close_cursor_statement_CLOSE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <commit_statement> ::= COMMIT WORK */
void Rule_commit_statement_COMMIT_WORK(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <rollback_statement> ::= ROLLBACK WORK */
void Rule_rollback_statement_ROLLBACK_WORK(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <create_table> ::= CREATE TABLE Id '(' <column_def_list> ')' <not_fit_in_memory> */
void Rule_create_table_CREATE_TABLE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_def_list> ::= <column_def_list> ',' <column_def> */
void Rule_column_def_list_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_def_list> ::= <column_def> */
void Rule_column_def_list(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_def> ::= Id <type_name> <opt_column_len> <opt_unsigned> <opt_not_null> */
void Rule_column_def_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <opt_column_len> ::= '(' IntegerLiteral ')' */
void Rule_opt_column_len_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <opt_column_len> ::=  */
void Rule_opt_column_len(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <opt_unsigned> ::= UNSIGNED */
void Rule_opt_unsigned_UNSIGNED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <opt_unsigned> ::=  */
void Rule_opt_unsigned(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <opt_not_null> ::= NOT NULL */
void Rule_opt_not_null_NOT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <opt_not_null> ::=  */
void Rule_opt_not_null(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <not_fit_in_memory> ::= 'DOES_NOT_FIT_IN_MEMORY' */
void Rule_not_fit_in_memory_DOES_NOT_FIT_IN_MEMORY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <not_fit_in_memory> ::=  */
void Rule_not_fit_in_memory(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <create_index> ::= CREATE <unique_def> <clustered_def> INDEX Id ON Id '(' <column_list> ')' */
void Rule_create_index_CREATE_INDEX_Id_ON_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <unique_def> ::= UNIQUE */
void Rule_unique_def_UNIQUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <unique_def> ::=  */
void Rule_unique_def(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <clustered_def> ::= CLUSTERED */
void Rule_clustered_def_CLUSTERED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <clustered_def> ::=  */
void Rule_clustered_def(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_list> ::= <column_list> ',' Id */
void Rule_column_list_Comma_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_list> ::= Id */
void Rule_column_list_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};



/***** Rule jumptable *******************************************************/
void (*RuleJumpTable[])(struct TokenStruct *Token, struct ContextStruct *Context) = {
  /* 0. <SQLs> ::= <SQL> <SQLs> */
  Rule_SQLs,
  /* 1. <SQLs> ::= <SQL> */
  Rule_SQLs2,
  /* 2. <SQL> ::= <procedure_definition> ';' */
  Rule_SQL_Semi,
  /* 3. <SQL> ::= <statement> */
  Rule_SQL,
  /* 4. <procedure_definition> ::= PROCEDURE Id '(' <parameter_declaration_list> ')' IS <variable_declaration_list> <declaration_list> BEGIN <statement_list> END */
  Rule_procedure_definition_PROCEDURE_Id_LParan_RParan_IS_BEGIN_END,
  /* 5. <parameter_declaration_list> ::= <parameter_declaration_list> ',' <parameter_declaration> */
  Rule_parameter_declaration_list_Comma,
  /* 6. <parameter_declaration_list> ::= <parameter_declaration> */
  Rule_parameter_declaration_list,
  /* 7. <parameter_declaration_list> ::=  */
  Rule_parameter_declaration_list2,
  /* 8. <parameter_declaration> ::= Id IN <type_name> */
  Rule_parameter_declaration_Id_IN,
  /* 9. <parameter_declaration> ::= Id OUT <type_name> */
  Rule_parameter_declaration_Id_OUT,
  /* 10. <type_name> ::= INT */
  Rule_type_name_INT,
  /* 11. <type_name> ::= INTEGER */
  Rule_type_name_INTEGER,
  /* 12. <type_name> ::= CHAR */
  Rule_type_name_CHAR,
  /* 13. <type_name> ::= BINARY */
  Rule_type_name_BINARY,
  /* 14. <type_name> ::= BLOB */
  Rule_type_name_BLOB,
  /* 15. <variable_declaration_list> ::= <variable_declaration_list> <variable_declaration> */
  Rule_variable_declaration_list,
  /* 16. <variable_declaration_list> ::= <variable_declaration> */
  Rule_variable_declaration_list2,
  /* 17. <variable_declaration_list> ::=  */
  Rule_variable_declaration_list3,
  /* 18. <variable_declaration> ::= Id <type_name> ';' */
  Rule_variable_declaration_Id_Semi,
  /* 19. <declaration_list> ::= <declaration_list> <declaration> */
  Rule_declaration_list,
  /* 20. <declaration_list> ::= <declaration> */
  Rule_declaration_list2,
  /* 21. <declaration_list> ::=  */
  Rule_declaration_list3,
  /* 22. <declaration> ::= <cursor_declaration> */
  Rule_declaration,
  /* 23. <declaration> ::= <function_declaration> */
  Rule_declaration2,
  /* 24. <cursor_declaration> ::= DECLARE CURSOR Id IS <select_statement> ';' */
  Rule_cursor_declaration_DECLARE_CURSOR_Id_IS_Semi,
  /* 25. <function_declaration> ::= DECLARE FUNCTION Id ';' */
  Rule_function_declaration_DECLARE_FUNCTION_Id_Semi,
  /* 26. <statement_list> ::= <statement_list> <statement> */
  Rule_statement_list,
  /* 27. <statement_list> ::= <statement> */
  Rule_statement_list2,
  /* 28. <statement> ::= <stored_procedure_call> */
  Rule_statement,
  /* 29. <statement> ::= <predefined_procedure_call> ';' */
  Rule_statement_Semi,
  /* 30. <statement> ::= <while_statement> ';' */
  Rule_statement_Semi2,
  /* 31. <statement> ::= <for_statement> ';' */
  Rule_statement_Semi3,
  /* 32. <statement> ::= <exit_statement> ';' */
  Rule_statement_Semi4,
  /* 33. <statement> ::= <if_statement> ';' */
  Rule_statement_Semi5,
  /* 34. <statement> ::= <return_statement> ';' */
  Rule_statement_Semi6,
  /* 35. <statement> ::= <assignment_statement> ';' */
  Rule_statement_Semi7,
  /* 36. <statement> ::= <select_statement> ';' */
  Rule_statement_Semi8,
  /* 37. <statement> ::= <insert_statement> ';' */
  Rule_statement_Semi9,
  /* 38. <statement> ::= <row_printf_statement> ';' */
  Rule_statement_Semi10,
  /* 39. <statement> ::= <delete_statement_searched> ';' */
  Rule_statement_Semi11,
  /* 40. <statement> ::= <delete_statement_positioned> ';' */
  Rule_statement_Semi12,
  /* 41. <statement> ::= <update_statement_searched> ';' */
  Rule_statement_Semi13,
  /* 42. <statement> ::= <update_statement_positioned> ';' */
  Rule_statement_Semi14,
  /* 43. <statement> ::= <open_cursor_statement> ';' */
  Rule_statement_Semi15,
  /* 44. <statement> ::= <fetch_statement> ';' */
  Rule_statement_Semi16,
  /* 45. <statement> ::= <close_cursor_statement> ';' */
  Rule_statement_Semi17,
  /* 46. <statement> ::= <commit_statement> ';' */
  Rule_statement_Semi18,
  /* 47. <statement> ::= <rollback_statement> ';' */
  Rule_statement_Semi19,
  /* 48. <statement> ::= <create_table> ';' */
  Rule_statement_Semi20,
  /* 49. <statement> ::= <create_index> ';' */
  Rule_statement_Semi21,
  /* 50. <exp> ::= Id */
  Rule_exp_Id,
  /* 51. <exp> ::= <function_name> '(' <exp_list> ')' */
  Rule_exp_LParan_RParan,
  /* 52. <exp> ::= IntegerLiteral */
  Rule_exp_IntegerLiteral,
  /* 53. <exp> ::= RealLiteral */
  Rule_exp_RealLiteral,
  /* 54. <exp> ::= StringLiteral */
  Rule_exp_StringLiteral,
  /* 55. <exp> ::= 'PARS_FIXBINARY_LIT' */
  Rule_exp_PARS_FIXBINARY_LIT,
  /* 56. <exp> ::= 'PARS_BLOB_LIT' */
  Rule_exp_PARS_BLOB_LIT,
  /* 57. <exp> ::= NULL */
  Rule_exp_NULL,
  /* 58. <exp> ::= SQL */
  Rule_exp_SQL,
  /* 59. <exp> ::= <exp> '+' <exp> */
  Rule_exp_Plus,
  /* 60. <exp> ::= <exp> '-' <exp> */
  Rule_exp_Minus,
  /* 61. <exp> ::= <exp> '*' <exp> */
  Rule_exp_Times,
  /* 62. <exp> ::= <exp> '/' <exp> */
  Rule_exp_Div,
  /* 63. <exp> ::= '(' <exp> ')' */
  Rule_exp_LParan_RParan2,
  /* 64. <exp> ::= <exp> '=' <exp> */
  Rule_exp_Eq,
  /* 65. <exp> ::= <exp> '<' <exp> */
  Rule_exp_Lt,
  /* 66. <exp> ::= <exp> '>' <exp> */
  Rule_exp_Gt,
  /* 67. <exp> ::= <exp> '>=' <exp> */
  Rule_exp_GtEq,
  /* 68. <exp> ::= <exp> '<=' <exp> */
  Rule_exp_LtEq,
  /* 69. <exp> ::= <exp> '<>' <exp> */
  Rule_exp_LtGt,
  /* 70. <exp> ::= <exp> AND <exp> */
  Rule_exp_AND,
  /* 71. <exp> ::= <exp> OR <exp> */
  Rule_exp_OR,
  /* 72. <exp> ::= NOT <exp> */
  Rule_exp_NOT,
  /* 73. <exp> ::= Id '%' NOTFOUND */
  Rule_exp_Id_Percent_NOTFOUND,
  /* 74. <exp> ::= SQL '%' NOTFOUND */
  Rule_exp_SQL_Percent_NOTFOUND,
  /* 75. <function_name> ::= 'to_char' */
  Rule_function_name_to_char,
  /* 76. <function_name> ::= 'to_number' */
  Rule_function_name_to_number,
  /* 77. <function_name> ::= 'to_binary' */
  Rule_function_name_to_binary,
  /* 78. <function_name> ::= 'binary_to_number' */
  Rule_function_name_binary_to_number,
  /* 79. <function_name> ::= substr */
  Rule_function_name_substr,
  /* 80. <function_name> ::= concat */
  Rule_function_name_concat,
  /* 81. <function_name> ::= instr */
  Rule_function_name_instr,
  /* 82. <function_name> ::= length */
  Rule_function_name_length,
  /* 83. <function_name> ::= sysdate */
  Rule_function_name_sysdate,
  /* 84. <function_name> ::= rnd */
  Rule_function_name_rnd,
  /* 85. <function_name> ::= 'rnd_str' */
  Rule_function_name_rnd_str,
  /* 86. <exp_list> ::= <exp_list> ',' <exp> */
  Rule_exp_list_Comma,
  /* 87. <exp_list> ::= <exp> */
  Rule_exp_list,
  /* 88. <exp_list> ::=  */
  Rule_exp_list2,
  /* 89. <stored_procedure_call> ::= '{' Id '(' <question_mark_list> ')' '}' */
  Rule_stored_procedure_call_LBrace_Id_LParan_RParan_RBrace,
  /* 90. <question_mark_list> ::= <question_mark_list> ',' '?' */
  Rule_question_mark_list_Comma_Question,
  /* 91. <question_mark_list> ::= '?' */
  Rule_question_mark_list_Question,
  /* 92. <question_mark_list> ::=  */
  Rule_question_mark_list,
  /* 93. <predefined_procedure_call> ::= <predefined_procedure_name> '(' <exp_list> ')' */
  Rule_predefined_procedure_call_LParan_RParan,
  /* 94. <predefined_procedure_name> ::= REPLSTR */
  Rule_predefined_procedure_name_REPLSTR,
  /* 95. <predefined_procedure_name> ::= PRINTF */
  Rule_predefined_procedure_name_PRINTF,
  /* 96. <predefined_procedure_name> ::= ASSERT */
  Rule_predefined_procedure_name_ASSERT,
  /* 97. <while_statement> ::= WHILE <exp> LOOP <statement_list> END LOOP */
  Rule_while_statement_WHILE_LOOP_END_LOOP,
  /* 98. <for_statement> ::= FOR Id IN <exp> '..' <exp> LOOP <statement_list> END LOOP */
  Rule_for_statement_FOR_Id_IN_DotDot_LOOP_END_LOOP,
  /* 99. <exit_statement> ::= EXIT */
  Rule_exit_statement_EXIT,
  /* 100. <if_statement> ::= IF <exp> THEN <statement_list> <else_part> END IF */
  Rule_if_statement_IF_THEN_END_IF,
  /* 101. <else_part> ::= <elsif_list> */
  Rule_else_part,
  /* 102. <else_part> ::= ELSE <statement_list> */
  Rule_else_part_ELSE,
  /* 103. <else_part> ::=  */
  Rule_else_part2,
  /* 104. <elsif_list> ::= <elsif_list> <elsif_element> */
  Rule_elsif_list,
  /* 105. <elsif_list> ::= <elsif_element> */
  Rule_elsif_list2,
  /* 106. <elsif_element> ::= ELSIF */
  Rule_elsif_element_ELSIF,
  /* 107. <elsif_element> ::= <exp> THEN <statement_list> */
  Rule_elsif_element_THEN,
  /* 108. <return_statement> ::= RETURN */
  Rule_return_statement_RETURN,
  /* 109. <assignment_statement> ::= Id ':=' <exp> */
  Rule_assignment_statement_Id_ColonEq,
  /* 110. <select_statement> ::= SELECT <select_list> FROM <table_list> <search_condition> <for_update_clause> <lock_shared_clause> <order_by_clause> */
  Rule_select_statement_SELECT_FROM,
  /* 111. <select_list> ::= <select_item_list> INTO <variable_list> */
  Rule_select_list_INTO,
  /* 112. <select_list> ::= <select_item_list> */
  Rule_select_list,
  /* 113. <select_list> ::= '*' */
  Rule_select_list_Times,
  /* 114. <select_item_list> ::= <select_item_list> ',' <select_item> */
  Rule_select_item_list_Comma,
  /* 115. <select_item_list> ::= <select_item> */
  Rule_select_item_list,
  /* 116. <select_item_list> ::=  */
  Rule_select_item_list2,
  /* 117. <select_item> ::= <exp> */
  Rule_select_item,
  /* 118. <select_item> ::= count '(' '*' ')' */
  Rule_select_item_count_LParan_Times_RParan,
  /* 119. <select_item> ::= count '(' DISTINCT Id ')' */
  Rule_select_item_count_LParan_DISTINCT_Id_RParan,
  /* 120. <select_item> ::= sum '(' <exp> ')' */
  Rule_select_item_sum_LParan_RParan,
  /* 121. <variable_list> ::= <variable_list> ',' Id */
  Rule_variable_list_Comma_Id,
  /* 122. <variable_list> ::= Id */
  Rule_variable_list_Id,
  /* 123. <variable_list> ::=  */
  Rule_variable_list,
  /* 124. <table_list> ::= <table_list> ',' Id */
  Rule_table_list_Comma_Id,
  /* 125. <table_list> ::= Id */
  Rule_table_list_Id,
  /* 126. <search_condition> ::= WHERE <exp> */
  Rule_search_condition_WHERE,
  /* 127. <search_condition> ::=  */
  Rule_search_condition,
  /* 128. <for_update_clause> ::= FOR UPDATE */
  Rule_for_update_clause_FOR_UPDATE,
  /* 129. <for_update_clause> ::=  */
  Rule_for_update_clause,
  /* 130. <lock_shared_clause> ::= LOCK IN SHARE MODE */
  Rule_lock_shared_clause_LOCK_IN_SHARE_MODE,
  /* 131. <lock_shared_clause> ::=  */
  Rule_lock_shared_clause,
  /* 132. <order_by_clause> ::= ORDER BY Id <order_direction> */
  Rule_order_by_clause_ORDER_BY_Id,
  /* 133. <order_by_clause> ::=  */
  Rule_order_by_clause,
  /* 134. <order_direction> ::= ASC */
  Rule_order_direction_ASC,
  /* 135. <order_direction> ::= DESC */
  Rule_order_direction_DESC,
  /* 136. <order_direction> ::=  */
  Rule_order_direction,
  /* 137. <insert_statement> ::= <insert_statement_start> VALUES '(' <exp_list> ')' */
  Rule_insert_statement_VALUES_LParan_RParan,
  /* 138. <insert_statement> ::= <insert_statement_start> <select_statement> */
  Rule_insert_statement,
  /* 139. <insert_statement_start> ::= INSERT INTO Id */
  Rule_insert_statement_start_INSERT_INTO_Id,
  /* 140. <row_printf_statement> ::= 'ROW_PRINTF' <select_statement> */
  Rule_row_printf_statement_ROW_PRINTF,
  /* 141. <delete_statement_searched> ::= <delete_statement_start> <search_condition> */
  Rule_delete_statement_searched,
  /* 142. <delete_statement_start> ::= DELETE FROM Id */
  Rule_delete_statement_start_DELETE_FROM_Id,
  /* 143. <delete_statement_positioned> ::= <delete_statement_start> <cursor_positioned> */
  Rule_delete_statement_positioned,
  /* 144. <cursor_positioned> ::= WHERE CURRENT OF Id */
  Rule_cursor_positioned_WHERE_CURRENT_OF_Id,
  /* 145. <update_statement_searched> ::= <update_statement_start> <search_condition> */
  Rule_update_statement_searched,
  /* 146. <update_statement_start> ::= UPDATE Id SET <column_assignment_list> */
  Rule_update_statement_start_UPDATE_Id_SET,
  /* 147. <column_assignment_list> ::= <column_assignment_list> ',' <column_assignment> */
  Rule_column_assignment_list_Comma,
  /* 148. <column_assignment_list> ::= <column_assignment> */
  Rule_column_assignment_list,
  /* 149. <column_assignment> ::= Id '=' <exp> */
  Rule_column_assignment_Id_Eq,
  /* 150. <update_statement_positioned> ::= <update_statement_start> <cursor_positioned> */
  Rule_update_statement_positioned,
  /* 151. <open_cursor_statement> ::= OPEN Id */
  Rule_open_cursor_statement_OPEN_Id,
  /* 152. <fetch_statement> ::= FETCH Id INTO <variable_list> */
  Rule_fetch_statement_FETCH_Id_INTO,
  /* 153. <fetch_statement> ::= FETCH Id INTO <user_function_call> */
  Rule_fetch_statement_FETCH_Id_INTO2,
  /* 154. <user_function_call> ::= Id '(' ')' */
  Rule_user_function_call_Id_LParan_RParan,
  /* 155. <close_cursor_statement> ::= CLOSE Id */
  Rule_close_cursor_statement_CLOSE_Id,
  /* 156. <commit_statement> ::= COMMIT WORK */
  Rule_commit_statement_COMMIT_WORK,
  /* 157. <rollback_statement> ::= ROLLBACK WORK */
  Rule_rollback_statement_ROLLBACK_WORK,
  /* 158. <create_table> ::= CREATE TABLE Id '(' <column_def_list> ')' <not_fit_in_memory> */
  Rule_create_table_CREATE_TABLE_Id_LParan_RParan,
  /* 159. <column_def_list> ::= <column_def_list> ',' <column_def> */
  Rule_column_def_list_Comma,
  /* 160. <column_def_list> ::= <column_def> */
  Rule_column_def_list,
  /* 161. <column_def> ::= Id <type_name> <opt_column_len> <opt_unsigned> <opt_not_null> */
  Rule_column_def_Id,
  /* 162. <opt_column_len> ::= '(' IntegerLiteral ')' */
  Rule_opt_column_len_LParan_IntegerLiteral_RParan,
  /* 163. <opt_column_len> ::=  */
  Rule_opt_column_len,
  /* 164. <opt_unsigned> ::= UNSIGNED */
  Rule_opt_unsigned_UNSIGNED,
  /* 165. <opt_unsigned> ::=  */
  Rule_opt_unsigned,
  /* 166. <opt_not_null> ::= NOT NULL */
  Rule_opt_not_null_NOT_NULL,
  /* 167. <opt_not_null> ::=  */
  Rule_opt_not_null,
  /* 168. <not_fit_in_memory> ::= 'DOES_NOT_FIT_IN_MEMORY' */
  Rule_not_fit_in_memory_DOES_NOT_FIT_IN_MEMORY,
  /* 169. <not_fit_in_memory> ::=  */
  Rule_not_fit_in_memory,
  /* 170. <create_index> ::= CREATE <unique_def> <clustered_def> INDEX Id ON Id '(' <column_list> ')' */
  Rule_create_index_CREATE_INDEX_Id_ON_Id_LParan_RParan,
  /* 171. <unique_def> ::= UNIQUE */
  Rule_unique_def_UNIQUE,
  /* 172. <unique_def> ::=  */
  Rule_unique_def,
  /* 173. <clustered_def> ::= CLUSTERED */
  Rule_clustered_def_CLUSTERED,
  /* 174. <clustered_def> ::=  */
  Rule_clustered_def,
  /* 175. <column_list> ::= <column_list> ',' Id */
  Rule_column_list_Comma_Id,
  /* 176. <column_list> ::= Id */
  Rule_column_list_Id 
  };


/***** Main *****************************************************************/
/* Load input file from disk into memory. */
wchar_t* LoadInputFile(char* FileName){
  unsigned long i;
  FILE* Fin;
  char*    Buf1;
  wchar_t* Buf2;
  struct stat statbuf;
  size_t BytesRead;
  
  /* Sanity check. */
  if ((FileName == NULL) || (*FileName == '\0')) return(NULL);
  /* Open the file. */
  Fin = fopen(FileName,"rb");
  if (Fin == NULL) {
    fprintf(stdout,"Could not open input file: %s\n",FileName);
    return(NULL);
  }
  /* Get the size of the file. */
  if (fstat(fileno(Fin),&statbuf) != 0) {
    fprintf(stdout,"Could not stat() the input file: %s\n",FileName);
    fclose(Fin);
    return(NULL);
  }
  
  /* Allocate memory for the input. */
  Buf1 = (char *)malloc(statbuf.st_size + 1);
  Buf2 = (wchar_t *)malloc(sizeof(wchar_t) * (statbuf.st_size + 1));
  if ((Buf1 == NULL) || (Buf2 == NULL)) {
    fprintf(stdout,"Not enough memory to load the file: %s\n",FileName);
    fclose(Fin);
    if (Buf1 != NULL) free(Buf1);
    if (Buf2 != NULL) free(Buf2);
    return(NULL);
  }
  /* Load the file into memory. */
  BytesRead = fread(Buf1,1,statbuf.st_size,Fin);
  Buf1[BytesRead] = '\0';

  /* Close the file. */
  fclose(Fin);
  /* Exit if there was an error while reading the file. */
  if (BytesRead != statbuf.st_size) {
    fprintf(stdout,"Error while reading input file: %s\n",FileName);
    free(Buf1);
    free(Buf2);
    return(NULL);
  }
  /* Convert from ASCII to Unicode. */
  for (i = 0; i <= BytesRead; i++) Buf2[i] = Buf1[i];
  free(Buf1);
  
  return(Buf2);
}
//--
void ShowErrorMessage(struct TokenStruct *Token, int Result) {
  int Symbol;
  int i;
  wchar_t s1[BUFSIZ];
  
  switch(Result) {
  case PARSELEXICALERROR:
    fprintf(stdout,"Lexical error");
    break;
  case PARSECOMMENTERROR:
    fprintf(stdout,"Comment error");
    break;
  case PARSETOKENERROR:
    fprintf(stdout,"Tokenizer error");
    break;
  case PARSESYNTAXERROR:
    fprintf(stdout,"Syntax error");
    break;
  case PARSEMEMORYERROR:
    fprintf(stdout,"Out of memory");
    break;
  }
  if (Token != NULL) fprintf(stdout," at line %d column %d",Token->Line,Token->Column);
  fprintf(stdout,".\n");
  if (Result == PARSELEXICALERROR) {
    if (Token->Data != NULL) {
      ReadableString(Token->Data,s1,BUFSIZ);
      fprintf(stdout,"The grammar does not specify what to do with '%S'.\n",s1);
    } else {
      fprintf(stdout,"The grammar does not specify what to do.\n");
    }
  }
  if (Result == PARSETOKENERROR) {
    fprintf(stdout,"The tokenizer returned a non-terminal.\n");
  }
  if (Result == PARSECOMMENTERROR) {
    fprintf(stdout,"The comment has no end, it was started but not finished.\n");
  }
  if (Result == PARSESYNTAXERROR) {
    if (Token->Data != NULL) {
      ReadableString(Token->Data,s1,BUFSIZ);
      fprintf(stdout,"Encountered '%S', but expected ",s1);
    } else {
      fprintf(stdout,"Expected ");
    }
    for (i = 0; i < Grammar.LalrArray[Token->Symbol].ActionCount; i++) {
      Symbol = Grammar.LalrArray[Token->Symbol].Actions[i].Entry;
      if (Grammar.SymbolArray[Symbol].Kind == SYMBOLTERMINAL) {
        if (i > 0) {
          fprintf(stdout,", ");
          if (i >= Grammar.LalrArray[Token->Symbol].ActionCount - 2) fprintf(stdout,"or ");
        }
        fprintf(stdout,"'%S'",Grammar.SymbolArray[Symbol].Name);
      }
    }
    fprintf(stdout,".\n");
  }
}
int main(int argc, char *argv[]){
  int Result;
  wchar_t *InputBuf;
  struct TokenStruct *Token;
  struct ContextStruct Context;
  
  /* Load the inputfile into memory. */
  InputBuf = LoadInputFile("Example.input");
  if (InputBuf == NULL) exit(1);
  
  /* Run the Parser. */
  Result = Parse(InputBuf, wcslen(InputBuf),  TRIMREDUCTIONS, DEBUG,   &Token);//zlq
  
  /* Interpret the results. */
  if (Result != PARSEACCEPT) {
    ShowErrorMessage(Token,Result);
  } else {
    /* Initialize the Context. */
    Context.Debug = DEBUG;
    Context.Indent = 0;
    Context.ReturnValue = NULL;
    /* Start execution by calling the subroutine of the first Token on the TokenStack. 
       It's the "Start Symbol" that is defined in the grammar.
    */
    RuleJumpTable[Token->ReductionRule]( Token, &Context );//zlq
  }
  
  /* Cleanup. */
  DeleteTokens(Token);
  free(InputBuf);
}
