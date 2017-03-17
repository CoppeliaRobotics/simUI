Compiling:

1) Get v_repStubsGen, e.g.:

	> mkdir ~/python-packages
	> cd ~/python-packages
	> git clone https://github.com/fferri/v_repStubsGen.git
	> export PYTHONPATH=$PYTHONPATH:~/python-packages

2) Get required python packages for v_repStubsGen

   On Linux:

	> sudo apt-get install python-tempita

   On OS X:

	> pip install tempita

3) Download and install Qt (same version as V-REP) i.e. 5.5.0

4) Download and install SAXON

   On Linux:

	> sudo apt-get install xsltproc

   On OS X xsltproc is already available.


5) Generate makefiles

   On Linux:

	> ~/Qt5.5.0/5.5/gcc_64/bin/qmake v_repExtCustomUI.pro

   On OS X:

	> ~/Qt5.5.0/5.5/clang_64/bin/qmake v_repExtCustomUI.pro

6) Compile

	> make

