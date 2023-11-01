5 REM Draw various lines
10 REM VDU 23, 30, 20, id; pid; flags; x1; y1; x2; y2; color : Create primitive: Line
20 VDU 23, 30, 20, 3; 0; &11; 200; 20; 100; 120; &C0+&20: REM diagonal left
30 VDU 23, 30, 20, 4; 0; &11; 205; 20; 105; 120; &C0+&23: REM diagonal left
40 VDU 23, 30, 20, 5; 0; &01; 400; 20; 440; 60; &C0+&20: REM diagonal right
50 VDU 23, 30, 20, 6; 0; &01; 405; 20; 445; 60; &C0+&23: REM diagonal right
60 VDU 23, 30, 20, 7; 0; &01; 249; 550; 285; 550; &C0+&0C: REM horizontal
70 VDU 23, 30, 20, 8; 0; &01; 270; 520; 270; 570; &C0+&0D: REM vertical
80 VDU 23, 30, 20, 9; 0; &01; 25; 511; 699; 409; &C0+&1D: REM general

120 VDU 23, 30, 4, 3;
130 VDU 23, 30, 4, 4;
140 VDU 23, 30, 4, 5;
150 VDU 23, 30, 4, 6;
160 VDU 23, 30, 4, 7;
170 VDU 23, 30, 4, 8;
180 VDU 23, 30, 4, 9;

200 FOR I% = 0 TO 200
210 VDU 23, 30, 2, 3; 1; 0;
210 VDU 23, 30, 2, 4; 2; 1;
215 T=TIME
216 IF TIME-T < 2 THEN GOTO 215
220 NEXT I%
230 FOR I% = 0 TO 200
240 VDU 23, 30, 2, 3; -1; 0;
250 VDU 23, 30, 2, 4; -2; -1;
255 T=TIME
256 IF TIME-T < 2 THEN GOTO 255
260 NEXT I%
270 GOTO 200
