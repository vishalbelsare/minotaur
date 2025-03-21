// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file BrCand.h
 * \author Ashutosh Mahajan, IIT Bombay
 * \brief Declare the classes BranchCand  storing candidates for branching.
 */

#ifndef MINOTAURBRCAND_H
#define MINOTAURBRCAND_H

#include <string>
#include "Types.h"

namespace Minotaur {

class Handler;
class Variable;

/**
 * \brief Base class for describing candidates for branching on a node in
 * branch-and-bound.
 *
 * A BranchCand object is something that can be branched upon. This class is
 * abstract. Examples are BrVarCand, BrSOS1Cand, BrSOS2Cand, BrHyperCand, 
 * BrGenCand etc.
 */
class BrCand {
public:    
  /// Constructor
  BrCand();

  /// Destroy
  virtual ~BrCand() {};

  /**
   * \brief Decrement the number of branches currently associated with this candidate.
   */
  virtual void decrBranches() { --branches_; };

  /**
   * Return the distance of the current point from the branching constraint:
   * down direction. For an integer constrained variable, x, it would be
   * \f$ x - \lfloor x \rfloor \f$.
   */
  virtual double getDDist() = 0;

  /// Get the preferred direction.
  virtual BranchDirection getDir() const {return prefDir_;};

  /// Return the handler that created this candidate.
  virtual HandlerPtr getHandler() { return h_; };

  /// Display for debugging.
  virtual std::string getName() const {return "default br-cand";};

  /**
   * \brief Return the index in the pseudo cost array. If it is not in the
   * array, return a value less than 0.
   */
  virtual int getPCostIndex() const { return pCostIndex_; };

  /// Return the score for this candidate. 
  virtual double getScore() { return score_; };

  /**
   * Return the distance of the current point from the branching constraint:
   * up direction For an integer constrained variable, x, it would be \f$
   * x - \lceil x \rceil \f$.
   */
  virtual double getUDist() = 0;

  /**
   * \brief Return the number of branches currently associated with this candidate.
   */
  virtual int numBranches() { return branches_; };

  /**
   * \brief Set the preferred direction that will be processed first in
   * the branch-and-bound tree.
   */
  virtual void setDir(BranchDirection d) {prefDir_ = d;};

  /**
   * \brief Set the distance of the current solution from the down branch and
   * the up branch.
   */
  void setDist(double ddist, double udist);

  /// Set the handler that created this candidate.
  virtual void setHandler(HandlerPtr h) { h_ = h; };

  /**
   * \brief Set the number of branches associated with this candidate.
   */
  virtual void setNumBranches(int n) { branches_ = n; };

  /**
   * \brief Set score for this candidate.
   *
   * The score is used to compare two candidates. 
   * \param [in] score The desired score of this candidate.
   */
  virtual void setScore(const double score) { score_ = score; };

protected:
  /// Handler that created this candidate.
  HandlerPtr h_;

  /// Number of branches still in the tree which were created by this candidate.
  int branches_;

  /// Index of the this candidate in the pseudo-cost array.
  int pCostIndex_;

  /// Which is preferred for processing next? UpBranch or DownBranch?
  BranchDirection prefDir_;

  /// Score of this candidate.
  double score_;
};   

/**
 * \brief Comparison function to sort candidates in the decreasing order of
 * their score. 
 */
bool CompareScore(BrCandPtr c1, BrCandPtr c2); 
}

#endif

