# Interview


### There are 3 branches:

* master _(naive n^2 implementation)_

* groupped _(field divided into several segments and every ball is handled within a segment along with its 'neighbours')_

* groupped_multithreaded _(the same as groupped, but each segment is handled in a thread)_


### Material used

* https://en.wikipedia.org/wiki/Elastic_collision

* https://stackoverflow.com/questions/35211114/2d-elastic-ball-collision-physics

* https://imada.sdu.dk/~rolf/Edu/DM815/E10/2dcollisions.pdf

### Acknowledgements:

* [Javidx9](https://github.com/OneLoneCoder)

### todo:

* Use TBB thread pool

* Pull rendering stuff out of the main source

* Implement virtual shape hierarchy to have more robust and flexible architecture
