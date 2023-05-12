# Paxos Playground

A zero-setup, zero-dependency, single-process implemention of the Paxos algorithm for distributed consensus amongst a cluster of cooperating nodes. This is meant to serve as a playground to simulate Paxos runs with different configurations (number of nodes, number of proposers, network conditions, etc) without any setup overhead.

## Features

1. No external dependencies.
2. Runs in a single process. Nodes are simulated via multi-threading.
3. The network layer is completely stubbed. Network delays and paritions are simulated in code.
4. The cluster configuration (number of nodes, number of proposers, network conditions, etc) can be changed to run and observe Paxos in different conditions.

## Usage

1. Compile:
```
./build.sh
```
2. Run:
```
./paxos_exec
```
3. (Optional) If you wish to play with cluster configurations, the `main` entry-point is defined in *node.cc*. It has a demo configuration of 5 nodes and 2 proposers. The network delays and node partitions are simulated inside `Node::HandleProposal` and `Node::SendAcceptReqs`.

## Credits

- Leslie Lamport's [The Part-Time Parliament](https://lamport.azurewebsites.net/pubs/lamport-paxos.pdf) paper that introduced this technique.
- Martin Fowler's incredible [blog](https://martinfowler.com/articles/patterns-of-distributed-systems/paxos.html).
