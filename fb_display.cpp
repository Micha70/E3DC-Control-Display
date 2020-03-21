#include <iostream>
#include <string>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>
#include "framebuffer.h"
#include "io_BCM2835.h"
#include "fb_display.h"

//#include <time.h>
//#include <unistd.h>
//#include <iostream>

#define BATT_WITH  100
#define BATT_HIGH  20
#define BATT_POS_Y 5


#define CPU_FREQUENCY_MHZ 19200000

FBIO::FrameBuffer* FB;

uint8_t display_support; //0 not supported, !=0 display supported

status E3DC_status;

void background_light_display(struct tm* ptr_curr_time)
{
	unsigned long sec_of_day;
  unsigned long sec_switch_time_brightness;
	unsigned long sec_switch_time_darkness;

		//calculate second of day
	time_t rawtime;
	struct tm *my_time;
	time( &rawtime );
	my_time = localtime( &rawtime );

	sec_of_day=ptr_curr_time->tm_sec+ptr_curr_time->tm_min*60+ptr_curr_time->tm_hour*60*60;

	//switch_time=TIME_FOR_BRIGHTNESS;
	std::string switch_time=TIME_FOR_BRIGHTNESS;
  sec_switch_time_brightness=atol(switch_time.substr(0,2).c_str())*3600+atol(switch_time.substr(3,2).c_str())*60;

	//switch_time=TIME_FOR_DARKNESS;
 	switch_time=TIME_FOR_DARKNESS;
	sec_switch_time_darkness=atol(switch_time.substr(0,2).c_str())*3600+atol(switch_time.substr(3,2).c_str())*60;

//std::cout << "sec_switch_time_brightness=" << sec_switch_time_brightness << std::endl;
//std::cout << "sec_switch_time_darkness=" << sec_switch_time_darkness << std::endl;
//std::cout << "sec_of_day=" << sec_of_day << std::endl;


	if(sec_of_day<sec_switch_time_brightness)
		set_display_brightness(DARKNESS);
	else if (sec_of_day>=sec_switch_time_darkness)
		set_display_brightness(DARKNESS);
	else
		set_display_brightness(BRIGHTNESS);

}

void update_display_control(struct FBIO::FrameBuffer *pFB, struct status *status, struct tm* timeptr)
{
	unsigned x;
	char temp_buffer[255];

  FBIO::Font MyFont(1);

	const int w = pFB->GetWidth();
	//const int h = pFB->GetHeight();
	int h = pFB->GetHeight();
	unsigned batt_pos_x = (w-BATT_WITH)/2;
	pFB->ClearScreen(0x20, 0x20, 0x20); //fill grey



	//SOC Ausgabe
	// print Akku
	// oben / unten
	uint8_t col_batt_r;
	uint8_t col_batt_g;
	uint8_t col_batt_b;
	if(E3DC_status.SOC_percent>60)
	{	//grün
		col_batt_r=0x00;
		col_batt_g=0xff;
		col_batt_b=0x00;
	}
	else if(E3DC_status.SOC_percent>20)
	{ //gelb
		col_batt_r=0xff;
		col_batt_g=0xff;
		col_batt_b=0x00;
	}
  else
	{  //change to rot
		col_batt_r=0xff;
		col_batt_g=0xff*((20-E3DC_status.SOC_percent)/0xff);
		col_batt_b=0x00;
	}

	//Rahmen Batterie
  pFB->DrawRectangle(batt_pos_x, BATT_POS_Y, batt_pos_x+BATT_WITH, BATT_POS_Y+BATT_HIGH, col_batt_r, col_batt_g, col_batt_b, false);
  //DoppelRahmen
	pFB->DrawRectangle(batt_pos_x-1, BATT_POS_Y-1, batt_pos_x+BATT_WITH+1, BATT_POS_Y+BATT_HIGH+1, col_batt_r, col_batt_g, col_batt_b, false);



	//pol
	pFB->DrawLineV(batt_pos_x+BATT_WITH+2, BATT_POS_Y+BATT_HIGH/2-3, BATT_POS_Y+BATT_HIGH/2+3, col_batt_r, col_batt_g, col_batt_b);
	pFB->DrawLineV(batt_pos_x+BATT_WITH+3, BATT_POS_Y+BATT_HIGH/2-3, BATT_POS_Y+BATT_HIGH/2+3, col_batt_r, col_batt_g, col_batt_b);
	pFB->DrawLineV(batt_pos_x+BATT_WITH+4, BATT_POS_Y+BATT_HIGH/2-3, BATT_POS_Y+BATT_HIGH/2+3, col_batt_r, col_batt_g, col_batt_b);
	pFB->DrawLineV(batt_pos_x+BATT_WITH+5, BATT_POS_Y+BATT_HIGH/2-3, BATT_POS_Y+BATT_HIGH/2+3, col_batt_r, col_batt_g, col_batt_b);


  //  Füllstand
	for (x = batt_pos_x+2; x < ((unsigned) E3DC_status.SOC_percent)+batt_pos_x; x++)
  {
		pFB->DrawLine(x, BATT_POS_Y+2, x, BATT_POS_Y+BATT_HIGH-2, 0x7f, 0x7f, 0x7F); //green
	}

	MyFont.SetPixelSize(1);

	MyFont.SetPenColour(0xff, 0xff, 0xff);

	//output SOC
	//5signs including '%'
	MyFont.Printf(FB,w/2-(5*4),BATT_POS_Y+BATT_HIGH/4+2,"%2.1f %%",E3DC_status.SOC_percent);

	MyFont.SetPenColour(200, 200, 200);

	//AusgabeProduction
	//snprintf(temp_buffer,sizeof(temp_buffer),"Ladelst.: %4.1f W",E3DC_status.production_w);
	//fb_put_string(fb_info, 5, h-34, temp_buffer, strlen(temp_buffer), 0xffffff, 1, 3);
	MyFont.Printf(FB,5,h-52,"Produktion:");

	if(E3DC_status.production_w>0)
		MyFont.SetPenColour(0, 255, 0);
	else
		MyFont.SetPenColour(200, 200, 200);
  //x=5+8*11=93
	MyFont.Printf(FB,93,h-52," %4d W",E3DC_status.production_w);

	MyFont.SetPenColour(200, 200, 200);

	//Ladeleistung           Ladelstg.:
	MyFont.Printf(FB,5,h-42,"Ladelstg.: ");


	if(E3DC_status.chargepower_w>0)
		MyFont.SetPenColour(0, 255, 0);
	else if (E3DC_status.chargepower_w<0)
		MyFont.SetPenColour(255, 0, 0);
	else
		MyFont.SetPenColour(200, 200, 200);

	//Ladeleistung x=5+8*11=93
	MyFont.Printf(FB,93,h-42," %4d W",E3DC_status.chargepower_w);

	MyFont.SetPenColour(200, 200, 200);
  //Ausgabe Ladebeginn
	//snprintf(temp_buffer,sizeof(temp_buffer),"Ladebeginn: %s",E3DC_status.start_of_charge);
	//fb_put_string(fb_info, 5, h-22, temp_buffer, strlen(temp_buffer), 0xffffff, 1, 3);
	MyFont.Printf(FB,5,h-32,"Ladebeginn:  %s",E3DC_status.start_of_charge);

	//Ausgabe Safed today
	//snprintf(temp_buffer,sizeof(temp_buffer),"Ladebeginn: %s",E3DC_status.start_of_charge);
	//fb_put_string(fb_info, 5, h-22, temp_buffer, strlen(temp_buffer), 0xffffff, 1, 3);
	MyFont.Printf(FB,5,h-22, "Safed td:  %2.1f kWh",E3DC_status.safed_today_kwh);

	//Ausgabe Datum / Zeit
	strftime(temp_buffer, sizeof(temp_buffer), "%Y-%m-%d %H:%M:%S", timeptr);
	//fb_put_string(fb_info, 5, h-10, status->curr_timestamp, strlen(status->curr_timestamp), 0xffffff, 1, 3);
	MyFont.Printf(FB,5,h-10,temp_buffer);

	//update display BRIGHTNESS
	background_light_display(timeptr);

}

void update_display_control_status_message(struct FBIO::FrameBuffer *pFB, const char *string_buffer_1, const char *string_buffer_2)
{
	FBIO::Font MyFont(1);

	const int w = pFB->GetWidth();
	//const int h = pFB->GetHeight();
	int h = pFB->GetHeight();

	pFB->ClearScreen(0x20, 0x20, 0x20); //fill grey
  MyFont.Printf(FB,w/2-(8*strlen(string_buffer_1)/2),h/2-18,string_buffer_1);
	MyFont.Printf(FB,w/2-(8*strlen(string_buffer_2)/2),h/2+18,string_buffer_2);

}


bool init_display_control()
{
		display_support=0;
    FB = FBIO::FrameBuffer::Open(true);

    if(FB)
    {

			//MyFont->SetPixelSize(1);
      //init control of brightness
		  init_brightness_control();
		//int opt;
		//int req_fb = 1;  //framebuffer 1
		//fb_open(req_fb, &fb_info);
			display_support=1;
			return true;
    }
		return false;
}

void close_display_control()
{
	 set_display_brightness(DARKNESS);
	 close_brightness_control();
	 delete FB;
}

/*
int main(int argc, char **argv){

if(!init_display_control()) return 1;

//std::cout << "fb-display start" << std::endl;

strncpy(E3DC_status.start_of_charge,"10:45",sizeof(E3DC_status.start_of_charge));
E3DC_status.production_w=1234;
E3DC_status.SOC_percent=100;
E3DC_status.PV_prod_kw=2534;
E3DC_status.net__feed_kw=1500;
E3DC_status.safed_today_kwh=0.9;


time_t rawtime;
struct tm * timeinfo;



//struct timeval curTime;




set_display_brightness(BRIGHTNESS);

int i=0;


do{



		time (&rawtime);
  	timeinfo = localtime (&rawtime);

    	//sprintf(line,"%s = %s\n",REST_TSTMP,str_lasttimestamp);


  		update_display_control(FB, &E3DC_status, timeinfo);
  		sleep(1);
	}
	while(i++<1000);

	set_display_brightness(DARKNESS);

	close_display_control();







return 0;

} */
