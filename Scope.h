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

// TODO: statement, caselist, casedecl

extern int yylineno;
using namespace std;

// Single row in the table of a scope
class SymbolTableRow {
public:
    string name;
    string type;
    int offset;

    SymbolTableRow(string name, string type, int offset);
};

// The object storing the entries of the current scope
class SymbolTable {
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

    ~TypeNode() = default;
};

#define YYSTYPE TypeNode*

class Type : public TypeNode {
public:
    Type(TypeNode *type) : TypeNode(type->value) {};
};

class Call;

class Exp : public TypeNode {
    // Type is used for tagging in bison when creating the Exp object
    string type;
    bool valueAsBooleanValue;

    // This is for NUM, NUM B, STRING, TRUE and FALSE
    Exp(TypeNode *terminal, string type);

    // for Call
    Exp(Call *call);

    // for NOT Exp
    Exp(TypeNode *notNode, Exp *exp);

    // for Exp RELOP, MUL, DIV, ADD, SUB, OR, AND Exp
    Exp(Exp *e1, TypeNode *op, Exp *e2, string type);

    // for Exp ID
    Exp(TypeNode *id);

    // for Lparen Exp Rparen, need to just remove the parentheses
    Exp(Exp *ex, string type);
};

class ExpList : public TypeNode {
public:
    vector<Exp> list;

    ExpList(Exp *exp);

    ExpList(Exp *exp, ExpList *expList);
};

class Call : public TypeNode {
public:
    Call(TypeNode *id, ExpList *list);

    Call(TypeNode *id);
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
    FormalsList(FormalDecl *formal);

    // To append a new formal to an existing formal list
    FormalsList(FormalsList *fList, FormalDecl *formal);
};

class Formals : public TypeNode {
public:
    vector<FormalDecl *> formals;

    // for Epsilon
    Formals();

    // for formalList
    Formals(FormalsList *formList);
};

class RetType : public TypeNode {
    RetType(TypeNode *type);
};

class FuncDecl : public TypeNode {
public:
    string retType;

    FuncDecl(RetType *rType, TypeNode *id, Formals *params);
};

class Funcs : public TypeNode {
public:
    Funcs(string str) : TypeNode(std::move(str)) {};
};

class Program : public TypeNode {
public:
    Program();
};

#endif //HW3_SCOPE_H
