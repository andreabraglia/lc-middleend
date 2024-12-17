//===-- LICMZ.cpp - Custom Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Path:
//  SRC/llvm/lib/Transforms/Utils/LICMZ.cpp
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LICMZ.h"

using namespace llvm;

bool isInstructionInvariant(const Instruction &Inst, Loop &L);

bool isOperandInvariant(const Use &Usee, Loop &L) {
  // Se è una costante o un argomento di funzione è invariante
  if (isa<Constant>(Usee) || isa<Argument>(Usee)) {
    return true;
  }

  Instruction *Inst = dyn_cast<Instruction>(Usee);

  if (!Inst || isa<PHINode>(Inst)) {
    return false;
  }

  // Se la reaching definition è fuori dal loop allora è invariante
  if (!L.contains(Inst)) {
    return true;
  }

  return L.isLoopInvariant(Inst);
}

bool isInstructionInvariant(const Instruction &Inst, Loop &L) {
  // Se è un phi-node non è invariante
  if (isa<PHINode>(Inst)) {
    return false;
  }

  // Verifica se tutti gli operandi sono invariabili
  for (const auto &Usee : Inst.operands()) {
    // Se un'operando non è invariante allora l'istruzione non è invariante
    if (!isOperandInvariant(Usee, L)) {
      return false;
    }
  }

  return true;
}

// Controlla se un'istruzione non ha usi al di fuori del loop.
bool isDeadCode(const Instruction &Inst, Loop &L) {
  for (auto *User : Inst.users()) {
    // Se l'uso è dentro il loop allora l'istruzione non è dead
    if (!L.contains(dyn_cast<Instruction>(User))) {
      return false;
    }
  }

  return true;
}

bool isInstructionDominatedByExits(
    const Instruction &Inst,
    const SmallVector<std::pair<BasicBlock *, BasicBlock *>> &ExitBasicBlocks,
    const DominatorTree &DT) {
  // Controlla se l'istruzione domina tutte le uscite
  for (const auto &ExitBasicBlock : ExitBasicBlocks) {
    // Controlla che l'istruzione domini il blocco(second) fuori dal loop
    // verso cui punta l'arco di uscita
    if (!DT.dominates(&Inst, ExitBasicBlock.second)) {
      return false;
    }
  }

  // Se domina tutte le uscite
  return true;
}

PreservedAnalyses LICMZ::run(Loop &L, LoopAnalysisManager &LAM,
                             LoopStandardAnalysisResults &LAR, LPMUpdater &LU) {

  outs() << "LICMZ pass is running!\n";

  if (!L.isLoopSimplifyForm()) {
    outs() << "The loop is not in normal form\n";
    return PreservedAnalyses::all();
  }

  auto LBlocks = L.getBlocks();
  auto *LPreHeader = L.getLoopPreheader();
  Instruction &LastInstruction = LPreHeader->back();
  bool hasChanged = false;

  // Taken from LoopPeel.cpp (Loop peeling utilies)
  // - first: Il blocco all'interno del loop da cui parte l'arco di uscita.
  // - second: Il blocco fuori dal loop verso cui punta l'arco.
  llvm::SmallVector<std::pair<BasicBlock *, BasicBlock *>> ExitBasicBlocks;
  L.getExitEdges(ExitBasicBlocks);

  // Itero sui blocchi del loop
  for (auto *BB : LBlocks) {
    // Trovo le loop-invariant instructions
    for (auto iter = BB->begin(); iter != BB->end();) {
      outs() << "--------------------------------\n";
      outs() << "Checking instruction\n";
      Instruction &Inst = *iter++;

      if (!isInstructionInvariant(Inst, L)) {
        outs() << "Instruction is not invariant\n";
        Inst.print(outs());
        outs() << "\n";

        continue;
      }

      // Verifica dominanza e dead code
      if (!isInstructionDominatedByExits(Inst, ExitBasicBlocks, LAR.DT) &&
          !isDeadCode(Inst, L)) {
        outs() << "Instruction not dominates all the loop exit or is not "
                  "dead code after the loop\n";
        Inst.print(outs());
        outs() << "\n";
        continue;
      }

      outs() << "Instruction is loop invariant\n";
      Inst.print(outs());
      outs() << "\n";

      // Sposta l'istruzione nel preheader del loop
      outs() << "Moving instruction to loop preheader\n";
      outs() << "Last instruction in preheader: ";
      LPreHeader->back().print(outs());
      outs() << "\n";
      Inst.removeFromParent();
      Inst.insertBefore(&LPreHeader->back());

      hasChanged = true;
    }
  }

  return hasChanged ? PreservedAnalyses::none() : PreservedAnalyses::all();
}