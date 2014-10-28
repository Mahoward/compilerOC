
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

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;

string yyin_cpp_command;
string cpp_flag = "";

void yyin_cpp_popen (const char* filename) {
   yyin_cpp_command = CPP;
   yyin_cpp_command += " ";
   yyin_cpp_command += filename;
   yyin = popen (yyin_cpp_command.c_str(), "r");
   if (yyin == NULL) {
      syserrprintf (yyin_cpp_command.c_str());
      exit (get_exitstatus());
   }
}

void yyin_cpp_pclose (void) {
   int pclose_rc = pclose (yyin);
   eprint_status (yyin_cpp_command.c_str(), pclose_rc);
   if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
}

void print_str(const char* filename){
   int len = strlen(filename);
   char stringset_file[len];
   strncpy(stringset_file, strtok(filename, "."), len);
   strcat(stringset_file,".str");
   FILE *out = fopen(stringset_file, "w+");
   dump_stringset(out);
}

//set_opts modeled by:
//http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt
int set_opts(int argc, char **argv){
   int c;
   opterr = 0;
   while ((c = getopt (argc, argv, "ly@:D:")) != -1)
      switch (c){
         case 'l': yy_flex_debug = 1;   break;
         case 'y': yydebug = 1;   break;
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
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
         else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
         else
            fprintf (stderr,
                  "Unknown option character `\\x%x'.\n",
                  optopt);
         return -1;
      default:
         abort ();
      }
   return optind;
}

int main (int argc, char** argv) {
   set_execname (argv[0]);
   int argi = set_opts(argc, argv);
   if(argi == -1)return 1;
   if(argi+1 < argc){
      printf("Input Error: Multiple Input Files Not Allowed\n");
      return 1;
   }
   char *filename = argv[argi];
   char *is_oc = strrchr(filename,'o');
   if(is_oc == NULL || strcmp(is_oc,"oc") != 0){
      printf("Input Error: .oc files only\n");
      return 1;
   }

   yyin_cpp_popen(filename);
   yyin_cpp_pclose();

   //print_str(filename);

   return get_exitstatus();
}
