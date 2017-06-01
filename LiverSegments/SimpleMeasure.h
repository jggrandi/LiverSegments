#ifndef SIMPLEMEASURE_INCLUDED
#define SIMPLEMEASURE_INCLUDED

#include <vector>
#include "VertexPool.h"

class CSimpleMeasure {
public:
	CSimpleMeasure();
	~CSimpleMeasure();
	void SaveObjectTransform();
	void AddPoint(int xWindow, int yWindow);
	void RemovePoint();
	void GetCandidatePoint(int xWindow, int yWindow, float *xyz);
	void Draw() const;
	void DrawPoints();

	std::vector<std::vector<int>> voxelsVolume;

private:
	CSimpleMeasure(const CSimpleMeasure &simpleMeasure) {}
	CSimpleMeasure &operator=(const CSimpleMeasure &simpleMeasure) {}

protected:
	double m_viewMatrix[16];
    double m_projectionMatrix[16];
	int m_viewport[4];
	CVertexPool m_line;
};

#endif // SIMPLEMEASURE_INCLUDED

