include ../../../tsf4g.mk

# 新增：引入 scew 的头文件路径
TDR_CFLAGS = $(CFLAGS) -I$(TSF4G_LIBSRC) $(SCEW_INCLUDE)

TDR_CFILE=$(wildcard *.c)
TDR_OBJ=$(TDR_CFILE:.c=.o)
TDR_XML_WRAPPER = libtdrxmlwrapper.a

.PHONY: all clean

all: $(TDR_XML_WRAPPER)

-include $(TDR_CFILE:.c=.d)

%.d: %.c
	$(CC) -MM $(TDR_CFLAGS)  $< -o $@

%.o: %.c %.d
	$(CC) -c $(TDR_CFLAGS)  $<

$(TDR_XML_WRAPPER):	$(TDR_OBJ)
	ar rsv $(TDR_XML_WRAPPER) $^ 

clean:
	$(RM) *.o  *.d
	$(RM) $(TDR_XML_WRAPPER)