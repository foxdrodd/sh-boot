/*
 * BUFMAX defines the maximum number of characters in inbound/outbound
 * buffers at least NUMREGBYTES*2 are needed for register packets
 */
#define BUFMAX 1546		/* As we use the buffer for Ethernet too... */
#define OUTBUFMAX (NUMREGBYTES*2+512)

/*
 * Number of bytes for registers
 */
#define NUMREGBYTES 92

enum regnames
{
  R0,  R1, R2,  R3,  R4,   R5,   R6,  R7, 
  R8,  R9, R10, R11, R12,  R13,  R14, R15, 
  PC,  PR, GBR, VBR, MACH, MACL, SR,
};

/*
 * sh-stub.c
 */
extern char ingdbmode;
extern void breakpoint (void);
extern char in_nmi;   /* Set when handling an NMI, so we don't reenter */
extern int dofault;  /* Non zero, bus errors will raise exception */
extern char stepped;
extern unsigned int registers[NUMREGBYTES / 4];
extern char remcomInBuffer[BUFMAX];
extern char remcomOutBuffer[OUTBUFMAX];
extern char *mem2hex (const char *, char *, int);

/*
 * sh-sci.c
 */
extern void putDebugChar (char);
extern void putString (const char *);
extern int getDebugCharTimeout (int);
extern char getDebugChar (void);
extern void putpacket (register char *);
extern char highhex(int);
extern char lowhex(int);
extern int cache_control (unsigned int);

#if defined(CONFIG_IDE)
/*
 * ide.c
 */
extern int ide_detect_devices (void);
extern void ide_startup_devices (void);
extern int ide_read_sectors (int, unsigned long, unsigned char *, int);
#endif

/*
 * main.c
 */
extern void handle_bios_call (void);
extern void printouthex (int);
extern void printouthex32 (unsigned int);
void sleep128 (unsigned int);

#if defined(CONFIG_ETHERNET)
/*
 * eth.c
 */
extern int eth_reset (unsigned int);
extern int eth_receive (char *, unsigned int *);
extern int eth_transmit (const char *, unsigned int, unsigned int, const char *);
extern int eth_node_addr (unsigned int, char *);
#endif

#if defined(CONFIG_CPU_SUBTYPE_SH7751) && defined(CONFIG_PCI)
/*
 * pci-sh7751.c and/or pcnet32.c
 */
extern int init_pcic (void);
extern unsigned long pci_read_config_dword (unsigned long offset);
extern void pci_write_config_dword (unsigned long offset, unsigned long data);
extern unsigned long pci_nextio, pci_nextmem;
#endif
