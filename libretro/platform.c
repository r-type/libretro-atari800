/*
 * platform.c - platform interface implementation for libretro
 *
 * Copyright (C) 2010 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/* Atari800 includes */
#include "atari.h"
#include "input.h"
#include "log.h"
#include "monitor.h"
#include "platform.h"
#include "sound.h"
#include "videomode.h"
#include "cpu.h"
#include "devices.h"
#include "akey.h"
#include "pokeysnd.h"
#include "sound.h"
#include "screen.h"
#include "colours.h"

extern char Key_Sate[512];

#include "libretro-core.h"
#include "libretro.h"
#include "retroscreen.h"

extern int UI_is_active;

static int swap_joysticks = FALSE;
int PLATFORM_kbd_joy_0_enabled = TRUE;	/* enabled by default, doesn't hurt */
int PLATFORM_kbd_joy_1_enabled = FALSE;	/* disabled, would steal normal keys */
extern unsigned char MXjoy[2]; // joy
extern int retro_sound_finalized;

static UWORD *palette = NULL;

int skel_main(int argc, char **argv)
{

	/* initialise Atari800 core */
	if (!Atari800_Initialise(&argc, argv)){
		printf("Failed to initialise!\n");
		return 3;
	}
	
	retro_sound_finalized=1;

	printf("First retrun to main thread!\n");
    co_switch(mainThread);

	/* main loop */
	for (;;) {
		INPUT_key_code = PLATFORM_Keyboard();
		//SDL_INPUT_Mouse();
		Atari800_Frame();
		if (Atari800_display_screen)
			PLATFORM_DisplayScreen();
		
	}
}


int PLATFORM_Initialise(int *argc, char *argv[])
{

	Log_print("Core init");

	retro_InitGraphics();

	Devices_enable_h_patch = FALSE;
	INPUT_direct_mouse = TRUE;

	return TRUE;
}

int PLATFORM_Exit(int run_monitor)
{
	if (CPU_cim_encountered) {
		Log_print("CIM encountered");
		return TRUE;
	}

	Log_print("Core_exit");

	retro_ExitGraphics();

	return FALSE;
}

int PLATFORM_Keyboard(void)
{	

	/* OPTION / SELECT / START keys */
	INPUT_key_consol = INPUT_CONSOL_NONE;
	if (Key_Sate[RETROK_F2])
		INPUT_key_consol &= (~INPUT_CONSOL_OPTION);
	if (Key_Sate[RETROK_F3])
		INPUT_key_consol &= (~INPUT_CONSOL_SELECT);
	if (Key_Sate[RETROK_F4])
		INPUT_key_consol &= (~INPUT_CONSOL_START);

	if (Key_Sate[RETROK_SPACE])
		return AKEY_SPACE;

	if (Key_Sate[RETROK_F1])
		return AKEY_UI;

if (Key_Sate[RETROK_LEFT])return AKEY_LEFT;
if (Key_Sate[RETROK_RIGHT])return AKEY_RIGHT;
if (Key_Sate[RETROK_UP])return AKEY_UP;
if (Key_Sate[RETROK_DOWN])return AKEY_DOWN;
if (Key_Sate[RETROK_RETURN])return AKEY_RETURN;
if (Key_Sate[RETROK_ESCAPE])return AKEY_ESCAPE;
/*
	if (UI_is_active){
printf("ui....\n");
		if (MXjoy[0]&0x04)
			return AKEY_LEFT;
		if (MXjoy[0]&0x08)
			return AKEY_RIGHT;
		if (MXjoy[0]&0x01)
			return AKEY_UP;
		if (MXjoy[0]&0x02)
			return AKEY_DOWN;
		if (MXjoy[0]&0x80)
			return AKEY_RETURN;
		if (MXjoy[0]&0x40)
			return AKEY_ESCAPE;
	}
*/

	return AKEY_NONE;

}

/*
int PLATFORM_GetRawKey(void)
{

	input_poll_cb();

	for(i=0;i<320;i++)
        	Key_Sate[i]=input_state_cb(0, RETRO_DEVICE_KEYBOARD, 0,i) ? 0x80: 0;

}
*/

void PLATFORM_DisplayScreen(void)
{
	retro_Render();
}

double PLATFORM_Time(void)
{
	return GetTicks()/1000 ;//* 1e-3;
}

void PLATFORM_PaletteUpdate(void)
{
	retro_PaletteUpdate();
}

static void get_platform_PORT(unsigned char *s0, unsigned char *s1)
{
	int stick0, stick1;
	stick0 = stick1 = INPUT_STICK_CENTRE;

	if (PLATFORM_kbd_joy_0_enabled) {
		if (MXjoy[0]&0x04)
			stick0 &= INPUT_STICK_LEFT;
		if (MXjoy[0]&0x08)
			stick0 &= INPUT_STICK_RIGHT;
		if (MXjoy[0]&0x01)
			stick0 &= INPUT_STICK_FORWARD;
		if (MXjoy[0]&0x02)
			stick0 &= INPUT_STICK_BACK;
	}
	if (PLATFORM_kbd_joy_1_enabled) {
		if (MXjoy[1]&0x04)
			stick1 &= INPUT_STICK_LEFT;
		if (MXjoy[1]&0x08)
			stick1 &= INPUT_STICK_RIGHT;
		if (MXjoy[1]&0x01)
			stick1 &= INPUT_STICK_FORWARD;
		if (MXjoy[1]&0x02)
			stick1 &= INPUT_STICK_BACK;
	}

	if (swap_joysticks) {
		*s1 = stick0;
		*s0 = stick1;
	}
	else {
		*s0 = stick0;
		*s1 = stick1;
	}

 }

static void get_platform_TRIG(unsigned char *t0, unsigned char *t1)
{
	int trig0, trig1;
	trig0 = trig1 = 1;

	if (PLATFORM_kbd_joy_0_enabled) {
		//trig0 = !MXjoy[0]&0x80;
		trig0 = MXjoy[0]&0x80?0:1;
	}

	if (PLATFORM_kbd_joy_1_enabled) {
		//trig1 = !MXjoy[1]&0x80;
		trig1 = MXjoy[1]&0x80?0:1;
	}

	if (swap_joysticks) {
		*t1 = trig0;
		*t0 = trig1;
	}
	else {
		*t0 = trig0;
		*t1 = trig1;
	}

}

int PLATFORM_PORT(int num)
{
	if (num == 0) {
		UBYTE a, b;
		//update_SDL_joysticks();
//printf("ffff %d\n",MXjoy[0]) ;
		get_platform_PORT(&a, &b);
		return (b << 4) | (a & 0x0f);
	}

	return 0xff;//(Android_PortStatus >> (num << 3)) & 0xFF;
}

int PLATFORM_TRIG(int num)
{
	UBYTE a, b;
	get_platform_TRIG(&a, &b);
//printf("ffff 0x%x: %d %d\n",MXjoy[0],a,b) ;
	switch (num) {
	case 0:
		return a;
	case 1:
		return b;
	default:
		break;
	}

	return 0x01;//(Android_TrigStatus >> num) & 0x1;
}


/////////////////////////////////////////////////////////////
//   SOUND
/////////////////////////////////////////////////////////////


int PLATFORM_SoundSetup(Sound_setup_t *setup)
{
	//force 16 bit stereo sound at 44100
	setup->freq=44100;
	setup->sample_size=2;
	setup->channels=2;
	setup->buffer_ms=20;

	return TRUE;
}

void PLATFORM_SoundExit(void)
{

}

void PLATFORM_SoundPause(void)
{

}

void PLATFORM_SoundContinue(void)
{

}

void PLATFORM_SoundLock(void)
{

}

void PLATFORM_SoundUnlock(void)
{

}

/////////////////////////////////////////////////////////////
//   VIDEO
/////////////////////////////////////////////////////////////


void retro_PaletteUpdate(void)
{
	int i;

	if (!palette) {
		if ( !(palette = malloc(256 * sizeof(UWORD))) ) {
			Log_print("Cannot allocate memory for palette conversion.");
			return;
		}
	}
	memset(palette, 0, 256 * sizeof(UWORD));

	for (i = 0; i < 256; i++){

		palette[i] = ((Colours_table[i] & 0x00f80000) >> 8) |
					 ((Colours_table[i] & 0x0000fc00) >> 5) | 
	 	   			 ((Colours_table[i] & 0x000000f8) >> 3);

	}

	/* force full redraw */
	Screen_EntireDirty();
}

int retro_InitGraphics(void)
{

	/* Initialize palette */
	retro_PaletteUpdate();

	return TRUE;
}

void retro_Render(void)
{
	int x, y;
	UBYTE *src, *src_line;
	UWORD *dst, *dst_line;

	src_line = ((UBYTE *) Screen_atari) + 24;
	dst_line = Retro_Screen;

	for (y = 0; y < 240; y++) {

		src = src_line;
		dst = dst_line;

		for (x = 0; x < 336; x += 8) {

			*dst++ = palette[*src++]; *dst++ = palette[*src++];
					*dst++ = palette[*src++]; *dst++ = palette[*src++];
					*dst++ = palette[*src++]; *dst++ = palette[*src++];
					*dst++ = palette[*src++]; *dst++ = palette[*src++];
		}

		src_line += 384;
		dst_line += 336;
	}
}


void retro_ExitGraphics(void)
{

	if (palette)
		free(palette);
	palette = NULL;
}

