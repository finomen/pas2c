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
f = 1 = 2;
TYPE
pointer=^domain;
alias=typedef;
enum=(one,two,three);
subrange=-5..6;
subrange1="A".."Z";
array1 = ARRAY [1..10] of pointer;
array2 = ARRAY [1..10, subrange] of enum;
VAR
a, b, c : integer;
a, b, c : array [1..10, 20..30] of char;
PROCEDURE foo; EXTERNAL;
FUNCTION foo : real; EXTERNAL;
PROCEDURE bar (X, Y : Integer; S : String); FORWARD;
PROCEDURE bar2 (VAR X, Y : Integer; S : String); FORWARD;
FUNCTION bar (X, Y : Integer; S : String): real; EXTERNAL;
FUNCTION bar2 (VAR X, Y : Integer; S : String): real; EXTERNAL;

BEGIN
a := b;
b := a + c;
e := a ** b;
a := a * b;
a := a DIV k;
a := a AND b;
a := a OR b;
a := NOT b;

IF a = b THEN foo ELSE bar;
a := b
END
.