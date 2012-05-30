PROGRAM program_name;
10, 5, 100500;
CONST
a='hello';
b=5;
c=a;
d=a+c;
e=a*b;
f=a**b;
g=(f);
complex = (a + 6) ** (a + 4 * 7);
k = .7;
l = 1 = 2;
TYPE
domain=char;
pointer=^domain;
alias=domain;
enumt=(one,two,three);
subrange=-5..6;
subrange1="A".."Z";
array1 = ARRAY [1..10] of pointer;
array2 = ARRAY [1..10, subrange] of enumt;
VAR
ia, ib, ic : integer;
a1, b1, c1 : array [1..10, 20..30] of char;
PROCEDURE foo; EXTERNAL;
PROCEDURE foo (ia : Integer); FORWARD;
FUNCTION foo1 : real; FORWARD;
PROCEDURE bar1 (X, Y : Integer; S : String); FORWARD;
PROCEDURE bar2 (VAR X, Y : Integer; S : String); FORWARD;
FUNCTION bar3 (X, Y : Integer; S : String): real; EXTERNAL;
FUNCTION bar4 (VAR X, Y : Integer; S : String): real; EXTERNAL;

FUNCTION bar2 (X, Y: Integer; S: String) : real;
BEGIN
foo(5)
END;

FUNCTION foo1 : real;
BEGIN
foo(5)
END;

PROCEDURE foo (ia: Integer);
BEGIN
foo(5)
END;

PROCEDURE foo;
BEGIN
foo(5)
END;

BEGIN
ia := b;
ia := a1[1];
ib := ia + b;
ic := ia ** b;
ia := ia * ib;
ia := ia DIV k;
ia := ia AND b;
ia := ia OR b;
ia := NOT b;

IF ia = ib THEN foo;
IF ia = ib THEN foo ELSE foo1;
ia := ib;
FOR ia := b TO b DO foo;
WHILE ia = b DO foo;
ia := b;
REPEAT foo UNTIL foo1;
WHILE foo1 DO BEGIN
foo
END

END
.
