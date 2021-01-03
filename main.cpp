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

    int parentPathCost;
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

    int restaurantWanted;

    bool restaurantFound = false;

    Query(int id, Node * n1, Node * n2, int rWanted): id(id), n1(n1), n2(n2), restaurantWanted(rWanted){
        low = n1->lowOccurrence;
        high = n2->lowOccurrence;
        if(low > high){
            int tmp = high;
            high = low;
            low = tmp;
        }
    };

    int answer;

    struct Comparator{
        const int blockSize;
        Comparator(int blockSize): blockSize(blockSize){}
        bool operator () (const Query * q1, const Query * q2) const{
            int lowIndexQ1 = q1->low, highIndexQ1 = q1->high;

            int lowIndexQ2 = q2->low, highIndexQ2 = q2->low;

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
        tab.push_back(vector<Node *> ());
        for(int i = 0; i < data.size(); i ++){
            tab[0].push_back(data[i]);
        }
        for(int i = 1; i < height; i ++){
            tab.emplace_back();
            for(int j = 0; j < data.size() - pow(2, i) + 1; j ++){
                Node * n1 = minRangeQuery(j, j + pow(2, i - 1) - 1);
                Node * n2 = minRangeQuery(j + pow(2, i - 1), j + pow(2, i) - 1);
                tab[i].push_back((n1->level < n2->level ? n1 : n2));
            }
        }

    }

    Node * minRangeQuery(int min, int max){
        if(min > max){
            int tmp = max;
            max = min;
            min = tmp;
        }
        int y = log2(max - min + 1);
        int x1 = min, x2 = max - pow(2, y) + 1;
        return (tab[y][x1] < tab[y][x2] ? tab[y][x1] : tab[y][x2]);
    }
};

void dfs(Node * n, Edge * comingFrom, vector<int> &occurrences, vector<Node *> &eulerTour, int level = 0){
    if(comingFrom != nullptr){
        n->parentPathCost = comingFrom->cost;
    }else{
        n->parentPathCost = 0;
    }
    occurrences.push_back(n->id);
    eulerTour.push_back(n);

    n->lowOccurrence = occurrences.size() - 1;
    n->firstEuler = eulerTour.size() - 1;

    n->level = level;
    for(auto e : n->connections){
        if(e != comingFrom){
            Node * another = (e->n1 == n ? e->n2 : e->n1);
            dfs(another, e, occurrences, eulerTour, level + 1);
            eulerTour.push_back(n);
            n->lastEuler = eulerTour.size() - 1;
        }
    }
    if(eulerTour.back() != n){
        eulerTour.push_back(n);
        n->lastEuler = eulerTour.size() - 1;
    }
    occurrences.push_back(n->id);
    n->highOccurrence = occurrences.size() - 1;
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
        n2->connections.push_back(e);//
    }


    vector<int> dfsOrder; // each int corresponds to nodeID. first occurrence is preorder, second is postorder
    vector<Node *> eulerTour;

    dfs(nodes[0], nullptr, dfsOrder, eulerTour);
    int blockSize = ceil(sqrt(dfsOrder.size()));
    SparseTable sp = SparseTable(eulerTour);

    int queriesCount;
    cin >> queriesCount;

    vector<Query *> queries;
    for(int i = 0; i < queriesCount; i ++){
        int n1, n2, restaurant;
        cin >> n1 >> n2 >> restaurant;
        n1 --, n2 --, restaurant --;

        Node * lca = sp.minRangeQuery(nodes[n1]->lastEuler, nodes[n2]->firstEuler);
        if(lca == nodes[n1] || lca == nodes[n2]){
            queries.push_back(new Query(i, nodes[n1], nodes[n2], restaurant));
        }else{
            queries.push_back(new Query(i, nodes[n1], lca, restaurant));
            queries.push_back(new Query(i, lca, nodes[n2], restaurant));
        }
        cout<<"";
        //to same id queries - answer is the answer of both combined together
        //for example - answer for query of id 5 would be the first query of id 5 + second of id 5
    }



    sort(queries.begin(), queries.end(), Query::Comparator(blockSize));


    cout<<"";

    int typeOccurrence[typesCount];

    for(int i = 0; i < typesCount; i ++){
        typeOccurrence[i] = 0;
    }

    int low = 0, high = 0;

    int currentPrice = 0;
    typeOccurrence[nodes[dfsOrder[0]]->restaurantType] = 1;
    for(auto query : queries){
        while(low < query->low){
            Node * n = nodes[dfsOrder[low]];
            if(n->lowOccurrence == low && n->highOccurrence <= high){
                currentPrice += n->parentPathCost;
                typeOccurrence[n->restaurantType] ++;
                //removal of double occurrence
            }else{
                currentPrice -= n->parentPathCost;
                typeOccurrence[n->restaurantType] --;
            }
            low ++;
        }
        while(low > query->low){
            low --;
            Node * n = nodes[dfsOrder[low]];
            if(n->lowOccurrence == low && n->highOccurrence <= high){
                currentPrice -= n->parentPathCost;
                typeOccurrence[n->restaurantType] --;
            }else{
                currentPrice += n->parentPathCost;
                typeOccurrence[n->restaurantType] ++;
            }
        }
        while(high > query->high){
            Node * n = nodes[dfsOrder[high]];
            if(n->highOccurrence == high && n->lowOccurrence >= low){
                currentPrice += n->parentPathCost;
                typeOccurrence[n->restaurantType] ++;
            }else{
                currentPrice -= n->parentPathCost;
                typeOccurrence[n->restaurantType] --;
            }
            high --;
        }
        while(high < query->high){
            high ++;
            Node * n = nodes[dfsOrder[high]];
            if(n->highOccurrence == high && n->lowOccurrence >= low){
                currentPrice -= n->parentPathCost;
                typeOccurrence[n->restaurantType] --;
            }else{
                currentPrice += n->parentPathCost;
                typeOccurrence[n->restaurantType] ++;
            }
        }
        query->answer = currentPrice;
        if(typeOccurrence[query->restaurantWanted] > 0){
            query->restaurantFound = true;
        }
    }
    pair<bool, int> answers[queriesCount];
    for(auto &i : answers){
        i = make_pair(false, 0);
    }
    for(auto query : queries){
        answers[query->id].second += query->answer;
        answers[query->id].first += query->restaurantFound;
    }
    for(auto &i : answers){
        cout << (i.first ? i.second : -1) << "\n";
    }




    return 0;
}
