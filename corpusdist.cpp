#include <string>
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <sys/types.h>
#include <unistd.h>
#include "pin.H"

/*

requires -follow_execv for forks.


TODO:
Give coverage for execv/forks in same file? use getpid
Report on simple issues if found. ie double free.

*/

#define COUNTDOWNTODOOM 3000

/*
Other Ideas:

Better ways to deal with different address space
  * Create hashes of blocks and compare them

speed ups by keeping program in memory -- maybe not for code coverage.
  * how to deal with crash
  * detecting where to set checkpoints, before opening file, dealing with file open close
 
*/

PIN_LOCK lock;

struct myblock {
  ADDRINT a;
  struct myblock* next;
};

INT32 numThreads = 0;
const INT32 MaxNumThreads = 10000;

// The running count of instructions is kept here
// We let each thread's count be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This has shown to improve the performance of inscount2_mt by up to 6X
// on SPECOMP2001.
#define PADSIZE 56  // 64byte linesize : 64 - 8
struct THREAD_DATA
{
    struct myblock *blocks;
    UINT8 _pad[PADSIZE];
};

THREAD_DATA icount[MaxNumThreads];

std::ostringstream o;
o<<"corpusdist.out."<<getpid();

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool", "o", o.str(), "specify file name for the output file");

int Usage()
{
  cerr<<"Prints the blocks we've run in order to log code coverage for corpus distilation"<<endl;
  cerr << KNOB_BASE::StringKnobSummary()<<endl;
  return(-1);
}

void logbbl(ADDRINT a, THREADID tid)
{
  myblock *b = icount[tid].blocks;

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

  if (icount[tid].blocks) b->next = n;
  else icount[tid].blocks=n;

  return;
}

VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v)
{
    GetLock(&lock, threadid+1);
    numThreads++;
    ReleaseLock(&lock);
    
    ASSERT(numThreads <= MaxNumThreads, "Maximum number of threads exceeded\n");
}


BOOL CallbackForExec(CHILD_PROCESS childProcess, VOID *val) {
  #ifdef WINDOWS
  int argc = 0;
  const CHAR *const * argv = NULL;
  CHILD_PROCESS_GetCommandLine(childProcess, &argc, &argv);
  #endif
  return true;
}


void Trace(TRACE trace, void *v)
{
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
  {
    BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)logbbl, IARG_ADDRINT, IARG_UINT32, BBL_Address(bbl), IARG_THREAD_ID, IARG_END);
	
	/*
	//This seems like a performance nightmare
	
	long bbllen = BBL_NumIns(bbl);
    long checksum =0;
	for( INS ins= BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins) )
		checksum+=ins; //some way of getting a unique signature for the basic block
	*/
  }
  
}

void Doom(void * v)
{
  //set a timer and call this to force close a program, will call Fini
	if (!PIN_IsApplicationThread())
	{
		PIN_Sleep(COUNTDOWNTODOOM);
		PIN_ExitApplication(0);
	}
}

void Fini(int code, void *v)
{
  ofstream OutFile;
  OutFile.open(KnobOutputFile.Value().c_str());
  OutFile.setf(ios::showbase);
  for (int t=0; t<numThreads; t++)
	if (icount[t].blocks != NULL)
		for (myblock *b = icount[t].blocks; b != NULL ; b=b->next)
			OutFile<< b->a <<endl;
  OutFile<<"END";
  OutFile.close();
}

int main(int argc, char * argv[])
{
  if (PIN_Init(argc,argv)) return Usage();
  PIN_AddFollowChildProcessFunction(CallbackForExec,0);
  TRACE_AddInstrumentFunction(Trace, 0);
  PIN_AddFiniFunction(Fini, 0);
   
  PIN_AddThreadStartFunction(ThreadStart, 0);
  
  PIN_THREAD_UID uid;
  PIN_SpawnInternalThread(&Doom, NULL, 0, &uid);
  

  PIN_StartProgram();
  retrn 0;
}
