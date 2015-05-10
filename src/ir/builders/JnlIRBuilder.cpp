#include <iostream>
#include <sstream>
#include <stdexcept>

#include "JnlIRBuilder.h"
#include "Registers.h"
#include "SMT2Lib.h"
#include "SymbolicElement.h"


JnlIRBuilder::JnlIRBuilder(uint64_t address, const std::string &disassembly):
  BaseIRBuilder(address, disassembly) {
}


void JnlIRBuilder::imm(AnalysisProcessor &ap, Inst &inst) const {
  SymbolicElement   *se;
  std::stringstream expr, sf, of;
  uint64_t          imm   = std::get<1>(this->operands[0]);
  uint64_t          symSF = ap.getRegSymbolicID(ID_SF);
  uint64_t          symOF = ap.getRegSymbolicID(ID_OF);

  /* Create the SMT semantic */
  if (symSF != UNSET)
    sf << "#" << std::dec << symSF;
  else
    sf << smt2lib::bv(ap.getRegisterValue(ID_SF), 1);

  if (symOF != UNSET)
    of << "#" << std::dec << symOF;
  else
    of << smt2lib::bv(ap.getRegisterValue(ID_OF), 1);

  /* 
   * Finale expr
   * JNL: Jump if not less (SF=OF).
   * SMT: (= sf of)
   */
  expr << smt2lib::ite(
            smt2lib::equal(
                sf.str(),
                of.str()
            ),
            smt2lib::bv(imm, REG_SIZE_BIT),
            smt2lib::bv(this->nextAddress, REG_SIZE_BIT));

  /* Create the symbolic element */
  se = ap.createRegSE(expr, ID_RIP, "RIP");

  /* Add the constraint in the PathConstraints list */
  ap.addPathConstraint(se->getID());

  /* Add the symbolic element to the current inst */
  inst.addElement(se);
}


void JnlIRBuilder::reg(AnalysisProcessor &ap, Inst &inst) const {
  OneOperandTemplate::stop(this->disas);
}


void JnlIRBuilder::mem(AnalysisProcessor &ap, Inst &inst) const {
  OneOperandTemplate::stop(this->disas);
}


void JnlIRBuilder::none(AnalysisProcessor &ap, Inst &inst) const {
  OneOperandTemplate::stop(this->disas);
}


Inst *JnlIRBuilder::process(AnalysisProcessor &ap) const {
  this->checkSetup();

  Inst *inst = new Inst(ap.getThreadID(), this->address, this->disas);

  try {
    this->templateMethod(ap, *inst, this->operands, "JNL");
    ap.incNumberOfExpressions(inst->numberOfElements()); /* Used for statistics */
  }
  catch (std::exception &e) {
    delete inst;
    throw;
  }

  return inst;
}
