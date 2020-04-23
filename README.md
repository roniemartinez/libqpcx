libqpcx
=======

Qt C++ plugin for ZSoft's PCX (Personal Computer Exchange) and DCX (Multipage PCX) image file format


FILE TYPE SUPPORT
	
	PCX - Yes
	
	DCX - Yes (Only first page is shown)*
	
	*DCX is not an animated image format! It is a Multipage format.
	
	
COLOR SUPPORT

	BitsPerPixel	ColorPlanes		Supported?
	
	1				1				Yes
	
	1				4				Yes
	
	4				1				Yes	(Untested)
	
	8				1				Yes (Color - Tested / Grayscale - Untested)
	
	8				3				Yes
	
	8				4				Yes (Untested)

	
CONTRIBUTE

	If you have an existing PCX/DCX file. Feel free to send me an email at ronmarti18@gmail.com
	
	If you want to contribute to the code, just fork the project and pull requests


BUILD ON WINDOWS

````
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="C:/Qt/Qt-5.14.1-installer/5.14.2/msvc2017_64/bin" ..
````

open the folder in your favorite IDE and build it.


BUILD ON LINUX

````
mkdir build
cd build
cmake ..
make
````
