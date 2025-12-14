#  @file $RCSfile: tsf4g.mk,v $
#  general description of this module
#  $Id: tsf4g.mk,v 1.7 2008/01/09 03:58:28 jackyai Exp $
#  @author $Author: jackyai $
#  @date $Date: 2008/01/09 03:58:28 $
#  @version 1.0
#  @note Editor: Vim 6.1, Gcc 2.95.3, tab=4
#  @note Platform: Linux

TSF4G_HOME = /root/tsf4g
TSF4G_INC  = $(TSF4G_HOME)/include
TSF4G_SRC  = $(TSF4G_HOME)/src
TSF4G_LIB  = $(TSF4G_HOME)/lib
TSF4G_TOOL = $(TSF4G_HOME)/tools
TSF4G_DEPS = $(TSF4G_HOME)/deps
TSF4G_PACKLIBDIR=/root/tsf4g/lib_src/tmp
TSF4G_MYSQL_PREFIX = /usr/local
TSF4G_LIBSRC  = $(TSF4G_HOME)/lib_src
# 在 /root/tsf4g/tsf4g.mk 中添加以下内容
MYSQL_INSTALL_DIR = /usr/local/mysql-5.0.67-linux-i686-glibc23
# 定义 MySQL 头文件路径
TSF4G_DBMS_INCLUDE = -I$(MYSQL_INSTALL_DIR)/include
# 定义 MySQL 库路径和库名（关键）
TSF4G_DBMS_LIB = -L$(MYSQL_INSTALL_DIR)/lib -lmysqlclient
OS = linux
CC = gcc
RM = /bin/rm -f
SHARED = -shared -z defs
CDBG  = -g 
CINC  = -I./ -I$(TSF4G_INC)/
# ====== 新增 scew/XML 相关变量 ======
# scew 源码目录
SCEW_DIR = /root/tsf4g/deps/scew
# scew 头文件路径（供编译时 include）
SCEW_INCLUDE = -I$(SCEW_DIR)/scew
# scew 库文件路径和库名（供链接时使用）
SCEW_LIB = -L$(SCEW_DIR)/scew -L/root/tsf4g/lib -lscew
# 赋值给 TSF4G_XML_LIBS（供 tdr 模块使用）
TSF4G_XML_LIBS = $(SCEW_INCLUDE) $(SCEW_LIB)

# ====== 新增 expat 相关变量（scew 依赖）======
# expat 源码目录
EXPAT_DIR = /root/tsf4g/deps/expat
# expat 头文件路径（供编译时 include）
EXPAT_INCLUDE = -I$(EXPAT_DIR)/lib
# expat 库文件路径和库名（供链接时使用）
EXPAT_LIB = -L$(EXPAT_DIR)/.libs -lexpat
# 将 expat 配置合并到 XML 相关变量中（供依赖模块使用）
TSF4G_XML_LIBS += $(EXPAT_INCLUDE) $(EXPAT_LIB)


CFLAGS  = -Wall -Wextra -pipe -D_NEW_LIC -D_GNU_SOURCE -D_REENTRANT -fPIC $(CINC) $(CDBG)

LDPATH  = -L$(TSF4G_LIB)/ -L./ -L/usr/lib/ -L/usr/local/lib

LIBS  = -lnsl -lpthread -ldl
