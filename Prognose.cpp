//Prognose.cpp
// MiWa Version 2020.05.01

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <cstring>
#include "Prognose.h"
#include "E3DC_CONF.h"

using namespace std;

#define MAX_LINES_RET 25

extern e3dc_config_t e3dc_config;

struct powervalue{long timestamp;int power;};

//typedef powervalue powervalue;

struct powervalue WattValuesToday[MAX_LINES_RET];
struct powervalue WattValuesTomorrow[MAX_LINES_RET];
struct powervalue WattHourValuesToday[MAX_LINES_RET];
struct powervalue WattHourValuesTomorrow[MAX_LINES_RET];

long last_prognose_call;


void Prognose_init()
{
	last_prognose_call=0;
}

// Rückgabewert 0: Prognosewerte gültig
//              1: keine Prognose
int Prognose(remaining_delivery* remaining)
{

	time_t now = time(0);
	//sprintf("Number of sec since January 1,1970: %lu\n",now);
	tm *currenttime = localtime(&now);

	long current_sec=currenttime->tm_sec+currenttime->tm_min*60+currenttime->tm_hour*60*60;

	if((current_sec-last_prognose_call) < 15*60)
		return 0; //weniger als 15min seit letzem Aufruf


//get date of current day
	char current_date[20];

  sprintf(current_date,"%04d-%02d-%02d",1900 + currenttime->tm_year,1 + currenttime->tm_mon,currenttime->tm_mday);
	//printf("today %s\n",current_date);


	last_prognose_call=current_sec;



	enum{MAXSTRLEN=50000};
	char *buffer =(char *) calloc(MAXSTRLEN+1,1);
	//////////////////////// ermittele Leistungswerte ////////////////////////////////////////////////////////////////////

	string str_anfrage;

	str_anfrage = "curl -H \"Accept: text/csv\" https://api.forecast.solar/estimate/watts/";
	str_anfrage += to_string(e3dc_config.latitude);
	str_anfrage += "/" + to_string(e3dc_config.longitude);
	str_anfrage += "/" + to_string(e3dc_config.dach_neigung);
	str_anfrage += "/" + to_string(e3dc_config.dach_richtung);
	str_anfrage += "/" + to_string(e3dc_config.anlagen_leistung);
	//printf("8%s\n",str_anfrage.c_str());


	//printf("Test\n");

	FILE *f;

	//f = popen("curl -H \"Accept: text/csv\" https://api.forecast.solar/estimate/watts/49.137136/12.123933/40/0/5.7","r");
	f = popen(str_anfrage.c_str(),"r");
	if(!f)
		return 1;


	size_t n=fread(buffer,1,MAXSTRLEN,f);
	fclose(f);
	string str_content(buffer);


	if(!ferror(f) and n>50) //Start only analysis, wenn mehr als 50 bytes gelesen, sonst Fehler, letzte Werte nehmen, wenn vorhanden
	{
		printf("%zu Zeichen gelesen: %s",n,str_content.c_str());
	}
	else{return 1;}


//printf("Test 4\n");

free(buffer);
memset(WattValuesToday,0x0, sizeof(WattValuesToday));
memset(WattValuesTomorrow,0x0, sizeof(WattValuesTomorrow));
//////////// für Test //////////////////
//	strcpy(current_date,"2020-04-26");
//	string str_content("2020-04-26 00:00:00;0\n2020-04-26 00:00:01;23\n2020-04-26 06:45:00;160\n2020-04-26 07:00:00;331\n2020-04-26 08:00:00;1260\n2020-04-26 09:00:00;2656\n2020-04-26 10:00:00;4167\n2020-04-26 11:00:00;5461\n2020-04-26 12:00:00;6321\n2020-04-26 13:00:00;6646\n2020-04-26 14:00:00;6338\n2020-04-26 15:00:00;5421\n2020-04-26 16:00:00;4104\n2020-04-26 17:00:00;2616\n2020-04-26 18:00:00;1248\n2020-04-26 19:00:00;353\n2020-04-26 19:44:00;51\n2020-04-26 20:28:00;0\n2020-04-27 05:48:00;0\n2020-04-27 06:17:00;23\n2020-04-27 06:45:00;171\n2020-04-27 07:00:00;331\n2020-04-27 08:00:00;1260\n2020-04-27 09:00:00;2662\n2020-04-27 10:00:00;4178\n2020-04-27 11:00:00;5501\n2020-04-27 13:00:00;6703\n2020-04-27 12:00:00;6384\n2020-04-27 14:00:00;6384\n2020-04-27 15:00:00;5398\n2020-04-27 16:00:00;4030\n2020-04-27 17:00:00;2514\n2020-04-27 18:00:00;1174\n2020-04-27 19:00:00;336\n2020-04-27 19:45:00;46\n2020-04-27 20:29:00;0\n");
//fseek ( f , 0 , SEEK_SET );
//	size_t n=str_content.length();

//	printf("%zu Zeichen gelesen: %s",n,str_content.c_str());
//////////// für Test //////////////////



//Auswertung maximale Leistung für verbleibenden Tag und für morgen

char date[50];
char time[50];
int value;
string line;

int len_str=str_content.length();
int pos=0;
int substr_end=0;
string str_entry="";



//printf("pos=%d, substr_end=%d, len_str=%d; str_entry=%s\n", pos, substr_end, len_str, str_entry.c_str());

int index_today=0; int index_tomorrow=0;
//string analysieren und im entsprechenden feld ablegen: WattValuesToday, WattValuesTomorrow
 while(pos<len_str ){
	 		str_entry.empty();
			substr_end=str_content.find("\n",pos);
			str_entry=str_content.substr(pos,substr_end-pos);

			//printf("pos=%d, substr_end=%d, str_entry=%s\n", pos, substr_end, str_entry.c_str());
			//var.clear();
			value=0;
      memset(date, 0, sizeof(date));
			memset(time, 0, sizeof(time));


			if(sscanf(str_entry.c_str(), "%[^ \t] %[^\t; ];%d[^\n]", date,time, &value) == 3) {
				int hour;int min;int sec;
				sscanf(time, "%02d:%02d:%02d[^\n]",&hour,&min,&sec);
				//printf("hour %d, min %d, sec %d\n", hour, min, sec);
				//prüfen ob Zeile mit aktuellem Datum beginnt  i.e 2020-04-24
				if(strncmp(current_date,date,10)==0)
				{
	//printf("timestamp %ld\n",hour*60*60+min*60+sec);
					WattValuesToday[index_today].timestamp=hour*60*60+min*60+sec;
					WattValuesToday[index_today].power=value;
					index_today++;
				}
				else{
	//				printf("timestamp %ld\n",hour*60*60+min*60+sec);
						WattValuesTomorrow[index_tomorrow].timestamp=hour*60*60+min*60+sec;
						WattValuesTomorrow[index_tomorrow].power=value;
						index_tomorrow++;

				}

			}
		//	printf("Test4\n");

		 	pos=substr_end+1;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//remittele Rückgabewerte
	remaining->prognosis_remaining_max_power_today=0;
	//remaining->prognosis_remaining_energy_today=0;
	remaining->prognosis_expected_max_power_tomorrow=0;
	//remaining->prognosis_expected_energy_tomorrow=0;
	int i;

	//printf("current_sec=%ld\n",current_sec);
	for(i=0; i<index_today; i++)
	{
		//Testausgabe
		//printf("Today %d Timestamp=%ld Power=%d\n",i,WattValuesToday[i].timestamp,WattValuesToday[i].power );
		//
		//erwartete max leistung des verbleibenden Tages
		if(WattValuesToday[i].timestamp>current_sec)
		{	//nur spätere Werte berücksichtigen, max wert übernehmen
				if(WattValuesToday[i].power>remaining->prognosis_remaining_max_power_today)
					remaining->prognosis_remaining_max_power_today=WattValuesToday[i].power;
		}

	}
	for(i=0; i<index_tomorrow; i++)
	{
		//Testausgabe
		//printf("Tomorrow %d Timestamp=%ld Power=%d\n",i,WattValuesTomorrow[i].timestamp,WattValuesTomorrow[i].power );
		//nur max wert des tages ermitteln
		if(WattValuesTomorrow[i].power>remaining->prognosis_expected_max_power_tomorrow)
		remaining->prognosis_expected_max_power_tomorrow=WattValuesTomorrow[i].power;

	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	//////////////////////// ermittele erwartete Ertragswerte ////////////////////////////////////////////////////////////////////

	str_anfrage = "curl -H \"Accept: text/csv\" https://api.forecast.solar/estimate/watthours/";
	str_anfrage += to_string(e3dc_config.latitude);
	str_anfrage += "/" + to_string(e3dc_config.longitude);
	str_anfrage += "/" + to_string(e3dc_config.dach_neigung);
	str_anfrage += "/" + to_string(e3dc_config.dach_richtung);
	str_anfrage += "/" + to_string(e3dc_config.anlagen_leistung);
	//printf("8%s\n",str_anfrage.c_str());


	//printf("Test\n");

	//f = popen("curl -H \"Accept: text/csv\" https://api.forecast.solar/estimate/watthours/49.137136/12.123933/40/0/5.7","r");
	f = popen(str_anfrage.c_str(),"r");
	if(!f)
		return 1;

	buffer =(char *) calloc(MAXSTRLEN+1,1);


	n=fread(buffer,1,MAXSTRLEN,f);
	fclose(f);
	str_content=buffer;

	if(!ferror(f) and n>50) //Start only analysis, wenn mehr als 50 bytes gelesen, sonst Fehler, letzte Werte nehmen, wenn vorhanden
	{	//printf("%zu Zeichen gelesen: %s",n,str_content.c_str());
	}
	else{return 1;}

	free(buffer);

	memset(WattHourValuesToday,0x0, sizeof(WattHourValuesToday));
	memset(WattHourValuesTomorrow,0x0, sizeof(WattHourValuesTomorrow));
	//////////// für Test //////////////////
	/*
		strcpy(current_date,"2020-04-26");
		string str_content("2020-04-26 00:00:00;0\n2020-04-26 00:00:01;23\n2020-04-26 06:45:00;160\n2020-04-26 07:00:00;331\n2020-04-26 08:00:00;1260\n2020-04-26 09:00:00;2656\n2020-04-26 10:00:00;4167\n2020-04-26 11:00:00;5461\n2020-04-26 12:00:00;6321\n2020-04-26 13:00:00;6646\n2020-04-26 14:00:00;6338\n2020-04-26 15:00:00;5421\n2020-04-26 16:00:00;4104\n2020-04-26 17:00:00;2616\n2020-04-26 18:00:00;1248\n2020-04-26 19:00:00;353\n2020-04-26 19:44:00;51\n2020-04-26 20:28:00;0\n2020-04-27 05:48:00;0\n2020-04-27 06:17:00;23\n2020-04-27 06:45:00;171\n2020-04-27 07:00:00;331\n2020-04-27 08:00:00;1260\n2020-04-27 09:00:00;2662\n2020-04-27 10:00:00;4178\n2020-04-27 11:00:00;5501\n2020-04-27 13:00:00;6703\n2020-04-27 12:00:00;6384\n2020-04-27 14:00:00;6384\n2020-04-27 15:00:00;5398\n2020-04-27 16:00:00;4030\n2020-04-27 17:00:00;2514\n2020-04-27 18:00:00;1174\n2020-04-27 19:00:00;336\n2020-04-27 19:45:00;46\n2020-04-27 20:29:00;0\n");
			//fseek ( f , 0 , SEEK_SET );
		size_t n=str_content.length();

		printf("%zu Zeichen gelesen: %s",n,str_content.c_str());
	*/
	//////////// für Test //////////////////



	  //Auswertung maximaler Ertrag für verbleibenden Tag ufn für morgen
		//prüfen ob Zeile mit aktuellem Datum beginnt  i.e 2020-04-24

	len_str=str_content.length();
	pos=0;
	substr_end=0;
	str_entry="";



	index_today=0; index_tomorrow=0;
	 while(pos<len_str ){
		 		str_entry.empty();
				substr_end=str_content.find("\n",pos);
				str_entry=str_content.substr(pos,substr_end-pos);

				value=0;
	      memset(date, 0, sizeof(date));
				memset(time, 0, sizeof(time));


				if(sscanf(str_entry.c_str(), "%[^ \t] %[^\t; ];%d[^\n]", date,time, &value) == 3) {
					int hour;int min;int sec;
					sscanf(time, "%02d:%02d:%02d[^\n]",&hour,&min,&sec);
					//printf("hour %d, min %d, sec %d\n", hour, min, sec);
					if(strncmp(current_date,date,10)==0)
					{
						//printf("timestamp %ld\n",hour*60*60+min*60+sec);
						WattHourValuesToday[index_today].timestamp=hour*60*60+min*60+sec;
						WattHourValuesToday[index_today].power=value;
						index_today++;
					}
					else{
							//printf("index %d, timestamp %ld, value %d\n",index_tomorrow, hour*60*60+min*60+sec, value);
							WattHourValuesTomorrow[index_tomorrow].timestamp=hour*60*60+min*60+sec;
							WattHourValuesTomorrow[index_tomorrow].power=value;
							index_tomorrow++;

					}

				}
			//	printf("Test4\n");

			 	pos=substr_end+1;
		}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//remittele Rückgabewerte
	remaining->prognosis_remaining_energy_today=0;
	remaining->prognosis_expected_energy_tomorrow=0;

	long previous_expected_value=0;
	//printf("current_sec=%ld\n",current_sec);
	for(i=0; i<index_today; i++)
	{
		//ermittele erfolgten Ertrag bis jetzt
		if(WattHourValuesToday[i].timestamp<current_sec) previous_expected_value = WattHourValuesToday[i].power;

		if(WattHourValuesToday[i].timestamp>current_sec)	break;
	}
	//max ertag - erfolgter ertag --> verbleibender Ertrag
	remaining->prognosis_remaining_energy_today=WattHourValuesToday[index_today-1].power-previous_expected_value;

	//letzter ermittelter Wert ist erwartete Energie für morgen
	remaining->prognosis_expected_energy_tomorrow=WattHourValuesTomorrow[index_tomorrow-1].power;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	return 0;


}
