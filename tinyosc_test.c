#include "tinyosc.h"

#include "test_utils.h"

#define CAPACITY 256

static int buffers_match(const char *p, const char *q, int n) {
  int i;
  for (i = 0; i < n; ++i) {
    if (p[i] != q[i]) {
      int j;
      for (j = 0; j < n; ++j) {
        fprintf(stderr, "%x ", p[j]);
      }
      fprintf(stderr, "\n");
      for (j = 0; j < n; ++j) {
        fprintf(stderr, "%x ", q[j]);
      }
      fprintf(stderr, "\n");
      return 0;
    }
  }
  return 1;
}

static int test_pack_errors() {
  char data[CAPACITY];
  osc_packet packet;
  packet.data = data;

  EXPECT(osc_pack_message(&packet, CAPACITY, "", "") != 0);
  EXPECT(osc_pack_message(&packet, CAPACITY, "#", "") != 0);
  EXPECT(osc_pack_message(&packet, CAPACITY, "#foo", "") != 0);
  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo", "_") != 0);
  
  return 0;
}

static int test_pack_capacity() {
  char data[CAPACITY];
  osc_packet packet;
  packet.data = data;

  EXPECT(osc_pack_message(&packet, 0, "/ab", "") != 0);
  EXPECT(osc_pack_message(&packet, 4, "/ab", "") != 0);
  EXPECT(osc_pack_message(&packet, 8, "/ab", "") == 0);
  EXPECT(osc_pack_message(&packet, 4, "/foo", "") != 0);
  EXPECT(osc_pack_message(&packet, 8, "/foo", "") != 0);
  EXPECT(osc_pack_message(&packet, 12, "/foo", "") == 0);
  EXPECT(osc_pack_message(&packet, 12, "/foo", "i", 0) != 0);
  EXPECT(osc_pack_message(&packet, 16, "/foo", "i", 0) == 0);
  EXPECT(osc_pack_message(&packet, 12, "/foo", "f", 1.5) != 0);
  EXPECT(osc_pack_message(&packet, 16, "/foo", "f", 1.5) == 0);
  EXPECT(osc_pack_message(&packet, 12, "/foo", "s", "abc") != 0);
  EXPECT(osc_pack_message(&packet, 16, "/foo", "s", "abc") == 0);
  EXPECT(osc_pack_message(&packet, 16, "/foo", "s", "abcd") != 0);
  EXPECT(osc_pack_message(&packet, 20, "/foo", "s", "abcd") == 0);
  EXPECT(osc_pack_message(&packet, 20, "/foo", "b", 5, "abcde") != 0);
  EXPECT(osc_pack_message(&packet, 24, "/foo", "b", 5, "abcde") == 0);
  EXPECT(osc_pack_message(&packet, 16, "/foo", "ii", 0, 0) != 0);
  EXPECT(osc_pack_message(&packet, 20, "/foo", "ii", 0, 0) == 0);
  EXPECT(osc_pack_message(&packet, 16, "/foo", "fi", 1.5, 0) != 0);
  EXPECT(osc_pack_message(&packet, 20, "/foo", "fi", 1.5, 0) == 0);
  EXPECT(osc_pack_message(&packet, 16, "/foo", "si", "abc", 0) != 0);
  EXPECT(osc_pack_message(&packet, 20, "/foo", "si", "abc", 0) == 0);
  EXPECT(osc_pack_message(&packet, 24, "/foo", "bi", 5, "abcde", 0) != 0);
  EXPECT(osc_pack_message(&packet, 28, "/foo", "bi", 5, "abcde", 0) == 0);

  return 0;
}

static int test_pack_no_args() {
  char data[CAPACITY];
  osc_packet packet;
  packet.data = data;

  EXPECT(osc_pack_message(&packet, CAPACITY, "/", "") == 0);

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "") == 0);
  EXPECT(packet.size == 8);
  char ref0[] = { '/', 'a', 'b', 0, ',', 0, 0, 0 };
  EXPECT(buffers_match(ref0, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/bar", "") == 0);
  EXPECT(packet.size == 16);
  char ref1[] = {
    '/', 'f', 'o', 'o', '/', 'b', 'a', 'r', 0, 0, 0, 0,
    ',', 0, 0, 0
  };
  EXPECT(buffers_match(ref1, packet.data, packet.size));

  return 0;
}

static int test_pack_one_arg() {
  char data[CAPACITY];
  osc_packet packet;
  packet.data = data;

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "i", 0x12345678) == 0);
  EXPECT(packet.size == 12);
  char ref0[] = {
    '/', 'a', 'b', 0, ',', 'i', 0, 0,
    0x12, 0x34, 0x56, 0x78
  };
  EXPECT(buffers_match(ref0, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/abc", "m", 0x01020304) == 0);
  EXPECT(packet.size == 16);
  char ref1[] = {
    '/', 'a', 'b', 'c', 0, 0, 0, 0, ',', 'm', 0, 0,
    0x01, 0x02, 0x03, 0x04
  };
  EXPECT(buffers_match(ref1, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "f", 1.234) == 0);
  EXPECT(packet.size == 12);
  char ref2[] = {
    '/', 'a', 'b', 0, ',', 'f', 0, 0,
    0x3f, 0x9d, 0xf3, 0xb6
  };
  EXPECT(buffers_match(ref2, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "s", "") == 0);
  EXPECT(packet.size == 12);
  char ref3[] = {
    '/', 'a', 'b', 0, ',', 's', 0, 0,
    0, 0, 0, 0
  };
  EXPECT(buffers_match(ref3, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "s", "xyz") == 0);
  EXPECT(packet.size == 12);
  char ref4[] = {
    '/', 'a', 'b', 0, ',', 's', 0, 0,
    'x', 'y', 'z', 0
  };
  EXPECT(buffers_match(ref4, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "s", "abcdefg") == 0);
  EXPECT(packet.size == 16);
  char ref5[] = {
    '/', 'a', 'b', 0, ',', 's', 0, 0,
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 0
  };
  EXPECT(buffers_match(ref5, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "b", 0, NULL) == 0);
  EXPECT(packet.size == 12);
  char ref6[] = {
    '/', 'a', 'b', 0, ',', 'b', 0, 0,
    0, 0, 0, 0
  };
  EXPECT(buffers_match(ref6, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "b", 2, "abcdef") == 0);
  EXPECT(packet.size == 16);
  char ref7[] = {
    '/', 'a', 'b', 0, ',', 'b', 0, 0,
    0, 0, 0, 2, 'a', 'b', 0, 0
  };
  EXPECT(buffers_match(ref7, packet.data, packet.size));

  return 0;
}

static int test_pack_two_args() {
  char data[CAPACITY];
  osc_packet packet;
  packet.data = data;

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "ii", 0x01020304, -2) == 0);
  EXPECT(packet.size == 16);
  char ref0[] = {
    '/', 'a', 'b', 0, ',', 'i', 'i', 0,
    0x01, 0x02, 0x03, 0x04,
    0xff, 0xff, 0xff, 0xfe
  };
  EXPECT(buffers_match(ref0, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "fi", 440.0, -2) == 0);
  EXPECT(packet.size == 16);
  char ref1[] = {
    '/', 'a', 'b', 0, ',', 'f', 'i', 0,
    0x43, 0xdc, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xfe
  };
  EXPECT(buffers_match(ref1, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "si", "x", -2) == 0);
  EXPECT(packet.size == 16);
  char ref2[] = {
    '/', 'a', 'b', 0, ',', 's', 'i', 0,
    'x', 0, 0, 0,
    0xff, 0xff, 0xff, 0xfe
  };
  EXPECT(buffers_match(ref2, packet.data, packet.size));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/ab", "bi", 2, "abc", -2) == 0);
  EXPECT(packet.size == 20);
  char ref3[] = {
    '/', 'a', 'b', 0, ',', 'b', 'i', 0,
    0x00, 0x00, 0x00, 0x02,
    'a', 'b', 0, 0,
    0xff, 0xff, 0xff, 0xfe
  };
  EXPECT(buffers_match(ref3, packet.data, packet.size));

  return 0;
}

static int test_unpack_match() {
  osc_packet packet;
  packet.size = 4;
  packet.data = "/xy";

  EXPECT(osc_unpack_message(&packet, "/xyz", "") != 0);
  EXPECT(osc_unpack_message(&packet, "/xy", "i", NULL) != 0);
  EXPECT(osc_unpack_message(&packet, "/xy", "") == 0);

  char data[CAPACITY];
  packet.data = data;

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/*/bar", "") == 0);

  // Non-matching addresses.
  EXPECT(osc_unpack_message(&packet, "/foo", "") != 0);
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "") != 0);
  EXPECT(osc_unpack_message(&packet, "/fooo/x/bar", "") != 0);
  EXPECT(osc_unpack_message(&packet, "/foo/x/baz", "") != 0);

  // Matching addresses.
  EXPECT(osc_unpack_message(&packet, "/foo//bar", "") == 0);
  EXPECT(osc_unpack_message(&packet, "/foo/x/bar", "") == 0);

  return 0;
}

static int test_unpack_one_arg() {
  char data[CAPACITY];
  osc_packet packet;
  packet.data = data;

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/*", "i", 42) == 0);
  int i;
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "f", NULL) != 0);
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "i", &i) == 0);
  EXPECT(i == 42);

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/*", "f", -0.5) == 0);
  float f;
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "f", &f) == 0);
  EXPECT(f == -0.5);

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/*", "s", "") == 0);
  char s[16];
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "s", s) == 0);
  EXPECT(!strcmp("", s));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/*", "s", "bla") == 0);
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "s", s) == 0);
  EXPECT(!strcmp("bla", s));

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/*", "b", 0, NULL) == 0);
  int n;
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "b", &n, s) == 0);
  EXPECT(n == 0);

  EXPECT(osc_pack_message(&packet, CAPACITY, "/foo/*", "b", 2, "xy") == 0);
  EXPECT(osc_unpack_message(&packet, "/foo/bar", "b", &n, s) == 0);
  EXPECT(n == 2);
  EXPECT(buffers_match("xy", s, n));

  return 0;
}

int main(int argc, char **argv) {
  TEST(test_pack_errors);
  TEST(test_pack_capacity);
  TEST(test_pack_no_args);
  TEST(test_pack_one_arg);
  TEST(test_pack_two_args);
  TEST(test_unpack_match);
  TEST(test_unpack_one_arg);
}