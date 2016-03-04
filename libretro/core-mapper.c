#include "libretro.h"
#include "libretro-core.h"
#include "retroscreen.h"
//CORE VAR
#ifdef _WIN32
char slash = '\\';
#else
char slash = '/';
#endif
extern const char *retro_save_directory;
extern const char *retro_system_directory;
extern const char *retro_content_directory;
char RETRO_DIR[512];

char DISKA_NAME[512]="\0";
char DISKB_NAME[512]="\0";
char TAPE_NAME[512]="\0";

//TIME
#ifdef __CELLOS_LV2__
#include "sys/sys_time.h"
#include "sys/timer.h"
#define usleep  sys_timer_usleep
#else
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#endif

extern void Screen_SetFullUpdate(int scr);

long frame=0;
unsigned long  Ktime=0 , LastFPSTime=0;

//VIDEO
#ifdef  RENDER16B
	uint16_t Retro_Screen[400*300];
#else
	unsigned int Retro_Screen[400*300];
#endif 

//SOUND
short signed int SNDBUF[1024*2];
int snd_sampler_pal = 44100 / 50;
int snd_sampler_ntsc = 44100 / 60;

//PATH
char RPATH[512];

//EMU FLAGS
int NPAGE=-1, KCOL=1, BKGCOLOR=0;
int SHOWKEY=-1;

#if defined(ANDROID) || defined(__ANDROID__)
int MOUSE_EMULATED=1;
#else
int MOUSE_EMULATED=-1;
#endif
int MAXPAS=6,SHIFTON=-1,MOUSEMODE=-1,PAS=4;
int SND=1; //SOUND ON/OFF
static int firstps=0;
int pauseg=0; //enter_gui
int touch=-1; // gui mouse btn
//JOY
int al[2][2];//left analog1
int ar[2][2];//right analog1
unsigned char MXjoy[2]; // joy
int NUMjoy=1;
int NUMDRV=1;

//MOUSE
extern int pushi;  // gui mouse btn
int gmx,gmy; //gui mouse
int c64mouse_enable=0;
int mouse_wu=0,mouse_wd=0;
//KEYBOARD
char Key_Sate[512];
char Key_Sate2[512];
static char old_Key_Sate[512];

int mbt[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//STATS GUI
int BOXDEC= 32+2;
int STAT_BASEY;

/*static*/ retro_input_state_t input_state_cb;
static retro_input_poll_t input_poll_cb;


void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

long GetTicks(void)
{ // in MSec
#ifndef _ANDROID_

#ifdef __CELLOS_LV2__

   //#warning "GetTick PS3\n"

   unsigned long	ticks_micro;
   uint64_t secs;
   uint64_t nsecs;

   sys_time_get_current_time(&secs, &nsecs);
   ticks_micro =  secs * 1000000UL + (nsecs / 1000);

   return ticks_micro;///1000;
#else
   struct timeval tv;
   gettimeofday (&tv, NULL);
   return (tv.tv_sec*1000000 + tv.tv_usec);///1000;
#endif

#else

   struct timespec now;
   clock_gettime(CLOCK_MONOTONIC, &now);
   return (now.tv_sec*1000000 + now.tv_nsec/1000);///1000;
#endif

} 

int slowdown=0;

void gui_poll_events(void)
{
   Ktime = GetTicks();

   if(Ktime - LastFPSTime >= 1000/50)
   {
	  slowdown=0;
      frame++; 
      LastFPSTime = Ktime;
		
      co_switch(mainThread);

   }
}

void retro_mouse(int a,int b) {}
void retro_mouse_but0(int a) {}
void retro_mouse_but1(int a) {}
void enter_options(void) {}

#if defined(ANDROID) || defined(__ANDROID__)
#define DEFAULT_PATH "/mnt/sdcard/"
#else

#ifdef PS3PORT
#define DEFAULT_PATH "/dev_hdd0/HOMEBREW/"
#else
#define DEFAULT_PATH "/"
#endif

#endif

int STATUTON=-1;
#define RETRO_DEVICE_ATARI_KEYBOARD RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_KEYBOARD, 0)
#define RETRO_DEVICE_ATARI_JOYSTICK RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 1)

void texture_uninit(void)
{

}

void texture_init(void)
{
   memset(Retro_Screen, 0, sizeof(Retro_Screen));
   memset(old_Key_Sate ,0, sizeof(old_Key_Sate));

   gmx=(retrow/2)-1;
   gmy=(retroh/2)-1;
}

int bitstart=0;
int pushi=0; //mouse button
int keydown=0,keyup=0;
int KBMOD=-1;
int RSTOPON=-1;
int CTRLON=-1;

extern unsigned short int bmp[400*300];
extern unsigned atari_devices[ 2 ];

#include "pokeysnd.h"
extern int UI_is_active;
extern int CURRENT_TV;

void retro_sound_update()
{	
	int x,stop=CURRENT_TV==312?885:742;//FIXME: 882/735?
	
	if (! UI_is_active) {

		Sound_Callback(SNDBUF, 1024*2*2);
		//POKEYSND_Process(SNDBUF,snd_sampler_pal);
		for(x=0;x<stop*2;x+=2)
			retro_audio_cb(SNDBUF[x],SNDBUF[x+2]);

	}

}

//FIXME in kdbauto.c
extern void vkbd_key(int key,int pressed);

void vkbd_key(int key,int pressed){

	//printf("key(%x)=%x shift:%d\n",key,pressed,SHIFTON);
	if(pressed){

		if(SHIFTON==1)
			;//keyboard_matrix[0x25 >> 4] &= ~bit_values2[0x25 & 7]; // key needs to be SHIFTed

		//keyboard_matrix[(unsigned char)key >> 4] &= ~bit_values2[(unsigned char)key & 7]; // key is being held down

	}
	else {
		if(SHIFTON==1)
			;//keyboard_matrix[0x25 >> 4] |= bit_values2[0x25 & 7]; // make sure key is unSHIFTed

		//keyboard_matrix[(unsigned char)key >> 4] |= bit_values2[(unsigned char)key & 7];

	}

}

void retro_key_down(int key)
{
	int code;
/*
	if(key<512)
 		code=KeySymToCPCKey[key];	
	else code = CPC_KEY_NULL;
	CPC_SetKey(code);
*/
}

void retro_key_up(int key)
{
	int code;
/*
	if(key<512)
 		code=KeySymToCPCKey[key];
	else code = CPC_KEY_NULL;
	CPC_ClearKey(code);
*/
}

static int joy0_flag[6]={0,0,0,0,0,0};
   //0x01,0x02,0x04,0x08,0x80 0x40
   // UP  DWN  LEFT RGT  BTN0 BTN1
   // 0    1     2   3    4    5
void retro_joy0_test(unsigned char joy0)
{

	if(joy0&0x01 && joy0_flag[0]==0 ){
		//keyboard_matrix[(unsigned char)0x90 >> 4] &= ~bit_values2[(unsigned char)0x90 & 7]; 
		joy0_flag[0]=1;
	}
	else if(joy0_flag[0]){
		//keyboard_matrix[(unsigned char)0x90 >> 4] |= bit_values2[(unsigned char)0x90 & 7];
		joy0_flag[0]=0;
	}

	if(joy0&0x02 && joy0_flag[1]==0 ){
		//keyboard_matrix[(unsigned char)0x91 >> 4] &= ~bit_values2[(unsigned char)0x91 & 7]; 
		joy0_flag[1]=1;
	}
	else if(joy0_flag[1]){
		//keyboard_matrix[(unsigned char)0x91 >> 4] |= bit_values2[(unsigned char)0x91 & 7];
		joy0_flag[1]=0;
	}

	if(joy0&0x04 && joy0_flag[2]==0 ){
		//keyboard_matrix[(unsigned char)0x92 >> 4] &= ~bit_values2[(unsigned char)0x92 & 7]; 
		joy0_flag[2]=1;
	}
	else if(joy0_flag[2]){
		//keyboard_matrix[(unsigned char)0x92 >> 4] |= bit_values2[(unsigned char)0x92 & 7];
		joy0_flag[2]=0;
	}

	if(joy0&0x08 && joy0_flag[3]==0 ){
		//keyboard_matrix[(unsigned char)0x93 >> 4] &= ~bit_values2[(unsigned char)0x93 & 7]; 
		joy0_flag[3]=1;
	}
	else if(joy0_flag[3]){
		//keyboard_matrix[(unsigned char)0x93 >> 4] |= bit_values2[(unsigned char)0x93 & 7];
		joy0_flag[3]=0;
	}

	if(joy0&0x40 && joy0_flag[5]==0 ){
		//keyboard_matrix[(unsigned char)0x95 >> 4] &= ~bit_values2[(unsigned char)0x95 & 7]; 
		joy0_flag[5]=1;
	}
	else if(joy0_flag[5]){
		//keyboard_matrix[(unsigned char)0x95 >> 4] |= bit_values2[(unsigned char)0x95 & 7];
		joy0_flag[5]=0;
	}

	if(joy0&0x80 && joy0_flag[4]==0 ){
		//keyboard_matrix[(unsigned char)0x94 >> 4] &= ~bit_values2[(unsigned char)0x94 & 7]; 
		joy0_flag[4]=1;
	}
	else if(joy0_flag[4]){
		//keyboard_matrix[(unsigned char)0x94 >> 4] |= bit_values2[(unsigned char)0x94 & 7];
		joy0_flag[4]=0;
	}

}


void retro_virtualkb(void)
{
   // VKBD
   int i;
   //   RETRO        B    Y    SLT  STA  UP   DWN  LEFT RGT  A    X    L    R    L2   R2   L3   R3
   //   INDEX        0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15
   static int oldi=-1;
   static int vkx=0,vky=0;

   if(oldi!=-1)
   {
	  //retro_key_up(oldi);
	  vkbd_key(oldi,0);
      oldi=-1;
   }

   if(SHOWKEY==1)
   {
      static int vkflag[5]={0,0,0,0,0};		

      if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP) && vkflag[0]==0 )
         vkflag[0]=1;
      else if (vkflag[0]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP) )
      {
         vkflag[0]=0;
         vky -= 1; 
      }

      if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN) && vkflag[1]==0 )
         vkflag[1]=1;
      else if (vkflag[1]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN) )
      {
         vkflag[1]=0;
         vky += 1; 
      }

      if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT) && vkflag[2]==0 )
         vkflag[2]=1;
      else if (vkflag[2]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT) )
      {
         vkflag[2]=0;
         vkx -= 1;
      }

      if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT) && vkflag[3]==0 )
         vkflag[3]=1;
      else if (vkflag[3]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT) )
      {
         vkflag[3]=0;
         vkx += 1;
      }

      if(vkx<0)vkx=NPLGN-1;
      if(vkx>NPLGN-1)vkx=0;
      if(vky<0)vky=NLIGN-1;
      if(vky>NLIGN-1)vky=0;

      virtual_kdb(( char *)Retro_Screen,vkx,vky);
 
      i=8;
      if(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i)  && vkflag[4]==0) 	
         vkflag[4]=1;
      else if( !input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i)  && vkflag[4]==1)
      {
         vkflag[4]=0;
         i=check_vkey2(vkx,vky);

         if(i==-1){
            oldi=-1;
		 }
         if(i==-2)
         {
            NPAGE=-NPAGE;oldi=-1;
         }
         else if(i==-3)
         {
            //KDB bgcolor
            KCOL=-KCOL;
            oldi=-1;
         }
         else if(i==-4)
         {
            //VKbd show/hide 			
            oldi=-1;
            Screen_SetFullUpdate(0);
            SHOWKEY=-SHOWKEY;
         }
         else if(i==-5)
         {
			//FLIP DSK PORT 1-2
			NUMDRV=-NUMDRV;
            oldi=-1;
         }
         else if(i==-6)
         {
			//Exit
			retro_deinit();
			oldi=-1;
            exit(0);
         }
         else if(i==-7)
         {
			//SNA SAVE
		//	snapshot_save (RPATH);
            oldi=-1;
         }
         else if(i==-8)
         {
			//PLAY TAPE
		//	play_tape();
            oldi=-1;
         }

         else
         {

            if(i==0x25 /*i==-10*/) //SHIFT
            {
//			   if(SHIFTON == 1)retro_key_up(RETROK_RSHIFT);
//			   else retro_key_down(RETROK_LSHIFT);
               SHIFTON=-SHIFTON;

               oldi=-1;
            }
            else if(i==0x27/*i==-11*/) //CTRL
            {     
//         	   if(CTRLON == 1)retro_key_up(RETROK_LCTRL);
///			   else retro_key_down(RETROK_LCTRL);
               CTRLON=-CTRLON;

               oldi=-1;
            }
			else if(i==-12) //RSTOP
            {
//           	   if(RSTOPON == 1)retro_key_up(RETROK_ESCAPE);
//			   else retro_key_down(RETROK_ESCAPE);            
               RSTOPON=-RSTOPON;

               oldi=-1;
            }
			else if(i==-13) //GUI
            {     
			   // pauseg=1;

				SHOWKEY=-SHOWKEY;
				Screen_SetFullUpdate(0);
               oldi=-1;
            }
			else if(i==-14) //JOY PORT TOGGLE
            {    
 				//cur joy toggle
				//cur_port++;if(cur_port>2)cur_port=1;
               SHOWKEY=-SHOWKEY;
               oldi=-1;
            }
            else
            {
               oldi=i;
			   //retro_key_down(oldi); 
			   vkbd_key(oldi,1);            
            }

         }
      }

	}


}

void Screen_SetFullUpdate(int scr)
{
   if(scr==0 ||scr>1)memset(Retro_Screen, 0, sizeof(Retro_Screen));
  // if(scr>0)memset(bmp,0,sizeof(bmp));
}

void Process_key()
{
	int i;

	for(i=0;i<320;i++)
        	Key_Sate[i]=input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,i) ? 0x80: 0;
   
	if(memcmp( Key_Sate,old_Key_Sate , sizeof(Key_Sate) ) )
	 	for(i=0;i<320;i++)
			if(Key_Sate[i] && Key_Sate[i]!=old_Key_Sate[i]  )
        	{	
				if(i==RETROK_F12){
					//play_tape();
					continue;
				}
/*
				if(i==RETROK_RCTRL){
					//CTRLON=-CTRLON;
					printf("Modifier crtl pressed %d \n",CTRLON); 
					continue;
				}
				if(i==RETROK_RSHIFT){
					//SHITFON=-SHITFON;
					printf("Modifier shift pressed %d \n",SHIFTON); 
					continue;
				}
*/
				if(i==RETROK_LALT){
					//KBMOD=-KBMOD;
					printf("Modifier alt pressed %d \n",KBMOD); 
					continue;
				}
//printf("press: %d \n",i);
				retro_key_down(i);
	
        	}	
        	else if ( !Key_Sate[i] && Key_Sate[i]!=old_Key_Sate[i]  )
        	{
				if(i==RETROK_F12){
      				//kbd_buf_feed("|tape\nrun\"\n^");
					continue;
				}
/*
				if(i==RETROK_RCTRL){
					CTRLON=-CTRLON;
					printf("Modifier crtl released %d \n",CTRLON); 
					continue;
				}
				if(i==RETROK_RSHIFT){
					SHIFTON=-SHIFTON;
					printf("Modifier shift released %d \n",SHIFTON); 
					continue;
				}
*/
				if(i==RETROK_LALT){
					KBMOD=-KBMOD;
					printf("Modifier alt released %d \n",KBMOD); 
					continue;
				}
//printf("release: %d \n",i);
				retro_key_up(i);
	
        	}	

	memcpy(old_Key_Sate,Key_Sate , sizeof(Key_Sate) );

}
/*
void Print_Statut(void)
{
   DrawFBoxBmp(bmp,0,CROP_HEIGHT,CROP_WIDTH,STAT_YSZ,RGB565(0,0,0));

   Draw_text(bmp,STAT_DECX+40 ,STAT_BASEY,0xffff,0x8080,1,2,40,(SND>0?"SND":""));
   Draw_text(bmp,STAT_DECX+80 ,STAT_BASEY,0xffff,0x8080,1,2,40,"F:%d",dwFPS);
   Draw_text(bmp,STAT_DECX+120,STAT_BASEY,0xffff,0x8080,1,2,40,"DSK%c",NUMDRV>0?'A':'B');
   if(ZOOM>-1)
      Draw_text(bmp,(384-Mres[ZOOM].x)/2,(272-Mres[ZOOM].y)/2,0xffff,0x8080,1,2,40,"x:%3d y:%3d",Mres[ZOOM].x,Mres[ZOOM].y);
}
*/


/*
In joy mode
L3  GUI LOAD
R3  GUI SNAP 
L2  STATUS ON/OFF
R2  AUTOLOAD TAPE
L   CAT
R   RESET
SEL MOUSE/JOY IN GUI
STR ENTER/RETURN
A   FIRE1/VKBD KEY
B   RUN
X   FIRE2
Y   VKBD ON/OFF
In Keayboard mode
F8 LOAD DSK/TAPE
F9 MEM SNAPSHOT LOAD/SAVE
F10 MAIN GUI
F12 PLAY TAPE
*/

int Retro_PollEvent()
{
	//   RETRO        B    Y    SLT  STA  UP   DWN  LEFT RGT  A    X    L    R    L2   R2   L3   R3
    //   INDEX        0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15

   int SAVPAS=PAS;	
   int i;
   static int vbt[16]={0x0,0x0,0x0,0x0,0x01,0x02,0x04,0x08,0x80,0x40,0x0,0x0,0x0,0x0,0x0,0x0};
   //MXjoy[0]=0;

   input_poll_cb();

   int mouse_l;
   int mouse_r;
   int16_t mouse_x,mouse_y;
   mouse_x=mouse_y=0;

   if(SHOWKEY==-1 && pauseg==0)Process_key();

 
if(pauseg==0){ // if emulation running

	  //Joy mode

      for(i=4;i<10;i++)
      {
         if( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i))
            MXjoy[0] |= vbt[i]; // Joy press	
		 else if( MXjoy[0]&vbt[i])MXjoy[0] &= ~vbt[i]; // Joy press
      }

      if(SHOWKEY==-1)retro_joy0_test(MXjoy[0]);


if(atari_devices[0]==RETRO_DEVICE_ATARI_JOYSTICK){
   //shortcut for joy mode only

   i=1;//show vkbd toggle
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) )
   {
      mbt[i]=0;
      SHOWKEY=-SHOWKEY;
   }

   i=3;//type ENTER
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) )
   {
      mbt[i]=0;
	 // kbd_buf_feed("\n");
   }

/*
   i=10;//type DEL / ZOOM
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
      ZOOM++;if(ZOOM>4)ZOOM=-1;

   }
*/

   i=0;//type RUN"
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
	// kbd_buf_feed("RUN\"");
   }

   i=10;//Type CAT\n 
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
	//  kbd_buf_feed("CAT\n");
      //Screen_SetFullUpdate();
   }

   i=12;//show/hide statut
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
      STATUTON=-STATUTON;
     // Screen_SetFullUpdate();
   }

   i=13;//auto load tape
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
    // kbd_buf_feed("|tape\nrun\"\n^");
   }

   i=11;//reset
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
      //emu_reset();		
   }

   i=2;//mouse/joy toggle
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
      MOUSE_EMULATED=-MOUSE_EMULATED;
   }

   // L3 -> gui load
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, 14)){ 

   }
   // R3 -> gui snapshot
   if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, 15)){ 

   }

}//if atari_devices=joy


}// if pauseg=0
else{
   // if in gui

   i=2;//mouse/joy toggle
   if ( input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) && mbt[i]==0 )
      mbt[i]=1;
   else if ( mbt[i]==1 && ! input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, i) ){
      mbt[i]=0;
      MOUSE_EMULATED=-MOUSE_EMULATED;
   }

}

   if(MOUSE_EMULATED==1){

	  if(slowdown>0)return 1;

      if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))mouse_x += PAS;
      if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))mouse_x -= PAS;
      if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))mouse_y += PAS;
      if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))mouse_y -= PAS;
      mouse_l=input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
      mouse_r=input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);

      PAS=SAVPAS;

	  slowdown=1;
   }
   else {
//printf("-----------------%d \n",pauseg);
      mouse_wu = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELUP);
      mouse_wd = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_WHEELDOWN);
if(mouse_wu || mouse_wd)printf("-----------------MOUSE UP:%d DOWN:%d\n",mouse_wu, mouse_wd);
      mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
      mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
      mouse_l    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
      mouse_r    = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
   }

   static int mmbL=0,mmbR=0;

   if(mmbL==0 && mouse_l){

      mmbL=1;		
      pushi=1;
	  touch=1;

   }
   else if(mmbL==1 && !mouse_l) {

      mmbL=0;
      pushi=0;
	  touch=-1;
   }

   if(mmbR==0 && mouse_r){
      mmbR=1;		
   }
   else if(mmbR==1 && !mouse_r) {
      mmbR=0;
   }

if(pauseg==0 && c64mouse_enable){
/*
	mouse_move((int)mouse_x, (int)mouse_y);
	mouse_button(0,mmbL);
	mouse_button(1,mmbR);
*/
}

   gmx+=mouse_x;
   gmy+=mouse_y;
   if(gmx<0)gmx=0;
   if(gmx>retrow-1)gmx=retrow-1;
   if(gmy<0)gmy=0;
   if(gmy>retroh-1)gmy=retroh-1;


  if(SHOWKEY && pauseg==0)retro_virtualkb();

return 1;

}

