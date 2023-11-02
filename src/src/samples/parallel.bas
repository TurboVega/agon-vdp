5 REM Draw parallel lines
10 REM VDU 23, 30, 20, id; pid; flags; x1; y1; x2; y2; color : Create primitive: Line

20 VDU 23, 30, 20, 3; 0; &11; 200; 20; 100; 120; &C0+&3F: REM diagonal left
30 VDU 23, 30, 20, 4; 0; &11; 200; 20; 100; 120; &C0+&30: REM diagonal left
40 VDU 23, 30, 20, 5; 0; &11; 200; 20; 100; 120; &C0+&0C: REM diagonal left
50 VDU 23, 30, 20, 6; 0; &11; 200; 20; 100; 120; &C0+&03: REM diagonal left

120 VDU 23, 30, 4, 3;
130 VDU 23, 30, 4, 4;
140 VDU 23, 30, 4, 5;
150 VDU 23, 30, 4, 6;

210 VDU 23, 30, 2, 3; 0; 0;
220 VDU 23, 30, 2, 4; 1; 0;
230 VDU 23, 30, 2, 5; 2; 0;
240 VDU 23, 30, 2, 6; 3; 0;
