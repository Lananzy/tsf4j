/**
*
* @file     tdr_define_i.h 
* @brief    内部使用的宏定义
* 
* @author steve jackyai  
* @version 1.0
* @date 2007-04-02 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/


#ifndef TDR_DEFINE__I_H
#define TDR_DEFINE__I_H


/** @name 元数据标志值
* @{*/
#define TDR_FLAG_NETORDER	0x01
#define TDR_FLAG_COMPLEX		0x02

/** @}*/  //元数据标志值

/** @name meta标志值
* @{*/
#define TDR_META_FLAG_FIXSIZE	0x0001	/* a type that does not change size. */
#define TDR_META_FLAG_HAVE_ID        0x0002    /**< 设置了id属性*/
#define TDR_META_FLAG_RESOVLED	0x0004	/*是否已经解析的标志*/
#define TDR_META_FLAG_VARIABLE	0x0008  /*meta是否是可变的*/
#define TDR_META_FALG_STRICT_INPUT	0x0010 /*输入时输入文件的格式是否必须是严格的*/
#define TDR_META_FALG_HAVE_AUTOINVREMENT_ENTRY	0x0020 /*此结构体中包含autoincrement成员*/
#define TDR_META_FLAG_NEED_PREFIX_FOR_UNIQUENAME	0x0040	/*当成员展开存储时必须添加前缀来保证名字唯一性*/
/** @}*/  



#define	TDR_BUILD		0x0008			/*构建版本，metalib的数据结构每变更一次，此版本号需加一*/		


/*const of type */
#define  TDR_INVALID_INDEX		-1
#define  TDR_INVALID_PTR                -1
#define  TDR_INVALID_OFFSET             -1

#define TDR_INVALID_ID     -1           /**< 无效ID值*/

#define TDR_INVALID_VERSION -1          /**< 无效版本号*/


/**@name entry 标志
*@ {*/
#define TDR_ENTRY_FLAG_NONE		0x0000  /**< 无特殊标志*/
#define TDR_ENTRY_FLAG_RESOVLD          0x0001  /**<此entry已经正确解析*/
#define TDR_ENTRY_FLAG_POINT_TYPE		0x0002    /**<指针类型*/
#define TDR_ENTRY_FLAG_REFER_TYPE		0x0004    /**<引用类型*/
#define TDR_ENTRY_FLAG_HAVE_ID              0x0008        /**<entry定义了ID属性*/
#define TDR_ENTRY_FLAG_HAVE_MAXMIN_ID		0x0010		/**<entry定义了maxminid属性*/
#define TDR_ENTRY_FALG_FIXSIZE        0x0020        /**<entry的存储空间是固定的*/
#define TDR_ENTRY_FLAG_REFER_COUNT	0x0040	/*此成员是另外一个成员的数组计数器*/
/*@ }*/

/**@name entry对象关系映射标志
*@ {*/
#define TDR_ENTRY_DB_FLAG_NONE		0x00  /**< 无特殊标志*/
#define TDR_ENTRY_DB_FLAG_UNIQUE	0x01	/**<其取值是unique的*/
#define TDR_ENTRY_DB_FLAG_NOT_NULL	0x02	/**<其取值是不为空NOT NULL的*/
#define TDR_ENTRY_DB_FLAG_EXTEND_TO_TABLE 0x04			/**<entry在建表时进行扩展*/
#define TDR_ENTRY_DB_FLAG_PRIMARYKEY 0x10			/**<此entry是主键组成部分*/
#define TDR_ENTRY_DB_FLAG_AUTOINCREMENT	0x20		/**<此成员为autoincremnet成员*/
/*@ }*/

#define TDR_MAX_PATH       260      /**<路径名最大长度*/

#define TDR_MAX_HPP_STRING_WIDTH_LEN		32			/**<c语言串最大宽度*/

#define  TDR_MIN(a,b)  (((a) < (b)) ? (a) : (b))

#define TDR_MIN_COPY		64

#define TDR_MIN_INT_VALUE	0x80000000

#define TDR_TAB_SIZE	4		/**<tab键的空格数*/

/**@name 分表规则
*@ {*/
#define TDR_SPLITTABLE_RULE_NONE		0x00  /**< 没有指定分表规则*/
#define TDR_SPLITTABLE_RULE_BY_MOD          0x01  /**<按分表因子模的方式进行分表*/
/*@ }*/


#define TDR_MAX_UNIQUE_KEY_IN_TABLE   32  /**<数据库表中容许的unique键的最大数目*/


#define  TDR_POINTER_TYPE_ALIGN 4	/*指针数据类型对齐方式*/

#define TDR_MACROSGROUP_MAP_NUM	2	/*宏定义组数据映射区块的个数*/




#define  TDR_POINTER_TYPE_ALIGN 4	/*指针数据类型对齐方式*/

#define TDR_MAX_INT   (int)0x7FFFFFFF
#endif /* TDR_DEFINE__I_H */
