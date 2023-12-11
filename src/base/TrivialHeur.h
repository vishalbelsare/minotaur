//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2021 The MINOTAUR Team.
//

/**
 * \file TrivialHeur.h
 * \brief Define the class TrivialHeur derived from base class Heuristics.
 * \author Mustafa Vora, Indian Institute of Technology Bombay
 *
 * Declares the class TrivialHeur.
 */

#ifndef MINOTAURTRIVIALHEUR_H
#define MINOTAURTRIVIALHEUR_H

#include "Heuristic.h"
#include "Types.h"

namespace Minotaur
{
class Problem;
/**
    * \brief A statistic struct for trivial heuristic
    */
struct TrivialHeurStats {
  bool isZero;      /// Zero solution was feasible?
  bool isLB;        /// Lower bound solution was feasible?
  bool isUB;        /// Upper bound solution was feasible?
  UInt numRand;     /// Number of random solutions feasible
  bool isLockPoint; /// Lock point solution was feasible?
};

class TrivialHeur : public Heuristic
{
public:
  /// default constructor
  TrivialHeur(EnvPtr env, ProblemPtr p);

  /// default destructor
  ~TrivialHeur();

  /// call to heuristic
  void solve(NodePtr, RelaxationPtr, SolutionPoolPtr s_pool);

  /// writing the statistics to the logger
  void writeStats(std::ostream& out) const;

protected:
  // Environment
  EnvPtr env_;

  // For printing messages
  static const std::string me_;

  // Problem
  ProblemPtr p_;

  // Statistics
  TrivialHeurStats* stats_;

  // get the lock point
  bool getLockPoint_(double* x);

  // Check whether x is feasible
  bool isFeasible_(const double* x);
};

typedef TrivialHeur* TrivialHeurPtr;
} // namespace Minotaur
#endif

// Local Variables:
// mode: c++
// eval: (c-set-style "k&r")
// eval: (c-set-offset 'innamespace 0)
// eval: (setq c-basic-offset 2)
// eval: (setq fill-column 78)
// eval: (auto-fill-mode 1)
// eval: (setq column-number-mode 1)
// eval: (setq indent-tabs-mode nil)
// End:
