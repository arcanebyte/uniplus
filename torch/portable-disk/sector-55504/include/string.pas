{ NOTICE OF COPYRIGHT AND OWNERSHIP OF SOFTWARE:

  Copyright 1977, 1978, 1979, 1980, 1981, 1982  by Oregon Software, Inc.
  All Rights Reserved.

  This computer program is the property of Oregon Software, Inc.
  of Portland, Oregon, U.S.A., and may be used
  and copied only as specifically permitted under written
  license agreement signed by Oregon Software, Inc.

  Whether this program is copied in whole or in part and whether this
  program is copied in original or in modified form, ALL COPIES OF THIS
  PROGRAM MUST DISPLAY THIS NOTICE OF COPYRIGHT AND OWNERSHIP IN FULL.

   Pascal-1 and Pascal-2 String Package
   Release version: 1.3A  Level: 1  Date: 29-Apr-1982 10:02:11
   Processor: All  System: All
}

{
Strings are stored as a record structure with a fixed
maximum number of characters (normally 100 but easily
changeable), and an integer marking the current length of
the string.

    type String = record
           Len: Integer;
           Ch:  array[1..StringMax] of Char
           end

The capabilities provided are:

Len(S) - a function giving the current length of string S;

Clear(S) - initializes string S to empty;

ReadString(F,S) - reads a value for string S from the text
  file F.  The string is terminated by Eoln(F) and a
  Readln(F) is performed.  String overflow (a string longer
  than StringMax) results in truncation.

WriteString(F,S) - writes the string S to the text file F.
  This function can also be performed by
  Write(F,S.Ch:S.Len).

Concatenate(T,S) - appends string S to the target string T.
  The resulting value is string T.  Overflow results in
  truncation to StringMax characters.

Search(S,T,Start) - searchs string S for the first
  occurrence of string T to the right of position Start
  (characters are numbered beginning with one).  The
  function Search() returns the position of the first
  character in the matching substring, or the value zero if
  the string T does not appear.

Insert(T,S,Start) - inserts the string S into the target
  string T at position Start.  Characters are shifted to the
  right as necessary.  Overflow produces a truncated target
  string;  a Start position which would produce a string
  which was not contiguous has no effect.

The Start and Span parameters in the following procedures
define a substring beginning at position Start (between
characters Start-1 and Start) with a length of Abs(Span).
If Span is positive, the substring is to the right of Start;
if negative, the substring is to the left.

Delete(S,Start,Span) - deletes the substring defined by
  Start, Span from the string S.

Substring(T,S,Start,Span) - the substring of string S
  defined by Start, Span is assigned to the target string T.
}
const	stringmax=100;
type	string=record
	   len: 0..stringmax;
	   ch: packed array[1..stringmax] of char
	   end;

function len(VAR s:string):integer;
begin	len:= s.len
end;

procedure clear(var s:string);
var	i: integer;
begin	s.len:=0;
	for i:=1 to stringmax do s.ch[i]:=' '
end;

procedure concatenate(var s:string; VAR t:string);
var	i,j: integer;
begin
	if s.len+t.len>stringmax
	   then j:=stringmax-s.len { overflow }
	   else j:=t.len;
	for i:=1 to j do s.ch[s.len+i]:=t.ch[i];
	s.len:=s.len+j;
end;

function search(VAR s,t:string; start:integer):integer;
var	i,j: 0..stringmax;
	uneq: boolean;
begin
	if start<1 then start:=1;
	if (start+t.len>s.len+1) or (t.len=0)
	   then search:=0
	   else begin
	      i:=start-1;
	      repeat
		 i:=i+1; j:=0;
		 repeat
		    j:=j+1;
		    uneq:=t.ch[j]<>s.ch[i+j-1];
		 until uneq or (j=t.len);
	      until (not uneq) or (i=s.len-t.len+1);
	      if uneq
		 then search:=0
		 else search:=i;
	      end;
end;

procedure readstring(var f:text; var s:string);
begin
	clear(s);
	with s do
	   while (not eoln(f)) and (len<stringmax) do begin
	      len:=len+1;
	      read(f,ch[len]);
	      end;
	readln(f);
end;

procedure writestring(var f:text; VAR s:string);
begin	write(f,s.ch:s.len)
end;

procedure substring(var t:string; VAR s:string; start,span:integer);
var	i: integer;
begin
	if span<0 then begin span:= -span; start:=start-span end;
	if start<1 then begin span:=span+start-1; start:=1 end;
	if start+span>s.len+1 then span:=s.len-start+1;
	if span<=0
	   then clear(t)
	   else begin
	      for i:=1 to span do t.ch[i]:=s.ch[start+i-1];
	      for i:=span+1 to stringmax do t.ch[i]:=' ';
	      t.len:=span;
	      end;
end;

procedure delete(var s:string; start,span:integer);
var	i,limit: integer;
begin
	if span<0 then begin span:=-span; start:=start-span end;
	limit:=start+span;
	if start<1 then start:=1;
	if limit>s.len+1 then limit:=s.len+1;
	span:=limit-start;
	if span>0 then begin
	   for i:=0 to s.len-limit do s.ch[start+i]:=s.ch[limit+i];
	   for i:=s.len-span+1 to s.len do s.ch[i]:=' ';
	   s.len:=s.len-span;
	   end;
end;

procedure insert(var s:string;VAR t:string; p:integer);
var	i,j: integer;
begin
	if t.len>0 then
	   if (p>0) and (p<=s.len+1)
	      then begin
		 if s.len+t.len<=stringmax
		    then s.len:=s.len+t.len
		    else s.len:=stringmax { overflow } ;
		 for i:=s.len downto p+t.len do s.ch[i]:=s.ch[i-t.len];
		 if s.len<p+t.len
		    then j:=s.len
		    else j:=p+t.len-1;
		 for i:=p to j do s.ch[i]:=t.ch[i-p+1];
		 end
	      else { error: non-contiguous string }
end;
