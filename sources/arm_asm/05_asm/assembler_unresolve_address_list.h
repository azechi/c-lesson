#ifndef ASSEMBLER_UNRESOLVE_ADDRESS_LIST_H
#define ASSEMBLER_UNRESOLVE_ADDRESS_LIST_H

typedef struct UnresolveAddress_ UnresolveAddress;

void unresolve_address_clear();
void unresolve_address_push(UnresolveAddress *ua);
int unresolve_address_pop(UnresolveAddress *out_ua);


#endif
