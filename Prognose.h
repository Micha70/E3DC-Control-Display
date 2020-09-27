#ifndef __PROGNOSE_H__
#define __PROGNOSE_H__

struct def_remaining_delivery{
  long prognosis_remaining_max_power_today;
  long prognosis_remaining_energy_today;
  long prognosis_expected_energy_today_morning;
  long prognosis_expected_energy_today_afternoon;
  long prognosis_expected_max_power_tomorrow;
  long prognosis_expected_energy_tomorrow;};

typedef def_remaining_delivery remaining_delivery;




extern void Prognose_init();

// Rückgabewert 0: Prognosewerte gültig
//              1: keine Prognose
extern int Prognose(remaining_delivery* remaining);


#endif //__PROGNOSE_H__
