/*
practice_process_chunk.c

SPEC:

  typedef void (*line_cb)(const unsigned char *line, int len, void *ctx);

  void process_chunk(const unsigned char *buf, int len,
                     unsigned char **carry, int *carry_len,
                     line_cb on_line, void *ctx);

  - Treat buf as RAW BYTES (may contain '\0'). len is how many bytes are valid.
  - Conceptual stream = old carry bytes (if any) followed by buf[0..len).
  - For EVERY COMPLETE LINE ending in '\n':
        call on_line(line_ptr, line_len, ctx)
    where line_len includes the trailing '\n'.

  - After processing:
      *carry / *carry_len should become the bytes AFTER the last '\n' (may be
empty). carry must be malloc'd (or NULL if empty). You must free the previous
*carry if you replace it.

  - Do NOT use strlen / strchr.
  - Assume carry and carry_len pointers are non-NULL.
  - If len <= 0, treat buf as empty (still process carry if needed).

HOW TO RUN (local):
  gcc -std=c99 -Wall -Wextra -O0 practice_process_chunk.c && ./a.out
PAIR-PROGRAMMING TIP:
  Pass tests 1 & 2 first, then test 3 (carry joins), then test 4 (embedded
'\0').

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*line_cb)(const unsigned char *line, int len, void *ctx);

static int tests_failed = 0;

static void fail(const char *name, const char *msg) {
  printf("FAIL %-22s %s\n", name, msg);
  tests_failed++;
}

static void pass(const char *name) { printf("PASS %-22s\n", name); }

static int bytes_eq(const unsigned char *a, int alen, const unsigned char *b,
                    int blen) {
  return (alen == blen) && (memcmp(a, b, (size_t)alen) == 0);
}

/* Capture emitted lines into a small in-memory log. */
struct capture {
  unsigned char lines[16][64];
  int lens[16];
  int count;
};

static void on_line_capture(const unsigned char *line, int len, void *ctx) {
  struct capture *cap = (struct capture *)ctx;
  if (cap->count >= 16)
    return; // keep it simple for class
  if (len < 0 || len > 64)
    return;
  memcpy(cap->lines[cap->count], line, (size_t)len);
  cap->lens[cap->count] = len;
  cap->count++;
}

/* ===== TODO: students implement this ===== */
void process_chunk(const unsigned char *buf, int len, unsigned char **carry,
                   int *carry_len, line_cb on_line, void *ctx) {
  // TODO:
  // 1) Build a temporary "stream" = old carry + new buf (raw bytes)
  // 2) Scan stream for '\n'
  // 3) For each '\n', emit the line [start..i] inclusive via on_line(...)
  // 4) Whatever remains after the last '\n' becomes the new carry
  // 5) Free old *carry if replaced; set *carry=NULL when empty

  // Iterate through buf, call on_line for each line found

  int start = 0;
  for (int i = 0; i < len; i++) {
    if (buf[i] == '\n') {
      on_line(&buf[start], i - start + 1, ctx);
      int blah = i - start + 1;
      printf("%d\n", blah);
      printf("test test\n");
      start = i + 1;
    }
  }

  (void)buf;
  (void)len;
  (void)carry;
  (void)carry_len;
  (void)on_line;
  (void)ctx;
}
/* ======================================== */

static void reset_capture(struct capture *cap) { cap->count = 0; }

static void expect_line(struct capture *cap, int idx, const unsigned char *exp,
                        int exp_len, const char *testname) {
  if (idx >= cap->count) {
    fail(testname, "missing expected line");
    return;
  }
  if (!bytes_eq(cap->lines[idx], cap->lens[idx], exp, exp_len)) {
    fail(testname, "line bytes mismatch");
  }
}

static void reset_carry(unsigned char **carry, int *carry_len) {
  free(*carry);
  *carry = NULL;
  *carry_len = 0;
}

static void expect_carry(const unsigned char *carry, int carry_len,
                         const unsigned char *exp, int exp_len,
                         const char *testname) {
  if (carry_len != exp_len) {
    fail(testname, "carry length mismatch");
    return;
  }
  if (exp_len == 0) {
    if (carry != NULL) {
      fail(testname, "carry should be NULL when length=0");
    }
    return;
  }
  if (carry == NULL) {
    fail(testname, "carry is NULL but expected bytes");
    return;
  }
  if (memcmp(carry, exp, (size_t)exp_len) != 0) {
    fail(testname, "carry bytes mismatch");
  }
}

int main(void) {
  unsigned char *carry = NULL;
  int carry_len = 0;
  struct capture cap = {0};

  /* Test 1: No newline => everything becomes carry */
  reset_capture(&cap);
  const unsigned char t1[] = {'a', 'b', 'c'};
  process_chunk(t1, 3, &carry, &carry_len, on_line_capture, &cap);
  if (cap.count != 0)
    fail("t1 no newline", "should emit 0 lines");
  else
    pass("t1 no newline");
  const unsigned char t1c[] = {'a', 'b', 'c'};
  expect_carry(carry, carry_len, t1c, 3, "t1 carry");

  /* Test 2: Simple split in one chunk */
  reset_carry(&carry, &carry_len);
  reset_capture(&cap);
  const unsigned char t2[] = {'a', '\n', 'b', '\n'};
  process_chunk(t2, 4, &carry, &carry_len, on_line_capture, &cap);
  if (cap.count != 2)
    fail("t2 simple split", "expected 2 lines");
  else
    pass("t2 simple split");
  const unsigned char l0[] = {'a', '\n'};
  const unsigned char l1[] = {'b', '\n'};
  expect_line(&cap, 0, l0, 2, "t2 line0");
  expect_line(&cap, 1, l1, 2, "t2 line1");
  expect_carry(carry, carry_len, NULL, 0, "t2 carry");

  /* Test 3: Carry joins next chunk to form a complete line */
  // carry currently empty. Let's set up a carry by sending partial line:
  reset_capture(&cap);
  const unsigned char t3a[] = {'h', 'e', 'l'};
  process_chunk(t3a, 3, &carry, &carry_len, on_line_capture, &cap);
  // No lines emitted, carry="hel"
  reset_capture(&cap);
  const unsigned char t3b[] = {'l', 'o', '\n', 'x'};
  process_chunk(t3b, 4, &carry, &carry_len, on_line_capture, &cap);
  // Expect one emitted line "hello\n" and carry "x"
  if (cap.count != 1)
    fail("t3 carry join", "expected 1 line");
  else
    pass("t3 carry join");
  const unsigned char l3[] = {'h', 'e', 'l', 'l', 'o', '\n'};
  expect_line(&cap, 0, l3, 6, "t3 line0");
  const unsigned char t3c[] = {'x'};
  expect_carry(carry, carry_len, t3c, 1, "t3 carry");

  /* Test 4: Embedded '\\0' inside a line (raw bytes) */
  reset_capture(&cap);
  const unsigned char t4[] = {'p', '\0', 'q', '\n', 'r'};
  process_chunk(t4, 5, &carry, &carry_len, on_line_capture, &cap);
  if (cap.count != 1)
    fail("t4 raw bytes", "expected 1 line");
  else
    pass("t4 raw bytes");
  const unsigned char l4[] = {'x', 'p', '\0', 'q',
                              '\n'}; // Wait: carry currently "x" from t3
  // Careful: stream is carry("x") + t4 => first line should be "xp\0q\n"
  expect_line(&cap, 0, l4, 5, "t4 line0");
  const unsigned char t4c[] = {'r'};
  expect_carry(carry, carry_len, t4c, 1, "t4 carry");

  free(carry);

  if (tests_failed == 0) {
    printf("\nALL TESTS PASSED ✅\n");
    return 0;
  } else {
    printf("\n%d TEST(S) FAILED ❌\n", tests_failed);
    return 1;
  }
}