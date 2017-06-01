#ifndef VOLUME_ARRAY_INCLUDED
#define VOLUME_ARRAY_INCLUDED

#include <string>
#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

#include "Volume.h"

class VolumeArray
	: boost::noncopyable
{
public:

	VolumeArray(void){
		m_volumeArray.clear();
		contVolume=0;
	}
	//virtual ~VolumeArray(void);

	void Create(void)
	{
		m_volumeArray.clear();
	}

	bool AddVolume(boost::shared_ptr<Volume> volume)
	{
		if (!volume.get())
		{
			MarkError();

			return false;
		}

		m_volumeArray.push_back(volume);
		contVolume++;
		return true;		
	}

	bool GetVolume(int ID, boost::shared_ptr<Volume> & cArray){
		if (ID >=0 && ID<contVolume)
		{
			cArray = m_volumeArray[ID];
			return true;
		}
		return false;
	}

	bool GetVolumeArray(std::vector<boost::shared_ptr<Volume> >& cArray){
		cArray = m_volumeArray;
		return true;
	}


int RecortaVolume(float divisoriaDoVolume)
{
	float *molde = m_volumeArray[1]->GetData();
	float *resultado = m_volumeArray[2]->GetData();
	float *original = m_volumeArray[0]->GetData();
	int volume = 0;

	for (int i = 0; i < m_volumeArray[0]->m_width*m_volumeArray[0]->m_height*m_volumeArray[0]->m_depth ; i++){
		if (molde[i] > divisoriaDoVolume)
		{
			resultado[i] = original[i];
			volume++;
		}
		else{ 
			resultado[i] = 0;
		}
	}
	m_volumeArray[2]->SetTexture();
	return volume;
}
private:
	std::vector<boost::shared_ptr<Volume> > m_volumeArray;
	int contVolume;
};

#endif // VOLUME_ARRAY_INCLUDED

