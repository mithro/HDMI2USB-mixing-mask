
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
//#include <sys/types.h>

// p == 0 -> 1.0
#define PERCENT(p, max) \
  ((typeof (max))rint((p)*(max)))

enum __attribute__ ((__packed__)) RLEOp {
  RLE_INVALID = 0,
  RLE_REPEAT,
  RLE_PIXEL,
  // FIXME: Does a RLE_PIXEL_F16 which stores a 16bit float make sense?
};

#define packed_struct \
  struct __attribute__((__packed__))


// We want this structure to be 32bits (4 bytes) in size
packed_struct RLE {
  enum RLEOp opcode;           // 1 byte
  union {                      // 3 bytes
    packed_struct {
      uint8_t elements;		// How many of the following items to repeat
      uint16_t times;		// How many times to repeat the items
    } repeat;
    packed_struct {
      uint8_t r;
      uint8_t g;
      uint8_t b;
    } pixel;
  };
};

struct Resolution {
  int x;
  int y;
};

enum MaskType {
  INVALID = 0,
  ALLA,
  ALLB,
  HORIZONTAL,
  VERTICAL,
  // More
};

#define INIT_REPEAT(e, t) \
  (struct RLE){ .opcode = RLE_REPEAT, .repeat = { .elements = (uint8_t)(e), .times = (uint16_t)(t) } }

#define INIT_PIXEL(v) \
  (struct RLE){ .opcode = RLE_PIXEL, .pixel = { .r = v, .g = v, .b = v } }

#define MALLOC_RLE(n) \
  malloc(sizeof(struct RLE)*(n))

int print(struct RLE *data);
int print_multi(struct RLE data[], size_t elements) {
  assert(data);
  printf("%zu[", elements);
  //printf("[");
  int r = 0;
  for (size_t i = 0; i < elements; i++) {
    r += print(data+r);
    if (i != elements-1)
      printf(", ");
  }
  printf("]");
  return r;
}

int print(struct RLE *data) {
  assert(data);
  switch (data->opcode) {
    case RLE_REPEAT:
      printf("Repeat(% 4" PRIu16 " * ", data->repeat.times);
      int e = print_multi(data+1, data->repeat.elements);
      printf(")");
      return 1+e;
    case RLE_PIXEL:
      if ((data->pixel.r == data->pixel.g) && (data->pixel.r == data->pixel.b))
        printf("Pixel(%hhu)", data->pixel.r);
      else
        printf("Pixel(%hhu, %hhu, %hhu)", data->pixel.r, data->pixel.g, data->pixel.b);
      return 1;
    default:
      assert(false);
  }
}

#define indent_printf(indent, fmt, ...) \
  printf("\n%*s" fmt, indent, "", ##__VA_ARGS__)

int indent_print(int indent, struct RLE *data);
int indent_print_multi(int indent, struct RLE data[], size_t elements) {
  assert(data);
  printf("%zu[", elements);
  //printf("[");
  int r = 0;
  for (size_t i = 0; i < elements; i++) {
    r += indent_print(indent+2, data+r);
    if (i != elements-1)
      printf(", ");
  }
  indent_printf(indent, "]");
  return r;
}


int indent_print(int indent, struct RLE *data) {
  assert(data);
  switch (data->opcode) {
    case RLE_REPEAT:
      indent_printf(indent, "Repeat(%" PRIu16 " * ", data->repeat.times);
      int e = indent_print_multi(indent, data+1, data->repeat.elements);
      printf(")");
      return 1+e;
    case RLE_PIXEL:
      if ((data->pixel.r == data->pixel.g) && (data->pixel.r == data->pixel.b))
        indent_printf(indent, "Pixel(%hhu)", data->pixel.r);
      else
        indent_printf(indent, "Pixel(%hhu, %hhu, %hhu)", data->pixel.r, data->pixel.g, data->pixel.b);
      return 1;
    default:
      assert(false);
  }
}


void fill(enum MaskType mtype, struct Resolution res, float percent, struct RLE** data) {
  size_t r_size;
  struct RLE* r_data;

  switch(mtype) {
  case ALLA:
  case ALLB:
    r_size = 3;
    r_data = MALLOC_RLE(r_size);
    assert(r_data);

    r_data[0] = INIT_REPEAT(1, res.y);
    r_data[1] = INIT_REPEAT(1, res.x);

    if (mtype == ALLA)
      r_data[2] = INIT_PIXEL(255);
    else
      r_data[2] = INIT_PIXEL(0);

    break;

  case HORIZONTAL:
    r_size = 5;
    r_data = MALLOC_RLE(r_size);
    assert(r_data);
 
    r_data[0] = INIT_REPEAT(2, res.y);

    r_data[1] = INIT_REPEAT(1, PERCENT(percent, res.x));
    r_data[2] = INIT_PIXEL(255);

    r_data[3] = INIT_REPEAT(1, PERCENT(1.0 - percent, res.x));
    r_data[4] = INIT_PIXEL(0);
    break;

  case VERTICAL:
    r_size = 7;
    r_data = MALLOC_RLE(r_size);
    assert(r_data);

    r_data[0] = INIT_REPEAT(2, 1);
    r_data[1] = INIT_REPEAT(1, PERCENT(percent, res.y));
    r_data[2] = INIT_REPEAT(1, res.x);
    r_data[3] = INIT_PIXEL(255);

    r_data[4] = INIT_REPEAT(1, PERCENT(1.0 - percent, res.y));
    r_data[5] = INIT_REPEAT(1, res.x);
    r_data[6] = INIT_PIXEL(255);
    break;
  }

  *data = r_data;
  return;
}

void print_fill(enum MaskType mtype, struct Resolution res, float percent) {
  printf("\nprint_fill(%hhu, {%d,%d}, %f)->", mtype, res.x, res.y, percent);
  struct RLE *data = NULL;
  fill(mtype, res, percent, &data);
  assert(data != NULL);
  assert(data[0].opcode != RLE_INVALID);
  indent_print(2, data);
  printf("\n");
  free(data);
}


int main() {
  // FIXME: These should be a compiler asserts. Get one from ccan.
  assert(sizeof(enum RLEOp) == 1);
  assert(sizeof(struct RLE) == 4);

  struct Resolution res = { .x = 1280, .y = 720 };

 
  struct RLE pixel1 = INIT_PIXEL(0);
  assert(print(&pixel1) == 1);
  printf("\n");

  struct RLE pixel2 = INIT_PIXEL(255);
  assert(print(&pixel2) == 1);
  printf("\n");

  struct RLE repeat1 = INIT_REPEAT(0, 123);
  assert(print(&repeat1) == 1);
  printf("\n");

  struct RLE repeat2 = INIT_REPEAT(0, 4000);
  assert(print(&repeat2) == 1);
  printf("\n");

  struct RLE repeat3[2] = {
    INIT_REPEAT(1, 234),
    INIT_PIXEL(255),
  };
  assert(print(repeat3) == 2);
  printf("\n");

  struct RLE repeat4[3] = {
    INIT_REPEAT(1, 345), 
      INIT_REPEAT(1, 456),
        INIT_PIXEL(255),
  };
  assert(print(repeat4) == 3);
  printf("\n");

  struct RLE repeat5[3] = {
    INIT_REPEAT(2, 567),
      INIT_PIXEL(254),
      INIT_PIXEL(255),
  };
  assert(indent_print(0, repeat5) == 3);
  printf("\n");

  struct RLE repeat6[5] = { 
    INIT_REPEAT(2, 678), 
      INIT_REPEAT(1, 789), 
        INIT_PIXEL(255), 
      INIT_REPEAT(1, 890), 
        INIT_PIXEL(254),
  };
  assert(indent_print(0, repeat6) == 5);
  printf("\n\n");

  printf("  0%% %hhu\n", PERCENT(0, (uint8_t)255));
  printf(" 33%% %hhu\n", PERCENT(0.333, (uint8_t)255));
  printf(" 50%% %hhu\n", PERCENT(0.5, (uint8_t)255));
  printf("100%% %hhu\n", PERCENT(1.0, (uint8_t)255));

  printf("-----\n");
  print_fill(ALLA, res, 0);
  print_fill(ALLA, res, 1.0);
  printf("- HORIZONTAL ----\n");
  print_fill(HORIZONTAL, res, 0);
  print_fill(HORIZONTAL, res, 0.25);
  print_fill(HORIZONTAL, res, 0.5);
  printf("- VERTICAL ----\n");
  print_fill(VERTICAL, res, 0);
  print_fill(VERTICAL, res, 0.25);
  print_fill(VERTICAL, res, 0.5);
  return 0;
}

