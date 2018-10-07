SUBDIRS = \
	01_bareminimum    \
	02_multicorec     \
	03_uart1          \
	04_mailboxes      \
	05_uart0          \
	06_random         \
	07_delays         \
	08_power          \
	09_framebuffer    \
	0A_pcscreenfont   \
	0B_readsector     \
	0C_directory      \
	0D_readfile       \
	0E_initrd         \
	0F_executionlevel \
	10_virtualmemory  \
	11_exceptions     \
	12_printf         \
	13_debugger       \
	14_raspbootin64

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

all: subdirs
