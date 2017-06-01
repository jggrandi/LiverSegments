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

uniform sampler3D Volume;
uniform sampler3D Voronoi;
uniform sampler2D PreIntegrationTable;
uniform sampler2D VoronoiColors;

void main (void)
{
	vec4 lookup;

	// sample front scalar
	lookup.x = texture3D(Volume, gl_TexCoord[0].xyz).x;
	// sample back scalar
	lookup.y = texture3D(Volume, gl_TexCoord[1].xyz).x;

	vec4 lookup2;
	// sample front scalar
	lookup2.x = texture3D(Voronoi, gl_TexCoord[0].xyz).x;
	// sample back scalar
	lookup2.y = texture3D(Voronoi, gl_TexCoord[1].xyz).x;
	//lookup2.z = texture3D(Voronoi, gl_TexCoord[2].xyz).x;
	
	// loookup and return pre-integrated value
	gl_FragColor = (texture2D(PreIntegrationTable, lookup.xy) + texture2D(VoronoiColors, lookup2.xy))/2;
}

