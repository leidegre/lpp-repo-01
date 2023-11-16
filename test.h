#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum test_status {
  TEST_STATUS_NOT_RUN,
  TEST_STATUS_OK,
  TEST_STATUS_FAIL,
} test_status;

typedef struct test_case test_case;

struct test_case {
  const char *name_;
  test_case *next_; // The test case that came before this
  test_status status_;
};

typedef struct test_context {
  int argc_;
  char **argv_;
  test_case *curr_;
  test_case *last_; // Most recently registered test
} test_context;

void test_init(int argc, char **argv);
void test_exit();
void test_register(test_case *test);
int test_eval(test_case *test);
void test_begin(test_case *test, test_status status);
void test_end(test_case *test);

#define _TEST_CONCAT2(a, b) a##b
#define _TEST_CONCAT(a, b) _TEST_CONCAT2(a, b)

#define TEST_CASE(label)                                                       \
  test_case _TEST_CONCAT(test_, __LINE__) = {label, NULL,                      \
                                             TEST_STATUS_NOT_RUN};             \
  test_register(&_TEST_CONCAT(test_, __LINE__));                               \
  if (test_eval(&_TEST_CONCAT(test_, __LINE__)))

#ifdef __cplusplus
}
#endif