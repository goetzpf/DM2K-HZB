;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; This file contains color rule sets used in MEDM.
; You can edit it to add new sets or replace existents.
;
; Format of the file:
;  - any information started by ``;'' and up to the end of a line is considered
;    as a commentary;
;
;  - each color rule set begins with keyword ``colorRule'' followed by
;    the name of a color rule set, it MUST be one word without spaces 
;    (set_Cryogenic, my-color-rule, 13891235%^&*=\);
;
;  - each entry in a color rule set has to be on one line and 
;    to have three numbers:
;    lowest, highest limits of a value, and an index of a color in a color map;
;
;  - a color rule set can have any amount of rules within itself
;    (not just 16 as it was before);
;
;  - a color rule set finished by either a starting new rule set or by 
;    the end of file;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  Standart colormap for MEDM is here.
;
;  Note:
;       If You want to see a color on Your screen, please, start medm and open 
;       Color Palette. Colors on the Color Palette are numerated by columns 
;       from left to right. Within column it goes from up to down.
;
;
;N      [red] [green] [blue]
;
;0	 255,   255,   255     white
;1	 236,   236,   236
;2	 218,   218,   218
;3	 200,   200,   200
;4	 187,   187,   187
;5	 174,   174,   174
;6	 158,   158,   158     gray from white to black
;7	 145,   145,   145
;8	 133,   133,   133
;9	 120,   120,   120
;10	 105,   105,   105
;11	 90,    90,    90
;12	 70,    70,    70
;13	 45,    45,    45
;14	 0,     0,     0      black
;15	 0,     216,   0      green
;16	 30,    187,   0
;17	 51,    153,   0
;18	 45,    127,   0
;19	 33,    108,   0
;20	 253,   0,     0      red
;21	 222,   19,    9
;22	 190,   25,    11
;23	 160,   18,    7
;24	 130,   4,     0 
;25	 88,    147,   255    blue
;26	 89,    126,   225
;27	 75,    110,   199
;28	 58,    94,    171
;29	 39,    84,    141
;30	 251,   243,   74     yellow
;31	 249,   218,   60
;32	 238,   182,   43
;33	 225,   144,   21
;34	 205,   97,    0
;35	 255,   176,   255    violet
;36	 214,   127,   226
;37	 174,   78,    188
;38	 139,   26,    150
;39	 97,    10,    117
;40	 164,   170,   255    blue
;41	 135,   147,   226
;42	 106,   115,   193
;43	 77,    82,    164
;44	 52,    51,    134
;45	 199,   187,   109    brown
;46	 183,   157,   92
;47	 164,   126,   60
;48	 125,   86,    39
;49	 88,    52,    15
;50	 153,   255,   255    light blue
;51	 115,   223,   255
;52	 78,    165,   249
;53	 42,    99,    228
;54	 10,    0,     184
;55	 235,   241,   181    sand 
;56	 212,   219,   157
;57	 187,   193,   135
;58	 166,   164,   98
;59	 139,   130,   57
;60	 115,   255,   107    light green
;61	 82,    218,   59
;62	 60,    180,   32
;63	 40,    147,   21
;64	 26,    115,   9
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

colorRule Severity
	0	0	14
	1	1	30
	2	2	20
	3	3	0

colorRule BeamPosRMS
	0       0.06    15
	0.06    1       20

colorRule  set#1
	 0.00   0.00   20
	 1.00   1.00   16; it was 15 before 1.08.97
	 2.00   2.00   22
	 3.00   3.00   17
	 4.00   4.00   35
	 5.00   5.00   55
	 6.00   6.00   40
	 7.00   7.00   40
	 8.00   8.00   30
	 9.00   9.00   32
	10.00  10.00   30
	11.00  11.00   32
	12.00  12.00   30
	13.00  13.00   32
	14.00  14.00   30
	15.00  15.00   32

colorRule  set#2
	 0.00   0.00   20
	 1.00   1.00   24
	 2.00   2.00   16; it was 15 before 1.08.97
	 3.00   3.00   19
	 4.00   4.00   55
	 5.00   5.00   30
	 6.00   6.00   53; it was 50 before 1.08.97
	 7.00   7.00   54
	 8.00   8.00   45
	 9.00   9.00   47
	10.00  10.00   40
	11.00  11.00   42
	12.00  12.00   32
	13.00  13.00   33
	14.00  14.00    4
	15.00  15.00   14

colorRule  set#3
	-0.01   0.01  32
	 0.99   1.01  33
	 1.99   2.01  34
	 2.99   3.01  35
	 3.99   4.01  36
	 4.99   5.01  37
	 5.99   6.01  38
	 6.99   7.01  39
	 7.99   8.01  40
	 8.99   9.01  41
	 9.99  10.01  42
	10.99  11.01  43
	11.99  12.01  44
	12.99  13.01  45
	13.99  14.01  46
	14.99  15.01  47

colorRule  set#4
	 1.00   1.00   15
	 0.00   0.00   20
	 2.00   2.00   22
	 3.00   3.00   17
	 4.00   4.00   35
	 5.00   5.00   55
	 6.00   6.00   40
	 7.00   7.00   40
	 8.00   8.00   30
	 9.00   9.00   32
	10.00  10.00   30
	11.00  11.00   32
	12.00  12.00   30
	13.00  13.00   32
	14.00  14.00   30
	15.00  15.00   32

colorRule VacuumLogPressure
	0.0	6.4999	20
	6.5	6.9999	33
	7.0	7.4999	32
	7.5	7.9999	31
	8.0	8.4999	30
	8.5	8.9999	60
	9.0	9.4999	61
	9.5	1000	15

; -------------------------------------------
; Color Rules for HF Panels
; -------------------------------------------

colorRule HF#Std#Back
	0	0	10
	1	1	15
	2	2	30
	3	3	15
	4	4	20

colorRule HF#Std#Text
	0	0	4
	1	1	14
	2	2	14
	3	3	30
	4	4	0

colorRule HF#vAMsgPhs#Back
	0	0	10
	1	1	15
	2	2	20
	3	3	15
	4	4	20

colorRule HF#Stat4#Back
	0	0	15
	1	1	15
	2	2	15
	3	3	15

colorRule HF#Lcl#Text
	0	0	14
	1	1	0
	2	2	30
	3	3	30

colorRule HF#Lcl#Back
	0	0	15
	1	1	20
	2	2	15
	3	3	20

colorRule HF#Stat4#Text
	0	0	14
	1	1	14
	2	2	30
	3	3	30

colorRule HF#Itl#Back
	1	1	15
	2	2	15
	3	3	15
	5	5	15

colorRule HF#Itl#Text
	1	1	14
	2	2	14
	3	3	30
	5	5	30

colorRule HF#Tst#Back
	0	0	15
	1	1	15
	2	2	15
	3	3	15

colorRule HF#Tst#Text
	0	0	14
	1	1	30
	2	2	14
	3	3	30

colorRule HF#Stat3#Back
	0	0	10
	1	1	15
	2	2	15

colorRule HF#Stat3#Text
	0	0	4
	1	1	14
	2	2	30

colorRule HF#Off#Back
	0	0	15
	1	1	10
	2	2	15

colorRule HF#Off#Text
	0	0	14
	1	1	4
	2	2	30

colorRule HF#CY#Back
	0	0	10
	1	1	30

colorRule HF#CY#Text
	0	0	4
	1	1	14

colorRule HF#CYInv#Back
	0	0	30
	1	1	10

colorRule HF#CYInv#Text
	0	0	14
	1	1	4

colorRule HF#CYLcl#Text
	0	0	0
	1	1	14

colorRule HF#CYLcl#Back
	0	0	20
	1	1	15

colorRule HF#CTCstat#Text
	0	0	14
	1	1	14
	2	2	14
	3	3	4

colorRule HF#CTCstat#Back
	0	0	15
	1	1	30
	2	2	20
	3	3	10

; -------------------------------------------
; Color Rules for Power Mux Panels
; -------------------------------------------

colorRule PMux#Status
	0	0	30
	1	1	10
	2	2	15
	3	3	20

; -------------------------------------------
; Color Rules for Landau Panels
; -------------------------------------------

colorRule Landau#home
	0	0	36
	1	3	5

colorRule Landau#set
	0	0	5
	1	1	36
	2	3	5

colorRule Landau#vl
	0	1	5
	2	2	36
	3	3	5

colorRule Landau#vs
	0	2	5
	3	3	36
