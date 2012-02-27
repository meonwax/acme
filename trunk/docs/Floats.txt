Hi!

This is a preliminary release of ACME. I added basic support for
floating point maths. Consider this a special version for beta
testers. Known bugs: Segment counting may be screwed up at the moment.

New:
The maths parser knows about floating point maths, so you can finally
build sin/cos tables directly in ACME. But the expression parser still
uses integer calculations per default. Here are the rules:

a) if a maths operation is useless when done with integers, it is done
with floats and returns a float. Applies to sin(), cos(), tan(),
arcsin(), arccos(), arctan() and float(): These are always computed in
float mode and always return floats.

b) if a maths operation is useles when done with floats, it is done
with integers and returns an integer. Applies to NOT, AND, OR, XOR,
MOD, DIV, LSR, lowbyteof, highbyteof, bankbyteof and int(). These are
always computed in integer mode and always return integers.

c) All other mathematical operations are done in float mode if and
only if at least one of the operands is a float. So "1/2*2" will give
zero because it is done in integer mode, but "1.0/2*2" will give 1
because it is done in float mode.

To force a numerical value to be flagged as being a float, just add
a decimal point and a zero. If a decimal value ends with a
dot character, ACME switches to using the C type "double" and keeps
reading digits. The value is then flagged internally as being float.


Examples:

	!byte 1   / 2   * 2	; gives 0 (integer maths)
	!byte 1   / 2   * 2.0	; gives 0 (1/2 => 0 in integer maths,
				;	float usage comes too late)
	!byte 1   / 2.0 * 2	; gives 1 (FP in effect)
	!byte 1   / 2.0 * 2.0	; gives 1 (FP in effect)
	!byte 1.0 / 2   * 2	; gives 1 (FP in effect)
	!byte 1.0 / 2   * 2.0	; gives 1 (FP in effect)
	!byte 1.0 / 2.0 * 2	; gives 1 (FP in effect)
	!byte 1.0 / 2.0 * 2.0	; gives 1 (FP in effect)

You can use the new float() and int() functions to ensure the type of
maths:

	!byte a   / b   * c	; depends on a/b/c's internal flags
	!byte float(a)/b*c	; calculation is done in FP
	!byte int(a)/int(b)*int(c); calculation is done in integer

As you will have guessed, the trigonometric functions assume radians
for measuring angles (90 degrees equals PI/2).

Have a look at the example source code, it builds some sin/cos tables.

Have fun, and let me know what you think,

Marco Baye
