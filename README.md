# Paxos Playground

A zero-setup, zero-dependency, single-process implementation of the Paxos algorithm for distributed consensus amongst a cluster of cooperating nodes. This is meant to serve as a playground to simulate Paxos runs with different configurations (number of nodes, number of proposers, network conditions, etc) without any setup overhead.

## Features

1. No external dependencies.
2. Runs in a single process. Nodes are simulated via multi-threading.
3. The network layer is completely stubbed. Network delays and paritions are simulated in code.
4. The cluster configuration (number of nodes, number of proposers, network conditions, etc) can be changed to run and observe Paxos in different conditions.

## Usage

1. Compile:
```console
codzilla@hogwarts:~/Projects/paxos-playground$ ./build.sh
```
2. Run:
```console
codzilla@hogwarts:~/Projects/paxos-playground$ ./paxos_exec
```
3. (Optional) If you wish to play with cluster configurations, the `main` entry-point is defined in *node.cc*. It has a demo configuration of 5 nodes and 2 proposers. The network delays and node partitions are simulated inside `Node::HandleProposal` and `Node::SendAcceptReqs`.

## Credits

- Leslie Lamport's [The Part-Time Parliament](https://lamport.azurewebsites.net/pubs/lamport-paxos.pdf) paper that introduced this technique.
- Martin Fowler's incredible [blog](https://martinfowler.com/articles/patterns-of-distributed-systems/paxos.html).

## Sample Run

```console
codzilla@hogwarts:~/Projects/paxos$ ./build.sh
codzilla@hogwarts:~/Projects/paxos$ ./paxos_exec
Created Node with id: 1 , Generation Clock value: 0
Created Node with id: 2 , Generation Clock value: 1
Created Node with id: 3 , Generation Clock value: 2
Created Node with id: 4 , Generation Clock value: 3
Created Node with id: 5 , Generation Clock value: 4
Paxos consensus not achieved yet...
Paxos consensus not achieved yet...
Paxos consensus not achieved yet...
{ node_id: 2, proposed_value: bob } PROPOSED { node_id: 1, proposed_value: alice }
{ node_id: 1, proposed_value: alice } PROMISED { node_id: 2, proposed_value: bob }
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 1, proposed_value: alice }
{ node_id: 2, proposed_value: bob } PROPOSED { node_id: 2, proposed_value: bob }
Paxos consensus not achieved yet...
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 2, proposed_value: bob }
{ node_id: 2, proposed_value: bob } PROMISED { node_id: 1, proposed_value: alice }
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 3 }
{ node_id: 3 } PROMISED { node_id: 1, proposed_value: alice }
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 5 }
{ node_id: 5 } PROMISED { node_id: 1, proposed_value: alice }
{ node_id: 2, proposed_value: bob } PROPOSED { node_id: 3 }
{ node_id: 3 } PROMISED { node_id: 2, proposed_value: bob }
Paxos consensus not achieved yet...
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 4 }
{ node_id: 4 } PROMISED { node_id: 1, proposed_value: alice }
{ node_id: 1, proposed_value: alice } achieved quoram in PROPOSE phase. Moving to ACCEPT phase
{ node_id: 1, proposed_value: alice } requested ACCEPT from { node_id: 2, proposed_value: bob }
{ node_id: 2, proposed_value: bob } ACCEPTED { node_id: 1, proposed_value: alice }
Paxos consensus not achieved yet...
{ node_id: 2, proposed_value: bob } PROPOSED { node_id: 5 }
{ node_id: 5 } PROMISED { node_id: 2, proposed_value: bob }
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } requested ACCEPT from { node_id: 3 }
{ node_id: 1, proposed_value: alice } requested ACCEPT from { node_id: 5 }
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } requested ACCEPT from { node_id: 4 }
{ node_id: 4 } ACCEPTED { node_id: 1, proposed_value: alice }
{ node_id: 2, proposed_value: bob } PROPOSED { node_id: 4 }
{ node_id: 2, proposed_value: bob } achieved quoram in PROPOSE phase. Moving to ACCEPT phase
Paxos consensus not achieved yet...
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 1, proposed_value: alice }
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 2, proposed_value: bob }
Paxos consensus not achieved yet...
{ node_id: 2, proposed_value: bob } requested ACCEPT from { node_id: 1, proposed_value: alice }
{ node_id: 1, proposed_value: alice } ACCEPTED { node_id: 2, proposed_value: bob }
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 3 }
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 5 }
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 4 }
Paxos consensus not achieved yet...
{ node_id: 2, proposed_value: bob } requested ACCEPT from { node_id: 3 }
{ node_id: 3 } ACCEPTED { node_id: 2, proposed_value: bob }
Paxos consensus not achieved yet...
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 1, proposed_value: alice }
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 2, proposed_value: bob }
Paxos consensus not achieved yet...
Paxos consensus not achieved yet...
{ node_id: 2, proposed_value: bob } requested ACCEPT from { node_id: 5 }
{ node_id: 5 } ACCEPTED { node_id: 2, proposed_value: bob }
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 3 }
Paxos consensus achieved; value committed: bob
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 5 }
{ node_id: 1, proposed_value: alice } PROPOSED { node_id: 4 }
```