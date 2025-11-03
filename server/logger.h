// logger.h
#ifndef LOGGER_H
#define LOGGER_H

void init_logger(const char *filename);
void write_log(const char *fmt, ...);
void close_logger();

#endif
