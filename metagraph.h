#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <functional>
#include <limits>

namespace metagraph {
    class Edge;

    class Metagraph;

    class Node {
    private:
        int id;
        std::function<double()> attrFunc;
        double attrVal = std::numeric_limits<double>::quiet_NaN();

    public:
        Node() = default;

        explicit Node(int id) : id(id) {}

        int getId() const { return id; }

        double getAttribute() {
            return std::isnan(attrVal) ? attrFunc() : attrVal;
        }

        void setAttribute(double attr) {
            attrVal = attr;
        }

        void setAttributeFunction(const std::function<double()> &func) {
            attrFunc = func;
        }
    };

    class Edge {
    private:
        int id;
        int from;
        int to;
        std::function<double()> attrFunc;
        double attrVal = std::numeric_limits<double>::quiet_NaN();


    public:
        Edge() = default;

        Edge(int id, int from, int to) : id(id), from(from), to(to) {}

        int getFrom() const { return from; }

        int getTo() const { return to; }

        int getId() const { return id; }

        double getAttribute() {
            return std::isnan(attrVal) ? attrFunc() : attrVal;
        }

        void setAttribute(double attr) {
            attrVal = attr;
        }

        void setAttributeFunction(const std::function<double()> &func) {
            attrFunc = func;
        }
    };

    class Metagraph {
    private:
        int v = 0, e = 0;
        std::unordered_map<int, Node> nodes;
        std::unordered_map<int, Edge> edges;
        std::unordered_map<int, std::vector<Edge *>> adjacencyList, backwardsAdjacencyList;

    public:
        void addNode(int id) {
            v++;
            nodes[id] = Node(id);
        }

        void addEdge(int id, int from, int to) {
            e++;
            edges[id] = Edge(id, from, to);
            adjacencyList[from].push_back(&edges[id]);
            backwardsAdjacencyList[to].push_back(&edges[id]);
        }

        void setNodeAttributeFunctionMin(int id) {
            nodes[id].setAttributeFunction(std::function<double()>(
                    [&, id]() {
                        double result;
                        std::vector<Edge *> *incomingNeighboursRef = getEdgesTo(id);
                        if (!incomingNeighboursRef->empty()) {
                            result = (incomingNeighboursRef->at(0))->getAttribute();
                            for (int i = 1; i < incomingNeighboursRef->size(); i++) {
                                result = std::min(result, (incomingNeighboursRef->at(i))->getAttribute());
                            }
                        } else {
                            result = std::numeric_limits<double>::quiet_NaN();
                        }
                        nodes[id].setAttribute(result);
                        return result;
                    })
            );
        }

        void setNodeAttributeFunctionCopy(int id, int copyType, int copyId) {
            if (copyType == 0) {
                nodes[id].setAttributeFunction(std::function<double()>(
                        [&, id, copyId]() {
                            double result = (nodes[copyId].getAttribute());
                            nodes[id].setAttribute(result);
                            return result;
                        })
                );
            } else {
                nodes[id].setAttributeFunction(std::function<double()>(
                        [&, id, copyId]() {
                            double result = (edges[copyId].getAttribute());
                            nodes[id].setAttribute(result);
                            return result;
                        })
                );
            }
        }

        void setEdgeAttributeFunctionMultiply(int id) {
            edges[id].setAttributeFunction(std::function<double()>(
                    [&, id]() {
                        int fromId = edges[id].getFrom();
                        double result;
                        std::vector<Edge *> *incomingNeighboursRef = getEdgesTo(fromId);
                        result = nodes[fromId].getAttribute();
                        for (int i = 0; i < incomingNeighboursRef->size(); i++) {
                            result *= incomingNeighboursRef->at(i)->getAttribute();
                        }
                        edges[id].setAttribute(result);
                        return result;
                    })
            );
        }

        void setEdgeAttributeFunctionCopy(int id, int copyType, int copyId) {
            if (copyType == 0) {
                edges[id].setAttributeFunction(std::function<double()>(
                        [&, id, copyId]() {
                            double result = (nodes[copyId].getAttribute());
                            edges[id].setAttribute(result);
                            return result;
                        })
                );
            } else {
                edges[id].setAttributeFunction(std::function<double()>(
                        [&, id, copyId]() {
                            double result = (edges[copyId].getAttribute());
                            edges[id].setAttribute(result);
                            return result;
                        })
                );
            }
        }

        Node *getNode(int id) {
            return &nodes[id];
        }

        Edge *getEdge(int id) {
            return &edges[id];
        }

        std::vector<Edge *> *getEdgesFrom(int id) {
            return &adjacencyList[id];
        }

        std::vector<Edge *> *getEdgesTo(int id) {
            return &backwardsAdjacencyList[id];
        }

        Metagraph(const std::string &filename) {
            std::fstream fin(filename);
            int nv, ne, from, to;
            fin >> nv >> ne;
            for (int i = 0; i < nv; i++) {
                addNode(i);
            }
            for (int i = 0; i < ne; i++) {
                fin >> from >> to;
                addEdge(i, from, to);
            }
            std::string input;
            for (int i = 0; i < nv; i++){
                fin >> input;
                if (input == "v" or input == "e"){
                    int copyId;
                    fin >> copyId;
                    if (input == "v"){
                        setNodeAttributeFunctionCopy(i, 0,copyId);
                    } else setNodeAttributeFunctionCopy(i, 1, copyId);
                } else if (input == "min"){
                    setNodeAttributeFunctionMin(i);
                } else nodes[i].setAttribute(std::stod(input));
            }
            for (int i = 0; i < ne; i++){
                fin >> input;
                if (input == "v" or input == "e"){
                    int copyId;
                    fin >> copyId;
                    if (input == "v"){
                        setEdgeAttributeFunctionCopy(i, 0,copyId);
                    } else setEdgeAttributeFunctionCopy(i, 1, copyId);
                } else if (input == "*"){
                    setEdgeAttributeFunctionMultiply(i);
                } else edges[i].setAttribute(std::stod(input));
            }
        }

        void computeAttributes(const std::string &filename){
            std::ofstream fout(filename);
            for (int i = 0; i < v; i++){
                fout << nodes[i].getAttribute() << std::endl;
            }
            for (int i = 0; i < e; i++){
                fout << edges[i].getAttribute() << std::endl;
            }
        }
    };
}
