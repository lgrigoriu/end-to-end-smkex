#include "MpTests.h"
#include "MpTestsBuddy.h"

MpTests::MpTests() {

	testBuddy = new MpTestsBuddy();
	testSuiteList.push_back(testBuddy->testSuite);

	testAutoResendEngine = new MpTestAutoResendEngine();
	testSuiteList.push_back(testAutoResendEngine->testSuite);
}

void MpTests::Run() {

	int failedTests = 0;
	int testResult = 0;

	for (std::vector<MpTestSuiteData*>::iterator it = testSuiteList.begin();
			it != testSuiteList.end(); ++it) {

		MpTestSuiteData* suiteData = (MpTestSuiteData*) (*it);

		// print test suite
		std::cout << "*** " << suiteData->testSuiteName.c_str() << " ***"
				<< std::endl;

		int testNumber = 1;

		for (std::vector<MpTestData>::iterator it2 =
				(suiteData->testsList).begin();
				it2 != (suiteData->testsList).end(); ++it2) {

			// print test name
			std::cout << testNumber++ << "." << (*it2).testName.c_str()
					<< "...";

			testResult = (*it2).testFunction();

			if (testResult == 0) {
				std::cout << "OK";
			} else {
				std::cout << "FAILED!";
				failedTests++;
			}

			std::cout << endl;
		}

		std::cout << endl;
	}

	if (failedTests == 0) {
		std::cout << "ALL TESTS PASSED!";
	} else {
		std::cout << "TESTS FAILED:" << failedTests;
	}
}

MpTests::~MpTests() {
}

