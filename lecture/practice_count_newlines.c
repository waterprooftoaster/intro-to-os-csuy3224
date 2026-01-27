/*
practice_count_newlines.c

SPEC:
  int count_newlines(const char *buf, int n);

  - Treat buf as RAW BYTES (like from read), NOT a C string.
  - Return the number of '\n' characters in the first n bytes.
  - Do NOT stop at '\0'. Do NOT call strlen.
  - If n <= 0, return 0.
  - Assume buf is non-NULL when n > 0.

HOW TO RUN (local):
  gcc -std=c99 -Wall -Wextra -O0 practice_count_newlines.c && ./a.out
*/

#include <stdio.h>
#include <string.h>

static int tests_failed = 0;

static void expect_eq_int(const char *name, int got, int expected) {
  if (got != expected) {
    printf("FAIL %-22s got=%d expected=%d\n", name, got, expected);
    tests_failed++;
  } else {
    printf("PASS %-22s\n", name);
  }
}

int count_newlines(const char *buf, int n) {
  if (n <= 0)
    return 0;
  int count = 0;
  for (int i = 0; i < n; i++) {
    if (buf[i] == '\n')
      count++;
  }
  return count;
}

int main(void) {
  // Easy tests
  expect_eq_int("n=0", count_newlines("abc", 0), 0);
  expect_eq_int("no newlines", count_newlines("abc", 3), 0);
  expect_eq_int("one newline", count_newlines("a\nb", 3), 1);
  expect_eq_int("two newlines", count_newlines("\n\n", 2), 2);

  // Boundary: count only first n bytes
  expect_eq_int("prefix only", count_newlines("a\nb\nc", 3), 1); // "a\nb"
  expect_eq_int("all bytes", count_newlines("a\nb\nc", 5), 2);

  // Raw bytes: contains '\0' in the middle (must still count '\n')
  const char raw[] = {'x', '\0', '\n', 'y', '\n', 'z'};
  expect_eq_int("raw bytes", count_newlines(raw, 6), 2);

  if (tests_failed == 0) {
    printf("\nALL TESTS PASSED ✅\n");
    return 0;
  } else {
    printf("\n%d TEST(S) FAILED ❌\n", tests_failed);
    return 1;
  }
}