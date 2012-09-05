#ifndef PATTR_PARSER_H_
#define PATTR_PARSER_H_

struct cfg_value {
	int nelem;
	float v[4];
};

struct cfg_node {
	char *name, *valstr;

	long tm;
	struct cfg_value val, range;

	struct cfg_node *next;
};

struct cfg_node *psys_parse(const char *buf);


#endif	/* PATTR_PARSER_H_ */
