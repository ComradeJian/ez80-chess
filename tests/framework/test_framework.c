#include "test_framework.h"

void print_test_results(const TestSuite* suite) {
    dbg_printf("\n%s Results: %d/%d tests passed\n", suite->name,
               suite->tests_run - suite->tests_failed, suite->tests_run);
}