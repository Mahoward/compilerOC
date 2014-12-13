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
stack <symbol_table*> sym_stack;
int blocknr = 0;
int block_count = 0;
stack <int> block_stack;

//Declarations
void visit(astree* root);

/*-----------Utilities-------------*/
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

symbol* create_sym(astree* node){
  symbol* sym = new symbol();
  sym->struct_name = new string;
  sym->filenr = node->filenr;
  sym->linenr = node->linenr;
  sym->offset = node->offset;
  sym->blocknr = blocknr;
  sym->fields = NULL;
  sym->parameters = NULL;
  return sym;
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

void set_var_type(astree* node, symbol* sym, const string* struct_name){
  sym->attributes.set(ATTR_variable);
  sym->attributes.set(ATTR_lval);
  switch(node->symbol){
    case TOK_IDENT:
    sym->attributes.set(ATTR_struct);
    sym->struct_name->append(*struct_name);
    break;
    case TOK_INT:
    sym->attributes.set(ATTR_int);
    break;
    case TOK_VOID:
    sym->attributes.set(ATTR_void);
    break;
    case TOK_BOOL:
    sym->attributes.set(ATTR_bool);
    break;
    case TOK_CHAR:
    sym->attributes.set(ATTR_char);
    break;
    case TOK_STRING:
    sym->attributes.set(ATTR_string);
    break;
  }
}

void push_table(){
  symbol_table* table = new symbol_table();
  sym_stack.push(table);
}

void pop_table(){
  sym_stack.pop();
}

void insert_table(string* key, symbol* sym){
  sym_stack.top()->insert({key, sym});
}
/*
bool in_gtable(string *key){
  symbol_table::const_iterator inTable = global_table.find(key);
  if(inTable == global_table.end){
    return false;
  }else{
    return true;
  }
}

bool in_table(){

}
*/
void insert_global(string *key, symbol *sym){
  global_table.insert({key, sym});
}

void enter_block(){
  block_stack.push(blocknr);
  block_count++;
  blocknr = block_count;

}

void leave_block(){
  blocknr = block_stack.top();
  block_stack.pop();
}


/*-----------Printing-------------*/
void print_fields(string *struct_name, symbol* struct_sym){
  vector<string*> keys;
  keys.reserve(struct_sym->fields->size());

  vector<symbol*> syms;
  syms.reserve(struct_sym->fields->size());

  for(auto kv : *struct_sym->fields) {
    keys.push_back(kv.first);
    syms.push_back(kv.second);
  }

  for(size_t i = 0; i < keys.size(); i++){
    string *attp = get_att_string(syms[i]);
    printf("  %s (%ld.%ld.%ld) {%s} %s\n",
    keys[i]->c_str(), syms[i]->filenr,
    syms[i]->linenr, syms[i]->offset,
    struct_name->c_str(), attp->c_str());
  }

}

void print_struct(string *key, symbol* struct_sym){
  string *attp = get_att_string(struct_sym);
  printf("%s (%ld.%ld.%ld) {%ld} %s\n",
  key->c_str(), struct_sym->filenr,
  struct_sym->linenr, struct_sym->offset,
  struct_sym->blocknr, attp->c_str());
  print_fields(key, struct_sym);
}

void print_sym(string *key, symbol* sym){
  for(size_t i = 0; i < sym->blocknr; i++){
    printf("  ");
  }
  string *attp = get_att_string(sym);
  printf("%s (%ld.%ld.%ld) {%ld} %s\n",
  key->c_str(), sym->filenr,
  sym->linenr, sym->offset,
  sym->blocknr, attp->c_str());
}

/*-----------Logic-------------*/

/*---------Struct-----------*/
void set_field_type(astree* node, symbol* sym, const string* struct_name){
  sym->attributes.set(ATTR_field);
  switch(node->symbol){
    case TOK_IDENT:
    sym->attributes.set(ATTR_struct);
    sym->struct_name->append(*struct_name);
    break;
    case TOK_INT:
    sym->attributes.set(ATTR_int);
    break;
    case TOK_VOID:
    sym->attributes.set(ATTR_void);
    break;
    case TOK_BOOL:
    sym->attributes.set(ATTR_bool);
    break;
    case TOK_CHAR:
    sym->attributes.set(ATTR_char);
    break;
    case TOK_STRING:
    sym->attributes.set(ATTR_string);
    break;
  }
}

void populate_fields(astree* root, symbol_table& fields){
  for(size_t i = 0; i < root->children.size(); i++){
    if(root->children[i]->symbol != TOK_TYPEID){
      string *key = NULL;
      for(size_t q = 0; q < root->children[i]->children.size(); q++){
        if(root->children[i]->children[q]->symbol == TOK_FIELD){
          symbol* sym = create_sym(root->children[i]->children[q]);
          sym->struct_name = new string;
          key = (string *)root->children[i]->children[q]->lexinfo;
          set_field_type(root->children[i], sym,
          root->children[i]->lexinfo);
          fields.insert({key, sym});
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

void handle_struct(astree* root){
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

/*---------Function-----------*/

void populate_param(astree* root, vector<symbol*> parameters){
  enter_block();
  for(size_t i = 0; i < root->children.size(); i++){
    if(root->children[i]->symbol == TOK_PARAM){
      string *key = NULL;
      for(size_t q = 0; q < root->children[i]->children.size(); q++){
          key = (string *)root->children[i]->children[q]->children[0]->lexinfo;
          symbol *sym = create_sym(root->children[i]->children[q]->children[0]);
          sym->attributes.set(ATTR_param);
          set_var_type(root->children[i]->children[q], sym,
                       root->children[i]->children[q]->lexinfo);
          print_sym(key, sym);
          parameters.push_back(sym);
        }
      }
    }
    if(root->symbol == TOK_PROTOTYPE){
      leave_block();
    }else{
      blocknr = block_stack.top();
      block_stack.pop();
      block_count--;
    }
}

string *populate_function_sym(symbol* sym, astree* root){
  string *key = NULL;
  sym->parameters = new vector<symbol*>;
  for(size_t i = 0; i < root->children.size(); i++){
    if(root->children[i]->symbol == TOK_DECLID){
      key = (string *)root->children[i]->lexinfo;
    }
  }
  print_sym(key,sym);
  return key;
}

void handle_function(astree* root){
  symbol* sym;
  string *key = NULL;
  for(size_t i = 0; i < root->children.size(); i++){
    switch(root->children[i]->symbol){
      case TOK_IDENT:
        sym = create_sym(root->children[i]->children[0]);
        sym->attributes.set(ATTR_function);
        sym->attributes.set(ATTR_struct);
        sym->struct_name->append(*root->children[i]->lexinfo);
        key = populate_function_sym(sym, root->children[i]);
        break;
      case TOK_INT:
        sym = create_sym(root->children[i]->children[0]);
        sym->attributes.set(ATTR_function);
        sym->attributes.set(ATTR_int);
        key = populate_function_sym(sym, root->children[i]);
        break;
      case TOK_VOID:
        sym = create_sym(root->children[i]->children[0]);
        sym->attributes.set(ATTR_function);
        sym->attributes.set(ATTR_void);
        key = populate_function_sym(sym, root->children[i]);
        break;
      case TOK_BOOL:
        sym = create_sym(root->children[i]->children[0]);
        sym->attributes.set(ATTR_function);
        sym->attributes.set(ATTR_bool);
        key = populate_function_sym(sym, root->children[i]);
        break;
      case TOK_CHAR:
        sym = create_sym(root->children[i]->children[0]);
        sym->attributes.set(ATTR_function);
        sym->attributes.set(ATTR_char);
        key = populate_function_sym(sym, root->children[i]);
        break;
      case TOK_STRING:
        sym = create_sym(root->children[i]->children[0]);
        sym->attributes.set(ATTR_function);
        sym->attributes.set(ATTR_string);
        key = populate_function_sym(sym, root->children[i]);
        break;
      case TOK_PARAM:
        populate_param(root, *sym->parameters);
        break;
      case TOK_BLOCK:
        /*
        if(in_gtable(key)){
          eprintf("Multiple functions with name %s ", key->c_str());
          set_exitstatus(1);
          abort();
        }
        */
        visit(root->children[i]);
        break;
      default:
        break;
    }
  }
  insert_global(key, sym);
}

/*---------Vardecl-----------*/
void handle_vardecl(astree* root){
  symbol* sym;
  string* key;
  for(size_t i = 0; i < root->children[0]->children.size(); i++){
    if(root->children[0]->children[i]->symbol == TOK_DECLID){
      key = (string *)root->children[0]->children[i]->lexinfo;
      sym = create_sym(root->children[0]->children[i]);
    }
    if(root->children[0]->children[i]->symbol == TOK_ARRAY){
      sym->attributes.set(ATTR_array);
    }
  }
  set_var_type(root->children[0], sym, root->children[0]->lexinfo);
  //TODO TYPECHECK RIGHT SIDE OF VARDEL
  //check_var(sym);
  print_sym(key, sym);
  insert_table(key, sym);
  return;
}

/*---------Variable-----------*/
/*

case TOK_IDENT:
case TOK_INT:
case TOK_VOID:
case TOK_BOOL:
case TOK_CHAR:
case TOK_STRING:
handle_variable(root);
break;

void handle_variable(astree* root){
  //set_var_type(root, sym, root->lexinfo);

}
*/

/*---------Ifelse-----------*/
void handle_ifelse(astree* root){
  for(size_t i = 0; i< root->children.size(); i++){
    if(root->children[i]->symbol == TOK_BLOCK){
      visit(root->children[i]);
    }
  }
  return;
}

/*---------While-----------*/
void handle_while(astree* root){
  for(size_t i = 0; i< root->children.size(); i++){
    if(root->children[i]->symbol == TOK_BLOCK){
      visit(root->children[i]);
    }
  }
  return;
}

/*---------Main-----------*/
void visit(astree* root){
    switch(root->symbol){
      case TOK_STRUCT:
        handle_struct(root);
        break;
      case TOK_PROTOTYPE:
      case TOK_FUNCTION:
        handle_function(root);
        break;
      case TOK_BLOCK:
        enter_block();
        push_table();
        for(size_t i = 0; i < root->children.size(); i++){
          visit(root->children[i]);
        }
        leave_block();
        break;
      case TOK_VARDECL:
        handle_vardecl(root);
        break;
      case TOK_IFELSE:
      case TOK_IF:
        handle_ifelse(root);
        break;
      case TOK_WHILE:
        handle_while(root);
        break;
      case '+':
        break;
      case '-':
        break;
      case '*':
        break;
      case '/':
        break;
      case '%':
        break;
      case TOK_POS:
        break;
      case TOK_NEG:
        break;
      case TOK_EQ:
        break;
      case TOK_NE:
        break;
      case TOK_LT:
        break;
      case TOK_LE:
        break;
      case TOK_GT:
        break;
      case TOK_GE:
        break;

      case TOK_TYPEID:
        //Check to make sure this is found in the struct table
        break;
      case TOK_RETURN:
        break;
      case TOK_ARRAY:
        break;
      case TOK_ORD:
        break;
      case TOK_CHR:
        break;
      case TOK_NEWSTRING:
        break;
      case TOK_CALL:
        break;
      case TOK_NEWARRAY:
        break;
      case TOK_RETURNVOID:
        //Check the function is supposed to return void
        break;
      default:
        for(size_t i = 0; i < root->children.size(); i++){
          visit(root->children[i]);
        }
    }

}

void build_sym(astree* root){
  //global_table = new symbol_table();
  sym_stack.push(&global_table);
  visit(root);
}
