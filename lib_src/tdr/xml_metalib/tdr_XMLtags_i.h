/**
*
* @file     tdr_XMLTags_i_h.h  
* @brief    获取不同版本的元数据描述XML标签定义
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-05 
*
*
* Copyright (c)  2007, 腾讯科技有限公司互动娱乐研发部
* All rights reserved.
*
*/

#ifndef TDR_XMLTAGS_I_H
#define TDR_XMLTAGS_I_H

#include "tdr/tdr_XMLtags.h"
#include "tdr/tdr_define.h"

/**
*根据DR XML定义版本获取 根元素的标签定义
*/
#define TDR_GET_METALIB_ROOT_TAG(rootTag, ver)  \
    { \
        if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
            rootTag = TDR_TAG_TYPELIB; \
        } else\
        {\
            rootTag = TDR_TAG_METALIB;\
        }\
    }

/**
*根据DR XML定义版本获取 根元素的标签定义
*/
#define TDR_GET_MACRO_VALUE_TAG(Tag, ver)  \
    { \
        if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
            Tag = TDR_TAG_MACRO_ID; \
        } else\
        {\
            Tag = TDR_TAG_MACRO_VALUE;\
        }\
    }

/**
*根据DR XML定义版本获取 entry元素sizeinfo的标签定义
*/
#define TDR_GET_SIZEINFO_TAG(Tag, ver)  \
    { \
        if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
            Tag = TDR_TAG_TARGET; \
        } else\
        {\
            Tag = TDR_TAG_SIZEINFO;\
        }\
    }

#define TDR_GET_ENTRY_TAG_NAME(Tag, ver) \
    { \
    if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
        Tag = TDR_TAG_ITEM; \
        } else\
        {\
        Tag = TDR_TAG_ENTRY;\
        }\
    }

#endif /*TDR_XMLTAGS_I_H*/
