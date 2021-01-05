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
    const int restaurantType;

    int level;
    
    HeavyPath * heavyPath = nullptr; //if null - no belonging to any heavy path
    int idInHeavyPath;  //if heavy is not null - the id in the vector of the heavy path
    
    int subTreeSize;


    int firstEuler;
    int lastEuler;


    Node(int id, int restaurantType): id(id), restaurantType(restaurantType){};
};

struct SegmentTree{
    struct BinaryNode{
        const int id;
        unordered_set<int> occurrences; 
        BinaryNode(int id):id(id){}
    };
    vector<BinaryNode *> nodes;
    SegmentTree(vector<int> elements){
        int height = ceil(elements.size()) + 1;
        for(int i = 0; i < pow(2, height) - 1; i ++){
            nodes.push_back(new BinaryNode(i));
        }
        int firstFloorNode1 = pow(2, height - 1) - 1;
        for(int i = 0; i < elements.size(); i ++){
            nodes[firstFloorNode1 + i]->occurrences.insert(elements[i]);
        }
        for(int i = pow(2, height - 1) - 2; i >= 0; i --){
            BinaryNode * node = nodes[i];
            BinaryNode * left = getChild(node, true);
            BinaryNode * right = getChild(node, false);
            node->occurrences.insert(left->occurrences.begin(), left->occurrences.end());
            node->occurrences.insert(right->occurrences.begin(), right->occurrences.end());
        }
    }
    BinaryNode * getChild(BinaryNode * parent, bool left){
        return nodes[parent->id * 2 + 1 + !left];
    }
    BinaryNode * getParent(BinaryNode * child){
        return nodes[(child->id - 1) / 2];
    }
};

struct HeavyPath{
    vector<int> prefixSum1; //top to bottom, path cost
    
    vector<Node *> elements; //top to bottom, first Node's path is 0 in the prefix sum as theres no occurence of it during walk
    int id;
    Node * parentNode;

    SegmentTree * restaurantOccurrences;

    HeavyPath(Node * parentNode, int id): parentNode(parentNode), id(id){
        elements.push_back(parentNode);
    }
    void preprocess(){
        vector<int> occurrences;

        prefixSum1.push_back(0);//first Node

        occurrences.push_back(elements[0]->restaurantType);
        for(int i = 1; i < elements.size(); i ++){
            prefixSum1.push_back(prefixSum1.back() + elements[i]->parentPath->cost);
            occurrences.push_back(elements[i]->restaurantType);
        }
    }
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

void propagateHeavyPaths(Node * node, vector<HeavyPath *> &heavyPaths){
    for(auto e : node->connections){
        Node * another = (e->n1 != node ? e->n1 : e->n2); 
        if(another->subTreeSize > (node->subTreeSize / 2)){
            //the edge create heavypath
            if(node->heavyPath != nullptr){
                another->idInHeavyPath = node->heavyPath->elements.size();
                node->heavyPath->elements.push_back(another);
            }else{
                //create new heavyPath
                HeavyPath * p = new HeavyPath(another, heavyPaths.size());
                another->heavyPath = p;
                heavyPaths.push_back(p);
            }
        }
    }
}

int countPathCost(Node * n, Node * target){
    int cost = 0;
    while(n != target){
        if(n->heavyPath != nullptr && n->heavyPath->parentNode != n){
            //range query
            //check if parent doesnt belong to the heavy path
            int start, end = n->idInHeavyPath;
            if(target->heavyPath == n->heavyPath){
                start = target->idInHeavyPath;
                n = target;
            }else{
                start = 0;
                n = n->heavyPath->parentNode;
            }
            int tmpCost = n->heavyPath->prefixSum1[end] - n->heavyPath->prefixSum1[start];
            cost += tmpCost;
        }else{
            cost += n->parentPath->cost;
            n = (n->parentPath->n1 != n ? n->parentPath->n1 : n->parentPath->n2);
        }
    }
    return cost;
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
    vector<Node *> eulerTour;

    dfs(nodes[0], nullptr, eulerTour);
    SparseTable sp = SparseTable(eulerTour);

    //create segment trees on the tree
    vector<HeavyPath *> heavyPaths;
    
    propagateHeavyPaths(nodes[0], heavyPaths);

    for(auto h : heavyPaths){
        h->preprocess();
    }

    int queriesCount;
    cin >> queriesCount;

    for(int i = 0; i < queriesCount; i ++){
        int n1ID, n2ID, restaurant;
        cin >> n1ID >> n2ID >> restaurant;
        n1ID --, n2ID --, restaurant --;

        Node * n1 = nodes[n1ID];
        Node * n2 = nodes[n2ID];
        Node * lca = sp.minRangeQuery(n1->lastEuler, n2->firstEuler);
        int cost1 = countPathCost(n1, lca);
        int cost2 = countPathCost(n2, lca);
        cout << cost1 + cost2 << "\n";
    }


    return 0;
}
