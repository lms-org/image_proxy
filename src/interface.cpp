#include "image_proxy.h"

extern "C" {
void* getInstance() {
    return new ImageProxy();
}
}
