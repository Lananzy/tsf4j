#pragma once

#include <vector>
#include <list>

#include "../comm/ConvDefine.h"
#include "../comm/Tree.h"
#include "../comm/ConvResTree.h"
#include "../excel/ExcelReader.h"
#include "tdr/tdr.h"

enum tagGetExcelValueMethod
{
	GET_EXCEL_VALUE_KEYWORDS = 0,	/*通过关键字替换*/
	GET_EXCEL_VALUE_NUMBER,	/*取数字*/
	GET_EXCEL_VALUE_ALLINFO,	/*取所有信息*/
	GET_EXCEL_VALUE_PERSENT,	/*取百分数*/
	GET_ECCEL_VALUE_INTELLIGENT,	/*表示当内容可能为数字或关键字时，智能获取需要的数据信息（不推荐使用）*/

};
typedef enum tagGetExcelValueMethod GETEXCELVALUEMETHOD;


GETEXCELVALUEMETHOD ParseEntryGetValueMethod(IN LPTDRMETAENTRY pstEntry);

/*数据成员与Excel单元映射关系
*/
class CEntryCellItem
{
public:
	CEntryCellItem();
	~CEntryCellItem();

	void SetEntryIdx(int iIdxEntry){_iIdxEntry = iIdxEntry;};
	int GetEntryIdx()const {return _iIdxEntry;} ;

	void SetCellIndex(int iIdx) {_iIdxCell = iIdx;};
	int GetCellIndex() const {return _iIdxCell;};

	void SetEntryHandler(LPTDRMETAENTRY pstEntry){_pstEntry = pstEntry;};
	LPTDRMETAENTRY GetEntryHandler() const {return _pstEntry;};

	void SetGetValueMethod(GETEXCELVALUEMETHOD enMethod){_enGetValueMethod = enMethod;};
	GETEXCELVALUEMETHOD GetGetValueMethod() const {return _enGetValueMethod;};

	void SetHOff(int iHOff) {_iHOff = iHOff;};
	int GetHOff() {return _iHOff;};
private:
	int _iIdxEntry;		/*数组下标*/
	int _iIdxCell;	/*对应Excel单元格的索引，如果没有Excel单元个对应，则其值为－1*/
	LPTDRMETAENTRY _pstEntry;	/*此成员描述的句柄*/
	GETEXCELVALUEMETHOD _enGetValueMethod;	/*取值方法*/
	int _iHOff;	/*相对基地址的偏移*/
};

typedef std::vector<CEntryCellItem>  RESENTRYLIST;



class EntryMapParser
{
public:
	EntryMapParser(void);
	~EntryMapParser(void);

	/*分析ini配置文件
	*@param[in] pstMetalib 元数据库句柄
	*@param[in] pstMeta 保存资源文件的结构体名称
	*@param[in] szPath 配置文件所在目录
	*@param[in] stFileList 相关配置信息列表
	*/
	virtual int ParseEntryMap(IN LPTDRMETALIB &pstMetalib, IN LPTDRMETA pstMeta, IN const ENTRYMAPFILELIST &stFileList, IN const CString &szPath) = 0;
	
	/*根据Excel页表确定资源结构体中具体的映射成员列表
	*@param[out] stList 映射成员列表
	*@param[in] pstSheet Excel页表对象指针
	*@param[in] pstMeta 资源结构体句柄
	*/
	virtual int GenEntryCellIndex(OUT RESENTRYLIST &stList, IN CExcelSheet *pstSheet, IN LPTDRMETA pstMeta) = NULL;


};
