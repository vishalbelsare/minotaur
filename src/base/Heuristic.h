//
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2024 The Minotaur Team.
//

/**
 * \file Heuristic.h
 * \brief Define abstract base class for heuristics of various kinds.
 * \author Ashutosh Mahajan, Argonne National Laboratory
 * 
 * Defines the base class Heuristic. 
 */

#ifndef MINOTAURHEURISTIC_H
#define MINOTAURHEURISTIC_H

#include "Types.h"
#include "SolutionPool.h"

namespace Minotaur {

  class   Node;
  class   Relaxation;
  typedef Relaxation* RelaxationPtr;


  /**
   */
  class Heuristic {
    public:

      /// Default constructor.
      Heuristic() {};

      /// Destroy
      virtual ~Heuristic() {};

      /// Use this heuristic.
      virtual void solve(NodePtr node, RelaxationPtr rel, 
          SolutionPoolPtr s_pool) = 0;

      /// Use this heuristic.
      virtual void solveNode(ConstSolutionPtr , NodePtr , RelaxationPtr , 
          SolutionPoolPtr )
      {}
  

      /// Write statistics to the logger.
      virtual void writeStats(std::ostream &out) const = 0;
  };

}
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
