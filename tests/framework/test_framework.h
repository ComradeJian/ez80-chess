#pragma once

#include <debug.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test suite tracking information
 *
 * Maintains the state and results of a group of related tests.
 * Tests are organized into suites for logical grouping and
 * separate results reporting.
 */
typedef struct {
    const char* name;    /**< Suite identifier for reporting */
    size_t tests_run;    /**< Number of tests executed */
    size_t tests_failed; /**< Number of failed tests */
} TestSuite;

/**
 * @brief Initialize a new test suite
 *
 * Creates a named test suite with counters zeroed.
 * @example: INIT_TEST_SUITE(EXAMPLE_TESTS)
 */
#define INIT_TEST_SUITE(name) TestSuite name = {#name, 0, 0}

/* Test framework macros */

/**
 * @brief Begin a test suite execution block
 *
 * Initializes suite counters and dbg_printf()s header.
 */
#define TEST_SUITE(suite)                                                                          \
    dbg_printf("\nRunning %s...\n", suite.name);                                                   \
    suite.tests_run = 0;                                                                           \
    suite.tests_failed = 0;                                                                        \
    /* Prevents compiler warning; Compiler seems to lose track of usage across macro expansions */ \
    __attribute__((unused)) size_t test_counter = 0

/**
 * @brief Define an individual test case
 *
 * Creates a new test case within a suite.
 *
 * @note Must be matched with END_TEST_CASE:
 * ```c
 * TEST_CASE(NAMEOF_TESTS, "Test group descriptor") {
 *     ASSERT(NAMEOF_TESTS, condition);
 *     ASSERT(NAMEOF_TESTS, condition2);
 *     print_test_results(NAMEOF_TESTS);
 * } END_TEST_CASE(NAMEOF_TESTS);
 * ```
 */
#define TEST_CASE(suite, name)                                                                     \
    do {                                                                                           \
        dbg_printf("  %s: ", name);                                                                \
        suite.tests_run++;                                                                         \
        test_counter++;                                                                            \
        bool test_failed = false;                                                                  \
        {

#define END_TEST_CASE(suite)                                                                       \
    }                                                                                              \
    if (test_failed) {                                                                             \
        suite.tests_failed++;                                                                      \
        dbg_printf("FAILED\n");                                                                    \
    } else {                                                                                       \
        dbg_printf("passed\n");                                                                    \
    }                                                                                              \
    }                                                                                              \
    while (0)

/**
 * @brief Assert a test condition
 *
 * Checks a condition and records failure if false.
 * Provides detailed failure information including location.
 */
#define ASSERT(suite, condition)                                                                   \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            test_failed = true;                                                                    \
            dbg_printf("\n    Assertion failed: %s\n    File %s, Line %d\n", #condition, __FILE__, \
                       __LINE__);                                                                  \
        }                                                                                          \
    } while (0)

/**
 * @brief Print test suite results
 * @param suite Suite to report on
 *
 * Outputs summary of tests run and failed.
 */
void print_test_results(const TestSuite* suite);

#ifdef __cplusplus
}
#endif