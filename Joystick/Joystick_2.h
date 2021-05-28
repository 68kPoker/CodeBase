
#ifndef JOY_H
#define JOY_H

#define TIMEOUT 1

typedef struct IORequest *IO;
typedef struct IOStdReq  *IOStd;

IOStd openJoy(struct InputEvent *ie);
void closeJoy(IOStd io);

void readEvent(IOStd io, struct InputEvent *ie);
void clearIO(IOStd io);

#endif /* JOY_H */
