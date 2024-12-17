//===-- LocalOpts.h - Custom Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Path:
//  SRC/llvm/include/llvm/Transforms/Utils/LocalOpts.h
//===------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_LOCALOPTS_H
#define LLVM_TRANSFORMS_LOCALOPTS_H

#include "llvm/IR/Constants.h"
#include "llvm/IR/PassManager.h"

namespace llvm {
class LocalOpts : public PassInfoMixin<LocalOpts> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};
} // namespace llvm

#endif // LLVM_TRANSFORMS_LOCALOPTS_H