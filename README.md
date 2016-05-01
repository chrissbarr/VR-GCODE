# VR GCODE Viewer
Virtual Reality viewer for 3D Printer GCODE files

## About
This is a program that takes sliced GCODE files for 3D printing and renders a preview that can be viewed using a VR headset.

The advantage of this over a regular GCODE viewer is that it makes it easy to interpret the scale of the printed object and its various features. Additionally, it is possible to look at the object from various angles in a natural way.

## Demo Videos
Updated Video:
<a href="http://www.youtube.com/watch?feature=player_embedded&v=D7j5xxwkkqo
" target="_blank"><img src="http://img.youtube.com/vi/D7j5xxwkkqo/0.jpg" 
alt="Demo Video 2" width="800" height="450" border="10" /></a>

First Video:

<a href="http://www.youtube.com/watch?feature=player_embedded&v=w0qat7VMZCM
" target="_blank"><img src="http://img.youtube.com/vi/w0qat7VMZCM/0.jpg" 
alt="Demo Video 1" width="800" height="450" border="10" /></a>

## How To
A compiled copy of the program can be downloaded [here](https://dl.dropboxusercontent.com/u/20412569/downloads/VRGCODE.zip). Otherwise, Unreal Engine 4.11 is required to compile and run the files. 

Upon opening, the program will scan the "GCODEViewer" directory and look for any .gcode files (up to 10 total). These files will then be displayed, and a file can be selected to preview.

Once selected, the program will begin to parse the GCODE file. The amount of time this will take depends on the complexity of the GCODE file - size is a relatively good indicator, and the relation between file size and loading time is roughly linear. An indicator shows the loading progress.

Once the file has been parsed, rendering will begin. The program builds the virtual model one extrusion at a time, much as an actual printer would. This can take some time, and again the time taken will depend on the complexity of the file.

## Controls

<img src="https://raw.githubusercontent.com/chrissbarr/VR-GCODE/master/XboxControlScheme.png" width="800">

Select File - Space, Xbox A

Move Faster - Shift, Xbox B

Toggle Travel Moves - M, Xbox X

Reset Model Rotation and Scale - Tab / Xbox Y

Load New Model - Del, Xbox Back

Move Forward / Back - W, S  Xbox Left Thumbstick Y

Move Left / Right - A, D, Xbox Left Thumbstick X

Move Up / Down - Q, E, Xbox Bumpers

Rotate View - Mouse X

Scale Model - R, F, Xbox Triggers

Rotate Model - Arrow Keys, Xbox Right Thumbstick

Adjust Camera Clipping Plane - T, H, Xbox D-Pad Up/Down

Layer Jog - Num +, Num -, Xbox D-Pad Left/Right

Pause Model Construction / Loading - P, Xbox Start

## Performance Notes
Graphical performance may be poor (low FPS) during initial loading and model construction. Once the model is fully constructed performance should increase, but complex / large models may still prove troublesome to work with. Rotating / scaling models also has a performance impact that is noticeable on larger models.

Rendering an entire GCODE file for preview in VR is a computationally intensive task, and a computer with a relatively powerful GPU is recommended. All tests were performed using a GTX970.

## Todo
* Vive motion controls for scale / rotate
* Option to render different features in different colours (infill, non-print moves, etc)
* Adjustable quality levels
* Improved file handling / selection

## License
This work is licensed under a [Creative Commons Attribution-NonCommercial 4.0 International License](http://creativecommons.org/licenses/by-nc/4.0/).
