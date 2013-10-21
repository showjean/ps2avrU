#ifndef COMMON_INC_H
#define COMMON_INC_H

#ifndef outp
   #define outp(data,addr) addr = (data)
#endif

#ifndef inp
  #define inp(addr) (addr)
#endif

#ifndef BV
	#define BV(bit)			(1<<(bit))
#endif
#ifndef cbi
	#define cbi(reg,bit)	reg &= ~(BV(bit))
#endif
#ifndef sbi
	#define sbi(reg,bit)	reg |= (BV(bit))
#endif
#ifndef cli
	#define cli()			__asm__ __volatile__ ("cli" ::)
#endif
#ifndef sei
	#define sei()			__asm__ __volatile__ ("sei" ::)
#endif




#endif
