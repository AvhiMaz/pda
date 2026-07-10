#include "pda.h"

#include <stdio.h>

int main(void) {
    const uint8_t point[32] = {82,  173, 156, 51,  5,   84,  128, 195, 130, 24,  133,
                                179, 79,  137, 107, 102, 53,  209, 163, 160, 215, 225,
                                34,  59,  160, 52,  246, 139, 184, 235, 130, 212};

    printf("on_curve = %d\n", pda_is_on_curve(point));
    return 0;
}
