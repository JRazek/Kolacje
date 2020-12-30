#include <bits/stdc++.h>

using namespace std;
struct Node;

struct Edge{
    const int id;
    const int cost;

    Node * n1;
    Node * n2;

    Edge(int id, int cost, Node * n1, Node * n2):id(id), cost(cost), n1(n1), n2(n2){};
};

struct Node{
    const int id;

   // Node * parentPath;
    vector<Edge *> connections;
    const int restaurantType;

    int lowOccurrence;
    int highOccurrence;

    int level;

    int firstEuler;
    int lastEuler;


    Node(int id, int restaurantType): id(id), restaurantType(restaurantType){};
};

struct Query{
    const int id;
    const Node * n1;
    const Node * n2;

    int low, high; //Mo's range

    const int restaurantWanted;

    Query(int id, Node * n1, Node * n2, int rWanted): id(id), n1(n1), n2(n2), restaurantWanted(rWanted){};

    int answer;

    struct Comparator{
        const int blockSize;
        Comparator(int blockSize): blockSize(blockSize){}
        bool operator () (Query * q1, Query * q2) const{
            int lowIndexQ1, highIndexQ1;
            lowIndexQ1 = (q1->n1->lowOccurrence > q1->n2->lowOccurrence ? q1->n1->lowOccurrence : q1->n2->lowOccurrence);
            highIndexQ1 = (q1->n1->highOccurrence < q1->n2->highOccurrence ? q1->n1->highOccurrence : q1->n2->highOccurrence);
            q1->low = lowIndexQ1;
            q1->high = highIndexQ1;

            int lowIndexQ2, highIndexQ2;
            lowIndexQ2 = (q2->n1->lowOccurrence > q2->n2->lowOccurrence ? q2->n1->lowOccurrence : q2->n2->lowOccurrence);
            highIndexQ2 = (q2->n1->highOccurrence < q2->n2->highOccurrence ? q2->n1->highOccurrence : q2->n2->highOccurrence);
            q2->low = lowIndexQ2;
            q2->high = highIndexQ2;

            int blockQ1Left = lowIndexQ1 / blockSize;
            int blockQ2Left = lowIndexQ2 / blockSize;
            if(blockQ1Left != blockQ2Left){
                return blockQ1Left < blockQ2Left;
            }
            int blockQ1Right = highIndexQ1 / blockSize;
            int blockQ2Right = highIndexQ2 / blockSize;

            return blockQ1Right < blockQ2Right;

        }
    };

};

struct SparseTable{
    vector<vector<Node *>> tab;

    SparseTable(const vector<Node *> &data){
        int height = log2(data.size());

        tab.emplace_back();
        for(int i = 0; i < data.size(); i ++){
            tab[0].push_back(data[i]);
        }
        for(int i = 1; i < height; i++){
            tab.emplace_back();
            int delayed = pow(2, i) - 1;
            for(int j = 0; j < data.size() - pow(2, i); j ++){
                //fix the sparse table
            }
        }
        cout<<"";
    }

    Node * minRangeQuery(int min, int max){
        int y = log2(max - min + 1);
        int x1 = min, x2 = x1 + pow(2, y) - 1;

        Node * n1 = tab[y][x1];
        Node * n2 = tab[y][x2];

        return (n1->level < n2->level ? n1 : n2);

    }
};

void dfs(Node * n, int comingFrom, vector<int> &occurrences, vector<Node *> &eulerTour, int level = 0){
    occurrences.push_back(n->id);
    eulerTour.push_back(n);

    n->lowOccurrence = occurrences.size() - 1;
    n->firstEuler = eulerTour.size() - 1;

    n->level = level;
    for(auto e : n->connections){
        if(e->id != comingFrom){
            Node * another = (e->n1 == n ? e->n2 : e->n1);
            dfs(another, e->id, occurrences, eulerTour, ++level);
            eulerTour.push_back(n);
            n->lastEuler = eulerTour.size();
        }
    }
    if(eulerTour.back() != n){
        eulerTour.push_back(n);
        n->lastEuler = eulerTour.size();
    }
    occurrences.push_back(n->id);
    n->highOccurrence = occurrences.size() - 1;
}

void validateState(const bool state, const int low, const int high, int &currentPrice, const Node * n, const int ticketPrice){
    if(state){
        //added n to the mos, check if its the second occurrence which would delete it from the price
        if(low >= n->lowOccurrence && high <= n->highOccurrence){

        }
    }
}


int main() {
    int nodesCount, typesCount;
    cin >> nodesCount >> typesCount;

    vector<Node *> nodes;
    for(int i = 0; i < nodesCount; i ++){
        int type;
        cin >> type;
        type --;
        nodes.push_back(new Node(i, type));
    }
    for(int i = 0; i < nodesCount - 1; i ++){
        int n1ID, n2ID, cost;
        cin >> n1ID >> n2ID >> cost;
        n1ID --; n2ID --;
        Node * n1 = nodes[n1ID];
        Node * n2 = nodes[n2ID];
        Edge * e = new Edge(i, cost, n1, n2);
        n1->connections.push_back(e);
        n2->connections.push_back(e);
    }

    int queriesCount;
    cin >> queriesCount;

    vector<Query *> queries;
    for(int i = 0; i < queriesCount; i ++){
        int n1, n2, restaurant;
        cin >> n1 >> n2 >> restaurant;
        n1 --, n2 --;

        queries.push_back(new Query(i, nodes[n1], nodes[n2], restaurant));
    }

    vector<int> dfsOrder; // each int corresponds to nodeID. first occurrence is preorder, second is postorder
    vector<Node *> eulerTour;

    dfs(nodes[0], -1, dfsOrder, eulerTour);
    int blockSize = ceil(sqrt(dfsOrder.size()));


    sort(queries.begin(), queries.end(), Query::Comparator(blockSize));

    SparseTable sp = SparseTable(eulerTour);


  //  Node * parent = sp.minRangeQuery(nodes[0]->firstEuler, nodes[3]->lastEuler);

    cout<<"";
    int typeOccurrence[typesCount];
    for(auto &i : typeOccurrence){
        i = 0;
    }

    typeOccurrence[nodes[dfsOrder[0]]->restaurantType] = 1;

    int low = 0, high = 0;


    int currentPrice = 0;
    for(auto query : queries){
        while(low != query->low){
            if(low < query->low){

                Node * n = nodes[dfsOrder[low]];
               // typeOccurrence[n->restaurantType] --;
                low ++;

            }
            else{
             //   typeOccurrence[nodes[dfsOrder[low]]->restaurantType] --;
                low --;
            }
        }

        while(high != query->high){
            if(high < query->high){
                high ++;
            }
            else{
                high --;
            }
        }
        cout<<"";
    }




    return 0;
}
