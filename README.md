# VR GCODE Viewer
Virtual Reality viewer for 3D Printer GCODE files

## About
This is a program that takes sliced GCODE files for 3D printing and renders a preview that can be viewed using a VR headset.

The advantage of this over a regular GCODE viewer is that it makes it easy to interpret the scale of the printed object and its various features. Additionally, it is possible to look at the object from various angles in a natural way.

## Demo
<a href="http://www.youtube.com/watch?feature=player_embedded&v=w0qat7VMZCM
" target="_blank"><img src="http://img.youtube.com/vi/w0qat7VMZCM/0.jpg" 
alt="IMAGE ALT TEXT HERE" width="640" height="360" border="10" /></a>

## How To
This program is currently in its very early stages (proof-of-concept), and there is no proper user interface for selecting files to preview. 

Upon opening, the program will scan its root directory (in which the .exe resides) and look for any .gcode files. These files will then be listed in the program window, along with a corresponding number. 

Files can be selected by pressing 1-5 after the program has started, where the number corresponds to the number given next to the file name. If there are more than five files in the folder, only the first five (alphanumerically) can be selected.

Once selected, the program will begin to parse the GCODE file. The amount of time this will take depends on the complexity of the GCODE file - size is a relatively good indicator, and the relation between file size and loading time is roughly linear. A percentage read-out shows the loading progress.

Once the file has been parsed, rendering will begin. The program builds the virtual model one extrusion at a time, much as an actual printer would. This can take some time, and again the time taken will depend on the complexity of the file.

Graphical performance may be poor (low FPS) during initial loading and model construction. Once the model is fully constructed performance should increase, but complex / large models may still prove troublesome to work with.


## Controls

Move Forward / Back - W, S  Xbox Right Thumbstick Y

Move Left / Right - A, D, Xbox Right Thumbstick X

Move Up / Down - Q, E, Xbox D-Pad Y

Rotate View - Mouse X, Xbox Bumpers


Scale Model - R, F, Xbox Triggers

Rotate Model - Arrow Keys, Xbox Left Thumbstick

Reset Model Rotation and Scale - Shift / Xbox Y


Adjust Camera Clipping Plane - +, -, Xbox D-Pad X

Reset HMD Tracking - F8, Xbox Back




An Xbox controller is recommended.


## Todo
* Vive motion controls for scale / rotate
* Option to render different features in different colours (infill, non-print moves, etc)
* Adjustable quality levels
* Improved file handling / selection
