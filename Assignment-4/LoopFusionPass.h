//===-- LoopFusionPass.h - Custom Transformations ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Path:
//  SRC/llvm/include/llvm/Transforms/Utils/LoopFusionPass.h
//===----------------------------------------------------------------===//
#ifndef LLVM_TRANSFORMS_LoopFusionPass_H
#define LLVM_TRANSFORMS_LoopFusionPass_H

#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

namespace llvm {
class LoopFusionPass : public PassInfoMixin<LoopFusionPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_LoopFusionPass_H