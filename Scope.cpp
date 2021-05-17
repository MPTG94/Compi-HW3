//
// Created by 1912m on 12/05/2021.
//

#include "Scope.h"

#include <memory>
#include <utility>

vector<shared_ptr<SymbolTable>> symTabStack;
vector<int> offsetStack;
vector<string> varTypes = {"VOID", "INT", "BYTE", "BOOL", "STRING"};

string currentRunningFunctionScopeId;

int loopCounter = 0;

void enterLoop() {
    loopCounter++;
}

void exitLoop() {
    loopCounter--;
}

void exitProgramFuncs() {
    currentRunningFunctionScopeId = "";
}

void exitProgramRuntime() {
    shared_ptr<SymbolTable> globalScope = symTabStack.front();
    bool mainFunc = false;
    for (auto &row : globalScope->rows) {
        if (row->isFunc && row->name == "main" && row->type.back() == "VOID") {
            mainFunc = true;
        }
    }
    if (!mainFunc) {
        output::errorMainMissing();
        exit(0);
    }
    closeCurrentScope();
}

void openNewScope() {
    shared_ptr<SymbolTable> nScope = make_shared<SymbolTable>();
    symTabStack.push_back(nScope);
    offsetStack.push_back(offsetStack.back());
}

void closeCurrentScope() {
    output::endScope();
    shared_ptr<SymbolTable> currentScope = symTabStack.back();
    for (auto &row : currentScope->rows) {
        if (!row->isFunc) {
            // Print a normal variable
            output::printID(row->name, row->offset, row->type[0]);
        } else {
            string funcReturnType = row->type.back();
            // Taking out the return type from the vector for easy printing
            row->type.pop_back();
            output::printID(row->name, row->offset,
                            output::makeFunctionType(funcReturnType, row->type));
        }
    }

    currentScope->rows.clear();
    symTabStack.pop_back();
    offsetStack.pop_back();

}

bool isDeclared(const string &name) {
    for (int i = symTabStack.size() - 1; i >= 0; ++i) {
        for (auto &row : symTabStack[i]->rows) {
            if (row->name == name) {
                return true;
            }
        }
    }
    return false;
}

SymbolTableRow::SymbolTableRow(string name, vector<string> type, int offset, bool isFunc) : name(std::move(name)), type(std::move(type)),
                                                                                            offset(offset), isFunc(isFunc) {

}

TypeNode::TypeNode(string str) : value() {
    if (str == "void") {
        value = "VOID";
    } else if (str == "bool") {
        value = "BOOL";
    } else if (str == "int") {
        value = "INT";
    } else if (str == "byte") {
        value = "BYTE";
    } else {
        value = str;
    }

}

TypeNode::TypeNode() = default;

Program::Program() : TypeNode("Program") {
    shared_ptr<SymbolTable> symTab = std::make_shared<SymbolTable>();
    shared_ptr<SymbolTableRow> printFunc = std::make_shared<SymbolTableRow>(SymbolTableRow("print", {"STRING", "VOID"}, 0, true));
    shared_ptr<SymbolTableRow> printiFunc = std::make_shared<SymbolTableRow>(SymbolTableRow("printi", {"INT", "VOID"}, 1, true));
    // Placing the print and printi function at the bottom of the global symbol table
    symTab->rows.push_back(printFunc);
    symTab->rows.push_back(printiFunc);
    // Placing the global symbol table at the bottom of the global symbol table stack
    symTabStack.push_back(symTab);
    // Placing the global symbol table at the bottom of the offset stack
    offsetStack.push_back(0);
}

RetType::RetType(TypeNode *type) : TypeNode(type->value) {

}

Type::Type(TypeNode *type) : TypeNode(type->value) {

}

FormalDecl::FormalDecl(Type *t, TypeNode *id) : TypeNode(id->value), type(t->value) {

}

FormalsList::FormalsList(FormalDecl *formal) {
    formals.push_back(formal);
}

FormalsList::FormalsList(FormalsList *fList, FormalDecl *formal) {
    formals = vector<FormalDecl *>(fList->formals);
    formals.push_back(formal);
}

Formals::Formals() = default;

Formals::Formals(FormalsList *formList) {
    formals = vector<FormalDecl *>(formList->formals);
}

FuncDecl::FuncDecl(RetType *rType, TypeNode *id, Formals *funcParams) {
    if (isDeclared(id->value)) {
        // Trying to redeclare a name that is already used for a different variable/fucntion
        output::errorDef(yylineno, id->value);
        exit(0);
    }

    for (int i = 0; i < funcParams->formals.size(); ++i) {
        if (isDeclared(funcParams->formals[i]->value) || funcParams->formals[i]->value == id->value) {
            // Trying to shadow inside the function a variable that was already declared
            // Or trying to name a function with the same name as one of the function parameters
            output::errorDef(yylineno, id->value);
            exit(0);
        }

        for (int j = i + 1; j < funcParams->formals.size(); ++j) {
            if (funcParams->formals[i]->value == funcParams->formals[j]->value) {
                // Trying to declare a function where 2 parameters or more have the same name
                output::errorDef(yylineno, id->value);
                exit(0);
            }
        }
    }

    // This is the name of the newly declared function
    value = id->value;
    if (funcParams->formals.size() != 0) {
        // Saving the types of all the different function parameters
        for (auto &formal : funcParams->formals) {
            type.push_back(formal->type);
        }
    } else {
        // The func has no input parameters, no need to add a specific parameter type
    }
    // Saving the return type of the function
    type.push_back(rType->value);

    // Adding the new function to the symTab
    shared_ptr<SymbolTableRow> nFunc = std::make_shared<SymbolTableRow>(value, type, 0, true);
    symTabStack.back()->rows.push_back(nFunc);
    currentRunningFunctionScopeId = value;
}

Call::Call(TypeNode *id) {
    for (auto &symTab : symTabStack) {
        for (auto &row : symTab->rows) {
            if (row->name == id->value) {
                if (!row->isFunc) {
                    // We found a declaration of a variable with the same name, illegal
                    output::errorUndefFunc(yylineno, id->value);
                    exit(0);
                } else if (row->isFunc && row->type.size() == 1) {
                    // We found the right function, has the same name, it really is a function, and receives no parameters
                    // Saving the type of the function call return value
                    value = row->type.back();
                    return;
                } else {
                    //
                    vector<string> desiredParams = {""};
                    output::errorPrototypeMismatch(yylineno, id->value, desiredParams);
                    exit(0);
                }
            }
        }
    }
    // We didn't find a declaration of the desired function
    output::errorUndefFunc(yylineno, id->value);
    exit(0);
}

Call::Call(TypeNode *id, ExpList *list) {
    for (auto &symTab : symTabStack) {
        for (auto &row : symTab->rows) {
            if (row->name == id->value) {
                if (!row->isFunc) {
                    // We found a declaration of a variable with the same name, illegal
                    output::errorUndefFunc(yylineno, id->value);
                    exit(0);
                } else if (row->isFunc && row->type.size() + 1 == list->list.size()) {
                    // We found the right function, has the same name, it really is a function
                    // Now we need to check that the parameter types are correct between what the function accepts, and what was sent
                    for (int i = 0; i < list->list.size(); ++i) {
                        if (list->list[i].type == row->type[i]) {
                            // This parameter is of matching type so it is ok
                            continue;
                        } else if (list->list[i].type == "BYTE" && row->type[i] == "INT") {
                            // The function receives int as a paramter, in this instance a byte was sent, but it is ok to cast from BYTE to INT
                            continue;
                        }
                        // Removing the return type of the function so we have an easy list of requested parameters to print
                        row->type.pop_back();
                        output::errorPrototypeMismatch(yylineno, id->value, row->type);
                        exit(0);
                    }
                    // Saving the type of the function call return value
                    value = row->type.back();
                    return;
                } else {
                    // The number of parameters we received does not match the number the function takes as arguments
                    // Removing the return type of the function so we have an easy list of requested parameters to print
                    row->type.pop_back();
                    output::errorPrototypeMismatch(yylineno, id->value, row->type);
                    exit(0);
                }
            }
        }
    }
    // We didn't find a declaration of the desired function
    output::errorUndefFunc(yylineno, id->value);
    exit(0);
}

Exp::Exp(Call *call) {
    // Need to just take the return value of the function and use it as the return type of the expression
    value = call->value;
}

Exp::Exp(TypeNode *id) {
    // Need to make sure that the variable/func we want to use is declared
    if (!isDeclared(id->value)) {
        output::errorUndef(yylineno, id->value);
        exit(0);
    }

    // Need to save the type of the variable function as the type of the expression
    for (int i = symTabStack.size() - 1; i >= 0; ++i) {
        for (auto &row : symTabStack[i]->rows) {
            if (row->name == id->value) {
                // We found the variable/func we wanted to use in the expression
                value = id->value;
                // Getting the type of the variable, or the return type of the function
                type = row->type.back();
                return;
            }
        }
    }
}

Exp::Exp(TypeNode *notNode, Exp *exp) {
    if (exp->type != "BOOL") {
        // This is not a boolean expression, can't apply NOT
        output::errorMismatch(yylineno);
        exit(0);
    }
    type = "BOOL";
    valueAsBooleanValue = !valueAsBooleanValue;
}

Exp::Exp(TypeNode *terminal, string taggedTypeFromParser) : TypeNode(terminal->value) {
    type = std::move(taggedTypeFromParser);
    if (type == "BYTE") {
        // Need to check that BYTE size is legal
        if (stoi(terminal->value) > 255) {
            // Byte is too large
            output::errorByteTooLarge(yylineno, terminal->value);
            exit(0);
        }
    }
    if (type == "BOOL") {
        if (terminal->value == "true") {
            valueAsBooleanValue = true;
        } else {
            valueAsBooleanValue = false;
        }
    }

}

Exp::Exp(Exp *ex) {
    if (ex->type != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    value = ex->value;
    type = ex->type;
    valueAsBooleanValue = ex->valueAsBooleanValue;
}

// for Exp RELOP, MUL, DIV, ADD, SUB, OR, AND Exp
Exp::Exp(Exp *e1, TypeNode *op, Exp *e2, const string &taggedTypeFromParser) {
    // Need to check the type of the expressions on each side, to make sure that a logical operator (AND/OR) is used on a boolean type
    if ((e1->type == "INT" || e1->type == "BYTE") && (e2->type == "INT" || e2->type == "BYTE")) {
        if (taggedTypeFromParser == "EQ_NEQ_RELOP" || taggedTypeFromParser == "REL_RELOP") {
            // This is a boolean operation performed between 2 numbers (>=,<=,>,<,!=,==)
            type = "BOOL";
        } else if (taggedTypeFromParser == "ADD_SUB_BINOP" || taggedTypeFromParser == "MUL_DIV_BINOP") {
            // This is an arithmetic operation between two numbers
            if (e1->type == "INT" || e2->type == "INT") {
                // An automatic cast to int will be performed in case one of the operands is of integer type
                type = "INT";
            } else {
                type = "BOOL";
            }
        }
    } else if (e1->type == "BOOL" && e2->type == "BOOL") {
        // Both operands are boolean so this should be a boolean operation
        type = "BOOL";
        if (taggedTypeFromParser == "AND" || taggedTypeFromParser == "OR") {
            if (op->value == "AND") {
                if (e1->valueAsBooleanValue && e2->valueAsBooleanValue) {
                    valueAsBooleanValue = true;
                } else {
                    valueAsBooleanValue = false;
                }
            } else if (op->value == "OR") {
                if (e1->valueAsBooleanValue || e2->valueAsBooleanValue) {
                    valueAsBooleanValue = true;
                } else {
                    valueAsBooleanValue = false;
                }
            }
        } else {
            output::errorMismatch(yylineno);
            exit(0);
        }
    } else {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

ExpList::ExpList(Exp *exp) {
    list.push_back(exp);
}

ExpList::ExpList(Exp *exp, ExpList *expList) {
    list = vector<Exp>(expList->list);
    list.push_back(exp);
}

Statement::Statement(TypeNode *type) {
    if (loopCounter == 0) {
        // We are not inside any loop, so a break or continue is illegal in this context
        if (type->value == "break") {
            output::errorUnexpectedBreak(yylineno);
            exit(0);
        } else {
            output::errorUnexpectedContinue(yylineno);
            exit(0);
        }
    }
    dataTag = "break or continue";
}

Statement::Statement(string type, Exp *exp) {
    if (exp->type != "BOOL") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    dataTag = "if if else while";
}

// For Return SC -> this is for a function with a void return type
Statement::Statement(RetType *ret) {
    // Need to check if the current running function is of void type
    for (int i = symTabStack.size() - 1; i >= 0; i--) {
        // Need to search in the current symtab with no particular order for the current function
        for (auto &row : symTabStack[i]->rows) {
            if (row->isFunc && row->name == currentRunningFunctionScopeId) {
                // We found the current running function
                if (row->type.back() == ret->value) {
                    dataTag = "void return value";
                } else {
                    output::errorMismatch(yylineno);
                    exit(0);
                }
            }
        }
    }
}

Statement::Statement(Exp *exp) {
    // Need to check if the current running function is of the specified type
    if (exp->type == "VOID") {
        // Attempt to return a void expression from a value returning function
        output::errorMismatch(yylineno);
        exit(0);
    }

    for (int i = symTabStack.size() - 1; i >= 0; i--) {
        // Need to search in the current symtab with no particular order for the current function
        for (auto &row : symTabStack[i]->rows) {
            if (row->isFunc && row->name == currentRunningFunctionScopeId) {
                // We found the current running function
                if (row->type.back() == exp->value) {
                    dataTag = exp->value;
                } else if (row->type.back() == "INT" && exp->value == "BYTE") {
                    // Allowing automatic cast from byte to int
                    dataTag = row->type.back();
                } else {
                    output::errorMismatch(yylineno);
                    exit(0);
                }
            }
        }
    }
}

Statement::Statement(Call *call) {
    dataTag = "function call";
}

Statement::Statement(TypeNode *id, Exp *exp) {
    if (!isDeclared(id->value)) {
        output::errorUndef(yylineno, id->value);
        exit(0);
    }

    // Searching for the variable in the symtab
    for (int i = symTabStack.size() - 1; i >= 0; i--) {
        // Need to search in the current symtab with no particular order for the current function
        for (auto &row : symTabStack[i]->rows) {
            if (!row->isFunc && row->name == id->value) {
                // We found the desired variable
                if (row->type.back() == exp->type || row->type.back() == "INT" && exp->type == "BYTE") {
                    dataTag = row->type.back();
                }
            }
        }
    }
}

Statement::Statement(Type *t, TypeNode *id, Exp *exp) {
    if (isDeclared(id->value)) {
        // Trying to redeclare a name that is already used for a different variable/fucntion
        output::errorDef(yylineno, id->value);
        exit(0);
    }

    if (t->value == exp->type || t->value == "INT" && exp->type == "BYTE") {
        dataTag = t->value;
        // Creating a new variable on the stack will cause the next one to have a higher offset
        int offset = offsetStack.back()++;
        vector<string> varType = {t->value};
        shared_ptr<SymbolTableRow> nVar = std::make_shared<SymbolTableRow>(id->value, varType, offset, false);
        symTabStack.back()->rows.push_back(nVar);
    } else {
        output::errorMismatch(yylineno);
        exit(0);
    }
}

Statement::Statement(Type *t, TypeNode *id) {
    if (isDeclared(id->value)) {
        // Trying to redeclare a name that is already used for a different variable/fucntion
        output::errorDef(yylineno, id->value);
        exit(0);
    }
    // Creating a new variable on the stack will cause the next one to have a higher offset
    int offset = offsetStack.back()++;
    vector<string> varType = {t->value};
    shared_ptr<SymbolTableRow> nVar = std::make_shared<SymbolTableRow>(id->value, varType, offset, false);
    symTabStack.back()->rows.push_back(nVar);
    dataTag = t->value;
}

Statement::Statement(Statement *states) {
    dataTag = "statement block";
}

Statement::Statement(Exp *exp, CaseList *cList) {
    // Need to check that exp is a number (int,byte) and that all case decl in caselist are int or byte
    if (exp->type != "INT" && exp->type != "BYTE") {
        output::errorMismatch(yylineno);
        exit(0);
    }

    for (auto &i : cList->cases) {
        if (i.value != "INT" && i.value != "BYTE") {
            output::errorMismatch(yylineno);
            exit(0);
        }
    }

    dataTag = "switch block";
}

Statements::Statements(Statement *state) {

}

Statements::Statements(Statements *states, Statement *state) {

}

CaseDecl::CaseDecl(TypeNode *num, Statements *states) {
    if (num->value != "INT" && num->value != "BYTE") {
        output::errorMismatch(yylineno);
        exit(0);
    }
    value = "case";
}

CaseList::CaseList(CaseList *cList, CaseDecl *cDec) {
    cases = vector<CaseDecl *>(cList->cases);
    cases.push_back(cDec);
    value = "case list";
}

CaseList::CaseList(CaseDecl *cDec) {
    cases.push_back(cDec);
    value = "case list";
}

CaseList::CaseList(Statements *states) {

}
