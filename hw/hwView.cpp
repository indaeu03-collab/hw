
// hwView.cpp: ChwView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "hw.h"
#endif

#include "hwDoc.h"
#include "hwView.h"

#include <cmath>
#include <queue>
#include <limits>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ChwView

IMPLEMENT_DYNCREATE(ChwView, CView)

BEGIN_MESSAGE_MAP(ChwView, CView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// ChwView 생성/소멸

ChwView::ChwView() noexcept
{
	m_selectedNodeIndex = -1;
	// TODO: 여기에 생성 코드를 추가합니다.

}

ChwView::~ChwView()
{
}

BOOL ChwView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// ChwView 그리기

void ChwView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	HRESULT hr = m_bgImage.Load(_T("map.png"));
	if (FAILED(hr)) { 
	}
}

void ChwView::OnDraw(CDC* /*pDC*/)
{
	ChwDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
}
void ChwView::OnPaint()
{
    CPaintDC dc(this);

    if (!m_bgImage.IsNull()) {
        m_bgImage.Draw(dc.m_hDC, 0, 0);
    }

    CPen penLine(PS_SOLID, 2, RGB(0, 0, 0));       
    CPen penPath(PS_SOLID, 5, RGB(255, 0, 0));    
    CBrush brushNode(RGB(0, 0, 255));              
    CFont font;
    font.CreatePointFont(100, _T("Arial"));        

 
    CPen* pOldPen = dc.SelectObject(&penLine);
    CBrush* pOldBrush = dc.SelectObject(&brushNode);
    CFont* pOldFont = dc.SelectObject(&font);
    int nOldBkMode = dc.SetBkMode(TRANSPARENT);    

 
    for (const auto& edge : m_edges) {
        CPoint p1 = m_nodes[edge.u].pt;
        CPoint p2 = m_nodes[edge.v].pt;

        dc.MoveTo(p1);
        dc.LineTo(p2);


        CString strDist;
        strDist.Format(_T("%.0f"), edge.dist); 
        dc.TextOut((p1.x + p2.x) / 2, (p1.y + p2.y) / 2, strDist);
    }


    if (m_shortestPath.size() > 1) {
        dc.SelectObject(&penPath); 
        for (size_t i = 0; i < m_shortestPath.size() - 1; ++i) {
            CPoint p1 = m_nodes[m_shortestPath[i]].pt;
            CPoint p2 = m_nodes[m_shortestPath[i + 1]].pt;
            dc.MoveTo(p1);
            dc.LineTo(p2);
        }
        dc.SelectObject(&penLine); 
    }


    for (const auto& node : m_nodes) {
        dc.Ellipse(node.pt.x - 5, node.pt.y - 5, node.pt.x + 5, node.pt.y + 5);
    }

    if (m_selectedNodeIndex != -1) {
        CPoint pt = m_nodes[m_selectedNodeIndex].pt;
        dc.TextOut(pt.x - 15, pt.y - 25, _T("Selected"));
    }

    dc.SelectObject(pOldPen);
    dc.SelectObject(pOldBrush);
    dc.SelectObject(pOldFont);
    dc.SetBkMode(nOldBkMode);
}

void ChwView::OnLButtonDown(UINT nFlags, CPoint point)
{
    bool bCtrl = (GetKeyState(VK_CONTROL) < 0);
    bool bShift = (GetKeyState(VK_SHIFT) < 0);
    bool bAlt = (GetKeyState(VK_MENU) < 0); 

    if (bCtrl && bShift) {
        int clickedIdx = GetClickedNodeIndex(point);
        if (clickedIdx != -1) {
            if (m_selectedNodeIndex == -1) {
                m_selectedNodeIndex = clickedIdx;
                m_shortestPath.clear(); 
            }
            else {
                CalculateShortestPath(m_selectedNodeIndex, clickedIdx);
                m_selectedNodeIndex = -1; 
            }
            Invalidate(); 
        }
    }
    else if (bCtrl && !bShift && !bAlt) {
        Node newNode;
        newNode.pt = point;
        newNode.id = (int)m_nodes.size();
        m_nodes.push_back(newNode);

        m_shortestPath.clear(); 
        Invalidate();
    }

    else if (bAlt && !bCtrl) {
        int clickedIdx = GetClickedNodeIndex(point);
        if (clickedIdx != -1) {
            if (m_selectedNodeIndex == -1) {
                m_selectedNodeIndex = clickedIdx;
            }
            else {
                if (m_selectedNodeIndex != clickedIdx) {
                    CPoint p1 = m_nodes[m_selectedNodeIndex].pt;
                    CPoint p2 = m_nodes[clickedIdx].pt;
                    double dist = std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2));
                    m_edges.push_back({ m_selectedNodeIndex, clickedIdx, dist });
                }
                m_selectedNodeIndex = -1; 
            }
            Invalidate();
        }
    }
    else {
        m_selectedNodeIndex = -1;
        Invalidate();
    }

    CView::OnLButtonDown(nFlags, point);
}

int ChwView::GetClickedNodeIndex(CPoint point)
{
    const int RADIUS = 15; 
    for (int i = 0; i < (int)m_nodes.size(); ++i) {
        if (std::abs(point.x - m_nodes[i].pt.x) < RADIUS &&
            std::abs(point.y - m_nodes[i].pt.y) < RADIUS) {
            return i;
        }
    }
    return -1;
}
void ChwView::CalculateShortestPath(int startNode, int endNode)
{
    int n = (int)m_nodes.size();
    const double INF = (std::numeric_limits<double>::max)(); 

    std::vector<double> dist(n, INF); 
    std::vector<int> parent(n, -1);   

    std::priority_queue<std::pair<double, int>,
        std::vector<std::pair<double, int>>,
        std::greater<std::pair<double, int>>> pq;

    dist[startNode] = 0;
    pq.push({ 0, startNode });

    while (!pq.empty()) {
        double d = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;
        if (u == endNode) break; 

        for (const auto& edge : m_edges) {
            int v = -1;
            if (edge.u == u) v = edge.v;
            else if (edge.v == u) v = edge.u;

            if (v != -1) {
                if (dist[u] + edge.dist < dist[v]) {
                    dist[v] = dist[u] + edge.dist;
                    parent[v] = u;
                    pq.push({ dist[v], v });
                }
            }
        }
    }

    m_shortestPath.clear();
    if (dist[endNode] != INF) {
        int curr = endNode;
        while (curr != -1) {
            m_shortestPath.push_back(curr);
            curr = parent[curr];
        }
    }
    else {
        AfxMessageBox(_T("두 점 사이에 연결된 경로가 없습니다."));
    }
}


void ChwView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void ChwView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// ChwView 진단

#ifdef _DEBUG
void ChwView::AssertValid() const
{
	CView::AssertValid();
}

void ChwView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

ChwDoc* ChwView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(ChwDoc)));
	return (ChwDoc*)m_pDocument;
}
#endif //_DEBUG


// ChwView 메시지 처리기
