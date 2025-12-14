// ConvResTree.h  : 资源转换树头文件
//

#ifndef CONVBASE_H
#define CONVBASE_H

#include <string>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

CString GetRootPath();
CString MakePath(CString &szBasePath, CString &szPath);

/*判断一个字符串是否是十进制的数字串
*@return true:是，false：不是数字串
*/
bool IsDigitStr(CString &szTmp);


void GetNumber(IN CString& szCell, OUT CString& szNumber);
void GetPercent(IN CString& cNumber, OUT CString &szPresent);




#endif