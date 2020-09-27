#ifndef __FB_DISPLAY_H__
#define __FB_DISPLAY_H__

extern uint8_t display_support;

extern bool init_display_control();

extern void close_display_control();

extern void update_display_control(struct FBIO::FrameBuffer *pFB, struct status *status, struct tm* timeptr);
extern void update_display_control_status_message(struct FBIO::FrameBuffer *pFB, const char *string_buffer_1, const char *string_buffer_2);

struct status
{
	char ladeende[10];
	char regelbeginn[10];
	char regelende[10];
	int32_t production_w;
	int32_t chargepower_w;
	float PV_prod_kw;
	float net__feed_kw;
	float SOC_percent;
	float safed_today_kwh;
	int32_t exp_max_power_today;
	float exp_rem_energy_today;
	int prognose_kriterium;
	int ladeende_proz;
};

typedef struct status status;

extern status E3DC_status;

extern FBIO::FrameBuffer* FB;

#endif //__FB_DISPLAY_H__
