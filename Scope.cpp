//
// Created by 1912m on 12/05/2021.
//

#include "Scope.h"

#include <utility>

SymbolTableRow::SymbolTableRow(string name, string type, int offset) : name(std::move(name)), type(std::move(type)), offset(offset) {

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
