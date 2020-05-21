#include "broker/broker.h"

#include <string.h>

int main(int argc, char** argv) {
    if(argc == 2 && strcmp("--version", argv[1]) == 0) {
        printf("CHINBOX C-EBS version: %s\n", IOT_DSA_C_SDK_VERSION);
        return 0;
    }
    return broker_start();
}
