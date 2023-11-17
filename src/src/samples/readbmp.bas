10 infile=OPENIN "BITMAP_64X50_SOLID.BIN"
20 IF infile=0 THEN END
25 I%=0
30 REPEAT
40  temp%=BGET#infile
50  PRINT I%, temp%
55  I%=I%+1
60 UNTIL EOF#infile
70 CLOSE#infile
