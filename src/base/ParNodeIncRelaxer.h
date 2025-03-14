// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

/**
 * \file ParNodeIncRelaxer.h
 * \brief Declare the ParNodeIncRelaxer class. It creates relaxation by 
 * only incrementing changes made in ancestor nodes. All modifications to be
 * applied while creating the relaxation are fist translated appropriately.
 * \author Prashant Palkar, IIT Bombay
 */


#ifndef MINOTAURPARNODEINCRELAXER_H
#define MINOTAURPARNODEINCRELAXER_H

#include "Handler.h"
#include "NodeRelaxer.h"

namespace Minotaur {

/**
 * The root relaxation is stored as rel_. In each node, we apply all
 * modifications stored in each ancestor of the node. When we are done
 * processing the node, we undo all these changes. 
 *
 * If we dive after processing a node, we do not need to undo all changes
 * and apply them again. We just apply the modifications of the parent.
 */
class ParNodeIncRelaxer : public NodeRelaxer {
public:
  /// Default constructor
  ParNodeIncRelaxer(EnvPtr env, HandlerVector handlers);

  /// Destroy
  ~ParNodeIncRelaxer();

  // Implement NodeRelaxer::CreateRootRelaxation()
  RelaxationPtr createRootRelaxation(NodePtr rootNode, SolutionPool *sp,
                                     bool &prune);

  // Implement NodeRelaxer::CreateNodeRelaxation()
  RelaxationPtr createNodeRelaxation(NodePtr node, bool dived, bool &prune);

  /// Get the current value of modProb_ flag.
  bool getModFlag();

  // Implement NodeRelaxer::reset()
  void reset(NodePtr node, bool diving);

  /**
   * /brief Set the engine that is used to solve the relaxations. We need to set
   * it in order to be able to load warm-starts at a node.
   *
   * \param[in] e Engine that will be modified whenever a new node is about to
   * be processed.
   */
  void setEngine(EnginePtr e);

  /**
   * \brief If mod_prob is true, the problem will also be modified at each
   * node. By default, only the relaxation is modified.
   */
  void setModFlag(bool mod_prob);

  // get the relaxation pointer, rel_.
  RelaxationPtr getRelaxation();

  /// Set your own relaxation pointer.
  void setRelaxation(RelaxationPtr rel);

  /// Set the problem pointer
  void setProblem(ProblemPtr p);
private:
  /// Pointer engine used to solve the relaxation.
  EnginePtr engine_;

  /// Environment
  EnvPtr env_;

  /// Vector of handlers that will make the relaxation.
  HandlerVector handlers_;

  /**
   * True if Problem is modified in each node, false if only relaxation is
   * modified.
   */
  bool modProb_;

  /// The problem being solved by branch-and-bound.
  ProblemPtr p_;

  /**
   * \brief We only keep one relaxation. It is modified at each node and then
   * reset.
   */
  RelaxationPtr rel_;
};

typedef ParNodeIncRelaxer* ParNodeIncRelaxerPtr;
}
#endif

