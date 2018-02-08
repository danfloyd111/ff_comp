# README #

This README document all the necessary steps to get this application up and running.      
** NOTE: this document is actually under construction! **

### What is this repository for? ###

**Quick summary:**    
The FastFlow framework provides to the programmer of parallel applications for multicore memory-shared devices a series of nestable skeletons that are made available as objects in C++. These objects
can be specialized in order to form parallel applications in many forms, both stream parallel and data parallel. Before today this framework didn't have a skeleton that sequentially composes objects:
given two code snippets that computes on their input data ```x``` the functions ```f(x)``` and ```g(x)```, FastFlow didn't have a skeleton ```Comp(f,g)``` that computes, in the binary case taken as
example, the composition ```f(g(x))```.

The purpose of this project is to implement this skeleton in a way that it can takes as input not only sequential code but pipelines and farms either, going beyond the simple binary composition. The
business code of the Comp skeleton it has been written in the FastFlow style, it is a static header only C++ library and we provide the source code, some unit tests and a couple of benchmarks.

All of this project is made available under GNU Lesser General Public licence 3.0 as published by the Free Software Foundation, they are distributed hoping that they may be useful but without any 
warranty of any type. The licence is available [here](https://www.gnu.org/licenses/lgpl.html).

* **Version 0.1**

### How do I get set up? ###

* **Dependencies:**    
A C++ compiler std11 compliant (g++, icpc, ...), the FastFlow library and OpenCV library (this last one is optional).
* **Configuration:**
First of all you have to get a copy of the FastFlow headers only library contained into the [FastFlow repository](http://sourceforge.net/projects/mc-fastflow/) by using svn:    
``` svn co https://svn.code.sf.net/p/mc-fastflow/code fastflow ```     
Once the code has been downloaded, the directory containing the _ff_ sub-directory with the FastFlow header files should be named in the -I flag of icpc (or g++), such that
the header files may be correctly found.     
At this point if you want to use _ffcomp_ construct in your application just copy _comp.hpp_ into the _fastflow/ff_ directory previously downloaded, then if you
want to run some tests and benchmarks jump to "How to run tests" section of this document.   
> **Note:** the actual makefile is used only for personal debug and test and it will change in the next release, if you want to use it you have to open it with an editor and
change FFDIR and CC variables in order to suit your needs.
* **How to run tests:**     
If you have modified the Makefile as described directly into it then run ```make all``` to compile and run all the unit test suite.     
If you want to run the benchmark you can run it directly with ```test/bin/comp_benchmark [options]``` or use the wrapper bash script contained into _/test_ directory with
```test/comp_benchmark.sh [options]```      
**TODO:** add rules to compile and run ffcompvideo    
     
To compile manually the sources you need to pass at least ```-O3 -std=c++11 -pthread -I FFDIR``` where FFDIR is the path to the FastFlow headers.     
     
You can find more informations on the homepage of [FastFlow project](http://calvados.di.unipi.it)

### Contribution guidelines ###

* **Writing tests:** Write your tests and add them under __/test__ directory (please add a brief documentation directly into the source).
* **Code review:** Send me a mail or open an issue on GitHub/BitBucket.
* Other guidelines can be found on the [FastFlow manual](http://calvados.di.unipi.it/dokuwiki/doku.php/ffnamespace:refman)

### Who do I talk to? ###

* **Repo owner:** Daniele Paolini - [Git Hub](https://github.com/danfloyd111), [Bit Bucket](https://bitbucket.org/danpaol), mail at d.paolini1@studenti.unipi.it
* **Community or team contact:** FastFlow [core team and contributors](http://calvados.di.unipi.it/dokuwiki/doku.php/ffnamespace:people)
