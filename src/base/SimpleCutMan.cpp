//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file SimpleCutMan.cpp
 * \brief Implement the methods of SimpleCutMan class.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#include <cmath>
#include <iostream>

#include "MinotaurConfig.h"
#include "Environment.h"
#include "Function.h"
#include "Cut.h"
#include "Logger.h"
#include "Problem.h"
#include "SimpleCutMan.h"
#include "Solution.h"
#include "Types.h"

//#define SPEW 1

using namespace Minotaur;

typedef std::list<CutPtr>::iterator CLIter;
typedef std::vector<ConstraintPtr>::const_iterator ConstIter;

const std::string SimpleCutMan::me_ = "SimpleCutMan: ";


SimpleCutMan::SimpleCutMan()
  : env_(EnvPtr()),    // NULL
    p_(ProblemPtr()),  // NULL
    violAbs_(1e-4),
    violRel_(1e-3)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


SimpleCutMan::SimpleCutMan(EnvPtr env, ProblemPtr p)
  : env_(env),
    p_(p),
    violAbs_(1e-4),
    violRel_(1e-3)
{
  logger_ = (LoggerPtr) new Logger(LogDebug2);
}


SimpleCutMan::~SimpleCutMan()
{
  newCuts_.clear();
}


void SimpleCutMan::addCut(CutPtr c)
{
  newCuts_.push_back(c);
}


ConstraintPtr SimpleCutMan::addCut(ProblemPtr p, FunctionPtr f, double lb,
                                   double ub, bool, bool)
{
  CutPtr c = (CutPtr) new Cut(p->getNumVars(), f, lb, ub, true, true);
  addCut(c);
  return ConstraintPtr();
}


void SimpleCutMan::addCuts(CutVectorIter cbeg, CutVectorIter cend)
{
  for (CutVectorIter it = cbeg; it != cend; ++it) {
    addCut(*it);
  }
}


size_t SimpleCutMan::getNumCuts() const
{
  return enCuts_ + newCuts_.size();
}


size_t SimpleCutMan::getNumEnabledCuts() const
{
  return enCuts_;
}


size_t SimpleCutMan::getNumDisabledCuts() const
{
  return 0;
}


size_t SimpleCutMan::getNumNewCuts() const
{
  return newCuts_.size();
}


void SimpleCutMan::mvNewToPool_()
{
  for (CLIter it = newCuts_.begin(); it != newCuts_.end(); ++it) {
    pool_.push_back(*it);
  }
}


void SimpleCutMan::postSolveUpdate(ConstSolutionPtr, EngineStatus)
{
}


void SimpleCutMan::separate(ProblemPtr p, ConstSolutionPtr sol, bool *,
                            size_t *)
{
  const double *x = sol->getPrimal();
  CutPtr cut;
  double viol, act;
  int err;

  mvNewToPool_();

  for (CLIter it = pool_.begin(); it != pool_.end();) {
    cut = *it;
#if SPEW
    cut->write(logger_->msgStream(LogInfo));
#endif
    err = 0;
    act = cut->eval(x, &err);
    if (err != 0) {
      logger_->msgStream(LogInfo)
          << me_ << "Error evaluating activity of cut. "
          << "Not adding to relaxation. Cut is: " << std::endl;
      ++it;
      continue;
    }
    viol = std::max(cut->getLb() - act, act - cut->getUb());
    if (viol > violAbs_ + violRel_ * fabs(act)) {
#if SPEW
      logger_->msgStream(LogInfo)
          << me_ << "Solution violates cut, "
          << "adding cut to relaxation. Cut is: " << std::endl;
#endif
      ++enCuts_;
      cut->applyToProblem(p);
      it = pool_.erase(it);
    } else {
#if SPEW
      logger_->msgStream(LogInfo) << me_ << "Solution does not violate cut. "
                                  << "Cut is: " << std::endl;
#endif
      ++it;
    }
  }
}


void SimpleCutMan::write(std::ostream &out) const
{
  out << me_ << "Nothing to write" << std::endl;
}


void SimpleCutMan::writeStats(std::ostream &out) const
{
  out << me_ << "No stats availale" << std::endl;
}
