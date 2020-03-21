//
//  E3DC_CONF.h
//  RscpExcample
//
//  Created by Eberhard Mayer on 16.08.18.
//  Copyright © 2018 Eberhard Mayer. All rights reserved.
//

#ifndef RESTART_LOGGER_h
#define E3DC_CONF_h

void Restart_Logger(long *long_restartcounter, char *str_lasttimestamp);

#endif /* RESTART_LOGGER_h */

#define LOG_RESTART_FILE "restart_log.txt"
#define LOG_RESTART_FILE_BAK "restart_log.bak"


#define REST_CNT "RESTART_COUNTER"
#define REST_TSTMP "RESTART"
