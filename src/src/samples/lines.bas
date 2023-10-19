5 REM Draw various lines
10 REM VDU 23, 30, 20, id; pid; flags; x1; y1; x2; y2; color : Create primitive: Line
20 VDU 23, 30, 20, 3; 0; 1; 200; 20; 100; 120; &C0+&20: REM diagonal left
30 VDU 23, 30, 20, 4; 0; 1; 205; 20; 105; 120; &C0+&23: REM diagonal left
40 VDU 23, 30, 20, 5; 0; 1; 400; 20; 440; 60; &C0+&20: REM diagonal right
50 VDU 23, 30, 20, 6; 0; 1; 405; 20; 445; 60; &C0+&23: REM diagonal right
60 VDU 23, 30, 20, 7; 0; 1; 249; 550; 285; 550; &C0+&0C: REM horizontal
70 VDU 23, 30, 20, 8; 0; 1; 270; 520; 270; 570; &C0+&0D: REM vertical
80 VDU 23, 30, 20, 9; 0; 1; 25; 511; 699; 409; &C0+&1D: REM general
100 FOR I% = 0 TO 200
110 VDU 23, 30, 2, 3; 1; 0;
110 VDU 23, 30, 2, 4; 2; 1;
115 T=TIME
116 IF TIME-T < 2 THEN GOTO 115
120 NEXT I%
130 FOR I% = 0 TO 200
140 VDU 23, 30, 2, 3; -1; 0;
150 VDU 23, 30, 2, 4; -2; -1;
155 T=TIME
156 IF TIME-T < 2 THEN GOTO 155
160 NEXT I%
170 GOTO 100
