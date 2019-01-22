# Vanishing-point-detector

Open-spec University project for computer vision. This program will:
- convert an image to greyscale
- Canny edge detector to find edges
- Hough lines function to find vanishing lines
- calculate equation of lines and line crossing point

The included report and presentation go into more detail and showcase results. Originally written with C++ in Visual Studio, using OpenCV 3.4.0.

Source also includes several fail conditions to show limits of current approach.
Current approach works on idea of an image containing a single vanishing point. However, a better theory may be to consider every 3d object in the image as a set of 2d planes with four vanishing points.
Better detection system (such as optic flow) could identify every 2d plane and find all four associated vanishing points.
For all 2d planes orthogonal to the horizon, all vanishing points should appear on said horizon.
