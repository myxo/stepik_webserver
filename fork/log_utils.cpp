
#include <string>
#include <stdio.h>

char log_filename[] = "/tmp/log";

void log(std::string message){
    FILE *f = fopen(log_filename, "a");
    fprintf(f, "%s\n", message.c_str());
    fclose(f);
}