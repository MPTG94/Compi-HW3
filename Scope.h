//
// Created by 1912m on 12/05/2021.
//

#ifndef HW3_SCOPE_H
#define HW3_SCOPE_H

#include <memory>
#include "vector"
#include <string>
#include <utility>
#include "hw3_output.hpp"

extern int yylineno;
using namespace std;

void enterLoop();
void exitLoop();

void exitProgramFuncs();

void exitProgramRuntime();

void openNewScope();

void closeCurrentScope();

bool isDeclared(const string &name);

// Single row in the table of a scope
class SymbolTableRow {
public:
    string name;
    // This is for variables and function definitions
    // For a variable, the type at cell 0 is the actual type, other cells should be empty
    // For a function, all types except the last one, are parameter types, the last type is the return type of the function
    vector<string> type;
    int offset;
    bool isFunc;

    SymbolTableRow(string name, vector<string> type, int offset, bool isFunc);
};

// The object storing the entries of the current scope
class SymbolTable {
public:
    vector<shared_ptr<SymbolTableRow>> rows;

    SymbolTable() = default;
};

class Scope {
    vector<shared_ptr<SymbolTable>> tables;

    Scope() = default;
};

class TypeNode {
public:
    string value;

    explicit TypeNode(string str);

    TypeNode();

    ~TypeNode() = default;
};

#define YYSTYPE TypeNode*

class Type : public TypeNode {
public:
    explicit Type(TypeNode *type);
};

class Call;

class Exp : public TypeNode {
public:
    // Type is used for tagging in bison when creating the Exp object
    string type;
    bool valueAsBooleanValue;

    // This is for NUM, NUM B, STRING, TRUE and FALSE
    Exp(TypeNode *terminal, string taggedTypeFromParser);

    // for Call
    explicit Exp(Call *call);

    // for NOT Exp
    Exp(TypeNode *notNode, Exp *exp);

    // for Exp RELOP, MUL, DIV, ADD, SUB, OR, AND Exp
    Exp(Exp *e1, TypeNode *op, Exp *e2, const string &taggedTypeFromParser);

    // for Exp ID
    explicit Exp(TypeNode *id);

    // for Lparen Exp Rparen, need to just remove the parentheses
    Exp(Exp *ex);
};

class ExpList : public TypeNode {
public:
    vector<Exp> list;

    explicit ExpList(Exp *exp);

    ExpList(Exp *exp, ExpList *expList);
};

class Call : public TypeNode {
public:
    Call(TypeNode *id, ExpList *list);

    explicit Call(TypeNode *id);
};

class RetType : public TypeNode {
    explicit RetType(TypeNode *type);
};

class Statements;

class CaseList;


class Statement : public TypeNode {
public:
    string dataTag;

    // For Lbrace Statements Rbrace
    explicit Statement(Statement *states);

    // For Type ID SC
    Statement(Type *t, TypeNode *id);

    // For Type ID Assign Exp SC
    Statement(Type *t, TypeNode *id, Exp *exp);

    // For ID Assign Exp SC
    Statement(TypeNode *id, Exp *exp);

    // For Call SC
    explicit Statement(Call *call);

    // For Return SC -> this is for a function with a void return type
    explicit Statement(RetType *ret);

    // For Return Exp SC -> This is for a non-void function, exp stores the type so it is enough
    explicit Statement(Exp *exp);

    // For if,if/else,while
    Statement(string type, Exp *exp);

    // For break,continue
    explicit Statement(TypeNode *type);

    // For Switch LParen Exp RParen Lbrace CaseList Rbrace
    Statement(Exp *exp, CaseList *cList);
};

class Statements : public TypeNode {
public:
    // For Statement
    explicit Statements(Statement *state);

    // For Statements Statement
    Statements(Statements *states, Statement *state);
};

class CaseDecl : public TypeNode {
public:
    // For Case Num Colon Statements
    CaseDecl(TypeNode *num, Statements *states);
};

class CaseList : public TypeNode {
public:
    vector<CaseDecl *> cases;

    // For CaseDecl CaseList
    CaseList(CaseList *cList, CaseDecl *cDec);

    // For CaseDecl
    explicit CaseList(CaseDecl *cDec);

    // For Default Colon Statements
    explicit CaseList(Statements *states);
};

class FormalDecl : public TypeNode {
public:
    // The parameter type
    string type;

    // for Type ID
    FormalDecl(Type *t, TypeNode *id);
};

class FormalsList : public TypeNode {
public:
    vector<FormalDecl *> formals;

    // To initialize from an empty formal list
    explicit FormalsList(FormalDecl *formal);

    // To append a new formal to an existing formal list
    FormalsList(FormalsList *fList, FormalDecl *formal);
};

class Formals : public TypeNode {
public:
    vector<FormalDecl *> formals;

    // for Epsilon
    Formals();

    // for formalList
    explicit Formals(FormalsList *formList);
};

class FuncDecl : public TypeNode {
public:
    // This is an array to denote the types of the func parameters, with the func return type being the last elemtn of the array
    vector<string> type;

    FuncDecl(RetType *rType, TypeNode *id, Formals *funcParams);
};

class Funcs : public TypeNode {
public:
    Funcs() = default;
};

class Program : public TypeNode {
public:
    Program();
};

#endif //HW3_SCOPE_H
