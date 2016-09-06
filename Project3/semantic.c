#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "syntax.h"

#define TRUE 1
#define FALSE 0

/* ------------------------------------------------------- */
/* -------------------- LEXER SECTION -------------------- */
/* ------------------------------------------------------- */

#define KEYWORDS  11

#define MAX_TYPES 20
struct symbol* table[MAX_TYPES];
int new_types;

enum TokenTypes
{
    VAR = 1, WHILE, INT, REAL, STRING, BOOLEAN, 
    TYPE, LONG, DO, CASE, SWITCH,
    PLUS, MINUS, DIV, MULT, EQUAL,
    COLON, COMMA, SEMICOLON,
    LBRAC, RBRAC, LPAREN, RPAREN, LBRACE, RBRACE,
    NOTEQUAL, GREATER, LESS, LTEQ, GTEQ, DOT,
    ID, NUM, REALNUM,
    ERROR
};

char *reserved[] = {"",
    "VAR", "WHILE", "INT", "REAL", "STRING", "BOOLEAN",
    "TYPE", "LONG", "DO", "CASE", "SWITCH",
    "+", "-", "/", "*", "=",
    ":", ",", ";",
    "[", "]", "(", ")", "{", "}",
    "<>", ">", "<", "<=", ">=", ".",
    "ID", "NUM", "REALNUM",
    "ERROR"
};

// Global Variables associated with the next input token
#define MAX_TOKEN_LENGTH 100
char token[MAX_TOKEN_LENGTH]; // token string
int ttype; // token type
int activeToken = FALSE;
int tokenLength;
int line_no = 1;

void skipSpace()
{
    char c;

    c = getchar();
    line_no += (c == '\n');
    while (!feof(stdin) && isspace(c))
    {
        c = getchar();
        line_no += (c == '\n');
    }
    ungetc(c, stdin);
}

int isKeyword(char *s)
{
    int i;

    for (i = 1; i <= KEYWORDS; i++)
        if (strcmp(reserved[i], s) == 0)
            return i;
    return FALSE;
}

/*
 * ungetToken() simply sets a flag so that when getToken() is called
 * the old ttype is returned and the old token is not overwritten.
 * NOTE: BETWEEN ANY TWO SEPARATE CALLS TO ungetToken() THERE MUST BE
 * AT LEAST ONE CALL TO getToken(). CALLING TWO ungetToken() WILL NOT
 * UNGET TWO TOKENS
 */
void ungetToken()
{
    activeToken = TRUE;
}

int scan_number()
{
    char c;

    c = getchar();
    if (isdigit(c))
    {
        // First collect leading digits before dot
        // 0 is a nNUM by itself
        if (c == '0')
        {
            token[tokenLength] = c;
            tokenLength++;
            token[tokenLength] = '\0';
        }
        else
        {
            while (isdigit(c))
            {
                token[tokenLength] = c;
                tokenLength++;
                c = getchar();
            }
            ungetc(c, stdin);
            token[tokenLength] = '\0';
        }
        // Check if leading digits are integer part of a REALNUM
        c = getchar();
        if (c == '.')
        {
            c = getchar();
            if (isdigit(c))
            {
                token[tokenLength] = '.';
                tokenLength++;
                while (isdigit(c))
                {
                    token[tokenLength] = c;
                    tokenLength++;
                    c = getchar();
                }
                token[tokenLength] = '\0';
                if (!feof(stdin))
                    ungetc(c, stdin);
                return REALNUM;
            }
            else
            {
                ungetc(c, stdin);
                c = '.';
                ungetc(c, stdin);
                return NUM;
            }
        }
        else
        {
            ungetc(c, stdin);
            return NUM;
        }
    }
    else
        return ERROR;
}

int scan_id_or_keyword()
{
    int ttype;
    char c;

    c = getchar();
    if (isalpha(c))
    {
        while (isalnum(c))
        {
            token[tokenLength] = c;
            tokenLength++;
            c = getchar();
        }
        if (!feof(stdin))
            ungetc(c, stdin);
        token[tokenLength] = '\0';
        ttype = isKeyword(token);
        if (ttype == 0)
            ttype = ID;
        return ttype;
    }
    else
        return ERROR;
}

int getToken()
{
    char c;

    if (activeToken)
    {
        activeToken = FALSE;
        return ttype;
    }
    skipSpace();
    tokenLength = 0;
    c = getchar();
    switch (c)
    {
        case '.':
            return DOT;
        case '+':
            return PLUS;
        case '-':
            return MINUS;
        case '/':
            return DIV;
        case '*':
            return MULT;
        case '=':
            return EQUAL;
        case ':':
            return COLON;
        case ',':
            return COMMA;
        case ';':
            return SEMICOLON;
        case '[':
            return LBRAC;
        case ']':
            return RBRAC;
        case '(':
            return LPAREN;
        case ')':
            return RPAREN;
        case '{':
            return LBRACE;
        case '}':
            return RBRACE;
        case '<':
            c = getchar();
            if (c == '=')
                return LTEQ;
            else if (c == '>')
                return NOTEQUAL;
            else
            {
                ungetc(c, stdin);
                return LESS;
            }
        case '>':
            c = getchar();
            if (c == '=')
                return GTEQ;
            else
            {
                ungetc(c, stdin);
                return GREATER;
            }
        default:
            if (isdigit(c))
            {
                ungetc(c, stdin);
                return scan_number();
            }
            else if (isalpha(c))
            {
                ungetc(c, stdin);
                return scan_id_or_keyword();
            }
            else if (c == EOF)
                return EOF;
            else
                return ERROR;
    }
}

/* ----------------------------------------------------------------- */
/* -------------------- SYNTAX ANALYSIS SECTION -------------------- */
/* ----------------------------------------------------------------- */

void syntax_error(const char* msg)
{
    printf("Syntax error while parsing %s line %d\n", msg, line_no);
    exit(1);
}

/* -------------------- PRINTING PARSE TREE -------------------- */
void print_parse_tree(struct programNode* program)
{
    print_decl(program->decl);
    print_body(program->body);
}

void print_decl(struct declNode* dec)
{
    if (dec->type_decl_section != NULL)
    {
        print_type_decl_section(dec->type_decl_section);
    }
    if (dec->var_decl_section != NULL)
    {
        print_var_decl_section(dec->var_decl_section);
    }
}

void print_body(struct bodyNode* body)
{
    printf("{\n");
    print_stmt_list(body->stmt_list);
    printf("}\n");
}

void print_var_decl_section(struct var_decl_sectionNode* varDeclSection)
{
    printf("VAR\n");
    if (varDeclSection->var_decl_list != NULL)
        print_var_decl_list(varDeclSection->var_decl_list);
}

void print_var_decl_list(struct var_decl_listNode* varDeclList)
{
    print_var_decl(varDeclList->var_decl);
    if (varDeclList->var_decl_list != NULL)
        print_var_decl_list(varDeclList->var_decl_list);
}

void print_var_decl(struct var_declNode* varDecl)
{
    print_id_list(varDecl->id_list);
    printf(": ");
    print_type_name(varDecl->type_name);
    printf(";\n");
}

void print_type_decl_section(struct type_decl_sectionNode* typeDeclSection)
{
    printf("TYPE\n");
    if (typeDeclSection->type_decl_list != NULL)
        print_type_decl_list(typeDeclSection->type_decl_list);
}

void print_type_decl_list(struct type_decl_listNode* typeDeclList)
{
    print_type_decl(typeDeclList->type_decl);
    if (typeDeclList->type_decl_list != NULL)
        print_type_decl_list(typeDeclList->type_decl_list);
}

void print_type_decl(struct type_declNode* typeDecl)
{
    print_id_list(typeDecl->id_list);
    printf(": ");
    print_type_name(typeDecl->type_name);
    printf(";\n");
}

void print_type_name(struct type_nameNode* typeName)
{
    if (typeName->type != ID)
        printf("%s ", reserved[typeName->type]);
    else
        printf("%s ", typeName->id);
}

void print_id_list(struct id_listNode* idList)
{
    printf("%s ", idList->id);
    if (idList->id_list != NULL)
    {
        printf(", ");
        print_id_list(idList->id_list);
    }
}

void print_stmt_list(struct stmt_listNode* stmt_list)
{
    print_stmt(stmt_list->stmt);
    if (stmt_list->stmt_list != NULL)
        print_stmt_list(stmt_list->stmt_list);

}

void print_assign_stmt(struct assign_stmtNode* assign_stmt)
{
    printf("%s ", assign_stmt->id);
    printf("= ");
    print_expression_prefix(assign_stmt->expr);
    printf("; \n");
}

void print_stmt(struct stmtNode* stmt)
{
    switch (stmt->stmtType)
    {
        case ASSIGN:
            print_assign_stmt(stmt->assign_stmt);
            break;
        case WHILE:
            print_while_stmt(stmt->while_stmt);
            break;
        case DO:
            print_do_stmt(stmt->while_stmt);
            break;
        case SWITCH:
            print_switch_stmt(stmt->switch_stmt);
            break;
    }
}

void print_expression_prefix(struct exprNode* expr)
{
    if (expr->tag == EXPR)
    {
        printf("%s ", reserved[expr->op]);
        print_expression_prefix(expr->leftOperand);
        print_expression_prefix(expr->rightOperand);
    }
    else if (expr->tag == PRIMARY)
    {
        if (expr->primary->tag == ID)
            printf("%s ", expr->primary->id);
        else if (expr->primary->tag == NUM)
            printf("%d ", expr->primary->ival);
        else if (expr->primary->tag == REALNUM)
            printf("%.4f ", expr->primary->fval);
    }
}

void print_while_stmt(struct while_stmtNode* while_stmt)
{
    // TODO: implement this
    printf("WHILE ");
    print_condition(while_stmt->condition);
    print_body(while_stmt->body);
}

void print_do_stmt(struct while_stmtNode* do_stmt)
{
    // TODO: implement this
    printf("DO ");
    print_body(do_stmt->body);
    printf("WHILE ");
    print_condition(do_stmt->condition);
    printf("; \n");
    
}

void print_condition(struct conditionNode* condition)
{
    // TODO: implement this
    if (condition->left_operand->tag == ID)
        printf("%s ", condition->left_operand->id);
    else if (condition->left_operand->tag == NUM)
        printf("%d ", condition->left_operand->ival);
    else if (condition->left_operand->tag == REALNUM)
        printf("%.4f ", condition->left_operand->fval);
    
    if (condition->relop != NOOP)
    {
        printf("%s ", reserved[condition->relop]);
        if (condition->right_operand->tag == ID)
            printf("%s ", condition->right_operand->id);
        else if (condition->right_operand->tag == NUM)
            printf("%d ", condition->right_operand->ival);
        else if (condition->right_operand->tag == REALNUM)
            printf("%.4f ", condition->right_operand->fval);
    }
}

void print_case(struct caseNode* cas)
{
    // TODO: implement this
    printf("CASE ");
    printf("%d", cas->num);
    printf(": ");
    print_body(cas->body);
}

void print_case_list(struct case_listNode* case_list)
{
    // TODO: implement this
    print_case(case_list->cas);
    if (case_list->case_list != NULL)
        print_case_list(case_list->case_list);
}

void print_switch_stmt(struct switch_stmtNode* switc)
{
    // TODO: implement this
    printf("SWITCH ");
    printf("%s ", switc->id);
    printf("{\n");
    print_case_list(switc->case_list);
    printf("}\n");
}

/* -------------------- PARSING AND BUILDING PARSE TREE -------------------- */

// Note that the following function is not
// called case because case is a keyword in C/C++
struct caseNode* cas()
{
    // TODO: implement this
    struct caseNode* ca;
    ca = ALLOC(struct caseNode);
    
    ttype = getToken();
    if (ttype == CASE)
    {
        ttype = getToken();
        if (ttype == NUM)
        {
            ca->num = NUM;            
            ttype = getToken();
            if (ttype == COLON)
            {
                ttype = getToken();
                if (ttype == LBRACE)
                {
                    ungetToken();                
                    ca->body = body();           
                    return ca;
                }
                else
                    syntax_error("case. LBRACE expected");
            }
            else
                syntax_error("case. COLON expected");
        }
        else
            syntax_error("case. NUM expected");
    }
    else
        syntax_error("case. CASE expected");
    return NULL;
}

struct case_listNode* case_list()
{
    // TODO: implement this
    struct case_listNode* caseList;

    ttype = getToken();
    if (ttype == CASE)
    {
        ungetToken();
        caseList = ALLOC(struct case_listNode);
        caseList->cas = cas();
        ttype = getToken();
        if (ttype == CASE)
        {
            ungetToken();
            caseList->case_list = case_list();
            return caseList;
        }
        else // If the next token is not in FOLLOW(stmt_list),
             // let the caller handle it.
        {
            ungetToken();
            caseList->case_list = NULL;
            return caseList;
        }
    }
    else
        syntax_error("case_list. CASE expected");
    return NULL;
}

struct switch_stmtNode* switch_stmt()
{
    // TODO: implement this
    struct switch_stmtNode* switchStmt;

    ttype = getToken();
    if (ttype == SWITCH)
    {
        ttype = getToken();
        if (ttype == ID)
        {
            switchStmt = ALLOC(struct switch_stmtNode);
            switchStmt->id = strdup(token);
            ttype = getToken();
            if (ttype == LBRACE)
            {
                switchStmt->case_list = case_list();
                ttype = getToken();
                if (ttype == RBRACE)
                {
                    return switchStmt;
                }
                else
                    syntax_error("switch_stmt. RBRACE expected");
            }
            else
                syntax_error("switch_stmt. LBRACE expected");
        }
        else
            syntax_error("case_list. ID expected");
    }
    else
        syntax_error("case_list. SWITCH expected");
    return NULL;
}

struct while_stmtNode* do_stmt()
{
    // TODO: implement this
    struct while_stmtNode* doStmt;

    ttype = getToken();
    if (ttype == DO)
    {
        doStmt = ALLOC(struct while_stmtNode);
	doStmt->body = body();
	ttype = getToken();
        if (ttype == WHILE)
        {
            doStmt->condition = condition();
            ttype = getToken();
            if (ttype == SEMICOLON)
            {
                return doStmt;
            }
            else
                syntax_error("do_stmt. SEMICOLON expected");
        }
        else
            syntax_error("do_stmt. WHILE expected");
    }
    else
        syntax_error("do_stmt. DO expected");
    return NULL;
}

struct primaryNode* primary()
{
    // TODO: implement this
    struct primaryNode* prim;

    prim = ALLOC(struct primaryNode);
    ttype = getToken();
    if (ttype == ID || ttype == NUM || ttype == REALNUM)
    {
        switch (ttype)
	{
		case ID:
			prim->tag = ID;
			prim->id = strdup(token); 
			break;

		case NUM:
			prim->tag = NUM;
			prim->ival = atoi(token); 
			break;

		case REALNUM:
			prim->tag = REALNUM;
			prim->fval = atof(token);
			break;
	}
        return prim;
    }
    else
        syntax_error("primary. primary expected");
    return NULL;
}

struct conditionNode* condition()
{
    // TODO: implement this
    struct conditionNode* cond;

    cond = ALLOC(struct conditionNode);
    ttype = getToken();
    if (ttype == ID || ttype == NUM || ttype == REALNUM)
    {
        ungetToken();
		cond->left_operand = primary();
		ttype = getToken();
        if (ttype == NOTEQUAL || ttype == GREATER || ttype == LESS || ttype == LTEQ || ttype == GTEQ)
		{
			cond->relop = ttype;
			cond->right_operand = primary();
		}
		else
		{
			ungetToken();
			cond->relop = NOOP;
			cond->right_operand = NULL;
		}
        return cond;
    }
    else
        syntax_error("condition. ID, NUM, or REALNUM expected");
    return NULL;
}

struct while_stmtNode* while_stmt()
{
    // TODO: implement this
    struct while_stmtNode* whil;

    ttype = getToken();
    if (ttype == WHILE)
    {
        whil = ALLOC(struct while_stmtNode);
        whil->condition = condition();
        whil->body = body();
	return whil;
    }
    else
            syntax_error("while_stmt. WHILE expected");	
    return NULL;
}

struct exprNode* factor()
{
    struct exprNode* facto;

    ttype = getToken();
    if (ttype == LPAREN)
    {
        facto = expr();
        ttype = getToken();
        if (ttype == RPAREN)
            return facto;
        else
            syntax_error("factor. RPAREN expected");
    }
    else if (ttype == NUM)
    {
        facto = ALLOC(struct exprNode);
        facto->primary = ALLOC(struct primaryNode);
        facto->tag = PRIMARY;
        facto->op = NOOP;
        facto->leftOperand = NULL;
        facto->rightOperand = NULL;
        facto->primary->tag = NUM;
        facto->primary->ival = atoi(token);
        return facto;
    }
    else if (ttype == REALNUM)
    {
        facto = ALLOC(struct exprNode);
        facto->primary = ALLOC(struct primaryNode);
        facto->tag = PRIMARY;
        facto->op = NOOP;
        facto->leftOperand = NULL;
        facto->rightOperand = NULL;
        facto->primary->tag = REALNUM;
        facto->primary->fval = atof(token);
        return facto;
    }
    else if (ttype == ID)
    {
        facto = ALLOC(struct exprNode);
        facto->primary = ALLOC(struct primaryNode);
        facto->tag = PRIMARY;
        facto->op = NOOP;
        facto->leftOperand = NULL;
        facto->rightOperand = NULL;
        facto->primary->tag = ID;
        facto->primary->id = strdup(token);
        return facto;
    }
    else
        syntax_error("factor. NUM, REALNUM, or ID, expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct exprNode* term()
{
    struct exprNode* ter;
    struct exprNode* f;

    ttype = getToken();
    if (ttype == ID || ttype == LPAREN || ttype == NUM || ttype == REALNUM)
    {
        ungetToken();
        f = factor();
        ttype = getToken();
        if (ttype == MULT || ttype == DIV)
        {
            ter = ALLOC(struct exprNode);
            ter->op = ttype;
            ter->leftOperand = f;
            ter->rightOperand = term();
            ter->tag = EXPR;
            ter->primary = NULL;
            return ter;
        }
        else if (ttype == SEMICOLON || ttype == PLUS ||
                 ttype == MINUS || ttype == RPAREN)
        {
            ungetToken();
            return f;
        }
        else
            syntax_error("term. MULT or DIV expected");
    }
    else
        syntax_error("term. ID, LPAREN, NUM, or REALNUM expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct exprNode* expr()
{
    struct exprNode* exp;
    struct exprNode* t;

    ttype = getToken();
    if (ttype == ID || ttype == LPAREN || ttype == NUM || ttype == REALNUM)
    {
        ungetToken();
        t = term();
        ttype = getToken();
        if (ttype == PLUS || ttype == MINUS)
        {
            exp = ALLOC(struct exprNode);
            exp->op = ttype;
            exp->leftOperand = t;
            exp->rightOperand = expr();
            exp->tag = EXPR;
            exp->primary = NULL;
            return exp;
        }
        else if (ttype == SEMICOLON || ttype == MULT ||
                 ttype == DIV || ttype == RPAREN)
        {
            ungetToken();
            return t;
        }
        else
            syntax_error("expr. PLUS, MINUS, or SEMICOLON expected");
    }
    else
        syntax_error("expr. ID, LPAREN, NUM, or REALNUM expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct assign_stmtNode* assign_stmt()
{
    struct assign_stmtNode* assignStmt;

    ttype = getToken();
    if (ttype == ID)
    {
        assignStmt = ALLOC(struct assign_stmtNode);
        assignStmt->id = strdup(token);
        ttype = getToken();
        if (ttype == EQUAL)
        {
            assignStmt->expr = expr();
            ttype = getToken();
            if (ttype == SEMICOLON)
                return assignStmt;
            else
                syntax_error("assign_stmt. SEMICOLON expected");
        }
        else
            syntax_error("assign_stmt. EQUAL expected");
    }
    else
        syntax_error("assign_stmt. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct stmtNode* stmt()
{
    struct stmtNode* stm;

    stm = ALLOC(struct stmtNode);
    ttype = getToken();
    if (ttype == ID) // assign_stmt
    {
        ungetToken();
        stm->assign_stmt = assign_stmt();
        stm->stmtType = ASSIGN;
    }
    else if (ttype == WHILE) // while_stmt
    {
        ungetToken();
        stm->while_stmt = while_stmt();
        stm->stmtType = WHILE;
    }
    else if (ttype == DO)  // do_stmt
    {
        ungetToken();
        stm->while_stmt = do_stmt();
        stm->stmtType = DO;
    }
    else if (ttype == SWITCH) // switch_stmt
    {
        ungetToken();
        stm->switch_stmt = switch_stmt();
        stm->stmtType = SWITCH;
    }
    else
        syntax_error("stmt. ID, WHILE, DO or SWITCH expected");
    return stm;
}

struct stmt_listNode* stmt_list()
{
    struct stmt_listNode* stmtList;

    ttype = getToken();
    if (ttype == ID || ttype == WHILE ||
        ttype == DO || ttype == SWITCH)
    {
        ungetToken();
        stmtList = ALLOC(struct stmt_listNode);
        stmtList->stmt = stmt();
        ttype = getToken();
        if (ttype == ID || ttype == WHILE ||
            ttype == DO || ttype == SWITCH)
        {
            ungetToken();
            stmtList->stmt_list = stmt_list();
            return stmtList;
        }
        else // If the next token is not in FOLLOW(stmt_list),
             // let the caller handle it.
        {
            ungetToken();
            stmtList->stmt_list = NULL;
            return stmtList;
        }
    }
    else
        syntax_error("stmt_list. ID, WHILE, DO or SWITCH expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct bodyNode* body()
{
    struct bodyNode* bod;

    ttype = getToken();
    if (ttype == LBRACE)
    {
        bod = ALLOC(struct bodyNode);
        bod->stmt_list = stmt_list();
        ttype = getToken();
        if (ttype == RBRACE)
            return bod;
        else
            syntax_error("body. RBRACE expected");
    }
    else
        syntax_error("body. LBRACE expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_nameNode* type_name()
{
    struct type_nameNode* tName;

    tName = ALLOC(struct type_nameNode);
    ttype = getToken();
    if (ttype == ID || ttype == INT || ttype == REAL ||
            ttype == STRING || ttype == BOOLEAN || ttype == LONG)
    {
        tName->type = ttype;
        if (ttype == ID)
            tName->id = strdup(token);
        else
            tName->id = NULL;
        return tName;
    }
    else
        syntax_error("type_name. type name expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct id_listNode* id_list()
{
    struct id_listNode* idList;

    idList = ALLOC(struct id_listNode);
    ttype = getToken();
    if (ttype == ID)
    {
        idList->id = strdup(token);
        ttype = getToken();
        if (ttype == COMMA)
        {
            idList->id_list = id_list();
            return idList;
        }
        else if (ttype == COLON)
        {
            ungetToken();
            idList->id_list = NULL;
            return idList;
        }
        else
            syntax_error("id_list. COMMA or COLON expected");
    }
    else
        syntax_error("id_list. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_declNode* type_decl()
{
    struct type_declNode* typeDecl;

    typeDecl = ALLOC(struct type_declNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        typeDecl->id_list = id_list();
        ttype = getToken();
        if (ttype == COLON)
        {
            typeDecl->type_name = type_name();
            ttype = getToken();
            if (ttype == SEMICOLON)
                return typeDecl;
            else
                syntax_error("type_decl. SEMICOLON expected");
        }
        else
            syntax_error("type_decl. COLON expected");
    }
    else
        syntax_error("type_decl. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct var_declNode* var_decl()
{
    struct var_declNode* varDecl;

    varDecl = ALLOC(struct var_declNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        varDecl->id_list = id_list();
        ttype = getToken();
        if (ttype == COLON)
        {
            varDecl->type_name = type_name();
            ttype = getToken();
            if (ttype == SEMICOLON)
                return varDecl;
            else
                syntax_error("var_decl. SEMICOLON expected");
        }
        else
            syntax_error("var_decl. COLON expected");
    }
    else
        syntax_error("var_decl. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct var_decl_listNode* var_decl_list()
{
    struct var_decl_listNode* varDeclList;

    varDeclList = ALLOC(struct var_decl_listNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        varDeclList->var_decl = var_decl();
        ttype = getToken();
        if (ttype == ID)
        {
            ungetToken();
            varDeclList->var_decl_list = var_decl_list();
            return varDeclList;
        }
        else
        {
            ungetToken();
            varDeclList->var_decl_list = NULL;
            return varDeclList;
        }
    }
    else
        syntax_error("var_decl_list. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_decl_listNode* type_decl_list()
{
    struct type_decl_listNode* typeDeclList;

    typeDeclList = ALLOC(struct type_decl_listNode);
    ttype = getToken();
    if (ttype == ID)
    {
        ungetToken();
        typeDeclList->type_decl = type_decl();
        ttype = getToken();
        if (ttype == ID)
        {
            ungetToken();
            typeDeclList->type_decl_list = type_decl_list();
            return typeDeclList;
        }
        else
        {
            ungetToken();
            typeDeclList->type_decl_list = NULL;
            return typeDeclList;
        }
    }
    else
        syntax_error("type_decl_list. ID expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct var_decl_sectionNode* var_decl_section()
{
    struct var_decl_sectionNode *varDeclSection;

    varDeclSection = ALLOC(struct var_decl_sectionNode);
    ttype = getToken();
    if (ttype == VAR)
    {
        // no need to ungetToken()
        varDeclSection->var_decl_list = var_decl_list();
        return varDeclSection;
    }
    else
        syntax_error("var_decl_section. VAR expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct type_decl_sectionNode* type_decl_section()
{
    struct type_decl_sectionNode *typeDeclSection;

    typeDeclSection = ALLOC(struct type_decl_sectionNode);
    ttype = getToken();
    if (ttype == TYPE)
    {
        typeDeclSection->type_decl_list = type_decl_list();
        return typeDeclSection;
    }
    else
        syntax_error("type_decl_section. TYPE expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct declNode* decl()
{
    struct declNode* dec;

    dec = ALLOC(struct declNode);
    dec->type_decl_section = NULL;
    dec->var_decl_section = NULL;
    ttype = getToken();
    if (ttype == TYPE)
    {
        ungetToken();
        dec->type_decl_section = type_decl_section();
        ttype = getToken();
        if (ttype == VAR)
        {
            // type_decl_list is epsilon
            // or type_decl already parsed and the
            // next token is checked
            ungetToken();
            dec->var_decl_section = var_decl_section();
        }
        else
        {
            ungetToken();
            dec->var_decl_section = NULL;
        }
        return dec;
    }
    else
    {
        dec->type_decl_section = NULL;
        if (ttype == VAR)
        {
            // type_decl_list is epsilon
            // or type_decl already parsed and the
            // next token is checked
            ungetToken();
            dec->var_decl_section = var_decl_section();
            return dec;
        }
        else
        {
            if (ttype == LBRACE)
            {
                ungetToken();
                dec->var_decl_section = NULL;
                return dec;
            }
            else
                syntax_error("decl. LBRACE expected");
        }
    }
    return NULL; // control never reaches here, this is just for the sake of GCC
}

struct programNode* program()
{
    struct programNode* prog;

    prog = ALLOC(struct programNode);
    ttype = getToken();
    if (ttype == TYPE || ttype == VAR || ttype == LBRACE)
    {
        ungetToken();
        prog->decl = decl();
        prog->body = body();
        return prog;
    }
    else
        syntax_error("program. TYPE or VAR or LBRACE expected");
    return NULL; // control never reaches here, this is just for the sake of GCC
}

static char* type_string(struct type_nameNode* type_struct)
{
    switch (type_struct->type)
    {
        case INT:
            return "INT";
        case REAL:
            return "REAL";
        case STRING:
            return "STRING";
        case BOOLEAN:
            return "BOOLEAN";
        case ID:
            return type_struct->id;
        case LONG:
            return "LONG";
    }
}

int unify(int first, int second)
{
    if((first < 15) && (second < 15)) // both built-in
    {
        if (first == second)
        {
            return first;
        }
        else
            return -1;
    }
    else if((first > 14) && (second > 14)) // both user-defined
    {
        return first;
    }
    else
    {
        if(first < 15)
        {
            return first;
        }
        else if(second < 15)
        {
            return second;
        }
    }
}

int process_prim(struct primaryNode* prim)
{
    if (prim->tag == ID)
        {
            // Find what type it is
            char* current_id = prim->id;
                // find type in symbol table
                int i = 0;
                while (i < (5 + new_types))
                {
                    //printf("2 ids: %s %s\n", type_string(current_type_name), table[i]->id);                
                    if (strcmp(current_id, table[i]->id) == 0)
                    {
                        if (table[i]->flag == 0)
                        {
                            printf("ERROR CODE 1\n");
                            return -1;
                        }
                        else
                        {
                            return table[i]->type_number;
                            
                        }
                    }
                    ++i;
                }
                // If control reaches here, type was not found
                    ++new_types;                
                    table[4+new_types] = ALLOC(struct symbol);
                    table[4+new_types]->id = (char*)malloc(strlen(current_id) * sizeof(char));
                    strcpy(table[4+new_types]->id, current_id);
                    table[4+new_types]->type_number = (14+new_types);
                    table[4+new_types]->where = 2;
                    table[4+new_types]->flag = 1;
                    return (14+new_types); 
                
        }
        else if (prim->tag == NUM)
            return 10; // INT
        else if (prim->tag == REALNUM)
            return 11; // REAL
}

int process_expr(struct exprNode* expr)
{
    if (expr->tag == EXPR)
    {
        int lhs_number, rhs_number, result;
        lhs_number = process_expr(expr->leftOperand);
        rhs_number = process_expr(expr->rightOperand);
        if ((lhs_number < 0) || (rhs_number < 0))
        {
            return -1;
        }
        result = unify(lhs_number, rhs_number);
        if (result < 0)
        {
             printf("ERROR CODE 3\n");
        }
        return result;
    }
    else if (expr->tag == PRIMARY)
    {
        return process_prim(expr->primary);
    }
}

int process_assign(struct assign_stmtNode* assign_stmt)
{
    char* current_id = assign_stmt->id;
                // find type in symbol table
                int index = 0;
                int i = 0;
                int isFound = 0;
                int lhs_number, rhs_number, result;
                while ((i < (5 + new_types)) && (isFound == 0))
                {
                    //printf("2 ids: %s %s\n", type_string(current_type_name), table[i]->id);                
                    if (strcmp(current_id, table[i]->id) == 0)
                    {
                        if (table[i]->flag == 0)
                        {
                            printf("ERROR CODE 1\n");
                            return -1;
                        }
                        else
                        {
                            lhs_number = table[i]->type_number;
                            isFound = 1;
                            index = i;
                        }
                    }
                    ++i;
                }
                if(isFound == 0)
                {
                    ++new_types;                
                    table[4+new_types] = ALLOC(struct symbol);
                    table[4+new_types]->id = (char*)malloc(strlen(current_id) * sizeof(char));
                    strcpy(table[4+new_types]->id, current_id);
                    table[4+new_types]->type_number = (14+new_types);
                    table[4+new_types]->where = 2;
                    table[4+new_types]->flag = 1;
                    lhs_number = table[4+new_types]->type_number;
                    index = 4+new_types;
                }
                
                // Check the RHS
                rhs_number = process_expr(assign_stmt->expr);
     if (rhs_number > 0)
    {
                result = unify(lhs_number, rhs_number);
                
                if (result < 0)
                {
                    printf("ERROR CODE 3\n");
                }
                else
                {
                    table[index]->type_number = result;
                }
           return result; 
    }
     return rhs_number;
}

int process_while(struct while_stmtNode* whil)
{
    int body_number;
    body_number = process_body(whil->body);
    if (body_number < 0)
    {
        printf("ERROR CODE 3\n");
        return -1;
    }
    if (whil->condition->relop != NOOP)
    {
        int lhs_number, rhs_number, result;
        lhs_number = process_prim(whil->condition->left_operand);
        rhs_number = process_prim(whil->condition->right_operand);
        result = unify(lhs_number, rhs_number);
        if (result < 0)
        {
               printf("ERROR CODE 3\n");
               return -1;
        }
        else
        {
            return 13; // boolean
        }
    }
    else
    {
        return (process_prim(whil->condition->left_operand) == 13) ? 13 : -1;
    }
}

int process_switch(struct switch_stmtNode* swi)
{
    char* current_id = swi->id;
                //Ensure id is INT
               int i = 0;
               int isFound = 0;
                
                while ((i < (5 + new_types)) && (isFound == 0))
                {
                    //printf("2 ids: %s %s\n", type_string(current_type_name), table[i]->id);                
                    if (strcmp(current_id, table[i]->id) == 0)
                    {
                        if (table[i]->type_number != 10)
                        {
                            printf("ERROR CODE 3\n");
                            return -1;
                        }
                        else
                        {
                            isFound = 1;
                        }
                    }
                    ++i;
                }
                if(isFound == 0)
                {
                    ++new_types;                
                    table[4+new_types] = ALLOC(struct symbol);
                    table[4+new_types]->id = (char*)malloc(strlen(current_id) * sizeof(char));
                    strcpy(table[4+new_types]->id, current_id);
                    table[4+new_types]->type_number = 10;
                    table[4+new_types]->where = 2;
                    table[4+new_types]->flag = 1;
                }

                //on to the case list
       struct case_listNode* case_list = swi->case_list;
    int case_type;
    while(case_list != NULL)
    {
        struct caseNode* current_case = case_list->cas;
        case_type = process_body(current_case->body);
        if (case_type < 0)
        {
            return case_type;
        }   
        case_list = case_list->case_list;
    }
    return case_type;
}

int process_body(struct bodyNode* body)
{
    int i = 0;
    int isFound = 0;
    int overall_type = 0;

    struct stmt_listNode* program_stmt = body->stmt_list;
    while(program_stmt != NULL)
    {
        struct stmtNode* current_stmt = program_stmt->stmt;
        switch (current_stmt->stmtType)
        {
            case ASSIGN:
                overall_type = process_assign(current_stmt->assign_stmt);
                break;
            case WHILE:
                overall_type = process_while(current_stmt->while_stmt);
                break;
            case DO:
                overall_type = process_while(current_stmt->while_stmt);
                break;
            case SWITCH:
                overall_type = process_switch(current_stmt->switch_stmt);
                break;
        } 
        if (overall_type < 0)
        {
            return overall_type;
        }   
        program_stmt = program_stmt->stmt_list;
    }
    return overall_type;
} 

int main()
{
    struct programNode* parseTree;
    parseTree = program();
    // TODO: remove the next line after you complete the parser
    // This is just for debugging purposes
    //print_parse_tree(parseTree);
    // TODO: do type checking & print output according to project specification

    // Create built-in types
    new_types = 0;
    struct symbol* int_type = ALLOC(struct symbol);
    int_type->id = "INT";
    int_type->type_number = 10;
    int_type->where = 0;
    int_type->flag = 0;
    table[0] = int_type;
    
    struct symbol* real_type = ALLOC(struct symbol);
    real_type->id = "REAL";
    real_type->type_number = 11;
    real_type->where = 0;
    real_type->flag = 0;
    table[1] = real_type;

    struct symbol* boolean_type = ALLOC(struct symbol);
    boolean_type->id = "BOOLEAN";
    boolean_type->type_number = 12;
    boolean_type->where = 0;
    boolean_type->flag = 0;
    table[2] = boolean_type;

    struct symbol* string_type = ALLOC(struct symbol);
    string_type->id = "STRING";
    string_type->type_number = 13;
    string_type->where = 0;
    string_type->flag = 0;
    table[3] = string_type;

    struct symbol* long_type = ALLOC(struct symbol);
    long_type->id = "LONG";
    long_type->type_number = 14;
    long_type->where = 0;
    long_type->flag = 0;
    table[4] = long_type;

    // Check if TYPE section exists
    if (parseTree->decl->type_decl_section != NULL)
    {
        // Go through each id_list in the TYPE section
        struct type_decl_listNode* currentTypeList = parseTree->decl->type_decl_section->type_decl_list;
        while(currentTypeList != NULL)
        {
            struct id_listNode* current_id_list = currentTypeList->type_decl->id_list;
            struct type_nameNode* current_type_name = currentTypeList->type_decl->type_name;
            // Check to see if RHS is an implicit type
            int i = 0;
            int isFound = 0;
            int rhs_number = -1;
            
            while ((i < (5 + new_types)) && (isFound == 0))
            {
                if (strcmp(type_string(current_type_name), table[i]->id) == 0)
                {
                    rhs_number = table[i]->type_number;
                    isFound = 1;
                }
                ++i;
            }
            if(isFound == 0)
            {
                ++new_types;                
                table[4+new_types] = ALLOC(struct symbol);
                table[4+new_types]->id = (char*)malloc(strlen(current_type_name->id) * sizeof(char));
                strcpy(table[4+new_types]->id, current_type_name->id);
                table[4+new_types]->type_number = (14+new_types);
                table[4+new_types]->where = 2;
                table[4+new_types]->flag = 0;
                rhs_number = 14+new_types;
            }           

            while(current_id_list != NULL)
            {
                // Check to see if id is already in table
                int i;
                for (i = 0; i < (5 + new_types); i++)
                {
                    if (strcmp(current_id_list->id, table[i]->id) == 0)
                    {
                        printf("ERROR CODE 0\n");
                        return 1;
                    }
                }
                
                ++new_types;                
                table[4+new_types] = ALLOC(struct symbol);
                table[4+new_types]->id = (char*)malloc(strlen(current_id_list->id) * sizeof(char));
                strcpy(table[4+new_types]->id, current_id_list->id);
                table[4+new_types]->type_number = rhs_number;
                table[4+new_types]->where = 1;
                table[4+new_types]->flag = 0;                 
                current_id_list = current_id_list->id_list;
            }
            currentTypeList = currentTypeList->type_decl_list;
        } 
    }

    // Check if VAR section exists
    if (parseTree->decl->var_decl_section != NULL)
    {
        // Go through each id_list in the VAR section
        struct var_decl_listNode* currentVarList = parseTree->decl->var_decl_section->var_decl_list;
        while(currentVarList != NULL)
        {
            struct id_listNode* current_id_list = currentVarList->var_decl->id_list;
            struct type_nameNode* current_type_name = currentVarList->var_decl->type_name;
            // Check to see if RHS is an implicit type
            int i = 0;
            int isFound = 0;
            int rhs_number = -1;
            
            while ((i < (5 + new_types)) && (isFound == 0))
            {
                //printf("2 ids: %s %s\n", type_string(current_type_name), table[i]->id);                
                if (strcmp(type_string(current_type_name), table[i]->id) == 0)
                {
                    if (table[i]->flag == 1)
                    {
                        printf("ERROR CODE 4\n");
                        return 1;
                    }
                    else
                    {
                        rhs_number = table[i]->type_number;
                        isFound = 1;
                    }
                }
                ++i;
            }
            if(isFound == 0)
            {
                ++new_types;                
                table[4+new_types] = ALLOC(struct symbol);
                table[4+new_types]->id = (char*)malloc(strlen(current_type_name->id) * sizeof(char));
                strcpy(table[4+new_types]->id, current_type_name->id);
                table[4+new_types]->type_number = (14+new_types);
                table[4+new_types]->where = 2;
                table[4+new_types]->flag = 0;
                rhs_number = 14+new_types;
            }           

            while(current_id_list != NULL)
            {
                // Check to see if id is already in table
                int i;
                for (i = 0; i < (5 + new_types); i++)
                {   
                    if (strcmp(current_id_list->id, table[i]->id) == 0)
                    {
                        if (table[i]->flag == 1)
                        {                      
                            printf("ERROR CODE 2\n");
                            return 1;
                        }
                        else if (table[i]->flag == 0)
                        {
                            printf("ERROR CODE 1\n");
                            return 1;
                        }
                    }
                }
                
                ++new_types;                
                table[4+new_types] = ALLOC(struct symbol);
                table[4+new_types]->id = (char*)malloc(strlen(current_id_list->id) * sizeof(char));
                strcpy(table[4+new_types]->id, current_id_list->id);
                table[4+new_types]->type_number = rhs_number;
                table[4+new_types]->where = 1;
                table[4+new_types]->flag = 1;                 
                current_id_list = current_id_list->id_list;
            }
            currentVarList = currentVarList->var_decl_list;
        } 
    }

    // Parse the statements inside the body
    int result = process_body(parseTree->body);
    if (result < 0)
    {
        return 1;
    }

    // If there are no semantic errors, print out the types
    int type_array[5 + new_types];
    int i, j, k;
    for (i = 0; i < (5 + new_types); i++)
    {
        type_array[i] = 0;
    }
    for (i = 0; i < (5 + new_types); i++)
    { 
        int count = 0;        
        for (k = i; k < (5 + new_types); k++)
        {
            if (table[k]->type_number == table[i]->type_number)
            {
                ++count;
            }
        }
        if (count < 2)
        {
            type_array[i] = 1;
        }
        if(type_array[i] == 0)
        {
            printf("%s :", table[i]->id);
            type_array[i] = 1;
            // Print out types first            
            for (j = i; j < (5 + new_types); j++)
            {
                 if((type_array[j] == 0) && (table[j]->flag == 0) && (table[j]->type_number == table[i]->type_number))
                 {
                    printf(" %s", table[j]->id);
                    type_array[j] = 1;
                 }
            }

            // Print out vars after          
            for (j = i; j < (5 + new_types); j++)
            {
                 if((type_array[j] == 0) && (table[j]->flag == 1) && (table[j]->type_number == table[i]->type_number))
                 {
                    printf(" %s", table[j]->id);
                    type_array[j] = 1;
                 }
            }
            printf(" #\n");
        }
    }
    return 0;
}
