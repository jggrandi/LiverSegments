# LiverSegments

## GOAL: 
Tool to interactively classify regions of a volumetric model.
## INPUT: 
a .raw volume file with an .nhdr description file and a .raw volume file with the segmentation mask(generated with SmartContour) and the respective .nhdr description file

## OUTPUT: 
Visualization of the functional segments in different colors. The volume (in cm3) of the segmented organ.

## To run:
- try to run the .exe files in LiverSegments\bin

## To compile:
- there is a VS2010_project folder that contains the respective Visual Studio solution files
- open the *.sln file
- verify the includes and links in the project.
- new executable files will be created in <project_name>\bin

## Step-by-step for the included dataset (dataset 1)
1. run LiverSegments.exe
2. click Volume->Open
3. select the .nhdr file in folder dataset 1 and click open (it may take a few seconds to load)
4. A second file selector will apper. Select the mask .nhdr in folder dataset 1/mask and click open (it may take a few seconds to load). This process will load the segmented organ and the hole volume.
5. The option Select Volume changes the visualization to show only the segmented organ or the hole volume. Use the '+' and '-' to change (it may need to click more than once) and select only the segmented info.
6. Select Mode->Pan to pan(middle-drag), rotate (left-drag), zoom(right-drag) to ajust the position and orientation
7. Select Mode->Window to change the density window. Click in the view area and drag. It can be done with the 'A', 'D', 'W' and 'S' keys. The density values are displayed in the console window.
8. Add points by selecting the Add Point option. Click anywhere on the volume to add a point.
9. Click again in other position to create a line. The organ's color will change. You can keep clicking until you are done.
10. Click in new line and then in add point and repeat the steps 8 and 9 until you are done.
11. If it is necessary, you can navigate through the lines with the buttons Next and Previous Line to add more points in the desired Line.
12. Use the option Save Segm Img 3D to save the progress. To load use the option Load Segm Img 3D. Make sure that you have loaded the respective volume with the mask before that.

## Limitations and known bugs:
- Save Vessels Imgs 2D and Save Segm Imgs 2D are buggy
- Select volume button '-' is not working as planned

## OBS
We used Windows 7 64bit and MS Visual Studio 2010. 
A shader supported graphics card is necessary. We used an Nvidia Geforce 9600GT, but many other models should work fine. Binaries last tested in a Core i7 and nVidia GTX 1050


Good luck! :-)

