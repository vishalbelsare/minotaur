// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#include "MinotaurConfig.h"
#include "AMPLOsiUT.h"
#include "BranchAndBound.h"
#include "EngineFactory.h"
#include "Environment.h"
#include "IntVarHandler.h"
#include "LinearHandler.h"
#include "PCBProcessor.h"
#include "NodeIncRelaxer.h"
#include "OsiLPEngine.h"
#include "Option.h"
#include "Problem.h"
#include "ReliabilityBrancher.h"
#include "AMPLInterface.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION(AMPLOsiUT);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(AMPLOsiUT, "AMPLOsiUT");

using namespace Minotaur;


void AMPLOsiUT::setUp()
{
  env_ = (EnvPtr) new Environment();
  env_->setLogLevel(LogNone);
  iface_ = (MINOTAUR_AMPL::AMPLInterfacePtr) new AMPLInterface(env_);
  engine_ptr_ = (OsiLPEnginePtr) new OsiLPEngine(env_);
}


void AMPLOsiUT::tearDown()
{
  delete engine_ptr_;
  delete iface_;
  delete env_;
}


void AMPLOsiUT::testOsiLP()
{
  char file_name[] = "instances/lp0";
  ProblemPtr inst = iface_->readInstance(file_name);
  FunctionPtr f = FunctionPtr(); // NULL
  // LPFormulationPtr lpForm = (LPFormulationPtr) 
  //   new LPFormulation(inst);

  engine_ptr_->load(inst);
  EngineStatus status = engine_ptr_->solve();

  CPPUNIT_ASSERT(status == ProvenOptimal);
  CPPUNIT_ASSERT(engine_ptr_->getStatus() == ProvenOptimal);
  CPPUNIT_ASSERT(fabs(engine_ptr_->getSolutionValue()+8.42857) < 1e-5);

  inst->changeObj(f, 2.0);
  status = engine_ptr_->solve();
  CPPUNIT_ASSERT(status == ProvenOptimal);
  CPPUNIT_ASSERT(engine_ptr_->getStatus() == ProvenOptimal);
  CPPUNIT_ASSERT(fabs(engine_ptr_->getSolutionValue()-2.0) < 1e-5);

  inst->negateObj();
  status = engine_ptr_->solve();
  CPPUNIT_ASSERT(status == ProvenOptimal);
  CPPUNIT_ASSERT(engine_ptr_->getStatus() == ProvenOptimal);
  CPPUNIT_ASSERT(fabs(engine_ptr_->getSolutionValue()+2.0) < 1e-5);

  delete inst;

}


void AMPLOsiUT::testOsiLP2()
{
  char file_name[] = "instances/lp_eg0";
  ProblemPtr inst = iface_->readInstance(file_name);
  // LPFormulationPtr lpForm = (LPFormulationPtr)
  //   new LPFormulation(inst);

  engine_ptr_->load(inst);
  EngineStatus status = engine_ptr_->solve();

  CPPUNIT_ASSERT(status == ProvenInfeasible);
  CPPUNIT_ASSERT(engine_ptr_->getStatus() == ProvenInfeasible);
 
  delete inst;
}


void AMPLOsiUT::testOsiWarmStart()
{
  char file_name[] = "instances/lp0";
  ProblemPtr inst = iface_->readInstance(file_name);
  engine_ptr_->load(inst);
  EngineStatus status = engine_ptr_->solve();
  CPPUNIT_ASSERT(fabs(engine_ptr_->getSolutionValue()+8.42857) < 1e-5);
  CPPUNIT_ASSERT(status == ProvenOptimal);
  CPPUNIT_ASSERT(engine_ptr_->getStatus() == ProvenOptimal);

  // get warm start
  WarmStartPtr ws = engine_ptr_->getWarmStartCopy();
  EnvPtr env = (EnvPtr) new Environment();
  OsiLPEngine engine2(env);
  engine2.load(inst);
  engine2.loadFromWarmStart(ws);
  status = engine2.solve();
  CPPUNIT_ASSERT(fabs(engine2.getSolutionValue()+8.42857) < 1e-5);
  CPPUNIT_ASSERT(status == ProvenOptimal);
  CPPUNIT_ASSERT(engine2.getStatus() == ProvenOptimal);
  CPPUNIT_ASSERT(engine2.getIterationCount() == 0);

  delete ws;
  delete inst;
  delete env;
}


void AMPLOsiUT::testOsiBnB()
{
  EnvPtr env = (EnvPtr) new Environment();
  char file_name[] = "instances/milp";
  HandlerVector handlers;
  ReliabilityBrancherPtr br;
  EnginePtr e;
  int err = 0;

  env->startTimer(err);
  ProblemPtr p = iface_->readInstance(file_name);
  BranchAndBound *bab = new BranchAndBound(env, p);

  IntVarHandlerPtr v_hand = (IntVarHandlerPtr) new IntVarHandler(env, p);
  LinearHandlerPtr l_hand = (LinearHandlerPtr) new LinearHandler(env, p);
  handlers.push_back(v_hand);
  handlers.push_back(l_hand);
  v_hand->setModFlags(false, true);
  l_hand->setModFlags(false, true);

  EngineFactory efac(env);
  e = efac.getLPEngine();
  PCBProcessorPtr nproc = (PCBProcessorPtr) new PCBProcessor(env, e, handlers);
  br= (ReliabilityBrancherPtr) new ReliabilityBrancher(env, handlers);
  br->setEngine(e);
  nproc->setBrancher(br);
  bab->setNodeProcessor(nproc);

  NodeIncRelaxerPtr nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);
  bab->setNodeRelaxer(nr);
  nr->setEngine(e);
  nr->setModFlag(false);

  p->setNativeDer();
  bab->shouldCreateRoot(true);
  bab->setLogLevel(LogNone);
  bab->solve();
  CPPUNIT_ASSERT(bab->getUb() == 1.0);

  delete v_hand;
  delete l_hand;
  delete e;
  delete p;
  delete nproc;
  delete nr;
  delete bab;
  delete env;
}

