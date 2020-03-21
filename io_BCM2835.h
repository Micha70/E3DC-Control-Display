#define TIME_FOR_BRIGHTNESS "06:00"
#define TIME_FOR_DARKNESS "22:00"
#define BRIGHTNESS_LEVEL  80
#define DARKNESS_LEVEL  10

enum state{BRIGHTNESS=1, DARKNESS};

extern void set_display_brightness(unsigned int value);
extern int init_brightness_control(void);
extern void close_brightness_control(void);
