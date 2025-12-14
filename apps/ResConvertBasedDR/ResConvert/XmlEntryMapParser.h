#pragma once
#include "entrymapparser.h"

class CXmlEntryMapParser :
	public EntryMapParser
{
public:
	CXmlEntryMapParser(void);
	virtual ~CXmlEntryMapParser(void);

	/*分析ini配置文件
	*@param[in] pstMetalib 元数据库句柄
	*@param[in] pstMeta 保存资源文件的结构体名称
	*@param[in] szPath 配置文件所在目录
	*@param[in] stFileList 相关配置信息列表
	*/
	virtual int ParseEntryMap(IN LPTDRMETALIB &pstMetalib, IN LPTDRMETA pstMeta, IN const ENTRYMAPFILELIST &stFileList, IN const CString &szPath);

	/*根据Excel页表确定资源结构体中具体的映射成员列表
	*@param[out] stList 映射成员列表
	*@param[in] pstSheet Excel页表对象指针
	*@param[in] pstMeta 资源结构体句柄
	*/
	virtual int GenEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN LPTDRMETA pstMeta);

protected:
	/**生成成员的映射信息
	*@return 如果能成功映射到至少一个excel单元格，则返回true，否则返回false
	*/
	inline int GenSimpleEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet,IN LPTDRMETALIB pstLib, IN int iHMetaOff, IN LPTDRMETAENTRY pstEntry, IN CString szPreTile);

	inline int MatchSheetTitle(IN CExcelSheet *pstSheet, IN const CString &szTitle);

	
};
