//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2025 The Minotaur Team.
//

/**
 * \file SOS1Handler.h
 * \brief Declare the SOS1Handler class for handling SOS type I constraints.
 * It checks integrality and provides branching candidates. Does
 * not do any presolving and cut-generation.
 * \author Ashutosh Mahajan, IIT Bombay
 */

#ifndef MINOTAURSOS1HANDLER_H
#define MINOTAURSOS1HANDLER_H

#include "Handler.h"

namespace Minotaur {

/**
 * SOS1Handler class considers all SOS Type 1 constraints in the problem. 
 * It does not implement separate() function.  Implements functions for
 * checking feasibility and branching. 
 */
class SOS1Handler : public Handler {
public:

  /// Default constructor.
  SOS1Handler();

  /// Constructor.
  SOS1Handler(EnvPtr env, ProblemPtr problem);

  /// Destroy.
  ~SOS1Handler();

  // Does nothing.
  void relaxInitFull(RelaxationPtr rel, SolutionPool *sp, bool *is_inf);

  // Does nothing.
  void relaxInitInc(RelaxationPtr rel, SolutionPool *sp, bool *is_inf);

  // Does nothing.
  void relaxNodeFull(NodePtr node, RelaxationPtr rel, bool *is_inf);

  // Does nothing.
  void relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_inf);

  // Check if solution is feasible.
  bool isFeasible(ConstSolutionPtr sol, RelaxationPtr relaxation, 
                  bool &should_prune, double &inf_meas);

  /// Empty method as we do not do any separation in SOS1.
  void separate(ConstSolutionPtr sol, NodePtr node, RelaxationPtr rel, 
                CutManager *cutman, SolutionPoolPtr s_pool, ModVector &p_mods,
                ModVector &r_mods, bool *sol_found, SeparationStatus *status);
  // Base class method.
  void getBranchingCandidates(RelaxationPtr rel, const DoubleVector &x,
                              ModVector & mods, BrVarCandSet &cands,
                              BrCandVector &gencands, bool & is_inf);

  // Base class method.
  ModificationPtr getBrMod(BrCandPtr cand, DoubleVector &x, 
                           RelaxationPtr rel, BranchDirection dir);

  //  Base class method.
  Branches getBranches(BrCandPtr cand, DoubleVector & x,
                       RelaxationPtr rel, SolutionPoolPtr s_pool);
      
  bool isGUB(SOS *sos);
  bool isNeeded();

  /// Presolve. Do not do any presolving.
  SolveStatus presolve(PreModQ *, bool *, Solution **) {return Finished;};

  /// Does nothing.
  void postsolveGetX(const double *, UInt, DoubleVector *);

  /// Does nothing.
  bool presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &,
                    ModVector &)
  {return false;};

  // Write name
  std::string getName() const;

  void getNzNumSum_(SOSPtr sos, const DoubleVector x, int *nz,
                    double *nzsum);

  /// Does nothing.
  void writePreStats(std::ostream &) const {};

  /// Return the integer tolerance.
  double getTol() const;

  /// Set the integer tolerance.
  void setTol(double tol);

private:
  /// Environment.
  EnvPtr env_;

  /// Log
  LoggerPtr logger_;

  /// For log:
  static const std::string me_;

  /// The problem for which the handler was created.
  ProblemPtr problem_;

  /**
   * \brief Tolerance for checking integrality.
   *
   * If abs(x) < zTol_, then it is considered to be zero.
   */
  double zTol_;
};
typedef SOS1Handler* SOS1HandlerPtr;
typedef const SOS1Handler* ConstSOS1HandlerPtr;
}
#endif

