
## Requirements analysis

We will analyze the requirements. The task requires the creation of an inverted index with parallel access. That is, it is a data structure that will be used to read a large number of documents on the disk that contain certain text data (in the case of coursework, it will be words). In general, such a structure and its modified versions can be widely used to speed up searches in search engines and in a large number of information systems. In our case, the inverted index will be used to find all documents that contain a certain word. That is, the data structure for creating an index can be a dictionary with words as a key and a list (plural for uniqueness) of all documents that contain a word as a value. Thus, a hash table was chosen as the main data structure. Of course, with a large amount of data, namely 2,000 files with movie reviews, which usually contain as much text as a small article, the unique number of words can be much more than 2,000 and amount to 20,000 of the most used words, and it should be noted that the words case sensitive. With this formulation of the problem, no matter how good the hash function is chosen, and no matter what the size of the array at the base of the hash table, collisions will occur. Two main methods can be used to solve this problem: Open Addressing and Separate Chaining. In my implementation, Separate Chaining will be reversed. That is, each cell of the array will contain a linked list. If a collision occurs, but there is already an entry at the address that refers to another word, a new linked list node will be created. The nodes of the linked list will contain a term to identify the word and a set of values of the documents that contain the word. To save computer memory, all possible paths to documents will be contained in the array, and only indexes from this array will be stored in the Inverted Index. The set will be built on the basis of a binary tree that can be made AVL, this will allow:

- Display indexes and files in sorted order
- check for document availability for log(N) operations
- don't worry about the resize operation if a hash table is used as the main data structure for the set.

Now we will analyze the provision of parallel access to this data structure. Since there are 2 phases: creating an index and reading it, let's look at each of them. The read phase, provided no new documents are added to the index, allows any number of threads to access the index without any problems. When creating an index, there may be problems with simultaneous reading and writing to the hash table. Because collisions can occur, multiple threads can write to the same memory cell of a tree, array, or linked list. This problem can be solved using mutexes, closing access to the table if one thread is already working on it. But the use of one mutex can have a negative effect on the performance of the operation of adding to the hash table and it is not a fact that it will give a big increase when using threads, since in this case the threads will speed up only the part with the calculation of the hash function, and the insertion speed will be the same as when using one stream. To solve this problem, you can create a separate array with mutexes, each of which will "protect" a certain interval of values of the hash table array, which, most likely, can significantly speed up work with the use of threads. If the hash function uniformly selects cells from the array, then with an increase in the number of mutexes from 1 to N, where N is the number of cells in the hash table, each subsequent mutex will give less and less probability of increasing the speed when using threads on average, while the memory requirements will grow linearly. Therefore, it is advisable to choose such a number that each mutex protects, for example, 10 cells. This will allow almost no loss in speed, but will reduce the memory requirements by (N/10)\*sizeof(mutex). That is, when using a hash table of 200,000 cells. Using 20,000 mutexes instead of 200,000 will make it possible to use not 1.6 MB, but 0.16 MB with almost no loss in speed. As the size of the index increases, the absolute difference will be even greater. Now let's move on to creating a system model.

## System model

The basic principles of the data structure itself are described above. The server architecture must also be added to the system model. It will be implemented on sockets. The server, upon receiving a connection request from the client, will create a separate client socket through which communication will take place and will create a separate thread for processing requests from this socket.

Let's graphically represent the model:

![Untitled Diagram](https://user-images.githubusercontent.com/43760986/195766809-5c1f81b8-e5c7-4bb6-8308-b16f43e69521.png)


## Project assembly and launch

The latest version of JetBrains CLion was used to build the project. To run locally:

- Make sure the system is windows (since winsock was used).
- Make sure the compiler is installed (only tested on MinGW)
- Create a new project with VCS and clone the repository with cmake configured.
- Run Debug configuration course\_work\_parallel\_computing\_server for the server.
- Run Debug configuration course\_work\_parallel\_computing\_client for the server.


## System operation time

**Indexing time depends on the number of threads**

Time taken by function using 1 threads: 2832 milliseconds

Time taken by function using 2 threads: 821 milliseconds

Time taken by function using 3 threads: 532 milliseconds

Time taken by function using 4 threads: 409 milliseconds

Time taken by function using 5 threads: 359 milliseconds

Time taken by function using 6 threads: 359 milliseconds

Time taken by function using 7 threads: 360 milliseconds

Time taken by function using 8 threads: 357 milliseconds

Graphically:

![image](https://user-images.githubusercontent.com/43760986/195766904-b5571983-666e-4b0c-8454-ba2b63d71dfb.png)


