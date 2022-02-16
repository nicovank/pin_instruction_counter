#include "pin.H"

#include <fstream>
#include <iostream>
#include <unordered_map>

static KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "intruction_counter.out",
                                        "specify output file name");
static std::unordered_map<UINT32, int> Frequencies;

VOID OnThreadStart(THREADID ThreadId, CONTEXT* Context, INT32 Flags, VOID* _) {}

VOID OnThreadEnd(THREADID ThreadId, const CONTEXT* Context, INT32 Code, VOID* _) {}

VOID OnProgramEnd(INT32 Code, VOID* _) {
  std::ofstream OutputFile;
  OutputFile.open(KnobOutputFile.Value().c_str());
}

VOID OnInstruction(THREADID ThreadId, UINT32 Code) { ++Frequencies[Code]; }

VOID Instruction(INS Instruction, VOID* _) {
  INS_InsertCall(Instruction, IPOINT_BEFORE, (AFUNPTR) OnInstruction, IARG_THREAD_ID, IARG_UINT32,
                 INS_Opcode(Instruction), IARG_END);
}

INT32 Usage() {
  std::cerr << "" << std::endl;
  std::cerr << KNOB_BASE::StringKnobSummary() << std::endl;
  return EXIT_FAILURE;
}

int main(int argc, char* argv[]) {
  PIN_InitSymbols();
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  INS_AddInstrumentFunction(Instruction, nullptr);
  PIN_AddThreadStartFunction(OnThreadStart, nullptr);
  PIN_AddThreadFiniFunction(OnThreadEnd, nullptr);
  PIN_AddFiniFunction(OnProgramEnd, nullptr);

  PIN_StartProgram();
}
