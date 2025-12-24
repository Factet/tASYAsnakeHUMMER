#include <cassert>
#include <cstring>
#include <iostream>
#include "bspline2d.h"

BSpline2D::BSpline2D()
{
    m_Classname = __FUNCTION__;
    m_npoles = 0;
    m_poles = nullptr;
    m_ndegree = 0;
    m_knots = nullptr;
}

// найти номер интервала между узлами, которому принадлежит u
// The NURBS Book, alg. 2.1
int BSpline2D::FindSpan(double u) const
{
    if(u == GetKnot(m_npoles))  // количество полюсов m_poles = n + 1
    {
        return m_npoles-1;
    }
    int low = m_ndegree;
    int high = m_npoles;
    int mid = (low+high)/2;
    while(u < GetKnot(mid) || u >= GetKnot(mid+1))
    {
        if(u < GetKnot(mid))
        {
            high = mid;
        }
        else
        {
            low = mid;
        }
        mid =(low+high)/2;
    }
    return mid;
}

// Вычисление ненулевых базисных функций
// The NURBS Book, alg. 2.2
void BSpline2D::BasisFuns(int i, double u, double N[]) const
{
    double left[m_ndegree+1], right[m_ndegree+1], saved, temp;
    N[0] = 1.0;
    for(int j=1; j<=m_ndegree; j++)
    {
        left[j] = u-GetKnot(i+1-j);
        right[j] = GetKnot(i+j)-u;
        saved = 0.0;
        for(int r=0; r<j; r++)
        {
            temp = N[r]/(right[r+1]+left[j-r]);
            N[r] = saved+right[r+1]*temp;
            saved = left[j-r]*temp;
        }
        N[j] = saved;
    }
}

// Вычисление точки на кривой
// The NURBS Book, alg. 4.1
void BSpline2D::CurvePoint(double u, double C[m_nDimension]) const
{
    int span = FindSpan(u);
    double N[m_ndegree+1], W, x, y, w;
    BasisFuns(span,u,N);
    C[0] = 0.0;
    C[1] = 0.0;
    W    = 0.0;
    for(int i=0; i<=m_ndegree; i++)
    {
        GetPole(span-m_ndegree+i, &x, &y, &w);
        C[0] += N[i]*x;
        C[1] += N[i]*y;
        W    += N[i]*w;
    }
    C[0] /= W;
    C[1] /= W;
}

void  BSpline2D::Print(std::ostream &str) const
{
    str << m_Classname << " NURBS:" << (m_ndegree+1 == m_npoles ? " Bézier" : "") << std::endl
        << "Степень = " << m_ndegree << std::endl
        << "Число полюсов = " << m_npoles << std::endl
        << "Число узлов = " << m_ndegree+m_npoles+1 << std::endl
        << std::endl;
    double px, py, w;
    for(int i=0; i<m_npoles; i++)
    {
        GetPole(i, &px, &py, &w);
        str << "Полюс " << i << " = ( " << px/w << ", " << py/w << " ) вес = " << w << std::endl;
    }
    for(int i=0; i<m_ndegree+m_npoles+1; i++)
    {
        str << "Узел " << i << " = " << GetKnot(i) << std::endl;
    }
}

void  BSpline2D::Clean()
{
    if(m_npoles>0)
    {
        delete [] m_poles;
        m_poles = nullptr;
        m_npoles = 0;
        m_ndegree = 0;
        delete [] m_knots;
        m_knots = nullptr;
    }
}

void  BSpline2D::CreateKnotVector()
{
    int m = m_npoles + m_ndegree + 1; // Подсчет количества узлов
    assert(m_knots==nullptr);
    m_knots = new double[m];
    for (int i = 0; i < m_ndegree + 1; i++) // Первые p + 1 узлов 0.0
    {
        m_knots[i] = 0.0;
    }
    for (int i = m_ndegree + 1; i < m - m_ndegree -1; i++) // Внутренние узлы равномерно распределяются на интервале
    {
        m_knots[i] = ((double) (i - m_ndegree)) / (m_npoles - m_ndegree);
    }
    for (int i = m-m_ndegree-1; i < m; i++) // Последние p + 1 узлов 1.0
    {
        m_knots[i] = 1.0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void BSpline2D::CreateByPoles(int degree, int npoles, double poles[][m_nDimension])
{
    assert(npoles>1);
    Clean();

    m_npoles = npoles;
    m_poles = new double[m_npoles*(m_nDimension+1)];
    for(int i=0; i<m_npoles; i++)
    {
        m_poles[i*(m_nDimension+1)+0] = poles[i][0];
        m_poles[i*(m_nDimension+1)+1] = poles[i][1];
        m_poles[i*(m_nDimension+1)+2] = 1.0;
    }
    m_ndegree = degree < npoles ? degree : npoles-1;
    CreateKnotVector();
}

void BSpline2D::RemoveLastPole()
{
    if(m_npoles > 2)
    {
        --m_npoles;
        m_ndegree = m_ndegree < m_npoles ? m_ndegree : m_npoles-1;
        delete [] m_knots;
        m_knots = nullptr;
        CreateKnotVector();
    }
}

