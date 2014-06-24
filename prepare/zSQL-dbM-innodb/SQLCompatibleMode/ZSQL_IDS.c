//17328
//6544
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>//=>wchar_t
#include <sys/stat.h>
//-
#include <string.h>
//
#include "engine.h"
#include "ZSQL_IDS.h"

#define TRIMREDUCTIONS  0 /* 0=off, 1=on */
#define DEBUG           0 /* 0=off, 1=on */


/* Struct for transporting data between rules. 
   Add whatever you need.
   Note: you could also use [global variables] to store stuff, 
         but using a struct like this [makes the interpreter thread-safe].
*/
struct ContextStruct{
  wchar_t* ReturnValue; /* In this template all rules return a string. */
  //
  int Indent;           /* For printing debug messages. */
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
//void RuleTemplate(struct TokenStruct *Token, struct ContextStruct *Context) {
void _RuleTemplate(struct TokenStruct *Token, struct ContextStruct *Context) {
//(engine.h:)
///* Output from the parser. */
//struct TokenStruct{
//  int                  ReductionRule; /* Index into Grammar.RuleArray[]. */
//  struct TokenStruct** Tokens;        /* Array of reduction Tokens. */
//  int      Symbol;                    /* Index into Grammar.SymbolArray[]. */
//  wchar_t* Data;                      /* String with data from the input. */
//  long Line;                          /* Line number in the input. */
//  long Column;                        /* Column in the input. */
//};
//(ZSQL_IDS.c:)
///* Struct for transporting data between rules. 
//   Add whatever you need.
//   Note: you could also use [global variables] to store stuff, 
//         but using a struct like this [makes the interpreter thread-safe].
//*/
//struct ContextStruct{
//  wchar_t* ReturnValue; /* In this template all rules return a string. */
//  //
//  int Indent;           /* For printing debug messages. */
//  int Debug;            /* 0=off, 1=on */
//};
  int i;
  
  /* Debugging: show the description of the rule. */
  if(Context->Debug > 0){
    ShowIndent(Context->Indent);
    //fprintf(stdout,"Executing rule: %s\n",Grammar.RuleArray[Token->ReductionRule].Description);
    fprintf(stdout,"Executing rule: %S\n", Grammar.RuleArray[Token->ReductionRule].Description);
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
    //fprintf(stdout, "Token[%u] = Symbol('%s') = '%s'\n",  i, Grammar.SymbolArray[Token->Tokens[i]->Symbol].Name,  Context->ReturnValue);
    fprintf(stdout, "Token[%u] = Symbol('%S') = '%S'\n",  i, Grammar.SymbolArray[Token->Tokens[i]->Symbol].Name,  Context->ReturnValue);
  }
    }else{                                  /* It's a rule. */
  /* Debugging: show a description of the rule. */
  if(Context->Debug > 0) {
    ShowIndent(Context->Indent + 1);
    //fprintf(stdout,"Token[%u] = Rule = %s\n",  i,  Grammar.RuleArray[Token->Tokens[i]->ReductionRule].Description);
    fprintf(stdout,"Token[%u] = Rule = %S\n",  i,  Grammar.RuleArray[Token->Tokens[i]->ReductionRule].Description);
  }
      /* Call the [rule's subroutine] via the RuleJumpTable. */
      Context->Indent = Context->Indent + 1;
      RuleJumpTable[Token->Tokens[i]->ReductionRule]( Token->Tokens[i], Context );
      Context->Indent = Context->Indent - 1;
      
      /* At this point you will probably want to save the Context->ReturnValue somewhere. */
      
  /* Debugging: show the value that was returned by the rule's subroutine. */
  if (Context->Debug > 0) {
    ShowIndent(Context->Indent + 2);
    //fprintf(stdout,"Result value = %s\n",Context->ReturnValue);
    fprintf(stdout, "Result value = %S\n", Context->ReturnValue);
  }
    }
  }/*--“for(i=0; i < Grammar.RuleArray[Token->ReductionRule].SymbolsCount; i++){”*/
  
  /* Do whatever processing is needed by the rule. Remember to free() the Values you have saved. */
  
}
//
//void RuleTemplate2(struct TokenStruct* Token, struct ContextStruct* Context){
void RuleTemplate(struct TokenStruct* Token, struct ContextStruct* Context){
//struct TokenStruct{
//  int                  ReductionRule; /* Index into Grammar.RuleArray[]. */
//  struct TokenStruct** Tokens;        /* Array of reduction Tokens. */
//  int      Symbol;                    /* Index into Grammar.SymbolArray[]. */
//  wchar_t* Data;                      /* String with data from the input. */
//  long Line;                          /* Line number in the input. */
//  long Column;                        /* Column in the input. */
//};
//struct ContextStruct{
//  wchar_t* ReturnValue; /* In this template all rules return a string. */
//  //
//  int Indent;           /* For printing debug messages. */
//  int Debug;            /* 0=off, 1=on */
//};
//return;
  int i;
  
  if(Context->Debug > 0){
    //ShowIndent(Context->Indent);    fprintf(stdout,"Executing rule: %s\n",Grammar.RuleArray[Token->ReductionRule].Description);
    ShowIndent(Context->Indent);    fprintf(stdout, "Executing rule: %S\n", Grammar.RuleArray[Token->ReductionRule].Description);
  }
  /* For all [the sub-Tokens]. */
  for(i=0; i < Grammar.RuleArray[Token->ReductionRule].SymbolsCount; i++){
    if(Token->Tokens[i]->ReductionRule < 0){/* It's a Symbol. */ 
  if(Context->Debug > 0){
    //ShowIndent(Context->Indent + 1);    fprintf(stdout, "Token[%u] = Symbol('%s') = '%s'\n",  i, Grammar.SymbolArray[ Token->Tokens[i]->Symbol ].Name, Token->Tokens[i]->Data);
    ShowIndent(Context->Indent + 1);    fprintf(stdout, "Token[%u] = Symbol('%S') = '%S'\n",  i, Grammar.SymbolArray[ Token->Tokens[i]->Symbol ].Name, Token->Tokens[i]->Data);
  }
      if(Context->ReturnValue!=NULL)free(Context->ReturnValue);
      Context->ReturnValue=(wchar_t*)wcsdup(Token->Tokens[i]->Data);
    }else{                                  /* It's a rule. */
  if(Context->Debug > 0){
    //ShowIndent(Context->Indent + 1);    fprintf(stdout,"Token[%u] = Rule = %s\n",             i, Grammar.RuleArray[ Token->Tokens[i]->ReductionRule ].Description);
    ShowIndent(Context->Indent + 1);    fprintf(stdout,"Token[%u] = Rule = %S\n",             i, Grammar.RuleArray[ Token->Tokens[i]->ReductionRule ].Description);
  }
      Context->Indent = Context->Indent + 1;
      RuleJumpTable[Token->Tokens[i]->ReductionRule]( Token->Tokens[i], Context );
      Context->Indent = Context->Indent - 1;
      //
      /* At this point you will probably want to save the Context->ReturnValue somewhere. */
      //...
  if(Context->Debug > 0){
    //ShowIndent(Context->Indent + 2);    fprintf(stdout, "Result value = %s\n", Context->ReturnValue);
    ShowIndent(Context->Indent + 2);    fprintf(stdout, "Result value = %S\n", Context->ReturnValue);
  }
    }
  }/*--“for(i=0; i < Grammar.RuleArray[Token->ReductionRule].SymbolsCount; i++){”*/
}

//
//
/***** Rule subroutines *****************************************************/
/* <Id List> ::= <Id Member> <Id List_> */
void Rule_IdList(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id List_> ::= ',' <Id Member> <Id List_> */
void Rule_IdList__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id List_> ::=  */
void Rule_IdList_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id Member> ::= <CREATE_QUEUE_object> Id */
void Rule_IdMember_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id Member> ::= <CREATE_QUEUE_object> */
void Rule_IdMember(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE_QUEUE_object> ::= Id */
void Rule_CREATE_QUEUE_object_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE_QUEUE_object> ::= LocalTempTable */
void Rule_CREATE_QUEUE_object_LocalTempTable(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE_QUEUE_object> ::= GlobalTempTable */
void Rule_CREATE_QUEUE_object_GlobalTempTable(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Expression> ::= <And Exp> OR <Expression> */
void Rule_Expression_OR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Expression> ::= <And Exp> */
void Rule_Expression(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <And Exp> ::= <Not Exp> AND <And Exp> */
void Rule_AndExp_AND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <And Exp> ::= <Not Exp> */
void Rule_AndExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Not Exp> ::= NOT <Pred Exp> */
void Rule_NotExp_NOT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Not Exp> ::= <Pred Exp> */
void Rule_NotExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> BETWEEN <Add Exp> AND <Add Exp> */
void Rule_PredExp_BETWEEN_AND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> NOT BETWEEN <Add Exp> AND <Add Exp> */
void Rule_PredExp_NOT_BETWEEN_AND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Value> IS NOT NULL */
void Rule_PredExp_IS_NOT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= Id IS NOT NULL */
void Rule_PredExp_Id_IS_NOT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Value> IS NULL */
void Rule_PredExp_IS_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= Id IS NULL */
void Rule_PredExp_Id_IS_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> LIKE StringLiteral */
void Rule_PredExp_LIKE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> LIKE LocalVarId */
void Rule_PredExp_LIKE_LocalVarId(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> LIKE GlobalVarId */
void Rule_PredExp_LIKE_GlobalVarId(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> IN '(' <SELECT> ')' */
void Rule_PredExp_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> IN '(' <Expr List> ')' */
void Rule_PredExp_IN_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> '=' <Add Exp> */
void Rule_PredExp_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> '<>' <Add Exp> */
void Rule_PredExp_LtGt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> '!=' <Add Exp> */
void Rule_PredExp_ExclamEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> '>' <Add Exp> */
void Rule_PredExp_Gt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> '>=' <Add Exp> */
void Rule_PredExp_GtEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> '<' <Add Exp> */
void Rule_PredExp_Lt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> '<=' <Add Exp> */
void Rule_PredExp_LtEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Pred Exp> ::= <Add Exp> */
void Rule_PredExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Add Exp> ::= <Add Exp> '+' <Mult Exp> */
void Rule_AddExp_Plus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Add Exp> ::= <Add Exp> '-' <Mult Exp> */
void Rule_AddExp_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Add Exp> ::= <Add Exp> '||' <Mult Exp> */
void Rule_AddExp_PipePipe(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Add Exp> ::= <Mult Exp> */
void Rule_AddExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '*' '-' <Value> */
void Rule_MultExp_Times_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '*' '-' <CREATE_QUEUE_object> */
void Rule_MultExp_Times_Minus2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '*' <Value> */
void Rule_MultExp_Times(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '*' <CREATE_QUEUE_object> */
void Rule_MultExp_Times2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '/' '-' <Value> */
void Rule_MultExp_Div_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '/' '-' <CREATE_QUEUE_object> */
void Rule_MultExp_Div_Minus2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '/' <Value> */
void Rule_MultExp_Div(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Mult Exp> '/' <CREATE_QUEUE_object> */
void Rule_MultExp_Div2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= '-' <Value> */
void Rule_MultExp_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= '-' <CREATE_QUEUE_object> */
void Rule_MultExp_Minus2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <Value> */
void Rule_MultExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Mult Exp> ::= <CREATE_QUEUE_object> */
void Rule_MultExp2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= '(' <Expr List> ')' */
void Rule_Value_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= LocalVarId */
void Rule_Value_LocalVarId(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= GlobalVarId */
void Rule_Value_GlobalVarId(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= <Value_Common> */
void Rule_Value(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= CASE <CASE_WHEN_THENs> ELSE <Expression> END */
void Rule_Value_CASE_ELSE_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= CASE <CASE_WHEN_THENs> END */
void Rule_Value_CASE_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= CASE <Expression> <CASE_WHEN_THENs> ELSE <Expression> END */
void Rule_Value_CASE_ELSE_END2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= CASE <Expression> <CASE_WHEN_THENs> END */
void Rule_Value_CASE_END2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= '(' <SELECT> ')' */
void Rule_Value_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value> ::= <SETquery> */
void Rule_Value2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value_Common> ::= IntegerLiteral */
void Rule_Value_Common_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value_Common> ::= RealLiteral */
void Rule_Value_Common_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value_Common> ::= StringLiteral */
void Rule_Value_Common_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value_Common> ::= NULL */
void Rule_Value_Common_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Value_Common> ::= <FuncCall> */
void Rule_Value_Common(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE_WHEN_THENs> ::= WHEN <Expression> THEN <Expression> <CASE_WHEN_THENs> */
void Rule_CASE_WHEN_THENs_WHEN_THEN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE_WHEN_THENs> ::= WHEN <Expression> THEN <Expression> */
void Rule_CASE_WHEN_THENs_WHEN_THEN2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Expr List> ::= <Expression> ',' <Expr List> */
void Rule_ExprList_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Expr List> ::= <Expression> */
void Rule_ExprList(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLs_> ::= <SQL_> <SQLs_> */
void Rule_SQLs_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLs_> ::= <SQL_> */
void Rule_SQLs_2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQL_> ::= <IDS> <SemicolonOpt> */
void Rule_SQL_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLs> ::= <SQLs_> */
void Rule_SQLs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLs> ::= <DEFINE> <SemicolonOpt> */
void Rule_SQLs2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER ACCESS_METHOD> */
void Rule_IDS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER FRAGMENT> */
void Rule_IDS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER FUNCTION> */
void Rule_IDS3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER INDEX> */
void Rule_IDS4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER PROCEDURE> */
void Rule_IDS5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER ROUTINE> */
void Rule_IDS6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER SECURITY LABEL COMPONENT> */
void Rule_IDS7(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER SEQUENCE> */
void Rule_IDS8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ALTER TABLE> */
void Rule_IDS9(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CLOSE DATABASE> */
void Rule_IDS10(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE ACCESS_METHOD> */
void Rule_IDS11(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE AGGREGATE> */
void Rule_IDS12(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE CAST> */
void Rule_IDS13(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE DATABASE> */
void Rule_IDS14(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE DISTINCT TYPE> */
void Rule_IDS15(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE DUPLICATE> */
void Rule_IDS16(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE EXTERNAL TABLE> */
void Rule_IDS17(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE FUNCTION> */
void Rule_IDS18(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE INDEX> */
void Rule_IDS19(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE OPAQUE TYPE> */
void Rule_IDS20(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE OPCLASS> */
void Rule_IDS21(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE PROCEDURE> */
void Rule_IDS22(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE ROLE> */
void Rule_IDS23(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE ROUTINE FROM> */
void Rule_IDS24(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE ROW TYPE> */
void Rule_IDS25(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE SCHEMA> */
void Rule_IDS26(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE SCRATCH TABLE> */
void Rule_IDS27(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE SECURITY LABEL> */
void Rule_IDS28(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE SECURITY LABEL COMPONENT> */
void Rule_IDS29(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE SECURITY POLICY> */
void Rule_IDS30(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE SEQUENCE> */
void Rule_IDS31(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE SYNONYM> */
void Rule_IDS32(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE TABLE> */
void Rule_IDS33(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE TEMP TABLE> */
void Rule_IDS34(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE TRIGGER> */
void Rule_IDS35(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE VIEW> */
void Rule_IDS36(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE XADATASOURCE> */
void Rule_IDS37(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CREATE XADATASOURCE TYPE> */
void Rule_IDS38(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP ACCESS_METHOD> */
void Rule_IDS39(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP AGGREGATE> */
void Rule_IDS40(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP CAST> */
void Rule_IDS41(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP DATABASE> */
void Rule_IDS42(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP DUPLICATE> */
void Rule_IDS43(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP FUNCTION> */
void Rule_IDS44(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP INDEX> */
void Rule_IDS45(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP OPCLASS> */
void Rule_IDS46(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP PROCEDURE> */
void Rule_IDS47(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP ROLE> */
void Rule_IDS48(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP ROUTINE> */
void Rule_IDS49(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP ROW TYPE> */
void Rule_IDS50(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP SECURITY> */
void Rule_IDS51(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP SEQUENCE> */
void Rule_IDS52(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP SYNONYM> */
void Rule_IDS53(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP TABLE> */
void Rule_IDS54(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP TRIGGER> */
void Rule_IDS55(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP TYPE> */
void Rule_IDS56(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP VIEW> */
void Rule_IDS57(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP XADATASOURCE> */
void Rule_IDS58(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DROP XADATASOURCE TYPE> */
void Rule_IDS59(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <MOVE TABLE> */
void Rule_IDS60(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <RENAME COLUMN> */
void Rule_IDS61(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <RENAME DATABASE> */
void Rule_IDS62(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <RENAME INDEX> */
void Rule_IDS63(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <RENAME SECURITY> */
void Rule_IDS64(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <RENAME SEQUENCE> */
void Rule_IDS65(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <RENAME TABLE> */
void Rule_IDS66(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DELETE> */
void Rule_IDS67(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <INSERT> */
void Rule_IDS68(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <LOAD> */
void Rule_IDS69(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <MERGE> */
void Rule_IDS70(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SELECT> */
void Rule_IDS71(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <TRUNCATE> */
void Rule_IDS72(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <UNLOAD> */
void Rule_IDS73(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <UPDATE> */
void Rule_IDS74(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CLOSE> */
void Rule_IDS75(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DECLARE> */
void Rule_IDS76(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <FETCH> */
void Rule_IDS77(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <FLUSH> */
void Rule_IDS78(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <FREE> */
void Rule_IDS79(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <OPEN> */
void Rule_IDS80(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <PUT> */
void Rule_IDS81(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET AUTOFREE> */
void Rule_IDS82(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <EXECUTE IMMEDIATE> */
void Rule_IDS83(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <INFO> */
void Rule_IDS84(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <GRANT> */
void Rule_IDS85(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <GRANT FRAGMENT> */
void Rule_IDS86(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <LOCK TABLE> */
void Rule_IDS87(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <REVOKE> */
void Rule_IDS88(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <REVOKE FRAGMENT> */
void Rule_IDS89(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET ISOLATION> */
void Rule_IDS90(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET LOCK MODE> */
void Rule_IDS91(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET ROLE> */
void Rule_IDS92(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET SESSION AUTHORIZATION> */
void Rule_IDS93(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET TRANSACTION> */
void Rule_IDS94(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <UNLOCK TABLE> */
void Rule_IDS95(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <BEGIN WORK> */
void Rule_IDS96(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <COMMIT WORK> */
void Rule_IDS97(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ROLLBACK WORK> */
void Rule_IDS98(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET CONSTRAINTS> */
void Rule_IDS99(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET INDEXES> */
void Rule_IDS100(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET TRIGGERS> */
void Rule_IDS101(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET LOG> */
void Rule_IDS102(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET PLOAD FILE> */
void Rule_IDS103(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <START VIOLATIONS TABLE> */
void Rule_IDS104(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <STOP VIOLATIONS TABLE> */
void Rule_IDS105(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SAVEPOINT> */
void Rule_IDS106(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SAVE EXTERNAL DIRECTIVES> */
void Rule_IDS107(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET ALL_MUTABLES> */
void Rule_IDS108(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET Default Table Space> */
void Rule_IDS109(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET Default Table Type> */
void Rule_IDS110(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET ENVIRONMENT> */
void Rule_IDS111(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET EXPLAIN> */
void Rule_IDS112(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET OPTIMIZATION> */
void Rule_IDS113(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET PDQPRIORITY> */
void Rule_IDS114(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET Residency> */
void Rule_IDS115(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET SCHEDULE LEVEL> */
void Rule_IDS116(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET STATEMENT CACHE> */
void Rule_IDS117(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <UPDATE STATISTICS> */
void Rule_IDS118(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <EXECUTE FUNCTION> */
void Rule_IDS119(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <EXECUTE PROCEDURE> */
void Rule_IDS120(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET DEBUG FILE TO> */
void Rule_IDS121(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <OUTPUT> */
void Rule_IDS122(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <GET DIAGNOSTICS> */
void Rule_IDS123(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET COLLATION> */
void Rule_IDS124(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET DATASKIP> */
void Rule_IDS125(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET ENCRYPTION PASSWORD> */
void Rule_IDS126(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <WHENEVER> */
void Rule_IDS127(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <CONNECT> */
void Rule_IDS128(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DATABASE> */
void Rule_IDS129(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DISCONNECT> */
void Rule_IDS130(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SET CONNECTION> */
void Rule_IDS131(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <SPL> */
void Rule_IDS132(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <DISKINIT> */
void Rule_IDS133(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <onbar> */
void Rule_IDS134(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ontape> */
void Rule_IDS135(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <oncheck> */
void Rule_IDS136(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <onmode> */
void Rule_IDS137(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <onparams> */
void Rule_IDS138(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <ondblog> */
void Rule_IDS139(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <onlog> */
void Rule_IDS140(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IDS> ::= <onaudit> */
void Rule_IDS141(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <Label> */
void Rule_SPL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <CALL> */
void Rule_SPL2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <CASE> */
void Rule_SPL3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <CONTINUE> */
void Rule_SPL4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <EXIT> */
void Rule_SPL5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <FOR> */
void Rule_SPL6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <FOREACH> */
void Rule_SPL7(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <GOTO> */
void Rule_SPL8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <IF> */
void Rule_SPL9(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <LET> */
void Rule_SPL10(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <ON EXCEPTION> */
void Rule_SPL11(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <RAISE EXCEPTION> */
void Rule_SPL12(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <RETURN> */
void Rule_SPL13(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <SYSTEM> */
void Rule_SPL14(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <TRACE> */
void Rule_SPL15(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPL> ::= <WHILE> */
void Rule_SPL16(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ACCESS_METHOD> ::= ALTER 'ACCESS_METHOD' Id <ALTER ACCESS_METHODoptionS> */
void Rule_ALTERACCESS_METHOD_ALTER_ACCESS_METHOD_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ACCESS_METHODoptionS> ::= <ALTER ACCESS_METHODoption> <ALTER ACCESS_METHODoptionS_> */
void Rule_ALTERACCESS_METHODoptionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ACCESS_METHODoptionS_> ::= ',' <ALTER ACCESS_METHODoption> <ALTER ACCESS_METHODoptionS_> */
void Rule_ALTERACCESS_METHODoptionS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ACCESS_METHODoptionS_> ::=  */
void Rule_ALTERACCESS_METHODoptionS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ACCESS_METHODoption> ::= MODIFY <PurposeOption> */
void Rule_ALTERACCESS_METHODoption_MODIFY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ACCESS_METHODoption> ::= ADD <PurposeOption> */
void Rule_ALTERACCESS_METHODoption_ADD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ACCESS_METHODoption> ::= DROP <PurposeKeyword> */
void Rule_ALTERACCESS_METHODoption_DROP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_sptype' */
void Rule_PurposeKeyword_am_sptype(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_defopclass' */
void Rule_PurposeKeyword_am_defopclass(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_keyscan' */
void Rule_PurposeKeyword_am_keyscan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_unique' */
void Rule_PurposeKeyword_am_unique(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_cluster' */
void Rule_PurposeKeyword_am_cluster(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_rowids' */
void Rule_PurposeKeyword_am_rowids(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_readwrite' */
void Rule_PurposeKeyword_am_readwrite(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_parallel' */
void Rule_PurposeKeyword_am_parallel(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_costfactor' */
void Rule_PurposeKeyword_am_costfactor(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_create' */
void Rule_PurposeKeyword_am_create(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_drop' */
void Rule_PurposeKeyword_am_drop(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_open' */
void Rule_PurposeKeyword_am_open(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_close' */
void Rule_PurposeKeyword_am_close(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_insert' */
void Rule_PurposeKeyword_am_insert(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_delete' */
void Rule_PurposeKeyword_am_delete(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_update' */
void Rule_PurposeKeyword_am_update(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_stats' */
void Rule_PurposeKeyword_am_stats(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_scancost' */
void Rule_PurposeKeyword_am_scancost(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_check' */
void Rule_PurposeKeyword_am_check(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_beginscan' */
void Rule_PurposeKeyword_am_beginscan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_endscan' */
void Rule_PurposeKeyword_am_endscan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_rescan' */
void Rule_PurposeKeyword_am_rescan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_getnext' */
void Rule_PurposeKeyword_am_getnext(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_getbyid' */
void Rule_PurposeKeyword_am_getbyid(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword> ::= 'am_truncate' */
void Rule_PurposeKeyword_am_truncate(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption> ::= <PurposeKeyword> '=' Id */
void Rule_PurposeOption_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption> ::= <PurposeKeyword> '=' StringLiteral */
void Rule_PurposeOption_Eq_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption> ::= <PurposeKeyword> */
void Rule_PurposeOption(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption> ::= <PurposeKeyword> '=' IntegerLiteral */
void Rule_PurposeOption_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption> ::= <PurposeKeyword> '=' RealLiteral */
void Rule_PurposeOption_Eq_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_attach> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_detach> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_init> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_add> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_drop> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_modify> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_init> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_add> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_drop> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_modify> */
void Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach> ::= ATTACH <ALTER FRAGMENT_attach_S> */
void Rule_ALTERFRAGMENT_attach_ATTACH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_S> ::= <ALTER FRAGMENT_attach_> <ALTER FRAGMENT_attach_S_> */
void Rule_ALTERFRAGMENT_attach_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_S_> ::= ',' <ALTER FRAGMENT_attach_> <ALTER FRAGMENT_attach_S_> */
void Rule_ALTERFRAGMENT_attach_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_S_> ::=  */
void Rule_ALTERFRAGMENT_attach_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> AFTER Id */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_AFTER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> Id */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> BEFORE Id */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_BEFORE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id REMAINDER */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_REMAINDER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> AFTER Id */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_AFTER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> Id */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> BEFORE Id */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_BEFORE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> */
void Rule_ALTERFRAGMENT_attach_0_Id_AS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_0> ::= Id AS REMAINDER */
void Rule_ALTERFRAGMENT_attach_0_Id_AS_REMAINDER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_> ::= <ALTER FRAGMENT_attach_0> */
void Rule_ALTERFRAGMENT_attach_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_attach_> ::= Id */
void Rule_ALTERFRAGMENT_attach__Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_detach> ::= DETACH PARTITION Id Id */
void Rule_ALTERFRAGMENT_detach_DETACH_PARTITION_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_detach> ::= DETACH Id Id */
void Rule_ALTERFRAGMENT_detach_DETACH_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> <FRAGMENT BY_TABLE0> */
void Rule_ALTERFRAGMENT_init_INIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> <FRAGMENT BY_TABLE1> */
void Rule_ALTERFRAGMENT_init_INIT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> PARTITION Id IN Id */
void Rule_ALTERFRAGMENT_init_INIT_PARTITION_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> IN Id */
void Rule_ALTERFRAGMENT_init_INIT_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ROWIDSopt> ::= WITH ROWIDS */
void Rule_ROWIDSopt_WITH_ROWIDS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ROWIDSopt> ::=  */
void Rule_ROWIDSopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE1> ::= <FRAGMENT BY_INDEXopt1> EXPRESSION <FRAGMENT BY_Expr List1> */
void Rule_FRAGMENTBY_TABLE1_EXPRESSION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr List1> ::= <FRAGMENT BY_Expr1> <FRAGMENT BY_Expr List1_> */
void Rule_FRAGMENTBY_ExprList1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr List1_> ::= ',' <FRAGMENT BY_Expr1> <FRAGMENT BY_Expr List1_> */
void Rule_FRAGMENTBY_ExprList1__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr List1_> ::=  */
void Rule_FRAGMENTBY_ExprList1_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr1> ::= PARTITION Id REMAINDER IN Id */
void Rule_FRAGMENTBY_Expr1_PARTITION_Id_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr1> ::= REMAINDER IN Id */
void Rule_FRAGMENTBY_Expr1_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add> ::= ADD <FRAGMENT BY_Expr1> */
void Rule_ALTERFRAGMENT_add_ADD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add> ::= ADD PARTITION Id REMAINDER IN PARTITION Id IN Id */
void Rule_ALTERFRAGMENT_add_ADD_PARTITION_Id_REMAINDER_IN_PARTITION_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add> ::= ADD REMAINDER IN PARTITION Id IN Id */
void Rule_ALTERFRAGMENT_add_ADD_REMAINDER_IN_PARTITION_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add> ::= ADD PARTITION Id IN Id */
void Rule_ALTERFRAGMENT_add_ADD_PARTITION_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add> ::= ADD IN Id */
void Rule_ALTERFRAGMENT_add_ADD_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add> ::= ADD <ALTER FRAGMENT_add_0> */
void Rule_ALTERFRAGMENT_add_ADD2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> IN Id AFTER Id */
void Rule_ALTERFRAGMENT_add_0_PARTITION_Id_IN_Id_AFTER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> IN Id Id */
void Rule_ALTERFRAGMENT_add_0_PARTITION_Id_IN_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> IN Id BEFORE Id */
void Rule_ALTERFRAGMENT_add_0_PARTITION_Id_IN_Id_BEFORE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> */
void Rule_ALTERFRAGMENT_add_0_PARTITION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= <Expression> IN Id AFTER Id */
void Rule_ALTERFRAGMENT_add_0_IN_Id_AFTER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= <Expression> IN Id Id */
void Rule_ALTERFRAGMENT_add_0_IN_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= <Expression> IN Id BEFORE Id */
void Rule_ALTERFRAGMENT_add_0_IN_Id_BEFORE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_add_0> ::= <Expression> */
void Rule_ALTERFRAGMENT_add_0(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_drop> ::= DROP PARTITION Id */
void Rule_ALTERFRAGMENT_drop_DROP_PARTITION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_drop> ::= DROP Id */
void Rule_ALTERFRAGMENT_drop_DROP_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify> ::= MODIFY <ALTER FRAGMENT_modify_S> */
void Rule_ALTERFRAGMENT_modify_MODIFY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_S> ::= <ALTER FRAGMENT_modify_> <ALTER FRAGMENT_modify_S_> */
void Rule_ALTERFRAGMENT_modify_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_S_> ::= ',' <ALTER FRAGMENT_modify_> <ALTER FRAGMENT_modify_S_> */
void Rule_ALTERFRAGMENT_modify_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_S_> ::=  */
void Rule_ALTERFRAGMENT_modify_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= PARTITION Id TO PARTITION Id <Expression> IN Id */
void Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_PARTITION_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= Id TO PARTITION Id <Expression> IN Id */
void Rule_ALTERFRAGMENT_modify__Id_TO_PARTITION_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= PARTITION Id TO <Expression> IN Id */
void Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= Id TO <Expression> IN Id */
void Rule_ALTERFRAGMENT_modify__Id_TO_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= PARTITION Id TO PARTITION Id REMAINDER IN Id */
void Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_PARTITION_Id_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= Id TO PARTITION Id REMAINDER IN Id */
void Rule_ALTERFRAGMENT_modify__Id_TO_PARTITION_Id_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= PARTITION Id TO REMAINDER IN Id */
void Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FRAGMENT_modify_> ::= Id TO REMAINDER IN Id */
void Rule_ALTERFRAGMENT_modify__Id_TO_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION> ::= ALTER FUNCTION Id '(' <TypeList> ')' WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERFUNCTION_ALTER_FUNCTION_Id_LParan_RParan_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION> ::= ALTER FUNCTION Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERFUNCTION_ALTER_FUNCTION_Id_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION> ::= ALTER SPECIFIC FUNCTION Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERFUNCTION_ALTER_SPECIFIC_FUNCTION_Id_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TypeList> ::= <Type> <TypeList_> */
void Rule_TypeList(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TypeList_> ::= ',' <Type> <TypeList_> */
void Rule_TypeList__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TypeList_> ::=  */
void Rule_TypeList_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= CHAR '(' IntegerLiteral ')' */
void Rule_Type_CHAR_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= CHARACTER '(' IntegerLiteral ')' */
void Rule_Type_CHARACTER_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= NCHAR '(' IntegerLiteral ')' */
void Rule_Type_NCHAR_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= VARCHAR '(' IntegerLiteral ',' IntegerLiteral ')' */
void Rule_Type_VARCHAR_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= CHARACTER VARYING '(' IntegerLiteral ',' IntegerLiteral ')' */
void Rule_Type_CHARACTER_VARYING_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= NVARCHAR '(' IntegerLiteral ',' IntegerLiteral ')' */
void Rule_Type_NVARCHAR_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= VARCHAR '(' IntegerLiteral ')' */
void Rule_Type_VARCHAR_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= CHARACTER VARYING '(' IntegerLiteral ')' */
void Rule_Type_CHARACTER_VARYING_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= NVARCHAR '(' IntegerLiteral ')' */
void Rule_Type_NVARCHAR_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= VARCHAR */
void Rule_Type_VARCHAR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= CHARACTER VARYING */
void Rule_Type_CHARACTER_VARYING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= NVARCHAR */
void Rule_Type_NVARCHAR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= LVARCHAR '(' IntegerLiteral ')' */
void Rule_Type_LVARCHAR_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= LVARCHAR */
void Rule_Type_LVARCHAR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= DECIMAL <NUMERICopt> */
void Rule_Type_DECIMAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= DEC <NUMERICopt> */
void Rule_Type_DEC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= NUMERIC <NUMERICopt> */
void Rule_Type_NUMERIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= MONEY <NUMERICopt> */
void Rule_Type_MONEY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= INT */
void Rule_Type_INT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= INTEGER */
void Rule_Type_INTEGER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= 'INT8' */
void Rule_Type_INT8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= BIGINT */
void Rule_Type_BIGINT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= SMAILLINT */
void Rule_Type_SMAILLINT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= BIGSERIAL '(' IntegerLiteral ')' */
void Rule_Type_BIGSERIAL_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= SERIAL '(' IntegerLiteral ')' */
void Rule_Type_SERIAL_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= 'SERIAL8' '(' IntegerLiteral ')' */
void Rule_Type_SERIAL8_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= BIGSERIAL */
void Rule_Type_BIGSERIAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= SERIAL */
void Rule_Type_SERIAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= 'SERIAL8' */
void Rule_Type_SERIAL8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= FLOAT '(' IntegerLiteral ')' */
void Rule_Type_FLOAT_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= DOUBLE PRECISION '(' IntegerLiteral ')' */
void Rule_Type_DOUBLE_PRECISION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= FLOAT */
void Rule_Type_FLOAT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= DOUBLE PRECISION */
void Rule_Type_DOUBLE_PRECISION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= SMALLFLOAT */
void Rule_Type_SMALLFLOAT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= REAL */
void Rule_Type_REAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= TEXT <TEXTopt> */
void Rule_Type_TEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= BYTE <TEXTopt> */
void Rule_Type_BYTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= BLOB */
void Rule_Type_BLOB(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= CLOB */
void Rule_Type_CLOB(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= DATE */
void Rule_Type_DATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= INTERVAL <INTERVALopt> */
void Rule_Type_INTERVAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= DATETIME <DATETIMEopt> */
void Rule_Type_DATETIME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= BOOLEAN */
void Rule_Type_BOOLEAN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= IDSSECURITYLABEL */
void Rule_Type_IDSSECURITYLABEL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= Id */
void Rule_Type_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= ROW '(' <TypeList> ')' */
void Rule_Type_ROW_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= ROW */
void Rule_Type_ROW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= COLLECTION */
void Rule_Type_COLLECTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= SET <SETopt> */
void Rule_Type_SET(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= MULTISET <SETopt> */
void Rule_Type_MULTISET(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Type> ::= LIST <SETopt> */
void Rule_Type_LIST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <NUMERICopt> ::= '(' IntegerLiteral ',' IntegerLiteral ')' */
void Rule_NUMERICopt_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <NUMERICopt> ::= '(' IntegerLiteral ')' */
void Rule_NUMERICopt_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <NUMERICopt> ::=  */
void Rule_NUMERICopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TEXTopt> ::= IN TABLE */
void Rule_TEXTopt_IN_TABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TEXTopt> ::= IN Id */
void Rule_TEXTopt_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TEXTopt> ::=  */
void Rule_TEXTopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO DAY */
void Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_DAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO HOUR */
void Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO MINUTE */
void Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO SECOND */
void Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO FRACTION */
void Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY TO DAY */
void Rule_INTERVALopt_DAY_TO_DAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY TO HOUR */
void Rule_INTERVALopt_DAY_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY TO MINUTE */
void Rule_INTERVALopt_DAY_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY TO SECOND */
void Rule_INTERVALopt_DAY_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_DAY_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= DAY TO FRACTION */
void Rule_INTERVALopt_DAY_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO HOUR */
void Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO MINUTE */
void Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO SECOND */
void Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO FRACTION */
void Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR TO HOUR */
void Rule_INTERVALopt_HOUR_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR TO MINUTE */
void Rule_INTERVALopt_HOUR_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR TO SECOND */
void Rule_INTERVALopt_HOUR_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_HOUR_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= HOUR TO FRACTION */
void Rule_INTERVALopt_HOUR_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO MINUTE */
void Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO SECOND */
void Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO FRACTION */
void Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE TO MINUTE */
void Rule_INTERVALopt_MINUTE_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE TO SECOND */
void Rule_INTERVALopt_MINUTE_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_MINUTE_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MINUTE TO FRACTION */
void Rule_INTERVALopt_MINUTE_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= SECOND '(' IntegerLiteral ')' TO SECOND */
void Rule_INTERVALopt_SECOND_LParan_IntegerLiteral_RParan_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= SECOND '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_SECOND_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= SECOND '(' IntegerLiteral ')' TO FRACTION */
void Rule_INTERVALopt_SECOND_LParan_IntegerLiteral_RParan_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= SECOND TO SECOND */
void Rule_INTERVALopt_SECOND_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= SECOND TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_SECOND_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= SECOND TO FRACTION */
void Rule_INTERVALopt_SECOND_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= FRACTION TO FRACTION '(' IntegerLiteral ')' */
void Rule_INTERVALopt_FRACTION_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= FRACTION TO FRACTION */
void Rule_INTERVALopt_FRACTION_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= YEAR '(' IntegerLiteral ')' TO YEAR */
void Rule_INTERVALopt_YEAR_LParan_IntegerLiteral_RParan_TO_YEAR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= YEAR '(' IntegerLiteral ')' TO MONTH */
void Rule_INTERVALopt_YEAR_LParan_IntegerLiteral_RParan_TO_MONTH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= YEAR TO YEAR */
void Rule_INTERVALopt_YEAR_TO_YEAR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= YEAR TO MONTH */
void Rule_INTERVALopt_YEAR_TO_MONTH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MONTH '(' IntegerLiteral ')' TO MONTH */
void Rule_INTERVALopt_MONTH_LParan_IntegerLiteral_RParan_TO_MONTH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::= MONTH TO MONTH */
void Rule_INTERVALopt_MONTH_TO_MONTH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INTERVALopt> ::=  */
void Rule_INTERVALopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO YEAR */
void Rule_DATETIMEopt_YEAR_TO_YEAR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO MONTH */
void Rule_DATETIMEopt_YEAR_TO_MONTH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO DAY */
void Rule_DATETIMEopt_YEAR_TO_DAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO HOUR */
void Rule_DATETIMEopt_YEAR_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO MINUTE */
void Rule_DATETIMEopt_YEAR_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO SECOND */
void Rule_DATETIMEopt_YEAR_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO FRACTION '(' IntegerLiteral ')' */
void Rule_DATETIMEopt_YEAR_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= YEAR TO FRACTION */
void Rule_DATETIMEopt_YEAR_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MONTH TO MONTH */
void Rule_DATETIMEopt_MONTH_TO_MONTH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MONTH TO DAY */
void Rule_DATETIMEopt_MONTH_TO_DAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MONTH TO HOUR */
void Rule_DATETIMEopt_MONTH_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MONTH TO MINUTE */
void Rule_DATETIMEopt_MONTH_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MONTH TO SECOND */
void Rule_DATETIMEopt_MONTH_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MONTH TO FRACTION '(' IntegerLiteral ')' */
void Rule_DATETIMEopt_MONTH_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MONTH TO FRACTION */
void Rule_DATETIMEopt_MONTH_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= DAY TO DAY */
void Rule_DATETIMEopt_DAY_TO_DAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= DAY TO HOUR */
void Rule_DATETIMEopt_DAY_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= DAY TO MINUTE */
void Rule_DATETIMEopt_DAY_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= DAY TO SECOND */
void Rule_DATETIMEopt_DAY_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= DAY TO FRACTION '(' IntegerLiteral ')' */
void Rule_DATETIMEopt_DAY_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= DAY TO FRACTION */
void Rule_DATETIMEopt_DAY_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= HOUR TO HOUR */
void Rule_DATETIMEopt_HOUR_TO_HOUR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= HOUR TO MINUTE */
void Rule_DATETIMEopt_HOUR_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= HOUR TO SECOND */
void Rule_DATETIMEopt_HOUR_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= HOUR TO FRACTION '(' IntegerLiteral ')' */
void Rule_DATETIMEopt_HOUR_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= HOUR TO FRACTION */
void Rule_DATETIMEopt_HOUR_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MINUTE TO MINUTE */
void Rule_DATETIMEopt_MINUTE_TO_MINUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MINUTE TO SECOND */
void Rule_DATETIMEopt_MINUTE_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MINUTE TO FRACTION '(' IntegerLiteral ')' */
void Rule_DATETIMEopt_MINUTE_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= MINUTE TO FRACTION */
void Rule_DATETIMEopt_MINUTE_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= SECOND TO SECOND */
void Rule_DATETIMEopt_SECOND_TO_SECOND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= SECOND TO FRACTION '(' IntegerLiteral ')' */
void Rule_DATETIMEopt_SECOND_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= SECOND TO FRACTION */
void Rule_DATETIMEopt_SECOND_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= FRACTION TO FRACTION '(' IntegerLiteral ')' */
void Rule_DATETIMEopt_FRACTION_TO_FRACTION_LParan_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::= FRACTION TO FRACTION */
void Rule_DATETIMEopt_FRACTION_TO_FRACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATETIMEopt> ::=  */
void Rule_DATETIMEopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SETopt> ::= '(' <Type> NOT NULL ')' */
void Rule_SETopt_LParan_NOT_NULL_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SETopt> ::= '(' <Type> ')' */
void Rule_SETopt_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION_WITHs> ::= <ALTER FUNCTION_WITH> <ALTER FUNCTION_WITHs_> */
void Rule_ALTERFUNCTION_WITHs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION_WITHs_> ::= ',' <ALTER FUNCTION_WITH> <ALTER FUNCTION_WITHs_> */
void Rule_ALTERFUNCTION_WITHs__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION_WITHs_> ::=  */
void Rule_ALTERFUNCTION_WITHs_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION_WITH> ::= ADD <SPLdescriptor1> */
void Rule_ALTERFUNCTION_WITH_ADD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION_WITH> ::= MODIFY <SPLdescriptor1> */
void Rule_ALTERFUNCTION_WITH_MODIFY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION_WITH> ::= DROP <SPLdescriptor2> */
void Rule_ALTERFUNCTION_WITH_DROP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER FUNCTION_WITH> ::= MODIFY EXTERNAL NAME StringLiteral */
void Rule_ALTERFUNCTION_WITH_MODIFY_EXTERNAL_NAME_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= NOT VARIANT */
void Rule_SPLdescriptor1_NOT_VARIANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= VARIANT */
void Rule_SPLdescriptor1_VARIANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= NEGATOR '=' Id */
void Rule_SPLdescriptor1_NEGATOR_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= CLASS '=' Id */
void Rule_SPLdescriptor1_CLASS_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= ITERATOR */
void Rule_SPLdescriptor1_ITERATOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= PARALLELIZABLE */
void Rule_SPLdescriptor1_PARALLELIZABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= HANDLESNULLS */
void Rule_SPLdescriptor1_HANDLESNULLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= INTERNAL */
void Rule_SPLdescriptor1_INTERNAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= 'PERCALL_COST' '=' IntegerLiteral */
void Rule_SPLdescriptor1_PERCALL_COST_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= COSTFUNC '=' Id */
void Rule_SPLdescriptor1_COSTFUNC_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= SELFUNC '=' Id */
void Rule_SPLdescriptor1_SELFUNC_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= SELCONST '=' IntegerLiteral */
void Rule_SPLdescriptor1_SELCONST_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor1> ::= STACK '=' IntegerLiteral */
void Rule_SPLdescriptor1_STACK_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= NOT VARIANT */
void Rule_SPLdescriptor2_NOT_VARIANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= VARIANT */
void Rule_SPLdescriptor2_VARIANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= NEGATOR */
void Rule_SPLdescriptor2_NEGATOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= CLASS */
void Rule_SPLdescriptor2_CLASS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= ITERATOR */
void Rule_SPLdescriptor2_ITERATOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= PARALLELIZABLE */
void Rule_SPLdescriptor2_PARALLELIZABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= HANDLESNULLS */
void Rule_SPLdescriptor2_HANDLESNULLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= INTERNAL */
void Rule_SPLdescriptor2_INTERNAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= 'PERCALL_COST' */
void Rule_SPLdescriptor2_PERCALL_COST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= SELFUNC */
void Rule_SPLdescriptor2_SELFUNC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= SELCONST */
void Rule_SPLdescriptor2_SELCONST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SPLdescriptor2> ::= STACK */
void Rule_SPLdescriptor2_STACK(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER INDEX> ::= ALTER INDEX Id TO NOT CLUSTER */
void Rule_ALTERINDEX_ALTER_INDEX_Id_TO_NOT_CLUSTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER INDEX> ::= ALTER INDEX Id TO CLUSTER */
void Rule_ALTERINDEX_ALTER_INDEX_Id_TO_CLUSTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER INDEX> ::= ALTER INDEX Id <IndexLockmode> */
void Rule_ALTERINDEX_ALTER_INDEX_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER PROCEDURE> ::= ALTER PROCEDURE Id '(' <TypeList> ')' WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERPROCEDURE_ALTER_PROCEDURE_Id_LParan_RParan_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER PROCEDURE> ::= ALTER PROCEDURE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERPROCEDURE_ALTER_PROCEDURE_Id_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER PROCEDURE> ::= ALTER SPECIFIC PROCEDURE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERPROCEDURE_ALTER_SPECIFIC_PROCEDURE_Id_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ROUTINE> ::= ALTER ROUTINE Id '(' <TypeList> ')' WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERROUTINE_ALTER_ROUTINE_Id_LParan_RParan_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ROUTINE> ::= ALTER ROUTINE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERROUTINE_ALTER_ROUTINE_Id_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER ROUTINE> ::= ALTER SPECIFIC ROUTINE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
void Rule_ALTERROUTINE_ALTER_SPECIFIC_ROUTINE_Id_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCE> ::= ALTER SEQUENCE Id <ALTER SEQUENCEoptionS> */
void Rule_ALTERSEQUENCE_ALTER_SEQUENCE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoptionS> ::= <ALTER SEQUENCEoption> <ALTER SEQUENCEoptionS_> */
void Rule_ALTERSEQUENCEoptionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoptionS_> ::= ',' <ALTER SEQUENCEoption> <ALTER SEQUENCEoptionS_> */
void Rule_ALTERSEQUENCEoptionS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoptionS_> ::=  */
void Rule_ALTERSEQUENCEoptionS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= NOCYCLE */
void Rule_ALTERSEQUENCEoption_NOCYCLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= CYCLE */
void Rule_ALTERSEQUENCEoption_CYCLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= CACHE IntegerLiteral */
void Rule_ALTERSEQUENCEoption_CACHE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= NOCACHE */
void Rule_ALTERSEQUENCEoption_NOCACHE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= ORDER */
void Rule_ALTERSEQUENCEoption_ORDER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= NOORDER */
void Rule_ALTERSEQUENCEoption_NOORDER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= INCREMENT BY IntegerLiteral */
void Rule_ALTERSEQUENCEoption_INCREMENT_BY_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= INCREMENT IntegerLiteral */
void Rule_ALTERSEQUENCEoption_INCREMENT_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= RESTART WITH IntegerLiteral */
void Rule_ALTERSEQUENCEoption_RESTART_WITH_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= RESTART IntegerLiteral */
void Rule_ALTERSEQUENCEoption_RESTART_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= NOMAXVALUE */
void Rule_ALTERSEQUENCEoption_NOMAXVALUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= MAXVALUE IntegerLiteral */
void Rule_ALTERSEQUENCEoption_MAXVALUE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= NOMINVALUE */
void Rule_ALTERSEQUENCEoption_NOMINVALUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SEQUENCEoption> ::= MINVALUE IntegerLiteral */
void Rule_ALTERSEQUENCEoption_MINVALUE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SECURITY LABEL COMPONENT> ::= ALTER SECURITY LABEL COMPONENT Id ADD ARRAY '[' <SECURITY LABEL COMPONENT_ARRAYs> ']' */
void Rule_ALTERSECURITYLABELCOMPONENT_ALTER_SECURITY_LABEL_COMPONENT_Id_ADD_ARRAY_LBracket_RBracket(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SECURITY LABEL COMPONENT> ::= ALTER SECURITY LABEL COMPONENT Id ADD SET '{' <Id List> '}' */
void Rule_ALTERSECURITYLABELCOMPONENT_ALTER_SECURITY_LABEL_COMPONENT_Id_ADD_SET_LBrace_RBrace(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER SECURITY LABEL COMPONENT> ::= ALTER SECURITY LABEL COMPONENT Id ADD TREE '(' <SECURITY LABEL COMPONENT_TREEs> ')' */
void Rule_ALTERSECURITYLABELCOMPONENT_ALTER_SECURITY_LABEL_COMPONENT_Id_ADD_TREE_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_ARRAYs> ::= <SECURITY LABEL COMPONENT_ARRAY> <SECURITY LABEL COMPONENT_ARRAYs_> */
void Rule_SECURITYLABELCOMPONENT_ARRAYs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_ARRAYs_> ::= ',' <SECURITY LABEL COMPONENT_ARRAY> <SECURITY LABEL COMPONENT_ARRAYs_> */
void Rule_SECURITYLABELCOMPONENT_ARRAYs__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_ARRAYs_> ::=  */
void Rule_SECURITYLABELCOMPONENT_ARRAYs_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_ARRAY> ::= <Id List> BEFORE Id */
void Rule_SECURITYLABELCOMPONENT_ARRAY_BEFORE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_ARRAY> ::= <Id List> AFTER Id */
void Rule_SECURITYLABELCOMPONENT_ARRAY_AFTER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_TREEs> ::= <SECURITY LABEL COMPONENT_TREE> <SECURITY LABEL COMPONENT_TREEs_> */
void Rule_SECURITYLABELCOMPONENT_TREEs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_TREEs_> ::= ',' <SECURITY LABEL COMPONENT_TREE> <SECURITY LABEL COMPONENT_TREEs_> */
void Rule_SECURITYLABELCOMPONENT_TREEs__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_TREEs_> ::=  */
void Rule_SECURITYLABELCOMPONENT_TREEs_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL COMPONENT_TREE> ::= Id UNDER Id */
void Rule_SECURITYLABELCOMPONENT_TREE_Id_UNDER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE> ::= ALTER TABLE Id <ALTER TABLE_basetable> */
void Rule_ALTERTABLE_ALTER_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE> ::= ALTER TABLE Id <ALTER TABLE_LogType> */
void Rule_ALTERTABLE_ALTER_TABLE_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_AddColumn> */
void Rule_ALTERTABLE_basetable(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_AddConstraint> */
void Rule_ALTERTABLE_basetable2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_Modify> */
void Rule_ALTERTABLE_basetable3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_Drop> */
void Rule_ALTERTABLE_basetable4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_DropConstraint> */
void Rule_ALTERTABLE_basetable5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_ModifyNextSize> */
void Rule_ALTERTABLE_basetable6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_LockMode> */
void Rule_ALTERTABLE_basetable7(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_Put> */
void Rule_ALTERTABLE_basetable8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_SecurityPolicy> */
void Rule_ALTERTABLE_basetable9(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= <ALTER TABLE_AddType> */
void Rule_ALTERTABLE_basetable10(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= ADD ROWIDS */
void Rule_ALTERTABLE_basetable_ADD_ROWIDS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= ADD CRCOLS */
void Rule_ALTERTABLE_basetable_ADD_CRCOLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= ADD VERCOLS */
void Rule_ALTERTABLE_basetable_ADD_VERCOLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= DROP ROWIDS */
void Rule_ALTERTABLE_basetable_DROP_ROWIDS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= DROP CRCOLS */
void Rule_ALTERTABLE_basetable_DROP_CRCOLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_basetable> ::= DROP VERCOLS */
void Rule_ALTERTABLE_basetable_DROP_VERCOLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_AddColumn> ::= ADD '(' <ColumnDefineS> ')' */
void Rule_ALTERTABLE_AddColumn_ADD_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_AddColumn> ::= ADD <ColumnDefine> */
void Rule_ALTERTABLE_AddColumn_ADD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefineS> ::= <ColumnDefine> <ColumnDefineS_> */
void Rule_ColumnDefineS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefineS_> ::= ',' <ColumnDefine> <ColumnDefineS_> */
void Rule_ColumnDefineS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefineS_> ::=  */
void Rule_ColumnDefineS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefine> ::= Id <Type> <DEFAULT_clauseOpt> <column_constraintOpt> BEFORE Id <ColumnSecurityOpt> */
void Rule_ColumnDefine_Id_BEFORE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefine> ::= <ALTER TABLE_Modify_0> */
void Rule_ColumnDefine(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT Id */
void Rule_DEFAULT_clauseOpt_DEFAULT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT <constant_expression> */
void Rule_DEFAULT_clauseOpt_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT USER */
void Rule_DEFAULT_clauseOpt_DEFAULT_USER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT CURRENT <DATETIMEopt> */
void Rule_DEFAULT_clauseOpt_DEFAULT_CURRENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT SYSDATE <DATETIMEopt> */
void Rule_DEFAULT_clauseOpt_DEFAULT_SYSDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT TODAY */
void Rule_DEFAULT_clauseOpt_DEFAULT_TODAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT SITENAME */
void Rule_DEFAULT_clauseOpt_DEFAULT_SITENAME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::= DEFAULT DBSERVERNAME */
void Rule_DEFAULT_clauseOpt_DEFAULT_DBSERVERNAME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFAULT_clauseOpt> ::=  */
void Rule_DEFAULT_clauseOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <constant_expression> ::= <CONST And Exp> OR <constant_expression> */
void Rule_constant_expression_OR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <constant_expression> ::= <CONST And Exp> */
void Rule_constant_expression(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST And Exp> ::= <CONST Not Exp> AND <CONST And Exp> */
void Rule_CONSTAndExp_AND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST And Exp> ::= <CONST Not Exp> */
void Rule_CONSTAndExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Not Exp> ::= NOT <CONST Pred Exp> */
void Rule_CONSTNotExp_NOT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Not Exp> ::= <CONST Pred Exp> */
void Rule_CONSTNotExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> BETWEEN <CONST Add Exp> AND <CONST Add Exp> */
void Rule_CONSTPredExp_BETWEEN_AND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> NOT BETWEEN <CONST Add Exp> AND <CONST Add Exp> */
void Rule_CONSTPredExp_NOT_BETWEEN_AND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Value> IS NOT NULL */
void Rule_CONSTPredExp_IS_NOT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Value> IS NULL */
void Rule_CONSTPredExp_IS_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> LIKE StringLiteral */
void Rule_CONSTPredExp_LIKE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> IN <CONST Tuple> */
void Rule_CONSTPredExp_IN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> '=' <CONST Add Exp> */
void Rule_CONSTPredExp_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> '<>' <CONST Add Exp> */
void Rule_CONSTPredExp_LtGt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> '!=' <CONST Add Exp> */
void Rule_CONSTPredExp_ExclamEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> '>' <CONST Add Exp> */
void Rule_CONSTPredExp_Gt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> '>=' <CONST Add Exp> */
void Rule_CONSTPredExp_GtEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> '<' <CONST Add Exp> */
void Rule_CONSTPredExp_Lt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> '<=' <CONST Add Exp> */
void Rule_CONSTPredExp_LtEq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Pred Exp> ::= <CONST Add Exp> */
void Rule_CONSTPredExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Add Exp> ::= <CONST Add Exp> '+' <CONST Mult Exp> */
void Rule_CONSTAddExp_Plus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Add Exp> ::= <CONST Add Exp> '-' <CONST Mult Exp> */
void Rule_CONSTAddExp_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Add Exp> ::= <CONST Mult Exp> */
void Rule_CONSTAddExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Mult Exp> ::= <CONST Mult Exp> '*' <CONST Negate Exp> */
void Rule_CONSTMultExp_Times(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Mult Exp> ::= <CONST Mult Exp> '/' <CONST Negate Exp> */
void Rule_CONSTMultExp_Div(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Mult Exp> ::= <CONST Negate Exp> */
void Rule_CONSTMultExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Negate Exp> ::= '-' <CONST Value> */
void Rule_CONSTNegateExp_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Negate Exp> ::= <CONST Value> */
void Rule_CONSTNegateExp(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Value> ::= <CONST Tuple> */
void Rule_CONSTValue(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Value> ::= <Value_Common> */
void Rule_CONSTValue2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Value> ::= CASE <CASE_WHEN_THENs> ELSE <constant_expression> END */
void Rule_CONSTValue_CASE_ELSE_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Value> ::= CASE <CASE_WHEN_THENs> END */
void Rule_CONSTValue_CASE_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Value> ::= CASE <constant_expression> <CASE_WHEN_THENs> ELSE <constant_expression> END */
void Rule_CONSTValue_CASE_ELSE_END2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Value> ::= CASE <constant_expression> <CASE_WHEN_THENs> END */
void Rule_CONSTValue_CASE_END2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Tuple> ::= '(' <CONST Expr List> ')' */
void Rule_CONSTTuple_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Expr List> ::= <constant_expression> ',' <CONST Expr List> */
void Rule_CONSTExprList_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONST Expr List> ::= <constant_expression> */
void Rule_CONSTExprList(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraintOpt> ::= NOT NULL <column_constraint_CONSTRAINTopt> <column_constraint_S> */
void Rule_column_constraintOpt_NOT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraintOpt> ::= NOT NULL <column_constraint_S> */
void Rule_column_constraintOpt_NOT_NULL2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraintOpt> ::= <column_constraint_S> */
void Rule_column_constraintOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraintOpt> ::=  */
void Rule_column_constraintOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_CONSTRAINTopt> ::= CONSTRAINT Id <IndexMode> */
void Rule_column_constraint_CONSTRAINTopt_CONSTRAINT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_CONSTRAINTopt> ::= CONSTRAINT Id */
void Rule_column_constraint_CONSTRAINTopt_CONSTRAINT_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_CONSTRAINTopt> ::= <IndexMode> */
void Rule_column_constraint_CONSTRAINTopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_CONSTRAINTopt> ::=  */
void Rule_column_constraint_CONSTRAINTopt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_S> ::= <column_constraint_> <column_constraint_CONSTRAINTopt> <column_constraint_S> */
void Rule_column_constraint_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_S> ::= <column_constraint_> <column_constraint_CONSTRAINTopt> */
void Rule_column_constraint_S2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_> ::= UNIQUE */
void Rule_column_constraint__UNIQUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_> ::= DISTINCT */
void Rule_column_constraint__DISTINCT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_> ::= PRIMARY KEY */
void Rule_column_constraint__PRIMARY_KEY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_> ::= <REFERENCES_clause> */
void Rule_column_constraint_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <column_constraint_> ::= <CHECK_clause> */
void Rule_column_constraint_2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REFERENCES_clause> ::= REFERENCES Id '(' <Id List> ')' ON DELETE CASCADE */
void Rule_REFERENCES_clause_REFERENCES_Id_LParan_RParan_ON_DELETE_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REFERENCES_clause> ::= REFERENCES Id '(' <Id List> ')' */
void Rule_REFERENCES_clause_REFERENCES_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REFERENCES_clause> ::= REFERENCES Id ON DELETE CASCADE */
void Rule_REFERENCES_clause_REFERENCES_Id_ON_DELETE_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REFERENCES_clause> ::= REFERENCES Id */
void Rule_REFERENCES_clause_REFERENCES_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CHECK_clause> ::= CHECK '(' <Expression> ')' */
void Rule_CHECK_clause_CHECK_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnSecurityOpt> ::= COLUMN SECURED WITH Id */
void Rule_ColumnSecurityOpt_COLUMN_SECURED_WITH_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnSecurityOpt> ::= SECURED WITH Id */
void Rule_ColumnSecurityOpt_SECURED_WITH_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnSecurityOpt> ::=  */
void Rule_ColumnSecurityOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_AddConstraint> ::= ADD CONSTRAINT '(' <table_constraintS> ')' */
void Rule_ALTERTABLE_AddConstraint_ADD_CONSTRAINT_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_AddConstraint> ::= ADD CONSTRAINT <table_constraint> */
void Rule_ALTERTABLE_AddConstraint_ADD_CONSTRAINT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraintS> ::= <table_constraint> <table_constraintS_> */
void Rule_table_constraintS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraintS_> ::= ',' <table_constraint> <table_constraintS_> */
void Rule_table_constraintS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraintS_> ::=  */
void Rule_table_constraintS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraint> ::= UNIQUE '(' <Id List> ')' <column_constraint_CONSTRAINTopt> */
void Rule_table_constraint_UNIQUE_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraint> ::= DISTINCT '(' <Id List> ')' <column_constraint_CONSTRAINTopt> */
void Rule_table_constraint_DISTINCT_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraint> ::= PRIMARY KEY '(' <Id List> ')' <column_constraint_CONSTRAINTopt> */
void Rule_table_constraint_PRIMARY_KEY_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraint> ::= <CHECK_clause> <column_constraint_CONSTRAINTopt> */
void Rule_table_constraint(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <table_constraint> ::= FOREIGN KEY '(' <Id List> ')' <REFERENCES_clause> <column_constraint_CONSTRAINTopt> */
void Rule_table_constraint_FOREIGN_KEY_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify> ::= MODIFY '(' <ALTER TABLE_Modify_S> ')' */
void Rule_ALTERTABLE_Modify_MODIFY_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify> ::= MODIFY <ALTER TABLE_Modify_> */
void Rule_ALTERTABLE_Modify_MODIFY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify_S> ::= <ALTER TABLE_Modify_> <ALTER TABLE_Modify_S_> */
void Rule_ALTERTABLE_Modify_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify_S_> ::= ',' <ALTER TABLE_Modify_> <ALTER TABLE_Modify_S_> */
void Rule_ALTERTABLE_Modify_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify_S_> ::=  */
void Rule_ALTERTABLE_Modify_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify_> ::= Id <Type> <DEFAULT_clauseOpt> <column_constraintOpt> DROP COLUMN SECURITY */
void Rule_ALTERTABLE_Modify__Id_DROP_COLUMN_SECURITY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify_> ::= <ALTER TABLE_Modify_0> */
void Rule_ALTERTABLE_Modify_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Modify_0> ::= Id <Type> <DEFAULT_clauseOpt> <column_constraintOpt> <ColumnSecurityOpt> */
void Rule_ALTERTABLE_Modify_0_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Drop> ::= DROP '(' <Id List> ')' */
void Rule_ALTERTABLE_Drop_DROP_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Drop> ::= DROP Id */
void Rule_ALTERTABLE_Drop_DROP_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_DropConstraint> ::= DROP CONSTRAINT '(' <Id List> ')' */
void Rule_ALTERTABLE_DropConstraint_DROP_CONSTRAINT_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_DropConstraint> ::= DROP CONSTRAINT Id */
void Rule_ALTERTABLE_DropConstraint_DROP_CONSTRAINT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_ModifyNextSize> ::= MODIFY NEXT SIZE IntegerLiteral */
void Rule_ALTERTABLE_ModifyNextSize_MODIFY_NEXT_SIZE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_LockMode> ::= LOCK MODE '(' PAGE ')' */
void Rule_ALTERTABLE_LockMode_LOCK_MODE_LParan_PAGE_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_LockMode> ::= LOCK MODE '(' ROW ')' */
void Rule_ALTERTABLE_LockMode_LOCK_MODE_LParan_ROW_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_LockMode> ::= LOCK MODE '(' TABLE ')' */
void Rule_ALTERTABLE_LockMode_LOCK_MODE_LParan_TABLE_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Put> ::= PUT Id IN '(' <Id List> ')' '(' <ALTER TABLE_PutOptions> ')' */
void Rule_ALTERTABLE_Put_PUT_Id_IN_LParan_RParan_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_Put> ::= PUT Id IN '(' <Id List> ')' */
void Rule_ALTERTABLE_Put_PUT_Id_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOptions> ::= <ALTER TABLE_PutOption> <ALTER TABLE_PutOptions_> */
void Rule_ALTERTABLE_PutOptions(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOptions_> ::= ',' <ALTER TABLE_PutOption> <ALTER TABLE_PutOptions_> */
void Rule_ALTERTABLE_PutOptions__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOptions_> ::=  */
void Rule_ALTERTABLE_PutOptions_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOption> ::= EXTENT SIZE IntegerLiteral */
void Rule_ALTERTABLE_PutOption_EXTENT_SIZE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOption> ::= NO LOG */
void Rule_ALTERTABLE_PutOption_NO_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOption> ::= LOG */
void Rule_ALTERTABLE_PutOption_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOption> ::= HIGH INTEG */
void Rule_ALTERTABLE_PutOption_HIGH_INTEG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOption> ::= NO KEEP ACCESS TIME */
void Rule_ALTERTABLE_PutOption_NO_KEEP_ACCESS_TIME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_PutOption> ::= KEEP ACCESS TIME */
void Rule_ALTERTABLE_PutOption_KEEP_ACCESS_TIME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_SecurityPolicy> ::= ADD SECURITY POLICY Id */
void Rule_ALTERTABLE_SecurityPolicy_ADD_SECURITY_POLICY_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_SecurityPolicy> ::= DROP SECURITY POLICY */
void Rule_ALTERTABLE_SecurityPolicy_DROP_SECURITY_POLICY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_AddType> ::= ADD TYPE Id */
void Rule_ALTERTABLE_AddType_ADD_TYPE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_LogType> ::= TYPE '(' STANDARD ')' */
void Rule_ALTERTABLE_LogType_TYPE_LParan_STANDARD_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_LogType> ::= TYPE '(' RAW ')' */
void Rule_ALTERTABLE_LogType_TYPE_LParan_RAW_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_LogType> ::= TYPE '(' OPERATIONAL ')' */
void Rule_ALTERTABLE_LogType_TYPE_LParan_OPERATIONAL_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ALTER TABLE_LogType> ::= TYPE '(' STATIC ')' */
void Rule_ALTERTABLE_LogType_TYPE_LParan_STATIC_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CLOSE DATABASE> ::= CLOSE DATABASE */
void Rule_CLOSEDATABASE_CLOSE_DATABASE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ACCESS_METHOD> ::= CREATE PRIMARY 'ACCESS_METHOD' Id <PurposeOptionS> */
void Rule_CREATEACCESS_METHOD_CREATE_PRIMARY_ACCESS_METHOD_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ACCESS_METHOD> ::= CREATE SECONDARY 'ACCESS_METHOD' Id <PurposeOptionS> */
void Rule_CREATEACCESS_METHOD_CREATE_SECONDARY_ACCESS_METHOD_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOptionS> ::= <PurposeOption> <PurposeOptionS_> */
void Rule_PurposeOptionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOptionS_> ::= ',' <PurposeOption> <PurposeOptionS_> */
void Rule_PurposeOptionS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOptionS_> ::=  */
void Rule_PurposeOptionS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE> ::= CREATE AGGREGATE Id WITH '(' <CREATE AGGREGATE_OptionS> ')' */
void Rule_CREATEAGGREGATE_CREATE_AGGREGATE_Id_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_OptionS> ::= <CREATE AGGREGATE_Option> <CREATE AGGREGATE_OptionS_> */
void Rule_CREATEAGGREGATE_OptionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_OptionS_> ::= ',' <CREATE AGGREGATE_Option> <CREATE AGGREGATE_OptionS_> */
void Rule_CREATEAGGREGATE_OptionS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_OptionS_> ::=  */
void Rule_CREATEAGGREGATE_OptionS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_Option> ::= INIT '=' Id */
void Rule_CREATEAGGREGATE_Option_INIT_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_Option> ::= ITER '=' Id */
void Rule_CREATEAGGREGATE_Option_ITER_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_Option> ::= COMBINE '=' Id */
void Rule_CREATEAGGREGATE_Option_COMBINE_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_Option> ::= FINAL '=' Id */
void Rule_CREATEAGGREGATE_Option_FINAL_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE AGGREGATE_Option> ::= HANDLESNULLS */
void Rule_CREATEAGGREGATE_Option_HANDLESNULLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE CAST> ::= CREATE EXPLICIT CAST '(' Id AS Id WITH Id ')' */
void Rule_CREATECAST_CREATE_EXPLICIT_CAST_LParan_Id_AS_Id_WITH_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE CAST> ::= CREATE EXPLICIT CAST '(' Id AS Id ')' */
void Rule_CREATECAST_CREATE_EXPLICIT_CAST_LParan_Id_AS_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE CAST> ::= CREATE IMPLICIT CAST '(' Id AS Id WITH Id ')' */
void Rule_CREATECAST_CREATE_IMPLICIT_CAST_LParan_Id_AS_Id_WITH_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE CAST> ::= CREATE IMPLICIT CAST '(' Id AS Id ')' */
void Rule_CREATECAST_CREATE_IMPLICIT_CAST_LParan_Id_AS_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DATABASE> ::= CREATE DATABASE Id IN Id <CREATE DATABASE_WITHopt> */
void Rule_CREATEDATABASE_CREATE_DATABASE_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DATABASE> ::= CREATE DATABASE Id <CREATE DATABASE_WITHopt> */
void Rule_CREATEDATABASE_CREATE_DATABASE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DATABASE_WITHopt> ::= WITH BUFFER LOG */
void Rule_CREATEDATABASE_WITHopt_WITH_BUFFER_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DATABASE_WITHopt> ::= WITH LOG */
void Rule_CREATEDATABASE_WITHopt_WITH_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DATABASE_WITHopt> ::= WITH LOG MODE ANSI */
void Rule_CREATEDATABASE_WITHopt_WITH_LOG_MODE_ANSI(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DATABASE_WITHopt> ::=  */
void Rule_CREATEDATABASE_WITHopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DISTINCT TYPE> ::= CREATE DISTINCT TYPE Id AS Id */
void Rule_CREATEDISTINCTTYPE_CREATE_DISTINCT_TYPE_Id_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DUPLICATE> ::= CREATE DUPLICATE OF TABLE Id IN '(' <Id List> ')' */
void Rule_CREATEDUPLICATE_CREATE_DUPLICATE_OF_TABLE_Id_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE DUPLICATE> ::= CREATE DUPLICATE OF TABLE Id IN Id */
void Rule_CREATEDUPLICATE_CREATE_DUPLICATE_OF_TABLE_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE> ::= CREATE EXTERNAL TABLE Id <CREATE EXTERNAL TABLE_fields> USING '(' <CREATE EXTERNAL TABLE_optionS> DATAFILES <StringLiteralS> <CREATE EXTERNAL TABLE_optionS> ')' */
void Rule_CREATEEXTERNALTABLE_CREATE_EXTERNAL_TABLE_Id_USING_LParan_DATAFILES_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields> ::= SAMEAS Id */
void Rule_CREATEEXTERNALTABLE_fields_SAMEAS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields> ::= <CREATE EXTERNAL TABLE_fields_S> */
void Rule_CREATEEXTERNALTABLE_fields(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_S> ::= <CREATE EXTERNAL TABLE_fields_> <CREATE EXTERNAL TABLE_fields_S_> */
void Rule_CREATEEXTERNALTABLE_fields_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_S_> ::= ',' <CREATE EXTERNAL TABLE_fields_> <CREATE EXTERNAL TABLE_fields_S_> */
void Rule_CREATEEXTERNALTABLE_fields_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_S_> ::=  */
void Rule_CREATEEXTERNALTABLE_fields_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_> ::= Id <Type> <CREATE EXTERNAL TABLE_fields_Opt1> */
void Rule_CREATEEXTERNALTABLE_fields__Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_Opt1> ::= EXTERNAL StringLiteral <DEFAULT_clauseOpt> <CREATE EXTERNAL TABLE_fields_Opt2> */
void Rule_CREATEEXTERNALTABLE_fields_Opt1_EXTERNAL_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_Opt1> ::= EXTERNAL <Type> NULL StringLiteral <DEFAULT_clauseOpt> <CREATE EXTERNAL TABLE_fields_Opt2> */
void Rule_CREATEEXTERNALTABLE_fields_Opt1_EXTERNAL_NULL_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_Opt1> ::= <DEFAULT_clauseOpt> <CREATE EXTERNAL TABLE_fields_Opt2> */
void Rule_CREATEEXTERNALTABLE_fields_Opt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_Opt2> ::= NOT NULL CHECK '(' <Expression> ')' */
void Rule_CREATEEXTERNALTABLE_fields_Opt2_NOT_NULL_CHECK_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_Opt2> ::= NOT NULL */
void Rule_CREATEEXTERNALTABLE_fields_Opt2_NOT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_Opt2> ::= CHECK '(' <Expression> ')' */
void Rule_CREATEEXTERNALTABLE_fields_Opt2_CHECK_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_fields_Opt2> ::=  */
void Rule_CREATEEXTERNALTABLE_fields_Opt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_optionS> ::= <CREATE EXTERNAL TABLE_option> <CREATE EXTERNAL TABLE_optionS_> */
void Rule_CREATEEXTERNALTABLE_optionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_optionS> ::=  */
void Rule_CREATEEXTERNALTABLE_optionS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_optionS_> ::= ',' <CREATE EXTERNAL TABLE_option> <CREATE EXTERNAL TABLE_optionS_> */
void Rule_CREATEEXTERNALTABLE_optionS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_optionS_> ::=  */
void Rule_CREATEEXTERNALTABLE_optionS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_option> ::= DEFAULT */
void Rule_CREATEEXTERNALTABLE_option_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_option> ::= EXPRESS */
void Rule_CREATEEXTERNALTABLE_option_EXPRESS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_option> ::= DELUXE */
void Rule_CREATEEXTERNALTABLE_option_DELUXE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_option> ::= MAXERRORS IntegerLiteral */
void Rule_CREATEEXTERNALTABLE_option_MAXERRORS_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_option> ::= REJECTFILE StringLiteral */
void Rule_CREATEEXTERNALTABLE_option_REJECTFILE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_option> ::= SIZE IntegerLiteral */
void Rule_CREATEEXTERNALTABLE_option_SIZE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE EXTERNAL TABLE_option> ::= <SELECT TABLE_option> */
void Rule_CREATEEXTERNALTABLE_option(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <StringLiteralS> ::= StringLiteral <StringLiteralS_> */
void Rule_StringLiteralS_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <StringLiteralS_> ::= ',' StringLiteral <StringLiteralS_> */
void Rule_StringLiteralS__Comma_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <StringLiteralS_> ::=  */
void Rule_StringLiteralS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DBAopt> ::= DBA */
void Rule_DBAopt_DBA(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DBAopt> ::=  */
void Rule_DBAopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParams> ::= <RoutineParam> <RoutineParams_> */
void Rule_RoutineParams(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParams_> ::= ',' <RoutineParam> <RoutineParams_> */
void Rule_RoutineParams__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParams_> ::=  */
void Rule_RoutineParams_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id <Type> DEFAULT <Expression> */
void Rule_RoutineParam_Id_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id <Type> */
void Rule_RoutineParam_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id LIKE Id DEFAULT <Expression> */
void Rule_RoutineParam_Id_LIKE_Id_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id LIKE Id */
void Rule_RoutineParam_Id_LIKE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id REFERENCES BYTE DEFAULT NULL */
void Rule_RoutineParam_Id_REFERENCES_BYTE_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id REFERENCES BYTE */
void Rule_RoutineParam_Id_REFERENCES_BYTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id REFERENCES TEXT DEFAULT NULL */
void Rule_RoutineParam_Id_REFERENCES_TEXT_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= Id REFERENCES TEXT */
void Rule_RoutineParam_Id_REFERENCES_TEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id <Type> DEFAULT <Expression> */
void Rule_RoutineParam_OUT_Id_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id <Type> */
void Rule_RoutineParam_OUT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id LIKE Id DEFAULT <Expression> */
void Rule_RoutineParam_OUT_Id_LIKE_Id_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id LIKE Id */
void Rule_RoutineParam_OUT_Id_LIKE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id REFERENCES BYTE DEFAULT NULL */
void Rule_RoutineParam_OUT_Id_REFERENCES_BYTE_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id REFERENCES BYTE */
void Rule_RoutineParam_OUT_Id_REFERENCES_BYTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id REFERENCES TEXT DEFAULT NULL */
void Rule_RoutineParam_OUT_Id_REFERENCES_TEXT_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= OUT Id REFERENCES TEXT */
void Rule_RoutineParam_OUT_Id_REFERENCES_TEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id <Type> DEFAULT <Expression> */
void Rule_RoutineParam_INOUT_Id_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id <Type> */
void Rule_RoutineParam_INOUT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id LIKE Id DEFAULT <Expression> */
void Rule_RoutineParam_INOUT_Id_LIKE_Id_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id LIKE Id */
void Rule_RoutineParam_INOUT_Id_LIKE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id REFERENCES BYTE DEFAULT NULL */
void Rule_RoutineParam_INOUT_Id_REFERENCES_BYTE_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id REFERENCES BYTE */
void Rule_RoutineParam_INOUT_Id_REFERENCES_BYTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id REFERENCES TEXT DEFAULT NULL */
void Rule_RoutineParam_INOUT_Id_REFERENCES_TEXT_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam> ::= INOUT Id REFERENCES TEXT */
void Rule_RoutineParam_INOUT_Id_REFERENCES_TEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_referencingOpt> ::= <referencing_clause> FOR Id */
void Rule_CREATEFUNCTION_referencingOpt_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_referencingOpt> ::=  */
void Rule_CREATEFUNCTION_referencingOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause> ::= <referencing_clause_DELETE> */
void Rule_referencing_clause(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause> ::= <referencing_clause_INSERT> */
void Rule_referencing_clause2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause> ::= <referencing_clause_UPDATE> */
void Rule_referencing_clause3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_DELETE> ::= REFERENCING OLD AS Id */
void Rule_referencing_clause_DELETE_REFERENCING_OLD_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_DELETE> ::= REFERENCING OLD Id */
void Rule_referencing_clause_DELETE_REFERENCING_OLD_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_INSERT> ::= REFERENCING NEW AS Id */
void Rule_referencing_clause_INSERT_REFERENCING_NEW_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_INSERT> ::= REFERENCING NEW Id */
void Rule_referencing_clause_INSERT_REFERENCING_NEW_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_UPDATE> ::= REFERENCING OLD AS Id NEW AS Id */
void Rule_referencing_clause_UPDATE_REFERENCING_OLD_AS_Id_NEW_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_UPDATE> ::= REFERENCING OLD Id NEW AS Id */
void Rule_referencing_clause_UPDATE_REFERENCING_OLD_Id_NEW_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_UPDATE> ::= REFERENCING OLD AS Id NEW Id */
void Rule_referencing_clause_UPDATE_REFERENCING_OLD_AS_Id_NEW_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <referencing_clause_UPDATE> ::= REFERENCING OLD Id NEW Id */
void Rule_referencing_clause_UPDATE_REFERENCING_OLD_Id_NEW_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return> ::= RETURNING <CREATE FUNCTION_return_S> <SemicolonOpt> */
void Rule_CREATEFUNCTION_return_RETURNING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return> ::= RETURNS <CREATE FUNCTION_return_S> <SemicolonOpt> */
void Rule_CREATEFUNCTION_return_RETURNS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_S> ::= <CREATE FUNCTION_return_> <CREATE FUNCTION_return_S_> */
void Rule_CREATEFUNCTION_return_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_S_> ::= ',' <CREATE FUNCTION_return_> <CREATE FUNCTION_return_S_> */
void Rule_CREATEFUNCTION_return_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_S_> ::=  */
void Rule_CREATEFUNCTION_return_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_> ::= <Type> AS Id */
void Rule_CREATEFUNCTION_return__AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_> ::= <Type> */
void Rule_CREATEFUNCTION_return_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_> ::= REFERENCES BYTE AS Id */
void Rule_CREATEFUNCTION_return__REFERENCES_BYTE_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_> ::= REFERENCES TEXT AS Id */
void Rule_CREATEFUNCTION_return__REFERENCES_TEXT_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_> ::= REFERENCES BYTE */
void Rule_CREATEFUNCTION_return__REFERENCES_BYTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_return_> ::= REFERENCES TEXT */
void Rule_CREATEFUNCTION_return__REFERENCES_TEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_specificOpt> ::= SPECIFIC Id */
void Rule_CREATEFUNCTION_specificOpt_SPECIFIC_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_specificOpt> ::=  */
void Rule_CREATEFUNCTION_specificOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_withOpt> ::= WITH '(' <CREATE FUNCTION_withS> ')' */
void Rule_CREATEFUNCTION_withOpt_WITH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_withOpt> ::=  */
void Rule_CREATEFUNCTION_withOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_withS> ::= <SPLdescriptor1> <CREATE FUNCTION_withS_> */
void Rule_CREATEFUNCTION_withS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_withS_> ::= ',' <SPLdescriptor1> <CREATE FUNCTION_withS_> */
void Rule_CREATEFUNCTION_withS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_withS_> ::=  */
void Rule_CREATEFUNCTION_withS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLBlock> ::= <DEFINEs> <ON EXCEPTIONs> BEGIN <SQLs> END */
void Rule_SQLBlock_BEGIN_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLBlock> ::= <DEFINEs> <ON EXCEPTIONs> <SQLs_> */
void Rule_SQLBlock(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINEs> ::= <DEFINE> <DEFINEs> */
void Rule_DEFINEs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINEs> ::=  */
void Rule_DEFINEs2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ON EXCEPTIONs> ::= <ON EXCEPTION> <ON EXCEPTIONs> */
void Rule_ONEXCEPTIONs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ON EXCEPTIONs> ::=  */
void Rule_ONEXCEPTIONs2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_documentOpt> ::= DOCUMENT <StringLiteralS> */
void Rule_CREATEFUNCTION_documentOpt_DOCUMENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_documentOpt> ::=  */
void Rule_CREATEFUNCTION_documentOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_listingOpt> ::= WITH LISTING IN StringLiteral */
void Rule_CREATEFUNCTION_listingOpt_WITH_LISTING_IN_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE FUNCTION_listingOpt> ::=  */
void Rule_CREATEFUNCTION_listingOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutine> ::= EXTERNAL NAME StringLiteral LANGUAGE C <ExternalRoutineOpt1> <ExternalRoutineOpt2> */
void Rule_ExternalRoutine_EXTERNAL_NAME_StringLiteral_LANGUAGE_C(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutine> ::= EXTERNAL NAME StringLiteral LANGUAGE JAVA <ExternalRoutineOpt1> <ExternalRoutineOpt2> */
void Rule_ExternalRoutine_EXTERNAL_NAME_StringLiteral_LANGUAGE_JAVA(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutineOpt1> ::= PARAMETER STYLE INFORMIX */
void Rule_ExternalRoutineOpt1_PARAMETER_STYLE_INFORMIX(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutineOpt1> ::= PARAMETER STYLE */
void Rule_ExternalRoutineOpt1_PARAMETER_STYLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutineOpt1> ::=  */
void Rule_ExternalRoutineOpt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutineOpt2> ::= NOT VARIANT */
void Rule_ExternalRoutineOpt2_NOT_VARIANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutineOpt2> ::= VARIANT */
void Rule_ExternalRoutineOpt2_VARIANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ExternalRoutineOpt2> ::=  */
void Rule_ExternalRoutineOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX> ::= CREATE <IndexType> <CREATE INDEX_indexScope> '(' <CREATE INDEX_indexKeyS> ')' <CREATE INDEX_Option> <IndexLockmodeOpt> <CREATE INDEX_onlineOpt> */
void Rule_CREATEINDEX_CREATE_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX> ::= CREATE GK <CREATE INDEX_indexScope> '(' <GK SELECT> ')' USING BITMAP <IndexLockmodeOpt> <CREATE INDEX_onlineOpt> */
void Rule_CREATEINDEX_CREATE_GK_LParan_RParan_USING_BITMAP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX> ::= CREATE GK <CREATE INDEX_indexScope> '(' <GK SELECT> ')' <IndexLockmodeOpt> <CREATE INDEX_onlineOpt> */
void Rule_CREATEINDEX_CREATE_GK_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexType> ::= DISTINCT CLUSTER */
void Rule_IndexType_DISTINCT_CLUSTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexType> ::= DISTINCT */
void Rule_IndexType_DISTINCT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexType> ::= UNIQUE CLUSTER */
void Rule_IndexType_UNIQUE_CLUSTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexType> ::= UNIQUE */
void Rule_IndexType_UNIQUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexType> ::= CLUSTER */
void Rule_IndexType_CLUSTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexType> ::=  */
void Rule_IndexType(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexScope> ::= INDEX Id ON Id */
void Rule_CREATEINDEX_indexScope_INDEX_Id_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexKeyS> ::= <CREATE INDEX_indexKey> <CREATE INDEX_indexKeyS_> */
void Rule_CREATEINDEX_indexKeyS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexKeyS_> ::= ',' <CREATE INDEX_indexKey> <CREATE INDEX_indexKeyS_> */
void Rule_CREATEINDEX_indexKeyS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexKeyS_> ::=  */
void Rule_CREATEINDEX_indexKeyS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexKey> ::= Id Id <ASC_DESCopt> */
void Rule_CREATEINDEX_indexKey_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexKey> ::= Id <ASC_DESCopt> */
void Rule_CREATEINDEX_indexKey_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexKey> ::= Id '(' <Id List> ')' Id <ASC_DESCopt> */
void Rule_CREATEINDEX_indexKey_Id_LParan_RParan_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_indexKey> ::= Id '(' <Id List> ')' <ASC_DESCopt> */
void Rule_CREATEINDEX_indexKey_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ASC_DESCopt> ::= ASC */
void Rule_ASC_DESCopt_ASC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ASC_DESCopt> ::= DESC */
void Rule_ASC_DESCopt_DESC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ASC_DESCopt> ::=  */
void Rule_ASC_DESCopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_Option> ::= <Access-Method_clauseOpt> <FILLFACTORopt> <CREATE INDEX_StorageOpt> <IndexModeOpt> */
void Rule_CREATEINDEX_Option(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Access-Method_clauseOpt> ::= USING Id '(' <RoutineParams2> ')' */
void Rule_AccessMethod_clauseOpt_USING_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Access-Method_clauseOpt> ::=  */
void Rule_AccessMethod_clauseOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FILLFACTORopt> ::= FILLFACTOR IntegerLiteral */
void Rule_FILLFACTORopt_FILLFACTOR_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FILLFACTORopt> ::=  */
void Rule_FILLFACTORopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_StorageOpt0> ::= IN Id */
void Rule_CREATEINDEX_StorageOpt0_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_StorageOpt> ::= <CREATE INDEX_StorageOpt0> */
void Rule_CREATEINDEX_StorageOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_StorageOpt> ::= IN TABLE */
void Rule_CREATEINDEX_StorageOpt_IN_TABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_StorageOpt> ::= <FRAGMENT BY_INDEX> */
void Rule_CREATEINDEX_StorageOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_StorageOpt> ::= USING BITMAP */
void Rule_CREATEINDEX_StorageOpt_USING_BITMAP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_StorageOpt> ::=  */
void Rule_CREATEINDEX_StorageOpt3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> EXPRESSION '(' <FRAGMENT BY_Expr List> ',' <FRAGMENT BY_REMAINDER> ')' */
void Rule_FRAGMENTBY_INDEX_EXPRESSION_LParan_Comma_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> HASH '(' <Id List> ')' IN Id */
void Rule_FRAGMENTBY_INDEX_HASH_LParan_RParan_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> HASH '(' <Id List> ')' IN '(' <Id List> ')' */
void Rule_FRAGMENTBY_INDEX_HASH_LParan_RParan_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> HYBRID '(' <Id List> ')' <FRAGMENT BY_EXPRESSION> */
void Rule_FRAGMENTBY_INDEX_HYBRID_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_INDEXopt1> ::= FRAGMENT BY */
void Rule_FRAGMENTBY_INDEXopt1_FRAGMENT_BY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_INDEXopt1> ::= PARTITION BY */
void Rule_FRAGMENTBY_INDEXopt1_PARTITION_BY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr List> ::= <FRAGMENT BY_Expr> <FRAGMENT BY_Expr List_> */
void Rule_FRAGMENTBY_ExprList(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr List_> ::= ',' <FRAGMENT BY_Expr> <FRAGMENT BY_Expr List_> */
void Rule_FRAGMENTBY_ExprList__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr List_> ::=  */
void Rule_FRAGMENTBY_ExprList_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr> ::= PARTITION Id '(' <Expression> ')' IN Id */
void Rule_FRAGMENTBY_Expr_PARTITION_Id_LParan_RParan_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_Expr> ::= '(' <Expression> ')' IN Id */
void Rule_FRAGMENTBY_Expr_LParan_RParan_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_REMAINDER> ::= PARTITION Id REMAINDER IN Id */
void Rule_FRAGMENTBY_REMAINDER_PARTITION_Id_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_REMAINDER> ::= REMAINDER IN Id */
void Rule_FRAGMENTBY_REMAINDER_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_REMAINDER> ::= <FRAGMENT BY_Expr> */
void Rule_FRAGMENTBY_REMAINDER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION> ::= EXPRESSION <FRAGMENT BY_EXPRESSION1s> ',' <FRAGMENT BY_EXPRESSION2> */
void Rule_FRAGMENTBY_EXPRESSION_EXPRESSION_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION1s> ::= <FRAGMENT BY_EXPRESSION1> <FRAGMENT BY_EXPRESSION1s_> */
void Rule_FRAGMENTBY_EXPRESSION1s(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION1s_> ::= ',' <FRAGMENT BY_EXPRESSION1> <FRAGMENT BY_EXPRESSION1s_> */
void Rule_FRAGMENTBY_EXPRESSION1s__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION1s_> ::=  */
void Rule_FRAGMENTBY_EXPRESSION1s_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION1> ::= <Expression> IN Id */
void Rule_FRAGMENTBY_EXPRESSION1_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION1> ::= <Expression> IN '(' <Id List> ')' */
void Rule_FRAGMENTBY_EXPRESSION1_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION2> ::= <FRAGMENT BY_EXPRESSION1> */
void Rule_FRAGMENTBY_EXPRESSION2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION2> ::= REMAINDER IN '(' <Id List> ')' */
void Rule_FRAGMENTBY_EXPRESSION2_REMAINDER_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_EXPRESSION2> ::= REMAINDER IN Id */
void Rule_FRAGMENTBY_EXPRESSION2_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexModeOpt> ::= <IndexMode> */
void Rule_IndexModeOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexModeOpt> ::=  */
void Rule_IndexModeOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexMode> ::= ENABLED */
void Rule_IndexMode_ENABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexMode> ::= DISABLED */
void Rule_IndexMode_DISABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexMode> ::= FILTERING WITHOUT ERROR */
void Rule_IndexMode_FILTERING_WITHOUT_ERROR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexMode> ::= FILTERING WITH ERROR */
void Rule_IndexMode_FILTERING_WITH_ERROR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexLockmodeOpt> ::= <IndexLockmode> */
void Rule_IndexLockmodeOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexLockmodeOpt> ::=  */
void Rule_IndexLockmodeOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexLockmode> ::= LOCK MODE NORMAL */
void Rule_IndexLockmode_LOCK_MODE_NORMAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IndexLockmode> ::= LOCK MODE COARSE */
void Rule_IndexLockmode_LOCK_MODE_COARSE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_onlineOpt> ::= ONLINE */
void Rule_CREATEINDEX_onlineOpt_ONLINE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE INDEX_onlineOpt> ::=  */
void Rule_CREATEINDEX_onlineOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT ALL <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
void Rule_GKSELECT_SELECT_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT DISTINCT <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
void Rule_GKSELECT_SELECT_DISTINCT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT UNIQUE <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
void Rule_GKSELECT_SELECT_UNIQUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
void Rule_GKSELECT_SELECT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT ALL <Expr List> <GK SELECT_FROM> */
void Rule_GKSELECT_SELECT_ALL2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT DISTINCT <Expr List> <GK SELECT_FROM> */
void Rule_GKSELECT_SELECT_DISTINCT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT UNIQUE <Expr List> <GK SELECT_FROM> */
void Rule_GKSELECT_SELECT_UNIQUE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT> ::= SELECT <Expr List> <GK SELECT_FROM> */
void Rule_GKSELECT_SELECT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT_FROM> ::= FROM <Id List> */
void Rule_GKSELECT_FROM_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GK SELECT_WHERE> ::= WHERE <Expression> */
void Rule_GKSELECT_WHERE_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE> ::= CREATE OPAQUE TYPE Id '(' INTERNALLENGTH '=' IntegerLiteral <CREATE OPAQUE TYPE_optionS> ')' */
void Rule_CREATEOPAQUETYPE_CREATE_OPAQUE_TYPE_Id_LParan_INTERNALLENGTH_Eq_IntegerLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE> ::= CREATE OPAQUE TYPE Id '(' INTERNALLENGTH '=' VARIABLE <CREATE OPAQUE TYPE_optionS> ')' */
void Rule_CREATEOPAQUETYPE_CREATE_OPAQUE_TYPE_Id_LParan_INTERNALLENGTH_Eq_VARIABLE_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE_optionS> ::= ',' <CREATE OPAQUE TYPE_option> <CREATE OPAQUE TYPE_optionS> */
void Rule_CREATEOPAQUETYPE_optionS_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE_optionS> ::=  */
void Rule_CREATEOPAQUETYPE_optionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE_option> ::= MAXLEN '=' IntegerLiteral */
void Rule_CREATEOPAQUETYPE_option_MAXLEN_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE_option> ::= CANNOTHASH */
void Rule_CREATEOPAQUETYPE_option_CANNOTHASH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE_option> ::= PASSEDBYVALUE */
void Rule_CREATEOPAQUETYPE_option_PASSEDBYVALUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPAQUE TYPE_option> ::= ALIGNMENT '=' IntegerLiteral */
void Rule_CREATEOPAQUETYPE_option_ALIGNMENT_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPCLASS> ::= CREATE OPCLASS Id FOR Id STRATEGIES '(' <CREATE OPCLASS_strategieS> ')' SUPPORT '(' <Id List> ')' */
void Rule_CREATEOPCLASS_CREATE_OPCLASS_Id_FOR_Id_STRATEGIES_LParan_RParan_SUPPORT_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPCLASS_strategieS> ::= <CREATE OPCLASS_strategy> <CREATE OPCLASS_strategieS_> */
void Rule_CREATEOPCLASS_strategieS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPCLASS_strategieS_> ::= ',' <CREATE OPCLASS_strategy> <CREATE OPCLASS_strategieS_> */
void Rule_CREATEOPCLASS_strategieS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPCLASS_strategieS_> ::=  */
void Rule_CREATEOPCLASS_strategieS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPCLASS_strategy> ::= Id '(' <Type> ',' <Type> ',' <Type> ')' */
void Rule_CREATEOPCLASS_strategy_Id_LParan_Comma_Comma_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPCLASS_strategy> ::= Id '(' <Type> ',' <Type> ')' */
void Rule_CREATEOPCLASS_strategy_Id_LParan_Comma_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE OPCLASS_strategy> ::= Id */
void Rule_CREATEOPCLASS_strategy_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
void Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE PROCEDURE_returnOpt> ::= <CREATE FUNCTION_return> */
void Rule_CREATEPROCEDURE_returnOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE PROCEDURE_returnOpt> ::=  */
void Rule_CREATEPROCEDURE_returnOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROLE> ::= CREATE ROLE Id */
void Rule_CREATEROLE_CREATE_ROLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROLE> ::= CREATE ROLE StringLiteral */
void Rule_CREATEROLE_CREATE_ROLE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROUTINE FROM> ::= CREATE ROUTINE FROM StringLiteral */
void Rule_CREATEROUTINEFROM_CREATE_ROUTINE_FROM_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROUTINE FROM> ::= CREATE ROUTINE FROM Id */
void Rule_CREATEROUTINEFROM_CREATE_ROUTINE_FROM_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROW TYPE> ::= CREATE ROW TYPE Id <CREATE ROW TYPE_field> UNDER Id */
void Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id_UNDER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROW TYPE> ::= CREATE ROW TYPE Id <CREATE ROW TYPE_field> */
void Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROW TYPE> ::= CREATE ROW TYPE Id UNDER Id */
void Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id_UNDER_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROW TYPE> ::= CREATE ROW TYPE Id */
void Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROW TYPE_field> ::= Id <Type> NOT NULL */
void Rule_CREATEROWTYPE_field_Id_NOT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE ROW TYPE_field> ::= Id <Type> */
void Rule_CREATEROWTYPE_field_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA> ::= CREATE SCHEMA AUTHORIZATION Id <CREATE SCHEMA_statementS> */
void Rule_CREATESCHEMA_CREATE_SCHEMA_AUTHORIZATION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statementS> ::= <CREATE SCHEMA_statement> <CREATE SCHEMA_statementS> */
void Rule_CREATESCHEMA_statementS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statementS> ::= <CREATE SCHEMA_statement> */
void Rule_CREATESCHEMA_statementS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE TABLE> */
void Rule_CREATESCHEMA_statement(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE VIEW> */
void Rule_CREATESCHEMA_statement2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <GRANT> */
void Rule_CREATESCHEMA_statement3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE INDEX> */
void Rule_CREATESCHEMA_statement4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE SYNONYM> */
void Rule_CREATESCHEMA_statement5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE TRIGGER> */
void Rule_CREATESCHEMA_statement6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE SEQUENCE> */
void Rule_CREATESCHEMA_statement7(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE ROW TYPE> */
void Rule_CREATESCHEMA_statement8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE OPAQUE TYPE> */
void Rule_CREATESCHEMA_statement9(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE DISTINCT TYPE> */
void Rule_CREATESCHEMA_statement10(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCHEMA_statement> ::= <CREATE CAST> */
void Rule_CREATESCHEMA_statement11(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE> ::= CREATE SCRATCH TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE SCRATCH TABLE_opt> */
void Rule_CREATESCRATCHTABLE_CREATE_SCRATCH_TABLE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefine_table_constraintS> ::= <ALTER TABLE_Modify_0> <ColumnDefine_table_constraintS_> */
void Rule_ColumnDefine_table_constraintS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefine_table_constraintS> ::= <table_constraint> <ColumnDefine_table_constraintS_> */
void Rule_ColumnDefine_table_constraintS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefine_table_constraintS_> ::= ',' <ALTER TABLE_Modify_0> <ColumnDefine_table_constraintS_> */
void Rule_ColumnDefine_table_constraintS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefine_table_constraintS_> ::= ',' <table_constraint> <ColumnDefine_table_constraintS_> */
void Rule_ColumnDefine_table_constraintS__Comma2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ColumnDefine_table_constraintS_> ::=  */
void Rule_ColumnDefine_table_constraintS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_opt> ::= IN Id <CREATE SCRATCH TABLE_LockMode> */
void Rule_CREATESCRATCHTABLE_opt_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_opt> ::= <FRAGMENT BY_TABLE> <CREATE SCRATCH TABLE_LockMode> */
void Rule_CREATESCRATCHTABLE_opt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_opt> ::= IN Id */
void Rule_CREATESCRATCHTABLE_opt_IN_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_opt> ::= <FRAGMENT BY_TABLE> */
void Rule_CREATESCRATCHTABLE_opt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_opt> ::=  */
void Rule_CREATESCRATCHTABLE_opt3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_LockMode> ::= LOCK MODE PAGE */
void Rule_CREATESCRATCHTABLE_LockMode_LOCK_MODE_PAGE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_LockMode> ::= LOCK MODE ROW */
void Rule_CREATESCRATCHTABLE_LockMode_LOCK_MODE_ROW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SCRATCH TABLE_LockMode> ::= LOCK MODE TABLE */
void Rule_CREATESCRATCHTABLE_LockMode_LOCK_MODE_TABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL> ::= CREATE SECURITY LABEL Id <CREATE SECURITY LABEL_componentS> */
void Rule_CREATESECURITYLABEL_CREATE_SECURITY_LABEL_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL_componentS> ::= <CREATE SECURITY LABEL_component> <CREATE SECURITY LABEL_componentS_> */
void Rule_CREATESECURITYLABEL_componentS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL_componentS_> ::= ',' <CREATE SECURITY LABEL_component> <CREATE SECURITY LABEL_componentS_> */
void Rule_CREATESECURITYLABEL_componentS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL_componentS_> ::=  */
void Rule_CREATESECURITYLABEL_componentS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL_component> ::= COMPONENT Id <Id List> */
void Rule_CREATESECURITYLABEL_component_COMPONENT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL COMPONENT> ::= CREATE SECURITY LABEL COMPONENT Id ARRAY '[' <SECURITY LABEL COMPONENT_ARRAYs> ']' */
void Rule_CREATESECURITYLABELCOMPONENT_CREATE_SECURITY_LABEL_COMPONENT_Id_ARRAY_LBracket_RBracket(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL COMPONENT> ::= CREATE SECURITY LABEL COMPONENT Id SET '{' <Id List> '}' */
void Rule_CREATESECURITYLABELCOMPONENT_CREATE_SECURITY_LABEL_COMPONENT_Id_SET_LBrace_RBrace(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY LABEL COMPONENT> ::= CREATE SECURITY LABEL COMPONENT Id TREE '(' <SECURITY LABEL COMPONENT_TREEs> ')' */
void Rule_CREATESECURITYLABELCOMPONENT_CREATE_SECURITY_LABEL_COMPONENT_Id_TREE_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY POLICY> ::= CREATE SECURITY POLICY Id COMPONENTS <Id List> <CREATE SECURITY POLICY_IDSLBACRULESopt> RESTRICT NOT AUTHORIZED WRITE SECURITY LABEL */
void Rule_CREATESECURITYPOLICY_CREATE_SECURITY_POLICY_Id_COMPONENTS_RESTRICT_NOT_AUTHORIZED_WRITE_SECURITY_LABEL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY POLICY> ::= CREATE SECURITY POLICY Id COMPONENTS <Id List> <CREATE SECURITY POLICY_IDSLBACRULESopt> OVERRIDE NOT AUTHORIZED WRITE SECURITY LABEL */
void Rule_CREATESECURITYPOLICY_CREATE_SECURITY_POLICY_Id_COMPONENTS_OVERRIDE_NOT_AUTHORIZED_WRITE_SECURITY_LABEL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY POLICY_IDSLBACRULESopt> ::= WITH IDSLBACRULES */
void Rule_CREATESECURITYPOLICY_IDSLBACRULESopt_WITH_IDSLBACRULES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SECURITY POLICY_IDSLBACRULESopt> ::=  */
void Rule_CREATESECURITYPOLICY_IDSLBACRULESopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SEQUENCE> ::= CREATE SEQUENCE Id <ALTER SEQUENCEoptionS> */
void Rule_CREATESEQUENCE_CREATE_SEQUENCE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SYNONYM> ::= CREATE PUBLIC SYNONYM Id FOR Id */
void Rule_CREATESYNONYM_CREATE_PUBLIC_SYNONYM_Id_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SYNONYM> ::= CREATE SYNONYM Id FOR Id */
void Rule_CREATESYNONYM_CREATE_SYNONYM_Id_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE SYNONYM> ::= CREATE PRIVATE SYNONYM Id FOR Id */
void Rule_CREATESYNONYM_CREATE_PRIVATE_SYNONYM_Id_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE> ::= CREATE <TableType> TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> <SECURITY LABEL_clause> */
void Rule_CREATETABLE_CREATE_TABLE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE> ::= CREATE <TableType> TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> */
void Rule_CREATETABLE_CREATE_TABLE_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE> ::= CREATE <TableType> TABLE Id OF TYPE Id */
void Rule_CREATETABLE_CREATE_TABLE_Id_OF_TYPE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TableType> ::= STANDARD */
void Rule_TableType_STANDARD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TableType> ::= RAW */
void Rule_TableType_RAW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TableType> ::= STATIC */
void Rule_TableType_STATIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TableType> ::= OPERATIONAL */
void Rule_TableType_OPERATIONAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TableType> ::=  */
void Rule_TableType(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_option> ::= <CREATE TABLE_Option_COLSopt> <CREATE TABLE_StorageOpt> <CREATE SCRATCH TABLE_LockMode> <Access-Method_clauseOpt> */
void Rule_CREATETABLE_option(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_option> ::=  */
void Rule_CREATETABLE_option2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Option_COLSopt> ::= WITH CRCOLS */
void Rule_CREATETABLE_Option_COLSopt_WITH_CRCOLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Option_COLSopt> ::= WITH VERCOLS */
void Rule_CREATETABLE_Option_COLSopt_WITH_VERCOLS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Option_COLSopt> ::=  */
void Rule_CREATETABLE_Option_COLSopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_StorageOpt> ::= <CREATE TABLE_StorageOpt1> <CREATE TABLE_Put> <EXTENT SIZEopt> */
void Rule_CREATETABLE_StorageOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_StorageOpt> ::= <CREATE TABLE_StorageOpt1> <EXTENT SIZEopt> */
void Rule_CREATETABLE_StorageOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_StorageOpt1> ::= <CREATE INDEX_StorageOpt0> */
void Rule_CREATETABLE_StorageOpt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_StorageOpt1> ::= <FRAGMENT BY_TABLE> */
void Rule_CREATETABLE_StorageOpt12(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE0> ::= <FRAGMENT BY_INDEXopt1> ROUND ROBIN IN <Id List> */
void Rule_FRAGMENTBY_TABLE0_ROUND_ROBIN_IN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE0> ::= <FRAGMENT BY_INDEXopt1> ROUND ROBIN <FRAGMENT BY_TABLE_1s> */
void Rule_FRAGMENTBY_TABLE0_ROUND_ROBIN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE0> ::= <FRAGMENT BY_INDEX> */
void Rule_FRAGMENTBY_TABLE0(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_TABLE0> */
void Rule_FRAGMENTBY_TABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> EXPRESSION USING Id <FRAGMENT BY_Expr List> */
void Rule_FRAGMENTBY_TABLE_EXPRESSION_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> RANG '(' Id <FRAGMENT BY_TABLE_rang1> ')' IN <Id List> REMAINDER IN Id */
void Rule_FRAGMENTBY_TABLE_RANG_LParan_Id_RParan_IN_REMAINDER_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> RANG '(' Id <FRAGMENT BY_TABLE_rang1> ')' IN <Id List> */
void Rule_FRAGMENTBY_TABLE_RANG_LParan_Id_RParan_IN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> HYBRID <FRAGMENT BY_TABLE_rang2> <FRAGMENT BY_TABLE_rang3> */
void Rule_FRAGMENTBY_TABLE_HYBRID(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang1> ::= MIN IntegerLiteral MAX IntegerLiteral */
void Rule_FRAGMENTBY_TABLE_rang1_MIN_IntegerLiteral_MAX_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang1> ::= MAX IntegerLiteral */
void Rule_FRAGMENTBY_TABLE_rang1_MAX_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang1> ::= MIN IntegerLiteral IntegerLiteral */
void Rule_FRAGMENTBY_TABLE_rang1_MIN_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang1> ::= IntegerLiteral */
void Rule_FRAGMENTBY_TABLE_rang1_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang2> ::= '(' RANGE '(' Id <FRAGMENT BY_TABLE_rang1> ')' ')' */
void Rule_FRAGMENTBY_TABLE_rang2_LParan_RANGE_LParan_Id_RParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang2> ::= '(' RANGE '(' Id ')' ')' */
void Rule_FRAGMENTBY_TABLE_rang2_LParan_RANGE_LParan_Id_RParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang3> ::= RANGE '(' Id <FRAGMENT BY_TABLE_rang1> ')' <FRAGMENT BY_TABLE_IN> */
void Rule_FRAGMENTBY_TABLE_rang3_RANGE_LParan_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_rang3> ::= RANGE '(' Id ')' <FRAGMENT BY_TABLE_IN> */
void Rule_FRAGMENTBY_TABLE_rang3_RANGE_LParan_Id_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_IN> ::= IN <FRAGMENT BY_TABLE_IN_s> REMAINDER IN <FRAGMENT BY_TABLE_IN_> */
void Rule_FRAGMENTBY_TABLE_IN_IN_REMAINDER_IN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_IN> ::= IN <FRAGMENT BY_TABLE_IN_s> */
void Rule_FRAGMENTBY_TABLE_IN_IN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_IN_s> ::= <FRAGMENT BY_TABLE_IN_> <FRAGMENT BY_TABLE_IN_s_> */
void Rule_FRAGMENTBY_TABLE_IN_s(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_IN_s_> ::= ',' <FRAGMENT BY_TABLE_IN_> <FRAGMENT BY_TABLE_IN_s_> */
void Rule_FRAGMENTBY_TABLE_IN_s__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_IN_s_> ::=  */
void Rule_FRAGMENTBY_TABLE_IN_s_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_IN_> ::= Id */
void Rule_FRAGMENTBY_TABLE_IN__Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_IN_> ::= '(' <Id List> ')' */
void Rule_FRAGMENTBY_TABLE_IN__LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLEopt1> ::= WITH ROWIDS */
void Rule_FRAGMENTBY_TABLEopt1_WITH_ROWIDS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLEopt1> ::=  */
void Rule_FRAGMENTBY_TABLEopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_1s> ::= <FRAGMENT BY_TABLE_1> <FRAGMENT BY_TABLE_1s_> */
void Rule_FRAGMENTBY_TABLE_1s(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_1s_> ::= ',' <FRAGMENT BY_TABLE_1> <FRAGMENT BY_TABLE_1s_> */
void Rule_FRAGMENTBY_TABLE_1s__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_1s_> ::=  */
void Rule_FRAGMENTBY_TABLE_1s_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FRAGMENT BY_TABLE_1> ::= PARTITION Id IN Id */
void Rule_FRAGMENTBY_TABLE_1_PARTITION_Id_IN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Put> ::= PUT <CREATE TABLE_Put_S> */
void Rule_CREATETABLE_Put_PUT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Put_S> ::= <CREATE TABLE_Put_> <CREATE TABLE_Put_S_> */
void Rule_CREATETABLE_Put_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Put_S_> ::= ',' <CREATE TABLE_Put_> <CREATE TABLE_Put_S_> */
void Rule_CREATETABLE_Put_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Put_S_> ::=  */
void Rule_CREATETABLE_Put_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Put_> ::= Id IN '(' <Id List> ')' */
void Rule_CREATETABLE_Put__Id_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TABLE_Put_> ::= '(' <ALTER TABLE_PutOptions> ')' */
void Rule_CREATETABLE_Put__LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXTENT SIZEopt> ::= EXTENT SIZE IntegerLiteral NEXT SIZE IntegerLiteral */
void Rule_EXTENTSIZEopt_EXTENT_SIZE_IntegerLiteral_NEXT_SIZE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXTENT SIZEopt> ::= EXTENT SIZE IntegerLiteral */
void Rule_EXTENTSIZEopt_EXTENT_SIZE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXTENT SIZEopt> ::= NEXT SIZE IntegerLiteral */
void Rule_EXTENTSIZEopt_NEXT_SIZE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXTENT SIZEopt> ::=  */
void Rule_EXTENTSIZEopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TEMP TABLE> ::= CREATE TEMP TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> WITH NO LOG */
void Rule_CREATETEMPTABLE_CREATE_TEMP_TABLE_Id_LParan_RParan_WITH_NO_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TEMP TABLE> ::= CREATE TEMP TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> */
void Rule_CREATETEMPTABLE_CREATE_TEMP_TABLE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id <CREATE TRIGGER1> ENABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_ENABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id <CREATE TRIGGER1> DISABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_DISABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id <CREATE TRIGGER1> */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF <CREATE TRIGGER2> ENABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_ENABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF <CREATE TRIGGER2> DISABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_DISABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF <CREATE TRIGGER2> */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF ENABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_ENABLED2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF DISABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_DISABLED2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id ENABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_ENABLED2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id DISABLED */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id_DISABLED2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER> ::= CREATE TRIGGER Id */
void Rule_CREATETRIGGER_CREATE_TRIGGER_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= DELETE ON Id <CREATE TRIGGER1_actionS> */
void Rule_CREATETRIGGER1_DELETE_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= DELETE ON Id <CREATE TRIGGER1_1> */
void Rule_CREATETRIGGER1_DELETE_ON_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= SELECT ON Id <CREATE TRIGGER1_actionS> */
void Rule_CREATETRIGGER1_SELECT_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= SELECT ON Id <CREATE TRIGGER1_1> */
void Rule_CREATETRIGGER1_SELECT_ON_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= SELECT OF <Id List> ON Id <CREATE TRIGGER1_actionS> */
void Rule_CREATETRIGGER1_SELECT_OF_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= SELECT OF <Id List> ON Id <CREATE TRIGGER1_1> */
void Rule_CREATETRIGGER1_SELECT_OF_ON_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= UPDATE ON Id <CREATE TRIGGER1_actionS> */
void Rule_CREATETRIGGER1_UPDATE_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= UPDATE ON Id <CREATE TRIGGER1_2> */
void Rule_CREATETRIGGER1_UPDATE_ON_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= UPDATE OF <Id List> ON Id <CREATE TRIGGER1_actionS> */
void Rule_CREATETRIGGER1_UPDATE_OF_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= UPDATE OF <Id List> ON Id <CREATE TRIGGER1_2> */
void Rule_CREATETRIGGER1_UPDATE_OF_ON_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= INSERT ON Id <CREATE TRIGGER1_new> <CREATE TRIGGER1_event> */
void Rule_CREATETRIGGER1_INSERT_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1> ::= INSERT ON Id <CREATE TRIGGER1_actionS> */
void Rule_CREATETRIGGER1_INSERT_ON_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_1> ::= <CREATE TRIGGER1_old> <CREATE TRIGGER1_event> */
void Rule_CREATETRIGGER1_1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_2> ::= <CREATE TRIGGER1_old> <CREATE TRIGGER1_event> */
void Rule_CREATETRIGGER1_2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_2> ::= <CREATE TRIGGER1_old> <CREATE TRIGGER1_new> <CREATE TRIGGER1_event> */
void Rule_CREATETRIGGER1_22(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> FOR EACH ROW <CREATE TRIGGER1_action> AFTER <CREATE TRIGGER1_action> */
void Rule_CREATETRIGGER1_actionS_BEFORE_FOR_EACH_ROW_AFTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> FOR EACH ROW <CREATE TRIGGER1_action> */
void Rule_CREATETRIGGER1_actionS_BEFORE_FOR_EACH_ROW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> AFTER <CREATE TRIGGER1_action> */
void Rule_CREATETRIGGER1_actionS_BEFORE_AFTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_actionS> ::= FOR EACH ROW <CREATE TRIGGER1_action> AFTER <CREATE TRIGGER1_action> */
void Rule_CREATETRIGGER1_actionS_FOR_EACH_ROW_AFTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> */
void Rule_CREATETRIGGER1_actionS_BEFORE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_actionS> ::= FOR EACH ROW <CREATE TRIGGER1_action> */
void Rule_CREATETRIGGER1_actionS_FOR_EACH_ROW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_actionS> ::= AFTER <CREATE TRIGGER1_action> */
void Rule_CREATETRIGGER1_actionS_AFTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action> ::= <CREATE TRIGGER1_action_S> */
void Rule_CREATETRIGGER1_action(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_S> ::= <CREATE TRIGGER1_action_> <CREATE TRIGGER1_action_S_> */
void Rule_CREATETRIGGER1_action_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_S_> ::= ',' <CREATE TRIGGER1_action_> <CREATE TRIGGER1_action_S_> */
void Rule_CREATETRIGGER1_action_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_S_> ::=  */
void Rule_CREATETRIGGER1_action_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_> ::= WHEN '(' <Expression> ')' '(' <CREATE TRIGGER1_action_S2> ')' */
void Rule_CREATETRIGGER1_action__WHEN_LParan_RParan_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_> ::= '(' <CREATE TRIGGER1_action_S2> ')' */
void Rule_CREATETRIGGER1_action__LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_S2> ::= <CREATE TRIGGER1_action_2> <CREATE TRIGGER1_action_S2_> */
void Rule_CREATETRIGGER1_action_S2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_S2_> ::= ',' <CREATE TRIGGER1_action_2> <CREATE TRIGGER1_action_S2_> */
void Rule_CREATETRIGGER1_action_S2__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_S2_> ::=  */
void Rule_CREATETRIGGER1_action_S2_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_2> ::= <INSERT> */
void Rule_CREATETRIGGER1_action_2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_2> ::= <DELETE> */
void Rule_CREATETRIGGER1_action_22(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_2> ::= <UPDATE> */
void Rule_CREATETRIGGER1_action_23(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_2> ::= <EXECUTE PROCEDURE> */
void Rule_CREATETRIGGER1_action_24(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_action_2> ::= <EXECUTE FUNCTION> */
void Rule_CREATETRIGGER1_action_25(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_old> ::= <referencing_clause_DELETE> */
void Rule_CREATETRIGGER1_old(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_new> ::= <referencing_clause_INSERT> */
void Rule_CREATETRIGGER1_new(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER1_event> ::= <CREATE TRIGGER1_actionS> */
void Rule_CREATETRIGGER1_event(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER2> ::= INSERT ON Id <referencing_clause_INSERT> FOR EACH ROW '(' <CREATE TRIGGER1_action_S2> ')' */
void Rule_CREATETRIGGER2_INSERT_ON_Id_FOR_EACH_ROW_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER2> ::= DELETE ON Id <referencing_clause_DELETE> FOR EACH ROW '(' <CREATE TRIGGER1_action_S2> ')' */
void Rule_CREATETRIGGER2_DELETE_ON_Id_FOR_EACH_ROW_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE TRIGGER2> ::= UPDATE ON Id <referencing_clause_UPDATE> FOR EACH ROW '(' <CREATE TRIGGER1_action_S2> ')' */
void Rule_CREATETRIGGER2_UPDATE_ON_Id_FOR_EACH_ROW_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE VIEW> ::= CREATE VIEW Id AS <SELECT> <WITH CHECKopt> */
void Rule_CREATEVIEW_CREATE_VIEW_Id_AS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE VIEW> ::= CREATE VIEW Id '(' <Id List> ')' AS <SELECT> <WITH CHECKopt> */
void Rule_CREATEVIEW_CREATE_VIEW_Id_LParan_RParan_AS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE VIEW> ::= CREATE VIEW Id OF TYPE Id AS <SELECT> <WITH CHECKopt> */
void Rule_CREATEVIEW_CREATE_VIEW_Id_OF_TYPE_Id_AS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WITH CHECKopt> ::= WITH CHECK OPTION */
void Rule_WITHCHECKopt_WITH_CHECK_OPTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WITH CHECKopt> ::=  */
void Rule_WITHCHECKopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE XADATASOURCE> ::= CREATE XADATASOURCE Id USING Id */
void Rule_CREATEXADATASOURCE_CREATE_XADATASOURCE_Id_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CREATE XADATASOURCE TYPE> ::= CREATE XADATASOURCE TYPE Id '(' <PurposeOption2S> ')' */
void Rule_CREATEXADATASOURCETYPE_CREATE_XADATASOURCE_TYPE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption2S> ::= <PurposeOption2> <PurposeOption2S_> */
void Rule_PurposeOption2S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption2S_> ::= ',' <PurposeOption2> <PurposeOption2S_> */
void Rule_PurposeOption2S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption2S_> ::=  */
void Rule_PurposeOption2S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption2> ::= <PurposeKeyword2> '=' Id */
void Rule_PurposeOption2_Eq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeOption2> ::= <PurposeKeyword2> '=' IntegerLiteral */
void Rule_PurposeOption2_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_flags' */
void Rule_PurposeKeyword2_xa_flags(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_version' */
void Rule_PurposeKeyword2_xa_version(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_open' */
void Rule_PurposeKeyword2_xa_open(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_close' */
void Rule_PurposeKeyword2_xa_close(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_start' */
void Rule_PurposeKeyword2_xa_start(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_end' */
void Rule_PurposeKeyword2_xa_end(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_rollback' */
void Rule_PurposeKeyword2_xa_rollback(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_prepare' */
void Rule_PurposeKeyword2_xa_prepare(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_commit' */
void Rule_PurposeKeyword2_xa_commit(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_recover' */
void Rule_PurposeKeyword2_xa_recover(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_forget' */
void Rule_PurposeKeyword2_xa_forget(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PurposeKeyword2> ::= 'xa_complete' */
void Rule_PurposeKeyword2_xa_complete(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP ACCESS_METHOD> ::= DROP 'ACCESS_METHOD' Id RESTRICT */
void Rule_DROPACCESS_METHOD_DROP_ACCESS_METHOD_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP AGGREGATE> ::= DROP AGGREGATE Id */
void Rule_DROPAGGREGATE_DROP_AGGREGATE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP CAST> ::= DROP CAST '(' Id AS Id ')' */
void Rule_DROPCAST_DROP_CAST_LParan_Id_AS_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP DATABASE> ::= DROP DATABASE Id */
void Rule_DROPDATABASE_DROP_DATABASE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP DUPLICATE> ::= DROP DUPLICATE OF TABLE Id */
void Rule_DROPDUPLICATE_DROP_DUPLICATE_OF_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP FUNCTION> ::= DROP FUNCTION Id '(' <TypeList> ')' */
void Rule_DROPFUNCTION_DROP_FUNCTION_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP FUNCTION> ::= DROP FUNCTION Id */
void Rule_DROPFUNCTION_DROP_FUNCTION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP FUNCTION> ::= DROP SPECIFIC FUNCTION Id */
void Rule_DROPFUNCTION_DROP_SPECIFIC_FUNCTION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP INDEX> ::= DROP INDEX Id ONLINE */
void Rule_DROPINDEX_DROP_INDEX_Id_ONLINE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP INDEX> ::= DROP INDEX Id */
void Rule_DROPINDEX_DROP_INDEX_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP OPCLASS> ::= DROP OPCLASS Id RESTRICT */
void Rule_DROPOPCLASS_DROP_OPCLASS_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP PROCEDURE> ::= DROP PROCEDURE Id '(' <TypeList> ')' */
void Rule_DROPPROCEDURE_DROP_PROCEDURE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP PROCEDURE> ::= DROP PROCEDURE Id */
void Rule_DROPPROCEDURE_DROP_PROCEDURE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP PROCEDURE> ::= DROP SPECIFIC PROCEDURE Id */
void Rule_DROPPROCEDURE_DROP_SPECIFIC_PROCEDURE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP ROLE> ::= DROP ROLE StringLiteral */
void Rule_DROPROLE_DROP_ROLE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP ROLE> ::= DROP ROLE Id */
void Rule_DROPROLE_DROP_ROLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP ROUTINE> ::= DROP ROUTINE Id '(' <TypeList> ')' */
void Rule_DROPROUTINE_DROP_ROUTINE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP ROUTINE> ::= DROP ROUTINE Id */
void Rule_DROPROUTINE_DROP_ROUTINE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP ROUTINE> ::= DROP SPECIFIC ROUTINE Id */
void Rule_DROPROUTINE_DROP_SPECIFIC_ROUTINE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP ROW TYPE> ::= DROP ROW TYPE Id RESTRICT */
void Rule_DROPROWTYPE_DROP_ROW_TYPE_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SEQUENCE> ::= DROP SEQUENCE Id */
void Rule_DROPSEQUENCE_DROP_SEQUENCE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SECURITY> ::= DROP SECURITY LABEL Id RESTRICT */
void Rule_DROPSECURITY_DROP_SECURITY_LABEL_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SECURITY> ::= DROP SECURITY LABEL Id */
void Rule_DROPSECURITY_DROP_SECURITY_LABEL_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SECURITY> ::= DROP SECURITY LABEL COMPONENT Id RESTRICT */
void Rule_DROPSECURITY_DROP_SECURITY_LABEL_COMPONENT_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SECURITY> ::= DROP SECURITY LABEL COMPONENT Id */
void Rule_DROPSECURITY_DROP_SECURITY_LABEL_COMPONENT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SECURITY> ::= DROP POLICY Id RESTRICT */
void Rule_DROPSECURITY_DROP_POLICY_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SECURITY> ::= DROP POLICY Id */
void Rule_DROPSECURITY_DROP_POLICY_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP SYNONYM> ::= DROP SYNONYM Id */
void Rule_DROPSYNONYM_DROP_SYNONYM_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP TABLE> ::= DROP TABLE Id CASCADE */
void Rule_DROPTABLE_DROP_TABLE_Id_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP TABLE> ::= DROP TABLE Id RESTRICT */
void Rule_DROPTABLE_DROP_TABLE_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP TABLE> ::= DROP TABLE Id */
void Rule_DROPTABLE_DROP_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP TRIGGER> ::= DROP TRIGGER Id */
void Rule_DROPTRIGGER_DROP_TRIGGER_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP TYPE> ::= DROP TYPE Id RESTRICT */
void Rule_DROPTYPE_DROP_TYPE_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP VIEW> ::= DROP VIEW Id CASCADE */
void Rule_DROPVIEW_DROP_VIEW_Id_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP VIEW> ::= DROP VIEW Id RESTRICT */
void Rule_DROPVIEW_DROP_VIEW_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP VIEW> ::= DROP VIEW Id */
void Rule_DROPVIEW_DROP_VIEW_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP XADATASOURCE> ::= DROP XADATASOURCE Id RESTRICT */
void Rule_DROPXADATASOURCE_DROP_XADATASOURCE_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DROP XADATASOURCE TYPE> ::= DROP XADATASOURCE TYPE Id RESTRICT */
void Rule_DROPXADATASOURCETYPE_DROP_XADATASOURCE_TYPE_Id_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLE> ::= MOVE TABLE Id TO DATABASE Id <MOVE TABLEopt1> <MOVE TABLEopt2> */
void Rule_MOVETABLE_MOVE_TABLE_Id_TO_DATABASE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt1> ::= RENAME Id */
void Rule_MOVETABLEopt1_RENAME_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt1> ::=  */
void Rule_MOVETABLEopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ',' ROLES ')' RESTRICT */
void Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_Comma_ROLES_RParan_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ',' ROLES ')' CASCADE */
void Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_Comma_ROLES_RParan_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ')' RESTRICT */
void Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_RParan_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ')' CASCADE */
void Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_RParan_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt2> ::= RESTRICT */
void Rule_MOVETABLEopt2_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MOVE TABLEopt2> ::= CASCADE */
void Rule_MOVETABLEopt2_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME COLUMN> ::= RENAME COLUMN Id TO Id */
void Rule_RENAMECOLUMN_RENAME_COLUMN_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME DATABASE> ::= RENAME DATABASE Id TO Id */
void Rule_RENAMEDATABASE_RENAME_DATABASE_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME INDEX> ::= RENAME INDEX Id TO Id */
void Rule_RENAMEINDEX_RENAME_INDEX_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME SECURITY> ::= RENAME SECURITY POLICY Id TO Id */
void Rule_RENAMESECURITY_RENAME_SECURITY_POLICY_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME SECURITY> ::= RENAME SECURITY LABEL Id TO Id */
void Rule_RENAMESECURITY_RENAME_SECURITY_LABEL_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME SECURITY> ::= RENAME SECURITY LABEL COMPONENT Id TO Id */
void Rule_RENAMESECURITY_RENAME_SECURITY_LABEL_COMPONENT_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME SEQUENCE> ::= RENAME SEQUENCE Id TO Id */
void Rule_RENAMESEQUENCE_RENAME_SEQUENCE_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RENAME TABLE> ::= RENAME TABLE Id TO Id */
void Rule_RENAMETABLE_RENAME_TABLE_Id_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETE> ::= DELETE <hint_clauseOpt> <DELETEopt1> <DELETEopt2> */
void Rule_DELETE_DELETE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= FROM Id */
void Rule_DELETEopt1_FROM_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= FROM ONLY '(' Id ')' */
void Rule_DELETEopt1_FROM_ONLY_LParan_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= Id */
void Rule_DELETEopt1_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= ONLY '(' Id ')' */
void Rule_DELETEopt1_ONLY_LParan_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= FROM <CollectionDerivedTable> */
void Rule_DELETEopt1_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= FROM ONLY '(' <CollectionDerivedTable> ')' */
void Rule_DELETEopt1_FROM_ONLY_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= <CollectionDerivedTable> */
void Rule_DELETEopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt1> ::= ONLY '(' <CollectionDerivedTable> ')' */
void Rule_DELETEopt1_ONLY_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt2> ::= USING <Id List> WHERE <Expression> */
void Rule_DELETEopt2_USING_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt2> ::= FROM <Id List> WHERE <Expression> */
void Rule_DELETEopt2_FROM_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt2> ::= WHERE <Expression> */
void Rule_DELETEopt2_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt2> ::= WHERE CURRENT OF Id */
void Rule_DELETEopt2_WHERE_CURRENT_OF_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DELETEopt2> ::=  */
void Rule_DELETEopt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_clauseOpt> ::= '--+' <hintS> */
void Rule_hint_clauseOpt_MinusMinusPlus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_clauseOpt> ::= '{+' <hintS> '}' */
void Rule_hint_clauseOpt_LBracePlus_RBrace(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_clauseOpt> ::= '/ *+' <hintS> '+* /' */
void Rule_hint_clauseOpt_DivTimesPlus_PlusTimesDiv(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_clauseOpt> ::=  */
void Rule_hint_clauseOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hintS> ::= <hint> <hintS_> */
void Rule_hintS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hintS_> ::= ',' <hint> <hintS_> */
void Rule_hintS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hintS_> ::=  */
void Rule_hintS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint> ::= <hint_ReadWrite> */
void Rule_hint(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint> ::= <hint_JoinOrder> */
void Rule_hint2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint> ::= <hint_JoinMethod> */
void Rule_hint3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint> ::= <hint_Object> */
void Rule_hint4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint> ::= <hint_Explain> */
void Rule_hint5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint> ::= <hint_Rewrite> */
void Rule_hint6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWrite> ::= <hint_ReadWriteOpt1> '(' Id <Id List> ')' StringLiteral */
void Rule_hint_ReadWrite_LParan_Id_RParan_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWrite> ::= <hint_ReadWriteOpt1> '(' Id <Id List> ')' */
void Rule_hint_ReadWrite_LParan_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWrite> ::= <hint_ReadWriteOpt2> '(' Id ')' StringLiteral */
void Rule_hint_ReadWrite_LParan_Id_RParan_StringLiteral2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWrite> ::= <hint_ReadWriteOpt2> '(' Id ')' */
void Rule_hint_ReadWrite_LParan_Id_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWriteOpt1> ::= 'INDEX_ALL' */
void Rule_hint_ReadWriteOpt1_INDEX_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWriteOpt1> ::= INDEX */
void Rule_hint_ReadWriteOpt1_INDEX(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWriteOpt1> ::= 'AVOID_INDEX' */
void Rule_hint_ReadWriteOpt1_AVOID_INDEX(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWriteOpt1> ::= 'AVOID_INDEX_SJ' */
void Rule_hint_ReadWriteOpt1_AVOID_INDEX_SJ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWriteOpt1> ::= 'INDEX_SJ' */
void Rule_hint_ReadWriteOpt1_INDEX_SJ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWriteOpt2> ::= FULL */
void Rule_hint_ReadWriteOpt2_FULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_ReadWriteOpt2> ::= 'AVOID_FULL' */
void Rule_hint_ReadWriteOpt2_AVOID_FULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinOrder> ::= ORDERED StringLiteral */
void Rule_hint_JoinOrder_ORDERED_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinOrder> ::= ORDERED */
void Rule_hint_JoinOrder_ORDERED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'USE_NL' '(' <Id List> ')' StringLiteral */
void Rule_hint_JoinMethod_USE_NL_LParan_RParan_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'USE_NL' '(' <Id List> ')' */
void Rule_hint_JoinMethod_USE_NL_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'AVOID_NL' '(' <Id List> ')' StringLiteral */
void Rule_hint_JoinMethod_AVOID_NL_LParan_RParan_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'AVOID_NL' '(' <Id List> ')' */
void Rule_hint_JoinMethod_AVOID_NL_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List> ')' StringLiteral */
void Rule_hint_JoinMethod_USE_HASH_LParan_RParan_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List> ')' */
void Rule_hint_JoinMethod_USE_HASH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List> ')' StringLiteral */
void Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List> ')' */
void Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List_hint_JoinMethod> ')' StringLiteral */
void Rule_hint_JoinMethod_USE_HASH_LParan_RParan_StringLiteral2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List_hint_JoinMethod> ')' */
void Rule_hint_JoinMethod_USE_HASH_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List_hint_JoinMethod> ')' StringLiteral */
void Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan_StringLiteral2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List_hint_JoinMethod> ')' */
void Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id List_hint_JoinMethod> ::= <Id_hint_JoinMethod> <Id List_hint_JoinMethod_> */
void Rule_IdList_hint_JoinMethod(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id List_hint_JoinMethod_> ::= ',' <Id_hint_JoinMethod> <Id List_hint_JoinMethod_> */
void Rule_IdList_hint_JoinMethod__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id List_hint_JoinMethod_> ::=  */
void Rule_IdList_hint_JoinMethod_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id_hint_JoinMethod> ::= Id '/BUILD' '/BROADCAST' */
void Rule_Id_hint_JoinMethod_Id_DivBUILD_DivBROADCAST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id_hint_JoinMethod> ::= Id '/BUILD' */
void Rule_Id_hint_JoinMethod_Id_DivBUILD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id_hint_JoinMethod> ::= Id '/PROBE' '/BROADCAST' */
void Rule_Id_hint_JoinMethod_Id_DivPROBE_DivBROADCAST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id_hint_JoinMethod> ::= Id '/PROBE' */
void Rule_Id_hint_JoinMethod_Id_DivPROBE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Object> ::= 'ALL_ROWS' StringLiteral */
void Rule_hint_Object_ALL_ROWS_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Object> ::= 'ALL_ROWS' */
void Rule_hint_Object_ALL_ROWS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Object> ::= 'FIRST_ROWS' StringLiteral */
void Rule_hint_Object_FIRST_ROWS_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Object> ::= 'FIRST_ROWS' */
void Rule_hint_Object_FIRST_ROWS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Explain> ::= EXPLAIN ',' 'AVOID_EXECUTE' StringLiteral */
void Rule_hint_Explain_EXPLAIN_Comma_AVOID_EXECUTE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Explain> ::= EXPLAIN ',' 'AVOID_EXECUTE' */
void Rule_hint_Explain_EXPLAIN_Comma_AVOID_EXECUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Explain> ::= EXPLAIN 'AVOID_EXECUTE' StringLiteral */
void Rule_hint_Explain_EXPLAIN_AVOID_EXECUTE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Explain> ::= EXPLAIN 'AVOID_EXECUTE' */
void Rule_hint_Explain_EXPLAIN_AVOID_EXECUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Explain> ::= EXPLAIN StringLiteral */
void Rule_hint_Explain_EXPLAIN_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Explain> ::= EXPLAIN */
void Rule_hint_Explain_EXPLAIN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Rewrite> ::= NESTED StringLiteral */
void Rule_hint_Rewrite_NESTED_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <hint_Rewrite> ::= NESTED */
void Rule_hint_Rewrite_NESTED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT_byVALUES> ::= INSERT INTO Id '(' <Id List> ')' VALUES '(' <Expr List> ')' */
void Rule_INSERT_byVALUES_INSERT_INTO_Id_LParan_RParan_VALUES_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT_byVALUES> ::= INSERT INTO Id VALUES '(' <Expr List> ')' */
void Rule_INSERT_byVALUES_INSERT_INTO_Id_VALUES_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO Id <Id List> '(' <Expr List> ')' */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO Id <Id List> <SELECT> */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO Id '(' <Expr List> ')' */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO Id <SELECT> */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT INTO Id <Id List> '(' <Expr List> ')' */
void Rule_INSERT2_INSERT_INTO_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT INTO Id <Id List> <SELECT> */
void Rule_INSERT2_INSERT_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> <Id List> '(' <Expr List> ')' */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> <Id List> <SELECT> */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> '(' <Expr List> ')' */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> <SELECT> */
void Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT INTO <CollectionDerivedTable> <Id List> '(' <Expr List> ')' */
void Rule_INSERT2_INSERT_INTO_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT2> ::= INSERT INTO <CollectionDerivedTable> <Id List> <SELECT> */
void Rule_INSERT2_INSERT_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT> ::= <INSERT_byVALUES> */
void Rule_INSERT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT> ::= INSERT INTO Id '(' <Id List> ')' <INSERT_EXECUTE> */
void Rule_INSERT_INSERT_INTO_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT> ::= INSERT INTO Id '(' <Id List> ')' <SELECT> */
void Rule_INSERT_INSERT_INTO_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT> ::= INSERT INTO Id <INSERT_EXECUTE> */
void Rule_INSERT_INSERT_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT> ::= INSERT INTO Id <SELECT> */
void Rule_INSERT_INSERT_INTO_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT> ::= <INSERT2> */
void Rule_INSERT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT_EXECUTE> ::= <EXECUTE PROCEDURE0> */
void Rule_INSERT_EXECUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INSERT_EXECUTE> ::= <EXECUTE FUNCTION0> */
void Rule_INSERT_EXECUTE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOAD> ::= LOAD FROM StringLiteral DELIMITER StringLiteral INSERT INTO Id '(' <Id List> ')' */
void Rule_LOAD_LOAD_FROM_StringLiteral_DELIMITER_StringLiteral_INSERT_INTO_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOAD> ::= LOAD FROM StringLiteral DELIMITER StringLiteral INSERT INTO Id */
void Rule_LOAD_LOAD_FROM_StringLiteral_DELIMITER_StringLiteral_INSERT_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOAD> ::= LOAD FROM StringLiteral INSERT INTO Id '(' <Id List> ')' */
void Rule_LOAD_LOAD_FROM_StringLiteral_INSERT_INTO_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOAD> ::= LOAD FROM StringLiteral INSERT INTO Id */
void Rule_LOAD_LOAD_FROM_StringLiteral_INSERT_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE> ::= MERGE INTO <hint_clauseOpt> Id AS Id <MERGE_USING> <MERGE_WHEN MATCHED> <MERGE_WHEN NOT MATCHED> */
void Rule_MERGE_MERGE_INTO_Id_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE> ::= MERGE INTO <hint_clauseOpt> Id Id <MERGE_USING> <MERGE_WHEN MATCHED> <MERGE_WHEN NOT MATCHED> */
void Rule_MERGE_MERGE_INTO_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE> ::= MERGE INTO <hint_clauseOpt> Id <MERGE_USING> <MERGE_WHEN MATCHED> <MERGE_WHEN NOT MATCHED> */
void Rule_MERGE_MERGE_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_USING> ::= USING Id AS Id ON <Expression> */
void Rule_MERGE_USING_USING_Id_AS_Id_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_USING> ::= USING Id Id ON <Expression> */
void Rule_MERGE_USING_USING_Id_Id_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_USING> ::= USING Id ON <Expression> */
void Rule_MERGE_USING_USING_Id_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_USING> ::= USING <SELECT> AS Id ON <Expression> */
void Rule_MERGE_USING_USING_AS_Id_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_USING> ::= USING <SELECT> Id ON <Expression> */
void Rule_MERGE_USING_USING_Id_ON2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_USING> ::= USING <SELECT> ON <Expression> */
void Rule_MERGE_USING_USING_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_WHEN MATCHED> ::= WHEN MATCHED THEN UPDATE <UPDATE_SET> */
void Rule_MERGE_WHENMATCHED_WHEN_MATCHED_THEN_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <MERGE_WHEN NOT MATCHED> ::= WHEN NOT MATCHED THEN INSERT <Id List> '(' <Expr List> ')' */
void Rule_MERGE_WHENNOTMATCHED_WHEN_NOT_MATCHED_THEN_INSERT_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};






//[zlq/SELECT:
/* <SELECTinto> ::= <SELECT_> <SELECT_unionS> <SELECT_ORDERopt> <SELECT_FORopt> <SELECT_INTOTableOpt> */
void Rule_SELECTinto(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
printf("Rule_SELECTinto\n");
};

/* <SELECT> ::= <SELECTinto> */
void Rule_SELECT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
printf("Rule_SELECT\n");
};

//
struct SELECT_Column* SELECT_columnS(struct TokenStruct* Token, struct SELECT_Column* columnS){
/*
<SELECT_Projection> ::= <SELECT_ProjectionOpt1> <SELECT_ProjectionOpt2> <SELECT_ProjectionOpt3> <SELECT_ColumnS>
*/

}
struct SELECT_* SELECT_unionS(struct TokenStruct* Token, struct SELECT_* select_OR_unionS){//只有上层无法处理的节点才放在[下层子节点自身]中处理
/*
<SELECT_unionS> ::= UNION ALL <SELECT_>  <SELECT_unionS>
<SELECT_unionS> ::= UNION     <SELECT_>  <SELECT_unionS>
<SELECT_unionS> ::= 
*/
  struct TokenStruct* tokSELECT;
  struct TokenStruct* tokUnionS;
  enum SELECT_unionType union_;
  struct SELECT_* select;
  
  if(!select_OR_unionS){//selectStmt->selectS=SELECT_unionS(Token->Tokens[0], NULL);
    tokSELECT=Token;
    tokUnionS=NULL;
    union_=NO_UNION;
  }else{                //selectStmt->selectS=SELECT_unionS(Token->Tokens[1], selectStmt->selectS);
    if(Grammar.RuleArray[Token->ReductionRule].SymbolsCount>0){
      if(Token->Tokens[1]->ReductionRule < 0){//<SELECT_unionS> ::= UNION ALL <SELECT_> <SELECT_unionS>
        tokSELECT=Token->Tokens[2];
        tokUnionS=Token->Tokens[3];
        union_=UNION_ALL;
      }else{                                  //<SELECT_unionS> ::= UNION     <SELECT_> <SELECT_unionS>
        tokSELECT=Token->Tokens[1];
        tokUnionS=Token->Tokens[2];
        union_=UNION_NORMAL;
      }
    }else{                                    //<SELECT_unionS> ::= 
      tokSELECT=NULL;
      tokUnionS=NULL;
      union_=NO_UNION;
    }
  }
  if(tokSELECT){
/*
<SELECT_> ::= SELECT <hint_clauseOpt> <SELECT_Projection> INTO <Id List> <SELECT_FROM> <SELECT_WHEREopt>
               <SELECT_HierarchicalOpt> <SELECT_GROUPopt> <SELECT_HAVINGopt>
<SELECT_> ::= SELECT <hint_clauseOpt> <SELECT_Projection>                <SELECT_FROM> <SELECT_WHEREopt>
               <SELECT_HierarchicalOpt> <SELECT_GROUPopt> <SELECT_HAVINGopt>
*/
    select=(struct SELECT_*)malloc(sizeof(struct SELECT_*));
    select->next=NULL;
    //
    select->union_=union_;
    ///
#if 0
    select->hintS
#endif
    // //----2013-01-13-16-57;+----
/*
<SELECT_Projection> ::= <SELECT_ProjectionOpt1> <SELECT_ProjectionOpt2> <SELECT_ProjectionOpt3> <SELECT_ColumnS>
*/
    select->projection.opt1=0;
//    select->projection.expr1=NULL;
    select->projection.opt2=0;
//    select->projection.expr2=NULL;
    select->projection.opt3=0;
//    select->projection.columnS=SELECT_columnS(Token->Tokens[2]->Tokens[3], NULL);
#if 0
    //
    select->intoS
    //
    select->fromS
    //
    select->where
    //
    select->Hire
    //
    select->groupS
    //
    select->havingS
#endif
    ///
    if(!select_OR_unionS){//selectStmt->selectS=SELECT_unionS(Token->Tokens[0], NULL);
      select_OR_unionS=select;
    }else{                //selectStmt->selectS=SELECT_unionS(Token->Tokens[1], selectStmt->selectS);
      select_OR_unionS->next=select;
    }
  }
  if(tokUnionS){
    //select_OR_unionS=SELECT_unionS(tokUnionS, select_OR_unionS);
    SELECT_unionS(tokUnionS, select);
  }
  
  return select_OR_unionS;
}//--SELECT_unionS
/* <SELECT> ::= <SELECT_> <SELECT_unionS> <SELECT_ORDERopt> <SELECT_FORopt> */
void Rule_SELECT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
//printf("Rule_SELECT2\n");
  printf("%S \n", Grammar.RuleArray[Token->ReductionRule].Description);
  printf("%S \n", Grammar.RuleArray[Token->Tokens[0]->ReductionRule].Description);
  printf("  %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[1]->ReductionRule].Description);
  printf("  %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[2]->ReductionRule].Description);
  printf("    %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[2]->Tokens[0]->ReductionRule].Description);
  printf("    %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[2]->Tokens[1]->ReductionRule].Description);
  printf("    %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[2]->Tokens[2]->ReductionRule].Description);
  printf("    %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[2]->Tokens[3]->ReductionRule].Description);
  printf("      %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[2]->Tokens[3]->Tokens[0]->ReductionRule].Description);
  printf("      %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[2]->Tokens[3]->Tokens[1]->ReductionRule].Description);
  printf("  %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[3]->ReductionRule].Description);
  printf("    %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[3]->Tokens[1]->ReductionRule].Description);
  printf("      %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[3]->Tokens[1]->Tokens[0]->ReductionRule].Description);
  printf("      %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[3]->Tokens[1]->Tokens[1]->ReductionRule].Description);
  printf("  %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[4]->ReductionRule].Description);
  printf("  %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[5]->ReductionRule].Description);
  printf("  %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[6]->ReductionRule].Description);
  printf("  %S \n", Grammar.RuleArray[Token->Tokens[0]->Tokens[7]->ReductionRule].Description);
  printf("%S \n", Grammar.RuleArray[Token->Tokens[1]->ReductionRule].Description);
  printf("%S \n", Grammar.RuleArray[Token->Tokens[2]->ReductionRule].Description);
  printf("%S \n", Grammar.RuleArray[Token->Tokens[3]->ReductionRule].Description);
  /*“SELECT * FROM T1;” =>:
<SELECT> ::= <SELECT_> <SELECT_unionS> <SELECT_ORDERopt> <SELECT_FORopt> 
<SELECT_> ::= SELECT <hint_clauseOpt> <SELECT_Projection> <SELECT_FROM> <SELECT_WHEREopt> <SELECT_HierarchicalOpt> <SELECT_GROUPopt> <SELECT_HAVINGopt> 
  <hint_clauseOpt> ::=  
  <SELECT_Projection> ::= <SELECT_ProjectionOpt1> <SELECT_ProjectionOpt2> <SELECT_ProjectionOpt3> <SELECT_ColumnS> 
    <SELECT_ProjectionOpt1> ::=  
    <SELECT_ProjectionOpt2> ::=  
    <SELECT_ProjectionOpt3> ::=  
    <SELECT_ColumnS> ::= <SELECT_Column> <SELECT_ColumnS_> 
      <SELECT_Column> ::= '*' 
      <SELECT_ColumnS_> ::=  
  <SELECT_FROM> ::= FROM <SELECT_FROM_ANSItableS> 
    <SELECT_FROM_ANSItableS> ::= <SELECT_FROM_ANSItable> <SELECT_FROM_ANSItableS_> 
      <SELECT_FROM_ANSItable> ::= Id 
      <SELECT_FROM_ANSItableS_> ::=  
  <SELECT_WHEREopt> ::=  
  <SELECT_HierarchicalOpt> ::=  
  <SELECT_GROUPopt> ::=  
  <SELECT_HAVINGopt> ::=  
<SELECT_unionS> ::=  
<SELECT_ORDERopt> ::=  
<SELECT_FORopt> ::=  
  */
  struct SyntaxNode_SELECT* selectStmt=(struct SyntaxNode_SELECT*)malloc(sizeof(struct SyntaxNode_SELECT));
  
  selectStmt->p.type=SYN_NODE_SELECT;
  //////strncpy(selectStmt->p.desc, (const char*)Grammar.RuleArray[Token->ReductionRule].Description, 255);
  ////selectStmt->p.desc=strndup((const char*)Grammar.RuleArray[Token->ReductionRule].Description, 255);
  //selectStmt->p.desc=(wchar_t*)malloc(sizeof(wchar_t)*(wcslen(Grammar.RuleArray[Token->ReductionRule].Description))+1);
  //wcsncpy(selectStmt->p.desc, (const wchar_t*)Grammar.RuleArray[Token->ReductionRule].Description, 255);
  selectStmt->p.desc=Grammar.RuleArray[Token->ReductionRule].Description;
  selectStmt->p.next=NULL; 
  //
  selectStmt->selectS=SELECT_unionS(Token->Tokens[0], NULL);
  selectStmt->selectS=SELECT_unionS(Token->Tokens[1], selectStmt->selectS);
#if 0
  selectStmt->orderS
  selectStmt->isForUpdate
#endif


  Token->SyntaxNode=(struct SyntaxNode_common*)selectStmt;
};
//zlq/SELECT;]







/* <SELECT_unionS> ::= UNION ALL <SELECT_> <SELECT_unionS> */
void Rule_SELECT_unionS_UNION_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_unionS> ::= UNION <SELECT_> <SELECT_unionS> */
void Rule_SELECT_unionS_UNION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_unionS> ::=  */
void Rule_SELECT_unionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_> ::= SELECT <hint_clauseOpt> <SELECT_Projection> INTO <Id List> <SELECT_FROM> <SELECT_WHEREopt> <SELECT_HierarchicalOpt> <SELECT_GROUPopt> <SELECT_HAVINGopt> */
void Rule_SELECT__SELECT_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_> ::= SELECT <hint_clauseOpt> <SELECT_Projection> <SELECT_FROM> <SELECT_WHEREopt> <SELECT_HierarchicalOpt> <SELECT_GROUPopt> <SELECT_HAVINGopt> */
void Rule_SELECT__SELECT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Projection> ::= <SELECT_ProjectionOpt1> <SELECT_ProjectionOpt2> <SELECT_ProjectionOpt3> <SELECT_ColumnS> */
void Rule_SELECT_Projection(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt1> ::= SKIP <Expression> */
void Rule_SELECT_ProjectionOpt1_SKIP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt1> ::=  */
void Rule_SELECT_ProjectionOpt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt2> ::= FIRST <Expression> */
void Rule_SELECT_ProjectionOpt2_FIRST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt2> ::= LIMIT <Expression> */
void Rule_SELECT_ProjectionOpt2_LIMIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt2> ::= MIDDLE <Expression> */
void Rule_SELECT_ProjectionOpt2_MIDDLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt2> ::=  */
void Rule_SELECT_ProjectionOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt3> ::= ALL */
void Rule_SELECT_ProjectionOpt3_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt3> ::= DISTINCT */
void Rule_SELECT_ProjectionOpt3_DISTINCT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt3> ::= UNIQUE */
void Rule_SELECT_ProjectionOpt3_UNIQUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ProjectionOpt3> ::=  */
void Rule_SELECT_ProjectionOpt3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ColumnS> ::= <SELECT_Column> <SELECT_ColumnS_> */
void Rule_SELECT_ColumnS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ColumnS_> ::= ',' <SELECT_Column> <SELECT_ColumnS_> */
void Rule_SELECT_ColumnS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ColumnS_> ::=  */
void Rule_SELECT_ColumnS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Column> ::= <Expression> AS Id */
void Rule_SELECT_Column_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Column> ::= <Expression> Id */
void Rule_SELECT_Column_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Column> ::= <Expression> */
void Rule_SELECT_Column(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Column> ::= Id '.*' */
void Rule_SELECT_Column_Id_DotTimes(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Column> ::= '*' */
void Rule_SELECT_Column_Times(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SETquery> ::= MULTISET '(' <SELECT> ')' */
void Rule_SETquery_MULTISET_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SETquery> ::= MULTISET '(' SELECT ITEM <SELECT> ')' */
void Rule_SETquery_MULTISET_LParan_SELECT_ITEM_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM> ::= FROM <SELECT_FROM_S> */
void Rule_SELECT_FROM_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM> ::= FROM <SELECT_FROM_ANSItableS> */
void Rule_SELECT_FROM_FROM2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_S> ::= <SELECT_FROM_> <SELECT_FROM_S_> */
void Rule_SELECT_FROM_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_S_> ::= ',' <SELECT_FROM_> <SELECT_FROM_S_> */
void Rule_SELECT_FROM_S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_S_> ::=  */
void Rule_SELECT_FROM_S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_> ::= <SELECT_FROM_table> ',' <SELECT_FROM_OUTERs> */
void Rule_SELECT_FROM__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_> ::= <SELECT_FROM_table> */
void Rule_SELECT_FROM_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_> ::= <SELECT_FROM_OUTERs> ',' <SELECT_FROM_tableS> */
void Rule_SELECT_FROM__Comma2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_table> ::= <SELECT_FROM_tableOpt1> <SELECT_FROM_tableOpt2> <SELECT_FROM_ANSItable> */
void Rule_SELECT_FROM_table(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_table> ::= <SELECT_FROM_tableOpt1> <SELECT_FROM_tableOpt2> <SELECT_FROM_ANSItable> '(' <Id List> ')' */
void Rule_SELECT_FROM_table_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_tableOpt1> ::= IntegerLiteral SAMPLES OF */
void Rule_SELECT_FROM_tableOpt1_IntegerLiteral_SAMPLES_OF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_tableOpt1> ::=  */
void Rule_SELECT_FROM_tableOpt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_tableOpt2> ::= LOCAL */
void Rule_SELECT_FROM_tableOpt2_LOCAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_tableOpt2> ::=  */
void Rule_SELECT_FROM_tableOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTERs> ::= <SELECT_FROM_OUTER> <SELECT_FROM_OUTERs_> */
void Rule_SELECT_FROM_OUTERs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTERs_> ::= ',' <SELECT_FROM_OUTER> <SELECT_FROM_OUTERs_> */
void Rule_SELECT_FROM_OUTERs__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTERs_> ::=  */
void Rule_SELECT_FROM_OUTERs_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTER> ::= OUTER <SELECT_FROM_table> */
void Rule_SELECT_FROM_OUTER_OUTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTER> ::= OUTER '(' <SELECT_FROM_OUTER_s> ')' */
void Rule_SELECT_FROM_OUTER_OUTER_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTER_s> ::= <SELECT_FROM_OUTER_> <SELECT_FROM_OUTER_s_> */
void Rule_SELECT_FROM_OUTER_s(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTER_s_> ::= ',' <SELECT_FROM_OUTER_> <SELECT_FROM_OUTER_s_> */
void Rule_SELECT_FROM_OUTER_s__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTER_s_> ::=  */
void Rule_SELECT_FROM_OUTER_s_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTER_> ::= <SELECT_FROM_tableS> <SELECT_FROM_OUTERs> */
void Rule_SELECT_FROM_OUTER_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_OUTER_> ::= <SELECT_FROM_OUTERs> <SELECT_FROM_tableS> */
void Rule_SELECT_FROM_OUTER_2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_tableS> ::= <SELECT_FROM_table> <SELECT_FROM_tableS_> */
void Rule_SELECT_FROM_tableS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_tableS_> ::= ',' <SELECT_FROM_table> <SELECT_FROM_tableS_> */
void Rule_SELECT_FROM_tableS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_tableS_> ::=  */
void Rule_SELECT_FROM_tableS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItableS> ::= <SELECT_FROM_ANSItable> <SELECT_FROM_ANSItableS_> */
void Rule_SELECT_FROM_ANSItableS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItableS_> ::= ',' <SELECT_FROM_ANSItable> <SELECT_FROM_ANSItableS_> */
void Rule_SELECT_FROM_ANSItableS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItableS_> ::=  */
void Rule_SELECT_FROM_ANSItableS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= Id AS Id */
void Rule_SELECT_FROM_ANSItable_Id_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= Id Id */
void Rule_SELECT_FROM_ANSItable_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= Id */
void Rule_SELECT_FROM_ANSItable_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= ONLY '(' Id ')' AS Id */
void Rule_SELECT_FROM_ANSItable_ONLY_LParan_Id_RParan_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= ONLY '(' Id ')' Id */
void Rule_SELECT_FROM_ANSItable_ONLY_LParan_Id_RParan_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= ONLY '(' Id ')' */
void Rule_SELECT_FROM_ANSItable_ONLY_LParan_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= <CollectionDerivedTable> */
void Rule_SELECT_FROM_ANSItable(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= <IteratorFunctionTable> */
void Rule_SELECT_FROM_ANSItable2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSItable> ::= <SELECT_FROM_ANSIjoin> */
void Rule_SELECT_FROM_ANSItable3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' AS Id '(' <Id List> ')' */
void Rule_CollectionDerivedTable_TABLE_LParan_RParan_AS_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' Id '(' <Id List> ')' */
void Rule_CollectionDerivedTable_TABLE_LParan_RParan_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' '(' <Id List> ')' */
void Rule_CollectionDerivedTable_TABLE_LParan_RParan_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' AS Id */
void Rule_CollectionDerivedTable_TABLE_LParan_RParan_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' Id */
void Rule_CollectionDerivedTable_TABLE_LParan_RParan_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' */
void Rule_CollectionDerivedTable_TABLE_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' AS Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' AS Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' AS Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' AS Id */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' AS Id */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' AS Id */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' Id */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' Id */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' Id */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' AS Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' AS Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' AS Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' AS Id */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' AS Id */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' AS Id */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' Id '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' Id */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' Id */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' Id */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' '(' <Id List> ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' */
void Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin> ::= <SELECT_FROM_ANSItable> <SELECT_FROM_ANSIjoin_S> */
void Rule_SELECT_FROM_ANSIjoin(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin> ::= '(' <SELECT_FROM_ANSIjoin> ')' */
void Rule_SELECT_FROM_ANSIjoin_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin_S> ::= <SELECT_FROM_ANSIjoin_> <SELECT_FROM_ANSIjoin_S> */
void Rule_SELECT_FROM_ANSIjoin_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin_S> ::= <SELECT_FROM_ANSIjoin_> */
void Rule_SELECT_FROM_ANSIjoin_S2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin_S> ::= CROSS JOIN <SELECT_FROM_ANSItable> */
void Rule_SELECT_FROM_ANSIjoin_S_CROSS_JOIN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin_> ::= INNER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
void Rule_SELECT_FROM_ANSIjoin__INNER_JOIN_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin_> ::= LEFT OUTER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
void Rule_SELECT_FROM_ANSIjoin__LEFT_OUTER_JOIN_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin_> ::= RIGHT OUTER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
void Rule_SELECT_FROM_ANSIjoin__RIGHT_OUTER_JOIN_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FROM_ANSIjoin_> ::= FULL OUTER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
void Rule_SELECT_FROM_ANSIjoin__FULL_OUTER_JOIN_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_WHEREopt> ::= WHERE <Expression> */
void Rule_SELECT_WHEREopt_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_WHEREopt> ::=  */
void Rule_SELECT_WHEREopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_HierarchicalOpt> ::= CONNECT BY NOCYCLE <SELECT_Hierarchical_CONNECT_BY_Expression> */
void Rule_SELECT_HierarchicalOpt_CONNECT_BY_NOCYCLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_HierarchicalOpt> ::= CONNECT BY <SELECT_Hierarchical_CONNECT_BY_Expression> */
void Rule_SELECT_HierarchicalOpt_CONNECT_BY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_HierarchicalOpt> ::= START WITH <Expression> CONNECT BY NOCYCLE <SELECT_Hierarchical_CONNECT_BY_Expression> */
void Rule_SELECT_HierarchicalOpt_START_WITH_CONNECT_BY_NOCYCLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_HierarchicalOpt> ::= START WITH <Expression> CONNECT BY <SELECT_Hierarchical_CONNECT_BY_Expression> */
void Rule_SELECT_HierarchicalOpt_START_WITH_CONNECT_BY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_HierarchicalOpt> ::=  */
void Rule_SELECT_HierarchicalOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Hierarchical_CONNECT_BY_Expression> ::= PRIOR <Expression> */
void Rule_SELECT_Hierarchical_CONNECT_BY_Expression_PRIOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_Hierarchical_CONNECT_BY_Expression> ::= <Expression> */
void Rule_SELECT_Hierarchical_CONNECT_BY_Expression(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_GROUPopt> ::= GROUP BY <Expr List> */
void Rule_SELECT_GROUPopt_GROUP_BY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_GROUPopt> ::=  */
void Rule_SELECT_GROUPopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_HAVINGopt> ::= HAVING <Expression> */
void Rule_SELECT_HAVINGopt_HAVING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_HAVINGopt> ::=  */
void Rule_SELECT_HAVINGopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDERopt> ::= ORDER BY <SELECT_ORDER_s> */
void Rule_SELECT_ORDERopt_ORDER_BY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDERopt> ::=  */
void Rule_SELECT_ORDERopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_s> ::= <SELECT_ORDER_> <SELECT_ORDER_s_> */
void Rule_SELECT_ORDER_s(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_s_> ::= ',' <SELECT_ORDER_> <SELECT_ORDER_s_> */
void Rule_SELECT_ORDER_s__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_s_> ::=  */
void Rule_SELECT_ORDER_s_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_> ::= <Expression> <SELECT_ORDER_opt1> ASC */
void Rule_SELECT_ORDER__ASC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_> ::= <Expression> <SELECT_ORDER_opt1> DESC */
void Rule_SELECT_ORDER__DESC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_> ::= <Expression> <SELECT_ORDER_opt1> */
void Rule_SELECT_ORDER_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_opt1> ::= '[' IntegerLiteral ',' IntegerLiteral ']' */
void Rule_SELECT_ORDER_opt1_LBracket_IntegerLiteral_Comma_IntegerLiteral_RBracket(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_ORDER_opt1> ::=  */
void Rule_SELECT_ORDER_opt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FORopt> ::= FOR READ ONLY */
void Rule_SELECT_FORopt_FOR_READ_ONLY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FORopt> ::= FOR UPDATE OF <Id List> */
void Rule_SELECT_FORopt_FOR_UPDATE_OF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FORopt> ::= FOR UPDATE */
void Rule_SELECT_FORopt_FOR_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_FORopt> ::=  */
void Rule_SELECT_FORopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_INTOTableOpt> ::= INTO TEMP Id WITH NO LOG */
void Rule_SELECT_INTOTableOpt_INTO_TEMP_Id_WITH_NO_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_INTOTableOpt> ::= INTO TEMP Id */
void Rule_SELECT_INTOTableOpt_INTO_TEMP_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_INTOTableOpt> ::= INTO <TableType> Id <CREATE TABLE_StorageOpt> <IndexLockmodeOpt> */
void Rule_SELECT_INTOTableOpt_INTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_INTOTableOpt> ::= INTO SCRATCH Id */
void Rule_SELECT_INTOTableOpt_INTO_SCRATCH_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT_INTOTableOpt> ::= INTO EXTERNAL Id USING <SELECT TABLE_optionS> DATAFILES <StringLiteralS> <SELECT TABLE_optionS> */
void Rule_SELECT_INTOTableOpt_INTO_EXTERNAL_Id_USING_DATAFILES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_optionS> ::= <SELECT TABLE_option> <SELECT TABLE_optionS_> */
void Rule_SELECTTABLE_optionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_optionS> ::=  */
void Rule_SELECTTABLE_optionS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_optionS_> ::= ',' <SELECT TABLE_option> <SELECT TABLE_optionS_> */
void Rule_SELECTTABLE_optionS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_optionS_> ::=  */
void Rule_SELECTTABLE_optionS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_option> ::= FORMAT StringLiteral */
void Rule_SELECTTABLE_option_FORMAT_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_option> ::= CODESET StringLiteral */
void Rule_SELECTTABLE_option_CODESET_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_option> ::= DELIMITER StringLiteral */
void Rule_SELECTTABLE_option_DELIMITER_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_option> ::= RECORDEND StringLiteral */
void Rule_SELECTTABLE_option_RECORDEND_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SELECT TABLE_option> ::= ESCAPE */
void Rule_SELECTTABLE_option_ESCAPE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRUNCATE> ::= TRUNCATE ONLY TABLE Id */
void Rule_TRUNCATE_TRUNCATE_ONLY_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRUNCATE> ::= TRUNCATE ONLY Id */
void Rule_TRUNCATE_TRUNCATE_ONLY_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRUNCATE> ::= TRUNCATE TABLE Id */
void Rule_TRUNCATE_TRUNCATE_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRUNCATE> ::= TRUNCATE Id */
void Rule_TRUNCATE_TRUNCATE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UNLOAD> ::= UNLOAD TO StringLiteral DELIMITER StringLiteral <SELECT> */
void Rule_UNLOAD_UNLOAD_TO_StringLiteral_DELIMITER_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UNLOAD> ::= UNLOAD TO StringLiteral <SELECT> */
void Rule_UNLOAD_UNLOAD_TO_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE> ::= UPDATE <hint_clauseOpt> Id <UPDATE_SET> <UPDATE_WHERE> */
void Rule_UPDATE_UPDATE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE> ::= UPDATE <hint_clauseOpt> ONLY '(' Id ')' <UPDATE_SET> <UPDATE_WHERE> */
void Rule_UPDATE_UPDATE_ONLY_LParan_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE> ::= UPDATE <CollectionDerivedTable> <UPDATE_SET> WHERE CURRENT OF Id */
void Rule_UPDATE_UPDATE_WHERE_CURRENT_OF_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE> ::= UPDATE <CollectionDerivedTable> <UPDATE_SET> */
void Rule_UPDATE_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_SET> ::= SET <Assign List> */
void Rule_UPDATE_SET_SET(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_SET> ::= SET <Id List> '=' '(' <Expr List> ')' */
void Rule_UPDATE_SET_SET_Eq_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_SET> ::= SET '*' '=' '(' <Expr List> ')' */
void Rule_UPDATE_SET_SET_Times_Eq_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_SET> ::= SET <Id List> '=' '(' <FuncCall> ')' */
void Rule_UPDATE_SET_SET_Eq_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_SET> ::= SET '*' '=' '(' <FuncCall> ')' */
void Rule_UPDATE_SET_SET_Times_Eq_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Assign List> ::= Id '=' <Expression> <Assign List_> */
void Rule_AssignList_Id_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Assign List_> ::= ',' Id '=' <Expression> <Assign List_> */
void Rule_AssignList__Comma_Id_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Assign List_> ::=  */
void Rule_AssignList_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <NamedParamS> ::= <NamedParam> <NamedParamS_> */
void Rule_NamedParamS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <NamedParamS_> ::= ',' <NamedParam> <NamedParamS_> */
void Rule_NamedParamS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <NamedParamS_> ::=  */
void Rule_NamedParamS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <NamedParam> ::= Id '=>' <Expression> */
void Rule_NamedParam_Id_EqGt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FuncCall> ::= Id '(' <Expr List> ')' */
void Rule_FuncCall_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FuncCall> ::= Id '(' ')' */
void Rule_FuncCall_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FuncCall> ::= CAST '(' <Expression> AS <Type> ')' */
void Rule_FuncCall_CAST_LParan_AS_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FuncCall> ::= Id '(' <NamedParamS> ')' */
void Rule_FuncCall_Id_LParan_RParan3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FuncCall> ::= Id '(' <SELECT> ')' */
void Rule_FuncCall_Id_LParan_RParan4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_WHERE> ::= <SELECT_FROM> WHERE <Expression> */
void Rule_UPDATE_WHERE_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_WHERE> ::= <SELECT_FROM> */
void Rule_UPDATE_WHERE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_WHERE> ::= WHERE <Expression> */
void Rule_UPDATE_WHERE_WHERE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_WHERE> ::=  */
void Rule_UPDATE_WHERE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE_WHERE> ::= WHERE CURRENT OF Id */
void Rule_UPDATE_WHERE_WHERE_CURRENT_OF_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CLOSE> ::= CLOSE Id */
void Rule_CLOSE_CLOSE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLARE> ::= DECLARE Id CURSOR <WITH HOLDopt> FOR <INSERT_byVALUES> */
void Rule_DECLARE_DECLARE_Id_CURSOR_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLARE> ::= DECLARE Id CURSOR <WITH HOLDopt> <DECLAREoption2> */
void Rule_DECLARE_DECLARE_Id_CURSOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLARE> ::= DECLARE Id CURSOR <WITH HOLDopt> <DECLAREoption3> */
void Rule_DECLARE_DECLARE_Id_CURSOR2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLARE> ::= DECLARE Id SCROLL CURSOR <WITH HOLDopt> <DECLAREoption3> */
void Rule_DECLARE_DECLARE_Id_SCROLL_CURSOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLARE> ::= DECLARE Id CURSOR FOR <SELECT> */
void Rule_DECLARE_DECLARE_Id_CURSOR_FOR2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLARE> ::= DECLARE Id CURSOR FOR <INSERT2> */
void Rule_DECLARE_DECLARE_Id_CURSOR_FOR3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WITH HOLDopt> ::= WITH HOLD */
void Rule_WITHHOLDopt_WITH_HOLD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WITH HOLDopt> ::=  */
void Rule_WITHHOLDopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLAREoption2> ::= <SELECT> FOR READ ONLY */
void Rule_DECLAREoption2_FOR_READ_ONLY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLAREoption2> ::= <SELECT> FOR UPDATE OF <Id List> */
void Rule_DECLAREoption2_FOR_UPDATE_OF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLAREoption2> ::= <SELECT> FOR UPDATE */
void Rule_DECLAREoption2_FOR_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLAREoption3> ::= FOR <SELECT> */
void Rule_DECLAREoption3_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLAREoption3> ::= FOR Id */
void Rule_DECLAREoption3_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLAREoption3> ::= FOR <EXECUTE PROCEDURE> */
void Rule_DECLAREoption3_FOR2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DECLAREoption3> ::= FOR <EXECUTE FUNCTION> */
void Rule_DECLAREoption3_FOR3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCH> ::= FETCH <FETCHopt1> Id <FETCHopt2> */
void Rule_FETCH_FETCH_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= NEXT */
void Rule_FETCHopt1_NEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= PRIOR */
void Rule_FETCHopt1_PRIOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= PREVIOUS */
void Rule_FETCHopt1_PREVIOUS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= FIRST */
void Rule_FETCHopt1_FIRST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= LAST */
void Rule_FETCHopt1_LAST(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= CURRENT */
void Rule_FETCHopt1_CURRENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= RELATIVE '+' IntegerLiteral */
void Rule_FETCHopt1_RELATIVE_Plus_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= RELATIVE '+' Id */
void Rule_FETCHopt1_RELATIVE_Plus_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= RELATIVE '-' IntegerLiteral */
void Rule_FETCHopt1_RELATIVE_Minus_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= RELATIVE '-' Id */
void Rule_FETCHopt1_RELATIVE_Minus_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= RELATIVE IntegerLiteral */
void Rule_FETCHopt1_RELATIVE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= RELATIVE Id */
void Rule_FETCHopt1_RELATIVE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= ABSOLUTE IntegerLiteral */
void Rule_FETCHopt1_ABSOLUTE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::= ABSOLUTE Id */
void Rule_FETCHopt1_ABSOLUTE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt1> ::=  */
void Rule_FETCHopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt2> ::= USING SQL DESCRIPTOR StringLiteral */
void Rule_FETCHopt2_USING_SQL_DESCRIPTOR_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt2> ::= USING SQL DESCRIPTOR Id */
void Rule_FETCHopt2_USING_SQL_DESCRIPTOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt2> ::= USING DESCRIPTOR Id */
void Rule_FETCHopt2_USING_DESCRIPTOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FETCHopt2> ::= <EXECUTE FUNCTION_intoS> */
void Rule_FETCHopt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FLUSH> ::= FLUSH Id */
void Rule_FLUSH_FLUSH_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FREE> ::= FREE Id */
void Rule_FREE_FREE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OPEN> ::= OPEN Id USING <Id List> <OPENopt1> */
void Rule_OPEN_OPEN_Id_USING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OPEN> ::= OPEN Id USING SQL DESCRIPTOR StringLiteral DESCRIPTOR Id <OPENopt1> */
void Rule_OPEN_OPEN_Id_USING_SQL_DESCRIPTOR_StringLiteral_DESCRIPTOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OPEN> ::= OPEN Id USING SQL DESCRIPTOR Id DESCRIPTOR Id <OPENopt1> */
void Rule_OPEN_OPEN_Id_USING_SQL_DESCRIPTOR_Id_DESCRIPTOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OPEN> ::= OPEN Id <OPENopt1> */
void Rule_OPEN_OPEN_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OPENopt1> ::= WITH REOPTIMIZATION */
void Rule_OPENopt1_WITH_REOPTIMIZATION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OPENopt1> ::=  */
void Rule_OPENopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PUT> ::= PUT Id FROM <EXECUTE FUNCTION_into> <EXECUTE FUNCTION_intoS_> */
void Rule_PUT_PUT_Id_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PUT> ::= PUT Id USING SQL DESCRIPTOR StringLiteral */
void Rule_PUT_PUT_Id_USING_SQL_DESCRIPTOR_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PUT> ::= PUT Id USING SQL DESCRIPTOR Id */
void Rule_PUT_PUT_Id_USING_SQL_DESCRIPTOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <PUT> ::= PUT Id USING DESCRIPTOR Id */
void Rule_PUT_PUT_Id_USING_DESCRIPTOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET AUTOFREE> ::= SET AUTOFREE */
void Rule_SETAUTOFREE_SET_AUTOFREE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE IMMEDIATE> ::= EXECUTE IMMEDIATE StringLiteral */
void Rule_EXECUTEIMMEDIATE_EXECUTE_IMMEDIATE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE IMMEDIATE> ::= EXECUTE IMMEDIATE Id */
void Rule_EXECUTEIMMEDIATE_EXECUTE_IMMEDIATE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO TABLES */
void Rule_INFO_INFO_TABLES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO COLUMNS FOR Id */
void Rule_INFO_INFO_COLUMNS_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO INDEXES FOR Id */
void Rule_INFO_INFO_INDEXES_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO STATUS FOR Id */
void Rule_INFO_INFO_STATUS_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO PRIVILEGES FOR Id */
void Rule_INFO_INFO_PRIVILEGES_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO ACCESS FOR Id */
void Rule_INFO_INFO_ACCESS_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO FRAGMENTS FOR Id */
void Rule_INFO_INFO_FRAGMENTS_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <INFO> ::= INFO REFERENCES FOR Id */
void Rule_INFO_INFO_REFERENCES_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT <db_privileges> <GRANT_TO1> */
void Rule_GRANT_GRANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT DEFAULT ROLE Id <GRANT_TO1> */
void Rule_GRANT_GRANT_DEFAULT_ROLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT DEFAULT ROLE StringLiteral <GRANT_TO1> */
void Rule_GRANT_GRANT_DEFAULT_ROLE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT Id <GRANT_TO2> */
void Rule_GRANT_GRANT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT <SecurityOption_GRANT> */
void Rule_GRANT_GRANT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT <tab_privileges_GRANT> <GRANT_TO2> */
void Rule_GRANT_GRANT3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT <routine_privileges> <GRANT_TO2> */
void Rule_GRANT_GRANT4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT <lang_privileges> <GRANT_TO2> */
void Rule_GRANT_GRANT5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT <type_privileges> <GRANT_TO2> */
void Rule_GRANT_GRANT6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT> ::= GRANT <seq_privileges> <GRANT_TO2> */
void Rule_GRANT_GRANT7(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO1> ::= TO PUBLIC */
void Rule_GRANT_TO1_TO_PUBLIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO1> ::= TO <Id List> */
void Rule_GRANT_TO1_TO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO1> ::= TO <StringLiteralS> */
void Rule_GRANT_TO1_TO2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO PUBLIC WITH GRANT OPTION AS Id */
void Rule_GRANT_TO2_TO_PUBLIC_WITH_GRANT_OPTION_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO <Id List> WITH GRANT OPTION AS Id */
void Rule_GRANT_TO2_TO_WITH_GRANT_OPTION_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO <StringLiteralS> WITH GRANT OPTION AS Id */
void Rule_GRANT_TO2_TO_WITH_GRANT_OPTION_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= <GRANT_TO1> AS Id */
void Rule_GRANT_TO2_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO <Id List> AS Id */
void Rule_GRANT_TO2_TO_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO <StringLiteralS> AS Id */
void Rule_GRANT_TO2_TO_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO PUBLIC WITH GRANT OPTION */
void Rule_GRANT_TO2_TO_PUBLIC_WITH_GRANT_OPTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO <Id List> WITH GRANT OPTION */
void Rule_GRANT_TO2_TO_WITH_GRANT_OPTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= TO <StringLiteralS> WITH GRANT OPTION */
void Rule_GRANT_TO2_TO_WITH_GRANT_OPTION2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT_TO2> ::= <GRANT_TO1> */
void Rule_GRANT_TO2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <db_privileges> ::= CONNECT */
void Rule_db_privileges_CONNECT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <db_privileges> ::= RESOURCE */
void Rule_db_privileges_RESOURCE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <db_privileges> ::= DBA */
void Rule_db_privileges_DBA(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_GRANT> ::= <DBSECADM_clause> TO <USERs1> */
void Rule_SecurityOption_GRANT_TO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_GRANT> ::= <EXEMPTION_clause> TO <USERs1> */
void Rule_SecurityOption_GRANT_TO2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_GRANT> ::= <SECURITY LABEL_clause> TO <USERs1> <SECURITY LABEL_clause_FOR> */
void Rule_SecurityOption_GRANT_TO3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_GRANT> ::= <SETSESSIONAUTH_clause> TO <USERs1> */
void Rule_SecurityOption_GRANT_TO4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_GRANT> ::= <SETSESSIONAUTH_clause> TO ROLE <Id List> */
void Rule_SecurityOption_GRANT_TO_ROLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <USERs1> ::= USER <Id List> */
void Rule_USERs1_USER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <USERs1> ::= <Id List> */
void Rule_USERs1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DBSECADM_clause> ::= DBSECADM */
void Rule_DBSECADM_clause_DBSECADM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause> ::= EXEMPTION ON RULE <EXEMPTION_clause_RULE> FOR Id */
void Rule_EXEMPTION_clause_EXEMPTION_ON_RULE_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLBACREADARRAY */
void Rule_EXEMPTION_clause_RULE_IDSLBACREADARRAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLBACREADTREE */
void Rule_EXEMPTION_clause_RULE_IDSLBACREADTREE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLBACREADSET */
void Rule_EXEMPTION_clause_RULE_IDSLBACREADSET(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLABCWRITEARRAY */
void Rule_EXEMPTION_clause_RULE_IDSLABCWRITEARRAY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLABCWRITEARRAY WRITEDOWN */
void Rule_EXEMPTION_clause_RULE_IDSLABCWRITEARRAY_WRITEDOWN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLABCWRITEARRAY WRITEUP */
void Rule_EXEMPTION_clause_RULE_IDSLABCWRITEARRAY_WRITEUP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLBACWRITESET */
void Rule_EXEMPTION_clause_RULE_IDSLBACWRITESET(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= IDSLBACWRITETREE */
void Rule_EXEMPTION_clause_RULE_IDSLBACWRITETREE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXEMPTION_clause_RULE> ::= ALL */
void Rule_EXEMPTION_clause_RULE_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL_clause> ::= SECURITY LABEL Id */
void Rule_SECURITYLABEL_clause_SECURITY_LABEL_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL_clause_FOR> ::= FOR ALL ACCESS */
void Rule_SECURITYLABEL_clause_FOR_FOR_ALL_ACCESS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL_clause_FOR> ::= FOR READ ACCESS */
void Rule_SECURITYLABEL_clause_FOR_FOR_READ_ACCESS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SECURITY LABEL_clause_FOR> ::= FOR WRITE ACCESS */
void Rule_SECURITYLABEL_clause_FOR_FOR_WRITE_ACCESS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SETSESSIONAUTH_clause> ::= SETSESSIONAUTH ON PUBLIC */
void Rule_SETSESSIONAUTH_clause_SETSESSIONAUTH_ON_PUBLIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SETSESSIONAUTH_clause> ::= SETSESSIONAUTH ON USER <Id List> */
void Rule_SETSESSIONAUTH_clause_SETSESSIONAUTH_ON_USER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SETSESSIONAUTH_clause> ::= SETSESSIONAUTH ON <Id List> */
void Rule_SETSESSIONAUTH_clause_SETSESSIONAUTH_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privileges_GRANT> ::= ALL PRIVILEGES ON Id */
void Rule_tab_privileges_GRANT_ALL_PRIVILEGES_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privileges_GRANT> ::= ALL ON Id */
void Rule_tab_privileges_GRANT_ALL_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privileges_GRANT> ::= <tab_privilege_GRANT> <tab_privilegeS__GRANT> ON Id */
void Rule_tab_privileges_GRANT_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilegeS__GRANT> ::= ',' <tab_privilege_GRANT> <tab_privilegeS__GRANT> */
void Rule_tab_privilegeS__GRANT_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilegeS__GRANT> ::=  */
void Rule_tab_privilegeS__GRANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= INSERT */
void Rule_tab_privilege_GRANT_INSERT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= DELETE */
void Rule_tab_privilege_GRANT_DELETE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= UPDATE '(' <Id List> ')' */
void Rule_tab_privilege_GRANT_UPDATE_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= SELECT '(' <Id List> ')' */
void Rule_tab_privilege_GRANT_SELECT_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= REFERENCES '(' <Id List> ')' */
void Rule_tab_privilege_GRANT_REFERENCES_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= UPDATE */
void Rule_tab_privilege_GRANT_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= SELECT */
void Rule_tab_privilege_GRANT_SELECT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= REFERENCES */
void Rule_tab_privilege_GRANT_REFERENCES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= ALTER */
void Rule_tab_privilege_GRANT_ALTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= INDEX */
void Rule_tab_privilege_GRANT_INDEX(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_GRANT> ::= UNDER */
void Rule_tab_privilege_GRANT_UNDER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <type_privileges> ::= USAGE ON TYPE Id */
void Rule_type_privileges_USAGE_ON_TYPE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <type_privileges> ::= UNDER ON TYPE Id */
void Rule_type_privileges_UNDER_ON_TYPE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON Id */
void Rule_routine_privileges_EXECUTE_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON FUNCTION Id '(' <RoutineParams> ')' */
void Rule_routine_privileges_EXECUTE_ON_FUNCTION_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON FUNCTION Id '(' ')' */
void Rule_routine_privileges_EXECUTE_ON_FUNCTION_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON PROCEDURE Id '(' <RoutineParams> ')' */
void Rule_routine_privileges_EXECUTE_ON_PROCEDURE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON PROCEDURE Id '(' ')' */
void Rule_routine_privileges_EXECUTE_ON_PROCEDURE_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON ROUTINE Id '(' <RoutineParams> ')' */
void Rule_routine_privileges_EXECUTE_ON_ROUTINE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON ROUTINE Id '(' ')' */
void Rule_routine_privileges_EXECUTE_ON_ROUTINE_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON SPECIFIC FUNCTION Id */
void Rule_routine_privileges_EXECUTE_ON_SPECIFIC_FUNCTION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON SPECIFIC PROCEDURE Id */
void Rule_routine_privileges_EXECUTE_ON_SPECIFIC_PROCEDURE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <routine_privileges> ::= EXECUTE ON SPECIFIC ROUTINE Id */
void Rule_routine_privileges_EXECUTE_ON_SPECIFIC_ROUTINE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <lang_privileges> ::= USAGE ON LANGUAGE SPL */
void Rule_lang_privileges_USAGE_ON_LANGUAGE_SPL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <seq_privileges> ::= SELECT ',' ALTER ON Id */
void Rule_seq_privileges_SELECT_Comma_ALTER_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <seq_privileges> ::= SELECT ON Id */
void Rule_seq_privileges_SELECT_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <seq_privileges> ::= ALTER ON Id */
void Rule_seq_privileges_ALTER_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <seq_privileges> ::= ALTER ',' SELECT ON Id */
void Rule_seq_privileges_ALTER_Comma_SELECT_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GRANT FRAGMENT> ::= GRANT FRAGMENT <fragment_privileges> ON Id '(' <Id List> ')' <GRANT_TO2> */
void Rule_GRANTFRAGMENT_GRANT_FRAGMENT_ON_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fragment_privileges> ::= ALL */
void Rule_fragment_privileges_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fragment_privileges> ::= <fragment_privilege> <fragment_privilegeS_> */
void Rule_fragment_privileges(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fragment_privilegeS_> ::= ',' <fragment_privilege> <fragment_privilegeS_> */
void Rule_fragment_privilegeS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fragment_privilegeS_> ::=  */
void Rule_fragment_privilegeS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fragment_privilege> ::= INSERT */
void Rule_fragment_privilege_INSERT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fragment_privilege> ::= DELETE */
void Rule_fragment_privilege_DELETE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <fragment_privilege> ::= UPDATE */
void Rule_fragment_privilege_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOCK TABLE> ::= LOCK TABLE Id IN SHARE MODE */
void Rule_LOCKTABLE_LOCK_TABLE_Id_IN_SHARE_MODE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOCK TABLE> ::= LOCK TABLE Id IN EXCLUSIVE MODE */
void Rule_LOCKTABLE_LOCK_TABLE_Id_IN_EXCLUSIVE_MODE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE <db_privileges> FROM <Id List> */
void Rule_REVOKE_REVOKE_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE DEFAULT ROLE FROM PUBLIC */
void Rule_REVOKE_REVOKE_DEFAULT_ROLE_FROM_PUBLIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE DEFAULT ROLE FROM <Id List> */
void Rule_REVOKE_REVOKE_DEFAULT_ROLE_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE DEFAULT ROLE FROM <StringLiteralS> */
void Rule_REVOKE_REVOKE_DEFAULT_ROLE_FROM2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE Id <REVOKE_FROM1> */
void Rule_REVOKE_REVOKE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE <SecurityOption_REVOKE> */
void Rule_REVOKE_REVOKE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE <tab_privileges_REVOKE> <REVOKE_FROM2> */
void Rule_REVOKE_REVOKE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE <routine_privileges> <REVOKE_FROM2> */
void Rule_REVOKE_REVOKE3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE <lang_privileges> <REVOKE_FROM2> */
void Rule_REVOKE_REVOKE4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE <type_privileges> <REVOKE_FROM2> */
void Rule_REVOKE_REVOKE5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE> ::= REVOKE <seq_privileges> <REVOKE_FROM2> */
void Rule_REVOKE_REVOKE6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM1> ::= FROM PUBLIC AS Id */
void Rule_REVOKE_FROM1_FROM_PUBLIC_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM1> ::= FROM <Id List> AS Id */
void Rule_REVOKE_FROM1_FROM_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM1> ::= FROM <StringLiteralS> AS Id */
void Rule_REVOKE_FROM1_FROM_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM1> ::= FROM PUBLIC */
void Rule_REVOKE_FROM1_FROM_PUBLIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM1> ::= FROM <Id List> */
void Rule_REVOKE_FROM1_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM1> ::= FROM <StringLiteralS> */
void Rule_REVOKE_FROM1_FROM2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM PUBLIC CASCADE AS Id */
void Rule_REVOKE_FROM2_FROM_PUBLIC_CASCADE_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM PUBLIC RESTRICT AS Id */
void Rule_REVOKE_FROM2_FROM_PUBLIC_RESTRICT_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <Id List> CASCADE AS Id */
void Rule_REVOKE_FROM2_FROM_CASCADE_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <StringLiteralS> CASCADE AS Id */
void Rule_REVOKE_FROM2_FROM_CASCADE_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <Id List> RESTRICT AS Id */
void Rule_REVOKE_FROM2_FROM_RESTRICT_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <StringLiteralS> RESTRICT AS Id */
void Rule_REVOKE_FROM2_FROM_RESTRICT_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <Id List> AS Id */
void Rule_REVOKE_FROM2_FROM_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <StringLiteralS> AS Id */
void Rule_REVOKE_FROM2_FROM_AS_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM PUBLIC CASCADE */
void Rule_REVOKE_FROM2_FROM_PUBLIC_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM PUBLIC RESTRICT */
void Rule_REVOKE_FROM2_FROM_PUBLIC_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <Id List> CASCADE */
void Rule_REVOKE_FROM2_FROM_CASCADE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <StringLiteralS> CASCADE */
void Rule_REVOKE_FROM2_FROM_CASCADE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <Id List> RESTRICT */
void Rule_REVOKE_FROM2_FROM_RESTRICT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <StringLiteralS> RESTRICT */
void Rule_REVOKE_FROM2_FROM_RESTRICT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <Id List> */
void Rule_REVOKE_FROM2_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE_FROM2> ::= FROM <StringLiteralS> */
void Rule_REVOKE_FROM2_FROM2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_REVOKE> ::= <DBSECADM_clause> FROM <USERs1> */
void Rule_SecurityOption_REVOKE_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_REVOKE> ::= <EXEMPTION_clause> FROM <USERs1> */
void Rule_SecurityOption_REVOKE_FROM2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_REVOKE> ::= <SECURITY LABEL_clause> FROM <USERs1> <SECURITY LABEL_clause_FOR> */
void Rule_SecurityOption_REVOKE_FROM3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_REVOKE> ::= <SETSESSIONAUTH_clause> FROM <USERs1> */
void Rule_SecurityOption_REVOKE_FROM4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SecurityOption_REVOKE> ::= <SETSESSIONAUTH_clause> FROM ROLE <Id List> */
void Rule_SecurityOption_REVOKE_FROM_ROLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privileges_REVOKE> ::= ALL PRIVILEGES ON Id */
void Rule_tab_privileges_REVOKE_ALL_PRIVILEGES_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privileges_REVOKE> ::= ALL ON Id */
void Rule_tab_privileges_REVOKE_ALL_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privileges_REVOKE> ::= <tab_privilege_REVOKE> <tab_privilegeS__REVOKE> ON Id */
void Rule_tab_privileges_REVOKE_ON_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilegeS__REVOKE> ::= ',' <tab_privilege_REVOKE> <tab_privilegeS__REVOKE> */
void Rule_tab_privilegeS__REVOKE_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilegeS__REVOKE> ::=  */
void Rule_tab_privilegeS__REVOKE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= INSERT */
void Rule_tab_privilege_REVOKE_INSERT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= DELETE */
void Rule_tab_privilege_REVOKE_DELETE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= UPDATE */
void Rule_tab_privilege_REVOKE_UPDATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= SELECT */
void Rule_tab_privilege_REVOKE_SELECT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= REFERENCES */
void Rule_tab_privilege_REVOKE_REFERENCES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= ALTER */
void Rule_tab_privilege_REVOKE_ALTER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= INDEX */
void Rule_tab_privilege_REVOKE_INDEX(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <tab_privilege_REVOKE> ::= UNDER */
void Rule_tab_privilege_REVOKE_UNDER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE FRAGMENT> ::= REVOKE FRAGMENT <fragment_privileges> ON Id '(' <Id List> ')' <REVOKE FRAGMENT_FROM2> */
void Rule_REVOKEFRAGMENT_REVOKE_FRAGMENT_ON_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE FRAGMENT_FROM2> ::= FROM PUBLIC AS Id */
void Rule_REVOKEFRAGMENT_FROM2_FROM_PUBLIC_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE FRAGMENT_FROM2> ::= FROM <Id List> AS Id */
void Rule_REVOKEFRAGMENT_FROM2_FROM_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE FRAGMENT_FROM2> ::= FROM PUBLIC */
void Rule_REVOKEFRAGMENT_FROM2_FROM_PUBLIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <REVOKE FRAGMENT_FROM2> ::= FROM <Id List> */
void Rule_REVOKEFRAGMENT_FROM2_FROM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION TO REPEATABLE READ */
void Rule_SETISOLATION_SET_ISOLATION_TO_REPEATABLE_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION TO COMMITTED READ LAST COMMITTED <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_TO_COMMITTED_READ_LAST_COMMITTED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION TO COMMITTED READ <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_TO_COMMITTED_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION TO CURSOR STABILITY <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_TO_CURSOR_STABILITY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION TO DIRTY READ WITH WARNING <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_TO_DIRTY_READ_WITH_WARNING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION TO DIRTY READ <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_TO_DIRTY_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION REPEATABLE READ */
void Rule_SETISOLATION_SET_ISOLATION_REPEATABLE_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION COMMITTED READ LAST COMMITTED <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_COMMITTED_READ_LAST_COMMITTED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION COMMITTED READ <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_COMMITTED_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION CURSOR STABILITY <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_CURSOR_STABILITY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION DIRTY READ WITH WARNING <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_DIRTY_READ_WITH_WARNING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION> ::= SET ISOLATION DIRTY READ <SET ISOLATION_Opt1> */
void Rule_SETISOLATION_SET_ISOLATION_DIRTY_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION_Opt1> ::= RETAIN UPDATE LOCKS */
void Rule_SETISOLATION_Opt1_RETAIN_UPDATE_LOCKS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ISOLATION_Opt1> ::=  */
void Rule_SETISOLATION_Opt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET LOCK MODE> ::= SET LOCK MODE TO NOWAIT */
void Rule_SETLOCKMODE_SET_LOCK_MODE_TO_NOWAIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET LOCK MODE> ::= SET LOCK MODE TO WAIT IntegerLiteral */
void Rule_SETLOCKMODE_SET_LOCK_MODE_TO_WAIT_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET LOCK MODE> ::= SET LOCK MODE TO WAIT */
void Rule_SETLOCKMODE_SET_LOCK_MODE_TO_WAIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ROLE> ::= SET ROLE Id */
void Rule_SETROLE_SET_ROLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ROLE> ::= SET ROLE StringLiteral */
void Rule_SETROLE_SET_ROLE_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ROLE> ::= SET ROLE NULL */
void Rule_SETROLE_SET_ROLE_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ROLE> ::= SET ROLE NONE */
void Rule_SETROLE_SET_ROLE_NONE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ROLE> ::= SET ROLE DEFAULT */
void Rule_SETROLE_SET_ROLE_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET SESSION AUTHORIZATION> ::= SET SESSION AUTHORIZATION TO Id */
void Rule_SETSESSIONAUTHORIZATION_SET_SESSION_AUTHORIZATION_TO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION> ::= SET TRANSACTION <SET TRANSACTION_optionS> */
void Rule_SETTRANSACTION_SET_TRANSACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_optionS> ::= <SET TRANSACTION_option> <SET TRANSACTION_optionS_> */
void Rule_SETTRANSACTION_optionS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_optionS_> ::= ',' <SET TRANSACTION_option> <SET TRANSACTION_optionS_> */
void Rule_SETTRANSACTION_optionS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_optionS_> ::=  */
void Rule_SETTRANSACTION_optionS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_option> ::= READ WRITE */
void Rule_SETTRANSACTION_option_READ_WRITE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_option> ::= READ ONLY */
void Rule_SETTRANSACTION_option_READ_ONLY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_option> ::= ISOLATION LEVEL READ COMMITTED */
void Rule_SETTRANSACTION_option_ISOLATION_LEVEL_READ_COMMITTED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_option> ::= ISOLATION LEVEL REPEATABLE READ */
void Rule_SETTRANSACTION_option_ISOLATION_LEVEL_REPEATABLE_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_option> ::= ISOLATION LEVEL SERIALIZABLE */
void Rule_SETTRANSACTION_option_ISOLATION_LEVEL_SERIALIZABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRANSACTION_option> ::= ISOLATION LEVEL READ UNCOMMITTED */
void Rule_SETTRANSACTION_option_ISOLATION_LEVEL_READ_UNCOMMITTED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UNLOCK TABLE> ::= UNLOCK TABLE Id */
void Rule_UNLOCKTABLE_UNLOCK_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <BEGIN WORK> ::= BEGIN WORK WITHOUT REPLICATION */
void Rule_BEGINWORK_BEGIN_WORK_WITHOUT_REPLICATION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <BEGIN WORK> ::= BEGIN WORK */
void Rule_BEGINWORK_BEGIN_WORK(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <BEGIN WORK> ::= BEGIN WITHOUT REPLICATION */
void Rule_BEGINWORK_BEGIN_WITHOUT_REPLICATION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <BEGIN WORK> ::= BEGIN */
void Rule_BEGINWORK_BEGIN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <COMMIT WORK> ::= COMMIT WORK */
void Rule_COMMITWORK_COMMIT_WORK(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <COMMIT WORK> ::= COMMIT */
void Rule_COMMITWORK_COMMIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ROLLBACK WORK> ::= ROLLBACK WORK */
void Rule_ROLLBACKWORK_ROLLBACK_WORK(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ROLLBACK WORK> ::= ROLLBACK */
void Rule_ROLLBACKWORK_ROLLBACK(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS ALL IMMEDIATE */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS_ALL_IMMEDIATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS ALL DEFERRED */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS_ALL_DEFERRED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> IMMEDIATE */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS_IMMEDIATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> DEFERRED */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS_DEFERRED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> <IndexMode> */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> DISABLED WITH ERROR */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS_DISABLED_WITH_ERROR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS FOR Id <IndexMode> */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONSTRAINTS> ::= SET CONSTRAINTS FOR Id DISABLED WITH ERROR */
void Rule_SETCONSTRAINTS_SET_CONSTRAINTS_FOR_Id_DISABLED_WITH_ERROR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET INDEXES> ::= SET INDEXES <Id List> <IndexMode> */
void Rule_SETINDEXES_SET_INDEXES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET INDEXES> ::= SET INDEXES FOR Id <IndexMode> */
void Rule_SETINDEXES_SET_INDEXES_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRIGGERS> ::= SET TRIGGERS <Id List> ENABLED */
void Rule_SETTRIGGERS_SET_TRIGGERS_ENABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRIGGERS> ::= SET TRIGGERS <Id List> DISABLED */
void Rule_SETTRIGGERS_SET_TRIGGERS_DISABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRIGGERS> ::= SET TRIGGERS FOR Id ENABLED */
void Rule_SETTRIGGERS_SET_TRIGGERS_FOR_Id_ENABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET TRIGGERS> ::= SET TRIGGERS FOR Id DISABLED */
void Rule_SETTRIGGERS_SET_TRIGGERS_FOR_Id_DISABLED(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET LOG> ::= SET LOG */
void Rule_SETLOG_SET_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET LOG> ::= SET BUFFERED LOG */
void Rule_SETLOG_SET_BUFFERED_LOG(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PLOAD FILE> ::= SET PLOAD FILE TO StringLiteral WITH APPEND */
void Rule_SETPLOADFILE_SET_PLOAD_FILE_TO_StringLiteral_WITH_APPEND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PLOAD FILE> ::= SET PLOAD FILE TO StringLiteral */
void Rule_SETPLOADFILE_SET_PLOAD_FILE_TO_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <START VIOLATIONS TABLE> ::= START VIOLATIONS TABLE FOR Id <START VIOLATIONS TABLE_opt1> */
void Rule_STARTVIOLATIONSTABLE_START_VIOLATIONS_TABLE_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <START VIOLATIONS TABLE> ::= START VIOLATIONS TABLE FOR Id USING Id <START VIOLATIONS TABLE_opt1> */
void Rule_STARTVIOLATIONSTABLE_START_VIOLATIONS_TABLE_FOR_Id_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <START VIOLATIONS TABLE> ::= START VIOLATIONS TABLE FOR Id USING Id ',' Id <START VIOLATIONS TABLE_opt1> */
void Rule_STARTVIOLATIONSTABLE_START_VIOLATIONS_TABLE_FOR_Id_USING_Id_Comma_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <START VIOLATIONS TABLE_opt1> ::= MAX ROWS IntegerLiteral */
void Rule_STARTVIOLATIONSTABLE_opt1_MAX_ROWS_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <START VIOLATIONS TABLE_opt1> ::= MAX VIOLATIONS IntegerLiteral */
void Rule_STARTVIOLATIONSTABLE_opt1_MAX_VIOLATIONS_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <STOP VIOLATIONS TABLE> ::= STOP VIOLATIONS TABLE FOR Id */
void Rule_STOPVIOLATIONSTABLE_STOP_VIOLATIONS_TABLE_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SAVE EXTERNAL DIRECTIVES> ::= SAVE EXTERNAL DIRECTIVES <Id List> ACTIVE FOR <SELECT> */
void Rule_SAVEEXTERNALDIRECTIVES_SAVE_EXTERNAL_DIRECTIVES_ACTIVE_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SAVE EXTERNAL DIRECTIVES> ::= SAVE EXTERNAL DIRECTIVES <Id List> INACTIVE FOR <SELECT> */
void Rule_SAVEEXTERNALDIRECTIVES_SAVE_EXTERNAL_DIRECTIVES_INACTIVE_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SAVE EXTERNAL DIRECTIVES> ::= SAVE EXTERNAL DIRECTIVES <Id List> TEST ONLY FOR <SELECT> */
void Rule_SAVEEXTERNALDIRECTIVES_SAVE_EXTERNAL_DIRECTIVES_TEST_ONLY_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SAVEPOINT> ::= SAVEPOINT Id UNIQUE */
void Rule_SAVEPOINT_SAVEPOINT_Id_UNIQUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SAVEPOINT> ::= SAVEPOINT Id */
void Rule_SAVEPOINT_SAVEPOINT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ALL_MUTABLES> ::= SET 'ALL_MUTABLES' TO MUTABLE */
void Rule_SETALL_MUTABLES_SET_ALL_MUTABLES_TO_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ALL_MUTABLES> ::= SET 'ALL_MUTABLES' TO IMMUTABLE */
void Rule_SETALL_MUTABLES_SET_ALL_MUTABLES_TO_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET 'TABLE_SPACE' TO <Id List> */
void Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET 'TABLE_SPACE' TO DEFAULT */
void Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET 'TABLE_SPACE' TO IMMUTABLE */
void Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET 'TABLE_SPACE' TO MUTABLE */
void Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO <Id List> */
void Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO DEFAULT */
void Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO IMMUTABLE */
void Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO MUTABLE */
void Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET 'TABLE_TYPE' TO STANDARD */
void Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_STANDARD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET 'TABLE_TYPE' TO OPERATIONAL */
void Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_OPERATIONAL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET 'TABLE_TYPE' TO RAW */
void Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_RAW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET 'TABLE_TYPE' TO STATIC */
void Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_STATIC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET 'TABLE_TYPE' TO IMMUTABLE */
void Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET 'TABLE_TYPE' TO MUTABLE */
void Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO SCRATCH */
void Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_SCRATCH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO DEFAULT */
void Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO IMMUTABLE */
void Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO MUTABLE */
void Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env1> <SET ENVIRONMENT_val1> */
void Rule_SETENVIRONMENT_SET_ENVIRONMENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env2> <SET ENVIRONMENT_val2> */
void Rule_SETENVIRONMENT_SET_ENVIRONMENT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env3val3> */
void Rule_SETENVIRONMENT_SET_ENVIRONMENT3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env4val4> */
void Rule_SETENVIRONMENT_SET_ENVIRONMENT4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env1> ::= 'BOUND_IMPL_PDQ' */
void Rule_SETENVIRONMENT_env1_BOUND_IMPL_PDQ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env1> ::= 'COMPUTE_QUOTA' */
void Rule_SETENVIRONMENT_env1_COMPUTE_QUOTA(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env1> ::= 'CLIENT_TZ' */
void Rule_SETENVIRONMENT_env1_CLIENT_TZ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env1> ::= 'IMPLICIT_PDQ' */
void Rule_SETENVIRONMENT_env1_IMPLICIT_PDQ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env1> ::= MAXSCAN */
void Rule_SETENVIRONMENT_env1_MAXSCAN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env1> ::= 'TMPSPACE_LIMIT' */
void Rule_SETENVIRONMENT_env1_TMPSPACE_LIMIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val1> ::= ON */
void Rule_SETENVIRONMENT_val1_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val1> ::= OFF */
void Rule_SETENVIRONMENT_val1_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val1> ::= StringLiteral */
void Rule_SETENVIRONMENT_val1_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val1> ::= DEFAULT */
void Rule_SETENVIRONMENT_val1_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val1> ::= MUTABLE */
void Rule_SETENVIRONMENT_val1_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val1> ::= IMMUTABLE */
void Rule_SETENVIRONMENT_val1_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env2> ::= 'TEMP_TAB_EXT_SIZE' */
void Rule_SETENVIRONMENT_env2_TEMP_TAB_EXT_SIZE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env2> ::= 'TEMP_TAB_NEXT_SIZE' */
void Rule_SETENVIRONMENT_env2_TEMP_TAB_NEXT_SIZE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env2> ::= OPTCOMPIND */
void Rule_SETENVIRONMENT_env2_OPTCOMPIND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val2> ::= DEFAULT */
void Rule_SETENVIRONMENT_val2_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val2> ::= IntegerLiteral */
void Rule_SETENVIRONMENT_val2_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val2> ::= MUTABLE */
void Rule_SETENVIRONMENT_val2_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_val2> ::= IMMUTABLE */
void Rule_SETENVIRONMENT_val2_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' OFF */
void Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' '0' */
void Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_0(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' ON */
void Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' '-1' */
void Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_Minus1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED ALL */
void Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED COMMITTED READ */
void Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_COMMITTED_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED DIRTY READ */
void Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_DIRTY_READ(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED NONE */
void Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_NONE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET EXPLAIN> ::= SET EXPLAIN OFF */
void Rule_SETEXPLAIN_SET_EXPLAIN_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET EXPLAIN> ::= SET EXPLAIN ON 'AVOID_EXECUTE' */
void Rule_SETEXPLAIN_SET_EXPLAIN_ON_AVOID_EXECUTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET EXPLAIN> ::= SET EXPLAIN ON */
void Rule_SETEXPLAIN_SET_EXPLAIN_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET EXPLAIN> ::= SET EXPLAIN ON FILE TO <Expression> WITH APPEND */
void Rule_SETEXPLAIN_SET_EXPLAIN_ON_FILE_TO_WITH_APPEND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET EXPLAIN> ::= SET EXPLAIN ON FILE TO <Expression> */
void Rule_SETEXPLAIN_SET_EXPLAIN_ON_FILE_TO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET OPTIMIZATION> ::= SET OPTIMIZATION HIGH */
void Rule_SETOPTIMIZATION_SET_OPTIMIZATION_HIGH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET OPTIMIZATION> ::= SET OPTIMIZATION LOW */
void Rule_SETOPTIMIZATION_SET_OPTIMIZATION_LOW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET OPTIMIZATION> ::= SET OPTIMIZATION 'FIRST_ROWS' */
void Rule_SETOPTIMIZATION_SET_OPTIMIZATION_FIRST_ROWS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET OPTIMIZATION> ::= SET OPTIMIZATION 'ALL_ROWS' */
void Rule_SETOPTIMIZATION_SET_OPTIMIZATION_ALL_ROWS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY DEFAULT */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY LOW */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_LOW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY OFF */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY HIGH */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_HIGH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY Id */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY LOW IntegerLiteral HIGH IntegerLiteral */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_LOW_IntegerLiteral_HIGH_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY MUTABLE */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_MUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET PDQPRIORITY> ::= SET PDQPRIORITY IMMUTABLE */
void Rule_SETPDQPRIORITY_SET_PDQPRIORITY_IMMUTABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET TABLE Id 'MEMORY_RESIDENT' */
void Rule_SETResidency_SET_TABLE_Id_MEMORY_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET TABLE Id 'NON_RESIDENT' */
void Rule_SETResidency_SET_TABLE_Id_NON_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET TABLE Id '(' <Id List> ')' 'MEMORY_RESIDENT' */
void Rule_SETResidency_SET_TABLE_Id_LParan_RParan_MEMORY_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET TABLE Id '(' <Id List> ')' 'NON_RESIDENT' */
void Rule_SETResidency_SET_TABLE_Id_LParan_RParan_NON_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET INDEX Id 'MEMORY_RESIDENT' */
void Rule_SETResidency_SET_INDEX_Id_MEMORY_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET INDEX Id 'NON_RESIDENT' */
void Rule_SETResidency_SET_INDEX_Id_NON_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET INDEX Id '(' <Id List> ')' 'MEMORY_RESIDENT' */
void Rule_SETResidency_SET_INDEX_Id_LParan_RParan_MEMORY_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET Residency> ::= SET INDEX Id '(' <Id List> ')' 'NON_RESIDENT' */
void Rule_SETResidency_SET_INDEX_Id_LParan_RParan_NON_RESIDENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET SCHEDULE LEVEL> ::= SET SCHEDULE LEVEL IntegerLiteral */
void Rule_SETSCHEDULELEVEL_SET_SCHEDULE_LEVEL_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET STATEMENT CACHE> ::= SET STATEMENT CACHE ON */
void Rule_SETSTATEMENTCACHE_SET_STATEMENT_CACHE_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET STATEMENT CACHE> ::= SET STATEMENT CACHE OFF */
void Rule_SETSTATEMENTCACHE_SET_STATEMENT_CACHE_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS> ::= UPDATE STATISTICS LOW <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt1> */
void Rule_UPDATESTATISTICS_UPDATE_STATISTICS_LOW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS> ::= UPDATE STATISTICS <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt1> */
void Rule_UPDATESTATISTICS_UPDATE_STATISTICS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS> ::= UPDATE STATISTICS MEDIUM <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt2> */
void Rule_UPDATESTATISTICS_UPDATE_STATISTICS_MEDIUM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS> ::= UPDATE STATISTICS HIGH <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt2> */
void Rule_UPDATESTATISTICS_UPDATE_STATISTICS_HIGH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS> ::= UPDATE STATISTICS <UPDATE STATISTICS_Routine Statistics> */
void Rule_UPDATESTATISTICS_UPDATE_STATISTICS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_table_columns> ::= FOR TABLE Id '(' <Id List> ')' */
void Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_table_columns> ::= FOR TABLE Id */
void Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_table_columns> ::= FOR TABLE ONLY '(' Id ')' '(' <Id List> ')' */
void Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_ONLY_LParan_Id_RParan_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_table_columns> ::= FOR TABLE ONLY '(' Id ')' */
void Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_ONLY_LParan_Id_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_table_columns> ::= FOR TABLE */
void Rule_UPDATESTATISTICS_table_columns_FOR_TABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_table_columns> ::=  */
void Rule_UPDATESTATISTICS_table_columns(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_opt1> ::= DROP DISTRIBUTIONS ONLY */
void Rule_UPDATESTATISTICS_opt1_DROP_DISTRIBUTIONS_ONLY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_opt1> ::= DROP DISTRIBUTIONS */
void Rule_UPDATESTATISTICS_opt1_DROP_DISTRIBUTIONS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_opt1> ::=  */
void Rule_UPDATESTATISTICS_opt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_opt2> ::= <Resolution_MEDIUM> */
void Rule_UPDATESTATISTICS_opt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_opt2> ::= <Resolution_HIGH> */
void Rule_UPDATESTATISTICS_opt22(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Resolution_MEDIUM> ::= SAMPLING SIZE IntegerLiteral RESOLUTION RealLiteral RealLiteral <DISTRIBUTIONS ONLYopt> */
void Rule_Resolution_MEDIUM_SAMPLING_SIZE_IntegerLiteral_RESOLUTION_RealLiteral_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Resolution_MEDIUM> ::= SAMPLING SIZE IntegerLiteral <DISTRIBUTIONS ONLYopt> */
void Rule_Resolution_MEDIUM_SAMPLING_SIZE_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Resolution_MEDIUM> ::= RESOLUTION RealLiteral RealLiteral <DISTRIBUTIONS ONLYopt> */
void Rule_Resolution_MEDIUM_RESOLUTION_RealLiteral_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Resolution_MEDIUM> ::= SAMPLING SIZE IntegerLiteral RESOLUTION RealLiteral <DISTRIBUTIONS ONLYopt> */
void Rule_Resolution_MEDIUM_SAMPLING_SIZE_IntegerLiteral_RESOLUTION_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Resolution_HIGH> ::= RESOLUTION RealLiteral <DISTRIBUTIONS ONLYopt> */
void Rule_Resolution_HIGH_RESOLUTION_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Resolution_HIGH> ::= <DISTRIBUTIONS ONLYopt> */
void Rule_Resolution_HIGH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISTRIBUTIONS ONLYopt> ::= DISTRIBUTIONS ONLY */
void Rule_DISTRIBUTIONSONLYopt_DISTRIBUTIONS_ONLY(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISTRIBUTIONS ONLYopt> ::=  */
void Rule_DISTRIBUTIONSONLYopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR PROCEDURE Id '(' <RoutineParams> ')' */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_PROCEDURE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR PROCEDURE Id */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_PROCEDURE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR PROCEDURE */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_PROCEDURE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR FUNCTION Id '(' <RoutineParams> ')' */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_FUNCTION_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR FUNCTION Id */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_FUNCTION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR FUNCTION */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_FUNCTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR ROUTINE Id '(' <RoutineParams> ')' */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_ROUTINE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR ROUTINE Id */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_ROUTINE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR ROUTINE */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_ROUTINE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR SPECIFIC PROCEDURE Id */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_SPECIFIC_PROCEDURE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR SPECIFIC FUNCTION Id */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_SPECIFIC_FUNCTION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <UPDATE STATISTICS_Routine Statistics> ::= FOR SPECIFIC ROUTINE Id */
void Rule_UPDATESTATISTICS_RoutineStatistics_FOR_SPECIFIC_ROUTINE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION> ::= EXECUTE FUNCTION Id '(' <RoutineParams2> ')' <EXECUTE FUNCTION_intoS> WITH TRIGGER REFERENCES */
void Rule_EXECUTEFUNCTION_EXECUTE_FUNCTION_Id_LParan_RParan_WITH_TRIGGER_REFERENCES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION> ::= EXECUTE FUNCTION Id '(' ')' <EXECUTE FUNCTION_intoS> WITH TRIGGER REFERENCES */
void Rule_EXECUTEFUNCTION_EXECUTE_FUNCTION_Id_LParan_RParan_WITH_TRIGGER_REFERENCES2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION> ::= <EXECUTE FUNCTION0> */
void Rule_EXECUTEFUNCTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION0> ::= EXECUTE FUNCTION Id '(' <RoutineParams2> ')' <EXECUTE FUNCTION_intoS> */
void Rule_EXECUTEFUNCTION0_EXECUTE_FUNCTION_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION0> ::= EXECUTE FUNCTION Id '(' ')' <EXECUTE FUNCTION_intoS> */
void Rule_EXECUTEFUNCTION0_EXECUTE_FUNCTION_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParams2> ::= <RoutineParam2> <RoutineParams2_> */
void Rule_RoutineParams2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParams2_> ::= ',' <RoutineParam2> <RoutineParams2_> */
void Rule_RoutineParams2__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParams2_> ::=  */
void Rule_RoutineParams2_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam2> ::= <Expression> */
void Rule_RoutineParam2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam2> ::= <SELECT> */
void Rule_RoutineParam22(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam2> ::= Id '=' <Expression> */
void Rule_RoutineParam2_Id_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RoutineParam2> ::= Id '=' <SELECT> */
void Rule_RoutineParam2_Id_Eq2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION_intoS> ::= INTO <EXECUTE FUNCTION_into> <EXECUTE FUNCTION_intoS_> */
void Rule_EXECUTEFUNCTION_intoS_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION_intoS_> ::= ',' <EXECUTE FUNCTION_into> <EXECUTE FUNCTION_intoS_> */
void Rule_EXECUTEFUNCTION_intoS__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION_intoS_> ::=  */
void Rule_EXECUTEFUNCTION_intoS_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION_into> ::= Id ':' Id */
void Rule_EXECUTEFUNCTION_into_Id_Colon_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION_into> ::= Id '$' Id */
void Rule_EXECUTEFUNCTION_into_Id_Dollar_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION_into> ::= Id INDICATOR Id */
void Rule_EXECUTEFUNCTION_into_Id_INDICATOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE FUNCTION_into> ::= Id */
void Rule_EXECUTEFUNCTION_into_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' INTO <Id List> WITH TRIGGER REFERENCES */
void Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO_WITH_TRIGGER_REFERENCES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' INTO <Id List> */
void Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' ')' INTO <Id List> WITH TRIGGER REFERENCES */
void Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO_WITH_TRIGGER_REFERENCES2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' ')' INTO <Id List> */
void Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' WITH TRIGGER REFERENCES */
void Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_WITH_TRIGGER_REFERENCES(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' ')' WITH TRIGGER REFERENCES */
void Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_WITH_TRIGGER_REFERENCES2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE> ::= <EXECUTE PROCEDURE0> */
void Rule_EXECUTEPROCEDURE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE0> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' */
void Rule_EXECUTEPROCEDURE0_EXECUTE_PROCEDURE_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXECUTE PROCEDURE0> ::= EXECUTE PROCEDURE Id '(' ')' */
void Rule_EXECUTEPROCEDURE0_EXECUTE_PROCEDURE_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET DEBUG FILE TO> ::= SET DEBUG FILE TO <Expression> WITH APPEND */
void Rule_SETDEBUGFILETO_SET_DEBUG_FILE_TO_WITH_APPEND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET DEBUG FILE TO> ::= SET DEBUG FILE TO <Expression> */
void Rule_SETDEBUGFILETO_SET_DEBUG_FILE_TO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS> ::= GET DIAGNOSTICS <GET DIAGNOSTICS_option1S> */
void Rule_GETDIAGNOSTICS_GET_DIAGNOSTICS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS> ::= GET DIAGNOSTICS EXCEPTION IntegerLiteral <GET DIAGNOSTICS_option2S> */
void Rule_GETDIAGNOSTICS_GET_DIAGNOSTICS_EXCEPTION_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS> ::= GET DIAGNOSTICS EXCEPTION Id <GET DIAGNOSTICS_option2S> */
void Rule_GETDIAGNOSTICS_GET_DIAGNOSTICS_EXCEPTION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option1S> ::= <GET DIAGNOSTICS_option1> <GET DIAGNOSTICS_option1S_> */
void Rule_GETDIAGNOSTICS_option1S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option1S_> ::= ',' <GET DIAGNOSTICS_option1> <GET DIAGNOSTICS_option1S_> */
void Rule_GETDIAGNOSTICS_option1S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option1S_> ::=  */
void Rule_GETDIAGNOSTICS_option1S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option1> ::= Id '=' 'ROW_COUNT' */
void Rule_GETDIAGNOSTICS_option1_Id_Eq_ROW_COUNT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option1> ::= Id '=' NUMBER */
void Rule_GETDIAGNOSTICS_option1_Id_Eq_NUMBER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option1> ::= Id '=' MORE */
void Rule_GETDIAGNOSTICS_option1_Id_Eq_MORE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2S> ::= <GET DIAGNOSTICS_option2> <GET DIAGNOSTICS_option2S_> */
void Rule_GETDIAGNOSTICS_option2S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2S_> ::= ',' <GET DIAGNOSTICS_option2> <GET DIAGNOSTICS_option2S_> */
void Rule_GETDIAGNOSTICS_option2S__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2S_> ::=  */
void Rule_GETDIAGNOSTICS_option2S_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2> ::= Id '=' 'CLASS_ORIGIN' */
void Rule_GETDIAGNOSTICS_option2_Id_Eq_CLASS_ORIGIN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2> ::= Id '=' 'CONNECTION_NAME' */
void Rule_GETDIAGNOSTICS_option2_Id_Eq_CONNECTION_NAME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2> ::= Id '=' 'MESSAGE_LENGTH' */
void Rule_GETDIAGNOSTICS_option2_Id_Eq_MESSAGE_LENGTH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2> ::= Id '=' 'MESSAGE_TEXT' */
void Rule_GETDIAGNOSTICS_option2_Id_Eq_MESSAGE_TEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2> ::= Id '=' 'RETURNED_SQLSTATE' */
void Rule_GETDIAGNOSTICS_option2_Id_Eq_RETURNED_SQLSTATE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2> ::= Id '=' 'SERVER_NAME' */
void Rule_GETDIAGNOSTICS_option2_Id_Eq_SERVER_NAME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GET DIAGNOSTICS_option2> ::= Id '=' 'SUBCLASS_ORIGIN' */
void Rule_GETDIAGNOSTICS_option2_Id_Eq_SUBCLASS_ORIGIN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OUTPUT> ::= OUTPUT StringLiteral WITHOUT HEADINGS <SELECT> */
void Rule_OUTPUT_OUTPUT_StringLiteral_WITHOUT_HEADINGS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OUTPUT> ::= OUTPUT StringLiteral <SELECT> */
void Rule_OUTPUT_OUTPUT_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OUTPUT> ::= OUTPUT PIPE Id WITHOUT HEADINGS <SELECT> */
void Rule_OUTPUT_OUTPUT_PIPE_Id_WITHOUT_HEADINGS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <OUTPUT> ::= OUTPUT PIPE Id <SELECT> */
void Rule_OUTPUT_OUTPUT_PIPE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET COLLATION> ::= SET COLLATION Id */
void Rule_SETCOLLATION_SET_COLLATION_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET COLLATION> ::= SET NO COLLATION */
void Rule_SETCOLLATION_SET_NO_COLLATION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET DATASKIP> ::= SET DATASKIP ON <Id List> */
void Rule_SETDATASKIP_SET_DATASKIP_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET DATASKIP> ::= SET DATASKIP ON */
void Rule_SETDATASKIP_SET_DATASKIP_ON2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET DATASKIP> ::= SET DATASKIP OFF */
void Rule_SETDATASKIP_SET_DATASKIP_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET DATASKIP> ::= SET DATASKIP DEFAULT */
void Rule_SETDATASKIP_SET_DATASKIP_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENCRYPTION PASSWORD> ::= SET ENCRYPTION PASSWORD StringLiteral WITH HINT StringLiteral */
void Rule_SETENCRYPTIONPASSWORD_SET_ENCRYPTION_PASSWORD_StringLiteral_WITH_HINT_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET ENCRYPTION PASSWORD> ::= SET ENCRYPTION PASSWORD StringLiteral */
void Rule_SETENCRYPTIONPASSWORD_SET_ENCRYPTION_PASSWORD_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER> ::= WHENEVER <WHENEVER_cond> <WHENEVER_action> */
void Rule_WHENEVER_WHENEVER(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_cond> ::= SQLERROR */
void Rule_WHENEVER_cond_SQLERROR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_cond> ::= NOT FOUND */
void Rule_WHENEVER_cond_NOT_FOUND(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_cond> ::= SQLWARNING */
void Rule_WHENEVER_cond_SQLWARNING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_cond> ::= ERROR */
void Rule_WHENEVER_cond_ERROR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_action> ::= CONTINUE */
void Rule_WHENEVER_action_CONTINUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_action> ::= GOTO Id */
void Rule_WHENEVER_action_GOTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_action> ::= CALL Id */
void Rule_WHENEVER_action_CALL_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHENEVER_action> ::= STOP */
void Rule_WHENEVER_action_STOP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO StringLiteral AS StringLiteral USER StringLiteral USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_StringLiteral_AS_StringLiteral_USER_StringLiteral_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO StringLiteral AS Id USER StringLiteral USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_StringLiteral_AS_Id_USER_StringLiteral_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO Id AS StringLiteral USER StringLiteral USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_Id_AS_StringLiteral_USER_StringLiteral_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO Id AS Id USER StringLiteral USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_Id_AS_Id_USER_StringLiteral_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO StringLiteral AS StringLiteral USER Id USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_StringLiteral_AS_StringLiteral_USER_Id_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO StringLiteral AS Id USER Id USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_StringLiteral_AS_Id_USER_Id_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO Id AS StringLiteral USER Id USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_Id_AS_StringLiteral_USER_Id_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO Id AS Id USER Id USING Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_Id_AS_Id_USER_Id_USING_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO StringLiteral AS StringLiteral <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_StringLiteral_AS_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO StringLiteral AS Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_StringLiteral_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO Id AS StringLiteral <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_Id_AS_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO Id AS Id <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_Id_AS_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECT> ::= CONNECT TO DEFAULT <CONNECTopt1> */
void Rule_CONNECT_CONNECT_TO_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECTopt1> ::= WITH CONCURRENT TRANSACTION */
void Rule_CONNECTopt1_WITH_CONCURRENT_TRANSACTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONNECTopt1> ::=  */
void Rule_CONNECTopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATABASE> ::= DATABASE Id EXCLUSIVE */
void Rule_DATABASE_DATABASE_Id_EXCLUSIVE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DATABASE> ::= DATABASE Id */
void Rule_DATABASE_DATABASE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISCONNECT> ::= DISCONNECT CURRENT */
void Rule_DISCONNECT_DISCONNECT_CURRENT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISCONNECT> ::= DISCONNECT ALL */
void Rule_DISCONNECT_DISCONNECT_ALL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISCONNECT> ::= DISCONNECT DEFAULT */
void Rule_DISCONNECT_DISCONNECT_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISCONNECT> ::= DISCONNECT StringLiteral */
void Rule_DISCONNECT_DISCONNECT_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISCONNECT> ::= DISCONNECT Id */
void Rule_DISCONNECT_DISCONNECT_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONNECTION> ::= SET CONNECTION StringLiteral DORMANT */
void Rule_SETCONNECTION_SET_CONNECTION_StringLiteral_DORMANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONNECTION> ::= SET CONNECTION Id DORMANT */
void Rule_SETCONNECTION_SET_CONNECTION_Id_DORMANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONNECTION> ::= SET CONNECTION DEFAULT DORMANT */
void Rule_SETCONNECTION_SET_CONNECTION_DEFAULT_DORMANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SET CONNECTION> ::= SET CONNECTION CURRENT DORMANT */
void Rule_SETCONNECTION_SET_CONNECTION_CURRENT_DORMANT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Label> ::= '<<' Id '>>' */
void Rule_Label_LtLt_Id_GtGt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CALL> ::= CALL Id '(' <Expr List> ')' */
void Rule_CALL_CALL_Id_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CALL> ::= CALL Id '(' ')' */
void Rule_CALL_CALL_Id_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CALL> ::= CALL Id '(' <Expr List> ')' RETURNING <Id List> */
void Rule_CALL_CALL_Id_LParan_RParan_RETURNING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CALL> ::= CALL Id '(' ')' RETURNING <Id List> */
void Rule_CALL_CALL_Id_LParan_RParan_RETURNING2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CALL> ::= CALL Id RETURNING <Id List> */
void Rule_CALL_CALL_Id_RETURNING(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CALL> ::= CALL Id */
void Rule_CALL_CALL_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE> ::= CASE <CASE_WHEN_THENs2> ELSE <SQLBlock> END */
void Rule_CASE_CASE_ELSE_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE> ::= CASE <CASE_WHEN_THENs2> END */
void Rule_CASE_CASE_END(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE> ::= CASE <Expression> <CASE_WHEN_THENs2> ELSE <SQLBlock> END */
void Rule_CASE_CASE_ELSE_END2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE> ::= CASE <Expression> <CASE_WHEN_THENs2> END */
void Rule_CASE_CASE_END2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE_WHEN_THENs2> ::= WHEN <Expression> THEN <SQLBlock> <CASE_WHEN_THENs2> */
void Rule_CASE_WHEN_THENs2_WHEN_THEN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CASE_WHEN_THENs2> ::= WHEN <Expression> THEN <SQLBlock> */
void Rule_CASE_WHEN_THENs2_WHEN_THEN2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONTINUE> ::= CONTINUE */
void Rule_CONTINUE_CONTINUE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONTINUE> ::= CONTINUE FOR */
void Rule_CONTINUE_CONTINUE_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONTINUE> ::= CONTINUE FOREACH */
void Rule_CONTINUE_CONTINUE_FOREACH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONTINUE> ::= CONTINUE LOOP */
void Rule_CONTINUE_CONTINUE_LOOP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <CONTINUE> ::= CONTINUE WHILE */
void Rule_CONTINUE_CONTINUE_WHILE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE GLOBAL <Id List> <Type> DEFAULT <Expression> */
void Rule_DEFINE_DEFINE_GLOBAL_DEFAULT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE GLOBAL <Id List> REFERENCES BYTE DEFAULT NULL */
void Rule_DEFINE_DEFINE_GLOBAL_REFERENCES_BYTE_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE GLOBAL <Id List> REFERENCES TEXT DEFAULT NULL */
void Rule_DEFINE_DEFINE_GLOBAL_REFERENCES_TEXT_DEFAULT_NULL(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE <Id List> <Type> */
void Rule_DEFINE_DEFINE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE <Id List> REFERENCES BYTE */
void Rule_DEFINE_DEFINE_REFERENCES_BYTE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE <Id List> REFERENCES TEXT */
void Rule_DEFINE_DEFINE_REFERENCES_TEXT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE <Id List> LIKE Id */
void Rule_DEFINE_DEFINE_LIKE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DEFINE> ::= DEFINE <Id List> PROCEDURE */
void Rule_DEFINE_DEFINE_PROCEDURE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXIT> ::= EXIT FOREACH */
void Rule_EXIT_EXIT_FOREACH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXIT> ::= EXIT FOR <EXITopt1> */
void Rule_EXIT_EXIT_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXIT> ::= EXIT LOOP <EXITopt1> */
void Rule_EXIT_EXIT_LOOP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXIT> ::= EXIT WHILE <EXITopt1> */
void Rule_EXIT_EXIT_WHILE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXITopt1> ::= Id WHEN <Expression> */
void Rule_EXITopt1_Id_WHEN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXITopt1> ::= WHEN <Expression> */
void Rule_EXITopt1_WHEN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <EXITopt1> ::=  */
void Rule_EXITopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= <Label> FOR Id IN '(' <Expression> TO <Expression> STEP <Expression> ')' <FORbody> */
void Rule_FOR_FOR_Id_IN_LParan_TO_STEP_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= <Label> FOR Id IN '(' <Expression> TO <Expression> ')' <FORbody> */
void Rule_FOR_FOR_Id_IN_LParan_TO_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= <Label> FOR Id IN '(' <Expr List> ')' <FORbody> */
void Rule_FOR_FOR_Id_IN_LParan_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= <Label> FOR Id '=' <Expression> TO <Expression> STEP <Expression> <FORbody> */
void Rule_FOR_FOR_Id_Eq_TO_STEP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= <Label> FOR Id '=' <Expression> TO <Expression> <FORbody> */
void Rule_FOR_FOR_Id_Eq_TO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= FOR Id IN '(' <Expression> TO <Expression> STEP <Expression> ')' <FORbody> */
void Rule_FOR_FOR_Id_IN_LParan_TO_STEP_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= FOR Id IN '(' <Expression> TO <Expression> ')' <FORbody> */
void Rule_FOR_FOR_Id_IN_LParan_TO_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= FOR Id IN '(' <Expr List> ')' <FORbody> */
void Rule_FOR_FOR_Id_IN_LParan_RParan2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= FOR Id '=' <Expression> TO <Expression> STEP <Expression> <FORbody> */
void Rule_FOR_FOR_Id_Eq_TO_STEP2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOR> ::= FOR Id '=' <Expression> TO <Expression> <FORbody> */
void Rule_FOR_FOR_Id_Eq_TO2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FORbody> ::= <SQLBlock> END FOR Id */
void Rule_FORbody_END_FOR_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FORbody> ::= <SQLBlock> END FOR */
void Rule_FORbody_END_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FORbody> ::= <LOOPbody> */
void Rule_FORbody(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOOPbody> ::= LOOP <SQLBlock> END LOOP Id */
void Rule_LOOPbody_LOOP_END_LOOP_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LOOPbody> ::= LOOP <SQLBlock> END LOOP */
void Rule_LOOPbody_LOOP_END_LOOP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOREACH> ::= FOREACH <FOREACHopt1> <SELECTinto> <SQLBlock> END FOR EACH */
void Rule_FOREACH_FOREACH_END_FOR_EACH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOREACHopt1> ::= WITH HOLD */
void Rule_FOREACHopt1_WITH_HOLD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOREACHopt1> ::= Id WITH HOLD FOR */
void Rule_FOREACHopt1_Id_WITH_HOLD_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOREACHopt1> ::= Id FOR */
void Rule_FOREACHopt1_Id_FOR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOREACHopt1> ::= <INSERT_EXECUTE> INTO <Id List> */
void Rule_FOREACHopt1_INTO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <FOREACHopt1> ::= <INSERT_EXECUTE> */
void Rule_FOREACHopt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <GOTO> ::= GOTO Id */
void Rule_GOTO_GOTO_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF> ::= IF <Expression> THEN <SQLBlock2> <IF_ELIFs> ELSE <SQLBlock2> END IF */
void Rule_IF_IF_THEN_ELSE_END_IF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF> ::= IF <Expression> THEN <SQLBlock2> <IF_ELIFs> END IF */
void Rule_IF_IF_THEN_END_IF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF> ::= IF <Expression> THEN <IF_ELIFs> ELSE <SQLBlock2> END IF */
void Rule_IF_IF_THEN_ELSE_END_IF2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF> ::= IF <Expression> THEN <IF_ELIFs> END IF */
void Rule_IF_IF_THEN_END_IF2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF_ELIFs> ::= <IF_ELIF> <IF_ELIFs> */
void Rule_IF_ELIFs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF_ELIFs> ::=  */
void Rule_IF_ELIFs2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF_ELIF> ::= ELIF <Expression> THEN <SQLBlock2> */
void Rule_IF_ELIF_ELIF_THEN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <IF_ELIF> ::= ELIF <Expression> THEN */
void Rule_IF_ELIF_ELIF_THEN2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SQLBlock2> ::= <SQLBlock> */
void Rule_SQLBlock2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <LET> ::= LET <Id List> '=' <Expr List> */
void Rule_LET_LET_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ON EXCEPTION> ::= ON EXCEPTION IN '(' <Expr List> ')' <ON EXCEPTION_SETopt> <SQLBlock> END EXCEPTION <WITH RESUMEopt> */
void Rule_ONEXCEPTION_ON_EXCEPTION_IN_LParan_RParan_END_EXCEPTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ON EXCEPTION> ::= ON EXCEPTION <ON EXCEPTION_SETopt> <SQLBlock> END EXCEPTION <WITH RESUMEopt> */
void Rule_ONEXCEPTION_ON_EXCEPTION_END_EXCEPTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ON EXCEPTION_SETopt> ::= SET Id ',' Id */
void Rule_ONEXCEPTION_SETopt_SET_Id_Comma_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ON EXCEPTION_SETopt> ::= SET Id */
void Rule_ONEXCEPTION_SETopt_SET_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ON EXCEPTION_SETopt> ::=  */
void Rule_ONEXCEPTION_SETopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WITH RESUMEopt> ::= WITH RESUME */
void Rule_WITHRESUMEopt_WITH_RESUME(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WITH RESUMEopt> ::=  */
void Rule_WITHRESUMEopt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RAISE EXCEPTION> ::= RAISE EXCEPTION <Expression> ',' <Expression> ',' <Expression> */
void Rule_RAISEEXCEPTION_RAISE_EXCEPTION_Comma_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RAISE EXCEPTION> ::= RAISE EXCEPTION <Expression> ',' <Expression> */
void Rule_RAISEEXCEPTION_RAISE_EXCEPTION_Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RAISE EXCEPTION> ::= RAISE EXCEPTION <Expression> */
void Rule_RAISEEXCEPTION_RAISE_EXCEPTION(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RETURN> ::= RETURN <Expr List> <WITH RESUMEopt> */
void Rule_RETURN_RETURN(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <RETURN> ::= RETURN */
void Rule_RETURN_RETURN2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SYSTEM> ::= SYSTEM <Expression> */
void Rule_SYSTEM_SYSTEM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRACE> ::= TRACE ON */
void Rule_TRACE_TRACE_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRACE> ::= TRACE OFF */
void Rule_TRACE_TRACE_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRACE> ::= TRACE PROCEDURE */
void Rule_TRACE_TRACE_PROCEDURE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <TRACE> ::= TRACE <Expression> */
void Rule_TRACE_TRACE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHILE> ::= <Label> WHILE <Expression> <WHILEbody> */
void Rule_WHILE_WHILE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHILE> ::= WHILE <Expression> <WHILEbody> */
void Rule_WHILE_WHILE2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHILEbody> ::= <SQLBlock> END WHILE Id */
void Rule_WHILEbody_END_WHILE_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHILEbody> ::= <SQLBlock> END WHILE */
void Rule_WHILEbody_END_WHILE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <WHILEbody> ::= <LOOPbody> */
void Rule_WHILEbody(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -b <onbarOpt1> <onbarOpt2> <onbarOpt3> <onbarOpt4> <onbarOpt5> */
void Rule_onbar_onbar_Minusb(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -m IntegerLiteral -r IntegerLiteral */
void Rule_onbar_onbar_Minusm_IntegerLiteral_Minusr_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -m IntegerLiteral */
void Rule_onbar_onbar_Minusm_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -m -r IntegerLiteral */
void Rule_onbar_onbar_Minusm_Minusr_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -m IntegerLiteral -r */
void Rule_onbar_onbar_Minusm_IntegerLiteral_Minusr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -m -r */
void Rule_onbar_onbar_Minusm_Minusr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -b -l '-C2' <onbarOpt5a2> */
void Rule_onbar_onbar_Minusb_Minusl_MinusC2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -b -l -c <onbarOpt5a2> */
void Rule_onbar_onbar_Minusb_Minusl_Minusc(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -b -l -s <onbarOpt5a2> */
void Rule_onbar_onbar_Minusb_Minusl_Minuss(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -b -l <onbarOpt5a2> */
void Rule_onbar_onbar_Minusb_Minusl(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -P -n <onbarOpt6> <onbarOpt7> <onbarOpt8> -b <onbarOpt9> <onbarOpt10> <onbarOpt11> */
void Rule_onbar_onbar_MinusP_Minusn_Minusb(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -P -n <onbarOpt6> <onbarOpt7> <onbarOpt8> <onbarOpt9> <onbarOpt10> <onbarOpt11> */
void Rule_onbar_onbar_MinusP_Minusn(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -v -t StringLiteral <onbarOpt2> <onbarOpt5a1> <onbarOpt5a3> */
void Rule_onbar_onbar_Minusv_Minust_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -v <onbarOpt2> <onbarOpt5a1> <onbarOpt5a3> */
void Rule_onbar_onbar_Minusv(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -r <onbarOpt12> <onbarOpt13> */
void Rule_onbar_onbar_Minusr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -RESTART */
void Rule_onbar_onbar_MinusRESTART(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -r -e <onbarOpt14> <onbarOpt2> -O <onbarOpt5a3_> */
void Rule_onbar_onbar_Minusr_Minuse_MinusO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar -r -e <onbarOpt14> <onbarOpt2> <onbarOpt5a3_> */
void Rule_onbar_onbar_Minusr_Minuse(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar <onbarOpt15> -q Id */
void Rule_onbar_onbar_Minusq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbar> ::= onbar <onbarOpt15> Id */
void Rule_onbar_onbar_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt1> ::= -P */
void Rule_onbarOpt1_MinusP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt1> ::=  */
void Rule_onbarOpt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt2> ::= -q Id */
void Rule_onbarOpt2_Minusq_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt2> ::=  */
void Rule_onbarOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt3> ::= -s */
void Rule_onbarOpt3_Minuss(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt3> ::=  */
void Rule_onbarOpt3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt4> ::= -v */
void Rule_onbarOpt4_Minusv(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt4> ::=  */
void Rule_onbarOpt4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a1> ::= -l IntegerLiteral */
void Rule_onbarOpt5a1_Minusl_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a1> ::=  */
void Rule_onbarOpt5a1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a2> ::= -O */
void Rule_onbarOpt5a2_MinusO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a2> ::=  */
void Rule_onbarOpt5a2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a3_> ::= -f StringLiteral */
void Rule_onbarOpt5a3__Minusf_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a3_> ::= <Id List> */
void Rule_onbarOpt5a3_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a3_> ::=  */
void Rule_onbarOpt5a3_2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a3> ::= <onbarOpt5a3_> */
void Rule_onbarOpt5a3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5a3> ::= -w */
void Rule_onbarOpt5a3_Minusw(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5> ::= <onbarOpt5a1> <onbarOpt5a2> <onbarOpt5a3> */
void Rule_onbarOpt5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt5> ::= -f */
void Rule_onbarOpt5_Minusf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt6> ::= Id */
void Rule_onbarOpt6_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt6> ::= Id Id */
void Rule_onbarOpt6_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt7> ::= -l */
void Rule_onbarOpt7_Minusl(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt7> ::=  */
void Rule_onbarOpt7(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt8> ::= -q */
void Rule_onbarOpt8_Minusq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt8> ::=  */
void Rule_onbarOpt8(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt9> ::= -u Id */
void Rule_onbarOpt9_Minusu_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt9> ::= -u StringLiteral */
void Rule_onbarOpt9_Minusu_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt9> ::=  */
void Rule_onbarOpt9(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt10> ::= -t IntegerLiteral */
void Rule_onbarOpt10_Minust_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt10> ::=  */
void Rule_onbarOpt10(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt11> ::= -x IntegerLiteral */
void Rule_onbarOpt11_Minusx_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt11> ::=  */
void Rule_onbarOpt11(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12> ::= <onbarOpt12a1> <onbarOpt12a2> <onbarOpt2> */
void Rule_onbarOpt12(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12> ::= <onbarOpt12bS> */
void Rule_onbarOpt122(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt13> ::= <onbarOpt5a3> */
void Rule_onbarOpt13(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt13> ::= -t StringLiteral */
void Rule_onbarOpt13_Minust_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt13> ::= -n IntegerLiteral */
void Rule_onbarOpt13_Minusn_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt13> ::= -w -t StringLiteral */
void Rule_onbarOpt13_Minusw_Minust_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt13> ::= -w -n IntegerLiteral */
void Rule_onbarOpt13_Minusw_Minusn_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12a1> ::= '-e2' */
void Rule_onbarOpt12a1_Minuse2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12a1> ::= -O */
void Rule_onbarOpt12a1_MinusO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12a2> ::= -i */
void Rule_onbarOpt12a2_Minusi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12a2> ::= '-I2' */
void Rule_onbarOpt12a2_MinusI2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12a2> ::=  */
void Rule_onbarOpt12a2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12bS> ::= <onbarOpt12b> <onbarOpt12bS> */
void Rule_onbarOpt12bS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12bS> ::= <onbarOpt12b> */
void Rule_onbarOpt12bS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12b> ::= -rename -P StringLiteral -O IntegerLiteral -n StringLiteral -O IntegerLiteral */
void Rule_onbarOpt12b_Minusrename_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusn_StringLiteral_MinusO_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt12b> ::= -rename -f StringLiteral */
void Rule_onbarOpt12b_Minusrename_Minusf_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt14> ::= <onbarOpt1> */
void Rule_onbarOpt14(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt14> ::= -t StringLiteral */
void Rule_onbarOpt14_Minust_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt14> ::= -n IntegerLiteral */
void Rule_onbarOpt14_Minusn_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt15> ::= OFF */
void Rule_onbarOpt15_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt15> ::= ON */
void Rule_onbarOpt15_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt15> ::= -d */
void Rule_onbarOpt15_Minusd(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onbarOpt15> ::=  */
void Rule_onbarOpt15(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -n <Id List2> */
void Rule_ontape_ontape_Minusn(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -s <ontapeOption1> <Id List2> */
void Rule_ontape_ontape_Minuss(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape <ontapeOption1> <Id List2> */
void Rule_ontape_ontape(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -v -s <onbarOpt5a1> -t STDIO -f */
void Rule_ontape_ontape_Minusv_Minuss_Minust_STDIO_Minusf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -s <onbarOpt5a1> -t STDIO -f */
void Rule_ontape_ontape_Minuss_Minust_STDIO_Minusf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -v -s <onbarOpt5a1> -f */
void Rule_ontape_ontape_Minusv_Minuss_Minusf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -s <onbarOpt5a1> -f */
void Rule_ontape_ontape_Minuss_Minusf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -v -s <onbarOpt5a1> -t STDIO */
void Rule_ontape_ontape_Minusv_Minuss_Minust_STDIO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -s <onbarOpt5a1> -t STDIO */
void Rule_ontape_ontape_Minuss_Minust_STDIO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -v -s <onbarOpt5a1> */
void Rule_ontape_ontape_Minusv_Minuss(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -s <onbarOpt5a1> */
void Rule_ontape_ontape_Minuss2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -a */
void Rule_ontape_ontape_Minusa(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -c */
void Rule_ontape_ontape_Minusc(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape <ontapeOption2> -d <Id List2> <ontapeOpt3> */
void Rule_ontape_ontape_Minusd(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape <ontapeOption2> <ontapeOpt3> */
void Rule_ontape_ontape2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape -l */
void Rule_ontape_ontape_Minusl(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontape> ::= ontape '-S2' */
void Rule_ontape_ontape_MinusS22(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOption1> ::= -b */
void Rule_ontapeOption1_Minusb(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOption1> ::= -u */
void Rule_ontapeOption1_Minusu(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOption1> ::= -a */
void Rule_ontapeOption1_Minusa(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOption2> ::= -r <onbarOpt12bS> */
void Rule_ontapeOption2_Minusr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOption2> ::= -r */
void Rule_ontapeOption2_Minusr2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOption2> ::= -P -e */
void Rule_ontapeOption2_MinusP_Minuse(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOption2> ::= -P */
void Rule_ontapeOption2_MinusP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOpt3> ::= -t STDIO -v */
void Rule_ontapeOpt3_Minust_STDIO_Minusv(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOpt3> ::= -t STDIO */
void Rule_ontapeOpt3_Minust_STDIO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ontapeOpt3> ::=  */
void Rule_ontapeOpt3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmode> ::= onmode <onmodeOption1> -y */
void Rule_onmode_onmode_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmode> ::= onmode <onmodeOption1> */
void Rule_onmode_onmode(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= '-BC 1' */
void Rule_onmodeOption1_MinusBC1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= '-BC 2' */
void Rule_onmodeOption1_MinusBC2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -k */
void Rule_onmodeOption1_Minusk(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -m */
void Rule_onmodeOption1_Minusm(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -s */
void Rule_onmodeOption1_Minuss(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -u */
void Rule_onmodeOption1_Minusu(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -j */
void Rule_onmodeOption1_Minusj(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c fuzzy */
void Rule_onmodeOption1_Minusc_fuzzy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c block */
void Rule_onmodeOption1_Minusc_block(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c unblock */
void Rule_onmodeOption1_Minusc_unblock(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c */
void Rule_onmodeOption1_Minusc(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -l */
void Rule_onmodeOption1_Minusl(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -z IntegerLiteral */
void Rule_onmodeOption1_Minusz_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -a IntegerLiteral */
void Rule_onmodeOption1_Minusa_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral CPU */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_CPU(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral ENCRYPT */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_ENCRYPT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral JVP */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_JVP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral Id */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral AIO */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_AIO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral LIO */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_LIO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral PIO */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_PIO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral SHM */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_SHM(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral SOC */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_SOC(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral STR */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_STR(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '+' IntegerLiteral TLI */
void Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_TLI(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '-' IntegerLiteral ENCRYPT */
void Rule_onmodeOption1_MinusP_Minus_IntegerLiteral_ENCRYPT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '-' IntegerLiteral JVP */
void Rule_onmodeOption1_MinusP_Minus_IntegerLiteral_JVP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -P '-' IntegerLiteral Id */
void Rule_onmodeOption1_MinusP_Minus_IntegerLiteral_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -r */
void Rule_onmodeOption1_Minusr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -d IntegerLiteral */
void Rule_onmodeOption1_Minusd_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -m IntegerLiteral */
void Rule_onmodeOption1_Minusm_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -q IntegerLiteral */
void Rule_onmodeOption1_Minusq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -s IntegerLiteral */
void Rule_onmodeOption1_Minuss_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -f */
void Rule_onmodeOption1_Minusf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -O */
void Rule_onmodeOption1_MinusO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -n */
void Rule_onmodeOption1_Minusn(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= '-r2' */
void Rule_onmodeOption1_Minusr2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= '-Z2' IntegerLiteral */
void Rule_onmodeOption1_MinusZ2_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -d STANDARD */
void Rule_onmodeOption1_Minusd_STANDARD(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -d PRIMARY Id */
void Rule_onmodeOption1_Minusd_PRIMARY_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -d SECONDARY Id */
void Rule_onmodeOption1_Minusd_SECONDARY_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -d idxauto ON */
void Rule_onmodeOption1_Minusd_idxauto_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -d idxauto OFF */
void Rule_onmodeOption1_Minusd_idxauto_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -d INDEX Id ':' Id '#' Id */
void Rule_onmodeOption1_Minusd_INDEX_Id_Colon_Id_Num_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -e ENABLE */
void Rule_onmodeOption1_Minuse_ENABLE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -e FLUSH */
void Rule_onmodeOption1_Minuse_FLUSH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -e OFF */
void Rule_onmodeOption1_Minuse_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -e ON */
void Rule_onmodeOption1_Minuse_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -w 'STMT_CACHE_HITS' IntegerLiteral */
void Rule_onmodeOption1_Minusw_STMT_CACHE_HITS_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -w 'STMT_CACHE_NOLIMIT' IntegerLiteral */
void Rule_onmodeOption1_Minusw_STMT_CACHE_NOLIMIT_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -y IntegerLiteral ON */
void Rule_onmodeOption1_Minusy_IntegerLiteral_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -y IntegerLiteral OFF */
void Rule_onmodeOption1_Minusy_IntegerLiteral_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -wm Id '=' <Expression> */
void Rule_onmodeOption1_Minuswm_Id_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -wf Id '=' <Expression> */
void Rule_onmodeOption1_Minuswf_Id_Eq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c START */
void Rule_onmodeOption1_Minusc_START(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c STOP IntegerLiteral */
void Rule_onmodeOption1_Minusc_STOP_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c kill IntegerLiteral */
void Rule_onmodeOption1_Minusc_kill_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c threshold IntegerLiteral */
void Rule_onmodeOption1_Minusc_threshold_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c HIGH */
void Rule_onmodeOption1_Minusc_HIGH(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= -c LOW */
void Rule_onmodeOption1_Minusc_LOW(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onmodeOption1> ::= '-C2' */
void Rule_onmodeOption1_MinusC2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -a -d Id -s IntegerLiteral -i */
void Rule_onparams_onparams_Minusa_Minusd_Id_Minuss_IntegerLiteral_Minusi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -a -d Id -s IntegerLiteral */
void Rule_onparams_onparams_Minusa_Minusd_Id_Minuss_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -a -d Id -i */
void Rule_onparams_onparams_Minusa_Minusd_Id_Minusi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -a -d Id */
void Rule_onparams_onparams_Minusa_Minusd_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -d -l IntegerLiteral -y */
void Rule_onparams_onparams_Minusd_Minusl_IntegerLiteral_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -d -l IntegerLiteral */
void Rule_onparams_onparams_Minusd_Minusl_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -P <onparamsOption1S> -y */
void Rule_onparams_onparams_MinusP_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -P <onparamsOption1S> */
void Rule_onparams_onparams_MinusP(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparams> ::= onparams -b -g IntegerLiteral <onparamsOpt2> <onparamsOpt3> <onparamsOpt4> <onparamsOpt5> */
void Rule_onparams_onparams_Minusb_Minusg_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOption1S> ::= <onparamsOption1> <onparamsOption1S> */
void Rule_onparamsOption1S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOption1S> ::= <onparamsOption1> */
void Rule_onparamsOption1S2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOption1> ::= -s IntegerLiteral */
void Rule_onparamsOption1_Minuss_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOption1> ::= -d Id */
void Rule_onparamsOption1_Minusd_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt2> ::= -n IntegerLiteral */
void Rule_onparamsOpt2_Minusn_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt2> ::=  */
void Rule_onparamsOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt3> ::= -r IntegerLiteral */
void Rule_onparamsOpt3_Minusr_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt3> ::=  */
void Rule_onparamsOpt3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt4> ::= -x RealLiteral */
void Rule_onparamsOpt4_Minusx_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt4> ::=  */
void Rule_onparamsOpt4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt5> ::= -m RealLiteral */
void Rule_onparamsOpt5_Minusm_RealLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onparamsOpt5> ::=  */
void Rule_onparamsOpt5(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblog> ::= ondblog <ondblogOption1> <ondblogOpt2> */
void Rule_ondblog_ondblog(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOption1> ::= buf */
void Rule_ondblogOption1_buf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOption1> ::= unbuf */
void Rule_ondblogOption1_unbuf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOption1> ::= nolog */
void Rule_ondblogOption1_nolog(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOption1> ::= ANSI */
void Rule_ondblogOption1_ANSI(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOption1> ::= cancel */
void Rule_ondblogOption1_cancel(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOpt2> ::= <Id List2> */
void Rule_ondblogOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOpt2> ::= -f StringLiteral */
void Rule_ondblogOpt2_Minusf_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <ondblogOpt2> ::=  */
void Rule_ondblogOpt22(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlog> ::= onlog <onlogOption1> <onlogOption2S> <oncheckOpt2> */
void Rule_onlog_onlog(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption1> ::= -d Id -b <onlogOption1_S> */
void Rule_onlogOption1_Minusd_Id_Minusb(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption1> ::= -d Id <onlogOption1_S> */
void Rule_onlogOption1_Minusd_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption1> ::= <onlogOption1_S> */
void Rule_onlogOption1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption1_S> ::= -n <onbarOpt6> <onlogOption1_S> */
void Rule_onlogOption1_S_Minusn(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption1_S> ::= -n <onbarOpt6> */
void Rule_onlogOption1_S_Minusn2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption2S> ::= <onlogOption2> <onlogOption2S> */
void Rule_onlogOption2S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption2S> ::=  */
void Rule_onlogOption2S2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption2> ::= -l */
void Rule_onlogOption2_Minusl(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption2> ::= -t IntegerLiteral */
void Rule_onlogOption2_Minust_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption2> ::= -u StringLiteral */
void Rule_onlogOption2_Minusu_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onlogOption2> ::= -x IntegerLiteral */
void Rule_onlogOption2_Minusx_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheck> ::= oncheck <oncheckOption1> <oncheckOpt1> <oncheckOpt2> */
void Rule_oncheck_oncheck(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt1> ::= -n */
void Rule_oncheckOpt1_Minusn(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt1> ::= -y */
void Rule_oncheckOpt1_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt1> ::=  */
void Rule_oncheckOpt1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt2> ::= -q */
void Rule_oncheckOpt2_Minusq(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt2> ::=  */
void Rule_oncheckOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -ce */
void Rule_oncheckOption1_Minusce(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pe */
void Rule_oncheckOption1_Minuspe(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -cr */
void Rule_oncheckOption1_Minuscr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pr */
void Rule_oncheckOption1_Minuspr(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= '-cR2' */
void Rule_oncheckOption1_MinuscR2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= '-pR2' */
void Rule_oncheckOption1_MinuspR2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption2> -x Id ':' Id '#' Id */
void Rule_oncheckOption1_Minusx_Id_Colon_Id_Num_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption2> -x Id */
void Rule_oncheckOption1_Minusx_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption2> Id ':' Id '#' Id */
void Rule_oncheckOption1_Id_Colon_Id_Num_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption2> Id */
void Rule_oncheckOption1_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption2> -x Id ':' Id */
void Rule_oncheckOption1_Minusx_Id_Colon_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption2> Id ':' Id */
void Rule_oncheckOption1_Id_Colon_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption3> Id ':' Id ',' Id */
void Rule_oncheckOption1_Id_Colon_Id_Comma_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption3> Id */
void Rule_oncheckOption1_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption3> Id ':' Id */
void Rule_oncheckOption1_Id_Colon_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -cc Id */
void Rule_oncheckOption1_Minuscc_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pc Id */
void Rule_oncheckOption1_Minuspc_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -cc */
void Rule_oncheckOption1_Minuscc(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pc */
void Rule_oncheckOption1_Minuspc(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption4> Id ':' Id ',' Id */
void Rule_oncheckOption1_Id_Colon_Id_Comma_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption4> Id */
void Rule_oncheckOption1_Id3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption4> Id ':' Id */
void Rule_oncheckOption1_Id_Colon_Id3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption5> Id ':' Id ',' Id IntegerLiteral */
void Rule_oncheckOption1_Id_Colon_Id_Comma_Id_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption5> Id */
void Rule_oncheckOption1_Id4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption5> Id ':' Id ',' Id */
void Rule_oncheckOption1_Id_Colon_Id_Comma_Id3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption5> Id ':' Id IntegerLiteral */
void Rule_oncheckOption1_Id_Colon_Id_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption5> Id ':' Id */
void Rule_oncheckOption1_Id_Colon_Id4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption5> IntegerLiteral IntegerLiteral */
void Rule_oncheckOption1_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= <oncheckOption5> IntegerLiteral */
void Rule_oncheckOption1_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp Id ':' Id ',' Id IntegerLiteral */
void Rule_oncheckOption1_Minuspp_Id_Colon_Id_Comma_Id_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp Id ':' Id IntegerLiteral */
void Rule_oncheckOption1_Minuspp_Id_Colon_Id_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp IntegerLiteral IntegerLiteral */
void Rule_oncheckOption1_Minuspp_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= '-pP2' IntegerLiteral IntegerLiteral */
void Rule_oncheckOption1_MinuspP2_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp Id IntegerLiteral IntegerLiteral */
void Rule_oncheckOption1_Minuspp_Id_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp Id IntegerLiteral */
void Rule_oncheckOption1_Minuspp_Id_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp Id IntegerLiteral -h */
void Rule_oncheckOption1_Minuspp_Id_IntegerLiteral_Minush(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp IntegerLiteral IntegerLiteral IntegerLiteral */
void Rule_oncheckOption1_Minuspp_IntegerLiteral_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pp IntegerLiteral IntegerLiteral -h */
void Rule_oncheckOption1_Minuspp_IntegerLiteral_IntegerLiteral_Minush(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -cs Id */
void Rule_oncheckOption1_Minuscs_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -cs */
void Rule_oncheckOption1_Minuscs(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= '-cS2' Id */
void Rule_oncheckOption1_MinuscS2_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= '-cS2' */
void Rule_oncheckOption1_MinuscS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -ps Id IntegerLiteral IntegerLiteral */
void Rule_oncheckOption1_Minusps_Id_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -ps */
void Rule_oncheckOption1_Minusps(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= '-pS2' Id IntegerLiteral IntegerLiteral */
void Rule_oncheckOption1_MinuspS2_Id_IntegerLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= '-pS2' */
void Rule_oncheckOption1_MinuspS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -u StringLiteral '(' StringLiteral ')' */
void Rule_oncheckOption1_Minusu_StringLiteral_LParan_StringLiteral_RParan(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -u StringLiteral */
void Rule_oncheckOption1_Minusu_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -cv <oncheckOpt6> Id <oncheckOpt7> Id <oncheckOpt7> */
void Rule_oncheckOption1_Minuscv_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption1> ::= -pv <oncheckOpt6> Id <oncheckOpt7> Id <oncheckOpt7> */
void Rule_oncheckOption1_Minuspv_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption2> ::= -ci */
void Rule_oncheckOption2_Minusci(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption2> ::= -cl */
void Rule_oncheckOption2_Minuscl(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption2> ::= -pk */
void Rule_oncheckOption2_Minuspk(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption2> ::= '-pK2' */
void Rule_oncheckOption2_MinuspK2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption2> ::= -pl */
void Rule_oncheckOption2_Minuspl(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption2> ::= '-pL2' */
void Rule_oncheckOption2_MinuspL2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption3> ::= -cd */
void Rule_oncheckOption3_Minuscd(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption3> ::= '-cD2' */
void Rule_oncheckOption3_MinuscD2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption4> ::= -pB */
void Rule_oncheckOption4_MinuspB(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption4> ::= -pt */
void Rule_oncheckOption4_Minuspt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption4> ::= '-pT2' */
void Rule_oncheckOption4_MinuspT2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption5> ::= -pd */
void Rule_oncheckOption5_Minuspd(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOption5> ::= '-pD2' */
void Rule_oncheckOption5_MinuspD2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt6> ::= -b */
void Rule_oncheckOpt6_Minusb(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt6> ::= -c */
void Rule_oncheckOpt6_Minusc(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt6> ::= -O */
void Rule_oncheckOpt6_MinusO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt6> ::= -t */
void Rule_oncheckOpt6_Minust(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt6> ::= -u */
void Rule_oncheckOpt6_Minusu(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt6> ::=  */
void Rule_oncheckOpt6(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt7> ::= '@' Id Id */
void Rule_oncheckOpt7_At_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt7> ::= '@' Id */
void Rule_oncheckOpt7_At_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <oncheckOpt7> ::= Id */
void Rule_oncheckOpt7_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SemicolonOpt> ::= ';' */
void Rule_SemicolonOpt_Semi(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <SemicolonOpt> ::=  */
void Rule_SemicolonOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISKINIT> ::= DISKINIT NAME '=' Id ',' PHYNAME '=' StringLiteral ',' VDEVNO '=' IntegerLiteral ',' SIZE '=' IntegerLiteral <SemicolonOpt> */
void Rule_DISKINIT_DISKINIT_NAME_Eq_Id_Comma_PHYNAME_Eq_StringLiteral_Comma_VDEVNO_Eq_IntegerLiteral_Comma_SIZE_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISKINIT> ::= DISKINIT Id ',' StringLiteral ',' IntegerLiteral ',' IntegerLiteral <SemicolonOpt> */
void Rule_DISKINIT_DISKINIT_Id_Comma_StringLiteral_Comma_IntegerLiteral_Comma_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <DISKINIT> ::= DISKINIT <onspaces> <SemicolonOpt> */
void Rule_DISKINIT_DISKINIT(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -c -d Id <onspaces_cOption1> <onspaces_cOption2> <onspaces_cOption3> */
void Rule_onspaces_Minusc_Minusd_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -c -d Id <onspaces_cOption1> <onspaces_cOption2> <onspaces_cOption3> -k IntegerLiteral */
void Rule_onspaces_Minusc_Minusd_Id_Minusk_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -c -b Id -g IntegerLiteral <onspaces_cOption2> <onspaces_cOption3> */
void Rule_onspaces_Minusc_Minusb_Id_Minusg_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -c -x Id -l StringLiteral -O IntegerLiteral -s IntegerLiteral */
void Rule_onspaces_Minusc_Minusx_Id_Minusl_StringLiteral_MinusO_IntegerLiteral_Minuss_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -c -s Id -t <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> <onspaces_cOpt4> */
void Rule_onspaces_Minusc_Minuss_Id_Minust(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -c -s Id <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> <onspaces_cOpt4> */
void Rule_onspaces_Minusc_Minuss_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -ch Id -Df <onspaces_defaultlist> */
void Rule_onspaces_Minusch_Id_MinusDf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -cl Id */
void Rule_onspaces_Minuscl_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -a Id <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> -u */
void Rule_onspaces_Minusa_Id_Minusu(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -a Id <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> */
void Rule_onspaces_Minusa_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -d Id -y */
void Rule_onspaces_Minusd_Id_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -d -f Id -y */
void Rule_onspaces_Minusd_Minusf_Id_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -d Id */
void Rule_onspaces_Minusd_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -d -f Id */
void Rule_onspaces_Minusd_Minusf_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -d Id -P StringLiteral -O IntegerLiteral -y */
void Rule_onspaces_Minusd_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -d Id -P StringLiteral -O IntegerLiteral */
void Rule_onspaces_Minusd_Id_MinusP_StringLiteral_MinusO_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -m Id <onspace_mS> -y */
void Rule_onspaces_Minusm_Id_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -m Id <onspace_mS> */
void Rule_onspaces_Minusm_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -r Id -y */
void Rule_onspaces_Minusr_Id_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -r Id */
void Rule_onspaces_Minusr_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -d -y */
void Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusd_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -O -y */
void Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_MinusO_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -d */
void Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusd(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -O */
void Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_MinusO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f ON <Id List2> -y */
void Rule_onspaces_Minusf_ON_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f OFF <Id List2> -y */
void Rule_onspaces_Minusf_OFF_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f ON <Id List2> */
void Rule_onspaces_Minusf_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f OFF <Id List2> */
void Rule_onspaces_Minusf_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f ON -y */
void Rule_onspaces_Minusf_ON_Minusy2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f OFF -y */
void Rule_onspaces_Minusf_OFF_Minusy2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f ON */
void Rule_onspaces_Minusf_ON2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -f OFF */
void Rule_onspaces_Minusf_OFF2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces> ::= -ren Id -n Id */
void Rule_onspaces_Minusren_Id_Minusn_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOption2> ::= -P StringLiteral -O IntegerLiteral -s IntegerLiteral */
void Rule_onspaces_cOption2_MinusP_StringLiteral_MinusO_IntegerLiteral_Minuss_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOption3> ::= -m StringLiteral IntegerLiteral */
void Rule_onspaces_cOption3_Minusm_StringLiteral_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOption3> ::=  */
void Rule_onspaces_cOption3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOpt2> ::= -Ms IntegerLiteral */
void Rule_onspaces_cOpt2_MinusMs_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOpt2> ::=  */
void Rule_onspaces_cOpt2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOpt3> ::= -Mo IntegerLiteral */
void Rule_onspaces_cOpt3_MinusMo_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOpt3> ::=  */
void Rule_onspaces_cOpt3(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOpt4> ::= -Df <onspaces_defaultlist> */
void Rule_onspaces_cOpt4_MinusDf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOpt4> ::=  */
void Rule_onspaces_cOpt4(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspace_mS> ::= <onspace_m> <onspace_mS> */
void Rule_onspace_mS(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspace_mS> ::= <onspace_m> */
void Rule_onspace_mS2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspace_m> ::= <onspaces_cOption2> */
void Rule_onspace_m(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspace_m> ::= -f StringLiteral */
void Rule_onspace_m_Minusf_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOption1> ::= -ef IntegerLiteral -en IntegerLiteral */
void Rule_onspaces_cOption1_Minusef_IntegerLiteral_Minusen_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOption1> ::= -t */
void Rule_onspaces_cOption1_Minust(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_cOption1> ::=  */
void Rule_onspaces_cOption1(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_defaultlist> ::= '"' <onspaces_default> <onspaces_defaultlist_> '"' */
void Rule_onspaces_defaultlist_Quote_Quote(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_defaultlist_> ::= ',' <onspaces_default> <onspaces_defaultlist_> */
void Rule_onspaces_defaultlist__Comma(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_defaultlist_> ::=  */
void Rule_onspaces_defaultlist_(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= ACCESSTIME '=' OFF */
void Rule_onspaces_default_ACCESSTIME_Eq_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= ACCESSTIME '=' ON */
void Rule_onspaces_default_ACCESSTIME_Eq_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= 'AVG_LO_SIZE' '=' IntegerLiteral */
void Rule_onspaces_default_AVG_LO_SIZE_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= BUFFERING '=' ON */
void Rule_onspaces_default_BUFFERING_Eq_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= BUFFERING '=' OFF */
void Rule_onspaces_default_BUFFERING_Eq_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= 'LOCK_MODE' '=' BLOB */
void Rule_onspaces_default_LOCK_MODE_Eq_BLOB(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= 'LOCK_MODE' '=' RANGE */
void Rule_onspaces_default_LOCK_MODE_Eq_RANGE(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= LOGGING '=' OFF */
void Rule_onspaces_default_LOGGING_Eq_OFF(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= LOGGING '=' ON */
void Rule_onspaces_default_LOGGING_Eq_ON(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= 'EXTENT_SIZE' '=' IntegerLiteral */
void Rule_onspaces_default_EXTENT_SIZE_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= 'MIN_EXT_SIZE' '=' IntegerLiteral */
void Rule_onspaces_default_MIN_EXT_SIZE_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onspaces_default> ::= 'NEXT_SIZE' '=' IntegerLiteral */
void Rule_onspaces_default_NEXT_SIZE_Eq_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id List2> ::= Id <Id List2> */
void Rule_IdList2_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Id List2> ::= Id */
void Rule_IdList2_Id2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -O */
void Rule_onaudit_onaudit_MinusO(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -O -u Id */
void Rule_onaudit_onaudit_MinusO_Minusu_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -O -y */
void Rule_onaudit_onaudit_MinusO_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -a <Audit-Mask Specification> */
void Rule_onaudit_onaudit_Minusa(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -f <onaudit Input-File Format> */
void Rule_onaudit_onaudit_Minusf(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -m <Audit-Mask Specification> */
void Rule_onaudit_onaudit_Minusm(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -d */
void Rule_onaudit_onaudit_Minusd(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -d -u Id */
void Rule_onaudit_onaudit_Minusd_Minusu_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -d -y */
void Rule_onaudit_onaudit_Minusd_Minusy(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -n */
void Rule_onaudit_onaudit_Minusn(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit -c */
void Rule_onaudit_onaudit_Minusc(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit> ::= onaudit <onaudit_auditmodeOpt> <onaudit_errormodeOpt> <onaudit_auditdirOpt> <onaudit_maxsizeOpt> */
void Rule_onaudit_onaudit(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit-Mask Specification> ::= -u Id -r Id <Audit-Mask Specification_S> */
void Rule_AuditMaskSpecification_Minusu_Id_Minusr_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit-Mask Specification> ::= -u Id <Audit-Mask Specification_S> */
void Rule_AuditMaskSpecification_Minusu_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit-Mask Specification_S> ::= <Audit-Mask Specification_> <Audit-Mask Specification_S> */
void Rule_AuditMaskSpecification_S(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit-Mask Specification_S> ::=  */
void Rule_AuditMaskSpecification_S2(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit-Mask Specification_> ::= -e <Audit Event Specification> */
void Rule_AuditMaskSpecification__Minuse(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit-Mask Specification_> ::= -e '+' <Audit Event Specification> */
void Rule_AuditMaskSpecification__Minuse_Plus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit-Mask Specification_> ::= -e '-' <Audit Event Specification> */
void Rule_AuditMaskSpecification__Minuse_Minus(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <Audit Event Specification> ::= <Id List> */
void Rule_AuditEventSpecification(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit Input-File Format> ::= Id Id <Audit-Mask Specification_S> */
void Rule_onauditInputFileFormat_Id_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit Input-File Format> ::= Id <Audit-Mask Specification_S> */
void Rule_onauditInputFileFormat_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_auditmodeOpt> ::= -l Id */
void Rule_onaudit_auditmodeOpt_Minusl_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_auditmodeOpt> ::=  */
void Rule_onaudit_auditmodeOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_errormodeOpt> ::= -e Id */
void Rule_onaudit_errormodeOpt_Minuse_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_errormodeOpt> ::=  */
void Rule_onaudit_errormodeOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_auditdirOpt> ::= -P StringLiteral */
void Rule_onaudit_auditdirOpt_MinusP_StringLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_auditdirOpt> ::= -P Id */
void Rule_onaudit_auditdirOpt_MinusP_Id(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_auditdirOpt> ::=  */
void Rule_onaudit_auditdirOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_maxsizeOpt> ::= -s IntegerLiteral */
void Rule_onaudit_maxsizeOpt_Minuss_IntegerLiteral(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};

/* <onaudit_maxsizeOpt> ::=  */
void Rule_onaudit_maxsizeOpt(struct TokenStruct *Token, struct ContextStruct *Context) {
  RuleTemplate(Token,Context);
};



/***** Rule jumptable *******************************************************/
void (*RuleJumpTable[])(struct TokenStruct *Token, struct ContextStruct *Context) = {
  /* 0. <Id List> ::= <Id Member> <Id List_> */
  Rule_IdList,
  /* 1. <Id List_> ::= ',' <Id Member> <Id List_> */
  Rule_IdList__Comma,
  /* 2. <Id List_> ::=  */
  Rule_IdList_,
  /* 3. <Id Member> ::= <CREATE_QUEUE_object> Id */
  Rule_IdMember_Id,
  /* 4. <Id Member> ::= <CREATE_QUEUE_object> */
  Rule_IdMember,
  /* 5. <CREATE_QUEUE_object> ::= Id */
  Rule_CREATE_QUEUE_object_Id,
  /* 6. <CREATE_QUEUE_object> ::= LocalTempTable */
  Rule_CREATE_QUEUE_object_LocalTempTable,
  /* 7. <CREATE_QUEUE_object> ::= GlobalTempTable */
  Rule_CREATE_QUEUE_object_GlobalTempTable,
  /* 8. <Expression> ::= <And Exp> OR <Expression> */
  Rule_Expression_OR,
  /* 9. <Expression> ::= <And Exp> */
  Rule_Expression,
  /* 10. <And Exp> ::= <Not Exp> AND <And Exp> */
  Rule_AndExp_AND,
  /* 11. <And Exp> ::= <Not Exp> */
  Rule_AndExp,
  /* 12. <Not Exp> ::= NOT <Pred Exp> */
  Rule_NotExp_NOT,
  /* 13. <Not Exp> ::= <Pred Exp> */
  Rule_NotExp,
  /* 14. <Pred Exp> ::= <Add Exp> BETWEEN <Add Exp> AND <Add Exp> */
  Rule_PredExp_BETWEEN_AND,
  /* 15. <Pred Exp> ::= <Add Exp> NOT BETWEEN <Add Exp> AND <Add Exp> */
  Rule_PredExp_NOT_BETWEEN_AND,
  /* 16. <Pred Exp> ::= <Value> IS NOT NULL */
  Rule_PredExp_IS_NOT_NULL,
  /* 17. <Pred Exp> ::= Id IS NOT NULL */
  Rule_PredExp_Id_IS_NOT_NULL,
  /* 18. <Pred Exp> ::= <Value> IS NULL */
  Rule_PredExp_IS_NULL,
  /* 19. <Pred Exp> ::= Id IS NULL */
  Rule_PredExp_Id_IS_NULL,
  /* 20. <Pred Exp> ::= <Add Exp> LIKE StringLiteral */
  Rule_PredExp_LIKE_StringLiteral,
  /* 21. <Pred Exp> ::= <Add Exp> LIKE LocalVarId */
  Rule_PredExp_LIKE_LocalVarId,
  /* 22. <Pred Exp> ::= <Add Exp> LIKE GlobalVarId */
  Rule_PredExp_LIKE_GlobalVarId,
  /* 23. <Pred Exp> ::= <Add Exp> IN '(' <SELECT> ')' */
  Rule_PredExp_IN_LParan_RParan,
  /* 24. <Pred Exp> ::= <Add Exp> IN '(' <Expr List> ')' */
  Rule_PredExp_IN_LParan_RParan2,
  /* 25. <Pred Exp> ::= <Add Exp> '=' <Add Exp> */
  Rule_PredExp_Eq,
  /* 26. <Pred Exp> ::= <Add Exp> '<>' <Add Exp> */
  Rule_PredExp_LtGt,
  /* 27. <Pred Exp> ::= <Add Exp> '!=' <Add Exp> */
  Rule_PredExp_ExclamEq,
  /* 28. <Pred Exp> ::= <Add Exp> '>' <Add Exp> */
  Rule_PredExp_Gt,
  /* 29. <Pred Exp> ::= <Add Exp> '>=' <Add Exp> */
  Rule_PredExp_GtEq,
  /* 30. <Pred Exp> ::= <Add Exp> '<' <Add Exp> */
  Rule_PredExp_Lt,
  /* 31. <Pred Exp> ::= <Add Exp> '<=' <Add Exp> */
  Rule_PredExp_LtEq,
  /* 32. <Pred Exp> ::= <Add Exp> */
  Rule_PredExp,
  /* 33. <Add Exp> ::= <Add Exp> '+' <Mult Exp> */
  Rule_AddExp_Plus,
  /* 34. <Add Exp> ::= <Add Exp> '-' <Mult Exp> */
  Rule_AddExp_Minus,
  /* 35. <Add Exp> ::= <Add Exp> '||' <Mult Exp> */
  Rule_AddExp_PipePipe,
  /* 36. <Add Exp> ::= <Mult Exp> */
  Rule_AddExp,
  /* 37. <Mult Exp> ::= <Mult Exp> '*' '-' <Value> */
  Rule_MultExp_Times_Minus,
  /* 38. <Mult Exp> ::= <Mult Exp> '*' '-' <CREATE_QUEUE_object> */
  Rule_MultExp_Times_Minus2,
  /* 39. <Mult Exp> ::= <Mult Exp> '*' <Value> */
  Rule_MultExp_Times,
  /* 40. <Mult Exp> ::= <Mult Exp> '*' <CREATE_QUEUE_object> */
  Rule_MultExp_Times2,
  /* 41. <Mult Exp> ::= <Mult Exp> '/' '-' <Value> */
  Rule_MultExp_Div_Minus,
  /* 42. <Mult Exp> ::= <Mult Exp> '/' '-' <CREATE_QUEUE_object> */
  Rule_MultExp_Div_Minus2,
  /* 43. <Mult Exp> ::= <Mult Exp> '/' <Value> */
  Rule_MultExp_Div,
  /* 44. <Mult Exp> ::= <Mult Exp> '/' <CREATE_QUEUE_object> */
  Rule_MultExp_Div2,
  /* 45. <Mult Exp> ::= '-' <Value> */
  Rule_MultExp_Minus,
  /* 46. <Mult Exp> ::= '-' <CREATE_QUEUE_object> */
  Rule_MultExp_Minus2,
  /* 47. <Mult Exp> ::= <Value> */
  Rule_MultExp,
  /* 48. <Mult Exp> ::= <CREATE_QUEUE_object> */
  Rule_MultExp2,
  /* 49. <Value> ::= '(' <Expr List> ')' */
  Rule_Value_LParan_RParan,
  /* 50. <Value> ::= LocalVarId */
  Rule_Value_LocalVarId,
  /* 51. <Value> ::= GlobalVarId */
  Rule_Value_GlobalVarId,
  /* 52. <Value> ::= <Value_Common> */
  Rule_Value,
  /* 53. <Value> ::= CASE <CASE_WHEN_THENs> ELSE <Expression> END */
  Rule_Value_CASE_ELSE_END,
  /* 54. <Value> ::= CASE <CASE_WHEN_THENs> END */
  Rule_Value_CASE_END,
  /* 55. <Value> ::= CASE <Expression> <CASE_WHEN_THENs> ELSE <Expression> END */
  Rule_Value_CASE_ELSE_END2,
  /* 56. <Value> ::= CASE <Expression> <CASE_WHEN_THENs> END */
  Rule_Value_CASE_END2,
  /* 57. <Value> ::= '(' <SELECT> ')' */
  Rule_Value_LParan_RParan2,
  /* 58. <Value> ::= <SETquery> */
  Rule_Value2,
  /* 59. <Value_Common> ::= IntegerLiteral */
  Rule_Value_Common_IntegerLiteral,
  /* 60. <Value_Common> ::= RealLiteral */
  Rule_Value_Common_RealLiteral,
  /* 61. <Value_Common> ::= StringLiteral */
  Rule_Value_Common_StringLiteral,
  /* 62. <Value_Common> ::= NULL */
  Rule_Value_Common_NULL,
  /* 63. <Value_Common> ::= <FuncCall> */
  Rule_Value_Common,
  /* 64. <CASE_WHEN_THENs> ::= WHEN <Expression> THEN <Expression> <CASE_WHEN_THENs> */
  Rule_CASE_WHEN_THENs_WHEN_THEN,
  /* 65. <CASE_WHEN_THENs> ::= WHEN <Expression> THEN <Expression> */
  Rule_CASE_WHEN_THENs_WHEN_THEN2,
  /* 66. <Expr List> ::= <Expression> ',' <Expr List> */
  Rule_ExprList_Comma,
  /* 67. <Expr List> ::= <Expression> */
  Rule_ExprList,
  /* 68. <SQLs_> ::= <SQL_> <SQLs_> */
  Rule_SQLs_,
  /* 69. <SQLs_> ::= <SQL_> */
  Rule_SQLs_2,
  /* 70. <SQL_> ::= <IDS> <SemicolonOpt> */
  Rule_SQL_,
  /* 71. <SQLs> ::= <SQLs_> */
  Rule_SQLs,
  /* 72. <SQLs> ::= <DEFINE> <SemicolonOpt> */
  Rule_SQLs2,
  /* 73. <IDS> ::= <ALTER ACCESS_METHOD> */
  Rule_IDS,
  /* 74. <IDS> ::= <ALTER FRAGMENT> */
  Rule_IDS2,
  /* 75. <IDS> ::= <ALTER FUNCTION> */
  Rule_IDS3,
  /* 76. <IDS> ::= <ALTER INDEX> */
  Rule_IDS4,
  /* 77. <IDS> ::= <ALTER PROCEDURE> */
  Rule_IDS5,
  /* 78. <IDS> ::= <ALTER ROUTINE> */
  Rule_IDS6,
  /* 79. <IDS> ::= <ALTER SECURITY LABEL COMPONENT> */
  Rule_IDS7,
  /* 80. <IDS> ::= <ALTER SEQUENCE> */
  Rule_IDS8,
  /* 81. <IDS> ::= <ALTER TABLE> */
  Rule_IDS9,
  /* 82. <IDS> ::= <CLOSE DATABASE> */
  Rule_IDS10,
  /* 83. <IDS> ::= <CREATE ACCESS_METHOD> */
  Rule_IDS11,
  /* 84. <IDS> ::= <CREATE AGGREGATE> */
  Rule_IDS12,
  /* 85. <IDS> ::= <CREATE CAST> */
  Rule_IDS13,
  /* 86. <IDS> ::= <CREATE DATABASE> */
  Rule_IDS14,
  /* 87. <IDS> ::= <CREATE DISTINCT TYPE> */
  Rule_IDS15,
  /* 88. <IDS> ::= <CREATE DUPLICATE> */
  Rule_IDS16,
  /* 89. <IDS> ::= <CREATE EXTERNAL TABLE> */
  Rule_IDS17,
  /* 90. <IDS> ::= <CREATE FUNCTION> */
  Rule_IDS18,
  /* 91. <IDS> ::= <CREATE INDEX> */
  Rule_IDS19,
  /* 92. <IDS> ::= <CREATE OPAQUE TYPE> */
  Rule_IDS20,
  /* 93. <IDS> ::= <CREATE OPCLASS> */
  Rule_IDS21,
  /* 94. <IDS> ::= <CREATE PROCEDURE> */
  Rule_IDS22,
  /* 95. <IDS> ::= <CREATE ROLE> */
  Rule_IDS23,
  /* 96. <IDS> ::= <CREATE ROUTINE FROM> */
  Rule_IDS24,
  /* 97. <IDS> ::= <CREATE ROW TYPE> */
  Rule_IDS25,
  /* 98. <IDS> ::= <CREATE SCHEMA> */
  Rule_IDS26,
  /* 99. <IDS> ::= <CREATE SCRATCH TABLE> */
  Rule_IDS27,
  /* 100. <IDS> ::= <CREATE SECURITY LABEL> */
  Rule_IDS28,
  /* 101. <IDS> ::= <CREATE SECURITY LABEL COMPONENT> */
  Rule_IDS29,
  /* 102. <IDS> ::= <CREATE SECURITY POLICY> */
  Rule_IDS30,
  /* 103. <IDS> ::= <CREATE SEQUENCE> */
  Rule_IDS31,
  /* 104. <IDS> ::= <CREATE SYNONYM> */
  Rule_IDS32,
  /* 105. <IDS> ::= <CREATE TABLE> */
  Rule_IDS33,
  /* 106. <IDS> ::= <CREATE TEMP TABLE> */
  Rule_IDS34,
  /* 107. <IDS> ::= <CREATE TRIGGER> */
  Rule_IDS35,
  /* 108. <IDS> ::= <CREATE VIEW> */
  Rule_IDS36,
  /* 109. <IDS> ::= <CREATE XADATASOURCE> */
  Rule_IDS37,
  /* 110. <IDS> ::= <CREATE XADATASOURCE TYPE> */
  Rule_IDS38,
  /* 111. <IDS> ::= <DROP ACCESS_METHOD> */
  Rule_IDS39,
  /* 112. <IDS> ::= <DROP AGGREGATE> */
  Rule_IDS40,
  /* 113. <IDS> ::= <DROP CAST> */
  Rule_IDS41,
  /* 114. <IDS> ::= <DROP DATABASE> */
  Rule_IDS42,
  /* 115. <IDS> ::= <DROP DUPLICATE> */
  Rule_IDS43,
  /* 116. <IDS> ::= <DROP FUNCTION> */
  Rule_IDS44,
  /* 117. <IDS> ::= <DROP INDEX> */
  Rule_IDS45,
  /* 118. <IDS> ::= <DROP OPCLASS> */
  Rule_IDS46,
  /* 119. <IDS> ::= <DROP PROCEDURE> */
  Rule_IDS47,
  /* 120. <IDS> ::= <DROP ROLE> */
  Rule_IDS48,
  /* 121. <IDS> ::= <DROP ROUTINE> */
  Rule_IDS49,
  /* 122. <IDS> ::= <DROP ROW TYPE> */
  Rule_IDS50,
  /* 123. <IDS> ::= <DROP SECURITY> */
  Rule_IDS51,
  /* 124. <IDS> ::= <DROP SEQUENCE> */
  Rule_IDS52,
  /* 125. <IDS> ::= <DROP SYNONYM> */
  Rule_IDS53,
  /* 126. <IDS> ::= <DROP TABLE> */
  Rule_IDS54,
  /* 127. <IDS> ::= <DROP TRIGGER> */
  Rule_IDS55,
  /* 128. <IDS> ::= <DROP TYPE> */
  Rule_IDS56,
  /* 129. <IDS> ::= <DROP VIEW> */
  Rule_IDS57,
  /* 130. <IDS> ::= <DROP XADATASOURCE> */
  Rule_IDS58,
  /* 131. <IDS> ::= <DROP XADATASOURCE TYPE> */
  Rule_IDS59,
  /* 132. <IDS> ::= <MOVE TABLE> */
  Rule_IDS60,
  /* 133. <IDS> ::= <RENAME COLUMN> */
  Rule_IDS61,
  /* 134. <IDS> ::= <RENAME DATABASE> */
  Rule_IDS62,
  /* 135. <IDS> ::= <RENAME INDEX> */
  Rule_IDS63,
  /* 136. <IDS> ::= <RENAME SECURITY> */
  Rule_IDS64,
  /* 137. <IDS> ::= <RENAME SEQUENCE> */
  Rule_IDS65,
  /* 138. <IDS> ::= <RENAME TABLE> */
  Rule_IDS66,
  /* 139. <IDS> ::= <DELETE> */
  Rule_IDS67,
  /* 140. <IDS> ::= <INSERT> */
  Rule_IDS68,
  /* 141. <IDS> ::= <LOAD> */
  Rule_IDS69,
  /* 142. <IDS> ::= <MERGE> */
  Rule_IDS70,
  /* 143. <IDS> ::= <SELECT> */
  Rule_IDS71,
  /* 144. <IDS> ::= <TRUNCATE> */
  Rule_IDS72,
  /* 145. <IDS> ::= <UNLOAD> */
  Rule_IDS73,
  /* 146. <IDS> ::= <UPDATE> */
  Rule_IDS74,
  /* 147. <IDS> ::= <CLOSE> */
  Rule_IDS75,
  /* 148. <IDS> ::= <DECLARE> */
  Rule_IDS76,
  /* 149. <IDS> ::= <FETCH> */
  Rule_IDS77,
  /* 150. <IDS> ::= <FLUSH> */
  Rule_IDS78,
  /* 151. <IDS> ::= <FREE> */
  Rule_IDS79,
  /* 152. <IDS> ::= <OPEN> */
  Rule_IDS80,
  /* 153. <IDS> ::= <PUT> */
  Rule_IDS81,
  /* 154. <IDS> ::= <SET AUTOFREE> */
  Rule_IDS82,
  /* 155. <IDS> ::= <EXECUTE IMMEDIATE> */
  Rule_IDS83,
  /* 156. <IDS> ::= <INFO> */
  Rule_IDS84,
  /* 157. <IDS> ::= <GRANT> */
  Rule_IDS85,
  /* 158. <IDS> ::= <GRANT FRAGMENT> */
  Rule_IDS86,
  /* 159. <IDS> ::= <LOCK TABLE> */
  Rule_IDS87,
  /* 160. <IDS> ::= <REVOKE> */
  Rule_IDS88,
  /* 161. <IDS> ::= <REVOKE FRAGMENT> */
  Rule_IDS89,
  /* 162. <IDS> ::= <SET ISOLATION> */
  Rule_IDS90,
  /* 163. <IDS> ::= <SET LOCK MODE> */
  Rule_IDS91,
  /* 164. <IDS> ::= <SET ROLE> */
  Rule_IDS92,
  /* 165. <IDS> ::= <SET SESSION AUTHORIZATION> */
  Rule_IDS93,
  /* 166. <IDS> ::= <SET TRANSACTION> */
  Rule_IDS94,
  /* 167. <IDS> ::= <UNLOCK TABLE> */
  Rule_IDS95,
  /* 168. <IDS> ::= <BEGIN WORK> */
  Rule_IDS96,
  /* 169. <IDS> ::= <COMMIT WORK> */
  Rule_IDS97,
  /* 170. <IDS> ::= <ROLLBACK WORK> */
  Rule_IDS98,
  /* 171. <IDS> ::= <SET CONSTRAINTS> */
  Rule_IDS99,
  /* 172. <IDS> ::= <SET INDEXES> */
  Rule_IDS100,
  /* 173. <IDS> ::= <SET TRIGGERS> */
  Rule_IDS101,
  /* 174. <IDS> ::= <SET LOG> */
  Rule_IDS102,
  /* 175. <IDS> ::= <SET PLOAD FILE> */
  Rule_IDS103,
  /* 176. <IDS> ::= <START VIOLATIONS TABLE> */
  Rule_IDS104,
  /* 177. <IDS> ::= <STOP VIOLATIONS TABLE> */
  Rule_IDS105,
  /* 178. <IDS> ::= <SAVEPOINT> */
  Rule_IDS106,
  /* 179. <IDS> ::= <SAVE EXTERNAL DIRECTIVES> */
  Rule_IDS107,
  /* 180. <IDS> ::= <SET ALL_MUTABLES> */
  Rule_IDS108,
  /* 181. <IDS> ::= <SET Default Table Space> */
  Rule_IDS109,
  /* 182. <IDS> ::= <SET Default Table Type> */
  Rule_IDS110,
  /* 183. <IDS> ::= <SET ENVIRONMENT> */
  Rule_IDS111,
  /* 184. <IDS> ::= <SET EXPLAIN> */
  Rule_IDS112,
  /* 185. <IDS> ::= <SET OPTIMIZATION> */
  Rule_IDS113,
  /* 186. <IDS> ::= <SET PDQPRIORITY> */
  Rule_IDS114,
  /* 187. <IDS> ::= <SET Residency> */
  Rule_IDS115,
  /* 188. <IDS> ::= <SET SCHEDULE LEVEL> */
  Rule_IDS116,
  /* 189. <IDS> ::= <SET STATEMENT CACHE> */
  Rule_IDS117,
  /* 190. <IDS> ::= <UPDATE STATISTICS> */
  Rule_IDS118,
  /* 191. <IDS> ::= <EXECUTE FUNCTION> */
  Rule_IDS119,
  /* 192. <IDS> ::= <EXECUTE PROCEDURE> */
  Rule_IDS120,
  /* 193. <IDS> ::= <SET DEBUG FILE TO> */
  Rule_IDS121,
  /* 194. <IDS> ::= <OUTPUT> */
  Rule_IDS122,
  /* 195. <IDS> ::= <GET DIAGNOSTICS> */
  Rule_IDS123,
  /* 196. <IDS> ::= <SET COLLATION> */
  Rule_IDS124,
  /* 197. <IDS> ::= <SET DATASKIP> */
  Rule_IDS125,
  /* 198. <IDS> ::= <SET ENCRYPTION PASSWORD> */
  Rule_IDS126,
  /* 199. <IDS> ::= <WHENEVER> */
  Rule_IDS127,
  /* 200. <IDS> ::= <CONNECT> */
  Rule_IDS128,
  /* 201. <IDS> ::= <DATABASE> */
  Rule_IDS129,
  /* 202. <IDS> ::= <DISCONNECT> */
  Rule_IDS130,
  /* 203. <IDS> ::= <SET CONNECTION> */
  Rule_IDS131,
  /* 204. <IDS> ::= <SPL> */
  Rule_IDS132,
  /* 205. <IDS> ::= <DISKINIT> */
  Rule_IDS133,
  /* 206. <IDS> ::= <onbar> */
  Rule_IDS134,
  /* 207. <IDS> ::= <ontape> */
  Rule_IDS135,
  /* 208. <IDS> ::= <oncheck> */
  Rule_IDS136,
  /* 209. <IDS> ::= <onmode> */
  Rule_IDS137,
  /* 210. <IDS> ::= <onparams> */
  Rule_IDS138,
  /* 211. <IDS> ::= <ondblog> */
  Rule_IDS139,
  /* 212. <IDS> ::= <onlog> */
  Rule_IDS140,
  /* 213. <IDS> ::= <onaudit> */
  Rule_IDS141,
  /* 214. <SPL> ::= <Label> */
  Rule_SPL,
  /* 215. <SPL> ::= <CALL> */
  Rule_SPL2,
  /* 216. <SPL> ::= <CASE> */
  Rule_SPL3,
  /* 217. <SPL> ::= <CONTINUE> */
  Rule_SPL4,
  /* 218. <SPL> ::= <EXIT> */
  Rule_SPL5,
  /* 219. <SPL> ::= <FOR> */
  Rule_SPL6,
  /* 220. <SPL> ::= <FOREACH> */
  Rule_SPL7,
  /* 221. <SPL> ::= <GOTO> */
  Rule_SPL8,
  /* 222. <SPL> ::= <IF> */
  Rule_SPL9,
  /* 223. <SPL> ::= <LET> */
  Rule_SPL10,
  /* 224. <SPL> ::= <ON EXCEPTION> */
  Rule_SPL11,
  /* 225. <SPL> ::= <RAISE EXCEPTION> */
  Rule_SPL12,
  /* 226. <SPL> ::= <RETURN> */
  Rule_SPL13,
  /* 227. <SPL> ::= <SYSTEM> */
  Rule_SPL14,
  /* 228. <SPL> ::= <TRACE> */
  Rule_SPL15,
  /* 229. <SPL> ::= <WHILE> */
  Rule_SPL16,
  /* 230. <ALTER ACCESS_METHOD> ::= ALTER 'ACCESS_METHOD' Id <ALTER ACCESS_METHODoptionS> */
  Rule_ALTERACCESS_METHOD_ALTER_ACCESS_METHOD_Id,
  /* 231. <ALTER ACCESS_METHODoptionS> ::= <ALTER ACCESS_METHODoption> <ALTER ACCESS_METHODoptionS_> */
  Rule_ALTERACCESS_METHODoptionS,
  /* 232. <ALTER ACCESS_METHODoptionS_> ::= ',' <ALTER ACCESS_METHODoption> <ALTER ACCESS_METHODoptionS_> */
  Rule_ALTERACCESS_METHODoptionS__Comma,
  /* 233. <ALTER ACCESS_METHODoptionS_> ::=  */
  Rule_ALTERACCESS_METHODoptionS_,
  /* 234. <ALTER ACCESS_METHODoption> ::= MODIFY <PurposeOption> */
  Rule_ALTERACCESS_METHODoption_MODIFY,
  /* 235. <ALTER ACCESS_METHODoption> ::= ADD <PurposeOption> */
  Rule_ALTERACCESS_METHODoption_ADD,
  /* 236. <ALTER ACCESS_METHODoption> ::= DROP <PurposeKeyword> */
  Rule_ALTERACCESS_METHODoption_DROP,
  /* 237. <PurposeKeyword> ::= 'am_sptype' */
  Rule_PurposeKeyword_am_sptype,
  /* 238. <PurposeKeyword> ::= 'am_defopclass' */
  Rule_PurposeKeyword_am_defopclass,
  /* 239. <PurposeKeyword> ::= 'am_keyscan' */
  Rule_PurposeKeyword_am_keyscan,
  /* 240. <PurposeKeyword> ::= 'am_unique' */
  Rule_PurposeKeyword_am_unique,
  /* 241. <PurposeKeyword> ::= 'am_cluster' */
  Rule_PurposeKeyword_am_cluster,
  /* 242. <PurposeKeyword> ::= 'am_rowids' */
  Rule_PurposeKeyword_am_rowids,
  /* 243. <PurposeKeyword> ::= 'am_readwrite' */
  Rule_PurposeKeyword_am_readwrite,
  /* 244. <PurposeKeyword> ::= 'am_parallel' */
  Rule_PurposeKeyword_am_parallel,
  /* 245. <PurposeKeyword> ::= 'am_costfactor' */
  Rule_PurposeKeyword_am_costfactor,
  /* 246. <PurposeKeyword> ::= 'am_create' */
  Rule_PurposeKeyword_am_create,
  /* 247. <PurposeKeyword> ::= 'am_drop' */
  Rule_PurposeKeyword_am_drop,
  /* 248. <PurposeKeyword> ::= 'am_open' */
  Rule_PurposeKeyword_am_open,
  /* 249. <PurposeKeyword> ::= 'am_close' */
  Rule_PurposeKeyword_am_close,
  /* 250. <PurposeKeyword> ::= 'am_insert' */
  Rule_PurposeKeyword_am_insert,
  /* 251. <PurposeKeyword> ::= 'am_delete' */
  Rule_PurposeKeyword_am_delete,
  /* 252. <PurposeKeyword> ::= 'am_update' */
  Rule_PurposeKeyword_am_update,
  /* 253. <PurposeKeyword> ::= 'am_stats' */
  Rule_PurposeKeyword_am_stats,
  /* 254. <PurposeKeyword> ::= 'am_scancost' */
  Rule_PurposeKeyword_am_scancost,
  /* 255. <PurposeKeyword> ::= 'am_check' */
  Rule_PurposeKeyword_am_check,
  /* 256. <PurposeKeyword> ::= 'am_beginscan' */
  Rule_PurposeKeyword_am_beginscan,
  /* 257. <PurposeKeyword> ::= 'am_endscan' */
  Rule_PurposeKeyword_am_endscan,
  /* 258. <PurposeKeyword> ::= 'am_rescan' */
  Rule_PurposeKeyword_am_rescan,
  /* 259. <PurposeKeyword> ::= 'am_getnext' */
  Rule_PurposeKeyword_am_getnext,
  /* 260. <PurposeKeyword> ::= 'am_getbyid' */
  Rule_PurposeKeyword_am_getbyid,
  /* 261. <PurposeKeyword> ::= 'am_truncate' */
  Rule_PurposeKeyword_am_truncate,
  /* 262. <PurposeOption> ::= <PurposeKeyword> '=' Id */
  Rule_PurposeOption_Eq_Id,
  /* 263. <PurposeOption> ::= <PurposeKeyword> '=' StringLiteral */
  Rule_PurposeOption_Eq_StringLiteral,
  /* 264. <PurposeOption> ::= <PurposeKeyword> */
  Rule_PurposeOption,
  /* 265. <PurposeOption> ::= <PurposeKeyword> '=' IntegerLiteral */
  Rule_PurposeOption_Eq_IntegerLiteral,
  /* 266. <PurposeOption> ::= <PurposeKeyword> '=' RealLiteral */
  Rule_PurposeOption_Eq_RealLiteral,
  /* 267. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_attach> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id,
  /* 268. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_detach> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id2,
  /* 269. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_init> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id3,
  /* 270. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_add> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id4,
  /* 271. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_drop> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id5,
  /* 272. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON TABLE Id <ALTER FRAGMENT_modify> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_TABLE_Id6,
  /* 273. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_init> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id,
  /* 274. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_add> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id2,
  /* 275. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_drop> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id3,
  /* 276. <ALTER FRAGMENT> ::= ALTER FRAGMENT ON INDEX Id <ALTER FRAGMENT_modify> */
  Rule_ALTERFRAGMENT_ALTER_FRAGMENT_ON_INDEX_Id4,
  /* 277. <ALTER FRAGMENT_attach> ::= ATTACH <ALTER FRAGMENT_attach_S> */
  Rule_ALTERFRAGMENT_attach_ATTACH,
  /* 278. <ALTER FRAGMENT_attach_S> ::= <ALTER FRAGMENT_attach_> <ALTER FRAGMENT_attach_S_> */
  Rule_ALTERFRAGMENT_attach_S,
  /* 279. <ALTER FRAGMENT_attach_S_> ::= ',' <ALTER FRAGMENT_attach_> <ALTER FRAGMENT_attach_S_> */
  Rule_ALTERFRAGMENT_attach_S__Comma,
  /* 280. <ALTER FRAGMENT_attach_S_> ::=  */
  Rule_ALTERFRAGMENT_attach_S_,
  /* 281. <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> AFTER Id */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_AFTER_Id,
  /* 282. <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> Id */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_Id,
  /* 283. <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> BEFORE Id */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_BEFORE_Id,
  /* 284. <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id <Expression> */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id,
  /* 285. <ALTER FRAGMENT_attach_0> ::= Id AS PARTITION Id REMAINDER */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_PARTITION_Id_REMAINDER,
  /* 286. <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> AFTER Id */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_AFTER_Id,
  /* 287. <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> Id */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_Id,
  /* 288. <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> BEFORE Id */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_BEFORE_Id,
  /* 289. <ALTER FRAGMENT_attach_0> ::= Id AS <Expression> */
  Rule_ALTERFRAGMENT_attach_0_Id_AS,
  /* 290. <ALTER FRAGMENT_attach_0> ::= Id AS REMAINDER */
  Rule_ALTERFRAGMENT_attach_0_Id_AS_REMAINDER,
  /* 291. <ALTER FRAGMENT_attach_> ::= <ALTER FRAGMENT_attach_0> */
  Rule_ALTERFRAGMENT_attach_,
  /* 292. <ALTER FRAGMENT_attach_> ::= Id */
  Rule_ALTERFRAGMENT_attach__Id,
  /* 293. <ALTER FRAGMENT_detach> ::= DETACH PARTITION Id Id */
  Rule_ALTERFRAGMENT_detach_DETACH_PARTITION_Id_Id,
  /* 294. <ALTER FRAGMENT_detach> ::= DETACH Id Id */
  Rule_ALTERFRAGMENT_detach_DETACH_Id_Id,
  /* 295. <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> <FRAGMENT BY_TABLE0> */
  Rule_ALTERFRAGMENT_init_INIT,
  /* 296. <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> <FRAGMENT BY_TABLE1> */
  Rule_ALTERFRAGMENT_init_INIT2,
  /* 297. <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> PARTITION Id IN Id */
  Rule_ALTERFRAGMENT_init_INIT_PARTITION_Id_IN_Id,
  /* 298. <ALTER FRAGMENT_init> ::= INIT <ROWIDSopt> IN Id */
  Rule_ALTERFRAGMENT_init_INIT_IN_Id,
  /* 299. <ROWIDSopt> ::= WITH ROWIDS */
  Rule_ROWIDSopt_WITH_ROWIDS,
  /* 300. <ROWIDSopt> ::=  */
  Rule_ROWIDSopt,
  /* 301. <FRAGMENT BY_TABLE1> ::= <FRAGMENT BY_INDEXopt1> EXPRESSION <FRAGMENT BY_Expr List1> */
  Rule_FRAGMENTBY_TABLE1_EXPRESSION,
  /* 302. <FRAGMENT BY_Expr List1> ::= <FRAGMENT BY_Expr1> <FRAGMENT BY_Expr List1_> */
  Rule_FRAGMENTBY_ExprList1,
  /* 303. <FRAGMENT BY_Expr List1_> ::= ',' <FRAGMENT BY_Expr1> <FRAGMENT BY_Expr List1_> */
  Rule_FRAGMENTBY_ExprList1__Comma,
  /* 304. <FRAGMENT BY_Expr List1_> ::=  */
  Rule_FRAGMENTBY_ExprList1_,
  /* 305. <FRAGMENT BY_Expr1> ::= PARTITION Id REMAINDER IN Id */
  Rule_FRAGMENTBY_Expr1_PARTITION_Id_REMAINDER_IN_Id,
  /* 306. <FRAGMENT BY_Expr1> ::= REMAINDER IN Id */
  Rule_FRAGMENTBY_Expr1_REMAINDER_IN_Id,
  /* 307. <ALTER FRAGMENT_add> ::= ADD <FRAGMENT BY_Expr1> */
  Rule_ALTERFRAGMENT_add_ADD,
  /* 308. <ALTER FRAGMENT_add> ::= ADD PARTITION Id REMAINDER IN PARTITION Id IN Id */
  Rule_ALTERFRAGMENT_add_ADD_PARTITION_Id_REMAINDER_IN_PARTITION_Id_IN_Id,
  /* 309. <ALTER FRAGMENT_add> ::= ADD REMAINDER IN PARTITION Id IN Id */
  Rule_ALTERFRAGMENT_add_ADD_REMAINDER_IN_PARTITION_Id_IN_Id,
  /* 310. <ALTER FRAGMENT_add> ::= ADD PARTITION Id IN Id */
  Rule_ALTERFRAGMENT_add_ADD_PARTITION_Id_IN_Id,
  /* 311. <ALTER FRAGMENT_add> ::= ADD IN Id */
  Rule_ALTERFRAGMENT_add_ADD_IN_Id,
  /* 312. <ALTER FRAGMENT_add> ::= ADD <ALTER FRAGMENT_add_0> */
  Rule_ALTERFRAGMENT_add_ADD2,
  /* 313. <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> IN Id AFTER Id */
  Rule_ALTERFRAGMENT_add_0_PARTITION_Id_IN_Id_AFTER_Id,
  /* 314. <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> IN Id Id */
  Rule_ALTERFRAGMENT_add_0_PARTITION_Id_IN_Id_Id,
  /* 315. <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> IN Id BEFORE Id */
  Rule_ALTERFRAGMENT_add_0_PARTITION_Id_IN_Id_BEFORE_Id,
  /* 316. <ALTER FRAGMENT_add_0> ::= PARTITION Id <Expression> */
  Rule_ALTERFRAGMENT_add_0_PARTITION_Id,
  /* 317. <ALTER FRAGMENT_add_0> ::= <Expression> IN Id AFTER Id */
  Rule_ALTERFRAGMENT_add_0_IN_Id_AFTER_Id,
  /* 318. <ALTER FRAGMENT_add_0> ::= <Expression> IN Id Id */
  Rule_ALTERFRAGMENT_add_0_IN_Id_Id,
  /* 319. <ALTER FRAGMENT_add_0> ::= <Expression> IN Id BEFORE Id */
  Rule_ALTERFRAGMENT_add_0_IN_Id_BEFORE_Id,
  /* 320. <ALTER FRAGMENT_add_0> ::= <Expression> */
  Rule_ALTERFRAGMENT_add_0,
  /* 321. <ALTER FRAGMENT_drop> ::= DROP PARTITION Id */
  Rule_ALTERFRAGMENT_drop_DROP_PARTITION_Id,
  /* 322. <ALTER FRAGMENT_drop> ::= DROP Id */
  Rule_ALTERFRAGMENT_drop_DROP_Id,
  /* 323. <ALTER FRAGMENT_modify> ::= MODIFY <ALTER FRAGMENT_modify_S> */
  Rule_ALTERFRAGMENT_modify_MODIFY,
  /* 324. <ALTER FRAGMENT_modify_S> ::= <ALTER FRAGMENT_modify_> <ALTER FRAGMENT_modify_S_> */
  Rule_ALTERFRAGMENT_modify_S,
  /* 325. <ALTER FRAGMENT_modify_S_> ::= ',' <ALTER FRAGMENT_modify_> <ALTER FRAGMENT_modify_S_> */
  Rule_ALTERFRAGMENT_modify_S__Comma,
  /* 326. <ALTER FRAGMENT_modify_S_> ::=  */
  Rule_ALTERFRAGMENT_modify_S_,
  /* 327. <ALTER FRAGMENT_modify_> ::= PARTITION Id TO PARTITION Id <Expression> IN Id */
  Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_PARTITION_Id_IN_Id,
  /* 328. <ALTER FRAGMENT_modify_> ::= Id TO PARTITION Id <Expression> IN Id */
  Rule_ALTERFRAGMENT_modify__Id_TO_PARTITION_Id_IN_Id,
  /* 329. <ALTER FRAGMENT_modify_> ::= PARTITION Id TO <Expression> IN Id */
  Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_IN_Id,
  /* 330. <ALTER FRAGMENT_modify_> ::= Id TO <Expression> IN Id */
  Rule_ALTERFRAGMENT_modify__Id_TO_IN_Id,
  /* 331. <ALTER FRAGMENT_modify_> ::= PARTITION Id TO PARTITION Id REMAINDER IN Id */
  Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_PARTITION_Id_REMAINDER_IN_Id,
  /* 332. <ALTER FRAGMENT_modify_> ::= Id TO PARTITION Id REMAINDER IN Id */
  Rule_ALTERFRAGMENT_modify__Id_TO_PARTITION_Id_REMAINDER_IN_Id,
  /* 333. <ALTER FRAGMENT_modify_> ::= PARTITION Id TO REMAINDER IN Id */
  Rule_ALTERFRAGMENT_modify__PARTITION_Id_TO_REMAINDER_IN_Id,
  /* 334. <ALTER FRAGMENT_modify_> ::= Id TO REMAINDER IN Id */
  Rule_ALTERFRAGMENT_modify__Id_TO_REMAINDER_IN_Id,
  /* 335. <ALTER FUNCTION> ::= ALTER FUNCTION Id '(' <TypeList> ')' WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERFUNCTION_ALTER_FUNCTION_Id_LParan_RParan_WITH_LParan_RParan,
  /* 336. <ALTER FUNCTION> ::= ALTER FUNCTION Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERFUNCTION_ALTER_FUNCTION_Id_WITH_LParan_RParan,
  /* 337. <ALTER FUNCTION> ::= ALTER SPECIFIC FUNCTION Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERFUNCTION_ALTER_SPECIFIC_FUNCTION_Id_WITH_LParan_RParan,
  /* 338. <TypeList> ::= <Type> <TypeList_> */
  Rule_TypeList,
  /* 339. <TypeList_> ::= ',' <Type> <TypeList_> */
  Rule_TypeList__Comma,
  /* 340. <TypeList_> ::=  */
  Rule_TypeList_,
  /* 341. <Type> ::= CHAR '(' IntegerLiteral ')' */
  Rule_Type_CHAR_LParan_IntegerLiteral_RParan,
  /* 342. <Type> ::= CHARACTER '(' IntegerLiteral ')' */
  Rule_Type_CHARACTER_LParan_IntegerLiteral_RParan,
  /* 343. <Type> ::= NCHAR '(' IntegerLiteral ')' */
  Rule_Type_NCHAR_LParan_IntegerLiteral_RParan,
  /* 344. <Type> ::= VARCHAR '(' IntegerLiteral ',' IntegerLiteral ')' */
  Rule_Type_VARCHAR_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan,
  /* 345. <Type> ::= CHARACTER VARYING '(' IntegerLiteral ',' IntegerLiteral ')' */
  Rule_Type_CHARACTER_VARYING_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan,
  /* 346. <Type> ::= NVARCHAR '(' IntegerLiteral ',' IntegerLiteral ')' */
  Rule_Type_NVARCHAR_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan,
  /* 347. <Type> ::= VARCHAR '(' IntegerLiteral ')' */
  Rule_Type_VARCHAR_LParan_IntegerLiteral_RParan,
  /* 348. <Type> ::= CHARACTER VARYING '(' IntegerLiteral ')' */
  Rule_Type_CHARACTER_VARYING_LParan_IntegerLiteral_RParan,
  /* 349. <Type> ::= NVARCHAR '(' IntegerLiteral ')' */
  Rule_Type_NVARCHAR_LParan_IntegerLiteral_RParan,
  /* 350. <Type> ::= VARCHAR */
  Rule_Type_VARCHAR,
  /* 351. <Type> ::= CHARACTER VARYING */
  Rule_Type_CHARACTER_VARYING,
  /* 352. <Type> ::= NVARCHAR */
  Rule_Type_NVARCHAR,
  /* 353. <Type> ::= LVARCHAR '(' IntegerLiteral ')' */
  Rule_Type_LVARCHAR_LParan_IntegerLiteral_RParan,
  /* 354. <Type> ::= LVARCHAR */
  Rule_Type_LVARCHAR,
  /* 355. <Type> ::= DECIMAL <NUMERICopt> */
  Rule_Type_DECIMAL,
  /* 356. <Type> ::= DEC <NUMERICopt> */
  Rule_Type_DEC,
  /* 357. <Type> ::= NUMERIC <NUMERICopt> */
  Rule_Type_NUMERIC,
  /* 358. <Type> ::= MONEY <NUMERICopt> */
  Rule_Type_MONEY,
  /* 359. <Type> ::= INT */
  Rule_Type_INT,
  /* 360. <Type> ::= INTEGER */
  Rule_Type_INTEGER,
  /* 361. <Type> ::= 'INT8' */
  Rule_Type_INT8,
  /* 362. <Type> ::= BIGINT */
  Rule_Type_BIGINT,
  /* 363. <Type> ::= SMAILLINT */
  Rule_Type_SMAILLINT,
  /* 364. <Type> ::= BIGSERIAL '(' IntegerLiteral ')' */
  Rule_Type_BIGSERIAL_LParan_IntegerLiteral_RParan,
  /* 365. <Type> ::= SERIAL '(' IntegerLiteral ')' */
  Rule_Type_SERIAL_LParan_IntegerLiteral_RParan,
  /* 366. <Type> ::= 'SERIAL8' '(' IntegerLiteral ')' */
  Rule_Type_SERIAL8_LParan_IntegerLiteral_RParan,
  /* 367. <Type> ::= BIGSERIAL */
  Rule_Type_BIGSERIAL,
  /* 368. <Type> ::= SERIAL */
  Rule_Type_SERIAL,
  /* 369. <Type> ::= 'SERIAL8' */
  Rule_Type_SERIAL8,
  /* 370. <Type> ::= FLOAT '(' IntegerLiteral ')' */
  Rule_Type_FLOAT_LParan_IntegerLiteral_RParan,
  /* 371. <Type> ::= DOUBLE PRECISION '(' IntegerLiteral ')' */
  Rule_Type_DOUBLE_PRECISION_LParan_IntegerLiteral_RParan,
  /* 372. <Type> ::= FLOAT */
  Rule_Type_FLOAT,
  /* 373. <Type> ::= DOUBLE PRECISION */
  Rule_Type_DOUBLE_PRECISION,
  /* 374. <Type> ::= SMALLFLOAT */
  Rule_Type_SMALLFLOAT,
  /* 375. <Type> ::= REAL */
  Rule_Type_REAL,
  /* 376. <Type> ::= TEXT <TEXTopt> */
  Rule_Type_TEXT,
  /* 377. <Type> ::= BYTE <TEXTopt> */
  Rule_Type_BYTE,
  /* 378. <Type> ::= BLOB */
  Rule_Type_BLOB,
  /* 379. <Type> ::= CLOB */
  Rule_Type_CLOB,
  /* 380. <Type> ::= DATE */
  Rule_Type_DATE,
  /* 381. <Type> ::= INTERVAL <INTERVALopt> */
  Rule_Type_INTERVAL,
  /* 382. <Type> ::= DATETIME <DATETIMEopt> */
  Rule_Type_DATETIME,
  /* 383. <Type> ::= BOOLEAN */
  Rule_Type_BOOLEAN,
  /* 384. <Type> ::= IDSSECURITYLABEL */
  Rule_Type_IDSSECURITYLABEL,
  /* 385. <Type> ::= Id */
  Rule_Type_Id,
  /* 386. <Type> ::= ROW '(' <TypeList> ')' */
  Rule_Type_ROW_LParan_RParan,
  /* 387. <Type> ::= ROW */
  Rule_Type_ROW,
  /* 388. <Type> ::= COLLECTION */
  Rule_Type_COLLECTION,
  /* 389. <Type> ::= SET <SETopt> */
  Rule_Type_SET,
  /* 390. <Type> ::= MULTISET <SETopt> */
  Rule_Type_MULTISET,
  /* 391. <Type> ::= LIST <SETopt> */
  Rule_Type_LIST,
  /* 392. <NUMERICopt> ::= '(' IntegerLiteral ',' IntegerLiteral ')' */
  Rule_NUMERICopt_LParan_IntegerLiteral_Comma_IntegerLiteral_RParan,
  /* 393. <NUMERICopt> ::= '(' IntegerLiteral ')' */
  Rule_NUMERICopt_LParan_IntegerLiteral_RParan,
  /* 394. <NUMERICopt> ::=  */
  Rule_NUMERICopt,
  /* 395. <TEXTopt> ::= IN TABLE */
  Rule_TEXTopt_IN_TABLE,
  /* 396. <TEXTopt> ::= IN Id */
  Rule_TEXTopt_IN_Id,
  /* 397. <TEXTopt> ::=  */
  Rule_TEXTopt,
  /* 398. <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO DAY */
  Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_DAY,
  /* 399. <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO HOUR */
  Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_HOUR,
  /* 400. <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO MINUTE */
  Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_MINUTE,
  /* 401. <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO SECOND */
  Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_SECOND,
  /* 402. <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 403. <INTERVALopt> ::= DAY '(' IntegerLiteral ')' TO FRACTION */
  Rule_INTERVALopt_DAY_LParan_IntegerLiteral_RParan_TO_FRACTION,
  /* 404. <INTERVALopt> ::= DAY TO DAY */
  Rule_INTERVALopt_DAY_TO_DAY,
  /* 405. <INTERVALopt> ::= DAY TO HOUR */
  Rule_INTERVALopt_DAY_TO_HOUR,
  /* 406. <INTERVALopt> ::= DAY TO MINUTE */
  Rule_INTERVALopt_DAY_TO_MINUTE,
  /* 407. <INTERVALopt> ::= DAY TO SECOND */
  Rule_INTERVALopt_DAY_TO_SECOND,
  /* 408. <INTERVALopt> ::= DAY TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_DAY_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 409. <INTERVALopt> ::= DAY TO FRACTION */
  Rule_INTERVALopt_DAY_TO_FRACTION,
  /* 410. <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO HOUR */
  Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_HOUR,
  /* 411. <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO MINUTE */
  Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_MINUTE,
  /* 412. <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO SECOND */
  Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_SECOND,
  /* 413. <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 414. <INTERVALopt> ::= HOUR '(' IntegerLiteral ')' TO FRACTION */
  Rule_INTERVALopt_HOUR_LParan_IntegerLiteral_RParan_TO_FRACTION,
  /* 415. <INTERVALopt> ::= HOUR TO HOUR */
  Rule_INTERVALopt_HOUR_TO_HOUR,
  /* 416. <INTERVALopt> ::= HOUR TO MINUTE */
  Rule_INTERVALopt_HOUR_TO_MINUTE,
  /* 417. <INTERVALopt> ::= HOUR TO SECOND */
  Rule_INTERVALopt_HOUR_TO_SECOND,
  /* 418. <INTERVALopt> ::= HOUR TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_HOUR_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 419. <INTERVALopt> ::= HOUR TO FRACTION */
  Rule_INTERVALopt_HOUR_TO_FRACTION,
  /* 420. <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO MINUTE */
  Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_MINUTE,
  /* 421. <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO SECOND */
  Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_SECOND,
  /* 422. <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 423. <INTERVALopt> ::= MINUTE '(' IntegerLiteral ')' TO FRACTION */
  Rule_INTERVALopt_MINUTE_LParan_IntegerLiteral_RParan_TO_FRACTION,
  /* 424. <INTERVALopt> ::= MINUTE TO MINUTE */
  Rule_INTERVALopt_MINUTE_TO_MINUTE,
  /* 425. <INTERVALopt> ::= MINUTE TO SECOND */
  Rule_INTERVALopt_MINUTE_TO_SECOND,
  /* 426. <INTERVALopt> ::= MINUTE TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_MINUTE_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 427. <INTERVALopt> ::= MINUTE TO FRACTION */
  Rule_INTERVALopt_MINUTE_TO_FRACTION,
  /* 428. <INTERVALopt> ::= SECOND '(' IntegerLiteral ')' TO SECOND */
  Rule_INTERVALopt_SECOND_LParan_IntegerLiteral_RParan_TO_SECOND,
  /* 429. <INTERVALopt> ::= SECOND '(' IntegerLiteral ')' TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_SECOND_LParan_IntegerLiteral_RParan_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 430. <INTERVALopt> ::= SECOND '(' IntegerLiteral ')' TO FRACTION */
  Rule_INTERVALopt_SECOND_LParan_IntegerLiteral_RParan_TO_FRACTION,
  /* 431. <INTERVALopt> ::= SECOND TO SECOND */
  Rule_INTERVALopt_SECOND_TO_SECOND,
  /* 432. <INTERVALopt> ::= SECOND TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_SECOND_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 433. <INTERVALopt> ::= SECOND TO FRACTION */
  Rule_INTERVALopt_SECOND_TO_FRACTION,
  /* 434. <INTERVALopt> ::= FRACTION TO FRACTION '(' IntegerLiteral ')' */
  Rule_INTERVALopt_FRACTION_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 435. <INTERVALopt> ::= FRACTION TO FRACTION */
  Rule_INTERVALopt_FRACTION_TO_FRACTION,
  /* 436. <INTERVALopt> ::= YEAR '(' IntegerLiteral ')' TO YEAR */
  Rule_INTERVALopt_YEAR_LParan_IntegerLiteral_RParan_TO_YEAR,
  /* 437. <INTERVALopt> ::= YEAR '(' IntegerLiteral ')' TO MONTH */
  Rule_INTERVALopt_YEAR_LParan_IntegerLiteral_RParan_TO_MONTH,
  /* 438. <INTERVALopt> ::= YEAR TO YEAR */
  Rule_INTERVALopt_YEAR_TO_YEAR,
  /* 439. <INTERVALopt> ::= YEAR TO MONTH */
  Rule_INTERVALopt_YEAR_TO_MONTH,
  /* 440. <INTERVALopt> ::= MONTH '(' IntegerLiteral ')' TO MONTH */
  Rule_INTERVALopt_MONTH_LParan_IntegerLiteral_RParan_TO_MONTH,
  /* 441. <INTERVALopt> ::= MONTH TO MONTH */
  Rule_INTERVALopt_MONTH_TO_MONTH,
  /* 442. <INTERVALopt> ::=  */
  Rule_INTERVALopt,
  /* 443. <DATETIMEopt> ::= YEAR TO YEAR */
  Rule_DATETIMEopt_YEAR_TO_YEAR,
  /* 444. <DATETIMEopt> ::= YEAR TO MONTH */
  Rule_DATETIMEopt_YEAR_TO_MONTH,
  /* 445. <DATETIMEopt> ::= YEAR TO DAY */
  Rule_DATETIMEopt_YEAR_TO_DAY,
  /* 446. <DATETIMEopt> ::= YEAR TO HOUR */
  Rule_DATETIMEopt_YEAR_TO_HOUR,
  /* 447. <DATETIMEopt> ::= YEAR TO MINUTE */
  Rule_DATETIMEopt_YEAR_TO_MINUTE,
  /* 448. <DATETIMEopt> ::= YEAR TO SECOND */
  Rule_DATETIMEopt_YEAR_TO_SECOND,
  /* 449. <DATETIMEopt> ::= YEAR TO FRACTION '(' IntegerLiteral ')' */
  Rule_DATETIMEopt_YEAR_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 450. <DATETIMEopt> ::= YEAR TO FRACTION */
  Rule_DATETIMEopt_YEAR_TO_FRACTION,
  /* 451. <DATETIMEopt> ::= MONTH TO MONTH */
  Rule_DATETIMEopt_MONTH_TO_MONTH,
  /* 452. <DATETIMEopt> ::= MONTH TO DAY */
  Rule_DATETIMEopt_MONTH_TO_DAY,
  /* 453. <DATETIMEopt> ::= MONTH TO HOUR */
  Rule_DATETIMEopt_MONTH_TO_HOUR,
  /* 454. <DATETIMEopt> ::= MONTH TO MINUTE */
  Rule_DATETIMEopt_MONTH_TO_MINUTE,
  /* 455. <DATETIMEopt> ::= MONTH TO SECOND */
  Rule_DATETIMEopt_MONTH_TO_SECOND,
  /* 456. <DATETIMEopt> ::= MONTH TO FRACTION '(' IntegerLiteral ')' */
  Rule_DATETIMEopt_MONTH_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 457. <DATETIMEopt> ::= MONTH TO FRACTION */
  Rule_DATETIMEopt_MONTH_TO_FRACTION,
  /* 458. <DATETIMEopt> ::= DAY TO DAY */
  Rule_DATETIMEopt_DAY_TO_DAY,
  /* 459. <DATETIMEopt> ::= DAY TO HOUR */
  Rule_DATETIMEopt_DAY_TO_HOUR,
  /* 460. <DATETIMEopt> ::= DAY TO MINUTE */
  Rule_DATETIMEopt_DAY_TO_MINUTE,
  /* 461. <DATETIMEopt> ::= DAY TO SECOND */
  Rule_DATETIMEopt_DAY_TO_SECOND,
  /* 462. <DATETIMEopt> ::= DAY TO FRACTION '(' IntegerLiteral ')' */
  Rule_DATETIMEopt_DAY_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 463. <DATETIMEopt> ::= DAY TO FRACTION */
  Rule_DATETIMEopt_DAY_TO_FRACTION,
  /* 464. <DATETIMEopt> ::= HOUR TO HOUR */
  Rule_DATETIMEopt_HOUR_TO_HOUR,
  /* 465. <DATETIMEopt> ::= HOUR TO MINUTE */
  Rule_DATETIMEopt_HOUR_TO_MINUTE,
  /* 466. <DATETIMEopt> ::= HOUR TO SECOND */
  Rule_DATETIMEopt_HOUR_TO_SECOND,
  /* 467. <DATETIMEopt> ::= HOUR TO FRACTION '(' IntegerLiteral ')' */
  Rule_DATETIMEopt_HOUR_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 468. <DATETIMEopt> ::= HOUR TO FRACTION */
  Rule_DATETIMEopt_HOUR_TO_FRACTION,
  /* 469. <DATETIMEopt> ::= MINUTE TO MINUTE */
  Rule_DATETIMEopt_MINUTE_TO_MINUTE,
  /* 470. <DATETIMEopt> ::= MINUTE TO SECOND */
  Rule_DATETIMEopt_MINUTE_TO_SECOND,
  /* 471. <DATETIMEopt> ::= MINUTE TO FRACTION '(' IntegerLiteral ')' */
  Rule_DATETIMEopt_MINUTE_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 472. <DATETIMEopt> ::= MINUTE TO FRACTION */
  Rule_DATETIMEopt_MINUTE_TO_FRACTION,
  /* 473. <DATETIMEopt> ::= SECOND TO SECOND */
  Rule_DATETIMEopt_SECOND_TO_SECOND,
  /* 474. <DATETIMEopt> ::= SECOND TO FRACTION '(' IntegerLiteral ')' */
  Rule_DATETIMEopt_SECOND_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 475. <DATETIMEopt> ::= SECOND TO FRACTION */
  Rule_DATETIMEopt_SECOND_TO_FRACTION,
  /* 476. <DATETIMEopt> ::= FRACTION TO FRACTION '(' IntegerLiteral ')' */
  Rule_DATETIMEopt_FRACTION_TO_FRACTION_LParan_IntegerLiteral_RParan,
  /* 477. <DATETIMEopt> ::= FRACTION TO FRACTION */
  Rule_DATETIMEopt_FRACTION_TO_FRACTION,
  /* 478. <DATETIMEopt> ::=  */
  Rule_DATETIMEopt,
  /* 479. <SETopt> ::= '(' <Type> NOT NULL ')' */
  Rule_SETopt_LParan_NOT_NULL_RParan,
  /* 480. <SETopt> ::= '(' <Type> ')' */
  Rule_SETopt_LParan_RParan,
  /* 481. <ALTER FUNCTION_WITHs> ::= <ALTER FUNCTION_WITH> <ALTER FUNCTION_WITHs_> */
  Rule_ALTERFUNCTION_WITHs,
  /* 482. <ALTER FUNCTION_WITHs_> ::= ',' <ALTER FUNCTION_WITH> <ALTER FUNCTION_WITHs_> */
  Rule_ALTERFUNCTION_WITHs__Comma,
  /* 483. <ALTER FUNCTION_WITHs_> ::=  */
  Rule_ALTERFUNCTION_WITHs_,
  /* 484. <ALTER FUNCTION_WITH> ::= ADD <SPLdescriptor1> */
  Rule_ALTERFUNCTION_WITH_ADD,
  /* 485. <ALTER FUNCTION_WITH> ::= MODIFY <SPLdescriptor1> */
  Rule_ALTERFUNCTION_WITH_MODIFY,
  /* 486. <ALTER FUNCTION_WITH> ::= DROP <SPLdescriptor2> */
  Rule_ALTERFUNCTION_WITH_DROP,
  /* 487. <ALTER FUNCTION_WITH> ::= MODIFY EXTERNAL NAME StringLiteral */
  Rule_ALTERFUNCTION_WITH_MODIFY_EXTERNAL_NAME_StringLiteral,
  /* 488. <SPLdescriptor1> ::= NOT VARIANT */
  Rule_SPLdescriptor1_NOT_VARIANT,
  /* 489. <SPLdescriptor1> ::= VARIANT */
  Rule_SPLdescriptor1_VARIANT,
  /* 490. <SPLdescriptor1> ::= NEGATOR '=' Id */
  Rule_SPLdescriptor1_NEGATOR_Eq_Id,
  /* 491. <SPLdescriptor1> ::= CLASS '=' Id */
  Rule_SPLdescriptor1_CLASS_Eq_Id,
  /* 492. <SPLdescriptor1> ::= ITERATOR */
  Rule_SPLdescriptor1_ITERATOR,
  /* 493. <SPLdescriptor1> ::= PARALLELIZABLE */
  Rule_SPLdescriptor1_PARALLELIZABLE,
  /* 494. <SPLdescriptor1> ::= HANDLESNULLS */
  Rule_SPLdescriptor1_HANDLESNULLS,
  /* 495. <SPLdescriptor1> ::= INTERNAL */
  Rule_SPLdescriptor1_INTERNAL,
  /* 496. <SPLdescriptor1> ::= 'PERCALL_COST' '=' IntegerLiteral */
  Rule_SPLdescriptor1_PERCALL_COST_Eq_IntegerLiteral,
  /* 497. <SPLdescriptor1> ::= COSTFUNC '=' Id */
  Rule_SPLdescriptor1_COSTFUNC_Eq_Id,
  /* 498. <SPLdescriptor1> ::= SELFUNC '=' Id */
  Rule_SPLdescriptor1_SELFUNC_Eq_Id,
  /* 499. <SPLdescriptor1> ::= SELCONST '=' IntegerLiteral */
  Rule_SPLdescriptor1_SELCONST_Eq_IntegerLiteral,
  /* 500. <SPLdescriptor1> ::= STACK '=' IntegerLiteral */
  Rule_SPLdescriptor1_STACK_Eq_IntegerLiteral,
  /* 501. <SPLdescriptor2> ::= NOT VARIANT */
  Rule_SPLdescriptor2_NOT_VARIANT,
  /* 502. <SPLdescriptor2> ::= VARIANT */
  Rule_SPLdescriptor2_VARIANT,
  /* 503. <SPLdescriptor2> ::= NEGATOR */
  Rule_SPLdescriptor2_NEGATOR,
  /* 504. <SPLdescriptor2> ::= CLASS */
  Rule_SPLdescriptor2_CLASS,
  /* 505. <SPLdescriptor2> ::= ITERATOR */
  Rule_SPLdescriptor2_ITERATOR,
  /* 506. <SPLdescriptor2> ::= PARALLELIZABLE */
  Rule_SPLdescriptor2_PARALLELIZABLE,
  /* 507. <SPLdescriptor2> ::= HANDLESNULLS */
  Rule_SPLdescriptor2_HANDLESNULLS,
  /* 508. <SPLdescriptor2> ::= INTERNAL */
  Rule_SPLdescriptor2_INTERNAL,
  /* 509. <SPLdescriptor2> ::= 'PERCALL_COST' */
  Rule_SPLdescriptor2_PERCALL_COST,
  /* 510. <SPLdescriptor2> ::= SELFUNC */
  Rule_SPLdescriptor2_SELFUNC,
  /* 511. <SPLdescriptor2> ::= SELCONST */
  Rule_SPLdescriptor2_SELCONST,
  /* 512. <SPLdescriptor2> ::= STACK */
  Rule_SPLdescriptor2_STACK,
  /* 513. <ALTER INDEX> ::= ALTER INDEX Id TO NOT CLUSTER */
  Rule_ALTERINDEX_ALTER_INDEX_Id_TO_NOT_CLUSTER,
  /* 514. <ALTER INDEX> ::= ALTER INDEX Id TO CLUSTER */
  Rule_ALTERINDEX_ALTER_INDEX_Id_TO_CLUSTER,
  /* 515. <ALTER INDEX> ::= ALTER INDEX Id <IndexLockmode> */
  Rule_ALTERINDEX_ALTER_INDEX_Id,
  /* 516. <ALTER PROCEDURE> ::= ALTER PROCEDURE Id '(' <TypeList> ')' WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERPROCEDURE_ALTER_PROCEDURE_Id_LParan_RParan_WITH_LParan_RParan,
  /* 517. <ALTER PROCEDURE> ::= ALTER PROCEDURE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERPROCEDURE_ALTER_PROCEDURE_Id_WITH_LParan_RParan,
  /* 518. <ALTER PROCEDURE> ::= ALTER SPECIFIC PROCEDURE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERPROCEDURE_ALTER_SPECIFIC_PROCEDURE_Id_WITH_LParan_RParan,
  /* 519. <ALTER ROUTINE> ::= ALTER ROUTINE Id '(' <TypeList> ')' WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERROUTINE_ALTER_ROUTINE_Id_LParan_RParan_WITH_LParan_RParan,
  /* 520. <ALTER ROUTINE> ::= ALTER ROUTINE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERROUTINE_ALTER_ROUTINE_Id_WITH_LParan_RParan,
  /* 521. <ALTER ROUTINE> ::= ALTER SPECIFIC ROUTINE Id WITH '(' <ALTER FUNCTION_WITHs> ')' */
  Rule_ALTERROUTINE_ALTER_SPECIFIC_ROUTINE_Id_WITH_LParan_RParan,
  /* 522. <ALTER SEQUENCE> ::= ALTER SEQUENCE Id <ALTER SEQUENCEoptionS> */
  Rule_ALTERSEQUENCE_ALTER_SEQUENCE_Id,
  /* 523. <ALTER SEQUENCEoptionS> ::= <ALTER SEQUENCEoption> <ALTER SEQUENCEoptionS_> */
  Rule_ALTERSEQUENCEoptionS,
  /* 524. <ALTER SEQUENCEoptionS_> ::= ',' <ALTER SEQUENCEoption> <ALTER SEQUENCEoptionS_> */
  Rule_ALTERSEQUENCEoptionS__Comma,
  /* 525. <ALTER SEQUENCEoptionS_> ::=  */
  Rule_ALTERSEQUENCEoptionS_,
  /* 526. <ALTER SEQUENCEoption> ::= NOCYCLE */
  Rule_ALTERSEQUENCEoption_NOCYCLE,
  /* 527. <ALTER SEQUENCEoption> ::= CYCLE */
  Rule_ALTERSEQUENCEoption_CYCLE,
  /* 528. <ALTER SEQUENCEoption> ::= CACHE IntegerLiteral */
  Rule_ALTERSEQUENCEoption_CACHE_IntegerLiteral,
  /* 529. <ALTER SEQUENCEoption> ::= NOCACHE */
  Rule_ALTERSEQUENCEoption_NOCACHE,
  /* 530. <ALTER SEQUENCEoption> ::= ORDER */
  Rule_ALTERSEQUENCEoption_ORDER,
  /* 531. <ALTER SEQUENCEoption> ::= NOORDER */
  Rule_ALTERSEQUENCEoption_NOORDER,
  /* 532. <ALTER SEQUENCEoption> ::= INCREMENT BY IntegerLiteral */
  Rule_ALTERSEQUENCEoption_INCREMENT_BY_IntegerLiteral,
  /* 533. <ALTER SEQUENCEoption> ::= INCREMENT IntegerLiteral */
  Rule_ALTERSEQUENCEoption_INCREMENT_IntegerLiteral,
  /* 534. <ALTER SEQUENCEoption> ::= RESTART WITH IntegerLiteral */
  Rule_ALTERSEQUENCEoption_RESTART_WITH_IntegerLiteral,
  /* 535. <ALTER SEQUENCEoption> ::= RESTART IntegerLiteral */
  Rule_ALTERSEQUENCEoption_RESTART_IntegerLiteral,
  /* 536. <ALTER SEQUENCEoption> ::= NOMAXVALUE */
  Rule_ALTERSEQUENCEoption_NOMAXVALUE,
  /* 537. <ALTER SEQUENCEoption> ::= MAXVALUE IntegerLiteral */
  Rule_ALTERSEQUENCEoption_MAXVALUE_IntegerLiteral,
  /* 538. <ALTER SEQUENCEoption> ::= NOMINVALUE */
  Rule_ALTERSEQUENCEoption_NOMINVALUE,
  /* 539. <ALTER SEQUENCEoption> ::= MINVALUE IntegerLiteral */
  Rule_ALTERSEQUENCEoption_MINVALUE_IntegerLiteral,
  /* 540. <ALTER SECURITY LABEL COMPONENT> ::= ALTER SECURITY LABEL COMPONENT Id ADD ARRAY '[' <SECURITY LABEL COMPONENT_ARRAYs> ']' */
  Rule_ALTERSECURITYLABELCOMPONENT_ALTER_SECURITY_LABEL_COMPONENT_Id_ADD_ARRAY_LBracket_RBracket,
  /* 541. <ALTER SECURITY LABEL COMPONENT> ::= ALTER SECURITY LABEL COMPONENT Id ADD SET '{' <Id List> '}' */
  Rule_ALTERSECURITYLABELCOMPONENT_ALTER_SECURITY_LABEL_COMPONENT_Id_ADD_SET_LBrace_RBrace,
  /* 542. <ALTER SECURITY LABEL COMPONENT> ::= ALTER SECURITY LABEL COMPONENT Id ADD TREE '(' <SECURITY LABEL COMPONENT_TREEs> ')' */
  Rule_ALTERSECURITYLABELCOMPONENT_ALTER_SECURITY_LABEL_COMPONENT_Id_ADD_TREE_LParan_RParan,
  /* 543. <SECURITY LABEL COMPONENT_ARRAYs> ::= <SECURITY LABEL COMPONENT_ARRAY> <SECURITY LABEL COMPONENT_ARRAYs_> */
  Rule_SECURITYLABELCOMPONENT_ARRAYs,
  /* 544. <SECURITY LABEL COMPONENT_ARRAYs_> ::= ',' <SECURITY LABEL COMPONENT_ARRAY> <SECURITY LABEL COMPONENT_ARRAYs_> */
  Rule_SECURITYLABELCOMPONENT_ARRAYs__Comma,
  /* 545. <SECURITY LABEL COMPONENT_ARRAYs_> ::=  */
  Rule_SECURITYLABELCOMPONENT_ARRAYs_,
  /* 546. <SECURITY LABEL COMPONENT_ARRAY> ::= <Id List> BEFORE Id */
  Rule_SECURITYLABELCOMPONENT_ARRAY_BEFORE_Id,
  /* 547. <SECURITY LABEL COMPONENT_ARRAY> ::= <Id List> AFTER Id */
  Rule_SECURITYLABELCOMPONENT_ARRAY_AFTER_Id,
  /* 548. <SECURITY LABEL COMPONENT_TREEs> ::= <SECURITY LABEL COMPONENT_TREE> <SECURITY LABEL COMPONENT_TREEs_> */
  Rule_SECURITYLABELCOMPONENT_TREEs,
  /* 549. <SECURITY LABEL COMPONENT_TREEs_> ::= ',' <SECURITY LABEL COMPONENT_TREE> <SECURITY LABEL COMPONENT_TREEs_> */
  Rule_SECURITYLABELCOMPONENT_TREEs__Comma,
  /* 550. <SECURITY LABEL COMPONENT_TREEs_> ::=  */
  Rule_SECURITYLABELCOMPONENT_TREEs_,
  /* 551. <SECURITY LABEL COMPONENT_TREE> ::= Id UNDER Id */
  Rule_SECURITYLABELCOMPONENT_TREE_Id_UNDER_Id,
  /* 552. <ALTER TABLE> ::= ALTER TABLE Id <ALTER TABLE_basetable> */
  Rule_ALTERTABLE_ALTER_TABLE_Id,
  /* 553. <ALTER TABLE> ::= ALTER TABLE Id <ALTER TABLE_LogType> */
  Rule_ALTERTABLE_ALTER_TABLE_Id2,
  /* 554. <ALTER TABLE_basetable> ::= <ALTER TABLE_AddColumn> */
  Rule_ALTERTABLE_basetable,
  /* 555. <ALTER TABLE_basetable> ::= <ALTER TABLE_AddConstraint> */
  Rule_ALTERTABLE_basetable2,
  /* 556. <ALTER TABLE_basetable> ::= <ALTER TABLE_Modify> */
  Rule_ALTERTABLE_basetable3,
  /* 557. <ALTER TABLE_basetable> ::= <ALTER TABLE_Drop> */
  Rule_ALTERTABLE_basetable4,
  /* 558. <ALTER TABLE_basetable> ::= <ALTER TABLE_DropConstraint> */
  Rule_ALTERTABLE_basetable5,
  /* 559. <ALTER TABLE_basetable> ::= <ALTER TABLE_ModifyNextSize> */
  Rule_ALTERTABLE_basetable6,
  /* 560. <ALTER TABLE_basetable> ::= <ALTER TABLE_LockMode> */
  Rule_ALTERTABLE_basetable7,
  /* 561. <ALTER TABLE_basetable> ::= <ALTER TABLE_Put> */
  Rule_ALTERTABLE_basetable8,
  /* 562. <ALTER TABLE_basetable> ::= <ALTER TABLE_SecurityPolicy> */
  Rule_ALTERTABLE_basetable9,
  /* 563. <ALTER TABLE_basetable> ::= <ALTER TABLE_AddType> */
  Rule_ALTERTABLE_basetable10,
  /* 564. <ALTER TABLE_basetable> ::= ADD ROWIDS */
  Rule_ALTERTABLE_basetable_ADD_ROWIDS,
  /* 565. <ALTER TABLE_basetable> ::= ADD CRCOLS */
  Rule_ALTERTABLE_basetable_ADD_CRCOLS,
  /* 566. <ALTER TABLE_basetable> ::= ADD VERCOLS */
  Rule_ALTERTABLE_basetable_ADD_VERCOLS,
  /* 567. <ALTER TABLE_basetable> ::= DROP ROWIDS */
  Rule_ALTERTABLE_basetable_DROP_ROWIDS,
  /* 568. <ALTER TABLE_basetable> ::= DROP CRCOLS */
  Rule_ALTERTABLE_basetable_DROP_CRCOLS,
  /* 569. <ALTER TABLE_basetable> ::= DROP VERCOLS */
  Rule_ALTERTABLE_basetable_DROP_VERCOLS,
  /* 570. <ALTER TABLE_AddColumn> ::= ADD '(' <ColumnDefineS> ')' */
  Rule_ALTERTABLE_AddColumn_ADD_LParan_RParan,
  /* 571. <ALTER TABLE_AddColumn> ::= ADD <ColumnDefine> */
  Rule_ALTERTABLE_AddColumn_ADD,
  /* 572. <ColumnDefineS> ::= <ColumnDefine> <ColumnDefineS_> */
  Rule_ColumnDefineS,
  /* 573. <ColumnDefineS_> ::= ',' <ColumnDefine> <ColumnDefineS_> */
  Rule_ColumnDefineS__Comma,
  /* 574. <ColumnDefineS_> ::=  */
  Rule_ColumnDefineS_,
  /* 575. <ColumnDefine> ::= Id <Type> <DEFAULT_clauseOpt> <column_constraintOpt> BEFORE Id <ColumnSecurityOpt> */
  Rule_ColumnDefine_Id_BEFORE_Id,
  /* 576. <ColumnDefine> ::= <ALTER TABLE_Modify_0> */
  Rule_ColumnDefine,
  /* 577. <DEFAULT_clauseOpt> ::= DEFAULT Id */
  Rule_DEFAULT_clauseOpt_DEFAULT_Id,
  /* 578. <DEFAULT_clauseOpt> ::= DEFAULT <constant_expression> */
  Rule_DEFAULT_clauseOpt_DEFAULT,
  /* 579. <DEFAULT_clauseOpt> ::= DEFAULT USER */
  Rule_DEFAULT_clauseOpt_DEFAULT_USER,
  /* 580. <DEFAULT_clauseOpt> ::= DEFAULT CURRENT <DATETIMEopt> */
  Rule_DEFAULT_clauseOpt_DEFAULT_CURRENT,
  /* 581. <DEFAULT_clauseOpt> ::= DEFAULT SYSDATE <DATETIMEopt> */
  Rule_DEFAULT_clauseOpt_DEFAULT_SYSDATE,
  /* 582. <DEFAULT_clauseOpt> ::= DEFAULT TODAY */
  Rule_DEFAULT_clauseOpt_DEFAULT_TODAY,
  /* 583. <DEFAULT_clauseOpt> ::= DEFAULT SITENAME */
  Rule_DEFAULT_clauseOpt_DEFAULT_SITENAME,
  /* 584. <DEFAULT_clauseOpt> ::= DEFAULT DBSERVERNAME */
  Rule_DEFAULT_clauseOpt_DEFAULT_DBSERVERNAME,
  /* 585. <DEFAULT_clauseOpt> ::=  */
  Rule_DEFAULT_clauseOpt,
  /* 586. <constant_expression> ::= <CONST And Exp> OR <constant_expression> */
  Rule_constant_expression_OR,
  /* 587. <constant_expression> ::= <CONST And Exp> */
  Rule_constant_expression,
  /* 588. <CONST And Exp> ::= <CONST Not Exp> AND <CONST And Exp> */
  Rule_CONSTAndExp_AND,
  /* 589. <CONST And Exp> ::= <CONST Not Exp> */
  Rule_CONSTAndExp,
  /* 590. <CONST Not Exp> ::= NOT <CONST Pred Exp> */
  Rule_CONSTNotExp_NOT,
  /* 591. <CONST Not Exp> ::= <CONST Pred Exp> */
  Rule_CONSTNotExp,
  /* 592. <CONST Pred Exp> ::= <CONST Add Exp> BETWEEN <CONST Add Exp> AND <CONST Add Exp> */
  Rule_CONSTPredExp_BETWEEN_AND,
  /* 593. <CONST Pred Exp> ::= <CONST Add Exp> NOT BETWEEN <CONST Add Exp> AND <CONST Add Exp> */
  Rule_CONSTPredExp_NOT_BETWEEN_AND,
  /* 594. <CONST Pred Exp> ::= <CONST Value> IS NOT NULL */
  Rule_CONSTPredExp_IS_NOT_NULL,
  /* 595. <CONST Pred Exp> ::= <CONST Value> IS NULL */
  Rule_CONSTPredExp_IS_NULL,
  /* 596. <CONST Pred Exp> ::= <CONST Add Exp> LIKE StringLiteral */
  Rule_CONSTPredExp_LIKE_StringLiteral,
  /* 597. <CONST Pred Exp> ::= <CONST Add Exp> IN <CONST Tuple> */
  Rule_CONSTPredExp_IN,
  /* 598. <CONST Pred Exp> ::= <CONST Add Exp> '=' <CONST Add Exp> */
  Rule_CONSTPredExp_Eq,
  /* 599. <CONST Pred Exp> ::= <CONST Add Exp> '<>' <CONST Add Exp> */
  Rule_CONSTPredExp_LtGt,
  /* 600. <CONST Pred Exp> ::= <CONST Add Exp> '!=' <CONST Add Exp> */
  Rule_CONSTPredExp_ExclamEq,
  /* 601. <CONST Pred Exp> ::= <CONST Add Exp> '>' <CONST Add Exp> */
  Rule_CONSTPredExp_Gt,
  /* 602. <CONST Pred Exp> ::= <CONST Add Exp> '>=' <CONST Add Exp> */
  Rule_CONSTPredExp_GtEq,
  /* 603. <CONST Pred Exp> ::= <CONST Add Exp> '<' <CONST Add Exp> */
  Rule_CONSTPredExp_Lt,
  /* 604. <CONST Pred Exp> ::= <CONST Add Exp> '<=' <CONST Add Exp> */
  Rule_CONSTPredExp_LtEq,
  /* 605. <CONST Pred Exp> ::= <CONST Add Exp> */
  Rule_CONSTPredExp,
  /* 606. <CONST Add Exp> ::= <CONST Add Exp> '+' <CONST Mult Exp> */
  Rule_CONSTAddExp_Plus,
  /* 607. <CONST Add Exp> ::= <CONST Add Exp> '-' <CONST Mult Exp> */
  Rule_CONSTAddExp_Minus,
  /* 608. <CONST Add Exp> ::= <CONST Mult Exp> */
  Rule_CONSTAddExp,
  /* 609. <CONST Mult Exp> ::= <CONST Mult Exp> '*' <CONST Negate Exp> */
  Rule_CONSTMultExp_Times,
  /* 610. <CONST Mult Exp> ::= <CONST Mult Exp> '/' <CONST Negate Exp> */
  Rule_CONSTMultExp_Div,
  /* 611. <CONST Mult Exp> ::= <CONST Negate Exp> */
  Rule_CONSTMultExp,
  /* 612. <CONST Negate Exp> ::= '-' <CONST Value> */
  Rule_CONSTNegateExp_Minus,
  /* 613. <CONST Negate Exp> ::= <CONST Value> */
  Rule_CONSTNegateExp,
  /* 614. <CONST Value> ::= <CONST Tuple> */
  Rule_CONSTValue,
  /* 615. <CONST Value> ::= <Value_Common> */
  Rule_CONSTValue2,
  /* 616. <CONST Value> ::= CASE <CASE_WHEN_THENs> ELSE <constant_expression> END */
  Rule_CONSTValue_CASE_ELSE_END,
  /* 617. <CONST Value> ::= CASE <CASE_WHEN_THENs> END */
  Rule_CONSTValue_CASE_END,
  /* 618. <CONST Value> ::= CASE <constant_expression> <CASE_WHEN_THENs> ELSE <constant_expression> END */
  Rule_CONSTValue_CASE_ELSE_END2,
  /* 619. <CONST Value> ::= CASE <constant_expression> <CASE_WHEN_THENs> END */
  Rule_CONSTValue_CASE_END2,
  /* 620. <CONST Tuple> ::= '(' <CONST Expr List> ')' */
  Rule_CONSTTuple_LParan_RParan,
  /* 621. <CONST Expr List> ::= <constant_expression> ',' <CONST Expr List> */
  Rule_CONSTExprList_Comma,
  /* 622. <CONST Expr List> ::= <constant_expression> */
  Rule_CONSTExprList,
  /* 623. <column_constraintOpt> ::= NOT NULL <column_constraint_CONSTRAINTopt> <column_constraint_S> */
  Rule_column_constraintOpt_NOT_NULL,
  /* 624. <column_constraintOpt> ::= NOT NULL <column_constraint_S> */
  Rule_column_constraintOpt_NOT_NULL2,
  /* 625. <column_constraintOpt> ::= <column_constraint_S> */
  Rule_column_constraintOpt,
  /* 626. <column_constraintOpt> ::=  */
  Rule_column_constraintOpt2,
  /* 627. <column_constraint_CONSTRAINTopt> ::= CONSTRAINT Id <IndexMode> */
  Rule_column_constraint_CONSTRAINTopt_CONSTRAINT_Id,
  /* 628. <column_constraint_CONSTRAINTopt> ::= CONSTRAINT Id */
  Rule_column_constraint_CONSTRAINTopt_CONSTRAINT_Id2,
  /* 629. <column_constraint_CONSTRAINTopt> ::= <IndexMode> */
  Rule_column_constraint_CONSTRAINTopt,
  /* 630. <column_constraint_CONSTRAINTopt> ::=  */
  Rule_column_constraint_CONSTRAINTopt2,
  /* 631. <column_constraint_S> ::= <column_constraint_> <column_constraint_CONSTRAINTopt> <column_constraint_S> */
  Rule_column_constraint_S,
  /* 632. <column_constraint_S> ::= <column_constraint_> <column_constraint_CONSTRAINTopt> */
  Rule_column_constraint_S2,
  /* 633. <column_constraint_> ::= UNIQUE */
  Rule_column_constraint__UNIQUE,
  /* 634. <column_constraint_> ::= DISTINCT */
  Rule_column_constraint__DISTINCT,
  /* 635. <column_constraint_> ::= PRIMARY KEY */
  Rule_column_constraint__PRIMARY_KEY,
  /* 636. <column_constraint_> ::= <REFERENCES_clause> */
  Rule_column_constraint_,
  /* 637. <column_constraint_> ::= <CHECK_clause> */
  Rule_column_constraint_2,
  /* 638. <REFERENCES_clause> ::= REFERENCES Id '(' <Id List> ')' ON DELETE CASCADE */
  Rule_REFERENCES_clause_REFERENCES_Id_LParan_RParan_ON_DELETE_CASCADE,
  /* 639. <REFERENCES_clause> ::= REFERENCES Id '(' <Id List> ')' */
  Rule_REFERENCES_clause_REFERENCES_Id_LParan_RParan,
  /* 640. <REFERENCES_clause> ::= REFERENCES Id ON DELETE CASCADE */
  Rule_REFERENCES_clause_REFERENCES_Id_ON_DELETE_CASCADE,
  /* 641. <REFERENCES_clause> ::= REFERENCES Id */
  Rule_REFERENCES_clause_REFERENCES_Id,
  /* 642. <CHECK_clause> ::= CHECK '(' <Expression> ')' */
  Rule_CHECK_clause_CHECK_LParan_RParan,
  /* 643. <ColumnSecurityOpt> ::= COLUMN SECURED WITH Id */
  Rule_ColumnSecurityOpt_COLUMN_SECURED_WITH_Id,
  /* 644. <ColumnSecurityOpt> ::= SECURED WITH Id */
  Rule_ColumnSecurityOpt_SECURED_WITH_Id,
  /* 645. <ColumnSecurityOpt> ::=  */
  Rule_ColumnSecurityOpt,
  /* 646. <ALTER TABLE_AddConstraint> ::= ADD CONSTRAINT '(' <table_constraintS> ')' */
  Rule_ALTERTABLE_AddConstraint_ADD_CONSTRAINT_LParan_RParan,
  /* 647. <ALTER TABLE_AddConstraint> ::= ADD CONSTRAINT <table_constraint> */
  Rule_ALTERTABLE_AddConstraint_ADD_CONSTRAINT,
  /* 648. <table_constraintS> ::= <table_constraint> <table_constraintS_> */
  Rule_table_constraintS,
  /* 649. <table_constraintS_> ::= ',' <table_constraint> <table_constraintS_> */
  Rule_table_constraintS__Comma,
  /* 650. <table_constraintS_> ::=  */
  Rule_table_constraintS_,
  /* 651. <table_constraint> ::= UNIQUE '(' <Id List> ')' <column_constraint_CONSTRAINTopt> */
  Rule_table_constraint_UNIQUE_LParan_RParan,
  /* 652. <table_constraint> ::= DISTINCT '(' <Id List> ')' <column_constraint_CONSTRAINTopt> */
  Rule_table_constraint_DISTINCT_LParan_RParan,
  /* 653. <table_constraint> ::= PRIMARY KEY '(' <Id List> ')' <column_constraint_CONSTRAINTopt> */
  Rule_table_constraint_PRIMARY_KEY_LParan_RParan,
  /* 654. <table_constraint> ::= <CHECK_clause> <column_constraint_CONSTRAINTopt> */
  Rule_table_constraint,
  /* 655. <table_constraint> ::= FOREIGN KEY '(' <Id List> ')' <REFERENCES_clause> <column_constraint_CONSTRAINTopt> */
  Rule_table_constraint_FOREIGN_KEY_LParan_RParan,
  /* 656. <ALTER TABLE_Modify> ::= MODIFY '(' <ALTER TABLE_Modify_S> ')' */
  Rule_ALTERTABLE_Modify_MODIFY_LParan_RParan,
  /* 657. <ALTER TABLE_Modify> ::= MODIFY <ALTER TABLE_Modify_> */
  Rule_ALTERTABLE_Modify_MODIFY,
  /* 658. <ALTER TABLE_Modify_S> ::= <ALTER TABLE_Modify_> <ALTER TABLE_Modify_S_> */
  Rule_ALTERTABLE_Modify_S,
  /* 659. <ALTER TABLE_Modify_S_> ::= ',' <ALTER TABLE_Modify_> <ALTER TABLE_Modify_S_> */
  Rule_ALTERTABLE_Modify_S__Comma,
  /* 660. <ALTER TABLE_Modify_S_> ::=  */
  Rule_ALTERTABLE_Modify_S_,
  /* 661. <ALTER TABLE_Modify_> ::= Id <Type> <DEFAULT_clauseOpt> <column_constraintOpt> DROP COLUMN SECURITY */
  Rule_ALTERTABLE_Modify__Id_DROP_COLUMN_SECURITY,
  /* 662. <ALTER TABLE_Modify_> ::= <ALTER TABLE_Modify_0> */
  Rule_ALTERTABLE_Modify_,
  /* 663. <ALTER TABLE_Modify_0> ::= Id <Type> <DEFAULT_clauseOpt> <column_constraintOpt> <ColumnSecurityOpt> */
  Rule_ALTERTABLE_Modify_0_Id,
  /* 664. <ALTER TABLE_Drop> ::= DROP '(' <Id List> ')' */
  Rule_ALTERTABLE_Drop_DROP_LParan_RParan,
  /* 665. <ALTER TABLE_Drop> ::= DROP Id */
  Rule_ALTERTABLE_Drop_DROP_Id,
  /* 666. <ALTER TABLE_DropConstraint> ::= DROP CONSTRAINT '(' <Id List> ')' */
  Rule_ALTERTABLE_DropConstraint_DROP_CONSTRAINT_LParan_RParan,
  /* 667. <ALTER TABLE_DropConstraint> ::= DROP CONSTRAINT Id */
  Rule_ALTERTABLE_DropConstraint_DROP_CONSTRAINT_Id,
  /* 668. <ALTER TABLE_ModifyNextSize> ::= MODIFY NEXT SIZE IntegerLiteral */
  Rule_ALTERTABLE_ModifyNextSize_MODIFY_NEXT_SIZE_IntegerLiteral,
  /* 669. <ALTER TABLE_LockMode> ::= LOCK MODE '(' PAGE ')' */
  Rule_ALTERTABLE_LockMode_LOCK_MODE_LParan_PAGE_RParan,
  /* 670. <ALTER TABLE_LockMode> ::= LOCK MODE '(' ROW ')' */
  Rule_ALTERTABLE_LockMode_LOCK_MODE_LParan_ROW_RParan,
  /* 671. <ALTER TABLE_LockMode> ::= LOCK MODE '(' TABLE ')' */
  Rule_ALTERTABLE_LockMode_LOCK_MODE_LParan_TABLE_RParan,
  /* 672. <ALTER TABLE_Put> ::= PUT Id IN '(' <Id List> ')' '(' <ALTER TABLE_PutOptions> ')' */
  Rule_ALTERTABLE_Put_PUT_Id_IN_LParan_RParan_LParan_RParan,
  /* 673. <ALTER TABLE_Put> ::= PUT Id IN '(' <Id List> ')' */
  Rule_ALTERTABLE_Put_PUT_Id_IN_LParan_RParan,
  /* 674. <ALTER TABLE_PutOptions> ::= <ALTER TABLE_PutOption> <ALTER TABLE_PutOptions_> */
  Rule_ALTERTABLE_PutOptions,
  /* 675. <ALTER TABLE_PutOptions_> ::= ',' <ALTER TABLE_PutOption> <ALTER TABLE_PutOptions_> */
  Rule_ALTERTABLE_PutOptions__Comma,
  /* 676. <ALTER TABLE_PutOptions_> ::=  */
  Rule_ALTERTABLE_PutOptions_,
  /* 677. <ALTER TABLE_PutOption> ::= EXTENT SIZE IntegerLiteral */
  Rule_ALTERTABLE_PutOption_EXTENT_SIZE_IntegerLiteral,
  /* 678. <ALTER TABLE_PutOption> ::= NO LOG */
  Rule_ALTERTABLE_PutOption_NO_LOG,
  /* 679. <ALTER TABLE_PutOption> ::= LOG */
  Rule_ALTERTABLE_PutOption_LOG,
  /* 680. <ALTER TABLE_PutOption> ::= HIGH INTEG */
  Rule_ALTERTABLE_PutOption_HIGH_INTEG,
  /* 681. <ALTER TABLE_PutOption> ::= NO KEEP ACCESS TIME */
  Rule_ALTERTABLE_PutOption_NO_KEEP_ACCESS_TIME,
  /* 682. <ALTER TABLE_PutOption> ::= KEEP ACCESS TIME */
  Rule_ALTERTABLE_PutOption_KEEP_ACCESS_TIME,
  /* 683. <ALTER TABLE_SecurityPolicy> ::= ADD SECURITY POLICY Id */
  Rule_ALTERTABLE_SecurityPolicy_ADD_SECURITY_POLICY_Id,
  /* 684. <ALTER TABLE_SecurityPolicy> ::= DROP SECURITY POLICY */
  Rule_ALTERTABLE_SecurityPolicy_DROP_SECURITY_POLICY,
  /* 685. <ALTER TABLE_AddType> ::= ADD TYPE Id */
  Rule_ALTERTABLE_AddType_ADD_TYPE_Id,
  /* 686. <ALTER TABLE_LogType> ::= TYPE '(' STANDARD ')' */
  Rule_ALTERTABLE_LogType_TYPE_LParan_STANDARD_RParan,
  /* 687. <ALTER TABLE_LogType> ::= TYPE '(' RAW ')' */
  Rule_ALTERTABLE_LogType_TYPE_LParan_RAW_RParan,
  /* 688. <ALTER TABLE_LogType> ::= TYPE '(' OPERATIONAL ')' */
  Rule_ALTERTABLE_LogType_TYPE_LParan_OPERATIONAL_RParan,
  /* 689. <ALTER TABLE_LogType> ::= TYPE '(' STATIC ')' */
  Rule_ALTERTABLE_LogType_TYPE_LParan_STATIC_RParan,
  /* 690. <CLOSE DATABASE> ::= CLOSE DATABASE */
  Rule_CLOSEDATABASE_CLOSE_DATABASE,
  /* 691. <CREATE ACCESS_METHOD> ::= CREATE PRIMARY 'ACCESS_METHOD' Id <PurposeOptionS> */
  Rule_CREATEACCESS_METHOD_CREATE_PRIMARY_ACCESS_METHOD_Id,
  /* 692. <CREATE ACCESS_METHOD> ::= CREATE SECONDARY 'ACCESS_METHOD' Id <PurposeOptionS> */
  Rule_CREATEACCESS_METHOD_CREATE_SECONDARY_ACCESS_METHOD_Id,
  /* 693. <PurposeOptionS> ::= <PurposeOption> <PurposeOptionS_> */
  Rule_PurposeOptionS,
  /* 694. <PurposeOptionS_> ::= ',' <PurposeOption> <PurposeOptionS_> */
  Rule_PurposeOptionS__Comma,
  /* 695. <PurposeOptionS_> ::=  */
  Rule_PurposeOptionS_,
  /* 696. <CREATE AGGREGATE> ::= CREATE AGGREGATE Id WITH '(' <CREATE AGGREGATE_OptionS> ')' */
  Rule_CREATEAGGREGATE_CREATE_AGGREGATE_Id_WITH_LParan_RParan,
  /* 697. <CREATE AGGREGATE_OptionS> ::= <CREATE AGGREGATE_Option> <CREATE AGGREGATE_OptionS_> */
  Rule_CREATEAGGREGATE_OptionS,
  /* 698. <CREATE AGGREGATE_OptionS_> ::= ',' <CREATE AGGREGATE_Option> <CREATE AGGREGATE_OptionS_> */
  Rule_CREATEAGGREGATE_OptionS__Comma,
  /* 699. <CREATE AGGREGATE_OptionS_> ::=  */
  Rule_CREATEAGGREGATE_OptionS_,
  /* 700. <CREATE AGGREGATE_Option> ::= INIT '=' Id */
  Rule_CREATEAGGREGATE_Option_INIT_Eq_Id,
  /* 701. <CREATE AGGREGATE_Option> ::= ITER '=' Id */
  Rule_CREATEAGGREGATE_Option_ITER_Eq_Id,
  /* 702. <CREATE AGGREGATE_Option> ::= COMBINE '=' Id */
  Rule_CREATEAGGREGATE_Option_COMBINE_Eq_Id,
  /* 703. <CREATE AGGREGATE_Option> ::= FINAL '=' Id */
  Rule_CREATEAGGREGATE_Option_FINAL_Eq_Id,
  /* 704. <CREATE AGGREGATE_Option> ::= HANDLESNULLS */
  Rule_CREATEAGGREGATE_Option_HANDLESNULLS,
  /* 705. <CREATE CAST> ::= CREATE EXPLICIT CAST '(' Id AS Id WITH Id ')' */
  Rule_CREATECAST_CREATE_EXPLICIT_CAST_LParan_Id_AS_Id_WITH_Id_RParan,
  /* 706. <CREATE CAST> ::= CREATE EXPLICIT CAST '(' Id AS Id ')' */
  Rule_CREATECAST_CREATE_EXPLICIT_CAST_LParan_Id_AS_Id_RParan,
  /* 707. <CREATE CAST> ::= CREATE IMPLICIT CAST '(' Id AS Id WITH Id ')' */
  Rule_CREATECAST_CREATE_IMPLICIT_CAST_LParan_Id_AS_Id_WITH_Id_RParan,
  /* 708. <CREATE CAST> ::= CREATE IMPLICIT CAST '(' Id AS Id ')' */
  Rule_CREATECAST_CREATE_IMPLICIT_CAST_LParan_Id_AS_Id_RParan,
  /* 709. <CREATE DATABASE> ::= CREATE DATABASE Id IN Id <CREATE DATABASE_WITHopt> */
  Rule_CREATEDATABASE_CREATE_DATABASE_Id_IN_Id,
  /* 710. <CREATE DATABASE> ::= CREATE DATABASE Id <CREATE DATABASE_WITHopt> */
  Rule_CREATEDATABASE_CREATE_DATABASE_Id,
  /* 711. <CREATE DATABASE_WITHopt> ::= WITH BUFFER LOG */
  Rule_CREATEDATABASE_WITHopt_WITH_BUFFER_LOG,
  /* 712. <CREATE DATABASE_WITHopt> ::= WITH LOG */
  Rule_CREATEDATABASE_WITHopt_WITH_LOG,
  /* 713. <CREATE DATABASE_WITHopt> ::= WITH LOG MODE ANSI */
  Rule_CREATEDATABASE_WITHopt_WITH_LOG_MODE_ANSI,
  /* 714. <CREATE DATABASE_WITHopt> ::=  */
  Rule_CREATEDATABASE_WITHopt,
  /* 715. <CREATE DISTINCT TYPE> ::= CREATE DISTINCT TYPE Id AS Id */
  Rule_CREATEDISTINCTTYPE_CREATE_DISTINCT_TYPE_Id_AS_Id,
  /* 716. <CREATE DUPLICATE> ::= CREATE DUPLICATE OF TABLE Id IN '(' <Id List> ')' */
  Rule_CREATEDUPLICATE_CREATE_DUPLICATE_OF_TABLE_Id_IN_LParan_RParan,
  /* 717. <CREATE DUPLICATE> ::= CREATE DUPLICATE OF TABLE Id IN Id */
  Rule_CREATEDUPLICATE_CREATE_DUPLICATE_OF_TABLE_Id_IN_Id,
  /* 718. <CREATE EXTERNAL TABLE> ::= CREATE EXTERNAL TABLE Id <CREATE EXTERNAL TABLE_fields> USING '(' <CREATE EXTERNAL TABLE_optionS> DATAFILES <StringLiteralS> <CREATE EXTERNAL TABLE_optionS> ')' */
  Rule_CREATEEXTERNALTABLE_CREATE_EXTERNAL_TABLE_Id_USING_LParan_DATAFILES_RParan,
  /* 719. <CREATE EXTERNAL TABLE_fields> ::= SAMEAS Id */
  Rule_CREATEEXTERNALTABLE_fields_SAMEAS_Id,
  /* 720. <CREATE EXTERNAL TABLE_fields> ::= <CREATE EXTERNAL TABLE_fields_S> */
  Rule_CREATEEXTERNALTABLE_fields,
  /* 721. <CREATE EXTERNAL TABLE_fields_S> ::= <CREATE EXTERNAL TABLE_fields_> <CREATE EXTERNAL TABLE_fields_S_> */
  Rule_CREATEEXTERNALTABLE_fields_S,
  /* 722. <CREATE EXTERNAL TABLE_fields_S_> ::= ',' <CREATE EXTERNAL TABLE_fields_> <CREATE EXTERNAL TABLE_fields_S_> */
  Rule_CREATEEXTERNALTABLE_fields_S__Comma,
  /* 723. <CREATE EXTERNAL TABLE_fields_S_> ::=  */
  Rule_CREATEEXTERNALTABLE_fields_S_,
  /* 724. <CREATE EXTERNAL TABLE_fields_> ::= Id <Type> <CREATE EXTERNAL TABLE_fields_Opt1> */
  Rule_CREATEEXTERNALTABLE_fields__Id,
  /* 725. <CREATE EXTERNAL TABLE_fields_Opt1> ::= EXTERNAL StringLiteral <DEFAULT_clauseOpt> <CREATE EXTERNAL TABLE_fields_Opt2> */
  Rule_CREATEEXTERNALTABLE_fields_Opt1_EXTERNAL_StringLiteral,
  /* 726. <CREATE EXTERNAL TABLE_fields_Opt1> ::= EXTERNAL <Type> NULL StringLiteral <DEFAULT_clauseOpt> <CREATE EXTERNAL TABLE_fields_Opt2> */
  Rule_CREATEEXTERNALTABLE_fields_Opt1_EXTERNAL_NULL_StringLiteral,
  /* 727. <CREATE EXTERNAL TABLE_fields_Opt1> ::= <DEFAULT_clauseOpt> <CREATE EXTERNAL TABLE_fields_Opt2> */
  Rule_CREATEEXTERNALTABLE_fields_Opt1,
  /* 728. <CREATE EXTERNAL TABLE_fields_Opt2> ::= NOT NULL CHECK '(' <Expression> ')' */
  Rule_CREATEEXTERNALTABLE_fields_Opt2_NOT_NULL_CHECK_LParan_RParan,
  /* 729. <CREATE EXTERNAL TABLE_fields_Opt2> ::= NOT NULL */
  Rule_CREATEEXTERNALTABLE_fields_Opt2_NOT_NULL,
  /* 730. <CREATE EXTERNAL TABLE_fields_Opt2> ::= CHECK '(' <Expression> ')' */
  Rule_CREATEEXTERNALTABLE_fields_Opt2_CHECK_LParan_RParan,
  /* 731. <CREATE EXTERNAL TABLE_fields_Opt2> ::=  */
  Rule_CREATEEXTERNALTABLE_fields_Opt2,
  /* 732. <CREATE EXTERNAL TABLE_optionS> ::= <CREATE EXTERNAL TABLE_option> <CREATE EXTERNAL TABLE_optionS_> */
  Rule_CREATEEXTERNALTABLE_optionS,
  /* 733. <CREATE EXTERNAL TABLE_optionS> ::=  */
  Rule_CREATEEXTERNALTABLE_optionS2,
  /* 734. <CREATE EXTERNAL TABLE_optionS_> ::= ',' <CREATE EXTERNAL TABLE_option> <CREATE EXTERNAL TABLE_optionS_> */
  Rule_CREATEEXTERNALTABLE_optionS__Comma,
  /* 735. <CREATE EXTERNAL TABLE_optionS_> ::=  */
  Rule_CREATEEXTERNALTABLE_optionS_,
  /* 736. <CREATE EXTERNAL TABLE_option> ::= DEFAULT */
  Rule_CREATEEXTERNALTABLE_option_DEFAULT,
  /* 737. <CREATE EXTERNAL TABLE_option> ::= EXPRESS */
  Rule_CREATEEXTERNALTABLE_option_EXPRESS,
  /* 738. <CREATE EXTERNAL TABLE_option> ::= DELUXE */
  Rule_CREATEEXTERNALTABLE_option_DELUXE,
  /* 739. <CREATE EXTERNAL TABLE_option> ::= MAXERRORS IntegerLiteral */
  Rule_CREATEEXTERNALTABLE_option_MAXERRORS_IntegerLiteral,
  /* 740. <CREATE EXTERNAL TABLE_option> ::= REJECTFILE StringLiteral */
  Rule_CREATEEXTERNALTABLE_option_REJECTFILE_StringLiteral,
  /* 741. <CREATE EXTERNAL TABLE_option> ::= SIZE IntegerLiteral */
  Rule_CREATEEXTERNALTABLE_option_SIZE_IntegerLiteral,
  /* 742. <CREATE EXTERNAL TABLE_option> ::= <SELECT TABLE_option> */
  Rule_CREATEEXTERNALTABLE_option,
  /* 743. <StringLiteralS> ::= StringLiteral <StringLiteralS_> */
  Rule_StringLiteralS_StringLiteral,
  /* 744. <StringLiteralS_> ::= ',' StringLiteral <StringLiteralS_> */
  Rule_StringLiteralS__Comma_StringLiteral,
  /* 745. <StringLiteralS_> ::=  */
  Rule_StringLiteralS_,
  /* 746. <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION,
  /* 747. <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION2,
  /* 748. <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION3,
  /* 749. <CREATE FUNCTION> ::= CREATE <DBAopt> FUNCTION Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE FUNCTION_return> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END FUNCTION <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEFUNCTION_CREATE_FUNCTION_Id_LParan_RParan_END_FUNCTION4,
  /* 750. <DBAopt> ::= DBA */
  Rule_DBAopt_DBA,
  /* 751. <DBAopt> ::=  */
  Rule_DBAopt,
  /* 752. <RoutineParams> ::= <RoutineParam> <RoutineParams_> */
  Rule_RoutineParams,
  /* 753. <RoutineParams_> ::= ',' <RoutineParam> <RoutineParams_> */
  Rule_RoutineParams__Comma,
  /* 754. <RoutineParams_> ::=  */
  Rule_RoutineParams_,
  /* 755. <RoutineParam> ::= Id <Type> DEFAULT <Expression> */
  Rule_RoutineParam_Id_DEFAULT,
  /* 756. <RoutineParam> ::= Id <Type> */
  Rule_RoutineParam_Id,
  /* 757. <RoutineParam> ::= Id LIKE Id DEFAULT <Expression> */
  Rule_RoutineParam_Id_LIKE_Id_DEFAULT,
  /* 758. <RoutineParam> ::= Id LIKE Id */
  Rule_RoutineParam_Id_LIKE_Id,
  /* 759. <RoutineParam> ::= Id REFERENCES BYTE DEFAULT NULL */
  Rule_RoutineParam_Id_REFERENCES_BYTE_DEFAULT_NULL,
  /* 760. <RoutineParam> ::= Id REFERENCES BYTE */
  Rule_RoutineParam_Id_REFERENCES_BYTE,
  /* 761. <RoutineParam> ::= Id REFERENCES TEXT DEFAULT NULL */
  Rule_RoutineParam_Id_REFERENCES_TEXT_DEFAULT_NULL,
  /* 762. <RoutineParam> ::= Id REFERENCES TEXT */
  Rule_RoutineParam_Id_REFERENCES_TEXT,
  /* 763. <RoutineParam> ::= OUT Id <Type> DEFAULT <Expression> */
  Rule_RoutineParam_OUT_Id_DEFAULT,
  /* 764. <RoutineParam> ::= OUT Id <Type> */
  Rule_RoutineParam_OUT_Id,
  /* 765. <RoutineParam> ::= OUT Id LIKE Id DEFAULT <Expression> */
  Rule_RoutineParam_OUT_Id_LIKE_Id_DEFAULT,
  /* 766. <RoutineParam> ::= OUT Id LIKE Id */
  Rule_RoutineParam_OUT_Id_LIKE_Id,
  /* 767. <RoutineParam> ::= OUT Id REFERENCES BYTE DEFAULT NULL */
  Rule_RoutineParam_OUT_Id_REFERENCES_BYTE_DEFAULT_NULL,
  /* 768. <RoutineParam> ::= OUT Id REFERENCES BYTE */
  Rule_RoutineParam_OUT_Id_REFERENCES_BYTE,
  /* 769. <RoutineParam> ::= OUT Id REFERENCES TEXT DEFAULT NULL */
  Rule_RoutineParam_OUT_Id_REFERENCES_TEXT_DEFAULT_NULL,
  /* 770. <RoutineParam> ::= OUT Id REFERENCES TEXT */
  Rule_RoutineParam_OUT_Id_REFERENCES_TEXT,
  /* 771. <RoutineParam> ::= INOUT Id <Type> DEFAULT <Expression> */
  Rule_RoutineParam_INOUT_Id_DEFAULT,
  /* 772. <RoutineParam> ::= INOUT Id <Type> */
  Rule_RoutineParam_INOUT_Id,
  /* 773. <RoutineParam> ::= INOUT Id LIKE Id DEFAULT <Expression> */
  Rule_RoutineParam_INOUT_Id_LIKE_Id_DEFAULT,
  /* 774. <RoutineParam> ::= INOUT Id LIKE Id */
  Rule_RoutineParam_INOUT_Id_LIKE_Id,
  /* 775. <RoutineParam> ::= INOUT Id REFERENCES BYTE DEFAULT NULL */
  Rule_RoutineParam_INOUT_Id_REFERENCES_BYTE_DEFAULT_NULL,
  /* 776. <RoutineParam> ::= INOUT Id REFERENCES BYTE */
  Rule_RoutineParam_INOUT_Id_REFERENCES_BYTE,
  /* 777. <RoutineParam> ::= INOUT Id REFERENCES TEXT DEFAULT NULL */
  Rule_RoutineParam_INOUT_Id_REFERENCES_TEXT_DEFAULT_NULL,
  /* 778. <RoutineParam> ::= INOUT Id REFERENCES TEXT */
  Rule_RoutineParam_INOUT_Id_REFERENCES_TEXT,
  /* 779. <CREATE FUNCTION_referencingOpt> ::= <referencing_clause> FOR Id */
  Rule_CREATEFUNCTION_referencingOpt_FOR_Id,
  /* 780. <CREATE FUNCTION_referencingOpt> ::=  */
  Rule_CREATEFUNCTION_referencingOpt,
  /* 781. <referencing_clause> ::= <referencing_clause_DELETE> */
  Rule_referencing_clause,
  /* 782. <referencing_clause> ::= <referencing_clause_INSERT> */
  Rule_referencing_clause2,
  /* 783. <referencing_clause> ::= <referencing_clause_UPDATE> */
  Rule_referencing_clause3,
  /* 784. <referencing_clause_DELETE> ::= REFERENCING OLD AS Id */
  Rule_referencing_clause_DELETE_REFERENCING_OLD_AS_Id,
  /* 785. <referencing_clause_DELETE> ::= REFERENCING OLD Id */
  Rule_referencing_clause_DELETE_REFERENCING_OLD_Id,
  /* 786. <referencing_clause_INSERT> ::= REFERENCING NEW AS Id */
  Rule_referencing_clause_INSERT_REFERENCING_NEW_AS_Id,
  /* 787. <referencing_clause_INSERT> ::= REFERENCING NEW Id */
  Rule_referencing_clause_INSERT_REFERENCING_NEW_Id,
  /* 788. <referencing_clause_UPDATE> ::= REFERENCING OLD AS Id NEW AS Id */
  Rule_referencing_clause_UPDATE_REFERENCING_OLD_AS_Id_NEW_AS_Id,
  /* 789. <referencing_clause_UPDATE> ::= REFERENCING OLD Id NEW AS Id */
  Rule_referencing_clause_UPDATE_REFERENCING_OLD_Id_NEW_AS_Id,
  /* 790. <referencing_clause_UPDATE> ::= REFERENCING OLD AS Id NEW Id */
  Rule_referencing_clause_UPDATE_REFERENCING_OLD_AS_Id_NEW_Id,
  /* 791. <referencing_clause_UPDATE> ::= REFERENCING OLD Id NEW Id */
  Rule_referencing_clause_UPDATE_REFERENCING_OLD_Id_NEW_Id,
  /* 792. <CREATE FUNCTION_return> ::= RETURNING <CREATE FUNCTION_return_S> <SemicolonOpt> */
  Rule_CREATEFUNCTION_return_RETURNING,
  /* 793. <CREATE FUNCTION_return> ::= RETURNS <CREATE FUNCTION_return_S> <SemicolonOpt> */
  Rule_CREATEFUNCTION_return_RETURNS,
  /* 794. <CREATE FUNCTION_return_S> ::= <CREATE FUNCTION_return_> <CREATE FUNCTION_return_S_> */
  Rule_CREATEFUNCTION_return_S,
  /* 795. <CREATE FUNCTION_return_S_> ::= ',' <CREATE FUNCTION_return_> <CREATE FUNCTION_return_S_> */
  Rule_CREATEFUNCTION_return_S__Comma,
  /* 796. <CREATE FUNCTION_return_S_> ::=  */
  Rule_CREATEFUNCTION_return_S_,
  /* 797. <CREATE FUNCTION_return_> ::= <Type> AS Id */
  Rule_CREATEFUNCTION_return__AS_Id,
  /* 798. <CREATE FUNCTION_return_> ::= <Type> */
  Rule_CREATEFUNCTION_return_,
  /* 799. <CREATE FUNCTION_return_> ::= REFERENCES BYTE AS Id */
  Rule_CREATEFUNCTION_return__REFERENCES_BYTE_AS_Id,
  /* 800. <CREATE FUNCTION_return_> ::= REFERENCES TEXT AS Id */
  Rule_CREATEFUNCTION_return__REFERENCES_TEXT_AS_Id,
  /* 801. <CREATE FUNCTION_return_> ::= REFERENCES BYTE */
  Rule_CREATEFUNCTION_return__REFERENCES_BYTE,
  /* 802. <CREATE FUNCTION_return_> ::= REFERENCES TEXT */
  Rule_CREATEFUNCTION_return__REFERENCES_TEXT,
  /* 803. <CREATE FUNCTION_specificOpt> ::= SPECIFIC Id */
  Rule_CREATEFUNCTION_specificOpt_SPECIFIC_Id,
  /* 804. <CREATE FUNCTION_specificOpt> ::=  */
  Rule_CREATEFUNCTION_specificOpt,
  /* 805. <CREATE FUNCTION_withOpt> ::= WITH '(' <CREATE FUNCTION_withS> ')' */
  Rule_CREATEFUNCTION_withOpt_WITH_LParan_RParan,
  /* 806. <CREATE FUNCTION_withOpt> ::=  */
  Rule_CREATEFUNCTION_withOpt,
  /* 807. <CREATE FUNCTION_withS> ::= <SPLdescriptor1> <CREATE FUNCTION_withS_> */
  Rule_CREATEFUNCTION_withS,
  /* 808. <CREATE FUNCTION_withS_> ::= ',' <SPLdescriptor1> <CREATE FUNCTION_withS_> */
  Rule_CREATEFUNCTION_withS__Comma,
  /* 809. <CREATE FUNCTION_withS_> ::=  */
  Rule_CREATEFUNCTION_withS_,
  /* 810. <SQLBlock> ::= <DEFINEs> <ON EXCEPTIONs> BEGIN <SQLs> END */
  Rule_SQLBlock_BEGIN_END,
  /* 811. <SQLBlock> ::= <DEFINEs> <ON EXCEPTIONs> <SQLs_> */
  Rule_SQLBlock,
  /* 812. <DEFINEs> ::= <DEFINE> <DEFINEs> */
  Rule_DEFINEs,
  /* 813. <DEFINEs> ::=  */
  Rule_DEFINEs2,
  /* 814. <ON EXCEPTIONs> ::= <ON EXCEPTION> <ON EXCEPTIONs> */
  Rule_ONEXCEPTIONs,
  /* 815. <ON EXCEPTIONs> ::=  */
  Rule_ONEXCEPTIONs2,
  /* 816. <CREATE FUNCTION_documentOpt> ::= DOCUMENT <StringLiteralS> */
  Rule_CREATEFUNCTION_documentOpt_DOCUMENT,
  /* 817. <CREATE FUNCTION_documentOpt> ::=  */
  Rule_CREATEFUNCTION_documentOpt,
  /* 818. <CREATE FUNCTION_listingOpt> ::= WITH LISTING IN StringLiteral */
  Rule_CREATEFUNCTION_listingOpt_WITH_LISTING_IN_StringLiteral,
  /* 819. <CREATE FUNCTION_listingOpt> ::=  */
  Rule_CREATEFUNCTION_listingOpt,
  /* 820. <ExternalRoutine> ::= EXTERNAL NAME StringLiteral LANGUAGE C <ExternalRoutineOpt1> <ExternalRoutineOpt2> */
  Rule_ExternalRoutine_EXTERNAL_NAME_StringLiteral_LANGUAGE_C,
  /* 821. <ExternalRoutine> ::= EXTERNAL NAME StringLiteral LANGUAGE JAVA <ExternalRoutineOpt1> <ExternalRoutineOpt2> */
  Rule_ExternalRoutine_EXTERNAL_NAME_StringLiteral_LANGUAGE_JAVA,
  /* 822. <ExternalRoutineOpt1> ::= PARAMETER STYLE INFORMIX */
  Rule_ExternalRoutineOpt1_PARAMETER_STYLE_INFORMIX,
  /* 823. <ExternalRoutineOpt1> ::= PARAMETER STYLE */
  Rule_ExternalRoutineOpt1_PARAMETER_STYLE,
  /* 824. <ExternalRoutineOpt1> ::=  */
  Rule_ExternalRoutineOpt1,
  /* 825. <ExternalRoutineOpt2> ::= NOT VARIANT */
  Rule_ExternalRoutineOpt2_NOT_VARIANT,
  /* 826. <ExternalRoutineOpt2> ::= VARIANT */
  Rule_ExternalRoutineOpt2_VARIANT,
  /* 827. <ExternalRoutineOpt2> ::=  */
  Rule_ExternalRoutineOpt2,
  /* 828. <CREATE INDEX> ::= CREATE <IndexType> <CREATE INDEX_indexScope> '(' <CREATE INDEX_indexKeyS> ')' <CREATE INDEX_Option> <IndexLockmodeOpt> <CREATE INDEX_onlineOpt> */
  Rule_CREATEINDEX_CREATE_LParan_RParan,
  /* 829. <CREATE INDEX> ::= CREATE GK <CREATE INDEX_indexScope> '(' <GK SELECT> ')' USING BITMAP <IndexLockmodeOpt> <CREATE INDEX_onlineOpt> */
  Rule_CREATEINDEX_CREATE_GK_LParan_RParan_USING_BITMAP,
  /* 830. <CREATE INDEX> ::= CREATE GK <CREATE INDEX_indexScope> '(' <GK SELECT> ')' <IndexLockmodeOpt> <CREATE INDEX_onlineOpt> */
  Rule_CREATEINDEX_CREATE_GK_LParan_RParan,
  /* 831. <IndexType> ::= DISTINCT CLUSTER */
  Rule_IndexType_DISTINCT_CLUSTER,
  /* 832. <IndexType> ::= DISTINCT */
  Rule_IndexType_DISTINCT,
  /* 833. <IndexType> ::= UNIQUE CLUSTER */
  Rule_IndexType_UNIQUE_CLUSTER,
  /* 834. <IndexType> ::= UNIQUE */
  Rule_IndexType_UNIQUE,
  /* 835. <IndexType> ::= CLUSTER */
  Rule_IndexType_CLUSTER,
  /* 836. <IndexType> ::=  */
  Rule_IndexType,
  /* 837. <CREATE INDEX_indexScope> ::= INDEX Id ON Id */
  Rule_CREATEINDEX_indexScope_INDEX_Id_ON_Id,
  /* 838. <CREATE INDEX_indexKeyS> ::= <CREATE INDEX_indexKey> <CREATE INDEX_indexKeyS_> */
  Rule_CREATEINDEX_indexKeyS,
  /* 839. <CREATE INDEX_indexKeyS_> ::= ',' <CREATE INDEX_indexKey> <CREATE INDEX_indexKeyS_> */
  Rule_CREATEINDEX_indexKeyS__Comma,
  /* 840. <CREATE INDEX_indexKeyS_> ::=  */
  Rule_CREATEINDEX_indexKeyS_,
  /* 841. <CREATE INDEX_indexKey> ::= Id Id <ASC_DESCopt> */
  Rule_CREATEINDEX_indexKey_Id_Id,
  /* 842. <CREATE INDEX_indexKey> ::= Id <ASC_DESCopt> */
  Rule_CREATEINDEX_indexKey_Id,
  /* 843. <CREATE INDEX_indexKey> ::= Id '(' <Id List> ')' Id <ASC_DESCopt> */
  Rule_CREATEINDEX_indexKey_Id_LParan_RParan_Id,
  /* 844. <CREATE INDEX_indexKey> ::= Id '(' <Id List> ')' <ASC_DESCopt> */
  Rule_CREATEINDEX_indexKey_Id_LParan_RParan,
  /* 845. <ASC_DESCopt> ::= ASC */
  Rule_ASC_DESCopt_ASC,
  /* 846. <ASC_DESCopt> ::= DESC */
  Rule_ASC_DESCopt_DESC,
  /* 847. <ASC_DESCopt> ::=  */
  Rule_ASC_DESCopt,
  /* 848. <CREATE INDEX_Option> ::= <Access-Method_clauseOpt> <FILLFACTORopt> <CREATE INDEX_StorageOpt> <IndexModeOpt> */
  Rule_CREATEINDEX_Option,
  /* 849. <Access-Method_clauseOpt> ::= USING Id '(' <RoutineParams2> ')' */
  Rule_AccessMethod_clauseOpt_USING_Id_LParan_RParan,
  /* 850. <Access-Method_clauseOpt> ::=  */
  Rule_AccessMethod_clauseOpt,
  /* 851. <FILLFACTORopt> ::= FILLFACTOR IntegerLiteral */
  Rule_FILLFACTORopt_FILLFACTOR_IntegerLiteral,
  /* 852. <FILLFACTORopt> ::=  */
  Rule_FILLFACTORopt,
  /* 853. <CREATE INDEX_StorageOpt0> ::= IN Id */
  Rule_CREATEINDEX_StorageOpt0_IN_Id,
  /* 854. <CREATE INDEX_StorageOpt> ::= <CREATE INDEX_StorageOpt0> */
  Rule_CREATEINDEX_StorageOpt,
  /* 855. <CREATE INDEX_StorageOpt> ::= IN TABLE */
  Rule_CREATEINDEX_StorageOpt_IN_TABLE,
  /* 856. <CREATE INDEX_StorageOpt> ::= <FRAGMENT BY_INDEX> */
  Rule_CREATEINDEX_StorageOpt2,
  /* 857. <CREATE INDEX_StorageOpt> ::= USING BITMAP */
  Rule_CREATEINDEX_StorageOpt_USING_BITMAP,
  /* 858. <CREATE INDEX_StorageOpt> ::=  */
  Rule_CREATEINDEX_StorageOpt3,
  /* 859. <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> EXPRESSION '(' <FRAGMENT BY_Expr List> ',' <FRAGMENT BY_REMAINDER> ')' */
  Rule_FRAGMENTBY_INDEX_EXPRESSION_LParan_Comma_RParan,
  /* 860. <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> HASH '(' <Id List> ')' IN Id */
  Rule_FRAGMENTBY_INDEX_HASH_LParan_RParan_IN_Id,
  /* 861. <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> HASH '(' <Id List> ')' IN '(' <Id List> ')' */
  Rule_FRAGMENTBY_INDEX_HASH_LParan_RParan_IN_LParan_RParan,
  /* 862. <FRAGMENT BY_INDEX> ::= <FRAGMENT BY_INDEXopt1> HYBRID '(' <Id List> ')' <FRAGMENT BY_EXPRESSION> */
  Rule_FRAGMENTBY_INDEX_HYBRID_LParan_RParan,
  /* 863. <FRAGMENT BY_INDEXopt1> ::= FRAGMENT BY */
  Rule_FRAGMENTBY_INDEXopt1_FRAGMENT_BY,
  /* 864. <FRAGMENT BY_INDEXopt1> ::= PARTITION BY */
  Rule_FRAGMENTBY_INDEXopt1_PARTITION_BY,
  /* 865. <FRAGMENT BY_Expr List> ::= <FRAGMENT BY_Expr> <FRAGMENT BY_Expr List_> */
  Rule_FRAGMENTBY_ExprList,
  /* 866. <FRAGMENT BY_Expr List_> ::= ',' <FRAGMENT BY_Expr> <FRAGMENT BY_Expr List_> */
  Rule_FRAGMENTBY_ExprList__Comma,
  /* 867. <FRAGMENT BY_Expr List_> ::=  */
  Rule_FRAGMENTBY_ExprList_,
  /* 868. <FRAGMENT BY_Expr> ::= PARTITION Id '(' <Expression> ')' IN Id */
  Rule_FRAGMENTBY_Expr_PARTITION_Id_LParan_RParan_IN_Id,
  /* 869. <FRAGMENT BY_Expr> ::= '(' <Expression> ')' IN Id */
  Rule_FRAGMENTBY_Expr_LParan_RParan_IN_Id,
  /* 870. <FRAGMENT BY_REMAINDER> ::= PARTITION Id REMAINDER IN Id */
  Rule_FRAGMENTBY_REMAINDER_PARTITION_Id_REMAINDER_IN_Id,
  /* 871. <FRAGMENT BY_REMAINDER> ::= REMAINDER IN Id */
  Rule_FRAGMENTBY_REMAINDER_REMAINDER_IN_Id,
  /* 872. <FRAGMENT BY_REMAINDER> ::= <FRAGMENT BY_Expr> */
  Rule_FRAGMENTBY_REMAINDER,
  /* 873. <FRAGMENT BY_EXPRESSION> ::= EXPRESSION <FRAGMENT BY_EXPRESSION1s> ',' <FRAGMENT BY_EXPRESSION2> */
  Rule_FRAGMENTBY_EXPRESSION_EXPRESSION_Comma,
  /* 874. <FRAGMENT BY_EXPRESSION1s> ::= <FRAGMENT BY_EXPRESSION1> <FRAGMENT BY_EXPRESSION1s_> */
  Rule_FRAGMENTBY_EXPRESSION1s,
  /* 875. <FRAGMENT BY_EXPRESSION1s_> ::= ',' <FRAGMENT BY_EXPRESSION1> <FRAGMENT BY_EXPRESSION1s_> */
  Rule_FRAGMENTBY_EXPRESSION1s__Comma,
  /* 876. <FRAGMENT BY_EXPRESSION1s_> ::=  */
  Rule_FRAGMENTBY_EXPRESSION1s_,
  /* 877. <FRAGMENT BY_EXPRESSION1> ::= <Expression> IN Id */
  Rule_FRAGMENTBY_EXPRESSION1_IN_Id,
  /* 878. <FRAGMENT BY_EXPRESSION1> ::= <Expression> IN '(' <Id List> ')' */
  Rule_FRAGMENTBY_EXPRESSION1_IN_LParan_RParan,
  /* 879. <FRAGMENT BY_EXPRESSION2> ::= <FRAGMENT BY_EXPRESSION1> */
  Rule_FRAGMENTBY_EXPRESSION2,
  /* 880. <FRAGMENT BY_EXPRESSION2> ::= REMAINDER IN '(' <Id List> ')' */
  Rule_FRAGMENTBY_EXPRESSION2_REMAINDER_IN_LParan_RParan,
  /* 881. <FRAGMENT BY_EXPRESSION2> ::= REMAINDER IN Id */
  Rule_FRAGMENTBY_EXPRESSION2_REMAINDER_IN_Id,
  /* 882. <IndexModeOpt> ::= <IndexMode> */
  Rule_IndexModeOpt,
  /* 883. <IndexModeOpt> ::=  */
  Rule_IndexModeOpt2,
  /* 884. <IndexMode> ::= ENABLED */
  Rule_IndexMode_ENABLED,
  /* 885. <IndexMode> ::= DISABLED */
  Rule_IndexMode_DISABLED,
  /* 886. <IndexMode> ::= FILTERING WITHOUT ERROR */
  Rule_IndexMode_FILTERING_WITHOUT_ERROR,
  /* 887. <IndexMode> ::= FILTERING WITH ERROR */
  Rule_IndexMode_FILTERING_WITH_ERROR,
  /* 888. <IndexLockmodeOpt> ::= <IndexLockmode> */
  Rule_IndexLockmodeOpt,
  /* 889. <IndexLockmodeOpt> ::=  */
  Rule_IndexLockmodeOpt2,
  /* 890. <IndexLockmode> ::= LOCK MODE NORMAL */
  Rule_IndexLockmode_LOCK_MODE_NORMAL,
  /* 891. <IndexLockmode> ::= LOCK MODE COARSE */
  Rule_IndexLockmode_LOCK_MODE_COARSE,
  /* 892. <CREATE INDEX_onlineOpt> ::= ONLINE */
  Rule_CREATEINDEX_onlineOpt_ONLINE,
  /* 893. <CREATE INDEX_onlineOpt> ::=  */
  Rule_CREATEINDEX_onlineOpt,
  /* 894. <GK SELECT> ::= SELECT ALL <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
  Rule_GKSELECT_SELECT_ALL,
  /* 895. <GK SELECT> ::= SELECT DISTINCT <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
  Rule_GKSELECT_SELECT_DISTINCT,
  /* 896. <GK SELECT> ::= SELECT UNIQUE <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
  Rule_GKSELECT_SELECT_UNIQUE,
  /* 897. <GK SELECT> ::= SELECT <Expr List> <GK SELECT_FROM> <GK SELECT_WHERE> */
  Rule_GKSELECT_SELECT,
  /* 898. <GK SELECT> ::= SELECT ALL <Expr List> <GK SELECT_FROM> */
  Rule_GKSELECT_SELECT_ALL2,
  /* 899. <GK SELECT> ::= SELECT DISTINCT <Expr List> <GK SELECT_FROM> */
  Rule_GKSELECT_SELECT_DISTINCT2,
  /* 900. <GK SELECT> ::= SELECT UNIQUE <Expr List> <GK SELECT_FROM> */
  Rule_GKSELECT_SELECT_UNIQUE2,
  /* 901. <GK SELECT> ::= SELECT <Expr List> <GK SELECT_FROM> */
  Rule_GKSELECT_SELECT2,
  /* 902. <GK SELECT_FROM> ::= FROM <Id List> */
  Rule_GKSELECT_FROM_FROM,
  /* 903. <GK SELECT_WHERE> ::= WHERE <Expression> */
  Rule_GKSELECT_WHERE_WHERE,
  /* 904. <CREATE OPAQUE TYPE> ::= CREATE OPAQUE TYPE Id '(' INTERNALLENGTH '=' IntegerLiteral <CREATE OPAQUE TYPE_optionS> ')' */
  Rule_CREATEOPAQUETYPE_CREATE_OPAQUE_TYPE_Id_LParan_INTERNALLENGTH_Eq_IntegerLiteral_RParan,
  /* 905. <CREATE OPAQUE TYPE> ::= CREATE OPAQUE TYPE Id '(' INTERNALLENGTH '=' VARIABLE <CREATE OPAQUE TYPE_optionS> ')' */
  Rule_CREATEOPAQUETYPE_CREATE_OPAQUE_TYPE_Id_LParan_INTERNALLENGTH_Eq_VARIABLE_RParan,
  /* 906. <CREATE OPAQUE TYPE_optionS> ::= ',' <CREATE OPAQUE TYPE_option> <CREATE OPAQUE TYPE_optionS> */
  Rule_CREATEOPAQUETYPE_optionS_Comma,
  /* 907. <CREATE OPAQUE TYPE_optionS> ::=  */
  Rule_CREATEOPAQUETYPE_optionS,
  /* 908. <CREATE OPAQUE TYPE_option> ::= MAXLEN '=' IntegerLiteral */
  Rule_CREATEOPAQUETYPE_option_MAXLEN_Eq_IntegerLiteral,
  /* 909. <CREATE OPAQUE TYPE_option> ::= CANNOTHASH */
  Rule_CREATEOPAQUETYPE_option_CANNOTHASH,
  /* 910. <CREATE OPAQUE TYPE_option> ::= PASSEDBYVALUE */
  Rule_CREATEOPAQUETYPE_option_PASSEDBYVALUE,
  /* 911. <CREATE OPAQUE TYPE_option> ::= ALIGNMENT '=' IntegerLiteral */
  Rule_CREATEOPAQUETYPE_option_ALIGNMENT_Eq_IntegerLiteral,
  /* 912. <CREATE OPCLASS> ::= CREATE OPCLASS Id FOR Id STRATEGIES '(' <CREATE OPCLASS_strategieS> ')' SUPPORT '(' <Id List> ')' */
  Rule_CREATEOPCLASS_CREATE_OPCLASS_Id_FOR_Id_STRATEGIES_LParan_RParan_SUPPORT_LParan_RParan,
  /* 913. <CREATE OPCLASS_strategieS> ::= <CREATE OPCLASS_strategy> <CREATE OPCLASS_strategieS_> */
  Rule_CREATEOPCLASS_strategieS,
  /* 914. <CREATE OPCLASS_strategieS_> ::= ',' <CREATE OPCLASS_strategy> <CREATE OPCLASS_strategieS_> */
  Rule_CREATEOPCLASS_strategieS__Comma,
  /* 915. <CREATE OPCLASS_strategieS_> ::=  */
  Rule_CREATEOPCLASS_strategieS_,
  /* 916. <CREATE OPCLASS_strategy> ::= Id '(' <Type> ',' <Type> ',' <Type> ')' */
  Rule_CREATEOPCLASS_strategy_Id_LParan_Comma_Comma_RParan,
  /* 917. <CREATE OPCLASS_strategy> ::= Id '(' <Type> ',' <Type> ')' */
  Rule_CREATEOPCLASS_strategy_Id_LParan_Comma_RParan,
  /* 918. <CREATE OPCLASS_strategy> ::= Id */
  Rule_CREATEOPCLASS_strategy_Id,
  /* 919. <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE,
  /* 920. <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <SQLBlock> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE2,
  /* 921. <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' <RoutineParams> ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE3,
  /* 922. <CREATE PROCEDURE> ::= CREATE <DBAopt> PROCEDURE Id '(' ')' <CREATE FUNCTION_referencingOpt> <CREATE PROCEDURE_returnOpt> <CREATE FUNCTION_specificOpt> <CREATE FUNCTION_withOpt> <ExternalRoutine> END PROCEDURE <CREATE FUNCTION_documentOpt> <CREATE FUNCTION_listingOpt> */
  Rule_CREATEPROCEDURE_CREATE_PROCEDURE_Id_LParan_RParan_END_PROCEDURE4,
  /* 923. <CREATE PROCEDURE_returnOpt> ::= <CREATE FUNCTION_return> */
  Rule_CREATEPROCEDURE_returnOpt,
  /* 924. <CREATE PROCEDURE_returnOpt> ::=  */
  Rule_CREATEPROCEDURE_returnOpt2,
  /* 925. <CREATE ROLE> ::= CREATE ROLE Id */
  Rule_CREATEROLE_CREATE_ROLE_Id,
  /* 926. <CREATE ROLE> ::= CREATE ROLE StringLiteral */
  Rule_CREATEROLE_CREATE_ROLE_StringLiteral,
  /* 927. <CREATE ROUTINE FROM> ::= CREATE ROUTINE FROM StringLiteral */
  Rule_CREATEROUTINEFROM_CREATE_ROUTINE_FROM_StringLiteral,
  /* 928. <CREATE ROUTINE FROM> ::= CREATE ROUTINE FROM Id */
  Rule_CREATEROUTINEFROM_CREATE_ROUTINE_FROM_Id,
  /* 929. <CREATE ROW TYPE> ::= CREATE ROW TYPE Id <CREATE ROW TYPE_field> UNDER Id */
  Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id_UNDER_Id,
  /* 930. <CREATE ROW TYPE> ::= CREATE ROW TYPE Id <CREATE ROW TYPE_field> */
  Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id,
  /* 931. <CREATE ROW TYPE> ::= CREATE ROW TYPE Id UNDER Id */
  Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id_UNDER_Id2,
  /* 932. <CREATE ROW TYPE> ::= CREATE ROW TYPE Id */
  Rule_CREATEROWTYPE_CREATE_ROW_TYPE_Id2,
  /* 933. <CREATE ROW TYPE_field> ::= Id <Type> NOT NULL */
  Rule_CREATEROWTYPE_field_Id_NOT_NULL,
  /* 934. <CREATE ROW TYPE_field> ::= Id <Type> */
  Rule_CREATEROWTYPE_field_Id,
  /* 935. <CREATE SCHEMA> ::= CREATE SCHEMA AUTHORIZATION Id <CREATE SCHEMA_statementS> */
  Rule_CREATESCHEMA_CREATE_SCHEMA_AUTHORIZATION_Id,
  /* 936. <CREATE SCHEMA_statementS> ::= <CREATE SCHEMA_statement> <CREATE SCHEMA_statementS> */
  Rule_CREATESCHEMA_statementS,
  /* 937. <CREATE SCHEMA_statementS> ::= <CREATE SCHEMA_statement> */
  Rule_CREATESCHEMA_statementS2,
  /* 938. <CREATE SCHEMA_statement> ::= <CREATE TABLE> */
  Rule_CREATESCHEMA_statement,
  /* 939. <CREATE SCHEMA_statement> ::= <CREATE VIEW> */
  Rule_CREATESCHEMA_statement2,
  /* 940. <CREATE SCHEMA_statement> ::= <GRANT> */
  Rule_CREATESCHEMA_statement3,
  /* 941. <CREATE SCHEMA_statement> ::= <CREATE INDEX> */
  Rule_CREATESCHEMA_statement4,
  /* 942. <CREATE SCHEMA_statement> ::= <CREATE SYNONYM> */
  Rule_CREATESCHEMA_statement5,
  /* 943. <CREATE SCHEMA_statement> ::= <CREATE TRIGGER> */
  Rule_CREATESCHEMA_statement6,
  /* 944. <CREATE SCHEMA_statement> ::= <CREATE SEQUENCE> */
  Rule_CREATESCHEMA_statement7,
  /* 945. <CREATE SCHEMA_statement> ::= <CREATE ROW TYPE> */
  Rule_CREATESCHEMA_statement8,
  /* 946. <CREATE SCHEMA_statement> ::= <CREATE OPAQUE TYPE> */
  Rule_CREATESCHEMA_statement9,
  /* 947. <CREATE SCHEMA_statement> ::= <CREATE DISTINCT TYPE> */
  Rule_CREATESCHEMA_statement10,
  /* 948. <CREATE SCHEMA_statement> ::= <CREATE CAST> */
  Rule_CREATESCHEMA_statement11,
  /* 949. <CREATE SCRATCH TABLE> ::= CREATE SCRATCH TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE SCRATCH TABLE_opt> */
  Rule_CREATESCRATCHTABLE_CREATE_SCRATCH_TABLE_Id_LParan_RParan,
  /* 950. <ColumnDefine_table_constraintS> ::= <ALTER TABLE_Modify_0> <ColumnDefine_table_constraintS_> */
  Rule_ColumnDefine_table_constraintS,
  /* 951. <ColumnDefine_table_constraintS> ::= <table_constraint> <ColumnDefine_table_constraintS_> */
  Rule_ColumnDefine_table_constraintS2,
  /* 952. <ColumnDefine_table_constraintS_> ::= ',' <ALTER TABLE_Modify_0> <ColumnDefine_table_constraintS_> */
  Rule_ColumnDefine_table_constraintS__Comma,
  /* 953. <ColumnDefine_table_constraintS_> ::= ',' <table_constraint> <ColumnDefine_table_constraintS_> */
  Rule_ColumnDefine_table_constraintS__Comma2,
  /* 954. <ColumnDefine_table_constraintS_> ::=  */
  Rule_ColumnDefine_table_constraintS_,
  /* 955. <CREATE SCRATCH TABLE_opt> ::= IN Id <CREATE SCRATCH TABLE_LockMode> */
  Rule_CREATESCRATCHTABLE_opt_IN_Id,
  /* 956. <CREATE SCRATCH TABLE_opt> ::= <FRAGMENT BY_TABLE> <CREATE SCRATCH TABLE_LockMode> */
  Rule_CREATESCRATCHTABLE_opt,
  /* 957. <CREATE SCRATCH TABLE_opt> ::= IN Id */
  Rule_CREATESCRATCHTABLE_opt_IN_Id2,
  /* 958. <CREATE SCRATCH TABLE_opt> ::= <FRAGMENT BY_TABLE> */
  Rule_CREATESCRATCHTABLE_opt2,
  /* 959. <CREATE SCRATCH TABLE_opt> ::=  */
  Rule_CREATESCRATCHTABLE_opt3,
  /* 960. <CREATE SCRATCH TABLE_LockMode> ::= LOCK MODE PAGE */
  Rule_CREATESCRATCHTABLE_LockMode_LOCK_MODE_PAGE,
  /* 961. <CREATE SCRATCH TABLE_LockMode> ::= LOCK MODE ROW */
  Rule_CREATESCRATCHTABLE_LockMode_LOCK_MODE_ROW,
  /* 962. <CREATE SCRATCH TABLE_LockMode> ::= LOCK MODE TABLE */
  Rule_CREATESCRATCHTABLE_LockMode_LOCK_MODE_TABLE,
  /* 963. <CREATE SECURITY LABEL> ::= CREATE SECURITY LABEL Id <CREATE SECURITY LABEL_componentS> */
  Rule_CREATESECURITYLABEL_CREATE_SECURITY_LABEL_Id,
  /* 964. <CREATE SECURITY LABEL_componentS> ::= <CREATE SECURITY LABEL_component> <CREATE SECURITY LABEL_componentS_> */
  Rule_CREATESECURITYLABEL_componentS,
  /* 965. <CREATE SECURITY LABEL_componentS_> ::= ',' <CREATE SECURITY LABEL_component> <CREATE SECURITY LABEL_componentS_> */
  Rule_CREATESECURITYLABEL_componentS__Comma,
  /* 966. <CREATE SECURITY LABEL_componentS_> ::=  */
  Rule_CREATESECURITYLABEL_componentS_,
  /* 967. <CREATE SECURITY LABEL_component> ::= COMPONENT Id <Id List> */
  Rule_CREATESECURITYLABEL_component_COMPONENT_Id,
  /* 968. <CREATE SECURITY LABEL COMPONENT> ::= CREATE SECURITY LABEL COMPONENT Id ARRAY '[' <SECURITY LABEL COMPONENT_ARRAYs> ']' */
  Rule_CREATESECURITYLABELCOMPONENT_CREATE_SECURITY_LABEL_COMPONENT_Id_ARRAY_LBracket_RBracket,
  /* 969. <CREATE SECURITY LABEL COMPONENT> ::= CREATE SECURITY LABEL COMPONENT Id SET '{' <Id List> '}' */
  Rule_CREATESECURITYLABELCOMPONENT_CREATE_SECURITY_LABEL_COMPONENT_Id_SET_LBrace_RBrace,
  /* 970. <CREATE SECURITY LABEL COMPONENT> ::= CREATE SECURITY LABEL COMPONENT Id TREE '(' <SECURITY LABEL COMPONENT_TREEs> ')' */
  Rule_CREATESECURITYLABELCOMPONENT_CREATE_SECURITY_LABEL_COMPONENT_Id_TREE_LParan_RParan,
  /* 971. <CREATE SECURITY POLICY> ::= CREATE SECURITY POLICY Id COMPONENTS <Id List> <CREATE SECURITY POLICY_IDSLBACRULESopt> RESTRICT NOT AUTHORIZED WRITE SECURITY LABEL */
  Rule_CREATESECURITYPOLICY_CREATE_SECURITY_POLICY_Id_COMPONENTS_RESTRICT_NOT_AUTHORIZED_WRITE_SECURITY_LABEL,
  /* 972. <CREATE SECURITY POLICY> ::= CREATE SECURITY POLICY Id COMPONENTS <Id List> <CREATE SECURITY POLICY_IDSLBACRULESopt> OVERRIDE NOT AUTHORIZED WRITE SECURITY LABEL */
  Rule_CREATESECURITYPOLICY_CREATE_SECURITY_POLICY_Id_COMPONENTS_OVERRIDE_NOT_AUTHORIZED_WRITE_SECURITY_LABEL,
  /* 973. <CREATE SECURITY POLICY_IDSLBACRULESopt> ::= WITH IDSLBACRULES */
  Rule_CREATESECURITYPOLICY_IDSLBACRULESopt_WITH_IDSLBACRULES,
  /* 974. <CREATE SECURITY POLICY_IDSLBACRULESopt> ::=  */
  Rule_CREATESECURITYPOLICY_IDSLBACRULESopt,
  /* 975. <CREATE SEQUENCE> ::= CREATE SEQUENCE Id <ALTER SEQUENCEoptionS> */
  Rule_CREATESEQUENCE_CREATE_SEQUENCE_Id,
  /* 976. <CREATE SYNONYM> ::= CREATE PUBLIC SYNONYM Id FOR Id */
  Rule_CREATESYNONYM_CREATE_PUBLIC_SYNONYM_Id_FOR_Id,
  /* 977. <CREATE SYNONYM> ::= CREATE SYNONYM Id FOR Id */
  Rule_CREATESYNONYM_CREATE_SYNONYM_Id_FOR_Id,
  /* 978. <CREATE SYNONYM> ::= CREATE PRIVATE SYNONYM Id FOR Id */
  Rule_CREATESYNONYM_CREATE_PRIVATE_SYNONYM_Id_FOR_Id,
  /* 979. <CREATE TABLE> ::= CREATE <TableType> TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> <SECURITY LABEL_clause> */
  Rule_CREATETABLE_CREATE_TABLE_Id_LParan_RParan,
  /* 980. <CREATE TABLE> ::= CREATE <TableType> TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> */
  Rule_CREATETABLE_CREATE_TABLE_Id_LParan_RParan2,
  /* 981. <CREATE TABLE> ::= CREATE <TableType> TABLE Id OF TYPE Id */
  Rule_CREATETABLE_CREATE_TABLE_Id_OF_TYPE_Id,
  /* 982. <TableType> ::= STANDARD */
  Rule_TableType_STANDARD,
  /* 983. <TableType> ::= RAW */
  Rule_TableType_RAW,
  /* 984. <TableType> ::= STATIC */
  Rule_TableType_STATIC,
  /* 985. <TableType> ::= OPERATIONAL */
  Rule_TableType_OPERATIONAL,
  /* 986. <TableType> ::=  */
  Rule_TableType,
  /* 987. <CREATE TABLE_option> ::= <CREATE TABLE_Option_COLSopt> <CREATE TABLE_StorageOpt> <CREATE SCRATCH TABLE_LockMode> <Access-Method_clauseOpt> */
  Rule_CREATETABLE_option,
  /* 988. <CREATE TABLE_option> ::=  */
  Rule_CREATETABLE_option2,
  /* 989. <CREATE TABLE_Option_COLSopt> ::= WITH CRCOLS */
  Rule_CREATETABLE_Option_COLSopt_WITH_CRCOLS,
  /* 990. <CREATE TABLE_Option_COLSopt> ::= WITH VERCOLS */
  Rule_CREATETABLE_Option_COLSopt_WITH_VERCOLS,
  /* 991. <CREATE TABLE_Option_COLSopt> ::=  */
  Rule_CREATETABLE_Option_COLSopt,
  /* 992. <CREATE TABLE_StorageOpt> ::= <CREATE TABLE_StorageOpt1> <CREATE TABLE_Put> <EXTENT SIZEopt> */
  Rule_CREATETABLE_StorageOpt,
  /* 993. <CREATE TABLE_StorageOpt> ::= <CREATE TABLE_StorageOpt1> <EXTENT SIZEopt> */
  Rule_CREATETABLE_StorageOpt2,
  /* 994. <CREATE TABLE_StorageOpt1> ::= <CREATE INDEX_StorageOpt0> */
  Rule_CREATETABLE_StorageOpt1,
  /* 995. <CREATE TABLE_StorageOpt1> ::= <FRAGMENT BY_TABLE> */
  Rule_CREATETABLE_StorageOpt12,
  /* 996. <FRAGMENT BY_TABLE0> ::= <FRAGMENT BY_INDEXopt1> ROUND ROBIN IN <Id List> */
  Rule_FRAGMENTBY_TABLE0_ROUND_ROBIN_IN,
  /* 997. <FRAGMENT BY_TABLE0> ::= <FRAGMENT BY_INDEXopt1> ROUND ROBIN <FRAGMENT BY_TABLE_1s> */
  Rule_FRAGMENTBY_TABLE0_ROUND_ROBIN,
  /* 998. <FRAGMENT BY_TABLE0> ::= <FRAGMENT BY_INDEX> */
  Rule_FRAGMENTBY_TABLE0,
  /* 999. <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_TABLE0> */
  Rule_FRAGMENTBY_TABLE,
  /* 1000. <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> EXPRESSION USING Id <FRAGMENT BY_Expr List> */
  Rule_FRAGMENTBY_TABLE_EXPRESSION_USING_Id,
  /* 1001. <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> RANG '(' Id <FRAGMENT BY_TABLE_rang1> ')' IN <Id List> REMAINDER IN Id */
  Rule_FRAGMENTBY_TABLE_RANG_LParan_Id_RParan_IN_REMAINDER_IN_Id,
  /* 1002. <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> RANG '(' Id <FRAGMENT BY_TABLE_rang1> ')' IN <Id List> */
  Rule_FRAGMENTBY_TABLE_RANG_LParan_Id_RParan_IN,
  /* 1003. <FRAGMENT BY_TABLE> ::= <FRAGMENT BY_TABLEopt1> <FRAGMENT BY_INDEXopt1> HYBRID <FRAGMENT BY_TABLE_rang2> <FRAGMENT BY_TABLE_rang3> */
  Rule_FRAGMENTBY_TABLE_HYBRID,
  /* 1004. <FRAGMENT BY_TABLE_rang1> ::= MIN IntegerLiteral MAX IntegerLiteral */
  Rule_FRAGMENTBY_TABLE_rang1_MIN_IntegerLiteral_MAX_IntegerLiteral,
  /* 1005. <FRAGMENT BY_TABLE_rang1> ::= MAX IntegerLiteral */
  Rule_FRAGMENTBY_TABLE_rang1_MAX_IntegerLiteral,
  /* 1006. <FRAGMENT BY_TABLE_rang1> ::= MIN IntegerLiteral IntegerLiteral */
  Rule_FRAGMENTBY_TABLE_rang1_MIN_IntegerLiteral_IntegerLiteral,
  /* 1007. <FRAGMENT BY_TABLE_rang1> ::= IntegerLiteral */
  Rule_FRAGMENTBY_TABLE_rang1_IntegerLiteral,
  /* 1008. <FRAGMENT BY_TABLE_rang2> ::= '(' RANGE '(' Id <FRAGMENT BY_TABLE_rang1> ')' ')' */
  Rule_FRAGMENTBY_TABLE_rang2_LParan_RANGE_LParan_Id_RParan_RParan,
  /* 1009. <FRAGMENT BY_TABLE_rang2> ::= '(' RANGE '(' Id ')' ')' */
  Rule_FRAGMENTBY_TABLE_rang2_LParan_RANGE_LParan_Id_RParan_RParan2,
  /* 1010. <FRAGMENT BY_TABLE_rang3> ::= RANGE '(' Id <FRAGMENT BY_TABLE_rang1> ')' <FRAGMENT BY_TABLE_IN> */
  Rule_FRAGMENTBY_TABLE_rang3_RANGE_LParan_Id_RParan,
  /* 1011. <FRAGMENT BY_TABLE_rang3> ::= RANGE '(' Id ')' <FRAGMENT BY_TABLE_IN> */
  Rule_FRAGMENTBY_TABLE_rang3_RANGE_LParan_Id_RParan2,
  /* 1012. <FRAGMENT BY_TABLE_IN> ::= IN <FRAGMENT BY_TABLE_IN_s> REMAINDER IN <FRAGMENT BY_TABLE_IN_> */
  Rule_FRAGMENTBY_TABLE_IN_IN_REMAINDER_IN,
  /* 1013. <FRAGMENT BY_TABLE_IN> ::= IN <FRAGMENT BY_TABLE_IN_s> */
  Rule_FRAGMENTBY_TABLE_IN_IN,
  /* 1014. <FRAGMENT BY_TABLE_IN_s> ::= <FRAGMENT BY_TABLE_IN_> <FRAGMENT BY_TABLE_IN_s_> */
  Rule_FRAGMENTBY_TABLE_IN_s,
  /* 1015. <FRAGMENT BY_TABLE_IN_s_> ::= ',' <FRAGMENT BY_TABLE_IN_> <FRAGMENT BY_TABLE_IN_s_> */
  Rule_FRAGMENTBY_TABLE_IN_s__Comma,
  /* 1016. <FRAGMENT BY_TABLE_IN_s_> ::=  */
  Rule_FRAGMENTBY_TABLE_IN_s_,
  /* 1017. <FRAGMENT BY_TABLE_IN_> ::= Id */
  Rule_FRAGMENTBY_TABLE_IN__Id,
  /* 1018. <FRAGMENT BY_TABLE_IN_> ::= '(' <Id List> ')' */
  Rule_FRAGMENTBY_TABLE_IN__LParan_RParan,
  /* 1019. <FRAGMENT BY_TABLEopt1> ::= WITH ROWIDS */
  Rule_FRAGMENTBY_TABLEopt1_WITH_ROWIDS,
  /* 1020. <FRAGMENT BY_TABLEopt1> ::=  */
  Rule_FRAGMENTBY_TABLEopt1,
  /* 1021. <FRAGMENT BY_TABLE_1s> ::= <FRAGMENT BY_TABLE_1> <FRAGMENT BY_TABLE_1s_> */
  Rule_FRAGMENTBY_TABLE_1s,
  /* 1022. <FRAGMENT BY_TABLE_1s_> ::= ',' <FRAGMENT BY_TABLE_1> <FRAGMENT BY_TABLE_1s_> */
  Rule_FRAGMENTBY_TABLE_1s__Comma,
  /* 1023. <FRAGMENT BY_TABLE_1s_> ::=  */
  Rule_FRAGMENTBY_TABLE_1s_,
  /* 1024. <FRAGMENT BY_TABLE_1> ::= PARTITION Id IN Id */
  Rule_FRAGMENTBY_TABLE_1_PARTITION_Id_IN_Id,
  /* 1025. <CREATE TABLE_Put> ::= PUT <CREATE TABLE_Put_S> */
  Rule_CREATETABLE_Put_PUT,
  /* 1026. <CREATE TABLE_Put_S> ::= <CREATE TABLE_Put_> <CREATE TABLE_Put_S_> */
  Rule_CREATETABLE_Put_S,
  /* 1027. <CREATE TABLE_Put_S_> ::= ',' <CREATE TABLE_Put_> <CREATE TABLE_Put_S_> */
  Rule_CREATETABLE_Put_S__Comma,
  /* 1028. <CREATE TABLE_Put_S_> ::=  */
  Rule_CREATETABLE_Put_S_,
  /* 1029. <CREATE TABLE_Put_> ::= Id IN '(' <Id List> ')' */
  Rule_CREATETABLE_Put__Id_IN_LParan_RParan,
  /* 1030. <CREATE TABLE_Put_> ::= '(' <ALTER TABLE_PutOptions> ')' */
  Rule_CREATETABLE_Put__LParan_RParan,
  /* 1031. <EXTENT SIZEopt> ::= EXTENT SIZE IntegerLiteral NEXT SIZE IntegerLiteral */
  Rule_EXTENTSIZEopt_EXTENT_SIZE_IntegerLiteral_NEXT_SIZE_IntegerLiteral,
  /* 1032. <EXTENT SIZEopt> ::= EXTENT SIZE IntegerLiteral */
  Rule_EXTENTSIZEopt_EXTENT_SIZE_IntegerLiteral,
  /* 1033. <EXTENT SIZEopt> ::= NEXT SIZE IntegerLiteral */
  Rule_EXTENTSIZEopt_NEXT_SIZE_IntegerLiteral,
  /* 1034. <EXTENT SIZEopt> ::=  */
  Rule_EXTENTSIZEopt,
  /* 1035. <CREATE TEMP TABLE> ::= CREATE TEMP TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> WITH NO LOG */
  Rule_CREATETEMPTABLE_CREATE_TEMP_TABLE_Id_LParan_RParan_WITH_NO_LOG,
  /* 1036. <CREATE TEMP TABLE> ::= CREATE TEMP TABLE Id '(' <ColumnDefine_table_constraintS> ')' <CREATE TABLE_option> */
  Rule_CREATETEMPTABLE_CREATE_TEMP_TABLE_Id_LParan_RParan,
  /* 1037. <CREATE TRIGGER> ::= CREATE TRIGGER Id <CREATE TRIGGER1> ENABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_ENABLED,
  /* 1038. <CREATE TRIGGER> ::= CREATE TRIGGER Id <CREATE TRIGGER1> DISABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_DISABLED,
  /* 1039. <CREATE TRIGGER> ::= CREATE TRIGGER Id <CREATE TRIGGER1> */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id,
  /* 1040. <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF <CREATE TRIGGER2> ENABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_ENABLED,
  /* 1041. <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF <CREATE TRIGGER2> DISABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_DISABLED,
  /* 1042. <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF <CREATE TRIGGER2> */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF,
  /* 1043. <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF ENABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_ENABLED2,
  /* 1044. <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF DISABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF_DISABLED2,
  /* 1045. <CREATE TRIGGER> ::= CREATE TRIGGER Id INSTEAD OF */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_INSTEAD_OF2,
  /* 1046. <CREATE TRIGGER> ::= CREATE TRIGGER Id ENABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_ENABLED2,
  /* 1047. <CREATE TRIGGER> ::= CREATE TRIGGER Id DISABLED */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id_DISABLED2,
  /* 1048. <CREATE TRIGGER> ::= CREATE TRIGGER Id */
  Rule_CREATETRIGGER_CREATE_TRIGGER_Id2,
  /* 1049. <CREATE TRIGGER1> ::= DELETE ON Id <CREATE TRIGGER1_actionS> */
  Rule_CREATETRIGGER1_DELETE_ON_Id,
  /* 1050. <CREATE TRIGGER1> ::= DELETE ON Id <CREATE TRIGGER1_1> */
  Rule_CREATETRIGGER1_DELETE_ON_Id2,
  /* 1051. <CREATE TRIGGER1> ::= SELECT ON Id <CREATE TRIGGER1_actionS> */
  Rule_CREATETRIGGER1_SELECT_ON_Id,
  /* 1052. <CREATE TRIGGER1> ::= SELECT ON Id <CREATE TRIGGER1_1> */
  Rule_CREATETRIGGER1_SELECT_ON_Id2,
  /* 1053. <CREATE TRIGGER1> ::= SELECT OF <Id List> ON Id <CREATE TRIGGER1_actionS> */
  Rule_CREATETRIGGER1_SELECT_OF_ON_Id,
  /* 1054. <CREATE TRIGGER1> ::= SELECT OF <Id List> ON Id <CREATE TRIGGER1_1> */
  Rule_CREATETRIGGER1_SELECT_OF_ON_Id2,
  /* 1055. <CREATE TRIGGER1> ::= UPDATE ON Id <CREATE TRIGGER1_actionS> */
  Rule_CREATETRIGGER1_UPDATE_ON_Id,
  /* 1056. <CREATE TRIGGER1> ::= UPDATE ON Id <CREATE TRIGGER1_2> */
  Rule_CREATETRIGGER1_UPDATE_ON_Id2,
  /* 1057. <CREATE TRIGGER1> ::= UPDATE OF <Id List> ON Id <CREATE TRIGGER1_actionS> */
  Rule_CREATETRIGGER1_UPDATE_OF_ON_Id,
  /* 1058. <CREATE TRIGGER1> ::= UPDATE OF <Id List> ON Id <CREATE TRIGGER1_2> */
  Rule_CREATETRIGGER1_UPDATE_OF_ON_Id2,
  /* 1059. <CREATE TRIGGER1> ::= INSERT ON Id <CREATE TRIGGER1_new> <CREATE TRIGGER1_event> */
  Rule_CREATETRIGGER1_INSERT_ON_Id,
  /* 1060. <CREATE TRIGGER1> ::= INSERT ON Id <CREATE TRIGGER1_actionS> */
  Rule_CREATETRIGGER1_INSERT_ON_Id2,
  /* 1061. <CREATE TRIGGER1_1> ::= <CREATE TRIGGER1_old> <CREATE TRIGGER1_event> */
  Rule_CREATETRIGGER1_1,
  /* 1062. <CREATE TRIGGER1_2> ::= <CREATE TRIGGER1_old> <CREATE TRIGGER1_event> */
  Rule_CREATETRIGGER1_2,
  /* 1063. <CREATE TRIGGER1_2> ::= <CREATE TRIGGER1_old> <CREATE TRIGGER1_new> <CREATE TRIGGER1_event> */
  Rule_CREATETRIGGER1_22,
  /* 1064. <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> FOR EACH ROW <CREATE TRIGGER1_action> AFTER <CREATE TRIGGER1_action> */
  Rule_CREATETRIGGER1_actionS_BEFORE_FOR_EACH_ROW_AFTER,
  /* 1065. <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> FOR EACH ROW <CREATE TRIGGER1_action> */
  Rule_CREATETRIGGER1_actionS_BEFORE_FOR_EACH_ROW,
  /* 1066. <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> AFTER <CREATE TRIGGER1_action> */
  Rule_CREATETRIGGER1_actionS_BEFORE_AFTER,
  /* 1067. <CREATE TRIGGER1_actionS> ::= FOR EACH ROW <CREATE TRIGGER1_action> AFTER <CREATE TRIGGER1_action> */
  Rule_CREATETRIGGER1_actionS_FOR_EACH_ROW_AFTER,
  /* 1068. <CREATE TRIGGER1_actionS> ::= BEFORE <CREATE TRIGGER1_action> */
  Rule_CREATETRIGGER1_actionS_BEFORE,
  /* 1069. <CREATE TRIGGER1_actionS> ::= FOR EACH ROW <CREATE TRIGGER1_action> */
  Rule_CREATETRIGGER1_actionS_FOR_EACH_ROW,
  /* 1070. <CREATE TRIGGER1_actionS> ::= AFTER <CREATE TRIGGER1_action> */
  Rule_CREATETRIGGER1_actionS_AFTER,
  /* 1071. <CREATE TRIGGER1_action> ::= <CREATE TRIGGER1_action_S> */
  Rule_CREATETRIGGER1_action,
  /* 1072. <CREATE TRIGGER1_action_S> ::= <CREATE TRIGGER1_action_> <CREATE TRIGGER1_action_S_> */
  Rule_CREATETRIGGER1_action_S,
  /* 1073. <CREATE TRIGGER1_action_S_> ::= ',' <CREATE TRIGGER1_action_> <CREATE TRIGGER1_action_S_> */
  Rule_CREATETRIGGER1_action_S__Comma,
  /* 1074. <CREATE TRIGGER1_action_S_> ::=  */
  Rule_CREATETRIGGER1_action_S_,
  /* 1075. <CREATE TRIGGER1_action_> ::= WHEN '(' <Expression> ')' '(' <CREATE TRIGGER1_action_S2> ')' */
  Rule_CREATETRIGGER1_action__WHEN_LParan_RParan_LParan_RParan,
  /* 1076. <CREATE TRIGGER1_action_> ::= '(' <CREATE TRIGGER1_action_S2> ')' */
  Rule_CREATETRIGGER1_action__LParan_RParan,
  /* 1077. <CREATE TRIGGER1_action_S2> ::= <CREATE TRIGGER1_action_2> <CREATE TRIGGER1_action_S2_> */
  Rule_CREATETRIGGER1_action_S2,
  /* 1078. <CREATE TRIGGER1_action_S2_> ::= ',' <CREATE TRIGGER1_action_2> <CREATE TRIGGER1_action_S2_> */
  Rule_CREATETRIGGER1_action_S2__Comma,
  /* 1079. <CREATE TRIGGER1_action_S2_> ::=  */
  Rule_CREATETRIGGER1_action_S2_,
  /* 1080. <CREATE TRIGGER1_action_2> ::= <INSERT> */
  Rule_CREATETRIGGER1_action_2,
  /* 1081. <CREATE TRIGGER1_action_2> ::= <DELETE> */
  Rule_CREATETRIGGER1_action_22,
  /* 1082. <CREATE TRIGGER1_action_2> ::= <UPDATE> */
  Rule_CREATETRIGGER1_action_23,
  /* 1083. <CREATE TRIGGER1_action_2> ::= <EXECUTE PROCEDURE> */
  Rule_CREATETRIGGER1_action_24,
  /* 1084. <CREATE TRIGGER1_action_2> ::= <EXECUTE FUNCTION> */
  Rule_CREATETRIGGER1_action_25,
  /* 1085. <CREATE TRIGGER1_old> ::= <referencing_clause_DELETE> */
  Rule_CREATETRIGGER1_old,
  /* 1086. <CREATE TRIGGER1_new> ::= <referencing_clause_INSERT> */
  Rule_CREATETRIGGER1_new,
  /* 1087. <CREATE TRIGGER1_event> ::= <CREATE TRIGGER1_actionS> */
  Rule_CREATETRIGGER1_event,
  /* 1088. <CREATE TRIGGER2> ::= INSERT ON Id <referencing_clause_INSERT> FOR EACH ROW '(' <CREATE TRIGGER1_action_S2> ')' */
  Rule_CREATETRIGGER2_INSERT_ON_Id_FOR_EACH_ROW_LParan_RParan,
  /* 1089. <CREATE TRIGGER2> ::= DELETE ON Id <referencing_clause_DELETE> FOR EACH ROW '(' <CREATE TRIGGER1_action_S2> ')' */
  Rule_CREATETRIGGER2_DELETE_ON_Id_FOR_EACH_ROW_LParan_RParan,
  /* 1090. <CREATE TRIGGER2> ::= UPDATE ON Id <referencing_clause_UPDATE> FOR EACH ROW '(' <CREATE TRIGGER1_action_S2> ')' */
  Rule_CREATETRIGGER2_UPDATE_ON_Id_FOR_EACH_ROW_LParan_RParan,
  /* 1091. <CREATE VIEW> ::= CREATE VIEW Id AS <SELECT> <WITH CHECKopt> */
  Rule_CREATEVIEW_CREATE_VIEW_Id_AS,
  /* 1092. <CREATE VIEW> ::= CREATE VIEW Id '(' <Id List> ')' AS <SELECT> <WITH CHECKopt> */
  Rule_CREATEVIEW_CREATE_VIEW_Id_LParan_RParan_AS,
  /* 1093. <CREATE VIEW> ::= CREATE VIEW Id OF TYPE Id AS <SELECT> <WITH CHECKopt> */
  Rule_CREATEVIEW_CREATE_VIEW_Id_OF_TYPE_Id_AS,
  /* 1094. <WITH CHECKopt> ::= WITH CHECK OPTION */
  Rule_WITHCHECKopt_WITH_CHECK_OPTION,
  /* 1095. <WITH CHECKopt> ::=  */
  Rule_WITHCHECKopt,
  /* 1096. <CREATE XADATASOURCE> ::= CREATE XADATASOURCE Id USING Id */
  Rule_CREATEXADATASOURCE_CREATE_XADATASOURCE_Id_USING_Id,
  /* 1097. <CREATE XADATASOURCE TYPE> ::= CREATE XADATASOURCE TYPE Id '(' <PurposeOption2S> ')' */
  Rule_CREATEXADATASOURCETYPE_CREATE_XADATASOURCE_TYPE_Id_LParan_RParan,
  /* 1098. <PurposeOption2S> ::= <PurposeOption2> <PurposeOption2S_> */
  Rule_PurposeOption2S,
  /* 1099. <PurposeOption2S_> ::= ',' <PurposeOption2> <PurposeOption2S_> */
  Rule_PurposeOption2S__Comma,
  /* 1100. <PurposeOption2S_> ::=  */
  Rule_PurposeOption2S_,
  /* 1101. <PurposeOption2> ::= <PurposeKeyword2> '=' Id */
  Rule_PurposeOption2_Eq_Id,
  /* 1102. <PurposeOption2> ::= <PurposeKeyword2> '=' IntegerLiteral */
  Rule_PurposeOption2_Eq_IntegerLiteral,
  /* 1103. <PurposeKeyword2> ::= 'xa_flags' */
  Rule_PurposeKeyword2_xa_flags,
  /* 1104. <PurposeKeyword2> ::= 'xa_version' */
  Rule_PurposeKeyword2_xa_version,
  /* 1105. <PurposeKeyword2> ::= 'xa_open' */
  Rule_PurposeKeyword2_xa_open,
  /* 1106. <PurposeKeyword2> ::= 'xa_close' */
  Rule_PurposeKeyword2_xa_close,
  /* 1107. <PurposeKeyword2> ::= 'xa_start' */
  Rule_PurposeKeyword2_xa_start,
  /* 1108. <PurposeKeyword2> ::= 'xa_end' */
  Rule_PurposeKeyword2_xa_end,
  /* 1109. <PurposeKeyword2> ::= 'xa_rollback' */
  Rule_PurposeKeyword2_xa_rollback,
  /* 1110. <PurposeKeyword2> ::= 'xa_prepare' */
  Rule_PurposeKeyword2_xa_prepare,
  /* 1111. <PurposeKeyword2> ::= 'xa_commit' */
  Rule_PurposeKeyword2_xa_commit,
  /* 1112. <PurposeKeyword2> ::= 'xa_recover' */
  Rule_PurposeKeyword2_xa_recover,
  /* 1113. <PurposeKeyword2> ::= 'xa_forget' */
  Rule_PurposeKeyword2_xa_forget,
  /* 1114. <PurposeKeyword2> ::= 'xa_complete' */
  Rule_PurposeKeyword2_xa_complete,
  /* 1115. <DROP ACCESS_METHOD> ::= DROP 'ACCESS_METHOD' Id RESTRICT */
  Rule_DROPACCESS_METHOD_DROP_ACCESS_METHOD_Id_RESTRICT,
  /* 1116. <DROP AGGREGATE> ::= DROP AGGREGATE Id */
  Rule_DROPAGGREGATE_DROP_AGGREGATE_Id,
  /* 1117. <DROP CAST> ::= DROP CAST '(' Id AS Id ')' */
  Rule_DROPCAST_DROP_CAST_LParan_Id_AS_Id_RParan,
  /* 1118. <DROP DATABASE> ::= DROP DATABASE Id */
  Rule_DROPDATABASE_DROP_DATABASE_Id,
  /* 1119. <DROP DUPLICATE> ::= DROP DUPLICATE OF TABLE Id */
  Rule_DROPDUPLICATE_DROP_DUPLICATE_OF_TABLE_Id,
  /* 1120. <DROP FUNCTION> ::= DROP FUNCTION Id '(' <TypeList> ')' */
  Rule_DROPFUNCTION_DROP_FUNCTION_Id_LParan_RParan,
  /* 1121. <DROP FUNCTION> ::= DROP FUNCTION Id */
  Rule_DROPFUNCTION_DROP_FUNCTION_Id,
  /* 1122. <DROP FUNCTION> ::= DROP SPECIFIC FUNCTION Id */
  Rule_DROPFUNCTION_DROP_SPECIFIC_FUNCTION_Id,
  /* 1123. <DROP INDEX> ::= DROP INDEX Id ONLINE */
  Rule_DROPINDEX_DROP_INDEX_Id_ONLINE,
  /* 1124. <DROP INDEX> ::= DROP INDEX Id */
  Rule_DROPINDEX_DROP_INDEX_Id,
  /* 1125. <DROP OPCLASS> ::= DROP OPCLASS Id RESTRICT */
  Rule_DROPOPCLASS_DROP_OPCLASS_Id_RESTRICT,
  /* 1126. <DROP PROCEDURE> ::= DROP PROCEDURE Id '(' <TypeList> ')' */
  Rule_DROPPROCEDURE_DROP_PROCEDURE_Id_LParan_RParan,
  /* 1127. <DROP PROCEDURE> ::= DROP PROCEDURE Id */
  Rule_DROPPROCEDURE_DROP_PROCEDURE_Id,
  /* 1128. <DROP PROCEDURE> ::= DROP SPECIFIC PROCEDURE Id */
  Rule_DROPPROCEDURE_DROP_SPECIFIC_PROCEDURE_Id,
  /* 1129. <DROP ROLE> ::= DROP ROLE StringLiteral */
  Rule_DROPROLE_DROP_ROLE_StringLiteral,
  /* 1130. <DROP ROLE> ::= DROP ROLE Id */
  Rule_DROPROLE_DROP_ROLE_Id,
  /* 1131. <DROP ROUTINE> ::= DROP ROUTINE Id '(' <TypeList> ')' */
  Rule_DROPROUTINE_DROP_ROUTINE_Id_LParan_RParan,
  /* 1132. <DROP ROUTINE> ::= DROP ROUTINE Id */
  Rule_DROPROUTINE_DROP_ROUTINE_Id,
  /* 1133. <DROP ROUTINE> ::= DROP SPECIFIC ROUTINE Id */
  Rule_DROPROUTINE_DROP_SPECIFIC_ROUTINE_Id,
  /* 1134. <DROP ROW TYPE> ::= DROP ROW TYPE Id RESTRICT */
  Rule_DROPROWTYPE_DROP_ROW_TYPE_Id_RESTRICT,
  /* 1135. <DROP SEQUENCE> ::= DROP SEQUENCE Id */
  Rule_DROPSEQUENCE_DROP_SEQUENCE_Id,
  /* 1136. <DROP SECURITY> ::= DROP SECURITY LABEL Id RESTRICT */
  Rule_DROPSECURITY_DROP_SECURITY_LABEL_Id_RESTRICT,
  /* 1137. <DROP SECURITY> ::= DROP SECURITY LABEL Id */
  Rule_DROPSECURITY_DROP_SECURITY_LABEL_Id,
  /* 1138. <DROP SECURITY> ::= DROP SECURITY LABEL COMPONENT Id RESTRICT */
  Rule_DROPSECURITY_DROP_SECURITY_LABEL_COMPONENT_Id_RESTRICT,
  /* 1139. <DROP SECURITY> ::= DROP SECURITY LABEL COMPONENT Id */
  Rule_DROPSECURITY_DROP_SECURITY_LABEL_COMPONENT_Id,
  /* 1140. <DROP SECURITY> ::= DROP POLICY Id RESTRICT */
  Rule_DROPSECURITY_DROP_POLICY_Id_RESTRICT,
  /* 1141. <DROP SECURITY> ::= DROP POLICY Id */
  Rule_DROPSECURITY_DROP_POLICY_Id,
  /* 1142. <DROP SYNONYM> ::= DROP SYNONYM Id */
  Rule_DROPSYNONYM_DROP_SYNONYM_Id,
  /* 1143. <DROP TABLE> ::= DROP TABLE Id CASCADE */
  Rule_DROPTABLE_DROP_TABLE_Id_CASCADE,
  /* 1144. <DROP TABLE> ::= DROP TABLE Id RESTRICT */
  Rule_DROPTABLE_DROP_TABLE_Id_RESTRICT,
  /* 1145. <DROP TABLE> ::= DROP TABLE Id */
  Rule_DROPTABLE_DROP_TABLE_Id,
  /* 1146. <DROP TRIGGER> ::= DROP TRIGGER Id */
  Rule_DROPTRIGGER_DROP_TRIGGER_Id,
  /* 1147. <DROP TYPE> ::= DROP TYPE Id RESTRICT */
  Rule_DROPTYPE_DROP_TYPE_Id_RESTRICT,
  /* 1148. <DROP VIEW> ::= DROP VIEW Id CASCADE */
  Rule_DROPVIEW_DROP_VIEW_Id_CASCADE,
  /* 1149. <DROP VIEW> ::= DROP VIEW Id RESTRICT */
  Rule_DROPVIEW_DROP_VIEW_Id_RESTRICT,
  /* 1150. <DROP VIEW> ::= DROP VIEW Id */
  Rule_DROPVIEW_DROP_VIEW_Id,
  /* 1151. <DROP XADATASOURCE> ::= DROP XADATASOURCE Id RESTRICT */
  Rule_DROPXADATASOURCE_DROP_XADATASOURCE_Id_RESTRICT,
  /* 1152. <DROP XADATASOURCE TYPE> ::= DROP XADATASOURCE TYPE Id RESTRICT */
  Rule_DROPXADATASOURCETYPE_DROP_XADATASOURCE_TYPE_Id_RESTRICT,
  /* 1153. <MOVE TABLE> ::= MOVE TABLE Id TO DATABASE Id <MOVE TABLEopt1> <MOVE TABLEopt2> */
  Rule_MOVETABLE_MOVE_TABLE_Id_TO_DATABASE_Id,
  /* 1154. <MOVE TABLEopt1> ::= RENAME Id */
  Rule_MOVETABLEopt1_RENAME_Id,
  /* 1155. <MOVE TABLEopt1> ::=  */
  Rule_MOVETABLEopt1,
  /* 1156. <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ',' ROLES ')' RESTRICT */
  Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_Comma_ROLES_RParan_RESTRICT,
  /* 1157. <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ',' ROLES ')' CASCADE */
  Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_Comma_ROLES_RParan_CASCADE,
  /* 1158. <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ')' RESTRICT */
  Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_RParan_RESTRICT,
  /* 1159. <MOVE TABLEopt2> ::= WITH '(' PRIVILEGES ')' CASCADE */
  Rule_MOVETABLEopt2_WITH_LParan_PRIVILEGES_RParan_CASCADE,
  /* 1160. <MOVE TABLEopt2> ::= RESTRICT */
  Rule_MOVETABLEopt2_RESTRICT,
  /* 1161. <MOVE TABLEopt2> ::= CASCADE */
  Rule_MOVETABLEopt2_CASCADE,
  /* 1162. <RENAME COLUMN> ::= RENAME COLUMN Id TO Id */
  Rule_RENAMECOLUMN_RENAME_COLUMN_Id_TO_Id,
  /* 1163. <RENAME DATABASE> ::= RENAME DATABASE Id TO Id */
  Rule_RENAMEDATABASE_RENAME_DATABASE_Id_TO_Id,
  /* 1164. <RENAME INDEX> ::= RENAME INDEX Id TO Id */
  Rule_RENAMEINDEX_RENAME_INDEX_Id_TO_Id,
  /* 1165. <RENAME SECURITY> ::= RENAME SECURITY POLICY Id TO Id */
  Rule_RENAMESECURITY_RENAME_SECURITY_POLICY_Id_TO_Id,
  /* 1166. <RENAME SECURITY> ::= RENAME SECURITY LABEL Id TO Id */
  Rule_RENAMESECURITY_RENAME_SECURITY_LABEL_Id_TO_Id,
  /* 1167. <RENAME SECURITY> ::= RENAME SECURITY LABEL COMPONENT Id TO Id */
  Rule_RENAMESECURITY_RENAME_SECURITY_LABEL_COMPONENT_Id_TO_Id,
  /* 1168. <RENAME SEQUENCE> ::= RENAME SEQUENCE Id TO Id */
  Rule_RENAMESEQUENCE_RENAME_SEQUENCE_Id_TO_Id,
  /* 1169. <RENAME TABLE> ::= RENAME TABLE Id TO Id */
  Rule_RENAMETABLE_RENAME_TABLE_Id_TO_Id,
  /* 1170. <DELETE> ::= DELETE <hint_clauseOpt> <DELETEopt1> <DELETEopt2> */
  Rule_DELETE_DELETE,
  /* 1171. <DELETEopt1> ::= FROM Id */
  Rule_DELETEopt1_FROM_Id,
  /* 1172. <DELETEopt1> ::= FROM ONLY '(' Id ')' */
  Rule_DELETEopt1_FROM_ONLY_LParan_Id_RParan,
  /* 1173. <DELETEopt1> ::= Id */
  Rule_DELETEopt1_Id,
  /* 1174. <DELETEopt1> ::= ONLY '(' Id ')' */
  Rule_DELETEopt1_ONLY_LParan_Id_RParan,
  /* 1175. <DELETEopt1> ::= FROM <CollectionDerivedTable> */
  Rule_DELETEopt1_FROM,
  /* 1176. <DELETEopt1> ::= FROM ONLY '(' <CollectionDerivedTable> ')' */
  Rule_DELETEopt1_FROM_ONLY_LParan_RParan,
  /* 1177. <DELETEopt1> ::= <CollectionDerivedTable> */
  Rule_DELETEopt1,
  /* 1178. <DELETEopt1> ::= ONLY '(' <CollectionDerivedTable> ')' */
  Rule_DELETEopt1_ONLY_LParan_RParan,
  /* 1179. <DELETEopt2> ::= USING <Id List> WHERE <Expression> */
  Rule_DELETEopt2_USING_WHERE,
  /* 1180. <DELETEopt2> ::= FROM <Id List> WHERE <Expression> */
  Rule_DELETEopt2_FROM_WHERE,
  /* 1181. <DELETEopt2> ::= WHERE <Expression> */
  Rule_DELETEopt2_WHERE,
  /* 1182. <DELETEopt2> ::= WHERE CURRENT OF Id */
  Rule_DELETEopt2_WHERE_CURRENT_OF_Id,
  /* 1183. <DELETEopt2> ::=  */
  Rule_DELETEopt2,
  /* 1184. <hint_clauseOpt> ::= '--+' <hintS> */
  Rule_hint_clauseOpt_MinusMinusPlus,
  /* 1185. <hint_clauseOpt> ::= '{+' <hintS> '}' */
  Rule_hint_clauseOpt_LBracePlus_RBrace,
  /* 1186. <hint_clauseOpt> ::= '/ *+' <hintS> '+* /' */
  Rule_hint_clauseOpt_DivTimesPlus_PlusTimesDiv,
  /* 1187. <hint_clauseOpt> ::=  */
  Rule_hint_clauseOpt,
  /* 1188. <hintS> ::= <hint> <hintS_> */
  Rule_hintS,
  /* 1189. <hintS_> ::= ',' <hint> <hintS_> */
  Rule_hintS__Comma,
  /* 1190. <hintS_> ::=  */
  Rule_hintS_,
  /* 1191. <hint> ::= <hint_ReadWrite> */
  Rule_hint,
  /* 1192. <hint> ::= <hint_JoinOrder> */
  Rule_hint2,
  /* 1193. <hint> ::= <hint_JoinMethod> */
  Rule_hint3,
  /* 1194. <hint> ::= <hint_Object> */
  Rule_hint4,
  /* 1195. <hint> ::= <hint_Explain> */
  Rule_hint5,
  /* 1196. <hint> ::= <hint_Rewrite> */
  Rule_hint6,
  /* 1197. <hint_ReadWrite> ::= <hint_ReadWriteOpt1> '(' Id <Id List> ')' StringLiteral */
  Rule_hint_ReadWrite_LParan_Id_RParan_StringLiteral,
  /* 1198. <hint_ReadWrite> ::= <hint_ReadWriteOpt1> '(' Id <Id List> ')' */
  Rule_hint_ReadWrite_LParan_Id_RParan,
  /* 1199. <hint_ReadWrite> ::= <hint_ReadWriteOpt2> '(' Id ')' StringLiteral */
  Rule_hint_ReadWrite_LParan_Id_RParan_StringLiteral2,
  /* 1200. <hint_ReadWrite> ::= <hint_ReadWriteOpt2> '(' Id ')' */
  Rule_hint_ReadWrite_LParan_Id_RParan2,
  /* 1201. <hint_ReadWriteOpt1> ::= 'INDEX_ALL' */
  Rule_hint_ReadWriteOpt1_INDEX_ALL,
  /* 1202. <hint_ReadWriteOpt1> ::= INDEX */
  Rule_hint_ReadWriteOpt1_INDEX,
  /* 1203. <hint_ReadWriteOpt1> ::= 'AVOID_INDEX' */
  Rule_hint_ReadWriteOpt1_AVOID_INDEX,
  /* 1204. <hint_ReadWriteOpt1> ::= 'AVOID_INDEX_SJ' */
  Rule_hint_ReadWriteOpt1_AVOID_INDEX_SJ,
  /* 1205. <hint_ReadWriteOpt1> ::= 'INDEX_SJ' */
  Rule_hint_ReadWriteOpt1_INDEX_SJ,
  /* 1206. <hint_ReadWriteOpt2> ::= FULL */
  Rule_hint_ReadWriteOpt2_FULL,
  /* 1207. <hint_ReadWriteOpt2> ::= 'AVOID_FULL' */
  Rule_hint_ReadWriteOpt2_AVOID_FULL,
  /* 1208. <hint_JoinOrder> ::= ORDERED StringLiteral */
  Rule_hint_JoinOrder_ORDERED_StringLiteral,
  /* 1209. <hint_JoinOrder> ::= ORDERED */
  Rule_hint_JoinOrder_ORDERED,
  /* 1210. <hint_JoinMethod> ::= 'USE_NL' '(' <Id List> ')' StringLiteral */
  Rule_hint_JoinMethod_USE_NL_LParan_RParan_StringLiteral,
  /* 1211. <hint_JoinMethod> ::= 'USE_NL' '(' <Id List> ')' */
  Rule_hint_JoinMethod_USE_NL_LParan_RParan,
  /* 1212. <hint_JoinMethod> ::= 'AVOID_NL' '(' <Id List> ')' StringLiteral */
  Rule_hint_JoinMethod_AVOID_NL_LParan_RParan_StringLiteral,
  /* 1213. <hint_JoinMethod> ::= 'AVOID_NL' '(' <Id List> ')' */
  Rule_hint_JoinMethod_AVOID_NL_LParan_RParan,
  /* 1214. <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List> ')' StringLiteral */
  Rule_hint_JoinMethod_USE_HASH_LParan_RParan_StringLiteral,
  /* 1215. <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List> ')' */
  Rule_hint_JoinMethod_USE_HASH_LParan_RParan,
  /* 1216. <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List> ')' StringLiteral */
  Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan_StringLiteral,
  /* 1217. <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List> ')' */
  Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan,
  /* 1218. <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List_hint_JoinMethod> ')' StringLiteral */
  Rule_hint_JoinMethod_USE_HASH_LParan_RParan_StringLiteral2,
  /* 1219. <hint_JoinMethod> ::= 'USE_HASH' '(' <Id List_hint_JoinMethod> ')' */
  Rule_hint_JoinMethod_USE_HASH_LParan_RParan2,
  /* 1220. <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List_hint_JoinMethod> ')' StringLiteral */
  Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan_StringLiteral2,
  /* 1221. <hint_JoinMethod> ::= 'AVOID_HASH' '(' <Id List_hint_JoinMethod> ')' */
  Rule_hint_JoinMethod_AVOID_HASH_LParan_RParan2,
  /* 1222. <Id List_hint_JoinMethod> ::= <Id_hint_JoinMethod> <Id List_hint_JoinMethod_> */
  Rule_IdList_hint_JoinMethod,
  /* 1223. <Id List_hint_JoinMethod_> ::= ',' <Id_hint_JoinMethod> <Id List_hint_JoinMethod_> */
  Rule_IdList_hint_JoinMethod__Comma,
  /* 1224. <Id List_hint_JoinMethod_> ::=  */
  Rule_IdList_hint_JoinMethod_,
  /* 1225. <Id_hint_JoinMethod> ::= Id '/BUILD' '/BROADCAST' */
  Rule_Id_hint_JoinMethod_Id_DivBUILD_DivBROADCAST,
  /* 1226. <Id_hint_JoinMethod> ::= Id '/BUILD' */
  Rule_Id_hint_JoinMethod_Id_DivBUILD,
  /* 1227. <Id_hint_JoinMethod> ::= Id '/PROBE' '/BROADCAST' */
  Rule_Id_hint_JoinMethod_Id_DivPROBE_DivBROADCAST,
  /* 1228. <Id_hint_JoinMethod> ::= Id '/PROBE' */
  Rule_Id_hint_JoinMethod_Id_DivPROBE,
  /* 1229. <hint_Object> ::= 'ALL_ROWS' StringLiteral */
  Rule_hint_Object_ALL_ROWS_StringLiteral,
  /* 1230. <hint_Object> ::= 'ALL_ROWS' */
  Rule_hint_Object_ALL_ROWS,
  /* 1231. <hint_Object> ::= 'FIRST_ROWS' StringLiteral */
  Rule_hint_Object_FIRST_ROWS_StringLiteral,
  /* 1232. <hint_Object> ::= 'FIRST_ROWS' */
  Rule_hint_Object_FIRST_ROWS,
  /* 1233. <hint_Explain> ::= EXPLAIN ',' 'AVOID_EXECUTE' StringLiteral */
  Rule_hint_Explain_EXPLAIN_Comma_AVOID_EXECUTE_StringLiteral,
  /* 1234. <hint_Explain> ::= EXPLAIN ',' 'AVOID_EXECUTE' */
  Rule_hint_Explain_EXPLAIN_Comma_AVOID_EXECUTE,
  /* 1235. <hint_Explain> ::= EXPLAIN 'AVOID_EXECUTE' StringLiteral */
  Rule_hint_Explain_EXPLAIN_AVOID_EXECUTE_StringLiteral,
  /* 1236. <hint_Explain> ::= EXPLAIN 'AVOID_EXECUTE' */
  Rule_hint_Explain_EXPLAIN_AVOID_EXECUTE,
  /* 1237. <hint_Explain> ::= EXPLAIN StringLiteral */
  Rule_hint_Explain_EXPLAIN_StringLiteral,
  /* 1238. <hint_Explain> ::= EXPLAIN */
  Rule_hint_Explain_EXPLAIN,
  /* 1239. <hint_Rewrite> ::= NESTED StringLiteral */
  Rule_hint_Rewrite_NESTED_StringLiteral,
  /* 1240. <hint_Rewrite> ::= NESTED */
  Rule_hint_Rewrite_NESTED,
  /* 1241. <INSERT_byVALUES> ::= INSERT INTO Id '(' <Id List> ')' VALUES '(' <Expr List> ')' */
  Rule_INSERT_byVALUES_INSERT_INTO_Id_LParan_RParan_VALUES_LParan_RParan,
  /* 1242. <INSERT_byVALUES> ::= INSERT INTO Id VALUES '(' <Expr List> ')' */
  Rule_INSERT_byVALUES_INSERT_INTO_Id_VALUES_LParan_RParan,
  /* 1243. <INSERT2> ::= INSERT AT IntegerLiteral INTO Id <Id List> '(' <Expr List> ')' */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id_LParan_RParan,
  /* 1244. <INSERT2> ::= INSERT AT IntegerLiteral INTO Id <Id List> <SELECT> */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id,
  /* 1245. <INSERT2> ::= INSERT AT IntegerLiteral INTO Id '(' <Expr List> ')' */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id_LParan_RParan2,
  /* 1246. <INSERT2> ::= INSERT AT IntegerLiteral INTO Id <SELECT> */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_Id2,
  /* 1247. <INSERT2> ::= INSERT INTO Id <Id List> '(' <Expr List> ')' */
  Rule_INSERT2_INSERT_INTO_Id_LParan_RParan,
  /* 1248. <INSERT2> ::= INSERT INTO Id <Id List> <SELECT> */
  Rule_INSERT2_INSERT_INTO_Id,
  /* 1249. <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> <Id List> '(' <Expr List> ')' */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_LParan_RParan,
  /* 1250. <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> <Id List> <SELECT> */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO,
  /* 1251. <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> '(' <Expr List> ')' */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO_LParan_RParan2,
  /* 1252. <INSERT2> ::= INSERT AT IntegerLiteral INTO <CollectionDerivedTable> <SELECT> */
  Rule_INSERT2_INSERT_AT_IntegerLiteral_INTO2,
  /* 1253. <INSERT2> ::= INSERT INTO <CollectionDerivedTable> <Id List> '(' <Expr List> ')' */
  Rule_INSERT2_INSERT_INTO_LParan_RParan,
  /* 1254. <INSERT2> ::= INSERT INTO <CollectionDerivedTable> <Id List> <SELECT> */
  Rule_INSERT2_INSERT_INTO,
  /* 1255. <INSERT> ::= <INSERT_byVALUES> */
  Rule_INSERT,
  /* 1256. <INSERT> ::= INSERT INTO Id '(' <Id List> ')' <INSERT_EXECUTE> */
  Rule_INSERT_INSERT_INTO_Id_LParan_RParan,
  /* 1257. <INSERT> ::= INSERT INTO Id '(' <Id List> ')' <SELECT> */
  Rule_INSERT_INSERT_INTO_Id_LParan_RParan2,
  /* 1258. <INSERT> ::= INSERT INTO Id <INSERT_EXECUTE> */
  Rule_INSERT_INSERT_INTO_Id,
  /* 1259. <INSERT> ::= INSERT INTO Id <SELECT> */
  Rule_INSERT_INSERT_INTO_Id2,
  /* 1260. <INSERT> ::= <INSERT2> */
  Rule_INSERT2,
  /* 1261. <INSERT_EXECUTE> ::= <EXECUTE PROCEDURE0> */
  Rule_INSERT_EXECUTE,
  /* 1262. <INSERT_EXECUTE> ::= <EXECUTE FUNCTION0> */
  Rule_INSERT_EXECUTE2,
  /* 1263. <LOAD> ::= LOAD FROM StringLiteral DELIMITER StringLiteral INSERT INTO Id '(' <Id List> ')' */
  Rule_LOAD_LOAD_FROM_StringLiteral_DELIMITER_StringLiteral_INSERT_INTO_Id_LParan_RParan,
  /* 1264. <LOAD> ::= LOAD FROM StringLiteral DELIMITER StringLiteral INSERT INTO Id */
  Rule_LOAD_LOAD_FROM_StringLiteral_DELIMITER_StringLiteral_INSERT_INTO_Id,
  /* 1265. <LOAD> ::= LOAD FROM StringLiteral INSERT INTO Id '(' <Id List> ')' */
  Rule_LOAD_LOAD_FROM_StringLiteral_INSERT_INTO_Id_LParan_RParan,
  /* 1266. <LOAD> ::= LOAD FROM StringLiteral INSERT INTO Id */
  Rule_LOAD_LOAD_FROM_StringLiteral_INSERT_INTO_Id,
  /* 1267. <MERGE> ::= MERGE INTO <hint_clauseOpt> Id AS Id <MERGE_USING> <MERGE_WHEN MATCHED> <MERGE_WHEN NOT MATCHED> */
  Rule_MERGE_MERGE_INTO_Id_AS_Id,
  /* 1268. <MERGE> ::= MERGE INTO <hint_clauseOpt> Id Id <MERGE_USING> <MERGE_WHEN MATCHED> <MERGE_WHEN NOT MATCHED> */
  Rule_MERGE_MERGE_INTO_Id_Id,
  /* 1269. <MERGE> ::= MERGE INTO <hint_clauseOpt> Id <MERGE_USING> <MERGE_WHEN MATCHED> <MERGE_WHEN NOT MATCHED> */
  Rule_MERGE_MERGE_INTO_Id,
  /* 1270. <MERGE_USING> ::= USING Id AS Id ON <Expression> */
  Rule_MERGE_USING_USING_Id_AS_Id_ON,
  /* 1271. <MERGE_USING> ::= USING Id Id ON <Expression> */
  Rule_MERGE_USING_USING_Id_Id_ON,
  /* 1272. <MERGE_USING> ::= USING Id ON <Expression> */
  Rule_MERGE_USING_USING_Id_ON,
  /* 1273. <MERGE_USING> ::= USING <SELECT> AS Id ON <Expression> */
  Rule_MERGE_USING_USING_AS_Id_ON,
  /* 1274. <MERGE_USING> ::= USING <SELECT> Id ON <Expression> */
  Rule_MERGE_USING_USING_Id_ON2,
  /* 1275. <MERGE_USING> ::= USING <SELECT> ON <Expression> */
  Rule_MERGE_USING_USING_ON,
  /* 1276. <MERGE_WHEN MATCHED> ::= WHEN MATCHED THEN UPDATE <UPDATE_SET> */
  Rule_MERGE_WHENMATCHED_WHEN_MATCHED_THEN_UPDATE,
  /* 1277. <MERGE_WHEN NOT MATCHED> ::= WHEN NOT MATCHED THEN INSERT <Id List> '(' <Expr List> ')' */
  Rule_MERGE_WHENNOTMATCHED_WHEN_NOT_MATCHED_THEN_INSERT_LParan_RParan,
  /* 1278. <SELECTinto> ::= <SELECT_> <SELECT_unionS> <SELECT_ORDERopt> <SELECT_FORopt> <SELECT_INTOTableOpt> */
  Rule_SELECTinto,
  /* 1279. <SELECT> ::= <SELECTinto> */
  Rule_SELECT,
  /* 1280. <SELECT> ::= <SELECT_> <SELECT_unionS> <SELECT_ORDERopt> <SELECT_FORopt> */
  Rule_SELECT2,
  /* 1281. <SELECT_unionS> ::= UNION ALL <SELECT_> <SELECT_unionS> */
  Rule_SELECT_unionS_UNION_ALL,
  /* 1282. <SELECT_unionS> ::= UNION <SELECT_> <SELECT_unionS> */
  Rule_SELECT_unionS_UNION,
  /* 1283. <SELECT_unionS> ::=  */
  Rule_SELECT_unionS,
  /* 1284. <SELECT_> ::= SELECT <hint_clauseOpt> <SELECT_Projection> INTO <Id List> <SELECT_FROM> <SELECT_WHEREopt> <SELECT_HierarchicalOpt> <SELECT_GROUPopt> <SELECT_HAVINGopt> */
  Rule_SELECT__SELECT_INTO,
  /* 1285. <SELECT_> ::= SELECT <hint_clauseOpt> <SELECT_Projection> <SELECT_FROM> <SELECT_WHEREopt> <SELECT_HierarchicalOpt> <SELECT_GROUPopt> <SELECT_HAVINGopt> */
  Rule_SELECT__SELECT,
  /* 1286. <SELECT_Projection> ::= <SELECT_ProjectionOpt1> <SELECT_ProjectionOpt2> <SELECT_ProjectionOpt3> <SELECT_ColumnS> */
  Rule_SELECT_Projection,
  /* 1287. <SELECT_ProjectionOpt1> ::= SKIP <Expression> */
  Rule_SELECT_ProjectionOpt1_SKIP,
  /* 1288. <SELECT_ProjectionOpt1> ::=  */
  Rule_SELECT_ProjectionOpt1,
  /* 1289. <SELECT_ProjectionOpt2> ::= FIRST <Expression> */
  Rule_SELECT_ProjectionOpt2_FIRST,
  /* 1290. <SELECT_ProjectionOpt2> ::= LIMIT <Expression> */
  Rule_SELECT_ProjectionOpt2_LIMIT,
  /* 1291. <SELECT_ProjectionOpt2> ::= MIDDLE <Expression> */
  Rule_SELECT_ProjectionOpt2_MIDDLE,
  /* 1292. <SELECT_ProjectionOpt2> ::=  */
  Rule_SELECT_ProjectionOpt2,
  /* 1293. <SELECT_ProjectionOpt3> ::= ALL */
  Rule_SELECT_ProjectionOpt3_ALL,
  /* 1294. <SELECT_ProjectionOpt3> ::= DISTINCT */
  Rule_SELECT_ProjectionOpt3_DISTINCT,
  /* 1295. <SELECT_ProjectionOpt3> ::= UNIQUE */
  Rule_SELECT_ProjectionOpt3_UNIQUE,
  /* 1296. <SELECT_ProjectionOpt3> ::=  */
  Rule_SELECT_ProjectionOpt3,
  /* 1297. <SELECT_ColumnS> ::= <SELECT_Column> <SELECT_ColumnS_> */
  Rule_SELECT_ColumnS,
  /* 1298. <SELECT_ColumnS_> ::= ',' <SELECT_Column> <SELECT_ColumnS_> */
  Rule_SELECT_ColumnS__Comma,
  /* 1299. <SELECT_ColumnS_> ::=  */
  Rule_SELECT_ColumnS_,
  /* 1300. <SELECT_Column> ::= <Expression> AS Id */
  Rule_SELECT_Column_AS_Id,
  /* 1301. <SELECT_Column> ::= <Expression> Id */
  Rule_SELECT_Column_Id,
  /* 1302. <SELECT_Column> ::= <Expression> */
  Rule_SELECT_Column,
  /* 1303. <SELECT_Column> ::= Id '.*' */
  Rule_SELECT_Column_Id_DotTimes,
  /* 1304. <SELECT_Column> ::= '*' */
  Rule_SELECT_Column_Times,
  /* 1305. <SETquery> ::= MULTISET '(' <SELECT> ')' */
  Rule_SETquery_MULTISET_LParan_RParan,
  /* 1306. <SETquery> ::= MULTISET '(' SELECT ITEM <SELECT> ')' */
  Rule_SETquery_MULTISET_LParan_SELECT_ITEM_RParan,
  /* 1307. <SELECT_FROM> ::= FROM <SELECT_FROM_S> */
  Rule_SELECT_FROM_FROM,
  /* 1308. <SELECT_FROM> ::= FROM <SELECT_FROM_ANSItableS> */
  Rule_SELECT_FROM_FROM2,
  /* 1309. <SELECT_FROM_S> ::= <SELECT_FROM_> <SELECT_FROM_S_> */
  Rule_SELECT_FROM_S,
  /* 1310. <SELECT_FROM_S_> ::= ',' <SELECT_FROM_> <SELECT_FROM_S_> */
  Rule_SELECT_FROM_S__Comma,
  /* 1311. <SELECT_FROM_S_> ::=  */
  Rule_SELECT_FROM_S_,
  /* 1312. <SELECT_FROM_> ::= <SELECT_FROM_table> ',' <SELECT_FROM_OUTERs> */
  Rule_SELECT_FROM__Comma,
  /* 1313. <SELECT_FROM_> ::= <SELECT_FROM_table> */
  Rule_SELECT_FROM_,
  /* 1314. <SELECT_FROM_> ::= <SELECT_FROM_OUTERs> ',' <SELECT_FROM_tableS> */
  Rule_SELECT_FROM__Comma2,
  /* 1315. <SELECT_FROM_table> ::= <SELECT_FROM_tableOpt1> <SELECT_FROM_tableOpt2> <SELECT_FROM_ANSItable> */
  Rule_SELECT_FROM_table,
  /* 1316. <SELECT_FROM_table> ::= <SELECT_FROM_tableOpt1> <SELECT_FROM_tableOpt2> <SELECT_FROM_ANSItable> '(' <Id List> ')' */
  Rule_SELECT_FROM_table_LParan_RParan,
  /* 1317. <SELECT_FROM_tableOpt1> ::= IntegerLiteral SAMPLES OF */
  Rule_SELECT_FROM_tableOpt1_IntegerLiteral_SAMPLES_OF,
  /* 1318. <SELECT_FROM_tableOpt1> ::=  */
  Rule_SELECT_FROM_tableOpt1,
  /* 1319. <SELECT_FROM_tableOpt2> ::= LOCAL */
  Rule_SELECT_FROM_tableOpt2_LOCAL,
  /* 1320. <SELECT_FROM_tableOpt2> ::=  */
  Rule_SELECT_FROM_tableOpt2,
  /* 1321. <SELECT_FROM_OUTERs> ::= <SELECT_FROM_OUTER> <SELECT_FROM_OUTERs_> */
  Rule_SELECT_FROM_OUTERs,
  /* 1322. <SELECT_FROM_OUTERs_> ::= ',' <SELECT_FROM_OUTER> <SELECT_FROM_OUTERs_> */
  Rule_SELECT_FROM_OUTERs__Comma,
  /* 1323. <SELECT_FROM_OUTERs_> ::=  */
  Rule_SELECT_FROM_OUTERs_,
  /* 1324. <SELECT_FROM_OUTER> ::= OUTER <SELECT_FROM_table> */
  Rule_SELECT_FROM_OUTER_OUTER,
  /* 1325. <SELECT_FROM_OUTER> ::= OUTER '(' <SELECT_FROM_OUTER_s> ')' */
  Rule_SELECT_FROM_OUTER_OUTER_LParan_RParan,
  /* 1326. <SELECT_FROM_OUTER_s> ::= <SELECT_FROM_OUTER_> <SELECT_FROM_OUTER_s_> */
  Rule_SELECT_FROM_OUTER_s,
  /* 1327. <SELECT_FROM_OUTER_s_> ::= ',' <SELECT_FROM_OUTER_> <SELECT_FROM_OUTER_s_> */
  Rule_SELECT_FROM_OUTER_s__Comma,
  /* 1328. <SELECT_FROM_OUTER_s_> ::=  */
  Rule_SELECT_FROM_OUTER_s_,
  /* 1329. <SELECT_FROM_OUTER_> ::= <SELECT_FROM_tableS> <SELECT_FROM_OUTERs> */
  Rule_SELECT_FROM_OUTER_,
  /* 1330. <SELECT_FROM_OUTER_> ::= <SELECT_FROM_OUTERs> <SELECT_FROM_tableS> */
  Rule_SELECT_FROM_OUTER_2,
  /* 1331. <SELECT_FROM_tableS> ::= <SELECT_FROM_table> <SELECT_FROM_tableS_> */
  Rule_SELECT_FROM_tableS,
  /* 1332. <SELECT_FROM_tableS_> ::= ',' <SELECT_FROM_table> <SELECT_FROM_tableS_> */
  Rule_SELECT_FROM_tableS__Comma,
  /* 1333. <SELECT_FROM_tableS_> ::=  */
  Rule_SELECT_FROM_tableS_,
  /* 1334. <SELECT_FROM_ANSItableS> ::= <SELECT_FROM_ANSItable> <SELECT_FROM_ANSItableS_> */
  Rule_SELECT_FROM_ANSItableS,
  /* 1335. <SELECT_FROM_ANSItableS_> ::= ',' <SELECT_FROM_ANSItable> <SELECT_FROM_ANSItableS_> */
  Rule_SELECT_FROM_ANSItableS__Comma,
  /* 1336. <SELECT_FROM_ANSItableS_> ::=  */
  Rule_SELECT_FROM_ANSItableS_,
  /* 1337. <SELECT_FROM_ANSItable> ::= Id AS Id */
  Rule_SELECT_FROM_ANSItable_Id_AS_Id,
  /* 1338. <SELECT_FROM_ANSItable> ::= Id Id */
  Rule_SELECT_FROM_ANSItable_Id_Id,
  /* 1339. <SELECT_FROM_ANSItable> ::= Id */
  Rule_SELECT_FROM_ANSItable_Id,
  /* 1340. <SELECT_FROM_ANSItable> ::= ONLY '(' Id ')' AS Id */
  Rule_SELECT_FROM_ANSItable_ONLY_LParan_Id_RParan_AS_Id,
  /* 1341. <SELECT_FROM_ANSItable> ::= ONLY '(' Id ')' Id */
  Rule_SELECT_FROM_ANSItable_ONLY_LParan_Id_RParan_Id,
  /* 1342. <SELECT_FROM_ANSItable> ::= ONLY '(' Id ')' */
  Rule_SELECT_FROM_ANSItable_ONLY_LParan_Id_RParan,
  /* 1343. <SELECT_FROM_ANSItable> ::= <CollectionDerivedTable> */
  Rule_SELECT_FROM_ANSItable,
  /* 1344. <SELECT_FROM_ANSItable> ::= <IteratorFunctionTable> */
  Rule_SELECT_FROM_ANSItable2,
  /* 1345. <SELECT_FROM_ANSItable> ::= <SELECT_FROM_ANSIjoin> */
  Rule_SELECT_FROM_ANSItable3,
  /* 1346. <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' AS Id '(' <Id List> ')' */
  Rule_CollectionDerivedTable_TABLE_LParan_RParan_AS_Id_LParan_RParan,
  /* 1347. <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' Id '(' <Id List> ')' */
  Rule_CollectionDerivedTable_TABLE_LParan_RParan_Id_LParan_RParan,
  /* 1348. <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' '(' <Id List> ')' */
  Rule_CollectionDerivedTable_TABLE_LParan_RParan_LParan_RParan,
  /* 1349. <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' AS Id */
  Rule_CollectionDerivedTable_TABLE_LParan_RParan_AS_Id,
  /* 1350. <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' Id */
  Rule_CollectionDerivedTable_TABLE_LParan_RParan_Id,
  /* 1351. <CollectionDerivedTable> ::= TABLE '(' <Expression> ')' */
  Rule_CollectionDerivedTable_TABLE_LParan_RParan,
  /* 1352. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' AS Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id_LParan_RParan,
  /* 1353. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' AS Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id_LParan_RParan,
  /* 1354. <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' AS Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id_LParan_RParan,
  /* 1355. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' AS Id */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id,
  /* 1356. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' AS Id */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id,
  /* 1357. <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' AS Id */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id,
  /* 1358. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id_LParan_RParan,
  /* 1359. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id_LParan_RParan,
  /* 1360. <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id_LParan_RParan,
  /* 1361. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' Id */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id,
  /* 1362. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' Id */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id,
  /* 1363. <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' Id */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id,
  /* 1364. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_LParan_RParan,
  /* 1365. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_LParan_RParan,
  /* 1366. <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_LParan_RParan,
  /* 1367. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' <Expr List> ')' ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan,
  /* 1368. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' <Expr List> ')' ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan,
  /* 1369. <IteratorFunctionTable> ::= TABLE '(' Id '(' <Expr List> ')' ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan,
  /* 1370. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' AS Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id_LParan_RParan2,
  /* 1371. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' AS Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id_LParan_RParan2,
  /* 1372. <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' AS Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id_LParan_RParan2,
  /* 1373. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' AS Id */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_AS_Id2,
  /* 1374. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' AS Id */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_AS_Id2,
  /* 1375. <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' AS Id */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_AS_Id2,
  /* 1376. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id_LParan_RParan2,
  /* 1377. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id_LParan_RParan2,
  /* 1378. <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' Id '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id_LParan_RParan2,
  /* 1379. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' Id */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_Id2,
  /* 1380. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' Id */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_Id2,
  /* 1381. <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' Id */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_Id2,
  /* 1382. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan_LParan_RParan2,
  /* 1383. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan_LParan_RParan2,
  /* 1384. <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' '(' <Id List> ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan_LParan_RParan2,
  /* 1385. <IteratorFunctionTable> ::= TABLE '(' FUNCTION Id '(' ')' ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_FUNCTION_Id_LParan_RParan_RParan2,
  /* 1386. <IteratorFunctionTable> ::= TABLE '(' PROCEDURE Id '(' ')' ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_PROCEDURE_Id_LParan_RParan_RParan2,
  /* 1387. <IteratorFunctionTable> ::= TABLE '(' Id '(' ')' ')' */
  Rule_IteratorFunctionTable_TABLE_LParan_Id_LParan_RParan_RParan2,
  /* 1388. <SELECT_FROM_ANSIjoin> ::= <SELECT_FROM_ANSItable> <SELECT_FROM_ANSIjoin_S> */
  Rule_SELECT_FROM_ANSIjoin,
  /* 1389. <SELECT_FROM_ANSIjoin> ::= '(' <SELECT_FROM_ANSIjoin> ')' */
  Rule_SELECT_FROM_ANSIjoin_LParan_RParan,
  /* 1390. <SELECT_FROM_ANSIjoin_S> ::= <SELECT_FROM_ANSIjoin_> <SELECT_FROM_ANSIjoin_S> */
  Rule_SELECT_FROM_ANSIjoin_S,
  /* 1391. <SELECT_FROM_ANSIjoin_S> ::= <SELECT_FROM_ANSIjoin_> */
  Rule_SELECT_FROM_ANSIjoin_S2,
  /* 1392. <SELECT_FROM_ANSIjoin_S> ::= CROSS JOIN <SELECT_FROM_ANSItable> */
  Rule_SELECT_FROM_ANSIjoin_S_CROSS_JOIN,
  /* 1393. <SELECT_FROM_ANSIjoin_> ::= INNER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
  Rule_SELECT_FROM_ANSIjoin__INNER_JOIN_ON,
  /* 1394. <SELECT_FROM_ANSIjoin_> ::= LEFT OUTER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
  Rule_SELECT_FROM_ANSIjoin__LEFT_OUTER_JOIN_ON,
  /* 1395. <SELECT_FROM_ANSIjoin_> ::= RIGHT OUTER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
  Rule_SELECT_FROM_ANSIjoin__RIGHT_OUTER_JOIN_ON,
  /* 1396. <SELECT_FROM_ANSIjoin_> ::= FULL OUTER JOIN <SELECT_FROM_ANSItable> ON <Expression> */
  Rule_SELECT_FROM_ANSIjoin__FULL_OUTER_JOIN_ON,
  /* 1397. <SELECT_WHEREopt> ::= WHERE <Expression> */
  Rule_SELECT_WHEREopt_WHERE,
  /* 1398. <SELECT_WHEREopt> ::=  */
  Rule_SELECT_WHEREopt,
  /* 1399. <SELECT_HierarchicalOpt> ::= CONNECT BY NOCYCLE <SELECT_Hierarchical_CONNECT_BY_Expression> */
  Rule_SELECT_HierarchicalOpt_CONNECT_BY_NOCYCLE,
  /* 1400. <SELECT_HierarchicalOpt> ::= CONNECT BY <SELECT_Hierarchical_CONNECT_BY_Expression> */
  Rule_SELECT_HierarchicalOpt_CONNECT_BY,
  /* 1401. <SELECT_HierarchicalOpt> ::= START WITH <Expression> CONNECT BY NOCYCLE <SELECT_Hierarchical_CONNECT_BY_Expression> */
  Rule_SELECT_HierarchicalOpt_START_WITH_CONNECT_BY_NOCYCLE,
  /* 1402. <SELECT_HierarchicalOpt> ::= START WITH <Expression> CONNECT BY <SELECT_Hierarchical_CONNECT_BY_Expression> */
  Rule_SELECT_HierarchicalOpt_START_WITH_CONNECT_BY,
  /* 1403. <SELECT_HierarchicalOpt> ::=  */
  Rule_SELECT_HierarchicalOpt,
  /* 1404. <SELECT_Hierarchical_CONNECT_BY_Expression> ::= PRIOR <Expression> */
  Rule_SELECT_Hierarchical_CONNECT_BY_Expression_PRIOR,
  /* 1405. <SELECT_Hierarchical_CONNECT_BY_Expression> ::= <Expression> */
  Rule_SELECT_Hierarchical_CONNECT_BY_Expression,
  /* 1406. <SELECT_GROUPopt> ::= GROUP BY <Expr List> */
  Rule_SELECT_GROUPopt_GROUP_BY,
  /* 1407. <SELECT_GROUPopt> ::=  */
  Rule_SELECT_GROUPopt,
  /* 1408. <SELECT_HAVINGopt> ::= HAVING <Expression> */
  Rule_SELECT_HAVINGopt_HAVING,
  /* 1409. <SELECT_HAVINGopt> ::=  */
  Rule_SELECT_HAVINGopt,
  /* 1410. <SELECT_ORDERopt> ::= ORDER BY <SELECT_ORDER_s> */
  Rule_SELECT_ORDERopt_ORDER_BY,
  /* 1411. <SELECT_ORDERopt> ::=  */
  Rule_SELECT_ORDERopt,
  /* 1412. <SELECT_ORDER_s> ::= <SELECT_ORDER_> <SELECT_ORDER_s_> */
  Rule_SELECT_ORDER_s,
  /* 1413. <SELECT_ORDER_s_> ::= ',' <SELECT_ORDER_> <SELECT_ORDER_s_> */
  Rule_SELECT_ORDER_s__Comma,
  /* 1414. <SELECT_ORDER_s_> ::=  */
  Rule_SELECT_ORDER_s_,
  /* 1415. <SELECT_ORDER_> ::= <Expression> <SELECT_ORDER_opt1> ASC */
  Rule_SELECT_ORDER__ASC,
  /* 1416. <SELECT_ORDER_> ::= <Expression> <SELECT_ORDER_opt1> DESC */
  Rule_SELECT_ORDER__DESC,
  /* 1417. <SELECT_ORDER_> ::= <Expression> <SELECT_ORDER_opt1> */
  Rule_SELECT_ORDER_,
  /* 1418. <SELECT_ORDER_opt1> ::= '[' IntegerLiteral ',' IntegerLiteral ']' */
  Rule_SELECT_ORDER_opt1_LBracket_IntegerLiteral_Comma_IntegerLiteral_RBracket,
  /* 1419. <SELECT_ORDER_opt1> ::=  */
  Rule_SELECT_ORDER_opt1,
  /* 1420. <SELECT_FORopt> ::= FOR READ ONLY */
  Rule_SELECT_FORopt_FOR_READ_ONLY,
  /* 1421. <SELECT_FORopt> ::= FOR UPDATE OF <Id List> */
  Rule_SELECT_FORopt_FOR_UPDATE_OF,
  /* 1422. <SELECT_FORopt> ::= FOR UPDATE */
  Rule_SELECT_FORopt_FOR_UPDATE,
  /* 1423. <SELECT_FORopt> ::=  */
  Rule_SELECT_FORopt,
  /* 1424. <SELECT_INTOTableOpt> ::= INTO TEMP Id WITH NO LOG */
  Rule_SELECT_INTOTableOpt_INTO_TEMP_Id_WITH_NO_LOG,
  /* 1425. <SELECT_INTOTableOpt> ::= INTO TEMP Id */
  Rule_SELECT_INTOTableOpt_INTO_TEMP_Id,
  /* 1426. <SELECT_INTOTableOpt> ::= INTO <TableType> Id <CREATE TABLE_StorageOpt> <IndexLockmodeOpt> */
  Rule_SELECT_INTOTableOpt_INTO_Id,
  /* 1427. <SELECT_INTOTableOpt> ::= INTO SCRATCH Id */
  Rule_SELECT_INTOTableOpt_INTO_SCRATCH_Id,
  /* 1428. <SELECT_INTOTableOpt> ::= INTO EXTERNAL Id USING <SELECT TABLE_optionS> DATAFILES <StringLiteralS> <SELECT TABLE_optionS> */
  Rule_SELECT_INTOTableOpt_INTO_EXTERNAL_Id_USING_DATAFILES,
  /* 1429. <SELECT TABLE_optionS> ::= <SELECT TABLE_option> <SELECT TABLE_optionS_> */
  Rule_SELECTTABLE_optionS,
  /* 1430. <SELECT TABLE_optionS> ::=  */
  Rule_SELECTTABLE_optionS2,
  /* 1431. <SELECT TABLE_optionS_> ::= ',' <SELECT TABLE_option> <SELECT TABLE_optionS_> */
  Rule_SELECTTABLE_optionS__Comma,
  /* 1432. <SELECT TABLE_optionS_> ::=  */
  Rule_SELECTTABLE_optionS_,
  /* 1433. <SELECT TABLE_option> ::= FORMAT StringLiteral */
  Rule_SELECTTABLE_option_FORMAT_StringLiteral,
  /* 1434. <SELECT TABLE_option> ::= CODESET StringLiteral */
  Rule_SELECTTABLE_option_CODESET_StringLiteral,
  /* 1435. <SELECT TABLE_option> ::= DELIMITER StringLiteral */
  Rule_SELECTTABLE_option_DELIMITER_StringLiteral,
  /* 1436. <SELECT TABLE_option> ::= RECORDEND StringLiteral */
  Rule_SELECTTABLE_option_RECORDEND_StringLiteral,
  /* 1437. <SELECT TABLE_option> ::= ESCAPE */
  Rule_SELECTTABLE_option_ESCAPE,
  /* 1438. <TRUNCATE> ::= TRUNCATE ONLY TABLE Id */
  Rule_TRUNCATE_TRUNCATE_ONLY_TABLE_Id,
  /* 1439. <TRUNCATE> ::= TRUNCATE ONLY Id */
  Rule_TRUNCATE_TRUNCATE_ONLY_Id,
  /* 1440. <TRUNCATE> ::= TRUNCATE TABLE Id */
  Rule_TRUNCATE_TRUNCATE_TABLE_Id,
  /* 1441. <TRUNCATE> ::= TRUNCATE Id */
  Rule_TRUNCATE_TRUNCATE_Id,
  /* 1442. <UNLOAD> ::= UNLOAD TO StringLiteral DELIMITER StringLiteral <SELECT> */
  Rule_UNLOAD_UNLOAD_TO_StringLiteral_DELIMITER_StringLiteral,
  /* 1443. <UNLOAD> ::= UNLOAD TO StringLiteral <SELECT> */
  Rule_UNLOAD_UNLOAD_TO_StringLiteral,
  /* 1444. <UPDATE> ::= UPDATE <hint_clauseOpt> Id <UPDATE_SET> <UPDATE_WHERE> */
  Rule_UPDATE_UPDATE_Id,
  /* 1445. <UPDATE> ::= UPDATE <hint_clauseOpt> ONLY '(' Id ')' <UPDATE_SET> <UPDATE_WHERE> */
  Rule_UPDATE_UPDATE_ONLY_LParan_Id_RParan,
  /* 1446. <UPDATE> ::= UPDATE <CollectionDerivedTable> <UPDATE_SET> WHERE CURRENT OF Id */
  Rule_UPDATE_UPDATE_WHERE_CURRENT_OF_Id,
  /* 1447. <UPDATE> ::= UPDATE <CollectionDerivedTable> <UPDATE_SET> */
  Rule_UPDATE_UPDATE,
  /* 1448. <UPDATE_SET> ::= SET <Assign List> */
  Rule_UPDATE_SET_SET,
  /* 1449. <UPDATE_SET> ::= SET <Id List> '=' '(' <Expr List> ')' */
  Rule_UPDATE_SET_SET_Eq_LParan_RParan,
  /* 1450. <UPDATE_SET> ::= SET '*' '=' '(' <Expr List> ')' */
  Rule_UPDATE_SET_SET_Times_Eq_LParan_RParan,
  /* 1451. <UPDATE_SET> ::= SET <Id List> '=' '(' <FuncCall> ')' */
  Rule_UPDATE_SET_SET_Eq_LParan_RParan2,
  /* 1452. <UPDATE_SET> ::= SET '*' '=' '(' <FuncCall> ')' */
  Rule_UPDATE_SET_SET_Times_Eq_LParan_RParan2,
  /* 1453. <Assign List> ::= Id '=' <Expression> <Assign List_> */
  Rule_AssignList_Id_Eq,
  /* 1454. <Assign List_> ::= ',' Id '=' <Expression> <Assign List_> */
  Rule_AssignList__Comma_Id_Eq,
  /* 1455. <Assign List_> ::=  */
  Rule_AssignList_,
  /* 1456. <NamedParamS> ::= <NamedParam> <NamedParamS_> */
  Rule_NamedParamS,
  /* 1457. <NamedParamS_> ::= ',' <NamedParam> <NamedParamS_> */
  Rule_NamedParamS__Comma,
  /* 1458. <NamedParamS_> ::=  */
  Rule_NamedParamS_,
  /* 1459. <NamedParam> ::= Id '=>' <Expression> */
  Rule_NamedParam_Id_EqGt,
  /* 1460. <FuncCall> ::= Id '(' <Expr List> ')' */
  Rule_FuncCall_Id_LParan_RParan,
  /* 1461. <FuncCall> ::= Id '(' ')' */
  Rule_FuncCall_Id_LParan_RParan2,
  /* 1462. <FuncCall> ::= CAST '(' <Expression> AS <Type> ')' */
  Rule_FuncCall_CAST_LParan_AS_RParan,
  /* 1463. <FuncCall> ::= Id '(' <NamedParamS> ')' */
  Rule_FuncCall_Id_LParan_RParan3,
  /* 1464. <FuncCall> ::= Id '(' <SELECT> ')' */
  Rule_FuncCall_Id_LParan_RParan4,
  /* 1465. <UPDATE_WHERE> ::= <SELECT_FROM> WHERE <Expression> */
  Rule_UPDATE_WHERE_WHERE,
  /* 1466. <UPDATE_WHERE> ::= <SELECT_FROM> */
  Rule_UPDATE_WHERE,
  /* 1467. <UPDATE_WHERE> ::= WHERE <Expression> */
  Rule_UPDATE_WHERE_WHERE2,
  /* 1468. <UPDATE_WHERE> ::=  */
  Rule_UPDATE_WHERE2,
  /* 1469. <UPDATE_WHERE> ::= WHERE CURRENT OF Id */
  Rule_UPDATE_WHERE_WHERE_CURRENT_OF_Id,
  /* 1470. <CLOSE> ::= CLOSE Id */
  Rule_CLOSE_CLOSE_Id,
  /* 1471. <DECLARE> ::= DECLARE Id CURSOR <WITH HOLDopt> FOR <INSERT_byVALUES> */
  Rule_DECLARE_DECLARE_Id_CURSOR_FOR,
  /* 1472. <DECLARE> ::= DECLARE Id CURSOR <WITH HOLDopt> <DECLAREoption2> */
  Rule_DECLARE_DECLARE_Id_CURSOR,
  /* 1473. <DECLARE> ::= DECLARE Id CURSOR <WITH HOLDopt> <DECLAREoption3> */
  Rule_DECLARE_DECLARE_Id_CURSOR2,
  /* 1474. <DECLARE> ::= DECLARE Id SCROLL CURSOR <WITH HOLDopt> <DECLAREoption3> */
  Rule_DECLARE_DECLARE_Id_SCROLL_CURSOR,
  /* 1475. <DECLARE> ::= DECLARE Id CURSOR FOR <SELECT> */
  Rule_DECLARE_DECLARE_Id_CURSOR_FOR2,
  /* 1476. <DECLARE> ::= DECLARE Id CURSOR FOR <INSERT2> */
  Rule_DECLARE_DECLARE_Id_CURSOR_FOR3,
  /* 1477. <WITH HOLDopt> ::= WITH HOLD */
  Rule_WITHHOLDopt_WITH_HOLD,
  /* 1478. <WITH HOLDopt> ::=  */
  Rule_WITHHOLDopt,
  /* 1479. <DECLAREoption2> ::= <SELECT> FOR READ ONLY */
  Rule_DECLAREoption2_FOR_READ_ONLY,
  /* 1480. <DECLAREoption2> ::= <SELECT> FOR UPDATE OF <Id List> */
  Rule_DECLAREoption2_FOR_UPDATE_OF,
  /* 1481. <DECLAREoption2> ::= <SELECT> FOR UPDATE */
  Rule_DECLAREoption2_FOR_UPDATE,
  /* 1482. <DECLAREoption3> ::= FOR <SELECT> */
  Rule_DECLAREoption3_FOR,
  /* 1483. <DECLAREoption3> ::= FOR Id */
  Rule_DECLAREoption3_FOR_Id,
  /* 1484. <DECLAREoption3> ::= FOR <EXECUTE PROCEDURE> */
  Rule_DECLAREoption3_FOR2,
  /* 1485. <DECLAREoption3> ::= FOR <EXECUTE FUNCTION> */
  Rule_DECLAREoption3_FOR3,
  /* 1486. <FETCH> ::= FETCH <FETCHopt1> Id <FETCHopt2> */
  Rule_FETCH_FETCH_Id,
  /* 1487. <FETCHopt1> ::= NEXT */
  Rule_FETCHopt1_NEXT,
  /* 1488. <FETCHopt1> ::= PRIOR */
  Rule_FETCHopt1_PRIOR,
  /* 1489. <FETCHopt1> ::= PREVIOUS */
  Rule_FETCHopt1_PREVIOUS,
  /* 1490. <FETCHopt1> ::= FIRST */
  Rule_FETCHopt1_FIRST,
  /* 1491. <FETCHopt1> ::= LAST */
  Rule_FETCHopt1_LAST,
  /* 1492. <FETCHopt1> ::= CURRENT */
  Rule_FETCHopt1_CURRENT,
  /* 1493. <FETCHopt1> ::= RELATIVE '+' IntegerLiteral */
  Rule_FETCHopt1_RELATIVE_Plus_IntegerLiteral,
  /* 1494. <FETCHopt1> ::= RELATIVE '+' Id */
  Rule_FETCHopt1_RELATIVE_Plus_Id,
  /* 1495. <FETCHopt1> ::= RELATIVE '-' IntegerLiteral */
  Rule_FETCHopt1_RELATIVE_Minus_IntegerLiteral,
  /* 1496. <FETCHopt1> ::= RELATIVE '-' Id */
  Rule_FETCHopt1_RELATIVE_Minus_Id,
  /* 1497. <FETCHopt1> ::= RELATIVE IntegerLiteral */
  Rule_FETCHopt1_RELATIVE_IntegerLiteral,
  /* 1498. <FETCHopt1> ::= RELATIVE Id */
  Rule_FETCHopt1_RELATIVE_Id,
  /* 1499. <FETCHopt1> ::= ABSOLUTE IntegerLiteral */
  Rule_FETCHopt1_ABSOLUTE_IntegerLiteral,
  /* 1500. <FETCHopt1> ::= ABSOLUTE Id */
  Rule_FETCHopt1_ABSOLUTE_Id,
  /* 1501. <FETCHopt1> ::=  */
  Rule_FETCHopt1,
  /* 1502. <FETCHopt2> ::= USING SQL DESCRIPTOR StringLiteral */
  Rule_FETCHopt2_USING_SQL_DESCRIPTOR_StringLiteral,
  /* 1503. <FETCHopt2> ::= USING SQL DESCRIPTOR Id */
  Rule_FETCHopt2_USING_SQL_DESCRIPTOR_Id,
  /* 1504. <FETCHopt2> ::= USING DESCRIPTOR Id */
  Rule_FETCHopt2_USING_DESCRIPTOR_Id,
  /* 1505. <FETCHopt2> ::= <EXECUTE FUNCTION_intoS> */
  Rule_FETCHopt2,
  /* 1506. <FLUSH> ::= FLUSH Id */
  Rule_FLUSH_FLUSH_Id,
  /* 1507. <FREE> ::= FREE Id */
  Rule_FREE_FREE_Id,
  /* 1508. <OPEN> ::= OPEN Id USING <Id List> <OPENopt1> */
  Rule_OPEN_OPEN_Id_USING,
  /* 1509. <OPEN> ::= OPEN Id USING SQL DESCRIPTOR StringLiteral DESCRIPTOR Id <OPENopt1> */
  Rule_OPEN_OPEN_Id_USING_SQL_DESCRIPTOR_StringLiteral_DESCRIPTOR_Id,
  /* 1510. <OPEN> ::= OPEN Id USING SQL DESCRIPTOR Id DESCRIPTOR Id <OPENopt1> */
  Rule_OPEN_OPEN_Id_USING_SQL_DESCRIPTOR_Id_DESCRIPTOR_Id,
  /* 1511. <OPEN> ::= OPEN Id <OPENopt1> */
  Rule_OPEN_OPEN_Id,
  /* 1512. <OPENopt1> ::= WITH REOPTIMIZATION */
  Rule_OPENopt1_WITH_REOPTIMIZATION,
  /* 1513. <OPENopt1> ::=  */
  Rule_OPENopt1,
  /* 1514. <PUT> ::= PUT Id FROM <EXECUTE FUNCTION_into> <EXECUTE FUNCTION_intoS_> */
  Rule_PUT_PUT_Id_FROM,
  /* 1515. <PUT> ::= PUT Id USING SQL DESCRIPTOR StringLiteral */
  Rule_PUT_PUT_Id_USING_SQL_DESCRIPTOR_StringLiteral,
  /* 1516. <PUT> ::= PUT Id USING SQL DESCRIPTOR Id */
  Rule_PUT_PUT_Id_USING_SQL_DESCRIPTOR_Id,
  /* 1517. <PUT> ::= PUT Id USING DESCRIPTOR Id */
  Rule_PUT_PUT_Id_USING_DESCRIPTOR_Id,
  /* 1518. <SET AUTOFREE> ::= SET AUTOFREE */
  Rule_SETAUTOFREE_SET_AUTOFREE,
  /* 1519. <EXECUTE IMMEDIATE> ::= EXECUTE IMMEDIATE StringLiteral */
  Rule_EXECUTEIMMEDIATE_EXECUTE_IMMEDIATE_StringLiteral,
  /* 1520. <EXECUTE IMMEDIATE> ::= EXECUTE IMMEDIATE Id */
  Rule_EXECUTEIMMEDIATE_EXECUTE_IMMEDIATE_Id,
  /* 1521. <INFO> ::= INFO TABLES */
  Rule_INFO_INFO_TABLES,
  /* 1522. <INFO> ::= INFO COLUMNS FOR Id */
  Rule_INFO_INFO_COLUMNS_FOR_Id,
  /* 1523. <INFO> ::= INFO INDEXES FOR Id */
  Rule_INFO_INFO_INDEXES_FOR_Id,
  /* 1524. <INFO> ::= INFO STATUS FOR Id */
  Rule_INFO_INFO_STATUS_FOR_Id,
  /* 1525. <INFO> ::= INFO PRIVILEGES FOR Id */
  Rule_INFO_INFO_PRIVILEGES_FOR_Id,
  /* 1526. <INFO> ::= INFO ACCESS FOR Id */
  Rule_INFO_INFO_ACCESS_FOR_Id,
  /* 1527. <INFO> ::= INFO FRAGMENTS FOR Id */
  Rule_INFO_INFO_FRAGMENTS_FOR_Id,
  /* 1528. <INFO> ::= INFO REFERENCES FOR Id */
  Rule_INFO_INFO_REFERENCES_FOR_Id,
  /* 1529. <GRANT> ::= GRANT <db_privileges> <GRANT_TO1> */
  Rule_GRANT_GRANT,
  /* 1530. <GRANT> ::= GRANT DEFAULT ROLE Id <GRANT_TO1> */
  Rule_GRANT_GRANT_DEFAULT_ROLE_Id,
  /* 1531. <GRANT> ::= GRANT DEFAULT ROLE StringLiteral <GRANT_TO1> */
  Rule_GRANT_GRANT_DEFAULT_ROLE_StringLiteral,
  /* 1532. <GRANT> ::= GRANT Id <GRANT_TO2> */
  Rule_GRANT_GRANT_Id,
  /* 1533. <GRANT> ::= GRANT <SecurityOption_GRANT> */
  Rule_GRANT_GRANT2,
  /* 1534. <GRANT> ::= GRANT <tab_privileges_GRANT> <GRANT_TO2> */
  Rule_GRANT_GRANT3,
  /* 1535. <GRANT> ::= GRANT <routine_privileges> <GRANT_TO2> */
  Rule_GRANT_GRANT4,
  /* 1536. <GRANT> ::= GRANT <lang_privileges> <GRANT_TO2> */
  Rule_GRANT_GRANT5,
  /* 1537. <GRANT> ::= GRANT <type_privileges> <GRANT_TO2> */
  Rule_GRANT_GRANT6,
  /* 1538. <GRANT> ::= GRANT <seq_privileges> <GRANT_TO2> */
  Rule_GRANT_GRANT7,
  /* 1539. <GRANT_TO1> ::= TO PUBLIC */
  Rule_GRANT_TO1_TO_PUBLIC,
  /* 1540. <GRANT_TO1> ::= TO <Id List> */
  Rule_GRANT_TO1_TO,
  /* 1541. <GRANT_TO1> ::= TO <StringLiteralS> */
  Rule_GRANT_TO1_TO2,
  /* 1542. <GRANT_TO2> ::= TO PUBLIC WITH GRANT OPTION AS Id */
  Rule_GRANT_TO2_TO_PUBLIC_WITH_GRANT_OPTION_AS_Id,
  /* 1543. <GRANT_TO2> ::= TO <Id List> WITH GRANT OPTION AS Id */
  Rule_GRANT_TO2_TO_WITH_GRANT_OPTION_AS_Id,
  /* 1544. <GRANT_TO2> ::= TO <StringLiteralS> WITH GRANT OPTION AS Id */
  Rule_GRANT_TO2_TO_WITH_GRANT_OPTION_AS_Id2,
  /* 1545. <GRANT_TO2> ::= <GRANT_TO1> AS Id */
  Rule_GRANT_TO2_AS_Id,
  /* 1546. <GRANT_TO2> ::= TO <Id List> AS Id */
  Rule_GRANT_TO2_TO_AS_Id,
  /* 1547. <GRANT_TO2> ::= TO <StringLiteralS> AS Id */
  Rule_GRANT_TO2_TO_AS_Id2,
  /* 1548. <GRANT_TO2> ::= TO PUBLIC WITH GRANT OPTION */
  Rule_GRANT_TO2_TO_PUBLIC_WITH_GRANT_OPTION,
  /* 1549. <GRANT_TO2> ::= TO <Id List> WITH GRANT OPTION */
  Rule_GRANT_TO2_TO_WITH_GRANT_OPTION,
  /* 1550. <GRANT_TO2> ::= TO <StringLiteralS> WITH GRANT OPTION */
  Rule_GRANT_TO2_TO_WITH_GRANT_OPTION2,
  /* 1551. <GRANT_TO2> ::= <GRANT_TO1> */
  Rule_GRANT_TO2,
  /* 1552. <db_privileges> ::= CONNECT */
  Rule_db_privileges_CONNECT,
  /* 1553. <db_privileges> ::= RESOURCE */
  Rule_db_privileges_RESOURCE,
  /* 1554. <db_privileges> ::= DBA */
  Rule_db_privileges_DBA,
  /* 1555. <SecurityOption_GRANT> ::= <DBSECADM_clause> TO <USERs1> */
  Rule_SecurityOption_GRANT_TO,
  /* 1556. <SecurityOption_GRANT> ::= <EXEMPTION_clause> TO <USERs1> */
  Rule_SecurityOption_GRANT_TO2,
  /* 1557. <SecurityOption_GRANT> ::= <SECURITY LABEL_clause> TO <USERs1> <SECURITY LABEL_clause_FOR> */
  Rule_SecurityOption_GRANT_TO3,
  /* 1558. <SecurityOption_GRANT> ::= <SETSESSIONAUTH_clause> TO <USERs1> */
  Rule_SecurityOption_GRANT_TO4,
  /* 1559. <SecurityOption_GRANT> ::= <SETSESSIONAUTH_clause> TO ROLE <Id List> */
  Rule_SecurityOption_GRANT_TO_ROLE,
  /* 1560. <USERs1> ::= USER <Id List> */
  Rule_USERs1_USER,
  /* 1561. <USERs1> ::= <Id List> */
  Rule_USERs1,
  /* 1562. <DBSECADM_clause> ::= DBSECADM */
  Rule_DBSECADM_clause_DBSECADM,
  /* 1563. <EXEMPTION_clause> ::= EXEMPTION ON RULE <EXEMPTION_clause_RULE> FOR Id */
  Rule_EXEMPTION_clause_EXEMPTION_ON_RULE_FOR_Id,
  /* 1564. <EXEMPTION_clause_RULE> ::= IDSLBACREADARRAY */
  Rule_EXEMPTION_clause_RULE_IDSLBACREADARRAY,
  /* 1565. <EXEMPTION_clause_RULE> ::= IDSLBACREADTREE */
  Rule_EXEMPTION_clause_RULE_IDSLBACREADTREE,
  /* 1566. <EXEMPTION_clause_RULE> ::= IDSLBACREADSET */
  Rule_EXEMPTION_clause_RULE_IDSLBACREADSET,
  /* 1567. <EXEMPTION_clause_RULE> ::= IDSLABCWRITEARRAY */
  Rule_EXEMPTION_clause_RULE_IDSLABCWRITEARRAY,
  /* 1568. <EXEMPTION_clause_RULE> ::= IDSLABCWRITEARRAY WRITEDOWN */
  Rule_EXEMPTION_clause_RULE_IDSLABCWRITEARRAY_WRITEDOWN,
  /* 1569. <EXEMPTION_clause_RULE> ::= IDSLABCWRITEARRAY WRITEUP */
  Rule_EXEMPTION_clause_RULE_IDSLABCWRITEARRAY_WRITEUP,
  /* 1570. <EXEMPTION_clause_RULE> ::= IDSLBACWRITESET */
  Rule_EXEMPTION_clause_RULE_IDSLBACWRITESET,
  /* 1571. <EXEMPTION_clause_RULE> ::= IDSLBACWRITETREE */
  Rule_EXEMPTION_clause_RULE_IDSLBACWRITETREE,
  /* 1572. <EXEMPTION_clause_RULE> ::= ALL */
  Rule_EXEMPTION_clause_RULE_ALL,
  /* 1573. <SECURITY LABEL_clause> ::= SECURITY LABEL Id */
  Rule_SECURITYLABEL_clause_SECURITY_LABEL_Id,
  /* 1574. <SECURITY LABEL_clause_FOR> ::= FOR ALL ACCESS */
  Rule_SECURITYLABEL_clause_FOR_FOR_ALL_ACCESS,
  /* 1575. <SECURITY LABEL_clause_FOR> ::= FOR READ ACCESS */
  Rule_SECURITYLABEL_clause_FOR_FOR_READ_ACCESS,
  /* 1576. <SECURITY LABEL_clause_FOR> ::= FOR WRITE ACCESS */
  Rule_SECURITYLABEL_clause_FOR_FOR_WRITE_ACCESS,
  /* 1577. <SETSESSIONAUTH_clause> ::= SETSESSIONAUTH ON PUBLIC */
  Rule_SETSESSIONAUTH_clause_SETSESSIONAUTH_ON_PUBLIC,
  /* 1578. <SETSESSIONAUTH_clause> ::= SETSESSIONAUTH ON USER <Id List> */
  Rule_SETSESSIONAUTH_clause_SETSESSIONAUTH_ON_USER,
  /* 1579. <SETSESSIONAUTH_clause> ::= SETSESSIONAUTH ON <Id List> */
  Rule_SETSESSIONAUTH_clause_SETSESSIONAUTH_ON,
  /* 1580. <tab_privileges_GRANT> ::= ALL PRIVILEGES ON Id */
  Rule_tab_privileges_GRANT_ALL_PRIVILEGES_ON_Id,
  /* 1581. <tab_privileges_GRANT> ::= ALL ON Id */
  Rule_tab_privileges_GRANT_ALL_ON_Id,
  /* 1582. <tab_privileges_GRANT> ::= <tab_privilege_GRANT> <tab_privilegeS__GRANT> ON Id */
  Rule_tab_privileges_GRANT_ON_Id,
  /* 1583. <tab_privilegeS__GRANT> ::= ',' <tab_privilege_GRANT> <tab_privilegeS__GRANT> */
  Rule_tab_privilegeS__GRANT_Comma,
  /* 1584. <tab_privilegeS__GRANT> ::=  */
  Rule_tab_privilegeS__GRANT,
  /* 1585. <tab_privilege_GRANT> ::= INSERT */
  Rule_tab_privilege_GRANT_INSERT,
  /* 1586. <tab_privilege_GRANT> ::= DELETE */
  Rule_tab_privilege_GRANT_DELETE,
  /* 1587. <tab_privilege_GRANT> ::= UPDATE '(' <Id List> ')' */
  Rule_tab_privilege_GRANT_UPDATE_LParan_RParan,
  /* 1588. <tab_privilege_GRANT> ::= SELECT '(' <Id List> ')' */
  Rule_tab_privilege_GRANT_SELECT_LParan_RParan,
  /* 1589. <tab_privilege_GRANT> ::= REFERENCES '(' <Id List> ')' */
  Rule_tab_privilege_GRANT_REFERENCES_LParan_RParan,
  /* 1590. <tab_privilege_GRANT> ::= UPDATE */
  Rule_tab_privilege_GRANT_UPDATE,
  /* 1591. <tab_privilege_GRANT> ::= SELECT */
  Rule_tab_privilege_GRANT_SELECT,
  /* 1592. <tab_privilege_GRANT> ::= REFERENCES */
  Rule_tab_privilege_GRANT_REFERENCES,
  /* 1593. <tab_privilege_GRANT> ::= ALTER */
  Rule_tab_privilege_GRANT_ALTER,
  /* 1594. <tab_privilege_GRANT> ::= INDEX */
  Rule_tab_privilege_GRANT_INDEX,
  /* 1595. <tab_privilege_GRANT> ::= UNDER */
  Rule_tab_privilege_GRANT_UNDER,
  /* 1596. <type_privileges> ::= USAGE ON TYPE Id */
  Rule_type_privileges_USAGE_ON_TYPE_Id,
  /* 1597. <type_privileges> ::= UNDER ON TYPE Id */
  Rule_type_privileges_UNDER_ON_TYPE_Id,
  /* 1598. <routine_privileges> ::= EXECUTE ON Id */
  Rule_routine_privileges_EXECUTE_ON_Id,
  /* 1599. <routine_privileges> ::= EXECUTE ON FUNCTION Id '(' <RoutineParams> ')' */
  Rule_routine_privileges_EXECUTE_ON_FUNCTION_Id_LParan_RParan,
  /* 1600. <routine_privileges> ::= EXECUTE ON FUNCTION Id '(' ')' */
  Rule_routine_privileges_EXECUTE_ON_FUNCTION_Id_LParan_RParan2,
  /* 1601. <routine_privileges> ::= EXECUTE ON PROCEDURE Id '(' <RoutineParams> ')' */
  Rule_routine_privileges_EXECUTE_ON_PROCEDURE_Id_LParan_RParan,
  /* 1602. <routine_privileges> ::= EXECUTE ON PROCEDURE Id '(' ')' */
  Rule_routine_privileges_EXECUTE_ON_PROCEDURE_Id_LParan_RParan2,
  /* 1603. <routine_privileges> ::= EXECUTE ON ROUTINE Id '(' <RoutineParams> ')' */
  Rule_routine_privileges_EXECUTE_ON_ROUTINE_Id_LParan_RParan,
  /* 1604. <routine_privileges> ::= EXECUTE ON ROUTINE Id '(' ')' */
  Rule_routine_privileges_EXECUTE_ON_ROUTINE_Id_LParan_RParan2,
  /* 1605. <routine_privileges> ::= EXECUTE ON SPECIFIC FUNCTION Id */
  Rule_routine_privileges_EXECUTE_ON_SPECIFIC_FUNCTION_Id,
  /* 1606. <routine_privileges> ::= EXECUTE ON SPECIFIC PROCEDURE Id */
  Rule_routine_privileges_EXECUTE_ON_SPECIFIC_PROCEDURE_Id,
  /* 1607. <routine_privileges> ::= EXECUTE ON SPECIFIC ROUTINE Id */
  Rule_routine_privileges_EXECUTE_ON_SPECIFIC_ROUTINE_Id,
  /* 1608. <lang_privileges> ::= USAGE ON LANGUAGE SPL */
  Rule_lang_privileges_USAGE_ON_LANGUAGE_SPL,
  /* 1609. <seq_privileges> ::= SELECT ',' ALTER ON Id */
  Rule_seq_privileges_SELECT_Comma_ALTER_ON_Id,
  /* 1610. <seq_privileges> ::= SELECT ON Id */
  Rule_seq_privileges_SELECT_ON_Id,
  /* 1611. <seq_privileges> ::= ALTER ON Id */
  Rule_seq_privileges_ALTER_ON_Id,
  /* 1612. <seq_privileges> ::= ALTER ',' SELECT ON Id */
  Rule_seq_privileges_ALTER_Comma_SELECT_ON_Id,
  /* 1613. <GRANT FRAGMENT> ::= GRANT FRAGMENT <fragment_privileges> ON Id '(' <Id List> ')' <GRANT_TO2> */
  Rule_GRANTFRAGMENT_GRANT_FRAGMENT_ON_Id_LParan_RParan,
  /* 1614. <fragment_privileges> ::= ALL */
  Rule_fragment_privileges_ALL,
  /* 1615. <fragment_privileges> ::= <fragment_privilege> <fragment_privilegeS_> */
  Rule_fragment_privileges,
  /* 1616. <fragment_privilegeS_> ::= ',' <fragment_privilege> <fragment_privilegeS_> */
  Rule_fragment_privilegeS__Comma,
  /* 1617. <fragment_privilegeS_> ::=  */
  Rule_fragment_privilegeS_,
  /* 1618. <fragment_privilege> ::= INSERT */
  Rule_fragment_privilege_INSERT,
  /* 1619. <fragment_privilege> ::= DELETE */
  Rule_fragment_privilege_DELETE,
  /* 1620. <fragment_privilege> ::= UPDATE */
  Rule_fragment_privilege_UPDATE,
  /* 1621. <LOCK TABLE> ::= LOCK TABLE Id IN SHARE MODE */
  Rule_LOCKTABLE_LOCK_TABLE_Id_IN_SHARE_MODE,
  /* 1622. <LOCK TABLE> ::= LOCK TABLE Id IN EXCLUSIVE MODE */
  Rule_LOCKTABLE_LOCK_TABLE_Id_IN_EXCLUSIVE_MODE,
  /* 1623. <REVOKE> ::= REVOKE <db_privileges> FROM <Id List> */
  Rule_REVOKE_REVOKE_FROM,
  /* 1624. <REVOKE> ::= REVOKE DEFAULT ROLE FROM PUBLIC */
  Rule_REVOKE_REVOKE_DEFAULT_ROLE_FROM_PUBLIC,
  /* 1625. <REVOKE> ::= REVOKE DEFAULT ROLE FROM <Id List> */
  Rule_REVOKE_REVOKE_DEFAULT_ROLE_FROM,
  /* 1626. <REVOKE> ::= REVOKE DEFAULT ROLE FROM <StringLiteralS> */
  Rule_REVOKE_REVOKE_DEFAULT_ROLE_FROM2,
  /* 1627. <REVOKE> ::= REVOKE Id <REVOKE_FROM1> */
  Rule_REVOKE_REVOKE_Id,
  /* 1628. <REVOKE> ::= REVOKE <SecurityOption_REVOKE> */
  Rule_REVOKE_REVOKE,
  /* 1629. <REVOKE> ::= REVOKE <tab_privileges_REVOKE> <REVOKE_FROM2> */
  Rule_REVOKE_REVOKE2,
  /* 1630. <REVOKE> ::= REVOKE <routine_privileges> <REVOKE_FROM2> */
  Rule_REVOKE_REVOKE3,
  /* 1631. <REVOKE> ::= REVOKE <lang_privileges> <REVOKE_FROM2> */
  Rule_REVOKE_REVOKE4,
  /* 1632. <REVOKE> ::= REVOKE <type_privileges> <REVOKE_FROM2> */
  Rule_REVOKE_REVOKE5,
  /* 1633. <REVOKE> ::= REVOKE <seq_privileges> <REVOKE_FROM2> */
  Rule_REVOKE_REVOKE6,
  /* 1634. <REVOKE_FROM1> ::= FROM PUBLIC AS Id */
  Rule_REVOKE_FROM1_FROM_PUBLIC_AS_Id,
  /* 1635. <REVOKE_FROM1> ::= FROM <Id List> AS Id */
  Rule_REVOKE_FROM1_FROM_AS_Id,
  /* 1636. <REVOKE_FROM1> ::= FROM <StringLiteralS> AS Id */
  Rule_REVOKE_FROM1_FROM_AS_Id2,
  /* 1637. <REVOKE_FROM1> ::= FROM PUBLIC */
  Rule_REVOKE_FROM1_FROM_PUBLIC,
  /* 1638. <REVOKE_FROM1> ::= FROM <Id List> */
  Rule_REVOKE_FROM1_FROM,
  /* 1639. <REVOKE_FROM1> ::= FROM <StringLiteralS> */
  Rule_REVOKE_FROM1_FROM2,
  /* 1640. <REVOKE_FROM2> ::= FROM PUBLIC CASCADE AS Id */
  Rule_REVOKE_FROM2_FROM_PUBLIC_CASCADE_AS_Id,
  /* 1641. <REVOKE_FROM2> ::= FROM PUBLIC RESTRICT AS Id */
  Rule_REVOKE_FROM2_FROM_PUBLIC_RESTRICT_AS_Id,
  /* 1642. <REVOKE_FROM2> ::= FROM <Id List> CASCADE AS Id */
  Rule_REVOKE_FROM2_FROM_CASCADE_AS_Id,
  /* 1643. <REVOKE_FROM2> ::= FROM <StringLiteralS> CASCADE AS Id */
  Rule_REVOKE_FROM2_FROM_CASCADE_AS_Id2,
  /* 1644. <REVOKE_FROM2> ::= FROM <Id List> RESTRICT AS Id */
  Rule_REVOKE_FROM2_FROM_RESTRICT_AS_Id,
  /* 1645. <REVOKE_FROM2> ::= FROM <StringLiteralS> RESTRICT AS Id */
  Rule_REVOKE_FROM2_FROM_RESTRICT_AS_Id2,
  /* 1646. <REVOKE_FROM2> ::= FROM <Id List> AS Id */
  Rule_REVOKE_FROM2_FROM_AS_Id,
  /* 1647. <REVOKE_FROM2> ::= FROM <StringLiteralS> AS Id */
  Rule_REVOKE_FROM2_FROM_AS_Id2,
  /* 1648. <REVOKE_FROM2> ::= FROM PUBLIC CASCADE */
  Rule_REVOKE_FROM2_FROM_PUBLIC_CASCADE,
  /* 1649. <REVOKE_FROM2> ::= FROM PUBLIC RESTRICT */
  Rule_REVOKE_FROM2_FROM_PUBLIC_RESTRICT,
  /* 1650. <REVOKE_FROM2> ::= FROM <Id List> CASCADE */
  Rule_REVOKE_FROM2_FROM_CASCADE,
  /* 1651. <REVOKE_FROM2> ::= FROM <StringLiteralS> CASCADE */
  Rule_REVOKE_FROM2_FROM_CASCADE2,
  /* 1652. <REVOKE_FROM2> ::= FROM <Id List> RESTRICT */
  Rule_REVOKE_FROM2_FROM_RESTRICT,
  /* 1653. <REVOKE_FROM2> ::= FROM <StringLiteralS> RESTRICT */
  Rule_REVOKE_FROM2_FROM_RESTRICT2,
  /* 1654. <REVOKE_FROM2> ::= FROM <Id List> */
  Rule_REVOKE_FROM2_FROM,
  /* 1655. <REVOKE_FROM2> ::= FROM <StringLiteralS> */
  Rule_REVOKE_FROM2_FROM2,
  /* 1656. <SecurityOption_REVOKE> ::= <DBSECADM_clause> FROM <USERs1> */
  Rule_SecurityOption_REVOKE_FROM,
  /* 1657. <SecurityOption_REVOKE> ::= <EXEMPTION_clause> FROM <USERs1> */
  Rule_SecurityOption_REVOKE_FROM2,
  /* 1658. <SecurityOption_REVOKE> ::= <SECURITY LABEL_clause> FROM <USERs1> <SECURITY LABEL_clause_FOR> */
  Rule_SecurityOption_REVOKE_FROM3,
  /* 1659. <SecurityOption_REVOKE> ::= <SETSESSIONAUTH_clause> FROM <USERs1> */
  Rule_SecurityOption_REVOKE_FROM4,
  /* 1660. <SecurityOption_REVOKE> ::= <SETSESSIONAUTH_clause> FROM ROLE <Id List> */
  Rule_SecurityOption_REVOKE_FROM_ROLE,
  /* 1661. <tab_privileges_REVOKE> ::= ALL PRIVILEGES ON Id */
  Rule_tab_privileges_REVOKE_ALL_PRIVILEGES_ON_Id,
  /* 1662. <tab_privileges_REVOKE> ::= ALL ON Id */
  Rule_tab_privileges_REVOKE_ALL_ON_Id,
  /* 1663. <tab_privileges_REVOKE> ::= <tab_privilege_REVOKE> <tab_privilegeS__REVOKE> ON Id */
  Rule_tab_privileges_REVOKE_ON_Id,
  /* 1664. <tab_privilegeS__REVOKE> ::= ',' <tab_privilege_REVOKE> <tab_privilegeS__REVOKE> */
  Rule_tab_privilegeS__REVOKE_Comma,
  /* 1665. <tab_privilegeS__REVOKE> ::=  */
  Rule_tab_privilegeS__REVOKE,
  /* 1666. <tab_privilege_REVOKE> ::= INSERT */
  Rule_tab_privilege_REVOKE_INSERT,
  /* 1667. <tab_privilege_REVOKE> ::= DELETE */
  Rule_tab_privilege_REVOKE_DELETE,
  /* 1668. <tab_privilege_REVOKE> ::= UPDATE */
  Rule_tab_privilege_REVOKE_UPDATE,
  /* 1669. <tab_privilege_REVOKE> ::= SELECT */
  Rule_tab_privilege_REVOKE_SELECT,
  /* 1670. <tab_privilege_REVOKE> ::= REFERENCES */
  Rule_tab_privilege_REVOKE_REFERENCES,
  /* 1671. <tab_privilege_REVOKE> ::= ALTER */
  Rule_tab_privilege_REVOKE_ALTER,
  /* 1672. <tab_privilege_REVOKE> ::= INDEX */
  Rule_tab_privilege_REVOKE_INDEX,
  /* 1673. <tab_privilege_REVOKE> ::= UNDER */
  Rule_tab_privilege_REVOKE_UNDER,
  /* 1674. <REVOKE FRAGMENT> ::= REVOKE FRAGMENT <fragment_privileges> ON Id '(' <Id List> ')' <REVOKE FRAGMENT_FROM2> */
  Rule_REVOKEFRAGMENT_REVOKE_FRAGMENT_ON_Id_LParan_RParan,
  /* 1675. <REVOKE FRAGMENT_FROM2> ::= FROM PUBLIC AS Id */
  Rule_REVOKEFRAGMENT_FROM2_FROM_PUBLIC_AS_Id,
  /* 1676. <REVOKE FRAGMENT_FROM2> ::= FROM <Id List> AS Id */
  Rule_REVOKEFRAGMENT_FROM2_FROM_AS_Id,
  /* 1677. <REVOKE FRAGMENT_FROM2> ::= FROM PUBLIC */
  Rule_REVOKEFRAGMENT_FROM2_FROM_PUBLIC,
  /* 1678. <REVOKE FRAGMENT_FROM2> ::= FROM <Id List> */
  Rule_REVOKEFRAGMENT_FROM2_FROM,
  /* 1679. <SET ISOLATION> ::= SET ISOLATION TO REPEATABLE READ */
  Rule_SETISOLATION_SET_ISOLATION_TO_REPEATABLE_READ,
  /* 1680. <SET ISOLATION> ::= SET ISOLATION TO COMMITTED READ LAST COMMITTED <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_TO_COMMITTED_READ_LAST_COMMITTED,
  /* 1681. <SET ISOLATION> ::= SET ISOLATION TO COMMITTED READ <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_TO_COMMITTED_READ,
  /* 1682. <SET ISOLATION> ::= SET ISOLATION TO CURSOR STABILITY <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_TO_CURSOR_STABILITY,
  /* 1683. <SET ISOLATION> ::= SET ISOLATION TO DIRTY READ WITH WARNING <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_TO_DIRTY_READ_WITH_WARNING,
  /* 1684. <SET ISOLATION> ::= SET ISOLATION TO DIRTY READ <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_TO_DIRTY_READ,
  /* 1685. <SET ISOLATION> ::= SET ISOLATION REPEATABLE READ */
  Rule_SETISOLATION_SET_ISOLATION_REPEATABLE_READ,
  /* 1686. <SET ISOLATION> ::= SET ISOLATION COMMITTED READ LAST COMMITTED <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_COMMITTED_READ_LAST_COMMITTED,
  /* 1687. <SET ISOLATION> ::= SET ISOLATION COMMITTED READ <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_COMMITTED_READ,
  /* 1688. <SET ISOLATION> ::= SET ISOLATION CURSOR STABILITY <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_CURSOR_STABILITY,
  /* 1689. <SET ISOLATION> ::= SET ISOLATION DIRTY READ WITH WARNING <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_DIRTY_READ_WITH_WARNING,
  /* 1690. <SET ISOLATION> ::= SET ISOLATION DIRTY READ <SET ISOLATION_Opt1> */
  Rule_SETISOLATION_SET_ISOLATION_DIRTY_READ,
  /* 1691. <SET ISOLATION_Opt1> ::= RETAIN UPDATE LOCKS */
  Rule_SETISOLATION_Opt1_RETAIN_UPDATE_LOCKS,
  /* 1692. <SET ISOLATION_Opt1> ::=  */
  Rule_SETISOLATION_Opt1,
  /* 1693. <SET LOCK MODE> ::= SET LOCK MODE TO NOWAIT */
  Rule_SETLOCKMODE_SET_LOCK_MODE_TO_NOWAIT,
  /* 1694. <SET LOCK MODE> ::= SET LOCK MODE TO WAIT IntegerLiteral */
  Rule_SETLOCKMODE_SET_LOCK_MODE_TO_WAIT_IntegerLiteral,
  /* 1695. <SET LOCK MODE> ::= SET LOCK MODE TO WAIT */
  Rule_SETLOCKMODE_SET_LOCK_MODE_TO_WAIT,
  /* 1696. <SET ROLE> ::= SET ROLE Id */
  Rule_SETROLE_SET_ROLE_Id,
  /* 1697. <SET ROLE> ::= SET ROLE StringLiteral */
  Rule_SETROLE_SET_ROLE_StringLiteral,
  /* 1698. <SET ROLE> ::= SET ROLE NULL */
  Rule_SETROLE_SET_ROLE_NULL,
  /* 1699. <SET ROLE> ::= SET ROLE NONE */
  Rule_SETROLE_SET_ROLE_NONE,
  /* 1700. <SET ROLE> ::= SET ROLE DEFAULT */
  Rule_SETROLE_SET_ROLE_DEFAULT,
  /* 1701. <SET SESSION AUTHORIZATION> ::= SET SESSION AUTHORIZATION TO Id */
  Rule_SETSESSIONAUTHORIZATION_SET_SESSION_AUTHORIZATION_TO_Id,
  /* 1702. <SET TRANSACTION> ::= SET TRANSACTION <SET TRANSACTION_optionS> */
  Rule_SETTRANSACTION_SET_TRANSACTION,
  /* 1703. <SET TRANSACTION_optionS> ::= <SET TRANSACTION_option> <SET TRANSACTION_optionS_> */
  Rule_SETTRANSACTION_optionS,
  /* 1704. <SET TRANSACTION_optionS_> ::= ',' <SET TRANSACTION_option> <SET TRANSACTION_optionS_> */
  Rule_SETTRANSACTION_optionS__Comma,
  /* 1705. <SET TRANSACTION_optionS_> ::=  */
  Rule_SETTRANSACTION_optionS_,
  /* 1706. <SET TRANSACTION_option> ::= READ WRITE */
  Rule_SETTRANSACTION_option_READ_WRITE,
  /* 1707. <SET TRANSACTION_option> ::= READ ONLY */
  Rule_SETTRANSACTION_option_READ_ONLY,
  /* 1708. <SET TRANSACTION_option> ::= ISOLATION LEVEL READ COMMITTED */
  Rule_SETTRANSACTION_option_ISOLATION_LEVEL_READ_COMMITTED,
  /* 1709. <SET TRANSACTION_option> ::= ISOLATION LEVEL REPEATABLE READ */
  Rule_SETTRANSACTION_option_ISOLATION_LEVEL_REPEATABLE_READ,
  /* 1710. <SET TRANSACTION_option> ::= ISOLATION LEVEL SERIALIZABLE */
  Rule_SETTRANSACTION_option_ISOLATION_LEVEL_SERIALIZABLE,
  /* 1711. <SET TRANSACTION_option> ::= ISOLATION LEVEL READ UNCOMMITTED */
  Rule_SETTRANSACTION_option_ISOLATION_LEVEL_READ_UNCOMMITTED,
  /* 1712. <UNLOCK TABLE> ::= UNLOCK TABLE Id */
  Rule_UNLOCKTABLE_UNLOCK_TABLE_Id,
  /* 1713. <BEGIN WORK> ::= BEGIN WORK WITHOUT REPLICATION */
  Rule_BEGINWORK_BEGIN_WORK_WITHOUT_REPLICATION,
  /* 1714. <BEGIN WORK> ::= BEGIN WORK */
  Rule_BEGINWORK_BEGIN_WORK,
  /* 1715. <BEGIN WORK> ::= BEGIN WITHOUT REPLICATION */
  Rule_BEGINWORK_BEGIN_WITHOUT_REPLICATION,
  /* 1716. <BEGIN WORK> ::= BEGIN */
  Rule_BEGINWORK_BEGIN,
  /* 1717. <COMMIT WORK> ::= COMMIT WORK */
  Rule_COMMITWORK_COMMIT_WORK,
  /* 1718. <COMMIT WORK> ::= COMMIT */
  Rule_COMMITWORK_COMMIT,
  /* 1719. <ROLLBACK WORK> ::= ROLLBACK WORK */
  Rule_ROLLBACKWORK_ROLLBACK_WORK,
  /* 1720. <ROLLBACK WORK> ::= ROLLBACK */
  Rule_ROLLBACKWORK_ROLLBACK,
  /* 1721. <SET CONSTRAINTS> ::= SET CONSTRAINTS ALL IMMEDIATE */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS_ALL_IMMEDIATE,
  /* 1722. <SET CONSTRAINTS> ::= SET CONSTRAINTS ALL DEFERRED */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS_ALL_DEFERRED,
  /* 1723. <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> IMMEDIATE */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS_IMMEDIATE,
  /* 1724. <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> DEFERRED */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS_DEFERRED,
  /* 1725. <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> <IndexMode> */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS,
  /* 1726. <SET CONSTRAINTS> ::= SET CONSTRAINTS <Id List> DISABLED WITH ERROR */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS_DISABLED_WITH_ERROR,
  /* 1727. <SET CONSTRAINTS> ::= SET CONSTRAINTS FOR Id <IndexMode> */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS_FOR_Id,
  /* 1728. <SET CONSTRAINTS> ::= SET CONSTRAINTS FOR Id DISABLED WITH ERROR */
  Rule_SETCONSTRAINTS_SET_CONSTRAINTS_FOR_Id_DISABLED_WITH_ERROR,
  /* 1729. <SET INDEXES> ::= SET INDEXES <Id List> <IndexMode> */
  Rule_SETINDEXES_SET_INDEXES,
  /* 1730. <SET INDEXES> ::= SET INDEXES FOR Id <IndexMode> */
  Rule_SETINDEXES_SET_INDEXES_FOR_Id,
  /* 1731. <SET TRIGGERS> ::= SET TRIGGERS <Id List> ENABLED */
  Rule_SETTRIGGERS_SET_TRIGGERS_ENABLED,
  /* 1732. <SET TRIGGERS> ::= SET TRIGGERS <Id List> DISABLED */
  Rule_SETTRIGGERS_SET_TRIGGERS_DISABLED,
  /* 1733. <SET TRIGGERS> ::= SET TRIGGERS FOR Id ENABLED */
  Rule_SETTRIGGERS_SET_TRIGGERS_FOR_Id_ENABLED,
  /* 1734. <SET TRIGGERS> ::= SET TRIGGERS FOR Id DISABLED */
  Rule_SETTRIGGERS_SET_TRIGGERS_FOR_Id_DISABLED,
  /* 1735. <SET LOG> ::= SET LOG */
  Rule_SETLOG_SET_LOG,
  /* 1736. <SET LOG> ::= SET BUFFERED LOG */
  Rule_SETLOG_SET_BUFFERED_LOG,
  /* 1737. <SET PLOAD FILE> ::= SET PLOAD FILE TO StringLiteral WITH APPEND */
  Rule_SETPLOADFILE_SET_PLOAD_FILE_TO_StringLiteral_WITH_APPEND,
  /* 1738. <SET PLOAD FILE> ::= SET PLOAD FILE TO StringLiteral */
  Rule_SETPLOADFILE_SET_PLOAD_FILE_TO_StringLiteral,
  /* 1739. <START VIOLATIONS TABLE> ::= START VIOLATIONS TABLE FOR Id <START VIOLATIONS TABLE_opt1> */
  Rule_STARTVIOLATIONSTABLE_START_VIOLATIONS_TABLE_FOR_Id,
  /* 1740. <START VIOLATIONS TABLE> ::= START VIOLATIONS TABLE FOR Id USING Id <START VIOLATIONS TABLE_opt1> */
  Rule_STARTVIOLATIONSTABLE_START_VIOLATIONS_TABLE_FOR_Id_USING_Id,
  /* 1741. <START VIOLATIONS TABLE> ::= START VIOLATIONS TABLE FOR Id USING Id ',' Id <START VIOLATIONS TABLE_opt1> */
  Rule_STARTVIOLATIONSTABLE_START_VIOLATIONS_TABLE_FOR_Id_USING_Id_Comma_Id,
  /* 1742. <START VIOLATIONS TABLE_opt1> ::= MAX ROWS IntegerLiteral */
  Rule_STARTVIOLATIONSTABLE_opt1_MAX_ROWS_IntegerLiteral,
  /* 1743. <START VIOLATIONS TABLE_opt1> ::= MAX VIOLATIONS IntegerLiteral */
  Rule_STARTVIOLATIONSTABLE_opt1_MAX_VIOLATIONS_IntegerLiteral,
  /* 1744. <STOP VIOLATIONS TABLE> ::= STOP VIOLATIONS TABLE FOR Id */
  Rule_STOPVIOLATIONSTABLE_STOP_VIOLATIONS_TABLE_FOR_Id,
  /* 1745. <SAVE EXTERNAL DIRECTIVES> ::= SAVE EXTERNAL DIRECTIVES <Id List> ACTIVE FOR <SELECT> */
  Rule_SAVEEXTERNALDIRECTIVES_SAVE_EXTERNAL_DIRECTIVES_ACTIVE_FOR,
  /* 1746. <SAVE EXTERNAL DIRECTIVES> ::= SAVE EXTERNAL DIRECTIVES <Id List> INACTIVE FOR <SELECT> */
  Rule_SAVEEXTERNALDIRECTIVES_SAVE_EXTERNAL_DIRECTIVES_INACTIVE_FOR,
  /* 1747. <SAVE EXTERNAL DIRECTIVES> ::= SAVE EXTERNAL DIRECTIVES <Id List> TEST ONLY FOR <SELECT> */
  Rule_SAVEEXTERNALDIRECTIVES_SAVE_EXTERNAL_DIRECTIVES_TEST_ONLY_FOR,
  /* 1748. <SAVEPOINT> ::= SAVEPOINT Id UNIQUE */
  Rule_SAVEPOINT_SAVEPOINT_Id_UNIQUE,
  /* 1749. <SAVEPOINT> ::= SAVEPOINT Id */
  Rule_SAVEPOINT_SAVEPOINT_Id,
  /* 1750. <SET ALL_MUTABLES> ::= SET 'ALL_MUTABLES' TO MUTABLE */
  Rule_SETALL_MUTABLES_SET_ALL_MUTABLES_TO_MUTABLE,
  /* 1751. <SET ALL_MUTABLES> ::= SET 'ALL_MUTABLES' TO IMMUTABLE */
  Rule_SETALL_MUTABLES_SET_ALL_MUTABLES_TO_IMMUTABLE,
  /* 1752. <SET Default Table Space> ::= SET 'TABLE_SPACE' TO <Id List> */
  Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO,
  /* 1753. <SET Default Table Space> ::= SET 'TABLE_SPACE' TO DEFAULT */
  Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO_DEFAULT,
  /* 1754. <SET Default Table Space> ::= SET 'TABLE_SPACE' TO IMMUTABLE */
  Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO_IMMUTABLE,
  /* 1755. <SET Default Table Space> ::= SET 'TABLE_SPACE' TO MUTABLE */
  Rule_SETDefaultTableSpace_SET_TABLE_SPACE_TO_MUTABLE,
  /* 1756. <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO <Id List> */
  Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO,
  /* 1757. <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO DEFAULT */
  Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO_DEFAULT,
  /* 1758. <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO IMMUTABLE */
  Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO_IMMUTABLE,
  /* 1759. <SET Default Table Space> ::= SET TEMP 'TABLE_SPACE' TO MUTABLE */
  Rule_SETDefaultTableSpace_SET_TEMP_TABLE_SPACE_TO_MUTABLE,
  /* 1760. <SET Default Table Type> ::= SET 'TABLE_TYPE' TO STANDARD */
  Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_STANDARD,
  /* 1761. <SET Default Table Type> ::= SET 'TABLE_TYPE' TO OPERATIONAL */
  Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_OPERATIONAL,
  /* 1762. <SET Default Table Type> ::= SET 'TABLE_TYPE' TO RAW */
  Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_RAW,
  /* 1763. <SET Default Table Type> ::= SET 'TABLE_TYPE' TO STATIC */
  Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_STATIC,
  /* 1764. <SET Default Table Type> ::= SET 'TABLE_TYPE' TO IMMUTABLE */
  Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_IMMUTABLE,
  /* 1765. <SET Default Table Type> ::= SET 'TABLE_TYPE' TO MUTABLE */
  Rule_SETDefaultTableType_SET_TABLE_TYPE_TO_MUTABLE,
  /* 1766. <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO SCRATCH */
  Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_SCRATCH,
  /* 1767. <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO DEFAULT */
  Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_DEFAULT,
  /* 1768. <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO IMMUTABLE */
  Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_IMMUTABLE,
  /* 1769. <SET Default Table Type> ::= SET TEMP 'TABLE_TYPE' TO MUTABLE */
  Rule_SETDefaultTableType_SET_TEMP_TABLE_TYPE_TO_MUTABLE,
  /* 1770. <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env1> <SET ENVIRONMENT_val1> */
  Rule_SETENVIRONMENT_SET_ENVIRONMENT,
  /* 1771. <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env2> <SET ENVIRONMENT_val2> */
  Rule_SETENVIRONMENT_SET_ENVIRONMENT2,
  /* 1772. <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env3val3> */
  Rule_SETENVIRONMENT_SET_ENVIRONMENT3,
  /* 1773. <SET ENVIRONMENT> ::= SET ENVIRONMENT <SET ENVIRONMENT_env4val4> */
  Rule_SETENVIRONMENT_SET_ENVIRONMENT4,
  /* 1774. <SET ENVIRONMENT_env1> ::= 'BOUND_IMPL_PDQ' */
  Rule_SETENVIRONMENT_env1_BOUND_IMPL_PDQ,
  /* 1775. <SET ENVIRONMENT_env1> ::= 'COMPUTE_QUOTA' */
  Rule_SETENVIRONMENT_env1_COMPUTE_QUOTA,
  /* 1776. <SET ENVIRONMENT_env1> ::= 'CLIENT_TZ' */
  Rule_SETENVIRONMENT_env1_CLIENT_TZ,
  /* 1777. <SET ENVIRONMENT_env1> ::= 'IMPLICIT_PDQ' */
  Rule_SETENVIRONMENT_env1_IMPLICIT_PDQ,
  /* 1778. <SET ENVIRONMENT_env1> ::= MAXSCAN */
  Rule_SETENVIRONMENT_env1_MAXSCAN,
  /* 1779. <SET ENVIRONMENT_env1> ::= 'TMPSPACE_LIMIT' */
  Rule_SETENVIRONMENT_env1_TMPSPACE_LIMIT,
  /* 1780. <SET ENVIRONMENT_val1> ::= ON */
  Rule_SETENVIRONMENT_val1_ON,
  /* 1781. <SET ENVIRONMENT_val1> ::= OFF */
  Rule_SETENVIRONMENT_val1_OFF,
  /* 1782. <SET ENVIRONMENT_val1> ::= StringLiteral */
  Rule_SETENVIRONMENT_val1_StringLiteral,
  /* 1783. <SET ENVIRONMENT_val1> ::= DEFAULT */
  Rule_SETENVIRONMENT_val1_DEFAULT,
  /* 1784. <SET ENVIRONMENT_val1> ::= MUTABLE */
  Rule_SETENVIRONMENT_val1_MUTABLE,
  /* 1785. <SET ENVIRONMENT_val1> ::= IMMUTABLE */
  Rule_SETENVIRONMENT_val1_IMMUTABLE,
  /* 1786. <SET ENVIRONMENT_env2> ::= 'TEMP_TAB_EXT_SIZE' */
  Rule_SETENVIRONMENT_env2_TEMP_TAB_EXT_SIZE,
  /* 1787. <SET ENVIRONMENT_env2> ::= 'TEMP_TAB_NEXT_SIZE' */
  Rule_SETENVIRONMENT_env2_TEMP_TAB_NEXT_SIZE,
  /* 1788. <SET ENVIRONMENT_env2> ::= OPTCOMPIND */
  Rule_SETENVIRONMENT_env2_OPTCOMPIND,
  /* 1789. <SET ENVIRONMENT_val2> ::= DEFAULT */
  Rule_SETENVIRONMENT_val2_DEFAULT,
  /* 1790. <SET ENVIRONMENT_val2> ::= IntegerLiteral */
  Rule_SETENVIRONMENT_val2_IntegerLiteral,
  /* 1791. <SET ENVIRONMENT_val2> ::= MUTABLE */
  Rule_SETENVIRONMENT_val2_MUTABLE,
  /* 1792. <SET ENVIRONMENT_val2> ::= IMMUTABLE */
  Rule_SETENVIRONMENT_val2_IMMUTABLE,
  /* 1793. <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' OFF */
  Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_OFF,
  /* 1794. <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' '0' */
  Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_0,
  /* 1795. <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' ON */
  Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_ON,
  /* 1796. <SET ENVIRONMENT_env3val3> ::= 'IFX_AUTO_REPREPARE' '-1' */
  Rule_SETENVIRONMENT_env3val3_IFX_AUTO_REPREPARE_Minus1,
  /* 1797. <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED ALL */
  Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_ALL,
  /* 1798. <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED COMMITTED READ */
  Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_COMMITTED_READ,
  /* 1799. <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED DIRTY READ */
  Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_DIRTY_READ,
  /* 1800. <SET ENVIRONMENT_env4val4> ::= USELASTCOMMITTED NONE */
  Rule_SETENVIRONMENT_env4val4_USELASTCOMMITTED_NONE,
  /* 1801. <SET EXPLAIN> ::= SET EXPLAIN OFF */
  Rule_SETEXPLAIN_SET_EXPLAIN_OFF,
  /* 1802. <SET EXPLAIN> ::= SET EXPLAIN ON 'AVOID_EXECUTE' */
  Rule_SETEXPLAIN_SET_EXPLAIN_ON_AVOID_EXECUTE,
  /* 1803. <SET EXPLAIN> ::= SET EXPLAIN ON */
  Rule_SETEXPLAIN_SET_EXPLAIN_ON,
  /* 1804. <SET EXPLAIN> ::= SET EXPLAIN ON FILE TO <Expression> WITH APPEND */
  Rule_SETEXPLAIN_SET_EXPLAIN_ON_FILE_TO_WITH_APPEND,
  /* 1805. <SET EXPLAIN> ::= SET EXPLAIN ON FILE TO <Expression> */
  Rule_SETEXPLAIN_SET_EXPLAIN_ON_FILE_TO,
  /* 1806. <SET OPTIMIZATION> ::= SET OPTIMIZATION HIGH */
  Rule_SETOPTIMIZATION_SET_OPTIMIZATION_HIGH,
  /* 1807. <SET OPTIMIZATION> ::= SET OPTIMIZATION LOW */
  Rule_SETOPTIMIZATION_SET_OPTIMIZATION_LOW,
  /* 1808. <SET OPTIMIZATION> ::= SET OPTIMIZATION 'FIRST_ROWS' */
  Rule_SETOPTIMIZATION_SET_OPTIMIZATION_FIRST_ROWS,
  /* 1809. <SET OPTIMIZATION> ::= SET OPTIMIZATION 'ALL_ROWS' */
  Rule_SETOPTIMIZATION_SET_OPTIMIZATION_ALL_ROWS,
  /* 1810. <SET PDQPRIORITY> ::= SET PDQPRIORITY DEFAULT */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_DEFAULT,
  /* 1811. <SET PDQPRIORITY> ::= SET PDQPRIORITY LOW */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_LOW,
  /* 1812. <SET PDQPRIORITY> ::= SET PDQPRIORITY OFF */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_OFF,
  /* 1813. <SET PDQPRIORITY> ::= SET PDQPRIORITY HIGH */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_HIGH,
  /* 1814. <SET PDQPRIORITY> ::= SET PDQPRIORITY Id */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_Id,
  /* 1815. <SET PDQPRIORITY> ::= SET PDQPRIORITY LOW IntegerLiteral HIGH IntegerLiteral */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_LOW_IntegerLiteral_HIGH_IntegerLiteral,
  /* 1816. <SET PDQPRIORITY> ::= SET PDQPRIORITY MUTABLE */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_MUTABLE,
  /* 1817. <SET PDQPRIORITY> ::= SET PDQPRIORITY IMMUTABLE */
  Rule_SETPDQPRIORITY_SET_PDQPRIORITY_IMMUTABLE,
  /* 1818. <SET Residency> ::= SET TABLE Id 'MEMORY_RESIDENT' */
  Rule_SETResidency_SET_TABLE_Id_MEMORY_RESIDENT,
  /* 1819. <SET Residency> ::= SET TABLE Id 'NON_RESIDENT' */
  Rule_SETResidency_SET_TABLE_Id_NON_RESIDENT,
  /* 1820. <SET Residency> ::= SET TABLE Id '(' <Id List> ')' 'MEMORY_RESIDENT' */
  Rule_SETResidency_SET_TABLE_Id_LParan_RParan_MEMORY_RESIDENT,
  /* 1821. <SET Residency> ::= SET TABLE Id '(' <Id List> ')' 'NON_RESIDENT' */
  Rule_SETResidency_SET_TABLE_Id_LParan_RParan_NON_RESIDENT,
  /* 1822. <SET Residency> ::= SET INDEX Id 'MEMORY_RESIDENT' */
  Rule_SETResidency_SET_INDEX_Id_MEMORY_RESIDENT,
  /* 1823. <SET Residency> ::= SET INDEX Id 'NON_RESIDENT' */
  Rule_SETResidency_SET_INDEX_Id_NON_RESIDENT,
  /* 1824. <SET Residency> ::= SET INDEX Id '(' <Id List> ')' 'MEMORY_RESIDENT' */
  Rule_SETResidency_SET_INDEX_Id_LParan_RParan_MEMORY_RESIDENT,
  /* 1825. <SET Residency> ::= SET INDEX Id '(' <Id List> ')' 'NON_RESIDENT' */
  Rule_SETResidency_SET_INDEX_Id_LParan_RParan_NON_RESIDENT,
  /* 1826. <SET SCHEDULE LEVEL> ::= SET SCHEDULE LEVEL IntegerLiteral */
  Rule_SETSCHEDULELEVEL_SET_SCHEDULE_LEVEL_IntegerLiteral,
  /* 1827. <SET STATEMENT CACHE> ::= SET STATEMENT CACHE ON */
  Rule_SETSTATEMENTCACHE_SET_STATEMENT_CACHE_ON,
  /* 1828. <SET STATEMENT CACHE> ::= SET STATEMENT CACHE OFF */
  Rule_SETSTATEMENTCACHE_SET_STATEMENT_CACHE_OFF,
  /* 1829. <UPDATE STATISTICS> ::= UPDATE STATISTICS LOW <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt1> */
  Rule_UPDATESTATISTICS_UPDATE_STATISTICS_LOW,
  /* 1830. <UPDATE STATISTICS> ::= UPDATE STATISTICS <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt1> */
  Rule_UPDATESTATISTICS_UPDATE_STATISTICS,
  /* 1831. <UPDATE STATISTICS> ::= UPDATE STATISTICS MEDIUM <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt2> */
  Rule_UPDATESTATISTICS_UPDATE_STATISTICS_MEDIUM,
  /* 1832. <UPDATE STATISTICS> ::= UPDATE STATISTICS HIGH <UPDATE STATISTICS_table_columns> <UPDATE STATISTICS_opt2> */
  Rule_UPDATESTATISTICS_UPDATE_STATISTICS_HIGH,
  /* 1833. <UPDATE STATISTICS> ::= UPDATE STATISTICS <UPDATE STATISTICS_Routine Statistics> */
  Rule_UPDATESTATISTICS_UPDATE_STATISTICS2,
  /* 1834. <UPDATE STATISTICS_table_columns> ::= FOR TABLE Id '(' <Id List> ')' */
  Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_Id_LParan_RParan,
  /* 1835. <UPDATE STATISTICS_table_columns> ::= FOR TABLE Id */
  Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_Id,
  /* 1836. <UPDATE STATISTICS_table_columns> ::= FOR TABLE ONLY '(' Id ')' '(' <Id List> ')' */
  Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_ONLY_LParan_Id_RParan_LParan_RParan,
  /* 1837. <UPDATE STATISTICS_table_columns> ::= FOR TABLE ONLY '(' Id ')' */
  Rule_UPDATESTATISTICS_table_columns_FOR_TABLE_ONLY_LParan_Id_RParan,
  /* 1838. <UPDATE STATISTICS_table_columns> ::= FOR TABLE */
  Rule_UPDATESTATISTICS_table_columns_FOR_TABLE,
  /* 1839. <UPDATE STATISTICS_table_columns> ::=  */
  Rule_UPDATESTATISTICS_table_columns,
  /* 1840. <UPDATE STATISTICS_opt1> ::= DROP DISTRIBUTIONS ONLY */
  Rule_UPDATESTATISTICS_opt1_DROP_DISTRIBUTIONS_ONLY,
  /* 1841. <UPDATE STATISTICS_opt1> ::= DROP DISTRIBUTIONS */
  Rule_UPDATESTATISTICS_opt1_DROP_DISTRIBUTIONS,
  /* 1842. <UPDATE STATISTICS_opt1> ::=  */
  Rule_UPDATESTATISTICS_opt1,
  /* 1843. <UPDATE STATISTICS_opt2> ::= <Resolution_MEDIUM> */
  Rule_UPDATESTATISTICS_opt2,
  /* 1844. <UPDATE STATISTICS_opt2> ::= <Resolution_HIGH> */
  Rule_UPDATESTATISTICS_opt22,
  /* 1845. <Resolution_MEDIUM> ::= SAMPLING SIZE IntegerLiteral RESOLUTION RealLiteral RealLiteral <DISTRIBUTIONS ONLYopt> */
  Rule_Resolution_MEDIUM_SAMPLING_SIZE_IntegerLiteral_RESOLUTION_RealLiteral_RealLiteral,
  /* 1846. <Resolution_MEDIUM> ::= SAMPLING SIZE IntegerLiteral <DISTRIBUTIONS ONLYopt> */
  Rule_Resolution_MEDIUM_SAMPLING_SIZE_IntegerLiteral,
  /* 1847. <Resolution_MEDIUM> ::= RESOLUTION RealLiteral RealLiteral <DISTRIBUTIONS ONLYopt> */
  Rule_Resolution_MEDIUM_RESOLUTION_RealLiteral_RealLiteral,
  /* 1848. <Resolution_MEDIUM> ::= SAMPLING SIZE IntegerLiteral RESOLUTION RealLiteral <DISTRIBUTIONS ONLYopt> */
  Rule_Resolution_MEDIUM_SAMPLING_SIZE_IntegerLiteral_RESOLUTION_RealLiteral,
  /* 1849. <Resolution_HIGH> ::= RESOLUTION RealLiteral <DISTRIBUTIONS ONLYopt> */
  Rule_Resolution_HIGH_RESOLUTION_RealLiteral,
  /* 1850. <Resolution_HIGH> ::= <DISTRIBUTIONS ONLYopt> */
  Rule_Resolution_HIGH,
  /* 1851. <DISTRIBUTIONS ONLYopt> ::= DISTRIBUTIONS ONLY */
  Rule_DISTRIBUTIONSONLYopt_DISTRIBUTIONS_ONLY,
  /* 1852. <DISTRIBUTIONS ONLYopt> ::=  */
  Rule_DISTRIBUTIONSONLYopt,
  /* 1853. <UPDATE STATISTICS_Routine Statistics> ::= FOR PROCEDURE Id '(' <RoutineParams> ')' */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_PROCEDURE_Id_LParan_RParan,
  /* 1854. <UPDATE STATISTICS_Routine Statistics> ::= FOR PROCEDURE Id */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_PROCEDURE_Id,
  /* 1855. <UPDATE STATISTICS_Routine Statistics> ::= FOR PROCEDURE */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_PROCEDURE,
  /* 1856. <UPDATE STATISTICS_Routine Statistics> ::= FOR FUNCTION Id '(' <RoutineParams> ')' */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_FUNCTION_Id_LParan_RParan,
  /* 1857. <UPDATE STATISTICS_Routine Statistics> ::= FOR FUNCTION Id */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_FUNCTION_Id,
  /* 1858. <UPDATE STATISTICS_Routine Statistics> ::= FOR FUNCTION */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_FUNCTION,
  /* 1859. <UPDATE STATISTICS_Routine Statistics> ::= FOR ROUTINE Id '(' <RoutineParams> ')' */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_ROUTINE_Id_LParan_RParan,
  /* 1860. <UPDATE STATISTICS_Routine Statistics> ::= FOR ROUTINE Id */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_ROUTINE_Id,
  /* 1861. <UPDATE STATISTICS_Routine Statistics> ::= FOR ROUTINE */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_ROUTINE,
  /* 1862. <UPDATE STATISTICS_Routine Statistics> ::= FOR SPECIFIC PROCEDURE Id */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_SPECIFIC_PROCEDURE_Id,
  /* 1863. <UPDATE STATISTICS_Routine Statistics> ::= FOR SPECIFIC FUNCTION Id */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_SPECIFIC_FUNCTION_Id,
  /* 1864. <UPDATE STATISTICS_Routine Statistics> ::= FOR SPECIFIC ROUTINE Id */
  Rule_UPDATESTATISTICS_RoutineStatistics_FOR_SPECIFIC_ROUTINE_Id,
  /* 1865. <EXECUTE FUNCTION> ::= EXECUTE FUNCTION Id '(' <RoutineParams2> ')' <EXECUTE FUNCTION_intoS> WITH TRIGGER REFERENCES */
  Rule_EXECUTEFUNCTION_EXECUTE_FUNCTION_Id_LParan_RParan_WITH_TRIGGER_REFERENCES,
  /* 1866. <EXECUTE FUNCTION> ::= EXECUTE FUNCTION Id '(' ')' <EXECUTE FUNCTION_intoS> WITH TRIGGER REFERENCES */
  Rule_EXECUTEFUNCTION_EXECUTE_FUNCTION_Id_LParan_RParan_WITH_TRIGGER_REFERENCES2,
  /* 1867. <EXECUTE FUNCTION> ::= <EXECUTE FUNCTION0> */
  Rule_EXECUTEFUNCTION,
  /* 1868. <EXECUTE FUNCTION0> ::= EXECUTE FUNCTION Id '(' <RoutineParams2> ')' <EXECUTE FUNCTION_intoS> */
  Rule_EXECUTEFUNCTION0_EXECUTE_FUNCTION_Id_LParan_RParan,
  /* 1869. <EXECUTE FUNCTION0> ::= EXECUTE FUNCTION Id '(' ')' <EXECUTE FUNCTION_intoS> */
  Rule_EXECUTEFUNCTION0_EXECUTE_FUNCTION_Id_LParan_RParan2,
  /* 1870. <RoutineParams2> ::= <RoutineParam2> <RoutineParams2_> */
  Rule_RoutineParams2,
  /* 1871. <RoutineParams2_> ::= ',' <RoutineParam2> <RoutineParams2_> */
  Rule_RoutineParams2__Comma,
  /* 1872. <RoutineParams2_> ::=  */
  Rule_RoutineParams2_,
  /* 1873. <RoutineParam2> ::= <Expression> */
  Rule_RoutineParam2,
  /* 1874. <RoutineParam2> ::= <SELECT> */
  Rule_RoutineParam22,
  /* 1875. <RoutineParam2> ::= Id '=' <Expression> */
  Rule_RoutineParam2_Id_Eq,
  /* 1876. <RoutineParam2> ::= Id '=' <SELECT> */
  Rule_RoutineParam2_Id_Eq2,
  /* 1877. <EXECUTE FUNCTION_intoS> ::= INTO <EXECUTE FUNCTION_into> <EXECUTE FUNCTION_intoS_> */
  Rule_EXECUTEFUNCTION_intoS_INTO,
  /* 1878. <EXECUTE FUNCTION_intoS_> ::= ',' <EXECUTE FUNCTION_into> <EXECUTE FUNCTION_intoS_> */
  Rule_EXECUTEFUNCTION_intoS__Comma,
  /* 1879. <EXECUTE FUNCTION_intoS_> ::=  */
  Rule_EXECUTEFUNCTION_intoS_,
  /* 1880. <EXECUTE FUNCTION_into> ::= Id ':' Id */
  Rule_EXECUTEFUNCTION_into_Id_Colon_Id,
  /* 1881. <EXECUTE FUNCTION_into> ::= Id '$' Id */
  Rule_EXECUTEFUNCTION_into_Id_Dollar_Id,
  /* 1882. <EXECUTE FUNCTION_into> ::= Id INDICATOR Id */
  Rule_EXECUTEFUNCTION_into_Id_INDICATOR_Id,
  /* 1883. <EXECUTE FUNCTION_into> ::= Id */
  Rule_EXECUTEFUNCTION_into_Id,
  /* 1884. <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' INTO <Id List> WITH TRIGGER REFERENCES */
  Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO_WITH_TRIGGER_REFERENCES,
  /* 1885. <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' INTO <Id List> */
  Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO,
  /* 1886. <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' ')' INTO <Id List> WITH TRIGGER REFERENCES */
  Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO_WITH_TRIGGER_REFERENCES2,
  /* 1887. <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' ')' INTO <Id List> */
  Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_INTO2,
  /* 1888. <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' WITH TRIGGER REFERENCES */
  Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_WITH_TRIGGER_REFERENCES,
  /* 1889. <EXECUTE PROCEDURE> ::= EXECUTE PROCEDURE Id '(' ')' WITH TRIGGER REFERENCES */
  Rule_EXECUTEPROCEDURE_EXECUTE_PROCEDURE_Id_LParan_RParan_WITH_TRIGGER_REFERENCES2,
  /* 1890. <EXECUTE PROCEDURE> ::= <EXECUTE PROCEDURE0> */
  Rule_EXECUTEPROCEDURE,
  /* 1891. <EXECUTE PROCEDURE0> ::= EXECUTE PROCEDURE Id '(' <RoutineParams2> ')' */
  Rule_EXECUTEPROCEDURE0_EXECUTE_PROCEDURE_Id_LParan_RParan,
  /* 1892. <EXECUTE PROCEDURE0> ::= EXECUTE PROCEDURE Id '(' ')' */
  Rule_EXECUTEPROCEDURE0_EXECUTE_PROCEDURE_Id_LParan_RParan2,
  /* 1893. <SET DEBUG FILE TO> ::= SET DEBUG FILE TO <Expression> WITH APPEND */
  Rule_SETDEBUGFILETO_SET_DEBUG_FILE_TO_WITH_APPEND,
  /* 1894. <SET DEBUG FILE TO> ::= SET DEBUG FILE TO <Expression> */
  Rule_SETDEBUGFILETO_SET_DEBUG_FILE_TO,
  /* 1895. <GET DIAGNOSTICS> ::= GET DIAGNOSTICS <GET DIAGNOSTICS_option1S> */
  Rule_GETDIAGNOSTICS_GET_DIAGNOSTICS,
  /* 1896. <GET DIAGNOSTICS> ::= GET DIAGNOSTICS EXCEPTION IntegerLiteral <GET DIAGNOSTICS_option2S> */
  Rule_GETDIAGNOSTICS_GET_DIAGNOSTICS_EXCEPTION_IntegerLiteral,
  /* 1897. <GET DIAGNOSTICS> ::= GET DIAGNOSTICS EXCEPTION Id <GET DIAGNOSTICS_option2S> */
  Rule_GETDIAGNOSTICS_GET_DIAGNOSTICS_EXCEPTION_Id,
  /* 1898. <GET DIAGNOSTICS_option1S> ::= <GET DIAGNOSTICS_option1> <GET DIAGNOSTICS_option1S_> */
  Rule_GETDIAGNOSTICS_option1S,
  /* 1899. <GET DIAGNOSTICS_option1S_> ::= ',' <GET DIAGNOSTICS_option1> <GET DIAGNOSTICS_option1S_> */
  Rule_GETDIAGNOSTICS_option1S__Comma,
  /* 1900. <GET DIAGNOSTICS_option1S_> ::=  */
  Rule_GETDIAGNOSTICS_option1S_,
  /* 1901. <GET DIAGNOSTICS_option1> ::= Id '=' 'ROW_COUNT' */
  Rule_GETDIAGNOSTICS_option1_Id_Eq_ROW_COUNT,
  /* 1902. <GET DIAGNOSTICS_option1> ::= Id '=' NUMBER */
  Rule_GETDIAGNOSTICS_option1_Id_Eq_NUMBER,
  /* 1903. <GET DIAGNOSTICS_option1> ::= Id '=' MORE */
  Rule_GETDIAGNOSTICS_option1_Id_Eq_MORE,
  /* 1904. <GET DIAGNOSTICS_option2S> ::= <GET DIAGNOSTICS_option2> <GET DIAGNOSTICS_option2S_> */
  Rule_GETDIAGNOSTICS_option2S,
  /* 1905. <GET DIAGNOSTICS_option2S_> ::= ',' <GET DIAGNOSTICS_option2> <GET DIAGNOSTICS_option2S_> */
  Rule_GETDIAGNOSTICS_option2S__Comma,
  /* 1906. <GET DIAGNOSTICS_option2S_> ::=  */
  Rule_GETDIAGNOSTICS_option2S_,
  /* 1907. <GET DIAGNOSTICS_option2> ::= Id '=' 'CLASS_ORIGIN' */
  Rule_GETDIAGNOSTICS_option2_Id_Eq_CLASS_ORIGIN,
  /* 1908. <GET DIAGNOSTICS_option2> ::= Id '=' 'CONNECTION_NAME' */
  Rule_GETDIAGNOSTICS_option2_Id_Eq_CONNECTION_NAME,
  /* 1909. <GET DIAGNOSTICS_option2> ::= Id '=' 'MESSAGE_LENGTH' */
  Rule_GETDIAGNOSTICS_option2_Id_Eq_MESSAGE_LENGTH,
  /* 1910. <GET DIAGNOSTICS_option2> ::= Id '=' 'MESSAGE_TEXT' */
  Rule_GETDIAGNOSTICS_option2_Id_Eq_MESSAGE_TEXT,
  /* 1911. <GET DIAGNOSTICS_option2> ::= Id '=' 'RETURNED_SQLSTATE' */
  Rule_GETDIAGNOSTICS_option2_Id_Eq_RETURNED_SQLSTATE,
  /* 1912. <GET DIAGNOSTICS_option2> ::= Id '=' 'SERVER_NAME' */
  Rule_GETDIAGNOSTICS_option2_Id_Eq_SERVER_NAME,
  /* 1913. <GET DIAGNOSTICS_option2> ::= Id '=' 'SUBCLASS_ORIGIN' */
  Rule_GETDIAGNOSTICS_option2_Id_Eq_SUBCLASS_ORIGIN,
  /* 1914. <OUTPUT> ::= OUTPUT StringLiteral WITHOUT HEADINGS <SELECT> */
  Rule_OUTPUT_OUTPUT_StringLiteral_WITHOUT_HEADINGS,
  /* 1915. <OUTPUT> ::= OUTPUT StringLiteral <SELECT> */
  Rule_OUTPUT_OUTPUT_StringLiteral,
  /* 1916. <OUTPUT> ::= OUTPUT PIPE Id WITHOUT HEADINGS <SELECT> */
  Rule_OUTPUT_OUTPUT_PIPE_Id_WITHOUT_HEADINGS,
  /* 1917. <OUTPUT> ::= OUTPUT PIPE Id <SELECT> */
  Rule_OUTPUT_OUTPUT_PIPE_Id,
  /* 1918. <SET COLLATION> ::= SET COLLATION Id */
  Rule_SETCOLLATION_SET_COLLATION_Id,
  /* 1919. <SET COLLATION> ::= SET NO COLLATION */
  Rule_SETCOLLATION_SET_NO_COLLATION,
  /* 1920. <SET DATASKIP> ::= SET DATASKIP ON <Id List> */
  Rule_SETDATASKIP_SET_DATASKIP_ON,
  /* 1921. <SET DATASKIP> ::= SET DATASKIP ON */
  Rule_SETDATASKIP_SET_DATASKIP_ON2,
  /* 1922. <SET DATASKIP> ::= SET DATASKIP OFF */
  Rule_SETDATASKIP_SET_DATASKIP_OFF,
  /* 1923. <SET DATASKIP> ::= SET DATASKIP DEFAULT */
  Rule_SETDATASKIP_SET_DATASKIP_DEFAULT,
  /* 1924. <SET ENCRYPTION PASSWORD> ::= SET ENCRYPTION PASSWORD StringLiteral WITH HINT StringLiteral */
  Rule_SETENCRYPTIONPASSWORD_SET_ENCRYPTION_PASSWORD_StringLiteral_WITH_HINT_StringLiteral,
  /* 1925. <SET ENCRYPTION PASSWORD> ::= SET ENCRYPTION PASSWORD StringLiteral */
  Rule_SETENCRYPTIONPASSWORD_SET_ENCRYPTION_PASSWORD_StringLiteral,
  /* 1926. <WHENEVER> ::= WHENEVER <WHENEVER_cond> <WHENEVER_action> */
  Rule_WHENEVER_WHENEVER,
  /* 1927. <WHENEVER_cond> ::= SQLERROR */
  Rule_WHENEVER_cond_SQLERROR,
  /* 1928. <WHENEVER_cond> ::= NOT FOUND */
  Rule_WHENEVER_cond_NOT_FOUND,
  /* 1929. <WHENEVER_cond> ::= SQLWARNING */
  Rule_WHENEVER_cond_SQLWARNING,
  /* 1930. <WHENEVER_cond> ::= ERROR */
  Rule_WHENEVER_cond_ERROR,
  /* 1931. <WHENEVER_action> ::= CONTINUE */
  Rule_WHENEVER_action_CONTINUE,
  /* 1932. <WHENEVER_action> ::= GOTO Id */
  Rule_WHENEVER_action_GOTO_Id,
  /* 1933. <WHENEVER_action> ::= CALL Id */
  Rule_WHENEVER_action_CALL_Id,
  /* 1934. <WHENEVER_action> ::= STOP */
  Rule_WHENEVER_action_STOP,
  /* 1935. <CONNECT> ::= CONNECT TO StringLiteral AS StringLiteral USER StringLiteral USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_StringLiteral_AS_StringLiteral_USER_StringLiteral_USING_Id,
  /* 1936. <CONNECT> ::= CONNECT TO StringLiteral AS Id USER StringLiteral USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_StringLiteral_AS_Id_USER_StringLiteral_USING_Id,
  /* 1937. <CONNECT> ::= CONNECT TO Id AS StringLiteral USER StringLiteral USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_Id_AS_StringLiteral_USER_StringLiteral_USING_Id,
  /* 1938. <CONNECT> ::= CONNECT TO Id AS Id USER StringLiteral USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_Id_AS_Id_USER_StringLiteral_USING_Id,
  /* 1939. <CONNECT> ::= CONNECT TO StringLiteral AS StringLiteral USER Id USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_StringLiteral_AS_StringLiteral_USER_Id_USING_Id,
  /* 1940. <CONNECT> ::= CONNECT TO StringLiteral AS Id USER Id USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_StringLiteral_AS_Id_USER_Id_USING_Id,
  /* 1941. <CONNECT> ::= CONNECT TO Id AS StringLiteral USER Id USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_Id_AS_StringLiteral_USER_Id_USING_Id,
  /* 1942. <CONNECT> ::= CONNECT TO Id AS Id USER Id USING Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_Id_AS_Id_USER_Id_USING_Id,
  /* 1943. <CONNECT> ::= CONNECT TO StringLiteral AS StringLiteral <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_StringLiteral_AS_StringLiteral,
  /* 1944. <CONNECT> ::= CONNECT TO StringLiteral AS Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_StringLiteral_AS_Id,
  /* 1945. <CONNECT> ::= CONNECT TO Id AS StringLiteral <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_Id_AS_StringLiteral,
  /* 1946. <CONNECT> ::= CONNECT TO Id AS Id <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_Id_AS_Id,
  /* 1947. <CONNECT> ::= CONNECT TO DEFAULT <CONNECTopt1> */
  Rule_CONNECT_CONNECT_TO_DEFAULT,
  /* 1948. <CONNECTopt1> ::= WITH CONCURRENT TRANSACTION */
  Rule_CONNECTopt1_WITH_CONCURRENT_TRANSACTION,
  /* 1949. <CONNECTopt1> ::=  */
  Rule_CONNECTopt1,
  /* 1950. <DATABASE> ::= DATABASE Id EXCLUSIVE */
  Rule_DATABASE_DATABASE_Id_EXCLUSIVE,
  /* 1951. <DATABASE> ::= DATABASE Id */
  Rule_DATABASE_DATABASE_Id,
  /* 1952. <DISCONNECT> ::= DISCONNECT CURRENT */
  Rule_DISCONNECT_DISCONNECT_CURRENT,
  /* 1953. <DISCONNECT> ::= DISCONNECT ALL */
  Rule_DISCONNECT_DISCONNECT_ALL,
  /* 1954. <DISCONNECT> ::= DISCONNECT DEFAULT */
  Rule_DISCONNECT_DISCONNECT_DEFAULT,
  /* 1955. <DISCONNECT> ::= DISCONNECT StringLiteral */
  Rule_DISCONNECT_DISCONNECT_StringLiteral,
  /* 1956. <DISCONNECT> ::= DISCONNECT Id */
  Rule_DISCONNECT_DISCONNECT_Id,
  /* 1957. <SET CONNECTION> ::= SET CONNECTION StringLiteral DORMANT */
  Rule_SETCONNECTION_SET_CONNECTION_StringLiteral_DORMANT,
  /* 1958. <SET CONNECTION> ::= SET CONNECTION Id DORMANT */
  Rule_SETCONNECTION_SET_CONNECTION_Id_DORMANT,
  /* 1959. <SET CONNECTION> ::= SET CONNECTION DEFAULT DORMANT */
  Rule_SETCONNECTION_SET_CONNECTION_DEFAULT_DORMANT,
  /* 1960. <SET CONNECTION> ::= SET CONNECTION CURRENT DORMANT */
  Rule_SETCONNECTION_SET_CONNECTION_CURRENT_DORMANT,
  /* 1961. <Label> ::= '<<' Id '>>' */
  Rule_Label_LtLt_Id_GtGt,
  /* 1962. <CALL> ::= CALL Id '(' <Expr List> ')' */
  Rule_CALL_CALL_Id_LParan_RParan,
  /* 1963. <CALL> ::= CALL Id '(' ')' */
  Rule_CALL_CALL_Id_LParan_RParan2,
  /* 1964. <CALL> ::= CALL Id '(' <Expr List> ')' RETURNING <Id List> */
  Rule_CALL_CALL_Id_LParan_RParan_RETURNING,
  /* 1965. <CALL> ::= CALL Id '(' ')' RETURNING <Id List> */
  Rule_CALL_CALL_Id_LParan_RParan_RETURNING2,
  /* 1966. <CALL> ::= CALL Id RETURNING <Id List> */
  Rule_CALL_CALL_Id_RETURNING,
  /* 1967. <CALL> ::= CALL Id */
  Rule_CALL_CALL_Id,
  /* 1968. <CASE> ::= CASE <CASE_WHEN_THENs2> ELSE <SQLBlock> END */
  Rule_CASE_CASE_ELSE_END,
  /* 1969. <CASE> ::= CASE <CASE_WHEN_THENs2> END */
  Rule_CASE_CASE_END,
  /* 1970. <CASE> ::= CASE <Expression> <CASE_WHEN_THENs2> ELSE <SQLBlock> END */
  Rule_CASE_CASE_ELSE_END2,
  /* 1971. <CASE> ::= CASE <Expression> <CASE_WHEN_THENs2> END */
  Rule_CASE_CASE_END2,
  /* 1972. <CASE_WHEN_THENs2> ::= WHEN <Expression> THEN <SQLBlock> <CASE_WHEN_THENs2> */
  Rule_CASE_WHEN_THENs2_WHEN_THEN,
  /* 1973. <CASE_WHEN_THENs2> ::= WHEN <Expression> THEN <SQLBlock> */
  Rule_CASE_WHEN_THENs2_WHEN_THEN2,
  /* 1974. <CONTINUE> ::= CONTINUE */
  Rule_CONTINUE_CONTINUE,
  /* 1975. <CONTINUE> ::= CONTINUE FOR */
  Rule_CONTINUE_CONTINUE_FOR,
  /* 1976. <CONTINUE> ::= CONTINUE FOREACH */
  Rule_CONTINUE_CONTINUE_FOREACH,
  /* 1977. <CONTINUE> ::= CONTINUE LOOP */
  Rule_CONTINUE_CONTINUE_LOOP,
  /* 1978. <CONTINUE> ::= CONTINUE WHILE */
  Rule_CONTINUE_CONTINUE_WHILE,
  /* 1979. <DEFINE> ::= DEFINE GLOBAL <Id List> <Type> DEFAULT <Expression> */
  Rule_DEFINE_DEFINE_GLOBAL_DEFAULT,
  /* 1980. <DEFINE> ::= DEFINE GLOBAL <Id List> REFERENCES BYTE DEFAULT NULL */
  Rule_DEFINE_DEFINE_GLOBAL_REFERENCES_BYTE_DEFAULT_NULL,
  /* 1981. <DEFINE> ::= DEFINE GLOBAL <Id List> REFERENCES TEXT DEFAULT NULL */
  Rule_DEFINE_DEFINE_GLOBAL_REFERENCES_TEXT_DEFAULT_NULL,
  /* 1982. <DEFINE> ::= DEFINE <Id List> <Type> */
  Rule_DEFINE_DEFINE,
  /* 1983. <DEFINE> ::= DEFINE <Id List> REFERENCES BYTE */
  Rule_DEFINE_DEFINE_REFERENCES_BYTE,
  /* 1984. <DEFINE> ::= DEFINE <Id List> REFERENCES TEXT */
  Rule_DEFINE_DEFINE_REFERENCES_TEXT,
  /* 1985. <DEFINE> ::= DEFINE <Id List> LIKE Id */
  Rule_DEFINE_DEFINE_LIKE_Id,
  /* 1986. <DEFINE> ::= DEFINE <Id List> PROCEDURE */
  Rule_DEFINE_DEFINE_PROCEDURE,
  /* 1987. <EXIT> ::= EXIT FOREACH */
  Rule_EXIT_EXIT_FOREACH,
  /* 1988. <EXIT> ::= EXIT FOR <EXITopt1> */
  Rule_EXIT_EXIT_FOR,
  /* 1989. <EXIT> ::= EXIT LOOP <EXITopt1> */
  Rule_EXIT_EXIT_LOOP,
  /* 1990. <EXIT> ::= EXIT WHILE <EXITopt1> */
  Rule_EXIT_EXIT_WHILE,
  /* 1991. <EXITopt1> ::= Id WHEN <Expression> */
  Rule_EXITopt1_Id_WHEN,
  /* 1992. <EXITopt1> ::= WHEN <Expression> */
  Rule_EXITopt1_WHEN,
  /* 1993. <EXITopt1> ::=  */
  Rule_EXITopt1,
  /* 1994. <FOR> ::= <Label> FOR Id IN '(' <Expression> TO <Expression> STEP <Expression> ')' <FORbody> */
  Rule_FOR_FOR_Id_IN_LParan_TO_STEP_RParan,
  /* 1995. <FOR> ::= <Label> FOR Id IN '(' <Expression> TO <Expression> ')' <FORbody> */
  Rule_FOR_FOR_Id_IN_LParan_TO_RParan,
  /* 1996. <FOR> ::= <Label> FOR Id IN '(' <Expr List> ')' <FORbody> */
  Rule_FOR_FOR_Id_IN_LParan_RParan,
  /* 1997. <FOR> ::= <Label> FOR Id '=' <Expression> TO <Expression> STEP <Expression> <FORbody> */
  Rule_FOR_FOR_Id_Eq_TO_STEP,
  /* 1998. <FOR> ::= <Label> FOR Id '=' <Expression> TO <Expression> <FORbody> */
  Rule_FOR_FOR_Id_Eq_TO,
  /* 1999. <FOR> ::= FOR Id IN '(' <Expression> TO <Expression> STEP <Expression> ')' <FORbody> */
  Rule_FOR_FOR_Id_IN_LParan_TO_STEP_RParan2,
  /* 2000. <FOR> ::= FOR Id IN '(' <Expression> TO <Expression> ')' <FORbody> */
  Rule_FOR_FOR_Id_IN_LParan_TO_RParan2,
  /* 2001. <FOR> ::= FOR Id IN '(' <Expr List> ')' <FORbody> */
  Rule_FOR_FOR_Id_IN_LParan_RParan2,
  /* 2002. <FOR> ::= FOR Id '=' <Expression> TO <Expression> STEP <Expression> <FORbody> */
  Rule_FOR_FOR_Id_Eq_TO_STEP2,
  /* 2003. <FOR> ::= FOR Id '=' <Expression> TO <Expression> <FORbody> */
  Rule_FOR_FOR_Id_Eq_TO2,
  /* 2004. <FORbody> ::= <SQLBlock> END FOR Id */
  Rule_FORbody_END_FOR_Id,
  /* 2005. <FORbody> ::= <SQLBlock> END FOR */
  Rule_FORbody_END_FOR,
  /* 2006. <FORbody> ::= <LOOPbody> */
  Rule_FORbody,
  /* 2007. <LOOPbody> ::= LOOP <SQLBlock> END LOOP Id */
  Rule_LOOPbody_LOOP_END_LOOP_Id,
  /* 2008. <LOOPbody> ::= LOOP <SQLBlock> END LOOP */
  Rule_LOOPbody_LOOP_END_LOOP,
  /* 2009. <FOREACH> ::= FOREACH <FOREACHopt1> <SELECTinto> <SQLBlock> END FOR EACH */
  Rule_FOREACH_FOREACH_END_FOR_EACH,
  /* 2010. <FOREACHopt1> ::= WITH HOLD */
  Rule_FOREACHopt1_WITH_HOLD,
  /* 2011. <FOREACHopt1> ::= Id WITH HOLD FOR */
  Rule_FOREACHopt1_Id_WITH_HOLD_FOR,
  /* 2012. <FOREACHopt1> ::= Id FOR */
  Rule_FOREACHopt1_Id_FOR,
  /* 2013. <FOREACHopt1> ::= <INSERT_EXECUTE> INTO <Id List> */
  Rule_FOREACHopt1_INTO,
  /* 2014. <FOREACHopt1> ::= <INSERT_EXECUTE> */
  Rule_FOREACHopt1,
  /* 2015. <GOTO> ::= GOTO Id */
  Rule_GOTO_GOTO_Id,
  /* 2016. <IF> ::= IF <Expression> THEN <SQLBlock2> <IF_ELIFs> ELSE <SQLBlock2> END IF */
  Rule_IF_IF_THEN_ELSE_END_IF,
  /* 2017. <IF> ::= IF <Expression> THEN <SQLBlock2> <IF_ELIFs> END IF */
  Rule_IF_IF_THEN_END_IF,
  /* 2018. <IF> ::= IF <Expression> THEN <IF_ELIFs> ELSE <SQLBlock2> END IF */
  Rule_IF_IF_THEN_ELSE_END_IF2,
  /* 2019. <IF> ::= IF <Expression> THEN <IF_ELIFs> END IF */
  Rule_IF_IF_THEN_END_IF2,
  /* 2020. <IF_ELIFs> ::= <IF_ELIF> <IF_ELIFs> */
  Rule_IF_ELIFs,
  /* 2021. <IF_ELIFs> ::=  */
  Rule_IF_ELIFs2,
  /* 2022. <IF_ELIF> ::= ELIF <Expression> THEN <SQLBlock2> */
  Rule_IF_ELIF_ELIF_THEN,
  /* 2023. <IF_ELIF> ::= ELIF <Expression> THEN */
  Rule_IF_ELIF_ELIF_THEN2,
  /* 2024. <SQLBlock2> ::= <SQLBlock> */
  Rule_SQLBlock2,
  /* 2025. <LET> ::= LET <Id List> '=' <Expr List> */
  Rule_LET_LET_Eq,
  /* 2026. <ON EXCEPTION> ::= ON EXCEPTION IN '(' <Expr List> ')' <ON EXCEPTION_SETopt> <SQLBlock> END EXCEPTION <WITH RESUMEopt> */
  Rule_ONEXCEPTION_ON_EXCEPTION_IN_LParan_RParan_END_EXCEPTION,
  /* 2027. <ON EXCEPTION> ::= ON EXCEPTION <ON EXCEPTION_SETopt> <SQLBlock> END EXCEPTION <WITH RESUMEopt> */
  Rule_ONEXCEPTION_ON_EXCEPTION_END_EXCEPTION,
  /* 2028. <ON EXCEPTION_SETopt> ::= SET Id ',' Id */
  Rule_ONEXCEPTION_SETopt_SET_Id_Comma_Id,
  /* 2029. <ON EXCEPTION_SETopt> ::= SET Id */
  Rule_ONEXCEPTION_SETopt_SET_Id,
  /* 2030. <ON EXCEPTION_SETopt> ::=  */
  Rule_ONEXCEPTION_SETopt,
  /* 2031. <WITH RESUMEopt> ::= WITH RESUME */
  Rule_WITHRESUMEopt_WITH_RESUME,
  /* 2032. <WITH RESUMEopt> ::=  */
  Rule_WITHRESUMEopt,
  /* 2033. <RAISE EXCEPTION> ::= RAISE EXCEPTION <Expression> ',' <Expression> ',' <Expression> */
  Rule_RAISEEXCEPTION_RAISE_EXCEPTION_Comma_Comma,
  /* 2034. <RAISE EXCEPTION> ::= RAISE EXCEPTION <Expression> ',' <Expression> */
  Rule_RAISEEXCEPTION_RAISE_EXCEPTION_Comma,
  /* 2035. <RAISE EXCEPTION> ::= RAISE EXCEPTION <Expression> */
  Rule_RAISEEXCEPTION_RAISE_EXCEPTION,
  /* 2036. <RETURN> ::= RETURN <Expr List> <WITH RESUMEopt> */
  Rule_RETURN_RETURN,
  /* 2037. <RETURN> ::= RETURN */
  Rule_RETURN_RETURN2,
  /* 2038. <SYSTEM> ::= SYSTEM <Expression> */
  Rule_SYSTEM_SYSTEM,
  /* 2039. <TRACE> ::= TRACE ON */
  Rule_TRACE_TRACE_ON,
  /* 2040. <TRACE> ::= TRACE OFF */
  Rule_TRACE_TRACE_OFF,
  /* 2041. <TRACE> ::= TRACE PROCEDURE */
  Rule_TRACE_TRACE_PROCEDURE,
  /* 2042. <TRACE> ::= TRACE <Expression> */
  Rule_TRACE_TRACE,
  /* 2043. <WHILE> ::= <Label> WHILE <Expression> <WHILEbody> */
  Rule_WHILE_WHILE,
  /* 2044. <WHILE> ::= WHILE <Expression> <WHILEbody> */
  Rule_WHILE_WHILE2,
  /* 2045. <WHILEbody> ::= <SQLBlock> END WHILE Id */
  Rule_WHILEbody_END_WHILE_Id,
  /* 2046. <WHILEbody> ::= <SQLBlock> END WHILE */
  Rule_WHILEbody_END_WHILE,
  /* 2047. <WHILEbody> ::= <LOOPbody> */
  Rule_WHILEbody,
  /* 2048. <onbar> ::= onbar -b <onbarOpt1> <onbarOpt2> <onbarOpt3> <onbarOpt4> <onbarOpt5> */
  Rule_onbar_onbar_Minusb,
  /* 2049. <onbar> ::= onbar -m IntegerLiteral -r IntegerLiteral */
  Rule_onbar_onbar_Minusm_IntegerLiteral_Minusr_IntegerLiteral,
  /* 2050. <onbar> ::= onbar -m IntegerLiteral */
  Rule_onbar_onbar_Minusm_IntegerLiteral,
  /* 2051. <onbar> ::= onbar -m -r IntegerLiteral */
  Rule_onbar_onbar_Minusm_Minusr_IntegerLiteral,
  /* 2052. <onbar> ::= onbar -m IntegerLiteral -r */
  Rule_onbar_onbar_Minusm_IntegerLiteral_Minusr,
  /* 2053. <onbar> ::= onbar -m -r */
  Rule_onbar_onbar_Minusm_Minusr,
  /* 2054. <onbar> ::= onbar -b -l '-C2' <onbarOpt5a2> */
  Rule_onbar_onbar_Minusb_Minusl_MinusC2,
  /* 2055. <onbar> ::= onbar -b -l -c <onbarOpt5a2> */
  Rule_onbar_onbar_Minusb_Minusl_Minusc,
  /* 2056. <onbar> ::= onbar -b -l -s <onbarOpt5a2> */
  Rule_onbar_onbar_Minusb_Minusl_Minuss,
  /* 2057. <onbar> ::= onbar -b -l <onbarOpt5a2> */
  Rule_onbar_onbar_Minusb_Minusl,
  /* 2058. <onbar> ::= onbar -P -n <onbarOpt6> <onbarOpt7> <onbarOpt8> -b <onbarOpt9> <onbarOpt10> <onbarOpt11> */
  Rule_onbar_onbar_MinusP_Minusn_Minusb,
  /* 2059. <onbar> ::= onbar -P -n <onbarOpt6> <onbarOpt7> <onbarOpt8> <onbarOpt9> <onbarOpt10> <onbarOpt11> */
  Rule_onbar_onbar_MinusP_Minusn,
  /* 2060. <onbar> ::= onbar -v -t StringLiteral <onbarOpt2> <onbarOpt5a1> <onbarOpt5a3> */
  Rule_onbar_onbar_Minusv_Minust_StringLiteral,
  /* 2061. <onbar> ::= onbar -v <onbarOpt2> <onbarOpt5a1> <onbarOpt5a3> */
  Rule_onbar_onbar_Minusv,
  /* 2062. <onbar> ::= onbar -r <onbarOpt12> <onbarOpt13> */
  Rule_onbar_onbar_Minusr,
  /* 2063. <onbar> ::= onbar -RESTART */
  Rule_onbar_onbar_MinusRESTART,
  /* 2064. <onbar> ::= onbar -r -e <onbarOpt14> <onbarOpt2> -O <onbarOpt5a3_> */
  Rule_onbar_onbar_Minusr_Minuse_MinusO,
  /* 2065. <onbar> ::= onbar -r -e <onbarOpt14> <onbarOpt2> <onbarOpt5a3_> */
  Rule_onbar_onbar_Minusr_Minuse,
  /* 2066. <onbar> ::= onbar <onbarOpt15> -q Id */
  Rule_onbar_onbar_Minusq_Id,
  /* 2067. <onbar> ::= onbar <onbarOpt15> Id */
  Rule_onbar_onbar_Id,
  /* 2068. <onbarOpt1> ::= -P */
  Rule_onbarOpt1_MinusP,
  /* 2069. <onbarOpt1> ::=  */
  Rule_onbarOpt1,
  /* 2070. <onbarOpt2> ::= -q Id */
  Rule_onbarOpt2_Minusq_Id,
  /* 2071. <onbarOpt2> ::=  */
  Rule_onbarOpt2,
  /* 2072. <onbarOpt3> ::= -s */
  Rule_onbarOpt3_Minuss,
  /* 2073. <onbarOpt3> ::=  */
  Rule_onbarOpt3,
  /* 2074. <onbarOpt4> ::= -v */
  Rule_onbarOpt4_Minusv,
  /* 2075. <onbarOpt4> ::=  */
  Rule_onbarOpt4,
  /* 2076. <onbarOpt5a1> ::= -l IntegerLiteral */
  Rule_onbarOpt5a1_Minusl_IntegerLiteral,
  /* 2077. <onbarOpt5a1> ::=  */
  Rule_onbarOpt5a1,
  /* 2078. <onbarOpt5a2> ::= -O */
  Rule_onbarOpt5a2_MinusO,
  /* 2079. <onbarOpt5a2> ::=  */
  Rule_onbarOpt5a2,
  /* 2080. <onbarOpt5a3_> ::= -f StringLiteral */
  Rule_onbarOpt5a3__Minusf_StringLiteral,
  /* 2081. <onbarOpt5a3_> ::= <Id List> */
  Rule_onbarOpt5a3_,
  /* 2082. <onbarOpt5a3_> ::=  */
  Rule_onbarOpt5a3_2,
  /* 2083. <onbarOpt5a3> ::= <onbarOpt5a3_> */
  Rule_onbarOpt5a3,
  /* 2084. <onbarOpt5a3> ::= -w */
  Rule_onbarOpt5a3_Minusw,
  /* 2085. <onbarOpt5> ::= <onbarOpt5a1> <onbarOpt5a2> <onbarOpt5a3> */
  Rule_onbarOpt5,
  /* 2086. <onbarOpt5> ::= -f */
  Rule_onbarOpt5_Minusf,
  /* 2087. <onbarOpt6> ::= Id */
  Rule_onbarOpt6_Id,
  /* 2088. <onbarOpt6> ::= Id Id */
  Rule_onbarOpt6_Id_Id,
  /* 2089. <onbarOpt7> ::= -l */
  Rule_onbarOpt7_Minusl,
  /* 2090. <onbarOpt7> ::=  */
  Rule_onbarOpt7,
  /* 2091. <onbarOpt8> ::= -q */
  Rule_onbarOpt8_Minusq,
  /* 2092. <onbarOpt8> ::=  */
  Rule_onbarOpt8,
  /* 2093. <onbarOpt9> ::= -u Id */
  Rule_onbarOpt9_Minusu_Id,
  /* 2094. <onbarOpt9> ::= -u StringLiteral */
  Rule_onbarOpt9_Minusu_StringLiteral,
  /* 2095. <onbarOpt9> ::=  */
  Rule_onbarOpt9,
  /* 2096. <onbarOpt10> ::= -t IntegerLiteral */
  Rule_onbarOpt10_Minust_IntegerLiteral,
  /* 2097. <onbarOpt10> ::=  */
  Rule_onbarOpt10,
  /* 2098. <onbarOpt11> ::= -x IntegerLiteral */
  Rule_onbarOpt11_Minusx_IntegerLiteral,
  /* 2099. <onbarOpt11> ::=  */
  Rule_onbarOpt11,
  /* 2100. <onbarOpt12> ::= <onbarOpt12a1> <onbarOpt12a2> <onbarOpt2> */
  Rule_onbarOpt12,
  /* 2101. <onbarOpt12> ::= <onbarOpt12bS> */
  Rule_onbarOpt122,
  /* 2102. <onbarOpt13> ::= <onbarOpt5a3> */
  Rule_onbarOpt13,
  /* 2103. <onbarOpt13> ::= -t StringLiteral */
  Rule_onbarOpt13_Minust_StringLiteral,
  /* 2104. <onbarOpt13> ::= -n IntegerLiteral */
  Rule_onbarOpt13_Minusn_IntegerLiteral,
  /* 2105. <onbarOpt13> ::= -w -t StringLiteral */
  Rule_onbarOpt13_Minusw_Minust_StringLiteral,
  /* 2106. <onbarOpt13> ::= -w -n IntegerLiteral */
  Rule_onbarOpt13_Minusw_Minusn_IntegerLiteral,
  /* 2107. <onbarOpt12a1> ::= '-e2' */
  Rule_onbarOpt12a1_Minuse2,
  /* 2108. <onbarOpt12a1> ::= -O */
  Rule_onbarOpt12a1_MinusO,
  /* 2109. <onbarOpt12a2> ::= -i */
  Rule_onbarOpt12a2_Minusi,
  /* 2110. <onbarOpt12a2> ::= '-I2' */
  Rule_onbarOpt12a2_MinusI2,
  /* 2111. <onbarOpt12a2> ::=  */
  Rule_onbarOpt12a2,
  /* 2112. <onbarOpt12bS> ::= <onbarOpt12b> <onbarOpt12bS> */
  Rule_onbarOpt12bS,
  /* 2113. <onbarOpt12bS> ::= <onbarOpt12b> */
  Rule_onbarOpt12bS2,
  /* 2114. <onbarOpt12b> ::= -rename -P StringLiteral -O IntegerLiteral -n StringLiteral -O IntegerLiteral */
  Rule_onbarOpt12b_Minusrename_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusn_StringLiteral_MinusO_IntegerLiteral,
  /* 2115. <onbarOpt12b> ::= -rename -f StringLiteral */
  Rule_onbarOpt12b_Minusrename_Minusf_StringLiteral,
  /* 2116. <onbarOpt14> ::= <onbarOpt1> */
  Rule_onbarOpt14,
  /* 2117. <onbarOpt14> ::= -t StringLiteral */
  Rule_onbarOpt14_Minust_StringLiteral,
  /* 2118. <onbarOpt14> ::= -n IntegerLiteral */
  Rule_onbarOpt14_Minusn_IntegerLiteral,
  /* 2119. <onbarOpt15> ::= OFF */
  Rule_onbarOpt15_OFF,
  /* 2120. <onbarOpt15> ::= ON */
  Rule_onbarOpt15_ON,
  /* 2121. <onbarOpt15> ::= -d */
  Rule_onbarOpt15_Minusd,
  /* 2122. <onbarOpt15> ::=  */
  Rule_onbarOpt15,
  /* 2123. <ontape> ::= ontape -n <Id List2> */
  Rule_ontape_ontape_Minusn,
  /* 2124. <ontape> ::= ontape -s <ontapeOption1> <Id List2> */
  Rule_ontape_ontape_Minuss,
  /* 2125. <ontape> ::= ontape <ontapeOption1> <Id List2> */
  Rule_ontape_ontape,
  /* 2126. <ontape> ::= ontape -v -s <onbarOpt5a1> -t STDIO -f */
  Rule_ontape_ontape_Minusv_Minuss_Minust_STDIO_Minusf,
  /* 2127. <ontape> ::= ontape -s <onbarOpt5a1> -t STDIO -f */
  Rule_ontape_ontape_Minuss_Minust_STDIO_Minusf,
  /* 2128. <ontape> ::= ontape -v -s <onbarOpt5a1> -f */
  Rule_ontape_ontape_Minusv_Minuss_Minusf,
  /* 2129. <ontape> ::= ontape -s <onbarOpt5a1> -f */
  Rule_ontape_ontape_Minuss_Minusf,
  /* 2130. <ontape> ::= ontape -v -s <onbarOpt5a1> -t STDIO */
  Rule_ontape_ontape_Minusv_Minuss_Minust_STDIO,
  /* 2131. <ontape> ::= ontape -s <onbarOpt5a1> -t STDIO */
  Rule_ontape_ontape_Minuss_Minust_STDIO,
  /* 2132. <ontape> ::= ontape -v -s <onbarOpt5a1> */
  Rule_ontape_ontape_Minusv_Minuss,
  /* 2133. <ontape> ::= ontape -s <onbarOpt5a1> */
  Rule_ontape_ontape_Minuss2,
  /* 2134. <ontape> ::= ontape -a */
  Rule_ontape_ontape_Minusa,
  /* 2135. <ontape> ::= ontape -c */
  Rule_ontape_ontape_Minusc,
  /* 2136. <ontape> ::= ontape <ontapeOption2> -d <Id List2> <ontapeOpt3> */
  Rule_ontape_ontape_Minusd,
  /* 2137. <ontape> ::= ontape <ontapeOption2> <ontapeOpt3> */
  Rule_ontape_ontape2,
  /* 2138. <ontape> ::= ontape -l */
  Rule_ontape_ontape_Minusl,
  /* 2139. <ontape> ::= ontape '-S2' */
  Rule_ontape_ontape_MinusS22,
  /* 2140. <ontapeOption1> ::= -b */
  Rule_ontapeOption1_Minusb,
  /* 2141. <ontapeOption1> ::= -u */
  Rule_ontapeOption1_Minusu,
  /* 2142. <ontapeOption1> ::= -a */
  Rule_ontapeOption1_Minusa,
  /* 2143. <ontapeOption2> ::= -r <onbarOpt12bS> */
  Rule_ontapeOption2_Minusr,
  /* 2144. <ontapeOption2> ::= -r */
  Rule_ontapeOption2_Minusr2,
  /* 2145. <ontapeOption2> ::= -P -e */
  Rule_ontapeOption2_MinusP_Minuse,
  /* 2146. <ontapeOption2> ::= -P */
  Rule_ontapeOption2_MinusP,
  /* 2147. <ontapeOpt3> ::= -t STDIO -v */
  Rule_ontapeOpt3_Minust_STDIO_Minusv,
  /* 2148. <ontapeOpt3> ::= -t STDIO */
  Rule_ontapeOpt3_Minust_STDIO,
  /* 2149. <ontapeOpt3> ::=  */
  Rule_ontapeOpt3,
  /* 2150. <onmode> ::= onmode <onmodeOption1> -y */
  Rule_onmode_onmode_Minusy,
  /* 2151. <onmode> ::= onmode <onmodeOption1> */
  Rule_onmode_onmode,
  /* 2152. <onmodeOption1> ::= '-BC 1' */
  Rule_onmodeOption1_MinusBC1,
  /* 2153. <onmodeOption1> ::= '-BC 2' */
  Rule_onmodeOption1_MinusBC2,
  /* 2154. <onmodeOption1> ::= -k */
  Rule_onmodeOption1_Minusk,
  /* 2155. <onmodeOption1> ::= -m */
  Rule_onmodeOption1_Minusm,
  /* 2156. <onmodeOption1> ::= -s */
  Rule_onmodeOption1_Minuss,
  /* 2157. <onmodeOption1> ::= -u */
  Rule_onmodeOption1_Minusu,
  /* 2158. <onmodeOption1> ::= -j */
  Rule_onmodeOption1_Minusj,
  /* 2159. <onmodeOption1> ::= -c fuzzy */
  Rule_onmodeOption1_Minusc_fuzzy,
  /* 2160. <onmodeOption1> ::= -c block */
  Rule_onmodeOption1_Minusc_block,
  /* 2161. <onmodeOption1> ::= -c unblock */
  Rule_onmodeOption1_Minusc_unblock,
  /* 2162. <onmodeOption1> ::= -c */
  Rule_onmodeOption1_Minusc,
  /* 2163. <onmodeOption1> ::= -l */
  Rule_onmodeOption1_Minusl,
  /* 2164. <onmodeOption1> ::= -z IntegerLiteral */
  Rule_onmodeOption1_Minusz_IntegerLiteral,
  /* 2165. <onmodeOption1> ::= -a IntegerLiteral */
  Rule_onmodeOption1_Minusa_IntegerLiteral,
  /* 2166. <onmodeOption1> ::= -P '+' IntegerLiteral CPU */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_CPU,
  /* 2167. <onmodeOption1> ::= -P '+' IntegerLiteral ENCRYPT */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_ENCRYPT,
  /* 2168. <onmodeOption1> ::= -P '+' IntegerLiteral JVP */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_JVP,
  /* 2169. <onmodeOption1> ::= -P '+' IntegerLiteral Id */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_Id,
  /* 2170. <onmodeOption1> ::= -P '+' IntegerLiteral AIO */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_AIO,
  /* 2171. <onmodeOption1> ::= -P '+' IntegerLiteral LIO */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_LIO,
  /* 2172. <onmodeOption1> ::= -P '+' IntegerLiteral PIO */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_PIO,
  /* 2173. <onmodeOption1> ::= -P '+' IntegerLiteral SHM */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_SHM,
  /* 2174. <onmodeOption1> ::= -P '+' IntegerLiteral SOC */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_SOC,
  /* 2175. <onmodeOption1> ::= -P '+' IntegerLiteral STR */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_STR,
  /* 2176. <onmodeOption1> ::= -P '+' IntegerLiteral TLI */
  Rule_onmodeOption1_MinusP_Plus_IntegerLiteral_TLI,
  /* 2177. <onmodeOption1> ::= -P '-' IntegerLiteral ENCRYPT */
  Rule_onmodeOption1_MinusP_Minus_IntegerLiteral_ENCRYPT,
  /* 2178. <onmodeOption1> ::= -P '-' IntegerLiteral JVP */
  Rule_onmodeOption1_MinusP_Minus_IntegerLiteral_JVP,
  /* 2179. <onmodeOption1> ::= -P '-' IntegerLiteral Id */
  Rule_onmodeOption1_MinusP_Minus_IntegerLiteral_Id,
  /* 2180. <onmodeOption1> ::= -r */
  Rule_onmodeOption1_Minusr,
  /* 2181. <onmodeOption1> ::= -d IntegerLiteral */
  Rule_onmodeOption1_Minusd_IntegerLiteral,
  /* 2182. <onmodeOption1> ::= -m IntegerLiteral */
  Rule_onmodeOption1_Minusm_IntegerLiteral,
  /* 2183. <onmodeOption1> ::= -q IntegerLiteral */
  Rule_onmodeOption1_Minusq_IntegerLiteral,
  /* 2184. <onmodeOption1> ::= -s IntegerLiteral */
  Rule_onmodeOption1_Minuss_IntegerLiteral,
  /* 2185. <onmodeOption1> ::= -f */
  Rule_onmodeOption1_Minusf,
  /* 2186. <onmodeOption1> ::= -O */
  Rule_onmodeOption1_MinusO,
  /* 2187. <onmodeOption1> ::= -n */
  Rule_onmodeOption1_Minusn,
  /* 2188. <onmodeOption1> ::= '-r2' */
  Rule_onmodeOption1_Minusr2,
  /* 2189. <onmodeOption1> ::= '-Z2' IntegerLiteral */
  Rule_onmodeOption1_MinusZ2_IntegerLiteral,
  /* 2190. <onmodeOption1> ::= -d STANDARD */
  Rule_onmodeOption1_Minusd_STANDARD,
  /* 2191. <onmodeOption1> ::= -d PRIMARY Id */
  Rule_onmodeOption1_Minusd_PRIMARY_Id,
  /* 2192. <onmodeOption1> ::= -d SECONDARY Id */
  Rule_onmodeOption1_Minusd_SECONDARY_Id,
  /* 2193. <onmodeOption1> ::= -d idxauto ON */
  Rule_onmodeOption1_Minusd_idxauto_ON,
  /* 2194. <onmodeOption1> ::= -d idxauto OFF */
  Rule_onmodeOption1_Minusd_idxauto_OFF,
  /* 2195. <onmodeOption1> ::= -d INDEX Id ':' Id '#' Id */
  Rule_onmodeOption1_Minusd_INDEX_Id_Colon_Id_Num_Id,
  /* 2196. <onmodeOption1> ::= -e ENABLE */
  Rule_onmodeOption1_Minuse_ENABLE,
  /* 2197. <onmodeOption1> ::= -e FLUSH */
  Rule_onmodeOption1_Minuse_FLUSH,
  /* 2198. <onmodeOption1> ::= -e OFF */
  Rule_onmodeOption1_Minuse_OFF,
  /* 2199. <onmodeOption1> ::= -e ON */
  Rule_onmodeOption1_Minuse_ON,
  /* 2200. <onmodeOption1> ::= -w 'STMT_CACHE_HITS' IntegerLiteral */
  Rule_onmodeOption1_Minusw_STMT_CACHE_HITS_IntegerLiteral,
  /* 2201. <onmodeOption1> ::= -w 'STMT_CACHE_NOLIMIT' IntegerLiteral */
  Rule_onmodeOption1_Minusw_STMT_CACHE_NOLIMIT_IntegerLiteral,
  /* 2202. <onmodeOption1> ::= -y IntegerLiteral ON */
  Rule_onmodeOption1_Minusy_IntegerLiteral_ON,
  /* 2203. <onmodeOption1> ::= -y IntegerLiteral OFF */
  Rule_onmodeOption1_Minusy_IntegerLiteral_OFF,
  /* 2204. <onmodeOption1> ::= -wm Id '=' <Expression> */
  Rule_onmodeOption1_Minuswm_Id_Eq,
  /* 2205. <onmodeOption1> ::= -wf Id '=' <Expression> */
  Rule_onmodeOption1_Minuswf_Id_Eq,
  /* 2206. <onmodeOption1> ::= -c START */
  Rule_onmodeOption1_Minusc_START,
  /* 2207. <onmodeOption1> ::= -c STOP IntegerLiteral */
  Rule_onmodeOption1_Minusc_STOP_IntegerLiteral,
  /* 2208. <onmodeOption1> ::= -c kill IntegerLiteral */
  Rule_onmodeOption1_Minusc_kill_IntegerLiteral,
  /* 2209. <onmodeOption1> ::= -c threshold IntegerLiteral */
  Rule_onmodeOption1_Minusc_threshold_IntegerLiteral,
  /* 2210. <onmodeOption1> ::= -c HIGH */
  Rule_onmodeOption1_Minusc_HIGH,
  /* 2211. <onmodeOption1> ::= -c LOW */
  Rule_onmodeOption1_Minusc_LOW,
  /* 2212. <onmodeOption1> ::= '-C2' */
  Rule_onmodeOption1_MinusC2,
  /* 2213. <onparams> ::= onparams -a -d Id -s IntegerLiteral -i */
  Rule_onparams_onparams_Minusa_Minusd_Id_Minuss_IntegerLiteral_Minusi,
  /* 2214. <onparams> ::= onparams -a -d Id -s IntegerLiteral */
  Rule_onparams_onparams_Minusa_Minusd_Id_Minuss_IntegerLiteral,
  /* 2215. <onparams> ::= onparams -a -d Id -i */
  Rule_onparams_onparams_Minusa_Minusd_Id_Minusi,
  /* 2216. <onparams> ::= onparams -a -d Id */
  Rule_onparams_onparams_Minusa_Minusd_Id,
  /* 2217. <onparams> ::= onparams -d -l IntegerLiteral -y */
  Rule_onparams_onparams_Minusd_Minusl_IntegerLiteral_Minusy,
  /* 2218. <onparams> ::= onparams -d -l IntegerLiteral */
  Rule_onparams_onparams_Minusd_Minusl_IntegerLiteral,
  /* 2219. <onparams> ::= onparams -P <onparamsOption1S> -y */
  Rule_onparams_onparams_MinusP_Minusy,
  /* 2220. <onparams> ::= onparams -P <onparamsOption1S> */
  Rule_onparams_onparams_MinusP,
  /* 2221. <onparams> ::= onparams -b -g IntegerLiteral <onparamsOpt2> <onparamsOpt3> <onparamsOpt4> <onparamsOpt5> */
  Rule_onparams_onparams_Minusb_Minusg_IntegerLiteral,
  /* 2222. <onparamsOption1S> ::= <onparamsOption1> <onparamsOption1S> */
  Rule_onparamsOption1S,
  /* 2223. <onparamsOption1S> ::= <onparamsOption1> */
  Rule_onparamsOption1S2,
  /* 2224. <onparamsOption1> ::= -s IntegerLiteral */
  Rule_onparamsOption1_Minuss_IntegerLiteral,
  /* 2225. <onparamsOption1> ::= -d Id */
  Rule_onparamsOption1_Minusd_Id,
  /* 2226. <onparamsOpt2> ::= -n IntegerLiteral */
  Rule_onparamsOpt2_Minusn_IntegerLiteral,
  /* 2227. <onparamsOpt2> ::=  */
  Rule_onparamsOpt2,
  /* 2228. <onparamsOpt3> ::= -r IntegerLiteral */
  Rule_onparamsOpt3_Minusr_IntegerLiteral,
  /* 2229. <onparamsOpt3> ::=  */
  Rule_onparamsOpt3,
  /* 2230. <onparamsOpt4> ::= -x RealLiteral */
  Rule_onparamsOpt4_Minusx_RealLiteral,
  /* 2231. <onparamsOpt4> ::=  */
  Rule_onparamsOpt4,
  /* 2232. <onparamsOpt5> ::= -m RealLiteral */
  Rule_onparamsOpt5_Minusm_RealLiteral,
  /* 2233. <onparamsOpt5> ::=  */
  Rule_onparamsOpt5,
  /* 2234. <ondblog> ::= ondblog <ondblogOption1> <ondblogOpt2> */
  Rule_ondblog_ondblog,
  /* 2235. <ondblogOption1> ::= buf */
  Rule_ondblogOption1_buf,
  /* 2236. <ondblogOption1> ::= unbuf */
  Rule_ondblogOption1_unbuf,
  /* 2237. <ondblogOption1> ::= nolog */
  Rule_ondblogOption1_nolog,
  /* 2238. <ondblogOption1> ::= ANSI */
  Rule_ondblogOption1_ANSI,
  /* 2239. <ondblogOption1> ::= cancel */
  Rule_ondblogOption1_cancel,
  /* 2240. <ondblogOpt2> ::= <Id List2> */
  Rule_ondblogOpt2,
  /* 2241. <ondblogOpt2> ::= -f StringLiteral */
  Rule_ondblogOpt2_Minusf_StringLiteral,
  /* 2242. <ondblogOpt2> ::=  */
  Rule_ondblogOpt22,
  /* 2243. <onlog> ::= onlog <onlogOption1> <onlogOption2S> <oncheckOpt2> */
  Rule_onlog_onlog,
  /* 2244. <onlogOption1> ::= -d Id -b <onlogOption1_S> */
  Rule_onlogOption1_Minusd_Id_Minusb,
  /* 2245. <onlogOption1> ::= -d Id <onlogOption1_S> */
  Rule_onlogOption1_Minusd_Id,
  /* 2246. <onlogOption1> ::= <onlogOption1_S> */
  Rule_onlogOption1,
  /* 2247. <onlogOption1_S> ::= -n <onbarOpt6> <onlogOption1_S> */
  Rule_onlogOption1_S_Minusn,
  /* 2248. <onlogOption1_S> ::= -n <onbarOpt6> */
  Rule_onlogOption1_S_Minusn2,
  /* 2249. <onlogOption2S> ::= <onlogOption2> <onlogOption2S> */
  Rule_onlogOption2S,
  /* 2250. <onlogOption2S> ::=  */
  Rule_onlogOption2S2,
  /* 2251. <onlogOption2> ::= -l */
  Rule_onlogOption2_Minusl,
  /* 2252. <onlogOption2> ::= -t IntegerLiteral */
  Rule_onlogOption2_Minust_IntegerLiteral,
  /* 2253. <onlogOption2> ::= -u StringLiteral */
  Rule_onlogOption2_Minusu_StringLiteral,
  /* 2254. <onlogOption2> ::= -x IntegerLiteral */
  Rule_onlogOption2_Minusx_IntegerLiteral,
  /* 2255. <oncheck> ::= oncheck <oncheckOption1> <oncheckOpt1> <oncheckOpt2> */
  Rule_oncheck_oncheck,
  /* 2256. <oncheckOpt1> ::= -n */
  Rule_oncheckOpt1_Minusn,
  /* 2257. <oncheckOpt1> ::= -y */
  Rule_oncheckOpt1_Minusy,
  /* 2258. <oncheckOpt1> ::=  */
  Rule_oncheckOpt1,
  /* 2259. <oncheckOpt2> ::= -q */
  Rule_oncheckOpt2_Minusq,
  /* 2260. <oncheckOpt2> ::=  */
  Rule_oncheckOpt2,
  /* 2261. <oncheckOption1> ::= -ce */
  Rule_oncheckOption1_Minusce,
  /* 2262. <oncheckOption1> ::= -pe */
  Rule_oncheckOption1_Minuspe,
  /* 2263. <oncheckOption1> ::= -cr */
  Rule_oncheckOption1_Minuscr,
  /* 2264. <oncheckOption1> ::= -pr */
  Rule_oncheckOption1_Minuspr,
  /* 2265. <oncheckOption1> ::= '-cR2' */
  Rule_oncheckOption1_MinuscR2,
  /* 2266. <oncheckOption1> ::= '-pR2' */
  Rule_oncheckOption1_MinuspR2,
  /* 2267. <oncheckOption1> ::= <oncheckOption2> -x Id ':' Id '#' Id */
  Rule_oncheckOption1_Minusx_Id_Colon_Id_Num_Id,
  /* 2268. <oncheckOption1> ::= <oncheckOption2> -x Id */
  Rule_oncheckOption1_Minusx_Id,
  /* 2269. <oncheckOption1> ::= <oncheckOption2> Id ':' Id '#' Id */
  Rule_oncheckOption1_Id_Colon_Id_Num_Id,
  /* 2270. <oncheckOption1> ::= <oncheckOption2> Id */
  Rule_oncheckOption1_Id,
  /* 2271. <oncheckOption1> ::= <oncheckOption2> -x Id ':' Id */
  Rule_oncheckOption1_Minusx_Id_Colon_Id,
  /* 2272. <oncheckOption1> ::= <oncheckOption2> Id ':' Id */
  Rule_oncheckOption1_Id_Colon_Id,
  /* 2273. <oncheckOption1> ::= <oncheckOption3> Id ':' Id ',' Id */
  Rule_oncheckOption1_Id_Colon_Id_Comma_Id,
  /* 2274. <oncheckOption1> ::= <oncheckOption3> Id */
  Rule_oncheckOption1_Id2,
  /* 2275. <oncheckOption1> ::= <oncheckOption3> Id ':' Id */
  Rule_oncheckOption1_Id_Colon_Id2,
  /* 2276. <oncheckOption1> ::= -cc Id */
  Rule_oncheckOption1_Minuscc_Id,
  /* 2277. <oncheckOption1> ::= -pc Id */
  Rule_oncheckOption1_Minuspc_Id,
  /* 2278. <oncheckOption1> ::= -cc */
  Rule_oncheckOption1_Minuscc,
  /* 2279. <oncheckOption1> ::= -pc */
  Rule_oncheckOption1_Minuspc,
  /* 2280. <oncheckOption1> ::= <oncheckOption4> Id ':' Id ',' Id */
  Rule_oncheckOption1_Id_Colon_Id_Comma_Id2,
  /* 2281. <oncheckOption1> ::= <oncheckOption4> Id */
  Rule_oncheckOption1_Id3,
  /* 2282. <oncheckOption1> ::= <oncheckOption4> Id ':' Id */
  Rule_oncheckOption1_Id_Colon_Id3,
  /* 2283. <oncheckOption1> ::= <oncheckOption5> Id ':' Id ',' Id IntegerLiteral */
  Rule_oncheckOption1_Id_Colon_Id_Comma_Id_IntegerLiteral,
  /* 2284. <oncheckOption1> ::= <oncheckOption5> Id */
  Rule_oncheckOption1_Id4,
  /* 2285. <oncheckOption1> ::= <oncheckOption5> Id ':' Id ',' Id */
  Rule_oncheckOption1_Id_Colon_Id_Comma_Id3,
  /* 2286. <oncheckOption1> ::= <oncheckOption5> Id ':' Id IntegerLiteral */
  Rule_oncheckOption1_Id_Colon_Id_IntegerLiteral,
  /* 2287. <oncheckOption1> ::= <oncheckOption5> Id ':' Id */
  Rule_oncheckOption1_Id_Colon_Id4,
  /* 2288. <oncheckOption1> ::= <oncheckOption5> IntegerLiteral IntegerLiteral */
  Rule_oncheckOption1_IntegerLiteral_IntegerLiteral,
  /* 2289. <oncheckOption1> ::= <oncheckOption5> IntegerLiteral */
  Rule_oncheckOption1_IntegerLiteral,
  /* 2290. <oncheckOption1> ::= -pp Id ':' Id ',' Id IntegerLiteral */
  Rule_oncheckOption1_Minuspp_Id_Colon_Id_Comma_Id_IntegerLiteral,
  /* 2291. <oncheckOption1> ::= -pp Id ':' Id IntegerLiteral */
  Rule_oncheckOption1_Minuspp_Id_Colon_Id_IntegerLiteral,
  /* 2292. <oncheckOption1> ::= -pp IntegerLiteral IntegerLiteral */
  Rule_oncheckOption1_Minuspp_IntegerLiteral_IntegerLiteral,
  /* 2293. <oncheckOption1> ::= '-pP2' IntegerLiteral IntegerLiteral */
  Rule_oncheckOption1_MinuspP2_IntegerLiteral_IntegerLiteral,
  /* 2294. <oncheckOption1> ::= -pp Id IntegerLiteral IntegerLiteral */
  Rule_oncheckOption1_Minuspp_Id_IntegerLiteral_IntegerLiteral,
  /* 2295. <oncheckOption1> ::= -pp Id IntegerLiteral */
  Rule_oncheckOption1_Minuspp_Id_IntegerLiteral,
  /* 2296. <oncheckOption1> ::= -pp Id IntegerLiteral -h */
  Rule_oncheckOption1_Minuspp_Id_IntegerLiteral_Minush,
  /* 2297. <oncheckOption1> ::= -pp IntegerLiteral IntegerLiteral IntegerLiteral */
  Rule_oncheckOption1_Minuspp_IntegerLiteral_IntegerLiteral_IntegerLiteral,
  /* 2298. <oncheckOption1> ::= -pp IntegerLiteral IntegerLiteral -h */
  Rule_oncheckOption1_Minuspp_IntegerLiteral_IntegerLiteral_Minush,
  /* 2299. <oncheckOption1> ::= -cs Id */
  Rule_oncheckOption1_Minuscs_Id,
  /* 2300. <oncheckOption1> ::= -cs */
  Rule_oncheckOption1_Minuscs,
  /* 2301. <oncheckOption1> ::= '-cS2' Id */
  Rule_oncheckOption1_MinuscS2_Id,
  /* 2302. <oncheckOption1> ::= '-cS2' */
  Rule_oncheckOption1_MinuscS2,
  /* 2303. <oncheckOption1> ::= -ps Id IntegerLiteral IntegerLiteral */
  Rule_oncheckOption1_Minusps_Id_IntegerLiteral_IntegerLiteral,
  /* 2304. <oncheckOption1> ::= -ps */
  Rule_oncheckOption1_Minusps,
  /* 2305. <oncheckOption1> ::= '-pS2' Id IntegerLiteral IntegerLiteral */
  Rule_oncheckOption1_MinuspS2_Id_IntegerLiteral_IntegerLiteral,
  /* 2306. <oncheckOption1> ::= '-pS2' */
  Rule_oncheckOption1_MinuspS2,
  /* 2307. <oncheckOption1> ::= -u StringLiteral '(' StringLiteral ')' */
  Rule_oncheckOption1_Minusu_StringLiteral_LParan_StringLiteral_RParan,
  /* 2308. <oncheckOption1> ::= -u StringLiteral */
  Rule_oncheckOption1_Minusu_StringLiteral,
  /* 2309. <oncheckOption1> ::= -cv <oncheckOpt6> Id <oncheckOpt7> Id <oncheckOpt7> */
  Rule_oncheckOption1_Minuscv_Id_Id,
  /* 2310. <oncheckOption1> ::= -pv <oncheckOpt6> Id <oncheckOpt7> Id <oncheckOpt7> */
  Rule_oncheckOption1_Minuspv_Id_Id,
  /* 2311. <oncheckOption2> ::= -ci */
  Rule_oncheckOption2_Minusci,
  /* 2312. <oncheckOption2> ::= -cl */
  Rule_oncheckOption2_Minuscl,
  /* 2313. <oncheckOption2> ::= -pk */
  Rule_oncheckOption2_Minuspk,
  /* 2314. <oncheckOption2> ::= '-pK2' */
  Rule_oncheckOption2_MinuspK2,
  /* 2315. <oncheckOption2> ::= -pl */
  Rule_oncheckOption2_Minuspl,
  /* 2316. <oncheckOption2> ::= '-pL2' */
  Rule_oncheckOption2_MinuspL2,
  /* 2317. <oncheckOption3> ::= -cd */
  Rule_oncheckOption3_Minuscd,
  /* 2318. <oncheckOption3> ::= '-cD2' */
  Rule_oncheckOption3_MinuscD2,
  /* 2319. <oncheckOption4> ::= -pB */
  Rule_oncheckOption4_MinuspB,
  /* 2320. <oncheckOption4> ::= -pt */
  Rule_oncheckOption4_Minuspt,
  /* 2321. <oncheckOption4> ::= '-pT2' */
  Rule_oncheckOption4_MinuspT2,
  /* 2322. <oncheckOption5> ::= -pd */
  Rule_oncheckOption5_Minuspd,
  /* 2323. <oncheckOption5> ::= '-pD2' */
  Rule_oncheckOption5_MinuspD2,
  /* 2324. <oncheckOpt6> ::= -b */
  Rule_oncheckOpt6_Minusb,
  /* 2325. <oncheckOpt6> ::= -c */
  Rule_oncheckOpt6_Minusc,
  /* 2326. <oncheckOpt6> ::= -O */
  Rule_oncheckOpt6_MinusO,
  /* 2327. <oncheckOpt6> ::= -t */
  Rule_oncheckOpt6_Minust,
  /* 2328. <oncheckOpt6> ::= -u */
  Rule_oncheckOpt6_Minusu,
  /* 2329. <oncheckOpt6> ::=  */
  Rule_oncheckOpt6,
  /* 2330. <oncheckOpt7> ::= '@' Id Id */
  Rule_oncheckOpt7_At_Id_Id,
  /* 2331. <oncheckOpt7> ::= '@' Id */
  Rule_oncheckOpt7_At_Id,
  /* 2332. <oncheckOpt7> ::= Id */
  Rule_oncheckOpt7_Id,
  /* 2333. <SemicolonOpt> ::= ';' */
  Rule_SemicolonOpt_Semi,
  /* 2334. <SemicolonOpt> ::=  */
  Rule_SemicolonOpt,
  /* 2335. <DISKINIT> ::= DISKINIT NAME '=' Id ',' PHYNAME '=' StringLiteral ',' VDEVNO '=' IntegerLiteral ',' SIZE '=' IntegerLiteral <SemicolonOpt> */
  Rule_DISKINIT_DISKINIT_NAME_Eq_Id_Comma_PHYNAME_Eq_StringLiteral_Comma_VDEVNO_Eq_IntegerLiteral_Comma_SIZE_Eq_IntegerLiteral,
  /* 2336. <DISKINIT> ::= DISKINIT Id ',' StringLiteral ',' IntegerLiteral ',' IntegerLiteral <SemicolonOpt> */
  Rule_DISKINIT_DISKINIT_Id_Comma_StringLiteral_Comma_IntegerLiteral_Comma_IntegerLiteral,
  /* 2337. <DISKINIT> ::= DISKINIT <onspaces> <SemicolonOpt> */
  Rule_DISKINIT_DISKINIT,
  /* 2338. <onspaces> ::= -c -d Id <onspaces_cOption1> <onspaces_cOption2> <onspaces_cOption3> */
  Rule_onspaces_Minusc_Minusd_Id,
  /* 2339. <onspaces> ::= -c -d Id <onspaces_cOption1> <onspaces_cOption2> <onspaces_cOption3> -k IntegerLiteral */
  Rule_onspaces_Minusc_Minusd_Id_Minusk_IntegerLiteral,
  /* 2340. <onspaces> ::= -c -b Id -g IntegerLiteral <onspaces_cOption2> <onspaces_cOption3> */
  Rule_onspaces_Minusc_Minusb_Id_Minusg_IntegerLiteral,
  /* 2341. <onspaces> ::= -c -x Id -l StringLiteral -O IntegerLiteral -s IntegerLiteral */
  Rule_onspaces_Minusc_Minusx_Id_Minusl_StringLiteral_MinusO_IntegerLiteral_Minuss_IntegerLiteral,
  /* 2342. <onspaces> ::= -c -s Id -t <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> <onspaces_cOpt4> */
  Rule_onspaces_Minusc_Minuss_Id_Minust,
  /* 2343. <onspaces> ::= -c -s Id <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> <onspaces_cOpt4> */
  Rule_onspaces_Minusc_Minuss_Id,
  /* 2344. <onspaces> ::= -ch Id -Df <onspaces_defaultlist> */
  Rule_onspaces_Minusch_Id_MinusDf,
  /* 2345. <onspaces> ::= -cl Id */
  Rule_onspaces_Minuscl_Id,
  /* 2346. <onspaces> ::= -a Id <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> -u */
  Rule_onspaces_Minusa_Id_Minusu,
  /* 2347. <onspaces> ::= -a Id <onspaces_cOption2> <onspaces_cOption3> <onspaces_cOpt2> <onspaces_cOpt3> */
  Rule_onspaces_Minusa_Id,
  /* 2348. <onspaces> ::= -d Id -y */
  Rule_onspaces_Minusd_Id_Minusy,
  /* 2349. <onspaces> ::= -d -f Id -y */
  Rule_onspaces_Minusd_Minusf_Id_Minusy,
  /* 2350. <onspaces> ::= -d Id */
  Rule_onspaces_Minusd_Id,
  /* 2351. <onspaces> ::= -d -f Id */
  Rule_onspaces_Minusd_Minusf_Id,
  /* 2352. <onspaces> ::= -d Id -P StringLiteral -O IntegerLiteral -y */
  Rule_onspaces_Minusd_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusy,
  /* 2353. <onspaces> ::= -d Id -P StringLiteral -O IntegerLiteral */
  Rule_onspaces_Minusd_Id_MinusP_StringLiteral_MinusO_IntegerLiteral,
  /* 2354. <onspaces> ::= -m Id <onspace_mS> -y */
  Rule_onspaces_Minusm_Id_Minusy,
  /* 2355. <onspaces> ::= -m Id <onspace_mS> */
  Rule_onspaces_Minusm_Id,
  /* 2356. <onspaces> ::= -r Id -y */
  Rule_onspaces_Minusr_Id_Minusy,
  /* 2357. <onspaces> ::= -r Id */
  Rule_onspaces_Minusr_Id,
  /* 2358. <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -d -y */
  Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusd_Minusy,
  /* 2359. <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -O -y */
  Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_MinusO_Minusy,
  /* 2360. <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -d */
  Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_Minusd,
  /* 2361. <onspaces> ::= -s Id -P StringLiteral -O IntegerLiteral -O */
  Rule_onspaces_Minuss_Id_MinusP_StringLiteral_MinusO_IntegerLiteral_MinusO,
  /* 2362. <onspaces> ::= -f ON <Id List2> -y */
  Rule_onspaces_Minusf_ON_Minusy,
  /* 2363. <onspaces> ::= -f OFF <Id List2> -y */
  Rule_onspaces_Minusf_OFF_Minusy,
  /* 2364. <onspaces> ::= -f ON <Id List2> */
  Rule_onspaces_Minusf_ON,
  /* 2365. <onspaces> ::= -f OFF <Id List2> */
  Rule_onspaces_Minusf_OFF,
  /* 2366. <onspaces> ::= -f ON -y */
  Rule_onspaces_Minusf_ON_Minusy2,
  /* 2367. <onspaces> ::= -f OFF -y */
  Rule_onspaces_Minusf_OFF_Minusy2,
  /* 2368. <onspaces> ::= -f ON */
  Rule_onspaces_Minusf_ON2,
  /* 2369. <onspaces> ::= -f OFF */
  Rule_onspaces_Minusf_OFF2,
  /* 2370. <onspaces> ::= -ren Id -n Id */
  Rule_onspaces_Minusren_Id_Minusn_Id,
  /* 2371. <onspaces_cOption2> ::= -P StringLiteral -O IntegerLiteral -s IntegerLiteral */
  Rule_onspaces_cOption2_MinusP_StringLiteral_MinusO_IntegerLiteral_Minuss_IntegerLiteral,
  /* 2372. <onspaces_cOption3> ::= -m StringLiteral IntegerLiteral */
  Rule_onspaces_cOption3_Minusm_StringLiteral_IntegerLiteral,
  /* 2373. <onspaces_cOption3> ::=  */
  Rule_onspaces_cOption3,
  /* 2374. <onspaces_cOpt2> ::= -Ms IntegerLiteral */
  Rule_onspaces_cOpt2_MinusMs_IntegerLiteral,
  /* 2375. <onspaces_cOpt2> ::=  */
  Rule_onspaces_cOpt2,
  /* 2376. <onspaces_cOpt3> ::= -Mo IntegerLiteral */
  Rule_onspaces_cOpt3_MinusMo_IntegerLiteral,
  /* 2377. <onspaces_cOpt3> ::=  */
  Rule_onspaces_cOpt3,
  /* 2378. <onspaces_cOpt4> ::= -Df <onspaces_defaultlist> */
  Rule_onspaces_cOpt4_MinusDf,
  /* 2379. <onspaces_cOpt4> ::=  */
  Rule_onspaces_cOpt4,
  /* 2380. <onspace_mS> ::= <onspace_m> <onspace_mS> */
  Rule_onspace_mS,
  /* 2381. <onspace_mS> ::= <onspace_m> */
  Rule_onspace_mS2,
  /* 2382. <onspace_m> ::= <onspaces_cOption2> */
  Rule_onspace_m,
  /* 2383. <onspace_m> ::= -f StringLiteral */
  Rule_onspace_m_Minusf_StringLiteral,
  /* 2384. <onspaces_cOption1> ::= -ef IntegerLiteral -en IntegerLiteral */
  Rule_onspaces_cOption1_Minusef_IntegerLiteral_Minusen_IntegerLiteral,
  /* 2385. <onspaces_cOption1> ::= -t */
  Rule_onspaces_cOption1_Minust,
  /* 2386. <onspaces_cOption1> ::=  */
  Rule_onspaces_cOption1,
  /* 2387. <onspaces_defaultlist> ::= '"' <onspaces_default> <onspaces_defaultlist_> '"' */
  Rule_onspaces_defaultlist_Quote_Quote,
  /* 2388. <onspaces_defaultlist_> ::= ',' <onspaces_default> <onspaces_defaultlist_> */
  Rule_onspaces_defaultlist__Comma,
  /* 2389. <onspaces_defaultlist_> ::=  */
  Rule_onspaces_defaultlist_,
  /* 2390. <onspaces_default> ::= ACCESSTIME '=' OFF */
  Rule_onspaces_default_ACCESSTIME_Eq_OFF,
  /* 2391. <onspaces_default> ::= ACCESSTIME '=' ON */
  Rule_onspaces_default_ACCESSTIME_Eq_ON,
  /* 2392. <onspaces_default> ::= 'AVG_LO_SIZE' '=' IntegerLiteral */
  Rule_onspaces_default_AVG_LO_SIZE_Eq_IntegerLiteral,
  /* 2393. <onspaces_default> ::= BUFFERING '=' ON */
  Rule_onspaces_default_BUFFERING_Eq_ON,
  /* 2394. <onspaces_default> ::= BUFFERING '=' OFF */
  Rule_onspaces_default_BUFFERING_Eq_OFF,
  /* 2395. <onspaces_default> ::= 'LOCK_MODE' '=' BLOB */
  Rule_onspaces_default_LOCK_MODE_Eq_BLOB,
  /* 2396. <onspaces_default> ::= 'LOCK_MODE' '=' RANGE */
  Rule_onspaces_default_LOCK_MODE_Eq_RANGE,
  /* 2397. <onspaces_default> ::= LOGGING '=' OFF */
  Rule_onspaces_default_LOGGING_Eq_OFF,
  /* 2398. <onspaces_default> ::= LOGGING '=' ON */
  Rule_onspaces_default_LOGGING_Eq_ON,
  /* 2399. <onspaces_default> ::= 'EXTENT_SIZE' '=' IntegerLiteral */
  Rule_onspaces_default_EXTENT_SIZE_Eq_IntegerLiteral,
  /* 2400. <onspaces_default> ::= 'MIN_EXT_SIZE' '=' IntegerLiteral */
  Rule_onspaces_default_MIN_EXT_SIZE_Eq_IntegerLiteral,
  /* 2401. <onspaces_default> ::= 'NEXT_SIZE' '=' IntegerLiteral */
  Rule_onspaces_default_NEXT_SIZE_Eq_IntegerLiteral,
  /* 2402. <Id List2> ::= Id <Id List2> */
  Rule_IdList2_Id,
  /* 2403. <Id List2> ::= Id */
  Rule_IdList2_Id2,
  /* 2404. <onaudit> ::= onaudit -O */
  Rule_onaudit_onaudit_MinusO,
  /* 2405. <onaudit> ::= onaudit -O -u Id */
  Rule_onaudit_onaudit_MinusO_Minusu_Id,
  /* 2406. <onaudit> ::= onaudit -O -y */
  Rule_onaudit_onaudit_MinusO_Minusy,
  /* 2407. <onaudit> ::= onaudit -a <Audit-Mask Specification> */
  Rule_onaudit_onaudit_Minusa,
  /* 2408. <onaudit> ::= onaudit -f <onaudit Input-File Format> */
  Rule_onaudit_onaudit_Minusf,
  /* 2409. <onaudit> ::= onaudit -m <Audit-Mask Specification> */
  Rule_onaudit_onaudit_Minusm,
  /* 2410. <onaudit> ::= onaudit -d */
  Rule_onaudit_onaudit_Minusd,
  /* 2411. <onaudit> ::= onaudit -d -u Id */
  Rule_onaudit_onaudit_Minusd_Minusu_Id,
  /* 2412. <onaudit> ::= onaudit -d -y */
  Rule_onaudit_onaudit_Minusd_Minusy,
  /* 2413. <onaudit> ::= onaudit -n */
  Rule_onaudit_onaudit_Minusn,
  /* 2414. <onaudit> ::= onaudit -c */
  Rule_onaudit_onaudit_Minusc,
  /* 2415. <onaudit> ::= onaudit <onaudit_auditmodeOpt> <onaudit_errormodeOpt> <onaudit_auditdirOpt> <onaudit_maxsizeOpt> */
  Rule_onaudit_onaudit,
  /* 2416. <Audit-Mask Specification> ::= -u Id -r Id <Audit-Mask Specification_S> */
  Rule_AuditMaskSpecification_Minusu_Id_Minusr_Id,
  /* 2417. <Audit-Mask Specification> ::= -u Id <Audit-Mask Specification_S> */
  Rule_AuditMaskSpecification_Minusu_Id,
  /* 2418. <Audit-Mask Specification_S> ::= <Audit-Mask Specification_> <Audit-Mask Specification_S> */
  Rule_AuditMaskSpecification_S,
  /* 2419. <Audit-Mask Specification_S> ::=  */
  Rule_AuditMaskSpecification_S2,
  /* 2420. <Audit-Mask Specification_> ::= -e <Audit Event Specification> */
  Rule_AuditMaskSpecification__Minuse,
  /* 2421. <Audit-Mask Specification_> ::= -e '+' <Audit Event Specification> */
  Rule_AuditMaskSpecification__Minuse_Plus,
  /* 2422. <Audit-Mask Specification_> ::= -e '-' <Audit Event Specification> */
  Rule_AuditMaskSpecification__Minuse_Minus,
  /* 2423. <Audit Event Specification> ::= <Id List> */
  Rule_AuditEventSpecification,
  /* 2424. <onaudit Input-File Format> ::= Id Id <Audit-Mask Specification_S> */
  Rule_onauditInputFileFormat_Id_Id,
  /* 2425. <onaudit Input-File Format> ::= Id <Audit-Mask Specification_S> */
  Rule_onauditInputFileFormat_Id,
  /* 2426. <onaudit_auditmodeOpt> ::= -l Id */
  Rule_onaudit_auditmodeOpt_Minusl_Id,
  /* 2427. <onaudit_auditmodeOpt> ::=  */
  Rule_onaudit_auditmodeOpt,
  /* 2428. <onaudit_errormodeOpt> ::= -e Id */
  Rule_onaudit_errormodeOpt_Minuse_Id,
  /* 2429. <onaudit_errormodeOpt> ::=  */
  Rule_onaudit_errormodeOpt,
  /* 2430. <onaudit_auditdirOpt> ::= -P StringLiteral */
  Rule_onaudit_auditdirOpt_MinusP_StringLiteral,
  /* 2431. <onaudit_auditdirOpt> ::= -P Id */
  Rule_onaudit_auditdirOpt_MinusP_Id,
  /* 2432. <onaudit_auditdirOpt> ::=  */
  Rule_onaudit_auditdirOpt,
  /* 2433. <onaudit_maxsizeOpt> ::= -s IntegerLiteral */
  Rule_onaudit_maxsizeOpt_Minuss_IntegerLiteral,
  /* 2434. <onaudit_maxsizeOpt> ::=  */
  Rule_onaudit_maxsizeOpt 
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
  //if (Token != NULL) fprintf(stdout," at line %d column %d",Token->Line,Token->Column);
  if(NULL!=Token) fprintf(stdout, " at line %ld column %ld", Token->Line, Token->Column);
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
  InputBuf = LoadInputFile("ZSQL_IDS__test.sql");
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
