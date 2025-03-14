// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 



#include <cmath>
#include <fstream>
#include <iostream>

#include "MinotaurConfig.h"
#include "FunctionUT.h"
#include "LinearFunction.h"
#include "QuadraticFunction.h"
#include "Variable.h"

CPPUNIT_TEST_SUITE_REGISTRATION(FunctionTest);
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(FunctionTest, "FunctionUT");

using namespace Minotaur;
using namespace std;


void FunctionTest::setUp()
{

  std::string vname = "common_var_name";
  for(UInt i = 0; i < 10; ++i) {
    // Create some variables
    vars_.push_back(VariablePtr(new Variable(i, i, -INFINITY, INFINITY, 
            Continuous, vname)));
  }

}


void FunctionTest::testEval()
{
  int error = 0;
  double x[2] = {1.0, 1.0};
  FunctionPtr f;

  // 7x1 - 2x2
  lf1_ = new LinearFunction();
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  f = (FunctionPtr) new Function(lf1_);
  CPPUNIT_ASSERT(f->eval(&x[0], &error)==5.0);
  CPPUNIT_ASSERT(0==error);

  double y[2] = {0.0, 1.0};
  CPPUNIT_ASSERT(f->eval(&y[0], &error)==-2.0);
  CPPUNIT_ASSERT(0==error);

  double z[4] = {1.0, 0.0, 0.0, 1.0};
  CPPUNIT_ASSERT(f->eval(&z[0], &error)==7.0);
  CPPUNIT_ASSERT(0==error);

  CPPUNIT_ASSERT(f->getNumVars()==2);
  CPPUNIT_ASSERT(lf1_->getNumTerms() == 2);
  delete f;

  // 7x1 - 2x2
  lf1_ = LinearFunctionPtr(new LinearFunction());
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  // 2 x1*x2
  qf1_ = new QuadraticFunction();
  qf1_->addTerm(VariablePair(vars_[0],vars_[1]), 2.0);
  f = new Function(lf1_, qf1_);
  CPPUNIT_ASSERT(f->getNumVars()==2);
  CPPUNIT_ASSERT(f->eval(&x[0], &error)==7.0);
  CPPUNIT_ASSERT(0==error);
  CPPUNIT_ASSERT(f->eval(&y[0], &error)==-2.0);
  CPPUNIT_ASSERT(0==error);
  CPPUNIT_ASSERT(f->eval(&z[0], &error)==7.0);
  CPPUNIT_ASSERT(0==error);
  delete f;
}

void FunctionTest::testBilinearRecognize()
{
  FunctionPtr f1;
  // 7x1 - 2x2
  lf1_ = LinearFunctionPtr(new LinearFunction());
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  f1 = new Function(lf1_);
  CPPUNIT_ASSERT(f1->getType() != Bilinear);
  delete f1;

  // 7x1 - 2x2
  lf1_ = LinearFunctionPtr(new LinearFunction());
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  // 2 x1*x2
  qf1_ = QuadraticFunctionPtr(new QuadraticFunction());
  qf1_->addTerm(VariablePair(vars_[0],vars_[1]), 2.0);
  
  f1 = new Function(lf1_, qf1_);
  CPPUNIT_ASSERT(f1->getType() == Bilinear);
  delete f1;
}

void FunctionTest::testGetFixVarOffset() 
{
  FunctionPtr f1;
  // 7x1 - 2x2
  lf1_ = LinearFunctionPtr(new LinearFunction());
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  // 2 x1*x2
  qf1_ = QuadraticFunctionPtr(new QuadraticFunction());
  qf1_->addTerm(VariablePair(vars_[0],vars_[1]), 2.0);
  
  f1 = new Function(lf1_, qf1_);
  CPPUNIT_ASSERT(f1->getFixVarOffset(vars_[0], 1.0) == 7.0); 
  delete f1;

  // 7x1 - 2x2
  lf1_ = LinearFunctionPtr(new LinearFunction());
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  // 3 x1^2 
  qf2_ = QuadraticFunctionPtr(new QuadraticFunction());
  qf2_->addTerm(VariablePair(vars_[0],vars_[0]), 3.0);
  f1 = new Function(lf1_, qf2_);
  CPPUNIT_ASSERT(f1->getFixVarOffset(vars_[0], 2.0) == 26.0);
  delete f1;
}

void FunctionTest::testEvalGradient()
{
  double y[] = {1.0, 1.0}; 
  double grad_f[] = {0.0, 0.0}; 
  double* x = &y[0];
  int error = 0;
  FunctionPtr f1;

  // 7x1 - 2x2
  lf1_ = LinearFunctionPtr(new LinearFunction());
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  f1 = new Function(lf1_);
  f1->evalGradient(x, grad_f, &error);
  CPPUNIT_ASSERT(grad_f[0] == 7.0); 
  CPPUNIT_ASSERT(grad_f[1] == -2.0);
  CPPUNIT_ASSERT(error == 0);
  delete f1;
  
  grad_f[0] = 0.0;
  grad_f[1] = 0.0;
  // 7x1 - 2x2
  lf1_ = LinearFunctionPtr(new LinearFunction());
  lf1_->addTerm(vars_[0], 7.0);
  lf1_->addTerm(vars_[1], -2.0);
  // 2 x1*x2
  qf1_ = QuadraticFunctionPtr(new QuadraticFunction());
  qf1_->addTerm(VariablePair(vars_[0],vars_[1]), 2.0);
  
  f1 = new Function(lf1_, qf1_);
  f1->evalGradient(x, grad_f, &error);    
  CPPUNIT_ASSERT(grad_f[0] == 9.0); 
  CPPUNIT_ASSERT(grad_f[1] == 0.0);
  CPPUNIT_ASSERT(error == 0);
  delete f1;
}

void FunctionTest::tearDown()
{
  int n = vars_.size();
  for (int i=0; i<n; ++i) {
    delete vars_[i];
  }
}



