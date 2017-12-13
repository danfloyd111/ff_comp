# README #

This README document all the necessary steps to get this application up and running.
** NOTE: this document is actually under construction! **

### What is this repository for? ###

* **Quick summary:**    
**TODO:** paste here the abstract of the thesis.

* **Version 0.1**

### How do I get set up? ###

* **Dependencies:**    
A C++ compiler std11 compliant (g++, icpc, ...) and the FastFlow library.
* **Configuration:**
First of all you have to get a copy of the FastFlow headers only library contained into the [FastFlow repository](http://sourceforge.net/projects/mc-fastflow/) by using svn:    
``` svn co https://svn.code.sf.net/p/mc-fastflow/code fastflow ```     
Once the code has been downloaded, the directory containing the _ff_ sub-directory with the FastFlow header files should be named in the -I flag of icpc (or g++), such that
the header files may be correctly found.     
At this point if you want to use __ff_comp__ construct in your application just copy __comp.hpp__ into the __fastflow/ff__ directory previously downloaded, otherwise if you
want to run some tests and benchmark jump to "How to run tests" section of this document.   
> **Note:** the actual makefile is used only for personal debug and test and it will change in the next release, if you want to use it you have to open it with an editor and
change FFDIR and CC variables in order to suit your needs.
* **How to run tests:**     
If you have modified the Makefile as described directly into it then run ```make all``` to compile and run all the test suite.     
If you want to run the benchmark you can run it directly with ```test/bin/comp_benchmark [options]``` or use the wrapper bash script contained into __/test__ directory with
```test/comp_benchmark.sh [options]```    
     
To compile manually the sources you need to pass at least ```-O3 -std=c++11 -pthread -I FFDIR``` where FFDIR is the path to the FastFlow headers.     
     
You can find more informations on the homepage of [FastFlow project](https://calvados.di.unipi.it)

### Contribution guidelines ###

* **Writing tests:** Write your tests and add them under __/test__ directory (please add a brief documentation directly into the source).
* **Code review:** Send me a mail or open an issue on GitHub/BitBucket.
* Other guidelines can be found on the [FastFlow manual](http://calvados.di.unpi.it/dokuwiki/doku.php/ffnamespace:refman)

### Who do I talk to? ###

* **Repo owner:** Daniele Paolini - [Git Hub](https://github.com/danfloyd111), [Bit Bucket](https://bitbucket.org/danpaol), mail at d.paolini1@studenti.unipi.it
* **Community or team contact:** FastFlow [core team and contributors](http://calvados.di.unipi.it/dokuwiki/doku.php/ffnamespace:people)
