
#include <string>
using namespace std;

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "astree.h"
#include "auxlib.h"
#include "lyutils.h"
#include "stringset.h"
#include "symtable.h"
#include "yyparse.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

string yyin_cpp_command;
string cpp_flag = "";

extern FILE  *tokfile;
char outfile[LINESIZE];

void yyin_cpp_popen(const char* filename){
   yyin_cpp_command = CPP;
   yyin_cpp_command += " ";
   yyin_cpp_command += cpp_flag;
   yyin_cpp_command += " ";
   yyin_cpp_command += filename;
   yyin = popen (yyin_cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (yyin_cpp_command.c_str());
      exit (get_exitstatus());
   }
}

void yyin_cpp_pclose(void){
   int pclose_rc = pclose (yyin);
   eprint_status (yyin_cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
}

char *print_file(char* filename,const char* extenstion){
   int len = strlen(filename);
   memset(outfile,0,strlen(outfile));
   strncpy(outfile, strtok(filename, "."), len);
   strcat(outfile,".");
   strcat(outfile,extenstion);
   return outfile;
}

int set_opts(int argc, char **argv){
   int c;
   opterr = 0;
   while ((c = getopt (argc, argv, "ly@:D:")) != -1)
      switch (c){
         case 'l': yy_flex_debug = 1;break;
         case 'y': yydebug = 1;break;
      case 'D':
         cpp_flag.append("-D");
         cpp_flag.append(optarg);
         break;
      case '@':
         char debug_arg[LINESIZE];
         debug_arg[0] = '@';
         strncat(debug_arg, optarg, strlen(optarg));
         set_debugflags(debug_arg);
         break;
      case '?':
         if (optopt == 'D' || optopt == '@')
          fprintf(stderr,"Option -%c requires an argument.\n",optopt);
         else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         else
            fprintf (stderr,
                  "Unknown option character `\\x%x'.\n",
                  optopt);
         return -1;
      default:
         return -1;
      }
   return optind;
}

int main (int argc, char** argv) {
   yydebug = 0;
   yy_flex_debug = 0;
   int parsecode = 0;
   set_execname (argv[0]);
   int argi = set_opts(argc, argv);
   if(argi == -1)return 1;
   if(argi+1 < argc){
      printf("Input Error: Multiple Input Files Not Allowed\n");
      return 1;
   }else if(argi == argc){
      printf("Input Error: No File Specified\n");
      return 1;
   }
   char *filename = argv[argi];
   char *is_oc = strrchr(filename,'o');
   if(is_oc == NULL || strcmp(is_oc,"oc") != 0){
      printf("Input Error: .oc files only\n");
      return 1;

   }

   yyin_cpp_popen(filename);
   tokfile = fopen(print_file(filename, "tok"), "w+");
   parsecode = yyparse();
   if (parsecode)
      errprintf ("%:parse failed (%d)\n", parsecode);


   FILE *str_out = fopen(print_file(filename, "str"), "w+");
   dump_stringset(str_out);

   printf("HI\n");
   visit(yyparse_astree);

   FILE *ast_out = fopen(print_file(filename, "ast"), "w+");
   dump_astree(ast_out, yyparse_astree);
   yyin_cpp_pclose();


   return get_exitstatus();
}
