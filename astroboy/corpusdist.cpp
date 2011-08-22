#include <iostream>
#include <fstream>
#include <malloc.h>
#include "pin.H"

/*
TODO:

Use builtin Pin buffer tools for writes instead of linked list.
Add support for multi-threaded software, forks
Report on simple issues if found. ie double free.


*/

/*
Other Ideas:

Better ways to deal with different address space
  * keep program in memory
  * offset stack randomization by basing everything as offsets from main
  * Create hashes of blocks and compare them

speed ups by keeping program in memory -- maybe not for code coverage.
  * how to deal with crash
  * detecting where to set checkpoints, before opening file, dealing with file open close
 
*/


//static UINT64 icount=0;

struct myblock {
  ADDRINT a;
  struct myblock* next;
};

static myblock *blocks;

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool", "o", "corpusdist.out", "specify file name for the output file");

int Usage()
{
  cerr<<"Prints the blocks we've run in order to log code coverage for corpus distilation"<<endl;
  cerr << KNOB_BASE::StringKnobSummary()<<endl;
  return(-1);
}

/*
void docount(UINT32 c)
{
  icount +=c;
}
*/
void logbbl(ADDRINT a)
{

  myblock *b = blocks;

  while ((b) && (b->next)) 
  {
    if (b->a == a) 
    {
      return; //skip reporting on blocks we've visited
    }
    b=b->next;
  }

  myblock *n = (myblock *) malloc(sizeof(struct myblock));
  n->a = a;
  n->next = NULL;

  if (blocks) b->next = n;
  else blocks=n;

  return;
}

void Trace(TRACE trace, void *v)
{
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
  {
    // BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, BBL_NumIns(bbl), IARG_END);
    BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)logbbl, IARG_ADDRINT, BBL_Address(bbl), IARG_END);
  }
}

void Fini(int code, void *v)
{
  ofstream OutFile;
  OutFile.open(KnobOutputFile.Value().c_str());
  OutFile.setf(ios::showbase);
  //  OutFile << "Instruction Count " << icount << endl;
  if (blocks != NULL)
    for (myblock *b = blocks; b != NULL ; b=b->next)
      OutFile<< b->a <<endl;
  OutFile<<"END";
  OutFile.close();
}

int main(int argc, char * argv[])
{
  if (PIN_Init(argc,argv)) return Usage();
  TRACE_AddInstrumentFunction(Trace, 0);
  PIN_AddFiniFunction(Fini, 0);
  PIN_StartProgram();
  return 0;
}
