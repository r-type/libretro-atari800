#ifndef VKBD_DEF_H
#define VKBD_DEF_H 1

typedef struct {
	char norml[NLETT];
	char shift[NLETT];
	int val;	
} Mvk;

Mvk MVk[NPLGN*NLIGN*2]={

	{ "ESC" ,"ESC"  ,RETROK_ESCAPE },//0
	{ " 1" ," !" , RETROK_1 },//0
	{ " 2" ," \"" ,RETROK_2 },
	{ " 3" ," #"  ,RETROK_3 },
	{ " 4" ," $"  ,RETROK_4 },
	{ " 5" ," %"  ,RETROK_5 },
	{ " 6" ," &"  ,RETROK_6 },
	{ " 7" ," \'"  ,RETROK_7 },
	{ " 8" ," ("  ,RETROK_8 },
	{ " 9" ," )"  ,RETROK_9 },
	{ " 0" ," _"  ,RETROK_0 },
	{ " ^" ,"Pnd"  ,0x30 },

	{ " q" ," Q"  ,0x83}, //10+2
	{ " w" ," W"  ,0x73},
	{ " e" ," E"  ,0x72},
	{ " r" ," R"  ,RETROK_r},
	{ " t" ," T"  ,0x63},
	{ " y" ," Y"  ,0x53},
	{ " u" ," U"  ,0x52},
	{ " i" ," I"  ,0x43},
	{ " o" ," O"  ,0x42},
	{ " p" ," P"  ,RETROK_p},
	{ " @" ," |"  ,0x32},
	{ " [" ," ["  ,0x21},

	{ " a" ," A"  ,0x85}, //20+4
	{ " s" ," S"  ,0x74},
	{ " d" ," D"  ,0x75},
	{ " f" ," F"  ,0x65},
	{ " g" ," G"  ,0x64},
	{ " h" ," H"  ,0x54},
	{ " j" ," J"  ,0x55},
	{ " k" ," K"  ,0x45},	
	{ " l" ," L"  ,0x44},
	{ " :" ," *"  ,0x35},
	{ " ;" ," +"  ,0x34},
	{ " ]" ," ]"  ,0x23},

	{ " z" ," Z"  ,0x87},//30+6
	{ " x" ," X"  ,0x77},
	{ " c" ," C"  ,0x76},
	{ " v" ," V"  ,0x67},
	{ " b" ," B"  ,0x66},
	{ " n" ," N"  ,0x56},
	{ " m"," M"   ,0x46},
	{ " ,"," <"   ,0x47},
	{ " ."," >"   ,0x37},
	{ " /" ," ?"  ,0x36},
	{ " \\"," \\"   ,0x26},
	{ "SHFT" ,"SHFT"  ,0x25},

	{ "PG2","PG2" ,-2}, //40+8
	{ "TAB","TAB" ,0x54},
	{ "CPSL" ,"CPSL"  ,0x86},
	{ "RET" ,"RET"  ,0x22},
	{ "DEL" ,"DEL"  ,0x97},
	{ "CTRL" ,"CTRL"  ,0x27},
	{ "CLR" ,"CLR" , 0x20},
	{ "Spc" ,"Spc",0x57},
	{ "= " ,"= "  ,RETROK_EQUALS},
	{ " *" ," *",  RETROK_ASTERISK},
	{ "# " ,"# "  ,RETROK_HASH},
	{ "Ent" ,"Ent",0x06},


	{ "ESC" ,"ESC"  ,RETROK_ESCAPE },//50+1°
	{ " 1" ," !" , RETROK_1 },//0
	{ " 2" ," \"" ,RETROK_2 },
	{ " 3" ," #"  ,RETROK_3 },
	{ " 4" ," $"  ,RETROK_4 },
	{ " 5" ," %"  ,RETROK_5 },
	{ " 6" ," &"  ,RETROK_6 },
	{ " 7" ," \'"  ,RETROK_7 },
	{ " 8" ," ("  ,RETROK_8 },
	{ " 9" ," )"  ,RETROK_9 },
	{ " 0" ," _"  ,RETROK_0 },
	{ " ^" ,"Pnd"  ,0x30 },

	{ " F7" ," F7"  ,RETROK_F7}, //60+12
	{ " F8" ," F8"  ,RETROK_F8},
	{ " F9" ," F9"  ,RETROK_F9},
	{ " F0" ," F0"  ,RETROK_F10},
	{ " t" ," T"  ,0x63},
	{ " /\\" ," /\\"  ,0x00},
	{ " u" ," U"  ,0x52},
	{ " i" ," I"  ,0x43},
	{ " o" ," O"  ,0x42},
	{ " p" ," P"  ,0x33},
	{ " @" ," |"  ,0x32},
	{ " [" ," ["  ,0x21},

	{ " F4" ," F4"  ,RETROK_F4}, //70+14
	{ " F5" ," F5"  ,RETROK_F5},
	{ " F6" ," F6"  ,RETROK_F6},
	{ " ." ," ."  ,0x07},
	{ " <-" ," <-"  ,0x10},
	{ "COPY" ,"COPY"  ,0x11},
	{ " ->" ," ->"  ,0x01},
	{ " r" ," R"  ,RETROK_r},
	{ " p" ," P"  ,RETROK_p},
	{ "= " ,"= "  ,RETROK_EQUALS},
	{ " *" ," *",  RETROK_ASTERISK},
	{ "# " ,"# "  ,RETROK_HASH},

	{ " F1" ," F1"  ,RETROK_F1},//80+16
	{ " F2" ," F2"  ,RETROK_F2},
	{ " F3" ," F3"  ,RETROK_F3},
	{ "Ent" ,"Ent"  ,0x06},
	{ " b" ," B"  ,0x66},
	{ " \\/" ," \\/"  ,0x02},
	{ " m"," M"   ,0x46},
	{ " ,"," <"   ,0x47},
	{ " ."," >"   ,0x37},
	{ "TAPE" ,"TAPE"  ,-8},
	{ "EXIT","EXIT"   ,-6},
	{ "SNA" ,"SNA"  ,-7},


	{ "PG1","PG1"  ,-2},//90+18
	{ "DSK","DSK"  ,-5},
	{ "GUI","GUI"  ,-13},
	{ "COL" ,"COL",-3},
	{ "CTRL" ,"CTRL" ,0x27},
	{ "SPC" ,"SPC" ,0x57},
	{ "SHFT" ,"SHFT" ,0x25},
	{ "ESC","ESC",0x82},
	{ "CLR" ,"CLR",0x20},
	{ "DEL" ,"DEL",0x97},
	{ "Ent" ,"Ent",0x22},
	{ "KBD" ,"KBD",-4},

} ;


#endif
