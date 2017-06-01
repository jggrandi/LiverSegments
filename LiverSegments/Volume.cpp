#include "Volume.h"


Volume::Volume(void){m_hasImage=0;};

bool Volume::LoadImage(const std::string& path)
{
	double kernelParam[3];

	// Scale parameter, in units of samples.
    kernelParam[0] = 1.0; 
	// B.
    kernelParam[1] = 0.0;
	// C.
    kernelParam[2] = 0.5;

	m_image.reset(new CGageAdaptor);

	if (!m_image.get())
	{
		MarkError();

		return false;
	}

	if (!m_image->Open(path))
	{
		MarkError();

		return false;
	}

	// BC family of cubic polynomial splines.
	if (!m_image->SetValueKernel(nrrdKernelBCCubic, kernelParam))
	{
		MarkError();

		return false;
	}

	// 1st deriv. of BC cubic family.
	if (!m_image->Set1stDerivativeKernel(nrrdKernelBCCubicD, kernelParam))
	{
		MarkError();

		return false;
	}

	if (!m_image->EnableQuery(CGageAdaptor::NORMAL))
	{
		MarkError();

		return false;
	}

	m_imageName = path;
	m_hasImage = 1;

	return true;
}

void Volume::SetSize (int width, int height, int depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
}

bool Volume::CreateVolume(int width, int height, int depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;
	if (!CreateVolume())
	{
		MarkError();
		return false;
	}
	return true;
}

bool Volume::CreateVolume()
{
	m_volumeTextureData.reserve(m_width*m_height*m_depth);

	int i,
		j,
		k;
	float value;

	if (m_hasImage){
		for (k=0; k<m_depth; ++k)
			for (i=0; i<m_height; ++i)
				for (j=0; j<m_width; ++j)
				{
					value = m_image->GetValue((float(j)/m_width)*m_image->GetWidth(), (float(i)/m_height)*m_image->GetHeight(), (float(k)/m_depth)*(m_image->GetDepth() - 1));

					m_volumeTextureData.push_back(value);
				}


		for (i=0; i<(m_width*m_height*m_depth); ++i)
		{
			m_volumeTextureData[i] = (m_volumeTextureData[i] - MINIMUM_VALUE)/(MAXIMUM_VALUE - MINIMUM_VALUE);

			if (m_volumeTextureData[i]< 0.0f)
				m_volumeTextureData[i] = 0.0f;
			if (m_volumeTextureData[i]>1.0f)
				m_volumeTextureData[i] = 1.0f;
		}
	}
	else {
		for (k=0; k<m_depth; ++k)
			for (i=0; i<m_height; ++i)
				for (j=0; j<m_width; ++j)
				{
					m_volumeTextureData.push_back(0);
				}	
	}

	m_volumeTexture.reset(new CTexture);

	if (!m_volumeTexture.get())
	{
		MarkError();

		return false;
	}

	if (!m_volumeTexture->Initialize(CTexture::TEXTURE_3D, CTexture::LUMINANCE))
	{
		MarkError();

		return false;
	}

	if (!m_volumeTexture->SetImage(m_width, m_height, m_depth, 0, CTexture::LUMINANCE, CTexture::FLOAT, &m_volumeTextureData[0]))
	{
		MarkError();

		return false;
	}

	m_volumeTexture->SetParameter(CTexture::MIN_FILTER, CTexture::LINEAR);
	m_volumeTexture->SetParameter(CTexture::MAG_FILTER, CTexture::LINEAR);
	m_volumeTexture->SetParameter(CTexture::WRAP_S, CTexture::CLAMP_TO_EDGE);
	m_volumeTexture->SetParameter(CTexture::WRAP_T, CTexture::CLAMP_TO_EDGE);

	m_meshBoundingBox.m_xMin = 0.0f;
	m_meshBoundingBox.m_xMax = 360.0f;
	m_meshBoundingBox.m_yMin = 0.0f;
	m_meshBoundingBox.m_yMax = 360.0f;
	m_meshBoundingBox.m_zMin = 0.0f;
	m_meshBoundingBox.m_zMax = 448.0f;

/*	if (!MyGLH::IsRenderingContextOk())
	{
		MarkError();

		return false;
	}
*/

	return true;
}

float *Volume::GetData()
{
	if (!m_volumeTextureData.empty())
		return &m_volumeTextureData[0];
}

bool Volume::SetBoundingBox(CBoundingBox meshBoundingBox)
{
	m_meshBoundingBox.m_xMax = meshBoundingBox.m_xMax;
	m_meshBoundingBox.m_xMin = meshBoundingBox.m_xMin;
	m_meshBoundingBox.m_yMax = meshBoundingBox.m_yMax;
	m_meshBoundingBox.m_yMin = meshBoundingBox.m_yMin;
	m_meshBoundingBox.m_zMax = meshBoundingBox.m_zMax;
	m_meshBoundingBox.m_zMin = meshBoundingBox.m_zMin;

	return true;
}
CBoundingBox Volume::GetBoundingBox()
{
	return m_meshBoundingBox;
}

bool Volume::SetTexture()
{
	if (!m_volumeTexture->SetImage(m_width, m_height, m_depth, 0, CTexture::LUMINANCE, CTexture::FLOAT, &m_volumeTextureData[0]))
	{
		MarkError();

		return false;
	}

}

bool Volume::GetCTexture(boost::shared_ptr<CTexture>& cArray){
	cArray = m_volumeTexture;
	return true;
}