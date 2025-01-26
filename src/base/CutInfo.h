//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file CutInfo.h
 * \brief Define the Cut class for valid inequalities and additional
 * information.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 */

#ifndef MINOTAURCUTINFO_H
#define MINOTAURCUTINFO_H

#include "Types.h"

namespace Minotaur {

  class Engine;
  typedef Engine* EnginePtr;

  /**
   * The Cut class is meant to manage the cutting planes generated by
   * different cut generators and handlers. This is a base class and special
   * classes of cuts can be derived from it.
   */
  class CutInfo {

  public:
    /// Default Constructor.
    CutInfo();

    /// Create a cut using a constraint.
    CutInfo(ConstraintPtr c, const double* hashval);

    /// Destroy.
    ~CutInfo();

    /// By how much does a given point x violate this cut.
    void evalScore(const double *x, double &vio, double &score);

    /// Write to out.
    void write(std::ostream &out) const;

    /// Write to std::cout.
    void write() const {write(std::cout);}

    /// Update active counts
    void updateActive(bool wasactive) {  
      if (wasactive) {
        cntSinceActive_ = 0;
        numActive_++;
      }
      else cntSinceActive_++;
    }

    /// Update viol counts
    void updateViol(bool wasviol) {
      if (wasviol) cntSinceViol_ = 0;
      else cntSinceViol_++;
    }

    /// Get basic info
    UInt getCntSinceActive() { return cntSinceActive_;}
    UInt getCntSinceViol() { return cntSinceViol_;}
    UInt getNumActive() { return numActive_;}
    double getHashVal() { return hashVal_;}

  protected:

    void evalFixedScore();

    /// Pointer to the constraint.
    ConstraintPtr cons_;

    /// True if allowed to delete the cut
    bool canDelete_;

    /// For cuts in relaxation, 
    UInt cntSinceActive_;

    /// For cuts in pool.
    UInt cntSinceViol_;

    /// Total number of relaxations where it was active.
    UInt numActive_;

    /// Score.
    double fixedScore_;

    /// To check duplicacy
    double hashVal_;


  };
  typedef CutInfo* CutInfoPtr;
  typedef std::vector< CutInfoPtr > CutInfoVector;
  typedef CutInfoVector::iterator CutInfoIterator;
  typedef CutInfoVector::const_iterator CutInfoConstIterator;
}
#endif

