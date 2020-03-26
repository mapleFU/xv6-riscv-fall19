//
// Created by 付旭炜 on 2019/12/4.
//

#ifndef XV6_RISCV_FALL19_EXCEPTIONS_H
#define XV6_RISCV_FALL19_EXCEPTIONS_H

#include "user/user.h"

const char*
fmtname(const char *path)
{
  // declearing a static buffer for return buffer safety.
  static char buf[DIRSIZ+1];
  const char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  // move info to buffer
  memmove(buf, p, strlen(p));
  // and `DIRSIZ-strlen(p)` to BLANK.
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

#define UNIMPLEMENTED()  \
    fprintf(2, "method not implemented %s; %d", __FILE__, __LINE__); \
    exit();

#endif //XV6_RISCV_FALL19_EXCEPTIONS_H
