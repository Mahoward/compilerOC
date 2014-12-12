#include <bitset>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

//Added C libraries
#include <stdio.h>
#include <stdlib.h>

//Added Project libraries
#include "astree.h"
#include "lyutils.h"
#include "symtable.h"
#include "yyparse.h"
#include "auxlib.h"

symbol_table struct_table;
symbol_table global_table;
stack <symbol_table> sym_stack;
int depth = 0;

string *get_att_string(symbol* sym){
  string *attrs = new string;
  if(sym->attributes[ATTR_void])    {attrs->append("void ");}
  if(sym->attributes[ATTR_bool])    {attrs->append("bool ");}
  if(sym->attributes[ATTR_char])    {attrs->append("char ");}
  if(sym->attributes[ATTR_int])     {attrs->append("int ");}
  if(sym->attributes[ATTR_null])    {attrs->append("null ");}
  if(sym->attributes[ATTR_string])  {attrs->append("string ");}
  if(sym->attributes[ATTR_struct])  {attrs->append("struct ");
                                      attrs->append("\"");
                                      attrs->append(*sym->struct_name);
                                      attrs->append("\" ");}
  if(sym->attributes[ATTR_array])   {attrs->append("array ");}
  if(sym->attributes[ATTR_function]){attrs->append("function ");}
  if(sym->attributes[ATTR_variable]){attrs->append("variable ");}
  if(sym->attributes[ATTR_field])   {attrs->append("field ");}
  if(sym->attributes[ATTR_typeid])  {attrs->append("typeid ");}
  if(sym->attributes[ATTR_param])   {attrs->append("param ");}
  if(sym->attributes[ATTR_lval])    {attrs->append("lval ");}
  if(sym->attributes[ATTR_const])   {attrs->append("const ");}
  if(sym->attributes[ATTR_vreg])    {attrs->append("vreg ");}
  if(sym->attributes[ATTR_vaddr])   {attrs->append("vaddr ");}
  return attrs;
}

void print_fields(string *struct_name, symbol* struct_sym){
  vector<string*> keys;
  keys.reserve(struct_sym->fields->size());

  vector<symbol*> syms;
  syms.reserve(struct_sym->fields->size());

  for(auto kv : *struct_sym->fields) {
    keys.push_back(kv.first);
    syms.push_back(kv.second);
  }
  /*
  printf("%s (%ld.%ld.%ld) {%ld} %s\n",
  key->c_str(), struct_sym->filenr,
  struct_sym->linenr, struct_sym->offset,
  struct_sym->blocknr, attp->c_str());
  */
}

void print_struct(string *key, symbol* struct_sym){
  string *attp = get_att_string(struct_sym);
  printf("%s (%ld.%ld.%ld) {%ld} %s\n",
  key->c_str(), struct_sym->filenr,
  struct_sym->linenr, struct_sym->offset,
  struct_sym->blocknr, attp->c_str());
  print_fields(key, struct_sym);
}
int var_type(astree* node){
  switch(node->symbol){
    case TOK_BOOL:
      return ATTR_bool;
      break;
    case TOK_CHAR:
      return ATTR_char;
      break;
    case TOK_INT:
      return ATTR_int;
      break;
    case TOK_STRING:
      return ATTR_string;
      break;
    case TOK_IDENT:
      return ATTR_struct;
      break;
    default:
      printf("VAR TYPE ERROR\n");
      return -1;
  }
}


void populate_fields(astree* root, symbol_table& fields){
  for(size_t i = 0; i < root->children.size(); i++){
    if(root->children[i]->symbol != TOK_TYPEID){
      string *key = NULL;
      for(size_t q = 0; q < root->children[i]->children.size(); q++){
        if(root->children[i]->children[q]->symbol == TOK_FIELD){
          symbol* sym = new symbol();
          sym->struct_name = new string;
          int attr = var_type(root->children[i]);
          if(attr == ATTR_struct){sym->struct_name->append(*root->children[i]->lexinfo);}
          sym->attributes.set(attr);
          sym->attributes.set(ATTR_field);
          key = (string *)root->children[i]->children[q]->lexinfo;
          //printf("%s\n", key->c_str());
          sym->filenr = root->children[i]->children[q]->filenr;
          sym->linenr = root->children[i]->children[q]->linenr;
          sym->offset = root->children[i]->children[q]->offset;
          fields.insert({key, sym});
          break;
        }
      }
    }
  }
  /* Debug code
  vector<string*> keys;
  keys.reserve(fields.size());

  for(auto kv : fields) {
    keys.push_back(kv.first);
  }
  printf("Keys of Fields\n");
  for (size_t w=0; w < keys.size(); w++){
    printf("%s\n", keys[w]->c_str());
  }
  */
}

string *populate_struct_sym(astree* root, symbol* struct_sym){
  string *key = NULL;
  for(size_t i = 0; i < root->children.size(); i++){
      if(root->children[i]->symbol == TOK_TYPEID){
        key = (string *)root->children[i]->lexinfo;
        struct_sym->struct_name->append(*root->children[i]->lexinfo);
        struct_sym->filenr = root->children[i]->filenr;
        struct_sym->linenr = root->children[i]->linenr;
        struct_sym->offset = root->children[i]->offset;
        struct_sym->blocknr = 0;
        return key;
        break;
      }
    }
  return key;
}

void insert_struct(astree* root){
  symbol* struct_sym = new symbol();
  struct_sym->struct_name = new string;
  struct_sym->attributes.set(ATTR_struct);
  struct_sym->fields = new symbol_table();
  populate_fields(root, *struct_sym->fields);
  string* key = populate_struct_sym(root, struct_sym);
  if(key == NULL){
    printf("KEY ERROR\n");
  }else{
    //printf("Key:%s\n", key->c_str());
    //printf("struct_sym: (%ld.%ld.%ld) {%ld}\n",
            //struct_sym->filenr, struct_sym->linenr,
            //struct_sym->offset, struct_sym->blocknr);
    //struct_table.insert({key, struct_sym});
    print_struct(key, struct_sym);
    struct_table.insert({key, struct_sym});
  }
}

string *populate_function_sym(symbol* sym, astree* root){
  string *key = NULL;
  for(size_t i = 0; i < root->children.size(); i++){
      if(root->children[i]->symbol == TOK_DECLID){
        key = (string *)root->children[i]->lexinfo;
        printf("So far so good\n");
        return key;
      }
  }
  return key;
}

void insert_function(astree* root){
  symbol* sym = new symbol();
  for(size_t i = 0; i < root->children.size(); i++){
    switch(root->children[i]->symbol){
      case TOK_IDENT:
        sym->attributes.set(ATTR_struct);
        populate_function_sym(sym, root->children[i]);
        break;
      case TOK_INT:
        sym->attributes.set(ATTR_int);
        populate_function_sym(sym, root->children[i]);
        break;
      case TOK_VOID:
        sym->attributes.set(ATTR_void);
        populate_function_sym(sym, root->children[i]);
        break;
      case TOK_BOOL:
        sym->attributes.set(ATTR_bool);
        populate_function_sym(sym, root->children[i]);
        break;
      case TOK_CHAR:
        sym->attributes.set(ATTR_char);
        populate_function_sym(sym, root->children[i]);
        break;
      case TOK_STRING:
        sym->attributes.set(ATTR_string);
        populate_function_sym(sym, root->children[i]);
        break;
      default:
        break;
    }
  }
}
/*
insert_new_var(root){
  case TOK_BOOL;
    if(root->children[0]->symbol == TOK_DECLID ||
       root->children[0]->symbol == TOK_ARRAY)
    break;
  case TOK_CHAR;
    break;
  case TOK_INT;
    break;
  case TOK_STRING;
    break;
}
*/
void visit(astree* root){
    switch(root->symbol){
      case TOK_STRUCT:
        insert_struct(root);
        break;
      case TOK_FUNCTION:
        insert_function(root);
        break;
      case TOK_VARDECL:
        //new_var(root);
        break;
      case TOK_TYPEID:
        //Check to make sure this is found in the struct table
        break;
      case TOK_IF:
        break;
      case TOK_WHILE:
        break;
      case TOK_RETURN:
        break;
      case TOK_ARRAY:
        break;
      case TOK_IDENT:
        break;
      case TOK_ORD:
        break;
      case TOK_CHR:
        break;
      case TOK_NEWSTRING:
        break;
      case TOK_BLOCK:
        break;
      case TOK_CALL:
        break;
      case TOK_IFELSE:
        break;
      case TOK_INITDECL:
        break;
      case TOK_DECLID:
        break;
      case TOK_POS:
        break;
      case TOK_NEG:
        break;
      case TOK_NEWARRAY:
        break;
      case TOK_RETURNVOID:
        //Check the function is supposed to return void
        break;
      case TOK_PROTOTYPE:
        break;
      default:
        for(size_t i = 0; i < root->children.size(); i++){
          visit(root->children[i]);
        }
    }

}
