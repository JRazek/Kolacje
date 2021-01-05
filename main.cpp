#include <bits/stdc++.h>

using namespace std;
struct Node;
struct HeavyPath;


struct Edge{
    const int id;
    const int cost;

    Node * n1;
    Node * n2;

    Edge(int id, int cost, Node * n1, Node * n2):id(id), cost(cost), n1(n1), n2(n2){};
};

struct Node{
    const int id;

    Edge * parentPath;
    vector<Edge *> connections;
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

void findCentroids(Node * n, Node * currentParent, vector<Node *> &centroidNodes, vector<Edge *> &centroidEdges, unordered_map<Node *, int> &parentsPathCosts){
    bool all = true;
    for(auto e : n->connections){
        if(e != n->parentPath){
            Node * child = (n != e->n1 ? e->n1 : e->n2);
            if(child->subTreeSize > (n->subTreeSize / 2)){
                all = false;
                int newSizeN = n->subTreeSize - child->subTreeSize;
                child->subTreeSize = n->subTreeSize;
                n->subTreeSize = newSizeN;
                //hierarchy swap
                n->parentPath = child->parentPath;
                child->parentPath = nullptr;
                findCentroids(child, currentParent, centroidNodes, centroidEdges, parentsPathCosts);
                break;
            }
        }
    }
    if(all){
        //this is a centroid

        for(auto e : n->connections){
            if(e != n->parentPath){
                Node * child = (n != e->n1 ? e->n1 : e->n2);
                findCentroids(child, n, centroidNodes, centroidEdges, parentsPathCosts);
            }
        }
        n->connections.clear();
        if(currentParent != nullptr){
            Edge * c = new Edge(centroidEdges.size(), parentsPathCosts[currentParent], currentParent, n);
            currentParent->connections.push_back(c);
            n->parentsPathCosts.insert(parentsPathCosts.begin(), parentsPathCosts.end());
            n->connections.push_back(c);
            n->parentPath = c;
        }        

    }
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

    vector<Node *> centroidNodes;
    vector<Edge *> centroidEdges;


    for(int i = 0; i < nodesCount; i ++){
        int type;
        cin >> type;
        type --;
        nodes.push_back(new Node(i, type));
        centroidNodes.push_back(new Node(i, type));
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

    findCentroids(nodes[0], nullptr, centroidNodes, centroidEdges, parentsPathCosts);


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
