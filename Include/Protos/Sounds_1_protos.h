
#ifndef SOUNDS_PROTOS_H
#define SOUNDS_PROTOS_H

/*==================== Dúwiëk ====================*/

typedef struct Audio AUD;

AUD *openAudio();
void closeAudio(AUD *);

void playSound(AUD *, SFX *);

#endif /* SOUNDS_PROTOS_H */
