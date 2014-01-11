parse_dms_example: parse_dms.o
	cc -Wall -g parse_dms_example.c parse_dms.o -o parse_dms_example
parse_dms.o:
	cc -Wall -g -c parse_dms.c
