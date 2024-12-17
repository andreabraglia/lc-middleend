//===-- LoopFusionPass.cpp - Custom Transformations --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Path:
//  SRC/llvm/lib/Transforms/Utils/LoopFusionPass.cpp
//===------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LoopFusionPass.h"

using namespace llvm;
using namespace std;

BasicBlock *getLoopBody(Loop *L, LoopInfo &LI) {
  BasicBlock *Header = L->getHeader();
  if (!Header) {
    return nullptr;
  }

  for (BasicBlock *BB : L->getBlocks()) {
    if (BB != Header && LI.getLoopFor(BB) == L) {
      // Trova il primo blocco interno effettivo del loop
      return BB;
    }
  }

  return nullptr;
}

BasicBlock *getLoopEntry(Loop *L) {
  if (!L) {
    return nullptr;
  }

  BasicBlock *Preheader = L->getLoopPreheader();
  if (!Preheader) {
    return nullptr;
  }

  if (!L->isGuarded()) {
    return Preheader;
  }

  // Loop is guarded
  return Preheader->getUniquePredecessor();
}

bool areLoopAdjacent(Loop *L1, Loop *L2) {
  if (!L1 || !L2) {
    return false;
  }

  BasicBlock *EntryL2 = getLoopEntry(L2);

  // Controllo se il primo loop (L1) è guarded (protetto da un controllo
  // condizionale)
  if (L1->isGuarded()) {
    BasicBlock *EntryL1 = getLoopEntry(L1);

    // Verifico che il terminatore del blocco di ingresso sia un'istruzione
    // di branch condizionale
    BranchInst *GuardBI = dyn_cast<BranchInst>(EntryL1->getTerminator());
    if (!GuardBI || !GuardBI->isConditional()) {
      return false;
    }

    // Verifico se uno dei successori di L1 punta al blocco di ingresso di L2
    for (unsigned i = 0; i < GuardBI->getNumSuccessors(); ++i) {
      if (GuardBI->getSuccessor(i) == EntryL2) {
        return true;
      }
    }

    return false;
  }

  // Verifico se il suo blocco di uscita punta al blocco di ingresso di L2
  SmallVector<BasicBlock *, 4> ExitingBlocks;
  L1->getExitingBlocks(ExitingBlocks);

  for (BasicBlock *ExitingBB : ExitingBlocks) {
    Instruction *Term = ExitingBB->getTerminator();
    if (!Term) {
      continue;
    }

    // Controlla se uno dei successori è il blocco di ingresso di L2
    for (unsigned i = 0; i < Term->getNumSuccessors(); ++i) {
      if (Term->getSuccessor(i) == EntryL2) {
        return true;
      }
    }
  }

  return false;
}

bool areLoopsControlFlowEquivalent(Loop *L1, Loop *L2, DominatorTree &DT,
                                   PostDominatorTree &PDT) {
  if (!L1 || !L2)
    return false;

  // Ottengo gli ingressi dei loop
  BasicBlock *EntryL1 = getLoopEntry(L1);
  BasicBlock *EntryL2 = getLoopEntry(L2);

  if (!EntryL1 || !EntryL2)
    return false;

  // Se L1 domina L2 e L2 postdomina L1 allora i due loop sono control flow
  // equivalenti
  return DT.dominates(EntryL1, EntryL2) && PDT.dominates(EntryL2, EntryL1);
}

bool areLoopTripCountEquivalent(Loop *L1, Loop *L2, ScalarEvolution &SE) {
  const SCEV *TripCount1 =
      SE.getTripCountFromExitCount(SE.getExitCount(L1, L1->getExitingBlock()));
  const SCEV *TripCount2 =
      SE.getTripCountFromExitCount(SE.getExitCount(L2, L2->getExitingBlock()));

  return TripCount1 == TripCount2;
}

bool areLoopDistanceNegative(Loop *L1, Loop *L2, DependenceInfo &DI) {
  // A negative distance dependence occurs between Lj and Lk, Lj before Lk,
  // when at iteration m from Lk uses a value that is computed by Lj at a future
  // iteration m+n (where n > 0).

  vector<Instruction *> InstructionsL1;
  vector<Instruction *> InstructionsL2;

  // Aggiungo le istruzioni di L1 e L2 ai due vettori
  for (BasicBlock *BB : L1->blocks()) {
    for (Instruction &I : *BB) {
      InstructionsL1.push_back(&I);
    }
  }

  for (BasicBlock *BB : L2->blocks()) {
    for (Instruction &I : *BB) {
      InstructionsL2.push_back(&I);
    }
  }

  // Verifico le dipendenze tra le istruzioni di L1 e L2
  for (Instruction *I0 : InstructionsL1) {
    // Verifico se c'è una dipendenza tra I0 e tutte le istruzioni in L2
    for (Instruction *I1 : InstructionsL2) {
      auto dep = DI.depends(I0, I1, true);

      if (!dep) {
        continue;
      }

      // Se c'è una dipendenza, la distanza è negativa
      if (dep->isAnti() || dep->isConfused()) {
        return true;
      }
    }
  }

  return false;
}

list<Loop *> getMergeableLoops(LoopInfo *LI) {
  list<Loop *> MergeableLoops;

  for (Loop *TopLevelLoop : *LI) {
    // Controllo che il loop abbia i blocchi e le strutture necessarie per
    // considerarlo valido:
    //  - Preheader: blocco che precede l'ingresso del loop
    //  - Header: blocco che definisce l'inizio del loop
    //  - Latch: blocco che permette di tornare all'inizio del loop
    //  - ExitingBlock: blocco che esce dal loop
    //  - ExitBlock: blocco successivo al loop
    //  - Simply form: verifico se il loop è nella forma semplificata
    if (TopLevelLoop->isInnermost()) {
      if (!TopLevelLoop->getLoopPreheader() || !TopLevelLoop->getHeader() ||
          !TopLevelLoop->getLoopLatch() || !TopLevelLoop->getExitingBlock() ||
          !TopLevelLoop->getExitBlock() ||
          !TopLevelLoop->isLoopSimplifyForm()) {
        outs() << "Loop not mergeable \n";
        continue;
      }

      MergeableLoops.push_front(TopLevelLoop);
    }
  }

  return MergeableLoops;
}

bool fuseLoops(Loop *L1, Loop *L2, LoopInfo &LI) {
  // Modificare gli usi della induction variable nel body del
  // loop 2 con quelli della induction variable del loop 1
  PHINode *ivL1 = L1->getCanonicalInductionVariable();
  PHINode *ivL2 = L2->getCanonicalInductionVariable();

  if (!ivL1 || !ivL2) {
    outs() << "[fuseLoops] - Error: Induction variable not found\n";
    return false;
  }

  ivL2->replaceAllUsesWith(ivL1);
  ivL2->eraseFromParent();

  // Blocco che precede l'ingresso del loop
  BasicBlock *PreheaderL2 = L2->getLoopPreheader();

  // Blocchi che definiscono l'inizio del loop
  BasicBlock *HeaderL1 = L1->getHeader();
  BasicBlock *HeaderL2 = L2->getHeader();

  // Blocchi che formano il body dei loop
  BasicBlock *BodyL2 = getLoopBody(L2, LI);

  // Blocchi che permette di tornare all'inizio del loop
  BasicBlock *LatchL1 = L1->getLoopLatch();
  BasicBlock *LatchL2 = L2->getLoopLatch();

  BasicBlock *ExitBlockL2 = L2->getExitBlock();

  // Sostituisco nel'HeaderL1 il successore PreheaderL2 con l'ExitBlockL2
  HeaderL1->getTerminator()->replaceSuccessorWith(PreheaderL2, ExitBlockL2);

  // Il body di L2 deve essere agganciato a seguito del body di L1
  for (auto *LatchL1Pred : predecessors(LatchL1)) {
    // LatchL1Pred sono i blocchi del body di L1
    LatchL1Pred->getTerminator()->replaceSuccessorWith(LatchL1, BodyL2);
  }
  // Aggancio il body di L2 al latch di L1
  for (auto *LatchL2Pred : predecessors(LatchL2)) {
    // LatchL2Pred sono i blocchi del body di L2
    LatchL2Pred->getTerminator()->replaceSuccessorWith(LatchL2, LatchL1);
  }

  // Sostituisco nel'HeaderL2 il successore BodyL2 con LatchL2
  HeaderL2->getTerminator()->replaceSuccessorWith(BodyL2, LatchL2);

  // Aggiorno il loop L1 per includere tutti i blocchi di L2,
  // tranne HeaderL2 e LatchL2
  for (auto blocksL2 : L2->blocks()) {
    if (blocksL2 != HeaderL2 && blocksL2 != LatchL2) {
      L1->addBasicBlockToLoop(blocksL2, LI);
    }
  }

  return true;
}

bool tryFuseLoops(list<Loop *> MergeableLoops, LoopInfo &LI, DominatorTree &DT,
                  PostDominatorTree &PDT, ScalarEvolution &SE,
                  DependenceInfo &DI) {
  bool hasChanged = false;

  for (auto itLoop1 = MergeableLoops.begin(); itLoop1 != MergeableLoops.end();
       ++itLoop1) {
    auto itLoop2 = itLoop1;
    ++itLoop2; // itLoop2 parte dall'elemento successivo a itLoop1

    for (; itLoop2 != MergeableLoops.end(); ++itLoop2) {

      Loop *L1 = *itLoop1;
      Loop *L2 = *itLoop2;

      outs() << "[TRYFUSE] Checking if loops can be fused\n";
      L1->print(outs());
      outs() << "\n";
      L2->print(outs());
      outs() << "\n";

      if (!areLoopAdjacent(L1, L2)) {
        outs() << "[TRYFUSE] - Loops are not adjacent\n";
        continue;
      }
      outs() << "[TRYFUSE] + Loops are adjacent\n";

      // Loops iterate the same number of times
      if (!areLoopTripCountEquivalent(L1, L2, SE)) {
        outs() << "[TRYFUSE] - Loops do not iterate the same number of times\n";
        continue;
      }
      outs() << "[TRYFUSE] + Loops iterate the same number of times\n";

      if (!areLoopsControlFlowEquivalent(L1, L2, DT, PDT)) {
        outs() << "[TRYFUSE] Loops are not control flow equivalent\n";
        continue;
      }
      outs() << "[TRYFUSE] + Loops are control flow equivalent\n";

      if (areLoopDistanceNegative(L1, L2, DI)) {
        outs() << "[TRYFUSE] - Loops have negative distance dependencies\n";
        continue;
      }
      outs()
          << "[TRYFUSE] + Loops do not have negative distance dependencies\n";

      bool isFused = fuseLoops(L1, L2, LI);

      outs() << "[TRYFUSE] " << (isFused ? "Loops fused" : "Loops not fused")
             << "\n";
      outs() << "-----------------------------------\n";

      if (isFused) {
        LI.erase(L2);
        hasChanged = true;
      }
    }
  }

  return hasChanged;
}

// In order for two loops, Lj and Lk to be fused, they must satisfy
// the following conditions:
//   1. Lj and Lk must be adjacent
//     • There cannot be any statements that execute between the end of Lj and
//       the beginning of Lk
//   2. Lj and Lk must iterate the same number of times
//   3. Lj and Lk must be control flow equivalent
//     • When Lj executes Lk also executes or when Lk executes Lj also executes
//   4. There cannot be any negative distance dependencies
//   between Lj and Lk
//     • A negative distance dependence occurs between Lj and Lk, Lj before Lk,
//       when at iteration m from Lk uses a value that is computed by Lj at a
//       future iteration m+n (where n > 0).
PreservedAnalyses LoopFusionPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  outs() << "Running LoopFusionPass on " << F.getName() << "\n";

  LoopInfo &LI = AM.getResult<LoopAnalysis>(F);
  DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
  PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
  ScalarEvolution &SE = AM.getResult<ScalarEvolutionAnalysis>(F);
  DependenceInfo &DI = AM.getResult<DependenceAnalysis>(F);

  bool Transformed = false;
  do {
    list<Loop *> MergeableLoops = getMergeableLoops(&LI);
    if (MergeableLoops.size() < 2) {
      outs() << "No mergeable loops found\n";
      break;
    }

    Transformed = tryFuseLoops(MergeableLoops, LI, DT, PDT, SE, DI);
    if (Transformed) {
      outs() << "[RUN] Loops fused\n";
      Transformed = false;
    } else {
      outs() << "[RUN] No loops fused\n";
    }
  } while (Transformed);

  // Elimino i blocchi inutilizzati
  EliminateUnreachableBlocks(F);

  // Verifico che la funzione sia corretta
  if (verifyFunction(F, &errs())) {
    errs() << "[RUN] Error: Function verification failed after loop fusion\n";
  }

  return Transformed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}