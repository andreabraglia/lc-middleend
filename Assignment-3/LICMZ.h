//===-- LICMZ.h - Custom Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Path:
//  SRC/llvm/include/llvm/Transforms/Utils/LICMZ.h
//===--------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_LICMZ_H
#define LLVM_TRANSFORMS_LICMZ_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class LICMZ : public PassInfoMixin<LICMZ> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &LAM,
                        LoopStandardAnalysisResults &LAR, LPMUpdater &LU);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_LICMZ_H