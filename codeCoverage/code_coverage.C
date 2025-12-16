
/*
 *  A simple code coverage tool using DyninstAPI
 *
 *  This tool uses DyninstAPI to instrument the functions and basic blocks in
 *  an executable and its shared libraries in order to record code coverage
 *  data when the executable is run. This code coverage data is output when the
 *  rewritten executable finishes running.
 *
 *  The intent of this tool is to demonstrate some capabilities of DyninstAPI;
 *  it should serve as a good stepping stone to building a more feature-rich
 *  code coverage tool on top of Dyninst.
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// Command line parsing
#include <getopt.h>

// DyninstAPI includes
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_flowGraph.h"
#include "BPatch_function.h"
#include "BPatch_module.h"
#include "BPatch_object.h"
#include "BPatch_point.h"

using namespace Dyninst;

#define MUTNAMELEN 1024
#define FUNCNAMELEN 32*1024
#define NO_ERROR -1
#define dprintf printf

BPatch *bpatch;

static const char* USAGE = " -s <shared library> <output shared library\n";

static const char* OPT_STR = "s";

// configuration options
char const* mutateeName = NULL;
char const* outfile = NULL;

set<string> skipLibraries;

void initSkipLibraries() {
  /* List of shared libraries to skip instrumenting */
  /* Do not instrument the instrumentation library */
  skipLibraries.insert("libInst.so");
  skipLibraries.insert("libc.so.6");
  skipLibraries.insert("libc.so.7");
  skipLibraries.insert("ld-2.5.so");
  skipLibraries.insert("ld-linux.so.2");
  skipLibraries.insert("ld-lsb.so.3");
  skipLibraries.insert("ld-linux-x86-64.so.2");
  skipLibraries.insert("ld-lsb-x86-64.so");
  skipLibraries.insert("ld-elf.so.1");
  skipLibraries.insert("ld-elf32.so.1");
  skipLibraries.insert("libstdc++.so.6");
  return;
}

bool parseArgs(int argc, char* argv[]) {
  int c;
  while((c = getopt(argc, argv, OPT_STR)) != -1) {
    switch((char)c) {
      case 's':
        /* if includeSharedLib is set,
         * all libraries linked to the binary will also be instrumented */
        //includeSharedLib = true;
        break;
      default: cerr << "Usage: " << argv[0] << USAGE; return false;
    }
  }

  int endArgs = optind;

  if(endArgs >= argc) {
    cerr << "Input binary not specified." << endl << "Usage: " << argv[0] << USAGE;
    return false;
  }
  /* Input Binary */
  mutateeName = argv[endArgs];

  endArgs++;
  if(endArgs >= argc) {
    cerr << "Output binary not specified." << endl << "Usage: " << argv[0] << USAGE;
    return false;
  }

  /* Rewritten Binary */
  outfile = argv[endArgs];
  printf("Instrumenting %s , output %s\n", mutateeName, outfile);
  return true;
}

BPatch_function * tauFindFunction (BPatch_image *appImage, const char * functionName)
{
  // Extract the vector of functions 
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction(functionName, found_funcs, false, true, true)) || !found_funcs.size()) {
    dprintf("tau_run: Unable to find function %s\n", functionName); 
    return NULL;
  }
  return found_funcs[0]; // return the first function found 
  // FOR DYNINST 3.0 and previous versions:
  // return appImage->findFunction(functionName);
}


//Only instruments the input library
int tauRewriteLibrary(BPatch *bpatch)
{
  using namespace std;
  dprintf("Inside tauRewriteLibrary, name=%s, out=%s\n", mutateeName, outfile);

  BPatch_binaryEdit* mutateeAddressSpace = bpatch->openBinary(mutateeName, false);

  if( mutateeAddressSpace == NULL ) {
    fprintf(stderr, "Failed to open binary %s\n",
           mutateeName);
    return -1;
  }

  const char* instLibrary = "libInst.so";
  bool result = mutateeAddressSpace->loadLibrary(instLibrary);
  if (!result) {
    printf("Error: loadLibrary(%s) failed. Please ensure that TAU's lib directory is in your LD_LIBRARY_PATH environment variable and retry.\n", instLibrary);
    printf("You may also want to use tau_exec while launching the rewritten binary. If TAU relies on some external libraries (Score-P), these may need to specified as tau_exec -loadlib=/path/to/library <mutatee> \n");
  }
  assert(result);

  BPatch_image* mutateeImage = mutateeAddressSpace->getImage();



  //As libraries do not have main, instrumentation is only inserted into
  //the entry and exit of function calls, tau_exec, or a program compiled with 
  //TAU, is necessary to initialize TAU
  //tau_trace_lib_entry uses TAU_START, tau_trace_lib_exist uses TAU_STOP
  dprintf("Searching for TAU functions\n");
  BPatch_function* entryLibTrace = tauFindFunction(mutateeImage, "FEntryCoverage");
  BPatch_function* exitLibTrace = tauFindFunction(mutateeImage, "FExitCoverage");

  if(!entryLibTrace || !exitLibTrace )
  {
    fprintf(stderr, "Couldn't find TAU hook functions, aborting\n");
    return -1;
  }

  //Get the different modules, which will be the monitoring library(tau) and
  //the library where we want to insert instrumentation
  dprintf("Getting modules \n");
  vector<BPatch_module *> *modules = mutateeImage->getModules();
  vector<BPatch_module *>::iterator moduleIter;

  //Get only the name of the library from mutateeName, as we can be selecting the
  //library from its original path, but the module only shows the library name
  string libpath(mutateeName);
  string mutateefilename = libpath.substr(libpath.find_last_of("/\\") + 1);

  //Iterate between the different modules and only insert instrumentation into
  //the desired library
  for (moduleIter = modules->begin(); moduleIter != modules->end();
       ++moduleIter) 
  {
    char moduleName[1024];
    (*moduleIter)->getName(moduleName, 1024);
    dprintf("module %s, mutatee %s \n", moduleName, mutateefilename.c_str());

	  string module_str = moduleName;
    if( strcmp(module_str.substr(module_str.find_last_of("/\\") + 1).c_str(), mutateefilename.c_str())!=0)
    {
        dprintf("Skipping module!\n");
        continue;
    }
    if(!(*moduleIter)->isSharedLib()) {
      printf("Module to instrument is not a shared library, cannot instrument\n");
      return -1;
    }

    dprintf("Instrumenting module\n");

    vector<BPatch_function *> *allFunctions = (*moduleIter)->getProcedures();
    vector<BPatch_function *>::iterator funcIter;

    //Iterate between the different function in the library
    for (funcIter = allFunctions->begin(); funcIter != allFunctions->end();
         ++funcIter) 
    {
      char funcName[1024];
      BPatch_function *curFunc = *funcIter;
      curFunc->getName(funcName, 1024);

      dprintf("Instrumenting Function %s\n", funcName);

      //Find the entry and exit points of each function
      BPatch_Vector<BPatch_point*>* funcEntry = curFunc->findPoint(BPatch_entry);
      BPatch_Vector<BPatch_point*>* funcExit = curFunc->findPoint(BPatch_exit);

      //TAU_START and TAU_STOP need the name of the function, pass it as an argument
      BPatch_Vector<BPatch_snippet *> regArgs;
      BPatch_constExpr coverageFunc(funcName);
      regArgs.push_back(&coverageFunc);

      //Create the function calls with the names of the functions as the input parameter
      BPatch_funcCallExpr entryTrace(*entryLibTrace, regArgs);
      BPatch_funcCallExpr exitTrace(*exitLibTrace, regArgs);

      //Insert the snippets to monitor the function calls
      BPatchSnippetHandle* handle_a = mutateeAddressSpace->insertSnippet(entryTrace, *funcEntry, BPatch_callBefore, BPatch_lastSnippet);
      BPatchSnippetHandle* handle_b = mutateeAddressSpace->insertSnippet(exitTrace, *funcExit, BPatch_callAfter, BPatch_lastSnippet);
      if(!handle_a)
      {
        printf("Failed to insert instrumentation at function entry of %s", funcName);
        return -1;
      }
      if(!handle_b)
      {
        printf("Failed to insert instrumentation at function exit of %s", funcName);
        return -1;
      }
    }
  }

  dprintf("Library instrumented, writing...\n");
  std::string modifiedFileName(outfile);
    // Output the instrumented binary
  if(!mutateeAddressSpace->writeFile(outfile)) {
    printf("Failed to write output file: %s \n",  outfile );
    return EXIT_FAILURE;
  }
  return 0;
}

int main(int argc, char* argv[]) {
  if(!parseArgs(argc, argv))
    return EXIT_FAILURE;

  bpatch = new BPatch;                           //create a new version. 
  return tauRewriteLibrary(bpatch);;
}
