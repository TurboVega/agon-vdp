5 REM OTF Primitives Demo
6 REM This demo shows various primitives in On-the-Fly mode.
7 REM Each type of primitive is in its own group, for testing purposes,
8 REM although that is not necessary for most of them. Also, the
9 REM program is verbose in order to illustrate concepts. It could
10 REM be leaner, in terms of code lines and variables.
11 CLS: VDU 23,1,0;0;0;0;
12 FREE_ID%=3
13 FDFLT%=&000F
14 FGRP%=&000E
15 OTF%=&1E17
16 DIR%=1
20 GRP_PT% = FN_Points(9,0,10)
30 GRP_LIN% = FN_Lines(9,10,10)
40 GRP_TRI_OUT% = FN_TriangleOutline(9,20,10)
50 GRP_SOL_TRI% = FN_SolidTriangle(9,30,10)
60 GRP_TRI_LST_OUT% = FN_TriangleListOutline(9,40,30)
70 GRP_SOL_TRI_LST% = FN_SolidTriangleList(9,70,30)
80 GRP_TRI_FAN_OUT% = FN_TriangleFanOutline(19,0,20)
90 GRP_SOL_TRI_FAN% = FN_SolidTriangleFan(19,20,20)
100 GRP_TRI_STR_OUT% = FN_TriangleStripOutline(19,40,30)
110 GRP_SOL_TRI_STR% = FN_SolidTriangleStrip(19,70,30)
120 GRP_QAD_OUT% = FN_QuadOutline(29,0,10)
130 GRP_SOL_QAD% = FN_SolidQuad(29,10,10)
140 GRP_QAD_LST_OUT% = FN_QuadListOutline(29,20,20)
150 GRP_SOL_QAD_LST% = FN_SolidQuadList(29,40,20)
160 GRP_QAD_STR_OUT% = FN_QuadStripOutline(29,60,20)
170 GRP_SOL_QAD_STR% = FN_SolidQuadStrip(29,80,20)
180 GRP_REC_OUT% = FN_RectangleOutline(39,0,10)
190 GRP_SOL_REC% = FN_SolidRectangle(39,10,10)
200 GRP_CIR_OUT% = FN_CircleOutline(39,20,10)
210 GRP_SOL_CIR% = FN_SolidCircle(39,30,10)
220 GRP_ELL_OUT% = FN_EllipseOutline(39,40,10)
230 GRP_SOL_ELL% = FN_SolidEllipse(39,50,10)
240 GRP_TER_SML% = FN_TerminalSmallFont(39,60,40)
250 GRP_SOL_BMP% = FN_SolidBitmap(49,0,10)
260 GRP_MSK_BMP% = FN_MaskedBitmap(49,10,10)
270 GRP_BMP_FRM% = FN_BitmapFrames(49,20,10)
280 GRP_BMP_SCR% = FN_BitmapScroll(49,30,10)
290 GRP_GRP_MOV% = FN_GroupMove(49,40,10)
300 GRP_TER_BIG% = FN_TerminalBigFont(49,50,50)
310 GRP_TRN_BMP% = FN_TransparentBitmaps(69,0,40)
320 GRP_SOL_REN% = FN_Solid3DRender(69,40,20)
330 GRP_MSK_REN% = FN_Masked3DRender(69,60,20)
340 GRP_TRN_REN% = FN_Transparent3DRender(69,80,20)
350 REM -----------
355 REM ANIMATIONS
357 REM -----------
400 IF INKEY(100)>=0 THEN GOTO 997
405 *FX 19
410 REM Flip bitmap Frames
420 SLICE%=SLICE%+50: IF SLICE%>=250 THEN SLICE%=0
430 VDU OTF%;123,GRP_BMP_FRM%+2;8;10;SLICE%;50;
440 REM Scroll bitmap
450 SCROLL%=SCROLL%+DIR%
455 IF SCROLL%<0 THEN SCROLL%=0:DIR%=1
457 IF SCROLL%>200 THEN SCROLL%=200:DIR%=-1
460 VDU OTF%;123,GRP_BMP_SCR%+2;8;10;SCROLL%;50;
996 GOTO 400
997 CLS: VDU 23,1,1;0;0;0;
998 END
999 REM -----------
1000 DEF FN_GetID
1010 ID%=FREE_ID%
1020 FREE_ID%=FREE_ID%+1
1030 =ID%
1099 REM -----------
1100 DEF PROC_Title(R%,C%,W%,T$)
1110 PRINT TAB(C%+(W%-LEN(T$))/2,R%);T$;
1120 ENDPROC
1199 REM -----------
1200 DEF PROC_Title2(R%,C%,W%,T1$,T2$)
1210 PROC_Title(R%-1,C%,W%,T1$)
1220 PROC_Title(R%,C%,W%,T2$)
1230 ENDPROC
1299 REM -----------
1300 DEF PROC_Title3(R%,C%,W%,T1$,T2$,T3$)
1310 PROC_Title(R%-2,C%,W%,T1$)
1320 PROC_Title(R%-1,C%,W%,T2$)
1330 PROC_Title(R%,C%,W%,T3$)
1330 ENDPROC
1399 REM -----------
1400 DEF PROC_SetArea(GRP%,R%,C%,W%)
1410 AreaX1%=C%*8
1420 AreaY2%=(R%+1)*8+1
1430 AreaX2%=AreaX1%+W%*8
1440 IF R%>=50 THEN AreaY1%=AreaY2%-20*8 ELSE AreaY1%=AreaY2%-10*8
1450 AreaWidth%=AreaX2%-AreaX1%
1460 AreaHeight%=AreaY2%-AreaY1%
1465 VDU OTF%;140,GRP%;0;FGRP%;AreaX1%;AreaY1%;AreaWidth%;AreaHeight%;
1470 ID%=FN_GetID
1480 VDU OTF%;40,ID%;GRP%;FDFLT%;0;0;AreaWidth%;AreaHeight%;&D5
1490 VDU OTF%;4,ID%;
1498 ENDPROC
1499 REM -----------
1500 DEF FN_Color
1510 =&C0+RND(62)
1599 REM -----------
10000 DEF FN_Points(R%,C%,W%)
10010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
10020 PROC_Title(R%,C%,W%,"Point")
10030 FOR I%=0 TO 9
10040 ID%=FN_GetID
10050 X%=RND(60)+10
10060 Y%=RND(60)+5
10070 VDU OTF%;10,ID%;GRP%;FDFLT%;X%;Y%;FN_Color
10080 VDU OTF%;4,ID%;
10090 NEXT I%
10998 =GRP%
10999 REM -----------
11000 DEF FN_Lines(R%,C%,W%)
11010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
11020 PROC_Title(R%,C%,W%,"Line")
11030 FOR I%=0 TO 9
11040 ID%=FN_GetID
11050 X1%=RND(60)+10:Y1%=RND(60)+5
11060 X2%=RND(60)+10:Y2%=RND(60)+5
11070 VDU OTF%;20,ID%;GRP%;FDFLT%;X1%;Y1%;X2%;Y2%;FN_Color
11080 VDU OTF%;4,ID%;
11090 NEXT I%
11998 =GRP%
11999 REM -----------
12000 DEF FN_TriangleOutline(R%,C%,W%)
12010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
12020 PROC_Title2(R%,C%,W%,"Triangle","Outline")
12030 FOR I%=0 TO 4
12040 ID%=FN_GetID
12050 X1%=RND(60)+10:Y1%=RND(60)+5
12060 X2%=RND(60)+10:Y2%=RND(60)+5
12070 X3%=RND(60)+10:Y3%=RND(60)+5
12080 VDU OTF%;30,ID%;GRP%;FDFLT%;FN_Color,X1%;Y1%;X2%;Y2%;X3%;Y3%;
12090 VDU OTF%;4,ID%;
12100 NEXT I%
12998 =GRP%
12999 REM -----------
13000 DEF FN_SolidTriangle(R%,C%,W%)
13010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
13020 PROC_Title2(R%,C%,W%,"Solid","Triangle")
13030 FOR I%=0 TO 4
13040 ID%=FN_GetID
13050 X1%=RND(60)+10:Y1%=RND(60)+5
13060 X2%=RND(60)+10:Y2%=RND(60)+5
13070 X3%=RND(60)+10:Y3%=RND(60)+5
13080 VDU OTF%;31,ID%;GRP%;FDFLT%;FN_Color,X1%;Y1%;X2%;Y2%;X3%;Y3%;
13090 VDU OTF%;4,ID%;
13100 NEXT I%
13998 =GRP%
13999 REM -----------
14000 DEF FN_TriangleListOutline(R%,C%,W%)
14010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
14020 PROC_Title(R%,C%,W%,"Triangle List Outline")
14030 ID%=FN_GetID
14040 VDU OTF%;32,ID%;GRP%;FDFLT%;5;FN_Color
14050 FOR I%=0 TO 4
14060 X1%=RND(210)+10:Y1%=RND(60)+5
14070 X2%=RND(210)+10:Y2%=RND(60)+5
14080 X3%=RND(210)+10:Y3%=RND(60)+5
14090 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;
14100 NEXT I%
14110 VDU OTF%;4,ID%;
14998 =GRP%
14999 REM -----------
15000 DEF FN_SolidTriangleList(R%,C%,W%)
15010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
15020 PROC_Title(R%,C%,W%,"Solid Triangle List")
15030 ID%=FN_GetID
15040 VDU OTF%;33,ID%;GRP%;FDFLT%;5;FN_Color
15050 FOR I%=0 TO 4
15060 X1%=RND(210)+10:Y1%=RND(60)+5
15070 X2%=RND(210)+10:Y2%=RND(60)+5
15080 X3%=RND(210)+10:Y3%=RND(60)+5
15090 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;
15100 NEXT I%
15110 VDU OTF%;4,ID%;
15998 =GRP%
15999 REM -----------
16000 DEF FN_TriangleFanOutline(R%,C%,W%)
16010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
16020 PROC_Title(R%,C%,W%,"Triangle Fan Outline")
16030 ID%=FN_GetID
16040 VDU OTF%;34,ID%;GRP%;FDFLT%;5;FN_Color
16050 SX0%=80: SY0%=65
16060 SX1%=20: SY1%=20
16070 VDU SX0%;SY0%;SX1%;SY1%;
16080 FOR I%=0 TO 4
16090 X%=20+I%*20: Y%=5+(I%*(I% AND 3))
16100 VDU X%;Y%;
16110 NEXT I%
16120 VDU OTF%;4,ID%;
16998 =GRP%
16999 REM -----------
17000 DEF FN_SolidTriangleFan(R%,C%,W%)
17010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
17020 PROC_Title(R%,C%,W%,"Solid Triangle Fan")
17030 ID%=FN_GetID
17040 VDU OTF%;35,ID%;GRP%;FDFLT%;5;FN_Color
17050 SX0%=80: SY0%=65
17060 SX1%=20: SY1%=20
17070 VDU SX0%;SY0%;SX1%;SY1%;
17080 FOR I%=0 TO 4
17090 X%=20+I%*20: Y%=5+(I%*(I% AND 3))
17100 VDU X%;Y%;
17110 NEXT I%
17120 VDU OTF%;4,ID%;
17998 =GRP%
17999 REM -----------
18000 DEF FN_TriangleStripOutline(R%,C%,W%)
18010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
18020 PROC_Title(R%,C%,W%,"Triangle Strip Outline")
18030 ID%=FN_GetID
18040 VDU OTF%;36,ID%;GRP%;FDFLT%;7;FN_Color
18050 SX0%=20: SY0%=65
18060 SX1%=25: SY1%=20
18070 VDU SX0%;SY0%;SX1%;SY1%;
18080 FOR I%=0 TO 6
18090 X%=30+I%*20: Y%=65-((I% AND 1)*40)
18100 VDU X%;Y%;
18110 NEXT I%
18120 VDU OTF%;4,ID%;
18998 =GRP%
18999 REM -----------
19000 DEF FN_SolidTriangleStrip(R%,C%,W%)
19010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
19020 PROC_Title(R%,C%,W%,"Solid Triangle Strip")
19030 ID%=FN_GetID
19040 VDU OTF%;37,ID%;GRP%;FDFLT%;7;FN_Color
19050 SX0%=20: SY0%=65
19060 SX1%=25: SY1%=20
19070 VDU SX0%;SY0%;SX1%;SY1%;
19080 FOR I%=0 TO 6
19090 X%=30+I%*20: Y%=65-((I% AND 1)*40)
19100 VDU X%;Y%;
19110 NEXT I%
19120 VDU OTF%;4,ID%;
19998 =GRP%
19999 REM -----------
20000 DEF FN_QuadOutline(R%,C%,W%)
20010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
20020 PROC_Title2(R%,C%,W%,"Quad","Outline")
20030 ID%=FN_GetID
20040 X1%=20:Y1%=52
20050 X2%=25:Y2%=5
20060 X3%=62:Y3%=15
20070 X4%=55:Y4%=50
20080 VDU OTF%;60,ID%;GRP%;FDFLT%;FN_Color,X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
20090 VDU OTF%;4,ID%;
20998 =GRP%
20999 REM -----------
21000 DEF FN_SolidQuad(R%,C%,W%)
21010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
21020 PROC_Title2(R%,C%,W%,"Solid","Quad")
21030 ID%=FN_GetID
21040 X1%=20:Y1%=52
21050 X2%=25:Y2%=5
21060 X3%=62:Y3%=15
21070 X4%=55:Y4%=50
21080 VDU OTF%;61,ID%;GRP%;FDFLT%;FN_Color,X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
21090 VDU OTF%;4,ID%;
21998 =GRP%
21999 REM -----------
22000 DEF FN_QuadListOutline(R%,C%,W%)
22010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
22020 PROC_Title(R%,C%,W%,"Quad List Outline")
22030 ID%=FN_GetID
22040 VDU OTF%;62,ID%;GRP%;FDFLT%;3;FN_Color
22050 X1%=20:Y1%=52
22060 X2%=25:Y2%=9
22070 X3%=62:Y3%=15
22080 X4%=55:Y4%=46
22090 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
22100 X1%=50:Y1%=52
22110 X2%=65:Y2%=5
22120 X3%=92:Y3%=15
22130 X4%=85:Y4%=50
22140 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
22150 X1%=80:Y1%=52
22160 X2%=85:Y2%=11
22170 X3%=122:Y3%=15
22180 X4%=115:Y4%=48
22190 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
22200 VDU OTF%;4,ID%;
22998 =GRP%
22999 REM -----------
23000 DEF FN_SolidQuadList(R%,C%,W%)
23010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
23020 PROC_Title(R%,C%,W%,"Solid Quad List")
23030 ID%=FN_GetID
23040 VDU OTF%;63,ID%;GRP%;FDFLT%;3;FN_Color
23050 X1%=20:Y1%=52
23060 X2%=25:Y2%=9
23070 X3%=62:Y3%=15
23080 X4%=55:Y4%=46
23090 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
23100 X1%=50:Y1%=49
23110 X2%=65:Y2%=5
23120 X3%=92:Y3%=15
23130 X4%=85:Y4%=50
23140 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
23150 X1%=80:Y1%=52
23160 X2%=85:Y2%=11
23170 X3%=122:Y3%=15
23180 X4%=115:Y4%=48
23190 VDU X1%;Y1%;X2%;Y2%;X3%;Y3%;X4%;Y4%;
23200 VDU OTF%;4,ID%;
23998 =GRP%
23999 REM -----------
24000 DEF FN_QuadStripOutline(R%,C%,W%)
24010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
24020 PROC_Title(R%,C%,W%,"Quad Strip Outline")
24030 ID%=FN_GetID
24040 VDU OTF%;64,ID%;GRP%;FDFLT%;6;FN_Color
24050 SX0%=20: SY0%=65
24060 SX1%=25: SY1%=20
24070 VDU SX0%;SY0%;SX1%;SY1%;
24080 FOR I%=0 TO 5
24090 X1%=30+I%*20: Y1%=10-((I% AND 3)*2)
24100 X2%=30+I%*22: Y2%=65-((I% AND 3)*3)
24110 VDU X1%;Y1%;X2%;Y2%;
24120 NEXT I%
24130 VDU OTF%;4,ID%;
24998 =GRP%
24999 REM -----------
25000 DEF FN_SolidQuadStrip(R%,C%,W%)
25010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
25020 PROC_Title(R%,C%,W%,"Solid Quad Strip")
25030 ID%=FN_GetID
25040 VDU OTF%;65,ID%;GRP%;FDFLT%;6;FN_Color
25050 SX0%=20: SY0%=65
25060 SX1%=25: SY1%=20
25070 VDU SX0%;SY0%;SX1%;SY1%;
25080 FOR I%=0 TO 5
25090 X1%=30+I%*20: Y1%=10-((I% AND 3)*2)
25100 X2%=30+I%*22: Y2%=65-((I% AND 3)*3)
25110 VDU X1%;Y1%;X2%;Y2%;
25120 NEXT I%
25130 VDU OTF%;4,ID%;
25998 =GRP%
25999 REM -----------
26000 DEF FN_RectangleOutline(R%,C%,W%)
26010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
26020 PROC_Title2(R%,C%,W%,"Rectangle","Outline")
26030 FOR I%=0 TO 4
26040 ID%=FN_GetID
26050 X%=RND(30)+10:Y%=RND(30)+5
26060 W%=RND(20)+10:H%=RND(10)+5
26070 VDU OTF%;40,ID%;GRP%;FDFLT%;X%;Y%;W%;H%;FN_Color
26080 VDU OTF%;4,ID%;
26090 NEXT I%
26998 =GRP%
26999 REM -----------
27000 DEF FN_SolidRectangle(R%,C%,W%)
27010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
27020 PROC_Title2(R%,C%,W%,"Solid","Rectangle")
27030 FOR I%=0 TO 4
27040 ID%=FN_GetID
27050 X%=RND(30)+10:Y%=RND(30)+5
27060 W%=RND(20)+10:H%=RND(10)+5
27070 VDU OTF%;41,ID%;GRP%;FDFLT%;X%;Y%;W%;H%;FN_Color
27080 VDU OTF%;4,ID%;
27090 NEXT I%
27998 =GRP%
27999 REM -----------
28000 DEF FN_CircleOutline(R%,C%,W%)
28010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
28020 PROC_Title2(R%,C%,W%,"Circle","Outline")
28998 =GRP%
28999 REM -----------
29000 DEF FN_SolidCircle(R%,C%,W%)
29010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
29020 PROC_Title2(R%,C%,W%,"Solid","Circle")
29998 =GRP%
29999 REM -----------
30000 DEF FN_EllipseOutline(R%,C%,W%)
30010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
30020 PROC_Title2(R%,C%,W%,"Ellipse","Outline")
30998 =GRP%
30999 REM -----------
31000 DEF FN_SolidEllipse(R%,C%,W%)
31010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
31020 PROC_Title2(R%,C%,W%,"Solid","Ellipse")
31998 =GRP%
31999 REM -----------
32000 DEF FN_TerminalSmallFont(R%,C%,W%)
32010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
32020 PROC_Title(R%,C%,W%,"Terminal Small Font")
32998 =GRP%
32999 REM -----------
33000 DEF FN_SolidBitmap(R%,C%,W%)
33010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
33020 PROC_Title2(R%,C%,W%,"Solid","Bitmap")
33030 ID%=FN_GetID
33040 VDU OTF%;120,ID%;GRP%;FDFLT%;64;50;
33050 infile=OPENIN "BITMAP_64X50_SOLID.BIN"
33060 IF infile=0 THEN END
33070 I%=0
33080 VDU OTF%;132,ID%;0;0;3200;
33090 REPEAT
33100  temp%=BGET#infile
33110  VDU temp%+&C0
33120 UNTIL EOF#infile
33130 CLOSE#infile
33140 VDU OTF%;1,ID%;8;10;
33150 VDU OTF%;4,ID%;
33998 =GRP%
33999 REM -----------
34000 DEF FN_MaskedBitmap(R%,C%,W%)
34010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
34020 PROC_Title2(R%,C%,W%,"Masked","Bitmap")
34030 ID%=FN_GetID
34040 VDU OTF%;121,ID%;GRP%;FDFLT%;64;50;&C0
34050 infile=OPENIN "BITMAP_64X50_MASKED.BIN"
34060 IF infile=0 THEN END
34070 I%=0
34080 VDU OTF%;133,ID%;0;0;3200;
34090 REPEAT
34100  temp%=BGET#infile
34110  VDU temp%+&C0
34120 UNTIL EOF#infile
34130 CLOSE#infile
34140 VDU OTF%;1,ID%;8;10;
34150 VDU OTF%;4,ID%;
34998 =GRP%
34999 REM -----------
35000 DEF FN_BitmapFrames(R%,C%,W%)
35010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
35020 PROC_Title2(R%,C%,W%,"Bitmap","Frames")
35030 ID%=FN_GetID
35040 VDU OTF%;120,ID%;GRP%;FDFLT%;64;250;
35050 infile=OPENIN "BITMAP_64X250_SOLID.BIN"
35060 IF infile=0 THEN END
35070 I%=0
35080 VDU OTF%;132,ID%;0;0;16000;
35090 REPEAT
35100  temp%=BGET#infile
35110  VDU temp%+&C0
35120 UNTIL EOF#infile
35130 CLOSE#infile
35140 VDU OTF%;123,ID%;8;10;0;50;
35150 VDU OTF%;4,ID%;
35998 =GRP%
35999 REM -----------
36000 DEF FN_BitmapScroll(R%,C%,W%)
36010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
36020 PROC_Title2(R%,C%,W%,"Bitmap","Scroll")
36030 ID%=FN_GetID
36040 VDU OTF%;120,ID%;GRP%;FDFLT%;64;250;
36050 infile=OPENIN "BITMAP_64X250_SOLID.BIN"
36060 IF infile=0 THEN END
36070 I%=0
36080 VDU OTF%;132,ID%;0;0;16000;
36090 REPEAT
36100  temp%=BGET#infile
36110  VDU temp%+&C0
36120 UNTIL EOF#infile
36130 CLOSE#infile
36140 VDU OTF%;123,ID%;8;10;0;50;
36150 VDU OTF%;4,ID%;
36998 =GRP%
36999 REM -----------
37000 DEF FN_GroupMove(R%,C%,W%)
37010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
37020 PROC_Title2(R%,C%,W%,"Group","Move")
37998 =GRP%
37999 REM -----------
38000 DEF FN_TerminalBigFont(R%,C%,W%)
38010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
38020 PROC_Title(R%,C%,W%,"Terminal Large Font")
38998 =GRP%
38999 REM -----------
39000 DEF FN_TransparentBitmaps(R%,C%,W%)
39010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
39020 PROC_Title(R%,C%,W%,"Transparent Bitmaps")
39998 =GRP%
39999 REM -----------
40000 DEF FN_Solid3DRender(R%,C%,W%)
40010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
40020 PROC_Title(R%,C%,W%,"Solid 3D Render")
40998 =GRP%
40999 REM -----------
41000 DEF FN_Masked3DRender(R%,C%,W%)
41010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
41020 PROC_Title(R%,C%,W%,"Masked 3D Render")
41998 =GRP%
41999 REM -----------
42000 DEF FN_Transparent3DRender(R%,C%,W%)
42010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
42020 PROC_Title2(R%,C%,W%,"Transparent","3D Render")
42998 =GRP%
42999 REM -----------
