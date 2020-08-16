/* 
 * CS:APP Data Lab 
 * 
 * Soros Liu 2019/08/22
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  /*
   * 1. x^y = (~x&y) | (x&~y)
   * 2. ~(a&b) = ~a | ~b, so ~(~a & ~b) = ~~a | ~~b = a | b
   */
  int a = ~x & y, b = x & ~y;
  return ~(~a & ~b);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /*
   * simply TMin32 = [10...0] with 31 0s, so just 1 << 31
   */
  return 1 << 31;
}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  /*
   * only for
   *    | 01...11 | 11...11 |
   * +1 | 10...00 | 00...00 |
   * ~  | 10...00 | 00...00 |
   * that we can get x+1 == ~x, which is `a`
   * so we just need to judge if x is not 0xffffffff, which is `b`
   */
  int a = !((x+1) ^ ~x), b = !!(x+1);
  return a & b;
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  /*
   * half the bit vector, `&` the halves until get a 4 bit vector
   * then test if it has pattern as `[1x1x]` (use mask 0xAA)
   * it evalutes true iff. all odd bits
   */
  int a = (x & (x >> 16));
  int b = (a & (a >> 8));
  return !((b & 0xAA) ^ 0xAA);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  /*
   * just the convert rule: -x = ~x + 1
   */ 
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /*
   * y = x - 0x30, so 0x0 <= y <= 0x9
   * for 0x0 <= y <= 0x7, just right shift 3 bits and check result is 0,
   * for y = 0x8 or y = 0x9, check as special cases
   */
  int y = x + ~0x30 + 1;
  return (!(y >> 3)) | !(y ^ 0x8) | !(y ^ 0x9);
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /*
   * with bit operation, we can use this rule:
   *   when mask = [00...00], (mask & a) | (~mask | b) = b;
   *   when mask = [11...11], (mask & a) | (~mask | b) = a;
   * so we try to make these two masks from x
   */ 
  int mask = !x + ~1 + 1;
  return (mask & y) | (~mask & z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  /*
   * check 4 cases:
   *   1. x < 0, y >= 0 => x <= y => return 1
   *   2. x >= 0, y < 0 => x >= y => return 0
   *   3. x < 0, y < 0 => y - x >= 0 iff. y >= x, there will be no overflow
   *   4. x >= 0, y >= 0 => same case as 3
   * 
   * so we first check case 1, which is `yIsPos & xIsNeg`
   * then we check case 3 and 4
   * but CAUTION HERE: if x >= 0, y < 0, y - x >= 0 when overflow occurs,
   * so we need to eliminate case 2 as `yIsPos | xIsNeg`, then check if `yMinusXIsPos`
   */
  int yIsPos = !(y >> 31), xIsNeg = !((x >> 31) + 1);
  int yMinusXIsPos = !((y + ~x + 1) >> 31);
  return (yIsPos & xIsNeg) | ((yIsPos | xIsNeg) & yMinusXIsPos);
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  /*
   * gather all the bits into the most significant bit
   * if x == 0, we will get [00...00], right shift 31 bits => 0x0, +1 => 0x1
   * if x != 0, we will get [10...00], right shift 31 bits => 0xffffffff, +1 => 0x0
   */
  int a = x | (x << 16);
  int b = a | (a << 8);
  int c = b | (b << 4);
  int d = c | (c << 2);
  int e = d | (d << 1);
  return (e >> 31) + 1;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  /*
   * from `./tests.c` file, 
   * we can get a fact that if x < 0, then `howManyBits(x) == howManyBits(~x)`
   * so, `x32` is for this purpose: flip bits if x < 0
   * 
   * then, howManyBits is to find the highest position of `1`, plus 1 bit for sign
   * so we take half of bit vector, to see if high half has `1`
   * if high half has `1`, then we take this half, record corresponding position (16, 8, 4 ...)
   * otherwise, we take the low half
   * can continue until only 1 bit left
   * 
   * finally, sum them up, plus 1 for sign bit
   */
  int x32 = x ^ (x >> 31);
  int neg1 = ~1 + 1;

  int high16 = x32 >> 16;
  int high16With1 = !!high16;
  int high16Mask = high16With1 + neg1;
  int x16 = ((~high16Mask & high16) | (high16Mask & x32)) & ((1 << 16) + neg1);

  int high8 = x16 >> 8;
  int high8With1 = !!high8;
  int high8Mask = high8With1 + neg1;
  int x8 = ((~high8Mask & high8) | (high8Mask & x16)) & 0xFF;

  int high4 = x8 >> 4;
  int high4With1 = !!high4;
  int high4Mask = high4With1 + neg1;
  int x4 = ((~high4Mask & high4) | (high4Mask & x8)) & 0x0F;

  int high2 = x4 >> 2;
  int high2With1 = !!high2;
  int high2Mask = high2With1 + neg1;
  int x2 = ((~high2Mask & high2) | (high2Mask & x4)) & 0x03;

  int high1 = x2 >> 1;
  int high1With1 = !!high1;
  int high1Mask = high1With1 + neg1;
  int x1 = ((~high1Mask & high1) | (high1Mask & x2)) & 0x01;

  return (high16With1 << 4) + (high8With1 << 3) + (high4With1 << 2) + (high2With1 << 1) + high1With1 + x1 + 1;
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  // check through case by case

  // get E and M part bit representation
  int E = (uf >> 23) & 0xFF;
  int M = uf & 0x7FFFFF;
  
  // special case for NaN and inf
  if (E == 0xFF) {
    return uf;
  }
  // Denormalized case: *2 for fraction part
  if (!E) {
    M = M << 1;
    // floatScale2 result is >= 1, so the result should be normalized
    if ((M & 0x800000) == 0x800000) {
      E = 0x01;
    }
  }
  // normalized case: +1 for exponent part
  else {
    E ++;
  }
  return (uf & 0x80000000) | (E << 23) | (M & 0x7FFFFF);
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  // get E, M, sign part of float bit representation
  int E = (uf >> 23) & 0xFF;
  int M = uf & 0x7FFFFF;
  int sign = uf & 0x80000000;

  // if E < 127, E-bias = E - 127 < 0, which means uf is (+-)0.xxxx, leading to int 0
  if (E < 127) {
    return 0;
  }
  // if E > 157, E-bias = E - 127 > 30, which means `out of range`
  if (E > 157) {
    return 0x80000000;
  }

  // result is 1.xxxx * 2^(E-127), so use the shift operation
  // but take care of sign bit
  return (sign | (1 << 30) | (M << 7)) >> (157 - E);
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  /*
   * 1. the `e` part of single precision float number is in range [-126, 127], can represent value in range [2^-126, 2^127],
   * 2. the `f` part is 23 bit wide, which means with leading 0, it can be in range [2^-23, 2^-1]
   * combine together, we can represent integers in range of [2^-149, 2^127]
   * out of that range, the result should be overflow to be +INF or downflow to be 0
   * 
   * for x >= -126, just need to calculate the `E` part
   * for -149 <= x < -126, just need to calculate the `M` part
   */
  if (x > 127) {
    return 0x7f800000;
  }
  if (x >= -126) {
    return (x+127) << 23;
  }
  if (x >= -149) {
    return 1 << (149 + x);
  }
  return 0;
}
