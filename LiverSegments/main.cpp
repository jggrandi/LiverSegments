// Body01.
#define MINIMUM_VALUE (-1024)
#define MAXIMUM_VALUE 3071

// fengine.
//#define MINIMUM_VALUE 0
//#define MAXIMUM_VALUE 255

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//#include <GL/glui.h>
#include <PinholeCamera.h>
#include <ErrorManager.h>
#include <GageAdaptor.h>
#include "VertexPool.h"
#include <BoundingBox.h>
#include <Texture.h>
#include <Shader.h>
#include <MyGLH.h>
#include <MyMath.h>
#include <Timer.h>

#include "wsg.h"

#include "VolumeArray.h"

#include "SimpleMeasure.h"

#include <Commdlg.h>
#include <AntTweakBar.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>


using namespace std;

// Image.
string m_imageName;
boost::shared_ptr<CGageAdaptor> m_image;

	int obj = 0;
int m_voxelsDoVolume =0;
int m_voxelsLoboEsquerdo =0;
int m_voxelsLoboDireito =0;
float m_volumeDeUmVoxel =0.0;
bool VolumeIsLoaded = false;

enum ApplicationModeEnum {
	// In this mode, the left button of the mouse displaces the image in the XY 
	// axis of the camera and the right button of the mouse changes camera's 
	// near plane size.
	MOVINGBODYMODE,
	// In this mode, the left button of the mouse adds a new point to the current 
	// volume
	ADDINGPOINTSMODE,
	// In this mode, the left button of the mouse changes the window (transfer 
	// function) of the current slice (m_currentSlice).
	LBUTTON2WINDOW,

	SELECTINGVOXEL
};

#define MOVING_BODY_ID			10
#define ADDING_POINTS_ID		20
#define NEW_LAYER_ID			30
#define LAYER_FORWARD_ID		40
#define LAYER_BACKWARD_ID		50
#define VOLUME_FORWARD_ID		60
#define VOLUME_BACKWARD_ID		70
#define REGULAGEM_JANELA_ID		80
#define SELECT_VOXEL_ID			90
#define FLOOD_FILL_ID			100
#define VORONOI_ALPHA_ID		110
#define OPEN_ID					120
#define OK_ID					130
#define SHOW_DATASET_ID			0
#define SHOW_MOLDE_ID			1
#define SHOW_SEGMENTADO_ID		2
#define SAVE_SEGMENTS_ID		3
#define SAVE_DENSITY_ID			5
#define SAVE_SEGMENTS_RAW_ID	6
#define LOAD_SEGMENTS_ID		7


Volume m_volumeData;
//	Volume m_volumeMolde;
boost::shared_ptr<Volume> m_voronoiVolume;

boost::shared_ptr<Volume> m_currentVolume;
boost::shared_ptr<VolumeArray> m_volumeArray;
	//vector<short> m_volumeTextureDataLabel;
int m_currentID = 0;

	int m_selectedVoxel[3];

	float	m_floodFillRange;

//GLUI_RadioGroup *radio;

ApplicationModeEnum m_applicationMode = MOVINGBODYMODE;

// .
boost::shared_ptr<CTexture> m_volumeTexture;
boost::shared_ptr<CTexture> m_voronoiVolumeTexture;
int m_voronoiAlpha = 10;
//CTexture m_preIntegrationTableTexture;
CTexture m_preIntegrationTableTexture;
CTexture m_voronoiTags;
CBoundingBox m_meshBoundingBox;
int m_width = 1;
int m_height = 1;
int m_depth = 1;
GLubyte m_lookUpTable[4*256];
int m_windowCenter = 400;
int m_windowWidth = 50;
CShader m_preIntegrationShader;
// Camera.
boost::shared_ptr<CPinholeCamera> m_camera;
int m_mouseButton = -1;
int m_mouseX = -1;
int m_mouseY = -1;
// Adding points.
boost::shared_ptr<CSimpleMeasure>   m_currentMeasureLayer;
vector <boost::shared_ptr<CSimpleMeasure>> m_simpleMeasureVector;
float m_alphaThreshold = 0.025f;


ApplicationModeEnum GetApplicationMode(void);
bool Open();
bool measureLayerForward();
bool measureLayerBackward();
bool AddMeasureLayer(void);
void DrawSlices(void);
void GetColorMapping(int normalizedValue, unsigned char *r, unsigned char *g, unsigned char *b);
void InitializeLookUpTable(int windowCenter, int windowWidth);
GLubyte clamp(int value, int minValue, int maxValue);
bool createPreintegrationTable(GLubyte* Table);
bool InitializeShaders(void);
bool OnCreate(int argc, char** argv);
void OnDestroy(void);
void OnPaint(void);
void OnSize(GLsizei w, GLsizei h);
void OnIdle(void);
void OnKeyDown(unsigned char key, int x, int y);
float getClosest(int pos[3]);
void calcEsqDir();
bool calcVoronoi();
void OnMouse(int button, int state, int x, int y);
void OnMouseMove(int x, int y);
void OnPassiveMouseMove(int x, int y);
bool SetApplicationMode(ApplicationModeEnum applicationMode);
void setMode( int control );
bool OnCreate(int argc, char** argv);
void onSave(std::string tipoDeArquivo);
bool LoadVolume(char* fileName = NULL);
bool LoadVoronoiVolume(char* fileName = NULL);
void onLoad();
void OpenMenuCreate();

int volumeFigado=0;

char show_text[255] = ""; 

/**
 *	Uses windows tool to select file to open
 */
bool OpenFileWindow(char file[255], char type[255])
{
	OPENFILENAME ofn;
	char szfile[255];

	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL ;
	ofn.lpstrFile = file ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szfile );
	ofn.lpstrFilter = type;
	ofn.nFilterIndex =1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir = NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if (GetOpenFileName( &ofn )) return true;
	else return false;
}


bool Open()
{	
	VolumeIsLoaded = false;
	//m_currentVolume = NULL;
	//openfile window	
	char szFile[255] ;
	char type[255] = "NHDR\0*.NHDR\0All\0*.*\0";	

	if (!OpenFileWindow(szFile,type)) return true;

	int width, height, slices;
	string file(szFile);
	ifstream nhdrFile;
	nhdrFile.open(file.c_str());
	string line;
	getline(nhdrFile, line);
	getline(nhdrFile, line);
	getline(nhdrFile, line);
	getline(nhdrFile, line);
	getline(nhdrFile, line);
	sscanf (line.c_str(),"%*s %d %d %d", &width, &height, &slices);
	nhdrFile.close();
	
	m_depth = slices;

	if (width>256) m_width = 256;
	else m_width = width;
	if (height>256) m_height = 256;
	else m_height = height;

	m_volumeDeUmVoxel=((36.0/512.0)*(36.0/512.0)*1.3*(512.0/m_width)*(512.0/m_height)*(345.0/m_depth));
	
	strcpy (show_text, "Loading...");
	OnPaint();
	glutPostRedisplay();

	if (!LoadVolume(szFile))
	{
		MarkError();
		return false;
	}

	if (!OpenFileWindow(szFile,type)) return true;

	strcpy (show_text, "Loading...");
	OnPaint();
	glutPostRedisplay();

	if (!LoadVolume(szFile))
	{
		MarkError();
		return false;
	}

	strcpy (show_text, "Building volume...");
	OnPaint();
	glutPostRedisplay();

	if (!LoadVolume())
	{
		MarkError();
		return false;
	}

	if (!LoadVoronoiVolume())
	{
		MarkError();
		return false;
	}


	volumeFigado = m_volumeArray->RecortaVolume(0.3f);

	InitializeLookUpTable(m_windowCenter, m_windowWidth);

	createPreintegrationTable(m_lookUpTable);


	VolumeIsLoaded = true;
	strcpy (show_text, "");

	glutPostRedisplay();

	return true;
}

void ShowText(char* text)
{
	glColor3f(0.0f, 0.0f, 0.0f);

	if (text!="")
	{
		glRasterPos2i(40, 5 );

		for(int i=0; i<(int)strlen( text ); i++ )
		  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, text[i] );
	}
}

bool measureLayerForward()
{
	for (int i = 0 ; i < m_simpleMeasureVector.size() ; i++){
		if (m_simpleMeasureVector[i] == m_currentMeasureLayer)
		{
			if (i < m_simpleMeasureVector.size()-1)
				i++;
			//m_currentLayer = i;
			m_currentMeasureLayer = (m_simpleMeasureVector[i]);
			return true;
		}
	}
	return false;
}

bool measureLayerBackward()
{
	for (int i = 0 ; i < m_simpleMeasureVector.size() ; i++)
	{
		if (m_simpleMeasureVector[i] == m_currentMeasureLayer)
		{
			if (i > 0)
				i--;
			//m_currentLayer = i;
			m_currentMeasureLayer = (m_simpleMeasureVector[i]);
			return true;
		}
	}
	return false;
}



/**
*/
bool AddMeasureLayer(void)
{
	boost::shared_ptr<CSimpleMeasure>  simpleMeasure;

	simpleMeasure.reset(new CSimpleMeasure);
	if (!simpleMeasure.get())
	{
		MarkError();

		return false;
	}

	m_currentMeasureLayer.reset(new CSimpleMeasure);

	if (!m_currentMeasureLayer.get())
	{
		MarkError();

		return false;
	}


	m_simpleMeasureVector.push_back(simpleMeasure);
	m_currentMeasureLayer = simpleMeasure;
	return true;

};

/**
*/
void DrawSlices(void)
{
	float modelViewMatrix[16],
		u[3],
		v[3],
		eyeVector[3];

	m_preIntegrationShader.Bind();

	m_volumeTexture->Bind(0);
	m_preIntegrationShader.SetTextureUnit("Volume", 0);

	m_voronoiVolumeTexture->Bind(1);
	m_preIntegrationShader.SetTextureUnit("Voronoi", 1);

	m_preIntegrationTableTexture.Bind(2);

	m_preIntegrationShader.SetTextureUnit("PreIntegrationTable", 2);

	m_voronoiTags.Bind(3);
	m_preIntegrationShader.SetTextureUnit("VoronoiColors", 3);

	m_preIntegrationShader.SetUniformParameter("SliceDistance", 0.0);
	//m_preIntegrationShader.SetUniformParameter("SliceDistance", SPACING);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_BLEND);


	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);

	u[0] = modelViewMatrix[0];
	u[1] = modelViewMatrix[4];
	u[2] = modelViewMatrix[8];

	v[0] = modelViewMatrix[1];
	v[1] = modelViewMatrix[5];
	v[2] = modelViewMatrix[9];

	MyMath::CrossProduct<float>(v, u, eyeVector);

	glColor3f(1.0f, 0.0f, 1.0f);

	plot(eyeVector);

	glDisable(GL_BLEND);

	CTexture::DisableTextureUnit(0);
	CTexture::DisableTextureUnit(1);
	CTexture::DisableTextureUnit(2);
	CTexture::DisableTextureUnit(3);

	CShader::Disable();

}

/**
*/
void GetColorMapping(int normalizedValue, unsigned char *r, unsigned char *g, unsigned char *b)
{
	int unnormalizedKeyValueArray[13] = {
		-2048,
		-1024,
		-741,
		-70,
		37,
		42,
		53,
		75,
		98,
		190,
		260,
		1376,
		3071  /*
		500,//-741,
		600,//-70,
		700,//37,
		1000,//42,
		1300,//53,
		1500,//75,
		2000,//98,
		4000,//190,
		10000,//260,
		35000,//1376,
		50000//3071  */
	};
	unsigned char colorMappingArray[13][3] = {
		{0, 0, 0},
		{0, 0, 0},
		{70, 28, 19},
		{255, 214, 152},
		{176, 140, 156},
		//{100, 42, 31},
		{128, 42, 31},
		//{68, 31, 25},
		{128, 31, 25},
		{149, 130, 100},
		{255, 255, 255},
		{164, 150, 123},
		{161, 146, 115},
		{161, 146, 115},
		{255, 255, 255}
	};
	int unnormalizedValue,
		i;
	float t;

	*r = 0;
	*g = 0;
	*b = 0;

	unnormalizedValue = MINIMUM_VALUE + (float(normalizedValue)/255.0f)*float(MAXIMUM_VALUE - MINIMUM_VALUE);

	for (i=0; i<(13 - 1); i++)
		if ((unnormalizedValue >= unnormalizedKeyValueArray[i]) &&(unnormalizedValue < unnormalizedKeyValueArray[i + 1]))
		{
			t = float(unnormalizedValue - unnormalizedKeyValueArray[i])/float(unnormalizedKeyValueArray[i + 1] - unnormalizedKeyValueArray[i]);

			*r = (1.0f - t)*float(colorMappingArray[i][0]) + t*float(colorMappingArray[i + 1][0]);
			*g = (1.0f - t)*float(colorMappingArray[i][1]) + t*float(colorMappingArray[i + 1][1]);
			*b = (1.0f - t)*float(colorMappingArray[i][2]) + t*float(colorMappingArray[i + 1][2]);
		}
}

/**
*/
void InitializeLookUpTable(int windowCenter, int windowWidth)
{
	int i;
	float windowCenterNormalized, 
		windowBeginNormalized,
		windowEndNormalized;
	unsigned char level;

	windowCenterNormalized = 255.0f*(float(windowCenter - MINIMUM_VALUE)/(MAXIMUM_VALUE - MINIMUM_VALUE));

	windowBeginNormalized = float(windowCenterNormalized) - 0.5f*float(windowWidth);
	windowEndNormalized = windowBeginNormalized + float(windowWidth);

	for (i=0; i<256; ++i)
	{
		if (i > windowEndNormalized)
			level = 255;
		else if (i > windowBeginNormalized)
			level = unsigned char(255.0f*((float(i) - windowBeginNormalized)/float(windowWidth)));
		else
			level = 0;

		GetColorMapping(i, &m_lookUpTable[4*i + 0], &m_lookUpTable[4*i + 1], &m_lookUpTable[4*i + 2]);
		m_lookUpTable[4*i + 3] = level;
	}

	cout << "Window center: " << windowCenter << endl;
	cout << "Window width: " << int((float(windowWidth)/255.0f)*5119.0f) << endl;
}

/**
@inproceedings{1103929,
 author = {Klaus Engel and Markus Hadwiger and Joe M. Kniss and Aaron E. Lefohn and Christof Rezk Salama and Daniel Weiskopf},
 title = {Real-time volume graphics},
 booktitle = {SIGGRAPH '04: ACM SIGGRAPH 2004 Course Notes},
 year = {2004},
 pages = {29},
 location = {Los Angeles, CA},
 doi = {http://doi.acm.org/10.1145/1103900.1103929},
 publisher = {ACM},
 address = {New York, NY, USA},
 }
*/
GLubyte clamp(int value, int minValue, int maxValue)
{
	if (value < minValue)
		return (GLubyte)minValue;
	else if (value > maxValue)
		return (GLubyte)maxValue;

	return (GLubyte)value;
}

/**
@inproceedings{1103929,
 author = {Klaus Engel and Markus Hadwiger and Joe M. Kniss and Aaron E. Lefohn and Christof Rezk Salama and Daniel Weiskopf},
 title = {Real-time volume graphics},
 booktitle = {SIGGRAPH '04: ACM SIGGRAPH 2004 Course Notes},
 year = {2004},
 pages = {29},
 location = {Los Angeles, CA},
 doi = {http://doi.acm.org/10.1145/1103900.1103929},
 publisher = {ACM},
 address = {New York, NY, USA},
 }
*/
bool createPreintegrationTable(GLubyte* Table) 
{
	double r,
		g,
		b,
		a,
		rInt[256],
		gInt[256],
		bInt[256],
		aInt[256],
		factor,
		// ?
		tauc;
	int rcol,
		gcol,
		bcol,
		acol,
		smin,
		smax,
		lookupindex;
	GLubyte lookupImg[256*256*4];
	CTimer timer;

	r = 0.0;
	g = 0.0;
	b = 0.0;
	a = 0.0;

	rInt[0] = 0.0;
	gInt[0] = 0.0;
	bInt[0] = 0.0;
	aInt[0] = 0.0;

	lookupindex = 0;

	// compute integral functions
	for (int i=1; i<256; i++)
	{
		tauc = (Table[(i - 1)*4 + 3] + Table[i*4 + 3])/2.0;
		r = r + (Table[(i - 1)*4 + 0] + Table[i*4 + 0])/2.0*tauc/255.0;
		g = g + (Table[(i - 1)*4 + 1] + Table[i*4 + 1])/2.0*tauc/255.0;
		b = b + (Table[(i - 1)*4 + 2] + Table[i*4 + 2])/2.0*tauc/255.0;
		a = a + tauc;

		rInt[i] = r;
		gInt[i] = g;
		bInt[i] = b;
		aInt[i] = a;
	}

	// compute look-up table from integral functions
	for (int sb=0; sb<256; sb++)
		for (int sf=0; sf<256; sf++)
		{
			if (sb < sf)
			{
				smin = sb;
				smax = sf;
			}
			else
			{
				smin = sf;
				smax = sb;
			}

			if (smax != smin)
			{
				factor = 1.0/(double)(smax - smin);

				rcol = (rInt[smax] - rInt[smin])*factor;
				gcol = (gInt[smax] - gInt[smin])*factor;
				bcol = (bInt[smax] - bInt[smin])*factor;
				acol = 256.0*(1.0 - exp(-(aInt[smax] - aInt[smin])*factor/255.0));
			}
			else
			{
				factor = 1.0/255.0;

				rcol = Table[smin*4+0]*Table[smin*4+3]*factor;
				gcol = Table[smin*4+1]*Table[smin*4+3]*factor;
				bcol = Table[smin*4+2]*Table[smin*4+3]*factor;
				acol = (1.0 - exp(-Table[smin*4 + 3]*factor))*256.0;
			}

			lookupImg[lookupindex++] = clamp(rcol, 0, 255);
			lookupImg[lookupindex++] = clamp(gcol, 0, 255);
			lookupImg[lookupindex++] = clamp(bcol, 0, 255);
			lookupImg[lookupindex++] = clamp(acol, 0, 255);
		}


	if (!m_preIntegrationTableTexture.IsValid())
	{
		if (!m_preIntegrationTableTexture.Initialize(CTexture::TEXTURE_2D, CTexture::RGBA))
		{
			MarkError();

			return false;
		}

		m_preIntegrationTableTexture.SetParameter(CTexture::MIN_FILTER, CTexture::LINEAR);
		m_preIntegrationTableTexture.SetParameter(CTexture::MAG_FILTER, CTexture::LINEAR);
		m_preIntegrationTableTexture.SetParameter(CTexture::WRAP_S, CTexture::CLAMP_TO_EDGE);
		m_preIntegrationTableTexture.SetParameter(CTexture::WRAP_T, CTexture::CLAMP_TO_EDGE);
	}

	if (!m_voronoiTags.IsValid())
	{
		if (!m_voronoiTags.Initialize(CTexture::TEXTURE_2D, CTexture::RGBA))
		{
			MarkError();

			return false;
		}

		m_voronoiTags.SetParameter(CTexture::MIN_FILTER, CTexture::LINEAR);
		m_voronoiTags.SetParameter(CTexture::MAG_FILTER, CTexture::LINEAR);
		m_voronoiTags.SetParameter(CTexture::WRAP_S, CTexture::CLAMP_TO_EDGE);
		m_voronoiTags.SetParameter(CTexture::WRAP_T, CTexture::CLAMP_TO_EDGE);
	}

	if (!m_preIntegrationTableTexture.SetImage(256, 256, 0, CTexture::RGBA, CTexture::UNSIGNED_BYTE, lookupImg))
	{
		MarkError();

		return false;
	}
	GLubyte lookupImgColors[10*4];
	//GLubyte lookupImgColors[10*3];
	lookupindex=0;
	acol=50;
	int VoronoiColors[10*4]={0,0,0,0,
		0,0,255,m_voronoiAlpha,
		0,255,0,m_voronoiAlpha,
		255,255,0,m_voronoiAlpha,
		0,255,0,m_voronoiAlpha,
		255,0,0,m_voronoiAlpha,
		0,0,255,m_voronoiAlpha,
		127,255,0,m_voronoiAlpha,
		255,0,127,m_voronoiAlpha,
		0,127,255,m_voronoiAlpha};

	for (int i = 0; i < 10 ; i ++)
	{
			lookupImgColors[lookupindex++] = clamp(VoronoiColors[lookupindex], 0, 255);
			lookupImgColors[lookupindex++] = clamp(VoronoiColors[lookupindex], 0, 255);
			lookupImgColors[lookupindex++] = clamp(VoronoiColors[lookupindex], 0, 255);
			lookupImgColors[lookupindex++] = clamp(VoronoiColors[lookupindex], 0, 255);
			
	}

	if (!m_voronoiTags.SetImage(10, 1, 0, CTexture::RGBA, CTexture::UNSIGNED_BYTE, lookupImgColors))
	{
		MarkError();

		return false;
	}

	if (!MyGLH::IsRenderingContextOk())
	{
		MarkError();

		return false;
	}

	cout << "Pre-integration table creation elapsed time: " << long(timer.GetElapsed()) << " ms." << endl;

	return true;
}

/**
*/
bool InitializeShaders(void)
{
	if (!m_preIntegrationShader.Initialize())
	{
		MarkError();

		return false;
	}

	if (!m_preIntegrationShader.LoadFromFile("preIntegrationVertexShader.glsl", CShader::VERTEX_PROGRAM))
	{
		MarkError();

		return false;
	}

	if (!m_preIntegrationShader.LoadFromFile("preIntegrationPixelShader.glsl", CShader::FRAGMENT_PROGRAM))
	{
		MarkError();

		return false;
	}

	if (!MyGLH::IsRenderingContextOk())
	{
		MarkError();

		return false;
	}

	return true;
}

/**
*/
void OnDestroy(void)
{
	int windowName;

	windowName = glutGetWindow();

	if (windowName)
		glutDestroyWindow(windowName);

	if (!CErrorManager::Instance().IsOk())
		CErrorManager::Instance().FlushErrors();

	exit(EXIT_SUCCESS);
}
  	CSimpleMeasure m_mouseLine;
/**
*/
void OnPaint(void)
{
	float width,
		height,
		depth,
		sizeMax;

	glAlphaFunc(GL_GREATER, m_alphaThreshold);

	glEnable(GL_ALPHA_TEST);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   // m_camera->ApplyTransform();

    glEnable(GL_DEPTH_TEST);

	//MyGLH::DrawGrid();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0, 100.0, 0.0, 100.0  );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	ShowText(show_text);

	if (!VolumeIsLoaded){
		// Draw tweak bars
		TwDraw();
		glutSwapBuffers();
		return;
	}

	m_camera->ApplyTransform();

	width = m_currentVolume->GetBoundingBox().m_xMax - m_currentVolume->GetBoundingBox().m_xMin ;// m_meshBoundingBox.m_xMax - m_meshBoundingBox.m_xMin;
	height = m_currentVolume->GetBoundingBox().m_yMax - m_currentVolume->GetBoundingBox().m_yMin ; //m_meshBoundingBox.m_yMax - m_meshBoundingBox.m_yMin;
	depth = m_currentVolume->GetBoundingBox().m_zMax - m_currentVolume->GetBoundingBox().m_zMin ; //m_meshBoundingBox.m_zMax - m_meshBoundingBox.m_zMin;

	

	sizeMax = max(width, max(height, depth));

	glScalef(width/sizeMax, height/sizeMax, depth/sizeMax);

	glPolygonOffset(1.0, 1.0);

	glEnable(GL_POLYGON_OFFSET_FILL);
	
	DrawSlices();

	glDisable(GL_POLYGON_OFFSET_FILL);
	
	GLfloat colorAux;
	for (int i = 0 ; i < m_simpleMeasureVector.size() ; i++){

		if (m_simpleMeasureVector[i] == m_currentMeasureLayer && GetApplicationMode() == ADDINGPOINTSMODE)
		{			
			glLineWidth(3.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
			m_currentMeasureLayer->AddPoint(m_mouseX,m_mouseY);
			m_currentMeasureLayer->SaveObjectTransform();
			m_currentMeasureLayer->Draw();
			m_currentMeasureLayer->RemovePoint();

			glLineWidth(1.0f);
			
		}else 
		{

			glPointSize(4.0f);
			glColor3f(1.0f, 0.0f, 0.0f);
			//colorAux+=0.2;
			//glColor3f(colorAux, 1-colorAux, colorAux*2);
			m_simpleMeasureVector[i]->SaveObjectTransform();
			m_simpleMeasureVector[i]->DrawPoints();
		}
	}
	
	glColor3f(1.0f, 0.0f, 0.0f);

    glDisable(GL_DEPTH_TEST);
	glDisable(GL_ALPHA_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	m_preIntegrationTableTexture.Bind(0);

	glColor3f(1.0f, 1.0f, 1.0f);

	// draw texture with window center and length
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(1.0f, 0.5f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(0.5f, 0.5f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(0.5f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glEnd();

	CTexture::DisableTextureUnit(0);

	glBegin(GL_LINE_LOOP);

	glVertex2f(0.5f, 0.5f);
	glVertex2f(1.0f, 0.5f);
	glVertex2f(1.0f, 1.0f);
	glVertex2f(0.5f, 1.0f);

	glEnd();

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0, 100.0, 0.0, 100.0  );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glColor3ub( 0, 0, 0 );
	
	glRasterPos2i( 35, 2 );
	char stringrender[20]="total de voxels: ";

	for( int i=0; i<(int)strlen( stringrender ); i++ )
	  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, stringrender[i] );

	char string2render[10];

	itoa (volumeFigado, string2render,10);
	for( int i=0; i<(int)strlen( string2render ); i++ )
	  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string2render[i] );
	
	glRasterPos2i( 35, 6 );
	char string3render[20]="volume total: ";

	for( int i=0; i<(int)strlen( string3render ); i++ )
	  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string3render[i] );

	m_voxelsDoVolume = volumeFigado * (m_volumeDeUmVoxel*100.0);
	float volume_total =(float)volumeFigado * m_volumeDeUmVoxel;
	int vol_inteiro = m_voxelsDoVolume/1000;
	
	itoa (vol_inteiro, string2render,10);
	for( int i=0; i<(int)strlen( string2render ); i++ )
	  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string2render[i] );
	glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, ',' );
	vol_inteiro = m_voxelsDoVolume%1000;
	itoa (vol_inteiro, string2render,10);
	for( int i=0; i<(int)strlen( string2render ); i++ )
	  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string2render[i] );
	char string4render[20]="cm3 ";
	for( int i=0; i<(int)strlen( string4render ); i++ )
	  glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, string4render[i] );

	// Draw tweak bars
	TwDraw();

	glutSwapBuffers();
}

/**
*/
void OnSize(GLsizei w, GLsizei h)
{
    if (h == 0)
        h = 1;
    
    glViewport(0, 0, w, h);

    m_camera->SetViewport(0, 0, w, h);

	TwWindowSize(w, h); //tweak bar
}

/**
*/
void OnIdle(void)
{
    glutPostRedisplay();
}

void OnKeyDown(unsigned char key, int x, int y) 
{
	if (!TwEventKeyboardGLUT(key, x, y))
	{
		switch (key)
		{
		case 'w':
		case 'W':
			m_windowCenter += 5;
			
			if (m_windowCenter > MAXIMUM_VALUE)
				m_windowCenter = MAXIMUM_VALUE;

			InitializeLookUpTable(m_windowCenter, m_windowWidth);
			
			createPreintegrationTable(m_lookUpTable);
			break;
		case 's':
		case 'S':
			m_windowCenter -= 5;
			
			if (m_windowCenter < MINIMUM_VALUE)
				m_windowCenter = MINIMUM_VALUE;

			InitializeLookUpTable(m_windowCenter, m_windowWidth);
			
			createPreintegrationTable(m_lookUpTable);
			break;
		case 'a':
		case 'A':
			m_windowWidth -= 2;
			
			if (m_windowWidth < 0)
				m_windowWidth = 0;

			InitializeLookUpTable(m_windowCenter, m_windowWidth);
			
			createPreintegrationTable(m_lookUpTable);
			break;
		case 'd':
		case 'D':
			m_windowWidth += 2;
			
			InitializeLookUpTable(m_windowCenter, m_windowWidth);
			
			createPreintegrationTable(m_lookUpTable);
			break;
		case 'o':
		case 'O':
			m_alphaThreshold += 0.025f;

			if (m_alphaThreshold > 1.0f) {
				m_alphaThreshold = 1.0f;
			}

			cout << "Alpha threshold level: " << m_alphaThreshold << endl;
			break;
		case '=':
			float t1;
			t1= m_camera->GetNearPlaneDepth();
			if(t1 <=2)
				m_camera->SetNearPlaneDepth(t1+0.1);
			t1= m_camera->GetNearPlaneDepth();
			cout << "NPlane:" << t1<<endl;
			break;
		case '-':
			float t2;
			t2= m_camera->GetNearPlaneDepth();
			if(t2 >= 0.2)
				m_camera->SetNearPlaneDepth(t2-0.1);
			t2= m_camera->GetNearPlaneDepth();
			cout << "NPlane:" << t2<<endl;
			break;
		case 'l':
		case 'L':
			m_alphaThreshold -= 0.025f;

			if (m_alphaThreshold < 0.0f) {
				m_alphaThreshold = 0.0f;
			}

			cout << "Alpha threshold level: " << m_alphaThreshold << endl;
			break;
		case 27:
			OnDestroy();
	        
			exit(EXIT_SUCCESS);
			break;
		}
	}
    
    glutPostRedisplay();
}
vector<vector<int>> m_tagedVoxelArray;
float getClosest(int pos[3])
{/*
	int closestIndex=0;
	float closestHipotenusa= sqrt(float((pos[0]-m_tagedVoxelArray[0][0])*(pos[0]-m_tagedVoxelArray[0][0])+(pos[1]-m_tagedVoxelArray[0][1])*(pos[1]-m_tagedVoxelArray[0][1])));
	closestHipotenusa = sqrt(float((closestHipotenusa*closestHipotenusa)+(pos[2]-m_tagedVoxelArray[0][2])*(pos[2]-m_tagedVoxelArray[0][2])));
	for (int i = 0 ; i < m_tagedVoxelArray.size() ; i++)
	{
		//for (int i = 0 ; i < m_tagedVoxelArray.size() ; i++)
		//{
		float hipotenusa = sqrt(float((pos[0]-m_tagedVoxelArray[i][0])*(pos[0]-m_tagedVoxelArray[i][0])+(pos[1]-m_tagedVoxelArray[i][1])*(pos[1]-m_tagedVoxelArray[i][1])));
		hipotenusa = sqrt(float((hipotenusa*hipotenusa)+(pos[2]-m_tagedVoxelArray[i][2])*(pos[2]-m_tagedVoxelArray[i][2])));
		if (hipotenusa<closestHipotenusa)
		{	
			closestHipotenusa=hipotenusa;
			closestIndex=i;
		}
	}
	return (float)closestIndex/10;*/
	int closestIndex=0;

	if (m_simpleMeasureVector.empty())
		return 0;

//	std::vector<int>	aux = m_simpleMeasureVector[0];
boost::shared_ptr<CSimpleMeasure> auxMeasure = m_simpleMeasureVector[0];
//std::vector<std::vector<int>>	aux = auxMeasure->voxelsVolume;
	if (auxMeasure->voxelsVolume.empty())
		return 0;

	float closestHipotenusa= (float((pos[0]-auxMeasure->voxelsVolume[0][0])*(pos[0]-auxMeasure->voxelsVolume[0][0])+(pos[1]-auxMeasure->voxelsVolume[0][1])*(pos[1]-auxMeasure->voxelsVolume[0][1])));
	closestHipotenusa = (float((closestHipotenusa)+(pos[2]-auxMeasure->voxelsVolume[0][2])*(pos[2]-auxMeasure->voxelsVolume[0][2])));

	for (int i = 0 ; i < m_simpleMeasureVector.size() ; i++){
		auxMeasure = m_simpleMeasureVector[i];
	//	aux = auxMeasure->voxelsVolume;
		for (int j = 0 ; j < auxMeasure->voxelsVolume.size() ; j++){
			float hipotenusa= (float((pos[0]-auxMeasure->voxelsVolume[j][0])*(pos[0]-auxMeasure->voxelsVolume[j][0])+(pos[1]-auxMeasure->voxelsVolume[j][1])*(pos[1]-auxMeasure->voxelsVolume[j][1])));
			hipotenusa = (float((hipotenusa)+(pos[2]-auxMeasure->voxelsVolume[j][2])*(pos[2]-auxMeasure->voxelsVolume[j][2])));
			if (hipotenusa<closestHipotenusa)
			{	
				closestHipotenusa=hipotenusa;
				closestIndex=i;
			}
		}

		//if (m_simpleMeasureVector[i] == m_currentMeasureLayer)
		//{
		//	if (i < m_simpleMeasureVector.size()-1)
		//		i++;
			//m_currentLayer = i;
		//	m_currentMeasureLayer = (m_simpleMeasureVector[i]);
		//	return true;
		//}
	}
/*	int closestIndex=0;
	float closestHipotenusa= sqrt(float((pos[0]-m_tagedVoxelArray[0][0])*(pos[0]-m_tagedVoxelArray[0][0])+(pos[1]-m_tagedVoxelArray[0][1])*(pos[1]-m_tagedVoxelArray[0][1])));
	closestHipotenusa = sqrt(float((closestHipotenusa*closestHipotenusa)+(pos[2]-m_tagedVoxelArray[0][2])*(pos[2]-m_tagedVoxelArray[0][2])));
	for (int i = 0 ; i < m_tagedVoxelArray.size() ; i++)
	{
		//for (int i = 0 ; i < m_tagedVoxelArray.size() ; i++)
		//{
		float hipotenusa = sqrt(float((pos[0]-m_tagedVoxelArray[i][0])*(pos[0]-m_tagedVoxelArray[i][0])+(pos[1]-m_tagedVoxelArray[i][1])*(pos[1]-m_tagedVoxelArray[i][1])));
		hipotenusa = sqrt(float((hipotenusa*hipotenusa)+(pos[2]-m_tagedVoxelArray[i][2])*(pos[2]-m_tagedVoxelArray[i][2])));
		if (hipotenusa<closestHipotenusa)
		{	
			closestHipotenusa=hipotenusa;
			closestIndex=i;
		}
	}
*/
//	cout << closestIndex << endl;
	return (float)closestIndex/10+0.1;

}

void calcEsqDir(){
m_voxelsLoboDireito=0;
m_voxelsLoboEsquerdo=0;
float *volumeVor = m_voronoiVolume->GetData();

	for (int k = 0 ; k < m_depth ; k++)
	{
		for (int j = 0 ; j < m_height ; j++)
		{
			for (int i = 0 ; i < m_width ; i++)
			{

				int pos3d[3];
				pos3d[2]= k;
				pos3d[0]= i;
				pos3d[1]= j;

				if (volumeVor[i+j*m_width+(k*m_width*m_height)]>0.05f && volumeVor[i+j*m_width+(k*m_width*m_height)]<0.15f )
					m_voxelsLoboDireito++;
				else if(volumeVor[i+j*m_width+(k*m_width*m_height)]>0.15f && volumeVor[i+j*m_width+(k*m_width*m_height)]<0.25f )
					m_voxelsLoboEsquerdo++;
			}
		}
	}
}

bool calcVoronoi()
{
	CTimer timer;
	boost::shared_ptr<Volume> volumeAux;
	 m_volumeArray->GetVolume(1,volumeAux);
	float *volume =	volumeAux->GetData();
	float *volume2 = m_voronoiVolume->GetData();
	for (int k = 0 ; k < m_depth ; k++)
	{
		for (int j = 0 ; j < m_height ; j++)
		{
			for (int i = 0 ; i < m_width ; i++)
			{

				int pos3d[3];
				pos3d[2]= k;
				pos3d[0]= i;
				pos3d[1]= j;

				if (volume[i+j*m_width+(k*m_width*m_height)]>0.3f)
					volume2[i+j*m_width+(k*m_width*m_height)]=getClosest(pos3d);
			}
		}
	}
	//calcEsqDir();
	m_voronoiVolume->SetTexture();
	m_voronoiVolume->GetCTexture(m_voronoiVolumeTexture);

	cout << "Voronoi elapsed time: " << timer.GetElapsed() << endl;

	return true;
}
/**
*/
void OnMouse(int button, int state, int x, int y) 
{
	if (!TwEventMouseButtonGLUT(button, state, x, y))//process tweak bar interaction
	{
		if (state == GLUT_DOWN) 
		{
			switch (GetApplicationMode())
			{

			case SELECTINGVOXEL:
				if (button == GLUT_LEFT_BUTTON)
				{
					float xyz[3];
					m_currentMeasureLayer->GetCandidatePoint(x, y, xyz);
					m_selectedVoxel[0]=m_width*(xyz[0]+0.5);
					m_selectedVoxel[1]=m_height*(xyz[1]+0.5);
					m_selectedVoxel[2]=m_depth*(xyz[2]+0.5);
					m_selectedVoxel[0]=m_width - m_selectedVoxel[0];
					vector<int> temp;
					temp.push_back(m_selectedVoxel[0]);
					temp.push_back(m_selectedVoxel[1]);
					temp.push_back(m_selectedVoxel[2]);

					m_tagedVoxelArray.push_back(temp);

					calcVoronoi();


				}
			case ADDINGPOINTSMODE:
				if (button == GLUT_LEFT_BUTTON)
				{
					float xyz[3];
					m_currentMeasureLayer->AddPoint(x, y);
					m_currentMeasureLayer->GetCandidatePoint(x, y, xyz);
					m_selectedVoxel[0]=m_width*(xyz[0]+0.5);
					m_selectedVoxel[1]=m_height*(xyz[1]+0.5);
					m_selectedVoxel[2]=m_depth*(xyz[2]+0.5);
					m_selectedVoxel[0]=m_width - m_selectedVoxel[0];

					vector<int> temp;
					temp.push_back(m_selectedVoxel[0]);
					temp.push_back(m_selectedVoxel[1]);
					temp.push_back(m_selectedVoxel[2]);

					if (temp[0]>=0 && temp[0]<=m_width && temp[1]>=0 && temp[1]<=m_height && temp[2]>=0 && temp[2]<=m_depth)
						m_currentMeasureLayer->voxelsVolume.push_back(temp);

					calcVoronoi();

				} else if (button == GLUT_RIGHT_BUTTON)
				{
					m_currentMeasureLayer->RemovePoint();
				}
			break;
			case MOVINGBODYMODE:
				m_mouseButton = button;
			break;
			case LBUTTON2WINDOW:
				m_mouseButton = button;
			break;
			}
		} else {
			m_mouseButton = -1;
		}

		m_mouseX = x;
		m_mouseY = y;
	}

	glutPostRedisplay();
}



/**
*/
void OnMouseMove(int x, int y)
{
	if (!TwEventMouseMotionGLUT(x, y))//process tweak bar interaction
	{
		switch (m_mouseButton)
		{
		case GLUT_LEFT_BUTTON:
			switch (GetApplicationMode())
			{
			case LBUTTON2WINDOW:
				m_windowCenter += 0.1f*(x - m_mouseX);
				m_windowWidth += 0.1f*(y - m_mouseY);	

				if (m_windowCenter > MAXIMUM_VALUE)
					m_windowCenter = MAXIMUM_VALUE;
				
				if (m_windowCenter < MINIMUM_VALUE)
					m_windowCenter = MINIMUM_VALUE;

				if (m_windowWidth < 0)
				m_windowWidth = 0;

				InitializeLookUpTable(m_windowCenter, m_windowWidth);
				
				createPreintegrationTable(m_lookUpTable);
				break;
			case MOVINGBODYMODE:
				m_camera->Pitch(0.1f*(y - m_mouseY));
				m_camera->Yaw(0.1f*(x - m_mouseX));
			break;
			}
			break;
		case GLUT_MIDDLE_BUTTON:

			m_camera->MoveSide(0.025*(x - m_mouseX));
			m_camera->MoveUp(0.025f*(m_mouseY - y));
			break;

		case GLUT_RIGHT_BUTTON:
			m_camera->MoveFront(0.01f*(y - m_mouseY));
			break;
		}

		m_mouseX = x;
		m_mouseY = y;
	}

    glutPostRedisplay();
}


void OnPassiveMouseMove(int x, int y)
{
	if( !TwEventMouseMotionGLUT(x, y) ) //process tweak bar interaction
	{
		m_mouseX = x;
		m_mouseY = y;
	}

    glutPostRedisplay();
}
/**
 *
 */

ApplicationModeEnum GetApplicationMode(void)
{
	return m_applicationMode;
}


/**
 *
 */


bool SetApplicationMode(ApplicationModeEnum applicationMode)
{
	switch (applicationMode)
	{
	case SELECTINGVOXEL:
		m_applicationMode = SELECTINGVOXEL;
		break;
	case ADDINGPOINTSMODE:
		//AddMeasureLayer();
		m_applicationMode = ADDINGPOINTSMODE;
		break;
	case MOVINGBODYMODE:
		m_applicationMode = MOVINGBODYMODE;
		break;
	case LBUTTON2WINDOW:
		m_applicationMode = LBUTTON2WINDOW;
		break;
		
	default:
		MarkError();

		return false;
	}
	return true;
}

/**
*/

void setMode( int control )
{
    switch (control)
    {

	case LOAD_SEGMENTS_ID:
		onLoad();
		break;
	case SAVE_DENSITY_ID:
		onSave("vasos");
		break;
	case SAVE_SEGMENTS_ID:
		onSave("separadas");
		break;
	case SAVE_SEGMENTS_RAW_ID:
		onSave("raw");
		break;
	case SELECT_VOXEL_ID:
		if (!SetApplicationMode(SELECTINGVOXEL))
		{
			MarkError();
			OnDestroy();
		}
		break;		
	case MOVING_BODY_ID:
		if (!SetApplicationMode(MOVINGBODYMODE))
		{
			MarkError();
			OnDestroy();
		}
		break;
	case ADDING_POINTS_ID:
		if (!SetApplicationMode(ADDINGPOINTSMODE))
		{
			MarkError();
			OnDestroy();
		}
		break;
	case REGULAGEM_JANELA_ID:
		if (!SetApplicationMode(LBUTTON2WINDOW))
		{
			MarkError();
			OnDestroy();
		}
		break;
	case NEW_LAYER_ID:
		//m_currentLayer++;
		AddMeasureLayer();
		break;
	case LAYER_FORWARD_ID:
		measureLayerForward();
		break;
		case VORONOI_ALPHA_ID:
		createPreintegrationTable(m_lookUpTable);
		break;	
	case FLOOD_FILL_ID:
		{
	/*		float newColor=2.0;
			float *aux = m_volumeSegmentado.getData();
			if (m_selectedVoxel[0]>=0 && m_selectedVoxel[0]<m_width && 
				m_selectedVoxel[1]>=0 && m_selectedVoxel[1]<m_height && 
				m_selectedVoxel[2]>=0 && m_selectedVoxel[2]<m_depth)
			{
				float oldColor = aux[m_width*m_height*m_selectedVoxel[2]+
					m_width*m_selectedVoxel[1]+m_selectedVoxel[0]];
				floodFillWithColorRange(m_selectedVoxel[0], m_selectedVoxel[1], 
					m_selectedVoxel[2], newColor, oldColor, m_floodFillRange, 
					aux);
			}

			if (!m_volumeTexture->SetImage(m_width, m_height, m_depth, 0, CTexture::LUMINANCE, CTexture::FLOAT, m_volumeSegmentado.getData()))
				MarkError();
*/
		}
		break;
	case LAYER_BACKWARD_ID:
		measureLayerBackward();
		break;
	case VOLUME_BACKWARD_ID:
		if (m_volumeArray->GetVolume(m_currentID-1,m_currentVolume))
			m_currentID--;
		else 
		m_currentVolume->GetCTexture(m_volumeTexture);
		std::cout << m_currentID << std::endl;
		break;
	case VOLUME_FORWARD_ID:
		if (m_volumeArray->GetVolume(m_currentID+1,m_currentVolume))
			m_currentID++;
		m_currentVolume->GetCTexture(m_volumeTexture);
		std::cout << m_currentID << std::endl;
		break;
	case 4:
		{
			switch(obj){
			case SHOW_DATASET_ID:
			//	if (!m_volumeTexture->SetImage(m_width, m_height, m_depth, 0, CTexture::LUMINANCE, CTexture::FLOAT, m_volumeData.getData()))
			//		MarkError();
				std::cout << obj << std::endl;
				break;
			case SHOW_MOLDE_ID:
			//	if (!m_volumeTexture->SetImage(m_width, m_height, m_depth, 0, CTexture::LUMINANCE, CTexture::FLOAT, m_volumeMolde.getData()))
			//		MarkError();
				break;
			case SHOW_SEGMENTADO_ID:
			//	if (!m_volumeTexture->SetImage(m_width, m_height, m_depth, 0, CTexture::LUMINANCE, CTexture::FLOAT, m_volumeSegmentado.getData()))
			//		MarkError();
				break;
			}
		}
		break;
	case OPEN_ID:
		Open();
		break;
	}
}

/************************************  
	Ant Tweak Bar Variables 
 ************************************/

TwBar *bar;	 // Pointer to a tweak bar
float barColor[] = { 0, 0, 0};
TwBar *OpenVolumeWindow;
std::string m_fileName;
int dimension = 256 , slices = 4;
int line_ant=0, line=0;
int volume_type=0;

/*********************** 
   Ant Tweak Bar Methods  
 ***********************/

void TweakBarCreate();

void TW_CALL OkCB(void* client)
{
	Open();
	TwDeleteBar(OpenVolumeWindow);
}

void OpenMenuCreate()
{
	OpenVolumeWindow = TwNewBar("Volume Options");

	char largura[255]; itoa(glutGet(GLUT_WINDOW_WIDTH)/2-100, largura, 10);
	char altura[255]; itoa(glutGet(GLUT_WINDOW_HEIGHT)/2-100, altura, 10);

	string definition = " 'Volume Options' size='250 100' position='";
	definition.append(largura);
	definition.append(" ");
	definition.append(altura);
	definition.append("'");
	TwDefine(definition.c_str());

	TwAddVarRW(OpenVolumeWindow, "dimension", TW_TYPE_UINT16, &dimension, " ");
	TwAddVarRW(OpenVolumeWindow, "slices", TW_TYPE_UINT16, &slices, " ");

	TwAddButton(OpenVolumeWindow, "open_ok", OkCB, NULL , 
                " label='ok' ");
}

void TW_CALL SetModeCB(void* client)
{
	int mode = *(int*)client;

	if (mode == OPEN_ID)
		setMode(OPEN_ID);
	else
		if (VolumeIsLoaded)			
			setMode(mode);
}

void TW_CALL ExitCB(void* client)
{
	//deseja salvar?
	OnDestroy();
	exit(EXIT_SUCCESS);
}

void TW_CALL setVoronoi(const void *value, void *clientData)
{ 
    m_voronoiAlpha = *(const int *)value;
	setMode(VORONOI_ALPHA_ID); 
}

void TW_CALL getVoronoi(void *value, void *clientData)
{ 
    *(int *)value = m_voronoiAlpha;
}

void TW_CALL setVolumeType(const void *value, void *clientData)
{
	if (VolumeIsLoaded){
		if (volume_type < *(const int *)value)		
			setMode(VOLUME_FORWARD_ID);
		else setMode(VOLUME_BACKWARD_ID);
	}

	volume_type = m_currentID;
}

void TW_CALL getVolumeType(void *value, void *clientData)
{ 
    *(int *)value = volume_type;
}

void TW_CALL ColorDefine(void* client)
{
	string definition;
	char buff[255];
	definition = " LiverSegments color='";
	definition.append(itoa((int)(barColor[0]*255), buff, 10));
	definition.append(" ");
	definition.append(itoa((int)(barColor[1]*255), buff, 10));
	definition.append(" ");
	definition.append(itoa((int)(barColor[2]*255), buff, 10));
	definition.append("'");
	TwDefine(definition.c_str());
}

void TweakBarCreate()
{	
	// Create a tweak bar for menu
    bar = TwNewBar("LiverSegments");
	string definition =" LiverSegments label='LiverSegments' fontSize=3 position='0 0' size='250 600' valuesWidth=75 ";	
	TwDefine(definition.c_str());
	ColorDefine(NULL); //set default color
	TwWindowSize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT)); 	
    TwDefine(" GLOBAL help='Help' "); // Message added to the help bar.

	// VOLUME -------------------------------------------------------------------------------

	int* open = (int*)malloc(sizeof(int));
	*open = OPEN_ID;
	TwAddButton(bar, "open", SetModeCB, open, 
                " group='Volume' label='Open' ");

	TwAddVarCB(bar, "selectVolume", TW_TYPE_UINT16, setVolumeType, getVolumeType, NULL, 
				" group='Volume' label='Select Volume' "); //min=0

	TwAddSeparator(bar, "", NULL);

	// MODE ---------------------------------------------------------------------------------

	int* move = (int*)malloc(sizeof(int));
	*move = MOVING_BODY_ID;
	TwAddButton(bar, "pan", SetModeCB, move, 
                " group='Mode' label='Pan' ");

	int* window = (int*)malloc(sizeof(int));
	*window = REGULAGEM_JANELA_ID;
	TwAddButton(bar, "window", SetModeCB, window, 
                " group='Mode' label='Window' ");

	TwAddSeparator(bar, "", NULL);

	// SEGMENTS ------------------------------------------------------------------------------

	int* add = (int*)malloc(sizeof(int));
	*add = ADDING_POINTS_ID;
	TwAddButton(bar, "addpoint", SetModeCB, add, 
                " group='Segments' label='Add Point' ");

	int* newLayer = (int*)malloc(sizeof(int));
	*newLayer = NEW_LAYER_ID;
	TwAddButton(bar, "newline", SetModeCB, newLayer, 
                " group='Segments' label='New Line' ");

	TwAddButton(bar, "blank1", NULL, NULL, " group='Segments' label=' ' ");

	int* layerForward = (int*)malloc(sizeof(int));
	*layerForward = LAYER_FORWARD_ID;
	TwAddButton(bar, "nextLine", SetModeCB, layerForward, 
				" group='Segments' label='Next Line' ");

	int* layerBackward = (int*)malloc(sizeof(int));
	*layerBackward = LAYER_BACKWARD_ID;
	TwAddButton(bar, "previousLine", SetModeCB, layerBackward, 
				" group='Segments' label='Previous Line' "); 

	TwAddSeparator(bar, "", NULL);

	// VORONOI/TRANSPARENCY -----------------------------------------------------------------
	
	TwAddVarCB(bar, "voronoi", TW_TYPE_UINT16,
		setVoronoi, getVoronoi, NULL,
                " group='Voronoi/Transparency' label='Alpha Channel'  min=0 max=255 ");

	TwAddSeparator(bar, "", NULL);

	// FILE ---------------------------------------------------------------------------------

	int* saveIm2D = (int*)malloc(sizeof(int));
	*saveIm2D = SAVE_DENSITY_ID;
	TwAddButton(bar, "saveVases", SetModeCB, saveIm2D , 
                " group='File' label='Save Vessels Imgs 2D' ");

	int* saveSeg2D = (int*)malloc(sizeof(int));
	*saveSeg2D = SAVE_SEGMENTS_ID;
	TwAddButton(bar, "saveSegm2D", SetModeCB, saveSeg2D, 
                " group='File' label='Save Segm Imgs 2D' ");

	int* saveSegRaw = (int*)malloc(sizeof(int));
	*saveSegRaw = SAVE_SEGMENTS_RAW_ID;
	TwAddButton(bar, "saveSegm3D", SetModeCB, saveSegRaw, 
                " group='File' label='Save Segm Img 3D' ");

	int* loadSeg = (int*)malloc(sizeof(int));
	*loadSeg = LOAD_SEGMENTS_ID;
	TwAddButton(bar, "loadSegm3D", SetModeCB, loadSeg, 
                " group='File' label='Load Segm Img 3D' ");
	
	TwAddSeparator(bar, "", NULL);

	// OPTIONS -----------------------------------------------------------------------------

	TwAddVarRW(bar, "Color", TW_TYPE_COLOR3F, &barColor, " group='Options' ");
	TwAddButton(bar, "colorb", ColorDefine, NULL, 
                " label='OK' group='Color' ");
	
	TwAddSeparator(bar, "", NULL);

	// EXIT ------------------------------------------------------------------------------------

	TwAddButton(bar, "exit", ExitCB, NULL, 
                " label='Exit' ");

}

/************************************************************************************************
 * The initialization function.
 * Initializes GLUT, GLEW and Ant Tweak Bar
 ************************************************************************************************/
bool OnCreate(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowPosition( 50, 50 );
	glutInitWindowSize(800, 600);

	int main_window = glutCreateWindow( "LiverSegments" );

 	if (glewInit() != GLEW_OK)
	{  glutPostRedisplay();
		MarkError();

		return false;
	}


	m_camera.reset(new CPinholeCamera);

	if (!m_camera.get())
	{
		MarkError();

		return false;
	}

	m_camera->Create(45.0f, 1.0f, 50.0f, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

	m_camera->ResetViewMatrix();

    m_camera->MoveFront(-2.0f);
    m_camera->ApplyTransform(); 

	glutDisplayFunc(OnPaint);
    glutKeyboardFunc(OnKeyDown);
    glutMouseFunc(OnMouse);
    glutMotionFunc(OnMouseMove);
	glutPassiveMotionFunc(OnPassiveMouseMove);
    glutReshapeFunc(OnSize);

	if( !TwInit(TW_OPENGL, NULL) )
    {
        // A fatal error occured    
        fprintf(stderr, "AntTweakBar initialization failed: %s\n", TwGetLastError());
        return false;
    }

	TweakBarCreate(); //menu bar	

	AddMeasureLayer();

	return true;
}

void onSave(string tipoDeArquivo){

	//windows method call to save file
	OPENFILENAME ofn ;	
	char szFile[255] ;

	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = NULL ;
	ofn.lpstrFile = szFile ;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof( szFile );
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL ;
	ofn.nMaxFileTitle = 0 ;
	ofn.lpstrInitialDir = NULL ;
	ofn.Flags = OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST;

	if (!GetSaveFileName(&ofn)) return;		
	
	int res = m_height*m_width;

	short **imageSegmentada;
	//alloc memoria para imageSegmentada
	imageSegmentada = (short**)malloc(m_depth * sizeof(short*));
	for (int i=0; i < m_depth; i++)
		imageSegmentada[i] = (short*)malloc(sizeof(short) * (res));
	//preenche com algum valor
	for( int k = 0; k < m_depth; k++ ){
		for( int j = 0; j < res; j++ ){
			imageSegmentada[k][j] = 0;
		}
	}

	if (tipoDeArquivo=="vasos"){
	float *volumeAtual = m_currentVolume->GetData();
		for (int k = 0 ; k < m_depth ; k++)
		{
			for (int i = 0 ; i < res ; i++)
			{
					if (volumeAtual[i+(k*res)]>0.51f)
						imageSegmentada[k][i]=255;
			}
		}
	tipoDeArquivo="separadas";
//	tipoDeArquivo="raw";
	}else{

		//atribui valores de acordo com o segmento
		float *volumeVor = m_voronoiVolume->GetData();
		for (int k = 0 ; k < m_depth ; k++)
		{
			for (int i = 0 ; i < m_height ; i++)
			{
				for (int j = 0 ; j < m_width ; j++)
				{
					if (volumeVor[i+j*m_width+(k*m_width*m_height)]<1.0f)
						imageSegmentada[k][i+j*m_width]=3071 * volumeVor[i+j*m_width+(k*m_width*m_height)];
					else
						imageSegmentada[k][i+j*m_width]=3071;
				}
			}
		}
	}


	string saveName(szFile);

	if (tipoDeArquivo == "raw"){
		FILE* shortFile;
		saveName.append(".raw");
		if( shortFile = fopen( saveName.c_str(), "wb+" ) ){
			for (int k=0 ; k < m_depth ; k++){
				for (int j=0; j<m_width;j++){
					for (int i=0 ; i < m_height ; i++){
						fwrite( &imageSegmentada[k][(m_width*j+i)], 1, sizeof(short), shortFile );					
					}
				}
			}
		}
		fclose(shortFile);
		
	}
	else{
		for (int k=0 ; k < m_depth ; k++){
			FILE* shortFile;
			char formato[5];
			itoa (k, formato,10);
			saveName.append(".");
			saveName.append(formato);
			if( shortFile = fopen( saveName.c_str(), "wb+" ) ){
				for (int j=0; j<m_width;j++){
					for (int i=0 ; i < m_height ; i++){
						fwrite( &imageSegmentada[k][(m_width*j+i)], 1, sizeof(short), shortFile );					
					}
				}
			}
			fclose(shortFile);
		}
	}
}

void onLoad(){

	
	if (m_voronoiVolume){
		float *volumeVor = m_voronoiVolume->GetData();
		FILE* shortFile;
		char szFile[255] ;
		char type[255] = "RAW\0*.RAW\0All\0*.*\0";
		int res = m_height*m_width;

		if (!OpenFileWindow(szFile,type)) return;	

		std::string file2loadRaw(szFile);
		if( shortFile = fopen( file2loadRaw.c_str(), "rb" )  ){
			// read file into image matrix
			for( int k = 0; k < m_depth; k++ ){
				for( int i = 0; i < res; i++ ){
					short value;
					fread( &value, 1, sizeof(short), shortFile );
					if (value == 20)
						volumeVor[i+k*res]=0.2f;
					else if (value == 40)
						volumeVor[i+k*res]=0.3f;
					else if (value == 60)
						volumeVor[i+k*res]=0.4f;
					else if (value == 80)
						volumeVor[i+k*res]=0.5f;
					else if (value == 100)
						volumeVor[i+k*res]=0.6f;
					else if (value == 120)
						volumeVor[i+k*res]=0.7f;
					else if (value == 140)
						volumeVor[i+k*res]=0.8f;
					else if (value == 160)
						volumeVor[i+k*res]=0.9f;
					else if (value == 180)
						volumeVor[i+k*res]=1.0f;
					else if (value >= 200)
						volumeVor[i+k*res]=1.0f;
					
				}
			}
			m_voronoiVolume->SetTexture();
			m_voronoiVolume->GetCTexture(m_voronoiVolumeTexture);
		}else printf( "ERROR: Could not open input file %s .\n", szFile );
	}else printf( "ERROR: No volume loaded.\n");


//	return true;
}


/**
*/
bool LoadVolume(char* fileName)
{
//init volume array in the first time
	if (!m_volumeArray.get())
	{
		m_volumeArray.reset(new VolumeArray);

		if (!m_volumeArray.get())
		{
			MarkError();

			return false;
		}
	}

	m_currentVolume.reset(new Volume);

	if (!m_currentVolume.get())
	{
		MarkError();

		return false;
	}
	if (!fileName==NULL)
	{
		if (!m_currentVolume->LoadImage(fileName))
		{
			MarkError();

			OnDestroy();
		}
	}

	if (!m_currentVolume->CreateVolume(m_width, m_height, m_depth))
	{
		MarkError();

		OnDestroy();
	}

	m_currentVolume->GetCTexture(m_volumeTexture);

	m_volumeArray->AddVolume(m_currentVolume);

	return true;

}

bool LoadVoronoiVolume(char* fileName)
{

	m_voronoiVolume.reset(new Volume);

	if (!m_voronoiVolume.get())
	{
		MarkError();

		return false;
	}
	if (!fileName==NULL)
	{
		if (!m_voronoiVolume->LoadImage(fileName))
		{
			MarkError();

			OnDestroy();
		}
	}

	if (!m_voronoiVolume->CreateVolume(m_width, m_height, m_depth))
	{
		MarkError();

		OnDestroy();
	}

	m_voronoiVolume->GetCTexture(m_voronoiVolumeTexture);



	return true;

}


/**
*/
int main(int argc, char **argv)
{


		//cout << "TestMarchingCubes [source] (nhdr file) [width] [height] [depth] (sampling)" << endl;


    if (!OnCreate(argc, argv))
	{
		MarkError();

		OnDestroy();
	}

	if (!InitializeShaders())
	{
		MarkError();
		OnDestroy();
	}
    
    glutMainLoop();
    
    return EXIT_SUCCESS;
}