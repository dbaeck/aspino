/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/TestMain.h>
#include <cxxtest/ErrorPrinter.h>

int main( int argc, char *argv[] ) {
 int status;
    CxxTest::ErrorPrinter tmp;
    CxxTest::RealWorldDescription::_worldName = "cxxtest";
    status = CxxTest::Main< CxxTest::ErrorPrinter >( tmp, argc, argv );
    return status;
}
bool suite_AlgorithmTests_init = false;
#include "aspino-test/AlgorithmTests.h"

static AlgorithmTests suite_AlgorithmTests;

static CxxTest::List Tests_AlgorithmTests = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_AlgorithmTests( "aspino-test/AlgorithmTests.h", 13, "AlgorithmTests", suite_AlgorithmTests, Tests_AlgorithmTests );

static class TestDescription_suite_AlgorithmTests_testAddition : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AlgorithmTests_testAddition() : CxxTest::RealTestDescription( Tests_AlgorithmTests, suiteDescription_AlgorithmTests, 15, "testAddition" ) {}
 void runTest() { suite_AlgorithmTests.testAddition(); }
} testDescription_suite_AlgorithmTests_testAddition;

static class TestDescription_suite_AlgorithmTests_testVecMerge : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AlgorithmTests_testVecMerge() : CxxTest::RealTestDescription( Tests_AlgorithmTests, suiteDescription_AlgorithmTests, 21, "testVecMerge" ) {}
 void runTest() { suite_AlgorithmTests.testVecMerge(); }
} testDescription_suite_AlgorithmTests_testVecMerge;

static class TestDescription_suite_AlgorithmTests_testStdVecMerge : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AlgorithmTests_testStdVecMerge() : CxxTest::RealTestDescription( Tests_AlgorithmTests, suiteDescription_AlgorithmTests, 35, "testStdVecMerge" ) {}
 void runTest() { suite_AlgorithmTests.testStdVecMerge(); }
} testDescription_suite_AlgorithmTests_testStdVecMerge;

static class TestDescription_suite_AlgorithmTests_testVecSplit : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AlgorithmTests_testVecSplit() : CxxTest::RealTestDescription( Tests_AlgorithmTests, suiteDescription_AlgorithmTests, 49, "testVecSplit" ) {}
 void runTest() { suite_AlgorithmTests.testVecSplit(); }
} testDescription_suite_AlgorithmTests_testVecSplit;

static class TestDescription_suite_AlgorithmTests_testStdVecSplit : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_AlgorithmTests_testStdVecSplit() : CxxTest::RealTestDescription( Tests_AlgorithmTests, suiteDescription_AlgorithmTests, 63, "testStdVecSplit" ) {}
 void runTest() { suite_AlgorithmTests.testStdVecSplit(); }
} testDescription_suite_AlgorithmTests_testStdVecSplit;

#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
