/*
 *	Generate the syscall functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "syscall_name.h"

static char namebuf[128];

static void write_call(int n)
{
  FILE *fp;
  snprintf(namebuf, 128, "fuzix68000/syscall_%s.S", syscall_name[n]);
  fp = fopen(namebuf, "w");
  if (fp == NULL) {
    perror(namebuf);
    exit(1);
  }
  fprintf(fp, "\t.text\n\n"
	      "\t.globl %1$s\n\n"
	      "%1$s:\n", syscall_name[n]);
  fprintf(fp, ".mri 1\n");
  fprintf(fp, "\tmove.w #%d,d0\n"
	      "\ttrap #14\n", n);
  /* ext is the same speed as tst so we might as well do the ext in case
     it is an error code. We have to ext the error because while we use
     shorts in kernel for such things and speed the standard says errno
     is integer */
  fprintf(fp, "\text.l d1\n"
              "\tbne _error\n"
              "\trts\n"
              "_error:\n"
              "\tmove.l d1,errno\n"
              "\trts\n");
  fclose(fp);
}

static void write_call_table(void)
{
  int i;
  for (i = 0; i < NR_SYSCALL; i++)
    write_call(i);
}

static void write_makefile(void)
{
  int i;
  FILE *fp = fopen("fuzix68000/Makefile", "w");
  if (fp == NULL) {
    perror("Makefile");
    exit(1);
  }
  fprintf(fp, "# Autogenerated by tools/syscall_68000\n");
  fprintf(fp, "CROSS_AS=m68k-linux-gnu-gcc\nCROSS_LD=m68k-linux-gnu-ld\nCROSS_AR=m68k-linux-gnu-ar\n");
  fprintf(fp, "ASOPTS=\n\n");
  fprintf(fp, "ASRCS = syscall_%s.S\n", syscall_name[0]);
  for (i = 1; i < NR_SYSCALL; i++)
    fprintf(fp, "ASRCS += syscall_%s.S\n", syscall_name[i]);
  fprintf(fp, "\n\nASRCALL = $(ASRCS) $(ASYS)\n");
  fprintf(fp, "\nAOBJS = $(ASRCALL:.S=.o)\n\n");
  fprintf(fp, "syslib.lib: $(AOBJS)\n");
  fprintf(fp, "\techo $(AOBJS) >syslib.l\n");
  fprintf(fp, "\t$(CROSS_AR) rc syslib.lib $(AOBJS)\n\n");
  fprintf(fp, "$(AOBJS): %%.o: %%.S\n");
  fprintf(fp, "\t$(CROSS_AS) $(ASOPTS) -c $<\n\n");
  fprintf(fp, "clean:\n");
  fprintf(fp, "\trm -f $(AOBJS) $(ASRCS) syslib.lib syslib.l *~\n\n");
  fclose(fp);
}

int main(int argc, char *argv[])
{
  write_makefile();
  write_call_table();
  exit(0);
}
