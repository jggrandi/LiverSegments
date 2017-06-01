#ifndef VOLUME_INCLUDED
#define VOLUME_INCLUDED

#define MINIMUM_VALUE (-1024)
#define MAXIMUM_VALUE 3071

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <PinholeCamera.h>
#include <ErrorManager.h>
#include <GageAdaptor.h>
#include <VertexPool.h>
#include <BoundingBox.h>
#include <Texture.h>
#include <Shader.h>
#include <MyGLH.h>
#include <MyMath.h>
#include <Timer.h>


//#include "SimpleMeasure.h"



class Volume
	: boost::noncopyable
{
public:

	Volume(void);
	//~Volume(void);
	bool LoadImage(const std::string& path);
	void SetSize (int width, int height, int depth);
	bool CreateVolume();
	bool CreateVolume(int width, int height, int depth);
	bool SetBoundingBox(CBoundingBox meshBoundingBox);
	CBoundingBox GetBoundingBox();
	float *GetData();
	bool SetTexture();
	bool GetCTexture(boost::shared_ptr<CTexture>& cArray);

//protected:
	//virtual void Create(void);
public:
	int m_width;
	int m_height;
	int m_depth;
	std::vector<float> m_volumeTextureData;
	std::string m_imageName;
	bool m_hasImage;
	boost::shared_ptr<CGageAdaptor> m_image;
	boost::shared_ptr<CTexture> m_volumeTexture;
	CBoundingBox m_meshBoundingBox;
};

#endif // VOLUME_INCLUDED

