#include <bits/stdc++.h>

using namespace std;
struct Node;
struct HeavyPath;


struct Edge{
    const int id;
    long cost;

    Node * n1;
    Node * n2;

    Edge(int id, Node * n1, Node * n2):id(id), n1(n1), n2(n2){};
    Edge(int id, long cost, Node * n1, Node * n2):id(id), cost(cost), n1(n1), n2(n2){};
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
    map<int, bool> visited;//dfdDfs with root of id <int>


    int firstEuler;
    int lastEuler;

    map<Node *, long> parentsPathCosts;//after centroid decomposition

    map<int, pair<vector<Node *>, long>> subtreeDistances;//closest distance to the n type of restaurant with cost of

    Node(int id, int restaurantType): id(id), restaurantType(restaurantType){};
    Node(int id): id(id){};
};


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

    Node * minRangeQuery(int min, int max) const{
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

bool isParent(const Node * parent, const Node * child, const SparseTable &sp){
    Node * lca = sp.minRangeQuery(parent->lastEuler, child->firstEuler);
    return (lca == parent);
}

//simple subtrees calculation and inheritance
int dfs0(Node * n, Edge * comingFrom){
    if(comingFrom != nullptr){
        n->parentPath = comingFrom;
    }

    int subTreeSize = 1;

    for(auto e : n->connections){
        if(comingFrom != e){
            Node * another = (e->n1 != n ? e->n1 : e->n2);
            subTreeSize += dfs0(another, e);
        }
    }
    n->subTreeSize = subTreeSize;
    return subTreeSize;
}

int dfs1(Node * n, vector<Node *> &eulerTour, int level = 0){
    n->level = level;

    n->firstEuler = eulerTour.size();
    n->lastEuler = eulerTour.size();

    eulerTour.push_back(n);

    int subTreeSize = 1;

    for(auto e : n->centroidConnections){
        if(e != n->centroidParentPath){
            Node * another = (e->n1 != n ? e->n1 : e->n2);
            subTreeSize += dfs1(another, eulerTour, level+1);
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

void dfs3(Node * n, Node * root, pair<Node * const, long> &parentsCost, const SparseTable &sp){
    n->parentsPathCosts[parentsCost.first] = parentsCost.second;
    n->visited[root->id] = true;
    if(n->id == 0){
        cout<<"";
    }
    for(auto e : n->connections){
        Node * another = (e->n1 != n ? e->n1 : e->n2);
        if(isParent(root, another, sp) && !another->visited[root->id]){
            parentsCost.second += e->cost;
            dfs3(another, root, parentsCost, sp);
            parentsCost.second -= e->cost;
        }
    }
    cout<<"";
}

void dfsDfs(Node * centroidRoot, const SparseTable &sp){
    pair<Node * const , long> cost(centroidRoot, 0);
    if(centroidRoot->id == 0)
        cout<<"";
    dfs3(centroidRoot, centroidRoot, cost, sp);
    for(auto e : centroidRoot->centroidConnections){
        if(e != centroidRoot->centroidParentPath){
            Node * another = (e->n1 != centroidRoot ? e->n1 : e->n2);
            dfsDfs(another, sp);
        }
    }
}

void dfsDfs2(Node * centroidRoot, Node * n){
    for(auto e : n->centroidConnections){
        if(e != n->centroidParentPath){
            Node * another = (e->n1 != n ? e->n1 : e->n2);
            dfsDfs2(centroidRoot, another);
        }
    }
    if((centroidRoot->subtreeDistances.find(n->restaurantType) != centroidRoot->subtreeDistances.end() &&
        centroidRoot->subtreeDistances[n->restaurantType].second > n->parentsPathCosts[centroidRoot]) ||
        centroidRoot->subtreeDistances.find(n->restaurantType) == centroidRoot->subtreeDistances.end()) {
            centroidRoot->subtreeDistances[n->restaurantType] = pair<vector<Node *>, long>({n}, n->parentsPathCosts[centroidRoot]);
    }
}

void dfs4(Node * centroidRoot){
   // map<int, int> distanceTable;
    dfsDfs2(centroidRoot, centroidRoot);
    //centroidRoot->subtreeDistances.insert(distanceTable.begin(), distanceTable.end());
    for(auto e : centroidRoot->centroidConnections){
        if(e != centroidRoot->centroidParentPath){
            Node * another = (e->n1 != centroidRoot ? e->n1 : e->n2);
            dfs4(another);
        }
    }
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
        Node * another = (e->n1 != root ? e->n1 : e->n2);//not necessary the root of the other centroid
        if(another->isVisible){
            another = decomposeGraph(another, ++edgeNum);
            Edge * edge = new Edge(edgeNum, root, another);
            root->centroidConnections.push_back(edge);
            another->centroidConnections.push_back(edge);
            another->centroidParentPath = edge;
        }
    }
    return root;
}

long getPath(Node * n1, Node * n2, SparseTable &sp){
    Node * lca = sp.minRangeQuery(n1->firstEuler, n2->lastEuler);
    return n1->parentsPathCosts[lca] + n2->parentsPathCosts[lca];
}

pair <vector<Node *>, long> findClosestPathToRestaurant(Node * n, int type){
    Node * p = n;

    long shortest = 1e15;
    Node * shortestNode = nullptr;
    while(true){
        if(p->subtreeDistances.find(type) != p->subtreeDistances.end()){
            if(p->subtreeDistances[type].second + n->parentsPathCosts[p] < shortest) {
                shortest = p->subtreeDistances[type].second + n->parentsPathCosts[p];
                shortestNode = p;
            }
        }
        if(p->centroidParentPath == nullptr) {
            break;
        }
        p = (p->centroidParentPath->n1 != p ? p->centroidParentPath->n1 : p->centroidParentPath->n2);
    }
    return pair<vector<Node *>, long>(shortestNode->subtreeDistances[type].first, shortest);
}

long manageQuery(Node * n1, Node * n2, int type, SparseTable &sp){
    pair<vector<Node *>, long> n1p1 = findClosestPathToRestaurant(n1, type);//candidates1
    pair<vector<Node *>, long> n2p1 = findClosestPathToRestaurant(n2, type);//candidates2

    long n2p2 = getPath(n1p1.first, n2, sp);
    long n1p2 = getPath(n2p1.first, n1, sp);

    return ((n1p1.second + n2p2 < n2p1.second + n1p2) ? n1p1.second + n2p2 : n2p1.second + n1p2);

    //int c1 = findClosestPathToRestaurant(n1, type);
    //int c2 = findClosestPathToRestaurant(n2, type);
}

int main() {
    int nodesCount, typesCount;
    cin >> nodesCount >> typesCount;

    vector<Node *> nodes;
    vector<Edge *> edges; //to delete after decomposition
    bool restaurantAvailable[1000001] = {false};

    for(int i = 0; i < nodesCount; i ++){
        int type;
        cin >> type;
        type --;
        restaurantAvailable[type] = true;
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


    dfs0(nodes[0], nullptr);


    int id = 0;
    Node * centroidRoot = decomposeGraph(nodes[0], id);

    vector<Node *> eulerTour;
    dfs1(centroidRoot, eulerTour, 0);

    SparseTable sp = SparseTable(eulerTour);

    dfsDfs(centroidRoot, sp);

    dfs4(centroidRoot);

    int queriesCount;
    cin >> queriesCount;

    for(int i = 0; i < queriesCount; i ++){
        int n1ID, n2ID, restaurant;
        cin >> n1ID >> n2ID >> restaurant;
        n1ID --, n2ID --, restaurant --;
        if(i == 36){
            cout<<"";
        }
        if(restaurantAvailable[restaurant]){
            Node * n1 = nodes[n1ID];
            Node * n2 = nodes[n2ID];
            long result = manageQuery(n1, n2, restaurant, sp);
            cout<<result<<"\n";
        }else{
            cout<<-1<<"\n";
        }
        //cout << (found ? sum : -1) << "\n";
    }


    return 0;
}
