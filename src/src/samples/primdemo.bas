10 REM OTF Primitives Demo
11 CLS: VDU 23,1,0;0;0;0;
12 FREE_ID%=3
13 FDFLT%=&000F
14 FGRP%=&000E
15 OTF%=&1E17
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
397 IF INKEY(100)<0 THEN GOTO 399
398 VDU 23,1,1;0;0;0;
399 END
999 REM -----------
1000 DEF FN_GetID
1010 ID%=FREE_ID%
1020 FREE_ID%=FREE_ID%+1
1030 =ID%
1099 REM -----------
1100 DEF PROC_Title(R%,C%,W%,T$)
1110 PRINT TAB(C%+(W%-LEN(T$))/2,R%);T$
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
10000 DEF FN_Points(R%,C%,W%)
10010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
10020 PROC_Title(R%,C%,W%,"Point")
10030 FOR I%=0 TO 9
10040 ID%=FN_GetID
10050 X%=RND(60)+10
10060 Y%=RND(60)+5
10070 VDU OTF%;10,ID%;GRP%;FDFLT%;X%;Y%;&C0+RND(62)
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
11070 VDU OTF%;20,ID%;GRP%;FDFLT%;X1%;Y1%;X2%;Y2%;&C0+RND(62)
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
12080 VDU OTF%;30,ID%;GRP%;FDFLT%;X1%;Y1%;X2%;Y2%;X3%;Y3%;&C0+RND(62)
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
13080 VDU OTF%;31,ID%;GRP%;FDFLT%;X1%;Y1%;X2%;Y2%;X3%;Y3%;&C0+RND(62)
13090 VDU OTF%;4,ID%;
13100 NEXT I%
13998 =GRP%
13999 REM -----------
14000 DEF FN_TriangleListOutline(R%,C%,W%)
14010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
14020 PROC_Title(R%,C%,W%,"Triangle List Outline")
14030 ID%=FN_GetID
14040 VDU OTF%;32,ID%;GRP%;FDFLT%;5;&C0+RND(62)
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
15040 VDU OTF%;33,ID%;GRP%;FDFLT%;5;&C0+RND(62)
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
16998 =GRP%
16999 REM -----------
17000 DEF FN_SolidTriangleFan(R%,C%,W%)
17010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
17020 PROC_Title(R%,C%,W%,"Solid Triangle Fan")
17998 =GRP%
17999 REM -----------
18000 DEF FN_TriangleStripOutline(R%,C%,W%)
18010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
18020 PROC_Title(R%,C%,W%,"Triangle Strip Outline")
18998 =GRP%
18999 REM -----------
19000 DEF FN_SolidTriangleStrip(R%,C%,W%)
19010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
19020 PROC_Title(R%,C%,W%,"Solid Triangle Strip")
19998 =GRP%
19999 REM -----------
20000 DEF FN_QuadOutline(R%,C%,W%)
20010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
20020 PROC_Title2(R%,C%,W%,"Quad","Outline")
20998 =GRP%
20999 REM -----------
21000 DEF FN_SolidQuad(R%,C%,W%)
21010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
21020 PROC_Title2(R%,C%,W%,"Solid","Quad")
21998 =GRP%
21999 REM -----------
22000 DEF FN_QuadListOutline(R%,C%,W%)
22010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
22020 PROC_Title(R%,C%,W%,"Quad List Outline")
22998 =GRP%
22999 REM -----------
23000 DEF FN_SolidQuadList(R%,C%,W%)
23010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
23020 PROC_Title(R%,C%,W%,"Solid Quad List")
23998 =GRP%
23999 REM -----------
24000 DEF FN_QuadStripOutline(R%,C%,W%)
24010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
24020 PROC_Title(R%,C%,W%,"Quad Strip Outline")
24998 =GRP%
24999 REM -----------
25000 DEF FN_SolidQuadStrip(R%,C%,W%)
25010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
25020 PROC_Title(R%,C%,W%,"Solid Quad Strip")
25998 =GRP%
25999 REM -----------
26000 DEF FN_RectangleOutline(R%,C%,W%)
26010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
26020 PROC_Title2(R%,C%,W%,"Rectangle","Outline")
26998 =GRP%
26999 REM -----------
27000 DEF FN_SolidRectangle(R%,C%,W%)
27010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
27020 PROC_Title2(R%,C%,W%,"Solid","Rectangle")
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
33998 =GRP%
33999 REM -----------
34000 DEF FN_MaskedBitmap(R%,C%,W%)
34010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
34020 PROC_Title2(R%,C%,W%,"Masked","Bitmap")
34998 =GRP%
34999 REM -----------
35000 DEF FN_BitmapFrames(R%,C%,W%)
35010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
35020 PROC_Title2(R%,C%,W%,"Bitmap","Frames")
35998 =GRP%
35999 REM -----------
36000 DEF FN_BitmapScroll(R%,C%,W%)
36010 GRP%=FN_GetID: PROC_SetArea(GRP%,R%,C%,W%)
36020 PROC_Title2(R%,C%,W%,"Bitmap","Scroll")
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
