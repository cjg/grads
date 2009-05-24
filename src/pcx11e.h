/*

  Header for PC/X11e things.

*/

#ifdef XLIBEMU

extern char x11_initialized;
extern int  console(void);
extern int  got_grads_cmd;
extern char grads_cmd_on;
extern char grads_cmd[1024];

#endif