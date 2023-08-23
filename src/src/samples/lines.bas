10 REM VDU 23, 30, 5, id; pid; flags, x1; y1; x2; y2; c: [17] Create primitive: Line
20 VDU 23, 30, 5, 2; 0; 1, 200; 20; 100; 120; &20: REM diagonal left
30 VDU 23, 30, 5, 3; 0; 1, 205; 20; 105; 120; &23: REM diagonal left
40 VDU 23, 30, 5, 5; 0; 1, 400; 20; 440; 60; &20: REM diagonal right
50 VDU 23, 30, 5, 6; 0; 1, 405; 20; 445; 60; &23: REM diagonal right
60 VDU 23, 30, 5, 7; 0; 1, 249; 550; 285; 550; &0C: REM horizontal
70 VDU 23, 30, 5, 8; 0; 1, 270; 520; 270; 570; &0D: REM vertical
80 VDU 23, 30, 5, 9; 0; 1, 25; 511; 699; 409; &1D: REM general
