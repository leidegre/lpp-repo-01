#include "test.h"

#include "stdio.h"

static test_context s_test_context;

void test_init(int argc, char *argv[]) {
  memset(&s_test_context, 0, sizeof(test_context));
  s_test_context.argc_ = argc;
  s_test_context.argv_ = argv;
}

void test_exit() {
  test_case *test = s_test_context.last_;

  if (test) {
    test_end(test);
  }

  puts(""); // new line

  int passed = 0, failed = 0, skipped = 0;

  while (test) {
    switch (test->status_) {
    case TEST_STATUS_NOT_RUN: {
      skipped++;
      break;
    }
    case TEST_STATUS_OK: {
      passed++;
      break;
    }
    case TEST_STATUS_FAIL: {
      failed++;
      break;
    }
    default:
      break; // whatever
    }
    test = test->next_;
  }

  printf("%i %s passed, %i %s failed, %i %s skipped\n", passed,
         passed == 1 ? "test" : "tests", failed, failed == 1 ? "test" : "tests",
         skipped, skipped == 1 ? "test" : "tests");
}

void test_register(test_case *test) {
  test->next_ = s_test_context.last_;
  s_test_context.last_ = test;
}

int test_eval(test_case *test) {
  if (test->next_) {
    test_end(test->next_);
  }

  // 1: run test; 0: skip test
  int eval = 1;

  for (int i = 1; i < s_test_context.argc_; i++) {
    if (strncmp(s_test_context.argv_[i], "-", 1) == 0) {
      i++;
      continue;
    } else {
      eval = 0;
      break;
    }
  }

  if (eval == 0) {
    for (int i = 1; i < s_test_context.argc_; i++) {
      if (strncmp(s_test_context.argv_[i], "-", 1) == 0) {
        i++;
        continue;
      } else if (strcmp(s_test_context.argv_[i], test->name_) == 0) {
        eval = 1;
        break;
      }
    }
  }

  if (eval == 1) {
    test_begin(test, TEST_STATUS_OK);
  } else {
    test_begin(test, TEST_STATUS_NOT_RUN);
  }

  return eval;
}

void test_begin(test_case *test, test_status status) {
  s_test_context.curr_ = test;
  s_test_context.curr_->status_ = status;
  putchar('.'); // progress indicator
  fflush(stdout);
}

void test_end(test_case *test) {
  // ...
}
