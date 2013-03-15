#ifndef BASICLOG_H
#define BASICLOG_H


#define MAXLOGSTRSIZE 2048

#define BLF_DISABLED    (1<<0)
#define BLF_FLUSHALWAYS (1<<1)

extern void VisLogStart();
extern void VisLogLine(char *logstr);
extern char strbuf[MAXLOGSTRSIZE];

void LogEnd();
unsigned char LogStart(unsigned char flags, char *lfname);
void LogLine(char *str);


#endif  // BASICLOG_H
