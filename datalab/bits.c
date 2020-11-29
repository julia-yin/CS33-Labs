/* 
 * CS:APP Data Lab 
 * 
 * Julia Yin, ID: 005311394
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
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

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

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

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
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
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
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~(~x | ~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  return (x>>(n<<3))&0xFF;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int shift_mask = ~(((1<<31)>>n)<<1); //creates a mask with the first n bits being 0s
  return (x>>n) & shift_mask;
}
/* 
 * rotateRight - Rotate x to the right by n
 *   Can assume that 0 <= n <= 31
 *   Examples: rotateRight(0x87654321,4) = 0x18765432
 *   Legal ops: ~ & ^ | + << >> !
 *   Max ops: 25
 *   Rating: 3 
 */
int rotateRight(int x, int n) {
  // step 1: create mask and store the last n bits before shifting
  int mask_2n_1 = ~(1<<31)>>(31+(~n+1));
  int stored_bits = x & mask_2n_1;
  // step 2: logical shift x by n (all beginning n bits are 0s)
  int shift = ~(((1<<31)>>n)<<1);
  int stored_mask = stored_bits<<(32+(~n+1));
  x = (x>>n) & shift;
  // step 3: create mask to replace first n bits with stored last n bits 
  return x | stored_mask;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int c = ~(!x) + 1; // 
  return ((c & z) | (~c & y));
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  x = x | (x>>16);
  x = x | (x>>8);
  x = x | (x>>4);
  x = x | (x>>2);
  x = x | (x>>1); // lsb is 1 if nonzero, 0 if zero 
    
  return (x & 0x1) ^ 0x1; // 0 if nonzero, 1 if zero
}
/*
 * bitParity - returns 1 if x contains an odd number of 0's
 *   Examples: bitParity(5) = 0, bitParity(7) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int bitParity(int x) {
  x = x ^ (x<<16);
  x = x ^ (x<<8);
  x = x ^ (x<<4);
  x = x ^ (x<<2);
  x = x ^ (x<<1); // by this point, if odd MSB = 1, even MSB = 0 (two or no 0s present)  
  x = x>>31; // if odd x = 0xFFFFFFFF, even x = 0
  return !(!(x));
}
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int isNegOne = !(~x);
  return (!(~(x+1)^x) & !isNegOne);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  int fitsPositive = !(x>>(n+(~0))); // all 0s if x is positive and fits
  int fitsNegative = !((~x)>>(n+(~0))); // all 0s if x is negative and fits
  return fitsPositive | fitsNegative;
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  int x_neg = x>>31;
  int x_opp = ~x;
  int x_posOrTmin = (x_opp >> 31) | (x_opp + 1) >> 31; // x_opp shifted by 31 will give -1 if x is positive, and x_opp + 1 will give Tmin if x is Tmin and shifting it will give -1; all other negative numbers will give 0x0 as the opposite will have a 0 in the MSB spot
    
  // Case 1: x is positive or Tmin, and shifting left by n will yield x/2^n
  // Case 2: x is negative and not Tmin, convert x to its positive counterpart and shift by n, then convert back to negative (negative numbers default to ceiling when using division)
  return (x_posOrTmin & (x>>n)) | (x_neg & (~((x_opp + 1)>>n) + 1));
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int negate(int x) {
  return (~x + 1);
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  int isZero = !x;
  x = (~(x>>31) & 0x1);
  return (x & (!isZero));
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  int x_positive = !(x>>31); // returns 1 if x is positive
  int y_positive = !(y>>31); // returns 1 if y is positive
  int same_sign = !(x_positive ^ y_positive); // returns 1 if x and y share the same sign
  int xy_equal = !(x^y); // returns 1 if x = y

  // Four cases: equal, same sign, x is positive, or y is positive
  // 1. Equal: return 0
  // 2. Same sign: do x-y and check the MSB to see if its positive or negative, pos = x greater, neg = y greater
  // 3. Opp sign: if x is positive, x is greater. otherwise, y is greater and return 0

  return (!xy_equal) & (((!same_sign) & x_positive) | (same_sign & !((x+(~y+1))>>31)));
}
/* 
 * subOK - Determine if can compute x-y without overflow
 *   Example: subOK(0x80000000,0x80000000) = 1,
 *            subOK(0x80000000,0x70000000) = 0, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int subOK(int x, int y) {
  // int Tmax = ~(1<<31);
  int x_neg = x>>31; // -1 if x is negative, 0 otherwise
  int y_neg = y>>31; // -1 if y is negative, 0 otherwise
  int oppY = ~y + 1; 
  
  // 3 Cases: Same sign, X+Y-, X-Y+
  // Same sign: return 1, will never overflow since the operation is subtraction
  // X is positive: calculate X+(-Y) aka X-Y, adding two positive numbers means that the MSB will become 1 in the event of overflow (result is greater than Tmax)
  // Y is positive: calculate X+(-Y), adding two negative numbers means that the MSB will become 0 in the event of overflow (result is less than Tmin)  
  return (!(x_neg ^ y_neg)) |  ((y_neg & (!((x+oppY)>>31))) | (x_neg & (!(!((x + oppY)>>31)))));
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  int eval;
  int shiftNum;
    
  eval = (!(!(x>>16)))<<4; // check the first 16 bits of x, return 0 if first 16 bits = 0 and 16 if first 16 bits are nonzero
  shiftNum = eval + 8; // check the upper 8 bits of the top and bottom 16 bits sections, shifts 24 if top 16 bits are nonzero and 8 if top 16 bits were 0
  eval = ((!(!(x>>shiftNum)))<<3) + eval; // four different sections of 8 bits to check starting from bit 24, 16, 8, or 0 (first operator gives 8 or 0 depending on whether or not that section was nonzero)
  shiftNum = eval + 4; // check upper 4 bits of each 8-bit section
  eval = ((!(!(x>>shiftNum)))<<2) + eval; // eight different sections of 4 bits to check starting from bit 28, 24, 20, 16, 12, 8, 4, or 0 (first operator gives 4 or 0 depending on if the section was nonzero)
  shiftNum = eval + 2; // check upper 2 bits of each 4-bit section
  eval = ((!(!(x>>shiftNum)))<<1) + eval; // sixteen different sections of 2 bits to check (first operator gives 2 or 0 depending on if the section was nonzero)
  shiftNum = eval + 1;
  eval = (!(!(x>>shiftNum))) + eval; // 32 different possible bits to check (first operator gives 1 or 0 depending on if the section was nonzero)
    
  return eval;
}

