# README #

This README would normally document whatever steps are necessary to get your application up and running.

### What is this repository for? ###

* Quick summary
* Version
* [Learn Markdown](https://bitbucket.org/tutorials/markdowndemo)

### How do I get set up? ###

* **Dependencies:** A C++ compiler std11 compliant (g++, icpc, ...) and the FastFlow library.
* **Configuration:** 
-  Add this custom class as friend of ff_node class defined in ff/node.hpp: into the file node.hpp type "friend class ff_comp;" into the ff_node class definition (somewhere between lines 430 - 440).
-  Add the following code snippet into ff/farm.hpp as a public function of the ff_farm class:
    
        const svector<ff_node*>& getWorkers() const { return std::move(Workers); }
    
* **How to run tests:**
* **Deployment instructions:**

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Who do I talk to? ###

* Repo owner or admin
* Other community or team contact
