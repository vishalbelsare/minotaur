//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008 - 2017 The MINOTAUR Team.
// 

/*! \brief Single-tree Outer-approximation algorithm with Lazy callback for solving convex MINLP
 *
 * \authors Meenarli Sharma and Prashant Palkar, IIT Bombay
 */


#ifdef USE_CPX
#include <CplexMILPEngine.h> //(why sequence matters?)
#endif

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sys/time.h>

#include <MinotaurConfig.h>
#include <AMPLHessian.h>
#include <AMPLJacobian.h>
#include <Environment.h>
#include <Handler.h>
#include <Option.h>
#include <Problem.h>
#include <Engine.h>
#include <QPEngine.h>
#include <LPEngine.h>
#include <Logger.h>
#include <MILPEngine.h>
#include "Modification.h"
#include <NLPEngine.h>
#include "NlPresHandler.h"
#include <NodeRelaxer.h>
#include <NodeIncRelaxer.h>
#include <MaxVioBrancher.h>
#include <STOAHandler.h>
#include <ReliabilityBrancher.h>
#include <AMPLInterface.h>
#include <BranchAndBound.h>
#include <PCBProcessor.h>
#include <Presolver.h>
#include <Timer.h>
#include <LexicoBrancher.h>
#include <Logger.h>
#include <LinearHandler.h>
#include <IntVarHandler.h>
#include <Solution.h>
#include <SolutionPool.h>
#include <TreeManager.h>
#include <EngineFactory.h>
#include <CxQuadHandler.h>
#include <Objective.h>

#include <Relaxation.h> //(why needed to included here?)


using namespace Minotaur;

EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p);
void writeSol(EnvPtr env, VarVector *orig_v, PresolverPtr pres,
              SolutionPtr sol, SolveStatus status,
              MINOTAUR_AMPL::AMPLInterface* iface);


void loadProblem(EnvPtr env, MINOTAUR_AMPL::AMPLInterface* iface,
                 ProblemPtr &oinst, double *obj_sense)
{
  Timer *timer     = env->getNewTimer();
  OptionDBPtr options = env->getOptions();
  JacobianPtr jac;
  HessianOfLagPtr hess;
  const std::string me("qg: ");

  timer->start();
  oinst = iface->readInstance(options->findString("problem_file")->getValue());
  env->getLogger()->msgStream(LogInfo) << me 
    << "time used in reading instance = " << std::fixed 
    << std::setprecision(2) << timer->query() << std::endl;

  // display the problem
  oinst->calculateSize();
  if (options->findBool("display_problem")->getValue()==true) {
    oinst->write(env->getLogger()->msgStream(LogNone), 12);
  }
  if (options->findBool("display_size")->getValue()==true) {
    oinst->writeSize(env->getLogger()->msgStream(LogNone));
  }
  // create the jacobian
  if (false==options->findBool("use_native_cgraph")->getValue()) {
    jac = (MINOTAUR_AMPL::AMPLJacobianPtr) 
      new MINOTAUR_AMPL::AMPLJacobian(iface);
    oinst->setJacobian(jac);

    // create the hessian
    hess = (MINOTAUR_AMPL::AMPLHessianPtr)
      new MINOTAUR_AMPL::AMPLHessian(iface);
    oinst->setHessian(hess);
  }

  // set initial point
  oinst->setInitialPoint(iface->getInitialPoint(), 
      oinst->getNumVars()-iface->getNumDefs());

  if (oinst->getObjective() &&
      oinst->getObjective()->getObjectiveType()==Maximize) {
    *obj_sense = -1.0;
    env->getLogger()->msgStream(LogInfo) << me 
      << "objective sense: maximize (will be converted to Minimize)"
      << std::endl;
  } else {
    *obj_sense = 1.0;
    env->getLogger()->msgStream(LogInfo) << me 
      << "objective sense: minimize" << std::endl;
  }

  delete timer;
}


void setInitialOptions(EnvPtr env)
{
  env->getOptions()->findBool("presolve")->setValue(true);
  env->getOptions()->findBool("use_native_cgraph")->setValue(true);
  env->getOptions()->findBool("nl_presolve")->setValue(true);
  env->getOptions()->findBool("separability")->setValue(false);
  env->getOptions()->findBool("perspective")->setValue(false);
}


void showHelp()
{
  std::cout << "Outer-approximation algorithm for convex MINLP"
            << std::endl
            << "Usage:" << std::endl
            << "To show version: oa -v (or --show_version yes) " << std::endl
            << "To show all options: oa -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: oa --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}


int showInfo(EnvPtr env)
{
  OptionDBPtr options = env->getOptions();
  const std::string me("oa: ");

  if (options->findBool("display_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    return 1;
  }

  if (options->findBool("display_help")->getValue() ||
      options->findFlag("?")->getValue()) {
    showHelp();
    return 1;
  }

  if (options->findBool("display_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    env->getLogger()->msgStream(LogNone) << me <<
      "Minotaur version " << env->getVersion() << std::endl;
    env->getLogger()->msgStream(LogNone) << me 
      << "Outer-approximation algorithm for convex MINLP" << std::endl;
    return 1;
  }

  if (options->findString("problem_file")->getValue()=="") {
    showHelp();
    return 1;
  }

  env->getLogger()->msgStream(LogInfo)
    << me << "Minotaur version " << env->getVersion() << std::endl
    << me << "Outer-approximation algorithm for convex MINLP"
    << std::endl;
  return 0;
}


/// Get wall clock time
double getWallTime() {
  struct timeval time;
  if (gettimeofday(&time,NULL)) {
    // Handle error
    return 0;
  }
  return (double)time.tv_sec + (double)time.tv_usec * .000001;
}


PresolverPtr presolve(EnvPtr env, ProblemPtr p, size_t ndefs, 
                      HandlerVector &handlers)
{
  PresolverPtr pres = PresolverPtr(); // NULL
  const std::string me("qg: ");

  p->calculateSize();
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    LinearHandlerPtr lhandler = (LinearHandlerPtr) new LinearHandler(env, p);
    handlers.push_back(lhandler);
    if (p->isQP() || p->isQuadratic() || p->isLinear() ||
        true==env->getOptions()->findBool("use_native_cgraph")->getValue()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
      lhandler->setPreOptCoeffImp(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
      lhandler->setPreOptCoeffImp(false);
    }
    if (ndefs>0) {
      lhandler->setPreOptDualFix(false);
    } else {
      lhandler->setPreOptDualFix(true);
    }

    if (!p->isLinear() && 
        true==env->getOptions()->findBool("use_native_cgraph")->getValue() && 
        true==env->getOptions()->findBool("nl_presolve")->getValue() 
       ) {
      NlPresHandlerPtr nlhand = (NlPresHandlerPtr) new NlPresHandler(env, p);
      handlers.push_back(nlhand);
    }

    // write the names.
    env->getLogger()->msgStream(LogExtraInfo) << me 
      << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); 
        ++h) {
      env->getLogger()->msgStream(LogExtraInfo) << me 
        << (*h)->getName() << std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(p, env, handlers);
  pres->standardize(); 
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    pres->solve();
  }
  return pres;
}



double getPerGap(double objLb, double objUb)
{
  // for minimization problems, gap = (ub - lb)/(ub) * 100
  // so that if one has a ub, she can say that the solution can not be more
  // than gap% away from the current ub.
  double gap = 0.0, etol = 1e-6;
  if (objUb >= INFINITY) {
    gap = INFINITY;
  } else if ((objUb > etol) && (fabs(objLb) < etol)) {
    gap = 100.0;
  } else {
    gap = (objUb - objLb)/(fabs(objUb)+etol) 
      * 100.0;
    if (gap<0.0) {
      gap = 0.0;
    }
  }
  return gap;
}


void writeSTOAStatus(EnvPtr env, double gap, double objLb, double objUb, 
                   double obj_sense, SolveStatus status, double wallTimeStart)
{

  const std::string me("oa main: ");
  int err = 0;

  env->getLogger()->msgStream(LogInfo)
    << me << std::fixed << std::setprecision(4) 
    << "best solution value = " << obj_sense*objUb << std::endl
    << me << std::fixed << std::setprecision(4)
    << "best bound estimate = "
    <<  obj_sense*objLb << std::endl
    << me << "gap = " << std::max(0.0, objUb - objLb)
    << std::endl
    << me << "gap percentage = " << gap << std::endl
    //<< me << "iterations = " << iterNum << std::endl
    << me << "time used (s) = " << std::fixed << std::setprecision(2) 
    //<< env->getTime(err) << std::endl
    << getWallTime() - wallTimeStart << std::endl
    << me << "status of outer approximation: " << getSolveStatusString(status) << std::endl;
  env->stopTimer(err); assert(0==err);
}


void showStatus(EnvironmentPtr env, double objLb, double objUb, double gap, double obj_sense)
{
  int err = 0;
  env->getLogger()->msgStream(LogInfo)
    << "oa main: " 
    << std::fixed
    << std::setprecision(1)  << "time = "  << env->getTime(err)
    << std::setprecision(4)  << " lb = "   << objLb
    << std::setprecision(4)  << " ub = "   << objUb
    << std::setprecision(2)  << " gap% = " << gap
    << std::setprecision(0)  << " obj_sense = " << obj_sense
    //<< " iterations = " << iterNum 
    << std::endl;
}


bool shouldStop(EnvironmentPtr env, SolveStatus &status, double gap, SolutionPoolPtr solPool)
//bool shouldStop(EnvironmentPtr env, SolveStatus &status, double gap, SolutionPoolPtr solPool)
{
  bool stop_oa = false;
  int err = 0;

  if (gap <= 0.0) {
    stop_oa = true;
    status = SolvedOptimal;
  } else if ( gap <= env->getOptions()->findDouble("obj_gap_percent")->getValue()) {
    stop_oa = true;
    status = SolvedGapLimit;
  } else if (env->getTime(err) > env->getOptions()->findDouble("bnb_time_limit")->getValue()) {
    stop_oa = true;
    status = TimeLimitReached;
  //} else if (iterNum >= env->getOptions()->findInt("oa_iter_limit")->getValue()) {
    //stop_oa = true;
    //status = IterationLimitReached;
  } else if (solPool->getNumSolsFound() >= (UInt) env->getOptions()->findInt("bnb_sol_limit")->getValue()) {
    stop_oa = true;
    status = SolLimitReached;
  }

  return stop_oa;
}


int main(int argc, char* argv[])
{
  EnvPtr env = (EnvPtr) new Environment();
  OptionDBPtr options;

  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();
  ProblemPtr inst;
  SolutionPtr sol2;
  double obj_sense =1.0, gap = INFINITY;
  
  // jacobian is read from AMPL interface and passed on to branch-and-bound
  JacobianPtr jPtr;
  // hessian is read from AMPL interface and passed on to branch-and-bound
  MINOTAUR_AMPL::AMPLHessianPtr hPtr;

  // the branch-and-bound
  PresolverPtr pres;
  EngineFactory *efac;
  const std::string me("oa: ");
  SolveStatus status;
  //handlers
  HandlerVector handlers;
  IntVarHandlerPtr v_hand;
  LinearHandlerPtr l_hand;
  STOAHandlerPtr stoa_hand;
  EngineStatus engineStatus;
  ModVector pmod, rmod;

  //engines
  EnginePtr nlp_e;
  
  
  VarVector *orig_v=0;
  int err = 0;
  double wallTimeStart = getWallTime();

#ifdef USE_CPX
  CplexMILPEnginePtr milp_e = new CplexMILPEngine(env);
#else
  MILPEnginePtr milp_e = 0;
  env->getLogger()->errStream() << me << "CPLEX MILP Engine not found, exiting" << std::endl;
  goto CLEANUP;
#endif

  // start timing.
  env->startTimer(err);
  if (err) {
    goto CLEANUP;
  }

  setInitialOptions(env);

  iface = (MINOTAUR_AMPL::AMPLInterfacePtr) 
    new MINOTAUR_AMPL::AMPLInterface(env, "qg");

  // parse options
  env->readOptions(argc, argv);
  options = env->getOptions();
  options->findString("interface_type")->setValue("AMPL");

  if (0!=showInfo(env)) {
    goto CLEANUP;
  }

  loadProblem(env, iface, inst, &obj_sense);

  // Separability detection
  //sepDetection(env, inst);

  //Linearize objective if nonlinear
  //linearizeObj(inst);


  // Initialize engines
  nlp_e = getNLPEngine(env, inst); //Engine for Original problem

  efac = new EngineFactory(env);
  //lp_e = efac->getLPEngine();   // lp engine 
  //milp_e = efac->getMILPEngine();   // milp engine
  //milp_e = (CplexMILPEnginePtr) new CplexMILPEngine(env);
  
  delete efac;

  // get presolver.
  orig_v = new VarVector(inst->varsBegin(), inst->varsEnd());
  pres = presolve(env, inst, iface->getNumDefs(), handlers);
  handlers.clear();
  if (Finished != pres->getStatus() && NotStarted != pres->getStatus()) {
    env->getLogger()->msgStream(LogInfo) << me 
      << "status of presolve: " 
      << getSolveStatusString(pres->getStatus()) << std::endl;
    writeSol(env, orig_v, pres, SolutionPtr(), pres->getStatus(), iface);
    goto CLEANUP;
  }
 
   if (options->findBool("solve")->getValue()==true) {
    if (true==options->findBool("use_native_cgraph")->getValue()) {
      inst->setNativeDer();
    }
    // Initialize the handlers for branch-and-cut
    l_hand = (LinearHandlerPtr) new LinearHandler(env, inst);
    l_hand->setModFlags(false, true);
    handlers.push_back(l_hand);
    assert(l_hand);

    SolutionPoolPtr solPool = (SolutionPoolPtr) new SolutionPool(env, inst, 1);
    
    stoa_hand = (STOAHandlerPtr) new STOAHandler(env, inst, nlp_e, milp_e, solPool);
    stoa_hand->setModFlags(false, true);
    handlers.push_back(stoa_hand);
    assert(stoa_hand);
  
    // Initialize the handlers for STOA
    //CutManager *cutMan = NULL;
    //bool solFound, shouldPrune;
    //double inf_meas;
    //UInt iterNum = 0;
    ConstSolutionPtr sol;
    //SeparationStatus sepStatus;
    bool prune = false;
    RelaxationPtr milp = RelaxationPtr();
    // Only store bound-changes of relaxation (not problem)
  
    NodeIncRelaxerPtr nr;
    nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
    nr->setModFlag(false);
    milp = nr->createRootRelaxation(NodePtr(), prune);
    nr->setEngine(milp_e); 
 
    //double solAbsTol = env->getOptions()->findDouble("solAbs_tol")->getValue();
    //double solRelTol = env->getOptions()->findDouble("solRel_tol")->getValue();
    //! initialize the MILP master problem by copying variables & linear constraints and by 
    //linearizing nonlinear constraints at the solution of NLP relaxation of the problem
  
    double objLb = -INFINITY, objUb = INFINITY;
    status = Started;

    //MS: Also look relTol if UB =0
    //milp_e->load(milp);
    //stoa_hand->solveMILP(&objLb, &sol, solPool, cutMan);
#ifdef USE_CPX
    engineStatus = milp_e->solveSTLazy(&objLb, &sol2, stoa_hand, &status);
#else
    env->getLogger()->errStream() << me << "CPLEX MILP Engine not found, exiting" << std::endl;
    goto CLEANUP;
#endif
    env->getLogger()->msgStream(LogDebug) << "Engine status :" 
      << engineStatus << std::endl;
 
    //iterNum++;
    //MS: different MILP engine status like unbounded, infeasible, and error to be handled
    //solFound  = stoa_hand->isFeasible(sol, RelaxationPtr(), shouldPrune, inf_meas); 
    //if (sol2) {
      //const double *x = sol2->getPrimal();
      //solPool->addSolution(x, sol2->getObjValue());
      //objUb = solPool->getBestSolutionValue();
      ////status = SolvedOptimal;
      //gap = 0;
      ////showStatus(env, objLb, objUb, gap);
    //}
    objUb = solPool->getBestSolutionValue();
    gap = getPerGap(objLb, objUb);
    showStatus(env, objLb, objUb, gap, obj_sense);
    writeSTOAStatus(env, gap, objLb, objUb, obj_sense, status, wallTimeStart);
    nlp_e->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    milp_e->writeStats(env->getLogger()->msgStream(LogExtraInfo));

    for (HandlerVector::iterator it=handlers.begin(); it!=handlers.end();
         ++it) {
      (*it)->writeStats(env->getLogger()->msgStream(LogExtraInfo));
    }
    //MS: Other solve status and right way of writing them
    //writeSol(env, orig_v, pres, solPool->getBestSolution(), status, iface);
   }

CLEANUP:
  if (iface) {
    delete iface;
  }
  if (orig_v) {
    delete orig_v;
  }
  return 0;
}


EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr(); // NULL
  bool cont=false;

  p->calculateSize();
  if (p->isLinear()) {
    e = efac->getLPEngine();
    if (e) {
      delete efac;
      return e;
    } else {
      cont = true;
    }
  }

  if (true==cont || p->isQP()) {
    e = efac->getQPEngine();
    if (e) {
      delete efac;
      return e;
    } else {
      cont = true;
    }
  }

  e = efac->getNLPEngine();

  assert (e || (!"No engine available for this problem."));
  delete efac;
  return e;
}


void writeSol(EnvPtr env, VarVector *orig_v,
              PresolverPtr pres, SolutionPtr sol, SolveStatus status,
              MINOTAUR_AMPL::AMPLInterface* iface)
{
  if (sol) {
    sol = pres->getPostSol(sol);
    iface->writeSolution(sol, status);
  }

  if (env->getOptions()->findFlag("AMPL")->getValue() ||
      true == env->getOptions()->findBool("write_sol_file")->getValue()) {
    iface->writeSolution(sol, status);
  } else if (sol && env->getLogger()->getMaxLevel()>=LogExtraInfo) {
    sol->writePrimal(env->getLogger()->msgStream(LogExtraInfo), orig_v);
  }
}





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
//
