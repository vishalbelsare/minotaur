// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2017 The MINOTAUR Team.
// 

/**
 * \file Linearizations.h
 * \Briefly declare a class for generating extra linearizations in linearization
 *  based methods.
 * \Author Meenarli Sharma, Indian Institute of Techonology Bombay.
 */

#ifndef MINOTAURLINEARIZATIONS_H
#define MINOTAURLINEARIZATIONS_H

#include <stack>

#include "Handler.h"
#include "Engine.h"
#include "Problem.h"
#include "Function.h"

#include "Solution.h"

namespace Minotaur {

  struct LinStats {
  size_t rs1Cuts; /// Number of cuts in root scheme 1.
  size_t rs2Cuts; /// Number of cuts in root scheme 2.
  size_t rgs1Cuts; /// Number of cuts in root gen scheme 1.
  size_t rgs2Cuts; /// Number of cuts in root gen scheme 2.
};

class Linearizations {

private: 
  /// Pointer to environment.
  EnvPtr env_;

  /// Pointer to relaxation of the problem.
  RelaxationPtr rel_;

  /// Pointer to original problem.
  ProblemPtr minlp_;

  /// Tolerance for checking integrality (should be obtained from env).
  double intTol_;

  /// Log.
  LoggerPtr logger_;

  /// For log:
  static const std::string me_;

  /// Vector of nonlinear constraints.
  std::vector<ConstraintPtr> nlCons_;

  /// NLP/QP Engine used to solve the NLP/QP relaxations.
  EnginePtr nlpe_;
  
  /// NLP/QP Engine used to approximate the center of the feasible region
  EnginePtr nlpe1_;
  
  /// LP Engine used in root linearization scheme 3
  EnginePtr lpe_;

  /// Modifications done to NLP before solving it.
  std::stack<Modification *> nlpMods_;

  /// Status of the NLP/QP engine.
  EngineStatus nlpStatus_;

  /**
   * The variable corresponding to the objective function. It is a part of
   * all linearizations of the objective function and it appears in the
   * objective.
   */
  VariablePtr objVar_;

  /// Nonlinearity status of objective function. 1 if nonlinear 0 otherwise.
  bool oNl_;

  /// Parameter for root lin scheme 1
  double rs1_;
  
  /// Parameters for root lin scheme 2
  double rs2Per_;

  double rs2NbhSize_;
  
  /// Parameter for root lin scheme 3
  UInt rs3_;
  
  /// Parameter for root lin scheme 1 for general problem
  bool rgs1_;
  
  /// Parameter for root lin scheme 1 for general problem
  bool rgs2_;

  /// Value of objective in relaxation solution
  double relobj_; 
  
  /// Absolute tolerance for constraint feasibility.
  double solAbsTol_;
  
  /// Approximation of the center of the feasible region
  double * solC_; 
 
  /// Solution of root NLP
  const double * nlpx_; 

  /// Relative tolerance for constraint feasibility.
  double solRelTol_;

  /// Absolute tolerance for pruning a node.
  double objATol_;

  /// Relative tolerance for pruning a node.
  double objRTol_;

  // populate this if any general scheme is on 
  std::vector<VariablePtr> varPtrs_; 

  /// Statistics.
  LinStats *stats_;

  public:
  /// Empty constructor.
  Linearizations();

  /**
   * \brief Default Constructor.
   *
   * \param [in] env Environment pointer.
   * \param [in] minlp The minlp for which cuts are generated (Not the
   * relaxation.)
   * \param [in] nlpe The engine to solve nonlinear continuous problem.
   */
  Linearizations(EnvPtr env, EnginePtr nlpe, RelaxationPtr rel,
                               ProblemPtr minlp); 
  
  /// Destroy.
  ~Linearizations();
   
  /// Root linearization schemes
  void rootLinearizations(std::vector<ConstraintPtr> nlCons,
                                        const double * nlpx);

  /**
   * Add linearizatios by performing line search between center of the
   * feasible region and the root LP solution - root linearization scheme 3
   */
  //void rootLinScheme3(ConstSolutionPtr sol, CutManager *,
                       //SolutionPoolPtr s_pool, bool *sol_found,
                       //SeparationStatus *status);

  void setLpEngine(EnginePtr lpe) {lpe_ = lpe;};

  /// Show statistics.
  void writeStats(std::ostream &out) const;


private:
 
  /// Add linearization in root linearization scheme 1 
  bool addCutAtRoot_(double *x, ConstraintPtr con, UInt &newConId);

  /// Add new cut in case of root linearization scheme 1
  bool addNewCut_(double *b1, ConstraintPtr con, 
                  UInt &newConId);

  void boundingVar_(double &varbound, UInt &pos, 
                                  double *lastDir,
                                  std::vector<double > &alphaSign);
  void boundingVar_(double &varbound,
                                  UInt vIdx, UInt fixIdx, double coeff,
                                 double fixCoeff, std::vector<double > &alphaSign, std::vector<UInt > &varIdx);
  /// Find approximate center of the feasible region
  void findCenter_(bool &noCenter);

  /// Find intersection of two linearizations in root linearization scheme 1  
  bool findIntersectPt_(std::vector<UInt > newConsId, VariablePtr vl,
                       VariablePtr vnl, double * iP);

  bool findBoundaryPt_(bool &isPtFound,
                                     double *xOut, double *xIn,
                                     std::vector<ConstraintPtr> &consToCheck);


  void setStepSize_(double &varbound, double &alpha,
                                   UInt vIdx, double value, double boundSign);

  bool foundLinPt_(UInt pos, std::vector<UInt> varIdx, std::vector<double> alphaSign, double varBound, double *xOut, bool isLast);

  /**
   * Insert a new point in the candidate list for adding linearization in
   * case of root linearization scheme 1
  */
  void insertNewPt_(UInt j, UInt k, std::vector<double > & xc, 
                    std::vector<double> & yc, ConstraintPtr con, 
                    VariablePtr vl, VariablePtr vnl, bool & shouldCont);

  /// Check feasibility of sol 
  //bool isFeas_(ConstSolutionPtr sol);

  /**
   * Obtain the linear function (lf) and constant (c) from the
   * linearization of function f at point x.
   */
  void linearAt_(FunctionPtr f, double fval, const double *x, 
                 double *c, LinearFunctionPtr *lf, int *error);

  /// Line search in root linearization scheme 3
  //bool lineSearchPt_(double* x, const double* l, const double* u,
                     //ConstraintPtr con, double & nlpact);
  
   
  /// Compute variable in the linear part 
  bool linPart_(double *b1, UInt vlIdx, ConstraintPtr con, 
                  double linVarCoeff, double nVarCoeff);

  UInt minChange_(double diff1, double diff2, double &alpha,
                  double &varbound, UInt vIdx, UInt fixIdx,
                  double coeff, double fixCoeff);

  void newPoint_(bool isLast,
                               std::vector<UInt> varIdx, double *xOut, std::vector<double> alphaSign);

  void populateVarIdx(VariablePtr v, std::vector<UInt > &varIdx,
                                      double &vMinUb, double &vMaxLb,
                                      UInt &lIdx, UInt &uIdx);
   /**
   * Add linerizations to constraints with exactly one var in the nonlinear
   * part - root linearization scheme 1
   */
  void rootLinScheme1_(ConstraintPtr con, double linTermCoeff, UInt vlIdx,
                       UInt vnIdx, double extraCoeff);
  
  /**
   * Add linearizations in the neighborhood of the root nonlinear relaxation
   * solution - root linearization scheme 2
   */
  void rootLinScheme2_(ConstraintPtr con, 
                       double lVarCoeff,
                            UInt vlIdx, UInt vnIdx);

  /**
   * Find points with reasoanble difference in curvature to add linearizaion
   * in root linearization scheme 2
   */
  void rScheme2Cut_(ConstraintPtr con, double &delta,
                                double linTermCoeff, double &lastSlope,
                                UInt vnIdx, double * npt, double * grad);

  void rootLinGenScheme1_();
  
  void rootLinGenScheme2_();

  void search_(double varbound, UInt vIdx, double val,
               std::vector<UInt > varIdx, 
                             double *xOut, bool &isFound,
                             std::vector<double > &alphaSign,
                             UInt pos, bool isLastDir);

  //bool cutAtLineSearchPt_(double *xout, double* xnew,
                                        //ConstraintPtr con);

  /// Find nonlinear constraint with only one variable in the nonlinear part
  bool uniVarNlFunc_(ConstraintPtr con, double &linTermCoeff, UInt & vlIdx,
                   UInt & vnIdx, double &extraCoeff);

  void varsInNonlinCons_();
  };

  typedef Linearizations* LinearizationsPtr;
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
