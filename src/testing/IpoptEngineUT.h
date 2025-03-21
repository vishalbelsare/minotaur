// 
//     Minotaur -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

#ifndef IPOPTENGINEUT_H
#define IPOPTENGINEUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Problem.h"
#include "Jacobian.h"
#include "HessianOfLag.h"
#include "NonlinearFunction.h"

using namespace Minotaur;

class IpoptEngineUT : public CppUnit::TestCase {
  public:
    IpoptEngineUT(std::string name) : TestCase(name) {}
    IpoptEngineUT() {}

    void testGetObjVal();
    void setUp();
    void tearDown();

    CPPUNIT_TEST_SUITE(IpoptEngineUT);
    CPPUNIT_TEST(testGetObjVal);
    CPPUNIT_TEST_SUITE_END();


  private:
    ProblemPtr instance_;
    void createInstance_(EnvPtr env);
};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// objective function
class myNLFun0 : public NonlinearFunction {
  public:
    myNLFun0() {}
    NonlinearFunctionPtr cloneWithVars(VariableConstIterator,
                                       int *err) const 
    {*err = 1; return NonlinearFunctionPtr();};
    
    
    double eval(const double *x, int *error);
    void evalGradient(const double *x, double *grad_f, int *error);
    void evalHessian(const double, const double *, 
                     const LTHessStor *, double *, 
                     int *) {};
    void fillHessStor(LTHessStor *) {};
    void fillJac(const double*, double *, int*) {};
    void finalHessStor(const LTHessStor *) {};
    UInt getGradientNzCount();
    UInt getHessianNzCount();
    void getVars(VariableSet *) {};
    bool isZero() const { return true; }
    void multiply(const double) {};
    void prepJac(VarSetConstIter, VarSetConstIter) {};
};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// function in constraint 
class myNLFun1 : public NonlinearFunction {
  public:
    myNLFun1() {}
    NonlinearFunctionPtr cloneWithVars(VariableConstIterator,
                                       int *err) const 
    {*err = 1; return NonlinearFunctionPtr();};
    
    
    double eval(const double *x, int *error);
    void evalGradient(const double *, double *, int *) {}
    void evalHessian(const double, const double *, 
                     const LTHessStor *, double *, 
                     int *) {};
    void fillHessStor(LTHessStor *) {};
    void fillJac(const double*, double *, int*) {};
    void finalHessStor(const LTHessStor *) {};
    UInt getGradientNzCount();
    UInt getHessianNzCount();
    void getVars(VariableSet *) {};
    bool isZero() const { return true; }
    void multiply(const double) {};
    void prepJac(VarSetConstIter, VarSetConstIter) {};
};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// jacobian 
class myJac : public Jacobian {
  public:
    myJac();
    UInt getNumNz();
    void fillRowColIndices(UInt *iRow, UInt *jCol);
    void fillRowColValues(const double *x, double *values, int *error);


  private:
    UInt nrows_;
    UInt ncols_;

};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// hessian 
class myHess : public HessianOfLag {
  public:
    myHess();
    UInt getNumNz() const;
    void fillRowColIndices(UInt *iRow, UInt *jCol);
    void fillRowColValues(const double *x, double obj_mult, 
                          const double *con_mult, double *values, int *error);

};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //

typedef myNLFun0* myNLFun0Ptr;
typedef myNLFun1* myNLFun1Ptr;
typedef myJac* myJacPtr;
typedef myHess* myHessPtr;

#endif     // #define IPOPTENGINEUT_H

