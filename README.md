# fast-mingle

### Overview
This is an implementation of the MINGLE edge bundling algorithm based loosely on 
[this paper](http://yifanhu.net/PUB/edge_bundling.pdf). It is written in C/C++ and 
operates with a simple heuristic closed-form equation instead of the golden section search
presented in the paper. While this abstracts slightly from the metaphor of ink saving, 
it does allow the algorithm to process much larger data sets. 

### File formats
Broadly, **fast-mingle** takes a graph _G=(V, E)_ and outputs a edge bundle tree
where each node in the tree represents an edge bundle. All input and output files
are in tsv format. There are two input files, one for vertices and another to 
specify the edges in the input graph. The vertices file has three headers, **id**, 
**x**, and **y**. The edges file has two headers, **id** and **connections**.  
