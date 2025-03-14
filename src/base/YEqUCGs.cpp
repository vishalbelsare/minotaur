//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file YEqUCGs.cpp
 * \brief Define class for storing auxiliary variables for univariate
 * nonlinear functions that are expressed as computational graphs.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#include <cmath>

#include "MinotaurConfig.h"
#include "CGraph.h"
#include "CNode.h"
#include "OpCode.h"
#include "Variable.h"
#include "YEqUCGs.h"

// #define SPEW 1

using namespace Minotaur;


YEqUCGs::YEqUCGs()
{
  UInt numops = 35;
  for (UInt i=0; i<numops; ++i) {
    rand_.push_back((double) rand()/(RAND_MAX));
  }
}


YEqUCGs::~YEqUCGs()
{
  cg_.clear();
  hash_.clear();
  op_.clear();
  rand_.clear();
  x_.clear();
  y_.clear();
}


double YEqUCGs::evalHash_(const CNode* node, UInt rank)
{
  OpCode op = node->getOp();
  double hash = rand_[op]*rank;
  if (OpVar==op) {
    hash *= node->getV()->getId();
  } else if (OpInt==op || OpNum==op) {
    hash *= node->getVal();
  } else if (1==node->numChild()) {
    hash += evalHash_(node->getL(), rank+1);
  } else if (2==node->numChild()) {
    hash += evalHash_(node->getL(), rank+1);
    hash += evalHash_(node->getR(), rank+2);
  } else if (2<node->numChild()) {
    UInt i= 0;
    CNode** c1 = node->getListL();
    CNode** c2 = node->getListR();
    while (c1<c2) {
      hash += evalHash_(*c1, rank+i);
      ++i;
      ++c1;
    }
  }
  return hash;
}


VariablePtr YEqUCGs::findY(CGraphPtr cg)
{
  double hash = evalHash_(cg->getOut(), 1);
  OpCode op = cg->getOut()->getOp();
  VariablePtr x = *(cg->varsBegin());
  for (UInt i=0; i<y_.size(); ++i) {
    if (fabs(hash-hash_[i])<1e-10
        && op == op_[i]
        && x == x_[i]
        && cg->isIdenticalTo(cg_[i])) {
      return y_[i];
    }
  }
  return VariablePtr();
}


void YEqUCGs::insert(VariablePtr auxvar, CGraphPtr cg)
{
  VariablePtr x;

  assert(auxvar && cg);

  x = *(cg->varsBegin());
  op_.push_back(cg->getOut()->getOp());
  hash_.push_back(evalHash_(cg->getOut(), 1));
  y_.push_back(auxvar);
  x_.push_back(x);
  cg_.push_back(cg);
}

