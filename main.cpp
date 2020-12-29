#include <bits/stdc++.h>

using namespace std;

struct Node{
    const int id;

    Node * parent;
    vector<Node *> connections;

    Node(int id): id(id){};
};

struct Query{
    const int id;
    const Node * n1;
    const Node * n2;

    Query(int id, Node * n1, Node * n2): id(id), n1(n1), n2(n2){};

    int answer;
};

int main() {
    int nodesCount, typesCount;
    cin >> nodesCount >> typesCount;

    vector<Node *> nodes;
    for(int i = 0; i < nodesCount; i ++){
        nodes.push_back(new Node(i));
    }
    for(int i = 0; i < nodesCount - 1; i ++){
        int n1ID, n2ID;
        cin >> n1ID >> n2ID;
        n1ID --; n2ID --;
        Node * n1 = nodes[n1ID];
        Node * n2 = nodes[n2ID];

        n1->connections.push_back(n2);
        n2->connections.push_back(n1);
    }

    int queriesCount;
    cin >> queriesCount;

    vector<Query *> queries;
    for(int i = 0; i < queriesCount; i ++){
        int n1, n2;
        cin >> n1 >> n2;
        n1 --, n2 --;

        queries.push_back(new Query(i, nodes[n1], nodes[n2]));
    }



    return 0;
}
