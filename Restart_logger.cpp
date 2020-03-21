#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include "Restart_logger.h"

void Restart_Logger(long *long_restartcounter, char *str_lasttimestamp)
{
  //function opens log file
  //searches for restart RESTART_COUNTER
  //increments it, writes the new counter in a new file LOGRESTART_FILE_BACK
  //adds all further lines in (timestamps)
  //adds actual timestamp
  //close and deletes LOGRESTART_FILE
  //renames LOGRESTART_FILE_BACK into LOGRESTART_FILE

  // get conf parameters
  FILE *fp_logrestart, *fp_logrestart_bak;
  //long long_restartcounter=0;
  char var[128], value[128], line[256];
  //fp = fopen(CONF_PATH LOGRESTART_FILE, "r");
  //if(!fp)
  fp_logrestart = fopen(LOG_RESTART_FILE, "r");
  if(!fp_logrestart)
  { //file does not exists, create one
    printf("File (restart log file) not found, new one will be created!\n");
    fp_logrestart = fopen(LOG_RESTART_FILE, "w+");
  }
  else
  { //file exists, read restart-counter from first line
    printf("Existing restart logfile found!\n");
    while (fgets(line, sizeof(line), fp_logrestart)) {
        memset(var, 0, sizeof(var));
        memset(value, 0, sizeof(value));
        if(sscanf(line, "%[^ \t=]%*[\t ]=%*[\t ]%[^\n]", var, value) == 2) {
            if(strcmp(var, REST_CNT) == 0)
                *long_restartcounter=atol(value);
                break;
              }
      }
  }
  //increment restart counter because of restart_log
  (*long_restartcounter)++;

  //printf("Restartcounter = %ld\n",*long_restartcounter);

  fp_logrestart_bak = fopen(LOG_RESTART_FILE_BAK, "w");
  //add new counter to backupfile
  sprintf(line,"%s = %ld\n",REST_CNT,*long_restartcounter);
  fputs(line,fp_logrestart_bak);
  // add remaining lines to backupfile
  while (fgets(line, sizeof(line), fp_logrestart)) {
    fputs(line,fp_logrestart_bak);
  }
  //add new timestamp from current restart_log



  timeval curTime;
  gettimeofday(&curTime, NULL);

  strftime(str_lasttimestamp, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

  sprintf(line,"%s = %s\n",REST_TSTMP,str_lasttimestamp);

  //printf("%s",line);

  fputs(line,fp_logrestart_bak);

  //close FILE
  fclose(fp_logrestart);
  fclose(fp_logrestart_bak);
  //delete FILE
  remove(LOG_RESTART_FILE);
  //rename LOG_RESTART_FILE_BAK -> LOG_RESTART_FILE

  rename(LOG_RESTART_FILE_BAK,LOG_RESTART_FILE);

  //return 0;

}
