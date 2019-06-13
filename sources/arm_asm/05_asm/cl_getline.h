#ifndef CL_GETLINE_H
#define CL_GETLINE_H


void cl_getline_set_str(const char *s);
int cl_getline_set_file(const char *filename);
int cl_getline(char **out_buf);


#endif
