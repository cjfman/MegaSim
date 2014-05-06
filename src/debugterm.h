// debugterm.h

#ifndef DEBUGTERM_h
#define DEBUGTERM_h

#ifdef __cplusplus
extern "C" {
#endif

extern int debug_mode;

int runDebugTerm(void);
int step(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DEBUGTERM_h
