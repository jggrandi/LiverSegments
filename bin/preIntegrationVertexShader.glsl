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

#version 110

uniform float SliceDistance;

void main()
{
	// compute the texture coordinate for sF
	gl_TexCoord[0] = gl_TextureMatrix[0]*gl_MultiTexCoord0;
	
	// transform view pos and view dir to obj space
	vec4 vPosition = vec4(0.0, 0.0, 0.0, 1.0);
	vPosition = gl_ModelViewMatrixInverse*vPosition;
	vec4 vDir = vec4(0.0, 0.0, -1.0, 1.0);
	vDir = normalize(gl_ModelViewMatrixInverse*vDir);
	
	// compute position of sB
	vec4 eyeToVert = normalize(gl_Vertex - vPosition);
	vec4 sB = gl_MultiTexCoord0 - eyeToVert*(SliceDistance/dot(vDir, eyeToVert));
	
	// compute the texture coordinate for sB
	gl_TexCoord[1] = gl_TextureMatrix[0]*sB;
	
	gl_Position = ftransform();		
}

