#ifndef _TREE_H_
#define _TREE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <assert.h>

template <class Type> class CTree;

/*树节点*/
template <class Type> 
class CTreeNode
{
public:
	CTreeNode():m_pstFirstChild(NULL), m_pstNextSibling(NULL){};
	CTreeNode(const Type &stValue, CTreeNode *fc = NULL, CTreeNode<Type> *ns = NULL):m_stValue(stValue),m_pstFirstChild(fc), m_pstNextSibling(ns){		
	};
	~CTreeNode(){};
	
	void SetNodeData(const Type &stValue){m_stValue = stValue;};
	Type &GetNodeData() {return m_stValue;};
public:
	friend class CTree<Type>;	

protected:
	Type	m_stValue;	/*节点附带的数据*/
	CTreeNode *m_pstFirstChild;	/*第一个儿子节点*/
	CTreeNode *m_pstNextSibling;	/*兄弟*/
};

/////////////////////////////////////////////////////////////////////////////////////
/*以儿子-兄弟法组织的树结构*/
template <class Type> 
class CTree
{
public:
	CTree(){m_pstRoot = NULL;};
	CTree(CTreeNode<Type> *pstRoot):m_pstRoot(pstRoot) {};
	~CTree(){};

	void SetRootNode(CTreeNode<Type> *pstRoot) {m_pstRoot = pstRoot;};
	CTreeNode<Type> *GetRootNode() {return m_pstRoot;};

	/**添加儿子节点
	*@param[in] pstParent	父节点指针
	*@param[in] pstChild	子节点指针	
	*@param[in] bAtHeader	如果其取值为true，则将节点插入到儿子链的头部；如果取值为false，则插入到尾部
	*/
	void AddChild(CTreeNode<Type> *pstParent, CTreeNode<Type> *pstChild, bool bAtHeader = true);

	/**获取指定节点的下一个儿子节点，如果pstChild为NULL则获取第一个儿子节点
	*@param[in] pstParent	父节点指针
	*@param[in] pstChild	子节点指针
	*@return a parent's child element or a contiguous element. NULL if
	* there are no more elements.
	*/
	CTreeNode<Type> *GetNextChild(CTreeNode<Type> *pstParent, CTreeNode<Type> *pstChild = NULL);

	/**获取指定节点的下一个兄弟节点
	*@param[in] pstNode	子节点指针
	*@return next sibling of the child element or a contiguous element. NULL if
	* there are no more elements.
	*/
	CTreeNode<Type> *GetNextSibling(CTreeNode<Type> *pstNode);

	int GetChildNum(CTreeNode<Type> *pstNode);
private:
	CTreeNode<Type> *m_pstRoot;	/*根节点*/
};


template <class Type>
void CTree<Type>::AddChild(CTreeNode<Type> *pstParent, CTreeNode<Type> *pstChild, bool bAtHeader /*= true*/)
{
	assert(NULL != pstParent);
	assert(NULL != pstChild);
	
	if (bAtHeader)
	{
		pstChild->m_pstNextSibling = pstParent->m_pstFirstChild;
		pstParent->m_pstFirstChild = pstChild;
	}else
	{
		CTreeNode<Type> *pstPre,*pstNext;
		pstPre = pstParent->m_pstFirstChild;		
		for (pstNext = pstPre; NULL != pstNext;)
		{
			pstPre = pstNext;
			pstNext = pstNext->m_pstNextSibling;
		}
		if (NULL == pstPre)
		{
			pstParent->m_pstFirstChild =  pstChild;
		}else
		{
			pstPre->m_pstNextSibling = pstChild;
		}
	}/*if (bAtHeader)*/	
}

template <class Type>
CTreeNode<Type> *CTree<Type>::GetNextChild(CTreeNode<Type> *pstParent, CTreeNode<Type> *pstChild /*= NULL*/)
{
	CTreeNode<Type> *pstNode = NULL;

	assert(NULL != pstParent);

	pstNode = pstParent->m_pstFirstChild;
	if (NULL != pstChild)
	{
		for (; NULL != pstNode;)
		{
			if (pstNode == pstChild)
			{
				pstNode = pstNode->m_pstNextSibling;
				break;
			}
			pstNode = pstNode->m_pstNextSibling;
		}
	}/*if (NULL != pstChild)*/
	
	return pstNode;
}

template <class Type>
CTreeNode<Type> *CTree<Type>::GetNextSibling(CTreeNode<Type> *pstNode)
{
	assert(NULL != pstNode);

	return pstNode->m_pstNextSibling;
}

template <class Type>
int CTree<Type>::GetChildNum(CTreeNode<Type> *pstNode)
{
	int iCount = 0;
	
	assert(NULL != pstNode);

	CTreeNode<Type> *pstChild = pstNode->m_pstFirstChild;
	for (; NULL != pstChild;)
	{
		iCount++;
		pstChild = pstChild->m_pstNextSibling;
	}

	return iCount;
}
#endif
