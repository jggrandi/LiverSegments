#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include <GL/gl.h>
#include <GL/glu.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//#include <iostream>

#include <ErrorManager.h>

#include "SimpleMeasure.h"

using namespace std;

/**
*/
CSimpleMeasure::CSimpleMeasure(void) {
	m_line.SetMode(CVertexPool::LINE_STRIP);
	m_line.SetVertexSize(3);
}

/**
*/
CSimpleMeasure::~CSimpleMeasure(void) {
}

/**
*/
void CSimpleMeasure::SaveObjectTransform() {
	glGetDoublev(GL_MODELVIEW_MATRIX, m_viewMatrix);

	glGetDoublev(GL_PROJECTION_MATRIX, m_projectionMatrix);

	glGetIntegerv(GL_VIEWPORT, m_viewport);
}

/**
*/
void CSimpleMeasure::AddPoint(int xWindow, int yWindow) {
	GLdouble x,
		y,
		z;
	int yWindowNorm;
	float depth;

	yWindowNorm = glutGet(GLUT_WINDOW_HEIGHT) - yWindow;

	glReadPixels((float)xWindow, (float)yWindowNorm, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

//	std::cout << xWindow << " " << yWindowNorm << " " << depth << std::endl;
	gluUnProject((float)xWindow, (float)yWindowNorm, depth, m_viewMatrix, m_projectionMatrix, m_viewport, &x, &y, &z);
//	std::cout << x << " " << y << " " << z << std::endl;
	
	m_line.AddElement(m_line.GetElementCount());
	m_line.AddVertex((float)x, (float)y, (float)z);
}

void CSimpleMeasure::RemovePoint(){
	m_line.RemoveElement();
	m_line.RemoveVertex3D();

}

void CSimpleMeasure::GetCandidatePoint(int xWindow, int yWindow, float xyz[3]) {
	GLdouble x,
		y,
		z;
	int yWindowNorm;
	float depth;

	yWindowNorm = glutGet(GLUT_WINDOW_HEIGHT) - yWindow;

	glReadPixels((float)xWindow, (float)yWindowNorm, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	gluUnProject((float)xWindow, (float)yWindowNorm, depth, m_viewMatrix, m_projectionMatrix, m_viewport, &x, &y, &z);

	xyz[0]= x;
	xyz[1]= y;
	xyz[2]= z;

}

/**
*/
void CSimpleMeasure::Draw() const {
	m_line.Draw();
}

void CSimpleMeasure::DrawPoints() {
	m_line.SetMode(CVertexPool::POINTS);
	m_line.Draw();
	m_line.SetMode(CVertexPool::LINE_STRIP);
}