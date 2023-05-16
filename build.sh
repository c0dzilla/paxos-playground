#!/bin/sh

g++ -std=c++11 -pthread -o paxos_exec node.cc node_registry.cc
