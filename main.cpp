#include <bits/stdc++.h>

using namespace std;
struct Node;
struct HeavyPath;


struct Edge{
    const int id;
    int cost;

    Node * n1;
    Node * n2;

    Edge(int id, Node * n1, Node * n2):id(id), n1(n1), n2(n2){};
    Edge(int id, int cost, Node * n1, Node * n2):id(id), cost(cost), n1(n1), n2(n2){};
};

struct Node{
    const int id;

    Edge * parentPath;
    vector<Edge *> connections;

    Edge * centroidParentPath;
    vector<Edge *> centroidConnections;

    int restaurantType;

    int level;
    
    int subTreeSize;

    bool isVisible = true;//wynika z hierarchii ale wtvr

    int firstEuler;
    int lastEuler;

    unordered_map<Node *, int> parentsPathCosts;//after centroid decomposition

    Node(int id, int restaurantType): id(id), restaurantType(restaurantType){};
    Node(int id): id(id){};
};

pair<int,int> commonPart(const pair<int,int> &range1, const pair<int,int> &range2){
    int min = (range1.first > range2.first ? range1.first : range2.first);
    int max = (range1.second < range2.second ? range1.second : range2.second);
    pair<int, int> range(min, max);
    return range;
}


struct SparseTable{
    vector<vector<Node *>> tab;

    SparseTable(const vector<Node *> &data){
        int height = ceil(log2(data.size()));
        //first row
        tab.emplace_back();
        for(int i = 0; i < data.size(); i++){
            tab[0].push_back(data[i]);
        }
        for(int i = 1; i < height; i ++){
            tab.emplace_back();
            for(int j = 0 ; j < data.size() - pow(2, i) + 1; j ++){
                int minRange = j, maxRange = j + pow(2, i) - 1;
                Node * min1 = minRangeQuery(j, j + pow(2, i - 1) - 1);
                Node * min2 = minRangeQuery(j + pow(2, i - 1), maxRange);
                tab.back().push_back((min1->level < min2->level ? min1 : min2));
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
        return (tab[y][x1]->level < tab[y][x2]->level ? tab[y][x1] : tab[y][x2]);
    }
};


//simple subtrees calculation and inheritance
int dfs(Node * n, Edge * comingFrom, vector<Node *> &eulerTour, int level = 0){
    if(comingFrom != nullptr){
        n->parentPath = comingFrom;
    }
    n->level = level;

    n->firstEuler = eulerTour.size();
    n->lastEuler = eulerTour.size();

    eulerTour.push_back(n);

    int subTreeSize = 1;

    for(auto e : n->connections){
        if(comingFrom != e){
            Node * another = (e->n1 != n ? e->n1 : e->n2);
            subTreeSize += dfs(another, e, eulerTour, level+1);
            n->lastEuler = eulerTour.size();
            eulerTour.push_back(n);
        }
    }
    if(eulerTour.back() != n){
        n->lastEuler = eulerTour.size();
        eulerTour.push_back(n);
    }
    n->subTreeSize = subTreeSize;
    return subTreeSize;
}


Node * findCentroid(Node * n){
    //first must be from the root of the tree. Otherwise it will not work.

    for(auto e : n->connections){
        if(e != n-> parentPath){
            Node * another = (e->n1 != n ? e->n1 : e->n2);
            if(another->isVisible && another->subTreeSize > (n->subTreeSize / 2)){
                int newNSubTreeSize = n->subTreeSize - another->subTreeSize;
                another->subTreeSize = n->subTreeSize;
                n->subTreeSize = newNSubTreeSize;
                n->parentPath = another->parentPath;
                another->parentPath = nullptr;
                return findCentroid(another);
            }
        }
    }
    return n;
}

Node * decomposeGraph(Node * root, int &edgeNum){

    root = findCentroid(root);
    root->isVisible = false;

    vector<Node *> children;
    for(auto e : root->connections){
        if(e != root -> parentPath){

            Node * another = (e->n1 != root ? e->n1 : e->n2);//not necessary the root of the other centroid
            if(another->isVisible){
                another = decomposeGraph(another, ++edgeNum);
                Edge * edge = new Edge(edgeNum, root, another); 
                root->centroidConnections.push_back(edge);
                another->centroidConnections.push_back(edge);
                another->centroidParentPath = edge;
            }
        }
    }
    return root;
}

int calculateSubTreeSizes(Node * n){
    int sum = 0;
    for(auto e : n->connections){
        if(e != n->parentPath){
            sum += calculateSubTreeSizes((e->n1 != n ? e->n1 : e->n2));
        }
    }
    sum += 1;
    n->subTreeSize = sum;
    return sum;
}
pair<bool, long> getPathData(Node * n, Node * target, int lookingFor){
    long cost = 0;
    bool foundResturant = false;
    while(n != target){
        cost += n->parentPath->cost;
        foundResturant += (n->restaurantType == lookingFor);
        n = (n->parentPath->n1 != n ? n->parentPath->n1 : n->parentPath->n2);
    }
    foundResturant += (target->restaurantType == lookingFor);
    return pair<bool, long>(foundResturant, cost);
}

int main() {
    int nodesCount, typesCount;
    cin >> nodesCount >> typesCount;

    vector<Node *> nodes;
    vector<Edge *> edges; //to delete after decomposition


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
    vector<Node *> eulerTour;

    dfs(nodes[0], nullptr, eulerTour);
    SparseTable sp = SparseTable(eulerTour);

    unordered_map<Node *, int> parentsPathCosts;

    int id = 0;
    Node * centroidRoot = decomposeGraph(nodes[0], id);



    int queriesCount;
    cin >> queriesCount;

    for(int i = 0; i < queriesCount; i ++){
        int n1ID, n2ID, restaurant;
        cin >> n1ID >> n2ID >> restaurant;
        n1ID --, n2ID --, restaurant --;
        if(i == 101){
            cout<<"";
        }

        Node * n1 = nodes[n1ID];
        Node * n2 = nodes[n2ID];
        Node * lca = sp.minRangeQuery(n1->lastEuler, n2->firstEuler);
        pair<bool, long> data1 = getPathData(n1, lca, restaurant);
        pair<bool, long> data2 = getPathData(n2, lca, restaurant);
        bool found = data1.first + data2.first;
        long sum = data1.second + data2.second;
        cout << (found ? sum : -1) << "\n";
    }


    return 0;
}
