
#ifndef SOUNDS_PROTOS_H
#define SOUNDS_PROTOS_H

/*==================== D�wi�k ====================*/

typedef struct Audio AUD;

AUD *openAudio();
void closeAudio(AUD *);

void playSound(AUD *, SFX *);

#endif /* SOUNDS_PROTOS_H */
