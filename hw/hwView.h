#include <vector>
#include <atlimage.h>
// hwView.h: ChwView 클래스의 인터페이스
//

#pragma once
struct Node {
	CPoint pt;
	int id;
};

struct Edge {
	int u, v;
	double dist;
};

class ChwView : public CView
{
protected: // serialization에서만 만들어집니다.
	ChwView() noexcept;
	DECLARE_DYNCREATE(ChwView)

// 특성입니다.
public:
	ChwDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
protected:
	CImage m_bgImage;                 
	std::vector<Node> m_nodes;        
	std::vector<Edge> m_edges;         
	std::vector<int> m_shortestPath;   
	int m_selectedNodeIndex = -1;

	int GetClickedNodeIndex(CPoint point);
	void CalculateShortestPath(int startNode, int endNode);
// 구현입니다.
public:
	virtual ~ChwView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // hwView.cpp의 디버그 버전
inline ChwDoc* ChwView::GetDocument() const
   { return reinterpret_cast<ChwDoc*>(m_pDocument); }
#endif

