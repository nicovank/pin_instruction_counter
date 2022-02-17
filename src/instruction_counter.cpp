#include "pin.H"

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

static KNOB<std::string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o",
                                        "intruction_counter.csv",
                                        "specify output file name");
static std::tr1::unordered_map<INT32, UINT64> Frequencies;

VOID OnThreadStart(THREADID ThreadId, CONTEXT *Context, INT32 Flags, VOID *_) {}

VOID OnThreadEnd(THREADID ThreadId, const CONTEXT *Context, INT32 Code,
                 VOID *_) {}

VOID OnProgramEnd(INT32 Code, VOID *_) {
  std::ofstream OutputFile;
  OutputFile.open(KnobOutputFile.Value().c_str());

  std::tr1::unordered_map<std::string, UINT64> AggregateFrequencies;

  for (auto Frequency : Frequencies) {
    INS Instruction;
    Instruction.index = Frequency.first;
    AggregateFrequencies[INS_Mnemonic(Instruction)] += Frequency.second;
  }

  std::vector<std::pair<std::string, UINT64>> SortedAggregateFrequencies;
  SortedAggregateFrequencies.reserve(AggregateFrequencies.size());

  for (auto Frequency : AggregateFrequencies) {
    bool inserted = false;

    for (auto it = SortedAggregateFrequencies.begin();
         it != SortedAggregateFrequencies.end(); ++it) {
      if (Frequency.second > it->second) {
        SortedAggregateFrequencies.insert(it, Frequency);
        inserted = true;
        break;
      }
    }

    if (!inserted) {
      SortedAggregateFrequencies.push_back(Frequency);
    }
  }

  for (auto Frequency : SortedAggregateFrequencies) {
    OutputFile << Frequency.first << "," << Frequency.second << std::endl;
  }
}

VOID OnInstruction(THREADID ThreadId, UINT32 InstructionIndex) {
  // TODO: This is not thread-safe.
  ++Frequencies[(INT32)InstructionIndex];
}

VOID Instruction(INS Instruction, VOID *_) {
  INS_InsertCall(Instruction, IPOINT_BEFORE, (AFUNPTR)OnInstruction,
                 IARG_THREAD_ID, IARG_UINT32, (UINT32)Instruction.index,
                 IARG_END);
}

INT32 Usage() {
  std::cerr << "" << std::endl;
  std::cerr << KNOB_BASE::StringKnobSummary() << std::endl;
  return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
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
