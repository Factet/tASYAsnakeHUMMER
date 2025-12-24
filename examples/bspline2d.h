class BSpline2D
{
public:
    const static int m_nDimension = 2;  // размерность пространства (без учета веса, учет веса добавляет еще 1)

    BSpline2D();
    ~BSpline2D()
    {
        Clean();
    }
    int GetDegree() const
    {
        return m_ndegree;
    }
    int GetNPoles() const
    {
        return m_npoles;
    }
    int GetNKnots() const
    {
        return m_npoles+m_ndegree+1;
    }

    // The NURBS Book
    int FindSpan(double u) const;
    void BasisFuns(int i, double u, double N[]) const;
    void CurvePoint(double u, double C[m_nDimension]) const;

    // создание NURBS:
    void CreateByPoles(int degree, int npoles, double poles[][m_nDimension]);
    void RemoveLastPole();

    void Print(std::ostream &str = std::cout) const;
    void GetPole(int i, double *x, double *y, double *w) const
    {
        assert(i<m_npoles);
        *x = m_poles[i*(m_nDimension+1)+0];
        *y = m_poles[i*(m_nDimension+1)+1];
        *w = m_poles[i*(m_nDimension+1)+2];
    }
    void SetPole(int i, double x, double y, double w)
    {
        assert(i<m_npoles);
        assert(w>0);
        m_poles[i*(m_nDimension+1)+0] = x;
        m_poles[i*(m_nDimension+1)+1] = y;
        m_poles[i*(m_nDimension+1)+2] = w;
    }
    double GetKnot(int i) const
    {
        assert(i < m_npoles+m_ndegree+1);
        return m_knots[i];
    }

private:
    void Clean();
    void CreateKnotVector();

private:
    const char *m_Classname;
    double *m_poles;
    double *m_knots;
    int m_ndegree;
    int m_npoles;
};


