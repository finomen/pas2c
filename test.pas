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
TYPE
pointer=^domain;
alias=typedef;
enum=(one,two,three);
subrange=-5..6;
subrange1='A'..'Z';
array1 = ARRAY [1..10] of pointer;
array2 = ARRAY [1..10, subrange] of enum;
.