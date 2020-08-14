# rear_sight_clone_rpi_webrtc_gst_ocv
This project contains tested code for working with a camera as a rear sight. 
This one share videostream via gstreamer into webrtc connection. 
Also this one modificates a frame image (add aim, small image), also I modify a .html script for moving cursor.

# how to use it :
## This commands run before compiling independend from a type of your computer, here matter only one thing, you should use Linux OS
* apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio

## on your personal computer with linux (tested on a webcamera, and it will work with any cameras wich work with gst-module v4l2src)
* install OpenCV via this link https://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html

## on your Raspberry PI (test on Raspberry PI 3 (and will work with RPI 4), with webcameras and raspberry camera)
* ... 
