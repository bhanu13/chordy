#Introduction
We implemented the Chord P2P Architecture in C++ using POSIX Threads and C sockets. The paper can be found here: https://pdos.csail.mit.edu/papers/chord:sigcomm01/chord_sigcomm.pdf</br>
##Architecture
We created a Node class with an Id, stored keys and a Finger table, and implemented the Chord system as a list of such Nodes. We further added a failure detector node with ID = 2^n using n bit identifiers for keys and nodes. The detector sent periodic heartbeat messages to the other nodes to detect crashes.The failure detector also acted as a stabilizer, requiring nodes to update their FT tables periodically as mentioned in the paper. The chord system was implemented to allow addition of nodes, finding keys on nodes and crashing a node based on the Algorithms mentioned in the SIGCOMM paper. 

##Assumptions
We made some assumptions to simplify the implementation. The node identifiers and keys both consist of 8 bits (i.e. in the range 0 to 255). We did not hash the keys or node identifiers (or, in other words, we assumed to be already specified as hash values). Failure detection is handled by storing an extra replica of each key at the successor node. Node-to-node communications were made through the message passing with simulated delays as in true systems.</br>

##Instructions to Run:
- Run `make` in the appropriate directory
- Run `./chord` after that.

##Commands:
- `join n`   :    Adds node `n` to the chord system.
- `find p k` :    Asks node `p` to find key `k`
- `show n`   :    Prints the finger table and keys at node `n`
- `show all` :    Prints the finger tables and keys at all the nodes sorted by the node ids in ascending order.
- `crash n`  :    Crashes node `n`
- `detect`   :    Toggles failure detection functionality.
- `print`    :    Prints the finger tables at all the nodes in ascending order of the node ids.
- `exit`     :    Quits the program.
- `rep n`    :    Prints all the replica keys at node `n`.
- `pred n`   :    Prints the predecessor of node `n`.

  
##Observations:
- Finding a key in a node scales significantly lesser than adding nodes to the system as the number of nodes increases. The number of messages transferred for finding a node is of the order of O(log(n)) where n is the no. of nodes in the chord system.
- The average number of messages per addition of node seems to stay close to constant. The number of messages transferred for joining a new node to the chord is significantly greater in this case, since finger tables of other nodes also had to be updated.

##Result:
The code mimicked a chord system and was used to analyze the chord architecture. A python file (sim.py) showing the simulation code is also included in the folder.

### Contributors:
Bhanu Agarwal (bhanu13), Ankur Gupta (agupta67)