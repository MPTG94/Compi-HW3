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

static bool isDeclared(const string &name) {
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
