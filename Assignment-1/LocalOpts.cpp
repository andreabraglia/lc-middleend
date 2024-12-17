//===-- LocalOpts.cpp - Custom Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Path:
//  SRC/llvm/lib/Transforms/Utils/LocalOpts.cpp
//===--------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

/**
0. Sostitiusce l'operazioni di moltiplicazione
  che ha tra gli operandi una costante che
  è una potenza di 2 con una shift (strength reduction)
*/
bool mutltipicationStrengthReduction(Instruction &Inst,
                                     Instruction::BinaryOps OptType) {
  auto *BinOp = dyn_cast<BinaryOperator>(&Inst);
  if (!BinOp)
    return false;

  if (BinOp->getOpcode() != Instruction::Mul &&
      BinOp->getOpcode() != Instruction::UDiv &&
      BinOp->getOpcode() != Instruction::SDiv)
    return false;

  int pos = 0;
  // Ciclo su tutti gli operandi dell'istruzione
  for (auto operand = Inst.op_begin(); operand != Inst.op_end();
       operand++, pos++) {

    ConstantInt *IntOperand = dyn_cast<ConstantInt>(operand);
    if (!IntOperand)
      continue;

    APInt IntVal = IntOperand->getValue();

    // Se l'operando non è una costante che è una potenza di 2 si passa
    // all'iterazione successiva
    if (!IntVal.isPowerOf2()) {
      continue;
    }

    outs() << "Strength Reduction for Multiplication\n"
           << IntVal << " in  position " << pos << "\n";

    // Calcolo il valore dello shift
    int shiftValue = IntVal.exactLogBase2();

    // Creo l'istruzione di shift
    Instruction *Shifted = BinaryOperator::Create(
        OptType == Instruction::Mul ? Instruction::Shl : Instruction::AShr,
        Inst.getOperand(1 - pos),                           // L'altro operando
        ConstantInt::get(IntOperand->getType(), shiftValue) // Costante di shift
    );

    // Inserisco l'istruzione di shift dopo Inst
    Shifted->insertAfter(&Inst);
    // Sostituisco tutte le occorrenze dell'istruzione Inst con lo shift
    Inst.replaceAllUsesWith(Shifted);

    outs() << "Shifted\n";
    Shifted->print(outs());
    outs() << "\nResult";
    Inst.print(outs());
    outs() << "\n";
    return true;
  }

  return false;
}

/**
1. Algebraic Identity
  𝑥 - 0 = 0 - 𝑥 -> -𝑥
  𝑥 + 0 = 0 + 𝑥 -> 𝑥
  𝑥 × 1 = 1 × 𝑥 -> x
  x / 1 -> x
*/
bool algebraicIdentity(Instruction &Inst, Instruction::BinaryOps OptType) {
  // Se l'operazione è una divisione(S -> signed || U -> unsigned)
  if (OptType == Instruction::UDiv || OptType == Instruction::SDiv) {
    APInt IntVal = (dyn_cast<ConstantInt>(Inst.getOperand(1)))->getValue();

    // Se il divisore è 1
    if (IntVal.isOne()) {
      outs() << "Algebraic-Identity for Division\n"
             << IntVal << " in  position 2"
             << "\n";

      Inst.replaceAllUsesWith(Inst.getOperand(0));
      return true;
    }

    // Se il divisore è -1
    if (IntVal.isAllOnes()) {
      outs() << "Algebraic-Identity for Division\n"
             << IntVal << " in  position 2"
             << "\n";
      APInt OtherOperandVal =
          (dyn_cast<ConstantInt>(Inst.getOperand(0)))->getValue();
      OtherOperandVal.setSignBit();

      Inst.replaceAllUsesWith(
          ConstantInt::get(Inst.getOperand(0)->getType(), OtherOperandVal));
    }

    return false;
  }

  int pos = 0;
  for (auto operand = Inst.op_begin(); operand != Inst.op_end();
       operand++, pos++) {

    ConstantInt *IntOperand = dyn_cast<ConstantInt>(operand);
    if (!IntOperand)
      continue;

    APInt IntVal = IntOperand->getValue();

    if ((OptType == Instruction::Mul && IntVal.isOne()) ||
        (OptType == Instruction::Add && IntVal.isZero()) ||
        (OptType == Instruction::Sub && IntVal.isZero() && pos == 1)) {

      outs() << "Algebraic-Identity \nFor value " << IntVal << " in position "
             << pos << "\n";
      Inst.print(outs());
      outs() << "\n";

      Inst.replaceAllUsesWith(Inst.getOperand(1 - pos));

      outs() << "After\n";
      // Inst.print(outs());
      outs() << "\n";

      return true;
    }

    outs() << "Algebraic-Identity not found\n";
    Inst.print(outs());
    outs() << "\n";
  }

  return false;
}

/**
2. Strength Reduction (più avanzato)
  15 × 𝑥 = 𝑥 × 15   -> (𝑥 ≪ 4) – x
  y = x / 8         -> y = x >> 3
*/
bool strengthReduction(Instruction &Inst, Instruction::BinaryOps OptType) {
  auto *BinOp = dyn_cast<BinaryOperator>(&Inst);
  if (!BinOp)
    return false;

  if (BinOp->getOpcode() != Instruction::Mul &&
      BinOp->getOpcode() != Instruction::UDiv &&
      BinOp->getOpcode() != Instruction::SDiv)
    return false;

  int pos = 0;
  for (auto operand = Inst.op_begin(); operand != Inst.op_end();
       operand++, pos++) {

    ConstantInt *IntOperand = dyn_cast<ConstantInt>(operand);
    if (!IntOperand)
      continue;

    APInt IntVal = IntOperand->getValue();
    Instruction::BinaryOps complOpt;

    int shiftValue = 0;
    if ((IntVal - 1).isPowerOf2()) {
      shiftValue = (IntVal - 1).exactLogBase2();
      complOpt = Instruction::Add;
    } else if ((IntVal + 1).isPowerOf2()) {
      shiftValue = (IntVal + 1).exactLogBase2();
      complOpt = Instruction::Sub;
    } else {
      continue;
    }

    Instruction *Shifted = BinaryOperator::CreateShl(
        Inst.getOperand(1 - pos),
        ConstantInt::get(IntOperand->getType(), shiftValue));

    Instruction *ComplInstruction =
        BinaryOperator::Create(complOpt, Inst.getOperand(1 - pos), Shifted);

    Shifted->insertAfter(&Inst);
    ComplInstruction->insertAfter(Shifted);
    Inst.replaceAllUsesWith(ComplInstruction);

    outs() << "Strength Reduction for Multiplication\n"
           << IntVal << " in  position " << pos << "\n";
    outs() << "Shifted\n";
    Shifted->print(outs());
    outs() << "\nComplInstruction\n";
    ComplInstruction->print(outs());
    outs() << "\nResult";
    Inst.print(outs());
    outs() << "\n";

    return true;
  }

  return false;
}

/**
3. Multi-Instruction Optimization
  𝑎 = 𝑏 + 1, 𝑐 = 𝑎 − 1
  ->
  𝑎 = 𝑏 + 1, 𝑐 = 𝑏
*/
bool multiInstructionOptimization(Instruction &Inst,
                                  Instruction::BinaryOps OptType) {
  int pos = 0;
  for (auto operand = Inst.op_begin(); operand != Inst.op_end();
       operand++, pos++) {
    ConstantInt *IntOperand = dyn_cast<ConstantInt>(operand);
    if (!IntOperand)
      continue;

    APInt IntVal = IntOperand->getValue();
    Instruction::BinaryOps complOpt =
        OptType == Instruction::Add ? Instruction::Sub : Instruction::Add;

    for (auto user = Inst.user_begin(); user != Inst.user_end(); user++) {
      BinaryOperator *UserBinOp = dyn_cast<BinaryOperator>(*user);
      if (!UserBinOp)
        continue;

      for (auto userOperand = user->op_begin(); userOperand != user->op_end();
           userOperand++) {
        ConstantInt *userIntOperand = dyn_cast<ConstantInt>(userOperand);
        if (userIntOperand && (userIntOperand->getValue() == IntVal &&
                               UserBinOp->getOpcode() == complOpt)) {
          user->replaceAllUsesWith(Inst.getOperand(1 - pos));
          outs() << "Multi-Instruction Optimization\n";
          Inst.print(outs());
          outs() << "\n";
          return true;
        }
      }
    }
  }

  return false;
}

bool runOnBasicBlock(BasicBlock &BB) {
  bool Transformed = false;

  // Ciclo su tutte le istruzioni del blocco base
  for (auto &Inst : BB) {
    auto *BinOp = dyn_cast<BinaryOperator>(&Inst);
    if (!BinOp)
      continue;

    Instruction::BinaryOps Opcode = BinOp->getOpcode(); // Tipo di operazione

    switch (Opcode) {
    case Instruction::Sub:
    case Instruction::Add: {
      if (multiInstructionOptimization(Inst, Opcode)) {
        Transformed = true;
        break;
      }
      outs() << "Instruction not multiInstructionOptimized \n";
      Inst.print(outs());
      outs() << "\n";
      if (algebraicIdentity(Inst, Opcode)) {
        Transformed = true;
      }

      break;
    }
    case Instruction::Mul: {
      if (algebraicIdentity(Inst, Opcode) || strengthReduction(Inst, Opcode)) {
        Transformed = true;
      }

      if (mutltipicationStrengthReduction(Inst, Opcode)) {
        Transformed = true;
      }

      break;
    }
    case Instruction::UDiv:
    case Instruction::SDiv: {
      if (strengthReduction(Inst, Opcode) || algebraicIdentity(Inst, Opcode)) {
        Transformed = true;
      }

      if (mutltipicationStrengthReduction(Inst, Opcode)) {
        Transformed = true;
      }
      break;
    }
    default:
      break;
    }
    outs() << "Instruction\n";
    Inst.print(outs());
    outs() << "\n---------------------------------------------\n";
  }

  // Rimuove le operazioni binarie senza utilizzi
  for (auto Inst = BB.begin(); Inst != BB.end(); Inst++) {
    BinaryOperator *BinOp = dyn_cast<BinaryOperator>(Inst);
    if (BinOp && Inst->hasNUses(0)) {
      Inst = Inst->eraseFromParent();
      Inst--;
    }
  }

  outs() << "Basic block is trasformed: " << Transformed << "\n";
  return Transformed;
}

bool runOnFunction(Function &F) {
  bool Transformed = false;

  for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
      outs() << "++ Function is trasformed\n";
    }
  }

  return Transformed;
}

PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  bool Transformed = false;
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter) {
    if (runOnFunction(*Fiter)) {
      Transformed = true;
      outs() << "+++ Module is trasformed\n";
    }
  }

  return (Transformed ? PreservedAnalyses::none() : PreservedAnalyses::all());
}
