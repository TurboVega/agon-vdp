10 Print "Sample Spaceship Drawing"
14 Rem Credit to the following:
15 Rem https://stackoverflow.com/questions/19350792/calculate-normal-of-a-single-triangle-in-3d-space
16 Rem https://mathinsight.org/dot_product_formula_components
17 Rem https://realityserver.com/articles/3d-transformations-part1-matrices
18 Rem https://www.mauriciopoppe.com/notes/computer-graphics/transformation-matrices/combining-transformations
20 SCALE = 16.0
30 Screen 12
40 MIDX% = 640 / 2
50 MIDY% = 480 / 2
1000 Dim DATAPOINT(150, 2), MODELPOINT(2, 2)
1020 For I% = 1 To 142: Read DATAPOINT(I%, 0), DATAPOINT(I%, 1), DATAPOINT(I%, 2): Next I%
1040 NF% = 280: GoSub 2000
1998 GoTo 1998
1999 Rem -------------------------------
2000 Rem Compute and output mesh
2010 For FCNT% = 1 To NF%

    2019 Rem Read 3 indexes into the the array of vertexes, for 1 triangle.
    2020 Read I0%, I1%, I2%

    2029 Rem Use simple division by Z to convert 3D to 2D.
    2030 MODELPOINT(0, 0) = DATAPOINT(I0%, 0): MODELPOINT(0, 1) = DATAPOINT(I0%, 1): MODELPOINT(0, 2) = DATAPOINT(I0%, 2)
    2040 PX1 = MODELPOINT(0, 0) * SCALE / MODELPOINT(0, 2): PY1 = MODELPOINT(0, 1) * SCALE / MODELPOINT(0, 2)
    2050 SX1% = MIDX% + Int(PX1): SY1% = MIDY% - Int(PY1)

    2130 MODELPOINT(1, 0) = DATAPOINT(I1%, 0): MODELPOINT(1, 1) = DATAPOINT(I1%, 1): MODELPOINT(1, 2) = DATAPOINT(I1%, 2)
    2140 PX2 = MODELPOINT(1, 0) * SCALE / MODELPOINT(1, 2): PY2 = MODELPOINT(1, 1) * SCALE / MODELPOINT(1, 2)
    2150 SX2% = MIDX% + Int(PX2): SY2% = MIDY% - Int(PY2)

    2230 MODELPOINT(2, 0) = DATAPOINT(I2%, 0): MODELPOINT(2, 1) = DATAPOINT(I2%, 1): MODELPOINT(2, 2) = DATAPOINT(I2%, 2)
    2240 PX3 = MODELPOINT(2, 0) * SCALE / MODELPOINT(2, 2): PY3 = MODELPOINT(2, 1) * SCALE / MODELPOINT(2, 2)
    2250 SX3% = MIDX% + Int(PX3): SY3% = MIDY% - Int(PY3)

    2299 Rem Use cross product to compute the vector normal of the triangle.
    2300 AX = MODELPOINT(1, 0) - MODELPOINT(0, 0): AY = MODELPOINT(1, 1) - MODELPOINT(0, 1): AZ = MODELPOINT(1, 2) - MODELPOINT(0, 2)
    2310 BX = MODELPOINT(2, 0) - MODELPOINT(0, 0): BY = MODELPOINT(2, 1) - MODELPOINT(0, 1): BZ = MODELPOINT(2, 2) - MODELPOINT(0, 2)
    2320 NX = AY * BZ - AZ * BY
    2330 NY = AZ * BX - AX * BZ
    2340 NZ = AX * BY - AY * BX

    2399 Rem Use dot product to know whether triangle faces forward vs. backward.
    2400 DP = MODELPOINT(0, 0) * NX + MODELPOINT(0, 1) * NY + MODELPOINT(0, 2) * NZ
    2410 If DP > 0 Then GoTo 2950


    2920 Line (SX1%, SY1%)-(SX2%, SY2%), 3
    2930 Line (SX2%, SY2%)-(SX3%, SY3%), 3
    2940 Line (SX3%, SY3%)-(SX1%, SY1%), 3
    2950 NTRI% = NTRI% + 1

2980 Next FCNT%
2990 Print NTRI%; " triangles";
2998 Return
2999 Rem -------------------------------
3001 Data 0.68946826,1,1.1682104
3002 Data 0.74679595,0.9194981,1.0629548
3003 Data 0.7810843,1,1
3004 Data 0.36260098,1,0.3300041
3005 Data 0.7365824,1.3046974,0.23783141
3006 Data 0.69956756,1,0.3300041
3007 Data 0.7810843,-0.59975624,1
3008 Data 0.7810843,0,0.5748985
3009 Data 0.7810843,0,1
3010 Data 0.28108427,-1,0.5
3011 Data 0,0,0.44583595
3012 Data 0.28108427,0,0.44583595
3013 Data 0,-1,1.4
3014 Data 0.17362198,-1.9306369,0.99943817
3015 Data 0.17362198,-1,1.4
3016 Data 0,0,1.1796646
3017 Data 0.7810843,1,0.5
3018 Data 0,1,0.5
3019 Data 0.28108427,1,0.5
3020 Data 0.28108427,1,1
3021 Data 0.37270033,0.13432103,1.1682104
3022 Data 0.37270033,1,1.1682104
3023 Data 0,1,1
3024 Data 0.7810843,-0.59975624,0.5
3025 Data 0.7810843,0,0.44583595
3026 Data 0.36960787,-0.92913795,0.91147643
3027 Data 0.28108427,-1,1
3028 Data 0.28108427,0,1
3029 Data 0.36260098,0.13432103,0.3300041
3030 Data 0.28108427,-1.7938288,0.7635075
3031 Data 0.28108427,0,1.1796646
3032 Data 0.28108427,-1.6228187,0.46859416
3033 Data 0,-1.6228187,0.46859416
3034 Data 0,-1,0.5
3035 Data 0.17345148,-2.2365396,0.44531238
3036 Data 0,-2.2365396,0.44531238
3037 Data 0.68946826,0.13432103,1.1682104
3038 Data 0.22616068,1.3046974,0.44517446
3039 Data 0.32558614,1.3046974,0.23783141
3040 Data 0.69956756,0.13432103,0.3300041
3041 Data 0.8360079,1.3046974,0.44517446
3042 Data 0.17345148,-2.3787012,0.6904753
3043 Data 0,-1.9306369,0.99943817
3044 Data 0,-2.3787012,0.6904753
3045 Data 0.8360079,1.3046974,1.0550216
3046 Data 0.7242643,1.3046974,1.2601869
3047 Data 0.33790427,1.3046974,1.2601869
3048 Data 0.22616068,1.3046974,1.0550216
3049 Data 0.6549733,0.9238256,0.44105074
3050 Data 0.7149141,0.9238256,0.566052
3051 Data 0.40719524,0.9238256,0.44105074
3052 Data 0.64754707,0.9238256,1.0574
3053 Data 0.41462147,0.9238256,1.0574
3054 Data 0.34725443,0.9238256,0.9337117
3055 Data 0.7149141,0.9238256,0.9337117
3056 Data 0.34725443,0.9238256,0.566052
3057 Data 1.5975951,0.29872745,0.32989454
3058 Data 2.414106,0.2727982,0.23505598
3059 Data 1.5975951,-0.163479,0.367528
3060 Data 2.414106,1.1387693,0.23505598
3061 Data 2.414106,0.5974549,0.21395314
3062 Data 1.5975951,0.29872745,0.41956782
3063 Data 2.414106,0.5974549,0.26423717
3064 Data 1.2757134,1.3317946,0.367528
3065 Data 0.73527634,0.20640372,1.0693259
3066 Data 0.7237567,0.9194981,1.1052557
3067 Data 1.5688164,1.4938041,1.5412182
3068 Data 1.5688164,1.0570647,1.5412182
3069 Data 0.36960787,-0.92913795,0.58852357
3070 Data 0.36960787,-0.7396879,0.91147643
3071 Data 0.69256073,-0.6706183,0.58852357
3072 Data 0.69256073,-0.6706183,0.91147643
3073 Data 0.36960787,-0.7396879,0.58852357
3074 Data 0.69256073,-0.48116827,0.58852357
3075 Data 0.69256073,-0.48116827,0.91147643
3076 Data 1.5566552,1.4938041,1.5635463
3077 Data -0.74679595,0.9194981,1.0629548
3078 Data -0.68946826,1,1.1682104
3079 Data -0.7810843,1,1
3080 Data -0.7365824,1.3046974,0.23783141
3081 Data -0.36260098,1,0.3300041
3082 Data -0.69956756,1,0.3300041
3083 Data -0.7810843,0,0.5748985
3084 Data -0.7810843,-0.59975624,1
3085 Data -0.7810843,0,1
3086 Data -0.28108427,-1,0.5
3087 Data -0.28108427,0,0.44583595
3088 Data -0.17362198,-1.9306369,0.99943817
3089 Data -0.17362198,-1,1.4
3090 Data -0.28108427,0,1.1796646
3091 Data -0.28108427,1,0.5
3092 Data -0.28108427,1,1
3093 Data -0.37270033,0.13432103,1.1682104
3094 Data -0.28108427,0,1
3095 Data -0.7810843,-0.59975624,0.5
3096 Data -0.7810843,0,0.44583595
3097 Data -0.28108427,-1,1
3098 Data -0.36960787,-0.92913795,0.58852357
3099 Data -0.36260098,0.13432103,0.3300041
3100 Data -0.28108427,-1.6228187,0.46859416
3101 Data -0.17345148,-2.2365396,0.44531238
3102 Data -0.28108427,-1.7938288,0.7635075
3103 Data -0.68946826,0.13432103,1.1682104
3104 Data -0.22616068,1.3046974,0.44517446
3105 Data -0.69956756,0.13432103,0.3300041
3106 Data -0.8360079,1.3046974,0.44517446
3107 Data -0.7810843,1,0.5
3108 Data -0.17345148,-2.3787012,0.6904753
3109 Data -0.8360079,1.3046974,1.0550216
3110 Data -0.33790427,1.3046974,1.2601869
3111 Data -0.37270033,1,1.1682104
3112 Data -0.22616068,1.3046974,1.0550216
3113 Data -0.6549733,0.9238256,0.44105074
3114 Data -0.40719524,0.9238256,0.44105074
3115 Data -0.32558614,1.3046974,0.23783141
3116 Data -0.64754707,0.9238256,1.0574
3117 Data -0.7242643,1.3046974,1.2601869
3118 Data -0.41462147,0.9238256,1.0574
3119 Data -0.7149141,0.9238256,0.9337117
3120 Data -0.34725443,0.9238256,0.9337117
3121 Data -2.414106,0.2727982,0.23505598
3122 Data -1.5975951,0.29872745,0.32989454
3123 Data -1.5975951,-0.163479,0.367528
3124 Data -2.414106,1.1387693,0.23505598
3125 Data -1.2757134,1.3317946,0.367528
3126 Data -1.5975951,0.29872745,0.41956782
3127 Data -2.414106,0.5974549,0.21395314
3128 Data -2.414106,0.5974549,0.26423717
3129 Data -0.73527634,0.20640372,1.0693259
3130 Data -1.5688164,1.4938041,1.5412182
3131 Data -0.7237567,0.9194981,1.1052557
3132 Data -1.5688164,1.0570647,1.5412182
3133 Data -0.36960787,-0.7396879,0.91147643
3134 Data -0.36960787,-0.92913795,0.91147643
3135 Data -0.69256073,-0.6706183,0.58852357
3136 Data -0.36960787,-0.7396879,0.58852357
3137 Data -0.69256073,-0.48116827,0.58852357
3138 Data -0.69256073,-0.6706183,0.91147643
3139 Data -0.69256073,-0.48116827,0.91147643
3140 Data -1.5566552,1.4938041,1.5635463
3141 Data -0.7149141,0.9238256,0.566052
3142 Data -0.34725443,0.9238256,0.566052
3143 Rem --------------
4001 Data 1,2,3
4002 Data 4,5,6
4003 Data 7,8,9
4004 Data 10,11,12
4005 Data 13,14,15
4006 Data 15,16,13
4007 Data 3,8,17
4008 Data 12,18,19
4009 Data 20,21,22
4010 Data 20,18,23
4011 Data 24,12,25
4012 Data 10,26,27
4013 Data 7,28,27
4014 Data 19,29,12
4015 Data 27,14,30
4016 Data 27,31,15
4017 Data 27,32,10
4018 Data 10,33,34
4019 Data 33,35,36
4020 Data 30,35,32
4021 Data 23,31,20
4022 Data 28,20,31
4023 Data 9,21,28
4024 Data 1,21,37
4025 Data 4,38,39
4026 Data 40,4,6
4027 Data 12,40,25
4028 Data 6,41,17
4029 Data 17,40,6
4030 Data 30,14,42
4031 Data 43,42,14
4032 Data 36,42,44
4033 Data 1,45,46
4034 Data 1,47,22
4035 Data 22,48,20
4036 Data 20,38,19
4037 Data 3,41,45
4038 Data 41,49,50
4039 Data 38,51,39
4040 Data 39,49,5
4041 Data 45,52,46
4042 Data 46,53,47
4043 Data 48,53,54
4044 Data 41,55,45
4045 Data 38,54,56
4046 Data 49,51,56
4047 Data 57,58,59
4048 Data 57,60,61
4049 Data 62,58,63
4050 Data 62,60,64
4051 Data 61,63,58
4052 Data 3,65,9
4053 Data 66,67,2
4054 Data 2,68,65
4055 Data 37,9,65
4056 Data 37,66,1
4057 Data 25,59,24
4058 Data 17,57,25
4059 Data 8,59,62
4060 Data 17,62,64
4061 Data 69,70,26
4062 Data 10,71,69
4063 Data 7,71,24
4064 Data 27,72,7
4065 Data 71,73,69
4066 Data 72,74,71
4067 Data 72,70,75
4068 Data 70,74,75
4069 Data 66,68,76
4070 Data 67,76,68
4071 Data 77,78,79
4072 Data 80,81,82
4073 Data 83,84,85
4074 Data 11,86,87
4075 Data 88,13,89
4076 Data 89,16,90
4077 Data 79,83,85
4078 Data 18,87,91
4079 Data 92,93,94
4080 Data 18,92,23
4081 Data 87,95,96
4082 Data 97,98,86
4083 Data 84,94,85
4084 Data 99,91,87
4085 Data 97,88,89
4086 Data 90,97,89
4087 Data 100,97,86
4088 Data 33,86,34
4089 Data 33,101,100
4090 Data 101,102,100
4091 Data 90,23,92
4092 Data 94,90,92
4093 Data 93,85,94
4094 Data 93,78,103
4095 Data 81,104,91
4096 Data 81,105,82
4097 Data 105,87,96
4098 Data 106,82,107
4099 Data 107,105,96
4100 Data 102,108,88
4101 Data 108,43,88
4102 Data 108,36,44
4103 Data 78,109,79
4104 Data 110,78,111
4105 Data 112,111,92
4106 Data 104,92,91
4107 Data 79,106,107
4108 Data 106,113,80
4109 Data 114,104,115
4110 Data 113,115,80
4111 Data 116,109,117
4112 Data 118,117,110
4113 Data 112,118,110
4114 Data 119,106,109
4115 Data 104,120,112
4116 Data 119,116,118
4117 Data 121,122,123
4118 Data 122,124,125
4119 Data 126,121,123
4120 Data 124,126,125
4121 Data 127,128,124
4122 Data 129,79,85
4123 Data 130,131,77
4124 Data 132,77,129
4125 Data 103,129,85
4126 Data 131,103,78
4127 Data 123,96,95
4128 Data 122,107,96
4129 Data 83,123,95
4130 Data 107,126,83
4131 Data 133,98,134
4132 Data 98,95,86
4133 Data 135,84,95
4134 Data 84,134,97
4135 Data 136,135,98
4136 Data 137,138,135
4137 Data 138,133,134
4138 Data 137,133,139
4139 Data 131,132,129
4140 Data 130,132,140
4141 Data 1,66,2
4142 Data 4,39,5
4143 Data 7,24,8
4144 Data 10,34,11
4145 Data 13,43,14
4146 Data 15,31,16
4147 Data 3,9,8
4148 Data 12,11,18
4149 Data 20,28,21
4150 Data 20,19,18
4151 Data 24,10,12
4152 Data 10,69,26
4153 Data 7,9,28
4154 Data 19,4,29
4155 Data 27,15,14
4156 Data 27,28,31
4157 Data 27,30,32
4158 Data 10,32,33
4159 Data 33,32,35
4160 Data 30,42,35
4161 Data 23,16,31
4162 Data 9,37,21
4163 Data 1,22,21
4164 Data 4,19,38
4165 Data 40,29,4
4166 Data 12,29,40
4167 Data 6,5,41
4168 Data 17,25,40
4169 Data 43,44,42
4170 Data 36,35,42
4171 Data 1,3,45
4172 Data 1,46,47
4173 Data 22,47,48
4174 Data 20,48,38
4175 Data 3,17,41
4176 Data 41,5,49
4177 Data 38,56,51
4178 Data 39,51,49
4179 Data 45,55,52
4180 Data 46,52,53
4181 Data 48,47,53
4182 Data 41,50,55
4183 Data 38,48,54
4184 Data 56,54,55
4185 Data 54,53,55
4186 Data 53,52,55
4187 Data 55,50,56
4188 Data 50,49,56
4189 Data 57,61,58
4190 Data 57,64,60
4191 Data 62,59,58
4192 Data 62,63,60
4193 Data 61,60,63
4194 Data 3,2,65
4195 Data 66,76,67
4196 Data 2,67,68
4197 Data 37,65,66
4198 Data 25,57,59
4199 Data 17,64,57
4200 Data 8,24,59
4201 Data 17,8,62
4202 Data 69,73,70
4203 Data 10,24,71
4204 Data 7,72,71
4205 Data 27,26,72
4206 Data 71,74,73
4207 Data 72,75,74
4208 Data 72,26,70
4209 Data 70,73,74
4210 Data 66,65,68
4211 Data 77,131,78
4212 Data 80,115,81
4213 Data 83,95,84
4214 Data 11,34,86
4215 Data 88,43,13
4216 Data 89,13,16
4217 Data 79,107,83
4218 Data 18,11,87
4219 Data 92,111,93
4220 Data 18,91,92
4221 Data 87,86,95
4222 Data 97,134,98
4223 Data 84,97,94
4224 Data 99,81,91
4225 Data 97,102,88
4226 Data 90,94,97
4227 Data 100,102,97
4228 Data 33,100,86
4229 Data 33,36,101
4230 Data 101,108,102
4231 Data 90,16,23
4232 Data 93,103,85
4233 Data 93,111,78
4234 Data 81,115,104
4235 Data 81,99,105
4236 Data 105,99,87
4237 Data 106,80,82
4238 Data 107,82,105
4239 Data 108,44,43
4240 Data 108,101,36
4241 Data 78,117,109
4242 Data 110,117,78
4243 Data 112,110,111
4244 Data 104,112,92
4245 Data 79,109,106
4246 Data 106,141,113
4247 Data 114,142,104
4248 Data 113,114,115
4249 Data 116,119,109
4250 Data 118,116,117
4251 Data 112,120,118
4252 Data 119,141,106
4253 Data 104,142,120
4254 Data 118,120,119
4255 Data 120,142,119
4256 Data 142,114,113
4257 Data 113,141,142
4258 Data 141,119,142
4259 Data 121,127,122
4260 Data 122,127,124
4261 Data 126,128,121
4262 Data 124,128,126
4263 Data 127,121,128
4264 Data 129,77,79
4265 Data 130,140,131
4266 Data 132,130,77
4267 Data 131,129,103
4268 Data 123,122,96
4269 Data 122,125,107
4270 Data 83,126,123
4271 Data 107,125,126
4272 Data 133,136,98
4273 Data 98,135,95
4274 Data 135,138,84
4275 Data 84,138,134
4276 Data 136,137,135
4277 Data 137,139,138
4278 Data 138,139,133
4279 Data 137,136,133
4280 Data 131,140,132
