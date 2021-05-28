
#include <exec/types.h>

#define PAL_CLOCK 3546895L

struct IOAudio *allocChannels(void);
void freeChannels(struct IOAudio *ioa);
void playSample(struct IOAudio *ioa, struct soundSample *s, WORD chan);
