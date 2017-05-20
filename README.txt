Compiling:

1) Install required python packages for v_repStubsGen

   On Linux:

	> sudo apt-get install python-tempita

   On OS X:

	> pip install tempita

2) Download and install Qt (same version as V-REP) i.e. 5.5.0

3) Download and install the XSLT processor

   On Linux:

	> sudo apt-get install xsltproc

   On OS X xsltproc is already available.


4) Generate makefiles

   On Linux:

	> ~/Qt5.5.0/5.5/gcc_64/bin/qmake v_repExtCustomUI.pro

   On OS X:

	> ~/Qt5.5.0/5.5/clang_64/bin/qmake v_repExtCustomUI.pro

5) Compile

	> make

