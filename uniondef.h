#ifndef Union_Def
#define Union_Def

typedef union
{
  uint32_t value;
  uint8_t bytes[4];
} uint32Union_t;

typedef union
{
  int32_t value;
  uint8_t bytes[4];
} int32Union_t;

typedef union
{
  double value;
  uint8_t bytes[4];
} doubleUnion_t;

#endif