#ifndef __SYMTABLE_H__
#define __SYMTABLE_H__

#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>

//Added C libraries
#include <stdio.h>
#include <stdlib.h>

//Added C++ libraries
#include <stack>


using namespace std;

enum { ATTR_void, ATTR_bool, ATTR_char, ATTR_int, ATTR_null,
  ATTR_string, ATTR_struct, ATTR_array, ATTR_function,
  ATTR_variable, ATTR_field, ATTR_typeid, ATTR_param,
  ATTR_lval, ATTR_const, ATTR_vreg, ATTR_vaddr,
  ATTR_bitset_size,
};
using attr_bitset = bitset<ATTR_bitset_size>;

struct symbol;
using symbol_table = unordered_map<string*,symbol*>;
using symbol_entry = pair<string*,symbol*>;

struct symbol {
  attr_bitset attributes;
  symbol_table* fields;
  size_t filenr, linenr, offset;
  size_t blocknr;
  string *struct_name;
  vector<symbol*>* parameters;
};

void build_sym(FILE* out, astree* root);

#endif
