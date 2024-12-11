#include <iostream>
#include <cstring>   // for memset
#include <unordered_map>

using namespace std;

const int MAX_TLB_SIZE = 1024;  
const int MAX_PAGES = 4096;  

// FIFO Algorithm
int FIFO(int* memoryAccesses, int N, int pageSize, int tlbSize) {
    int tlb[MAX_TLB_SIZE]; 
    unordered_map<int,int> tlb_lookup; 
    int front = 0, count = 0, hits = 0;
    memset(tlb, -1, sizeof(tlb));  
    
    for (int i = 0; i < N; ++i) {
        int page = (unsigned int )memoryAccesses[i]/(1024* pageSize);
        //bool hit = false;
        
        if (tlb_lookup[page]==1){
            //hit=true;
            hits++;
        }
        else{
            if (tlb[front]!=-1){
                tlb_lookup[tlb[front]]=0;
            }
            tlb[front]=page;
            front=(front+1)%tlbSize;
            tlb_lookup[page]=1;
        }
    }
    
    return hits;
}

// LIFO Algorithm
int LIFO(int* memoryAccesses, int N, int pageSize, int tlbSize) {
    int tlb[MAX_TLB_SIZE]; 
    unordered_map<int,int> tlb_lookup; 
    int top = 0, hits = 0;
    memset(tlb, -1, sizeof(tlb)); 
    
    for (int i = 0; i < N; ++i) {
        int page = (unsigned int )memoryAccesses[i]/(1024* pageSize);
        //bool hit = false;

        if (tlb_lookup[page]==1){
            //hit=true;
            hits++;
        }
        else{
            if (tlb[top]==-1 && top<tlbSize){
                tlb[top]=page;
                top++;
                tlb_lookup[page]=1;
            }
            else if (top==tlbSize){
                tlb_lookup[tlb[top-1]]=0;
                tlb[top-1]=page;
                tlb_lookup[page]=1;
            }
        }
    }
    
    return hits;
}

// // LRU Algorithm
// int LRU(int* memoryAccesses, int N, int pageSize, int tlbSize) {
//     int tlb[MAX_TLB_SIZE];  // TLB array
//     unordered_map<int,int> lastUsed;
//     int hits = 0;
//     int idx=0;
//     memset(tlb, -1, sizeof(tlb));
    
//     for (int i = 0; i < N; ++i) {
//         int page = getPageNumber(memoryAccesses[i], pageSize);
//         bool hit = false;


//         if (lastUsed[page]>=1){
//             lastUsed[page]=i+1;
//             hit=true;
//             hits++;
//         }
//         else{
//             if (lastUsed[page]==0 && idx<tlbSize){
//                 tlb[idx]=page;
//                 idx++;
//                 lastUsed[page]=i+1;
//             }
//             else if (idx==tlbSize){
//                 int minn=0;
//                 for (int j=1;j<tlbSize;j++){
//                     if (lastUsed[tlb[j]]<lastUsed[tlb[minn]]){
//                         minn=j;
//                     }
//                 }
//                 lastUsed[tlb[minn]]=0;
//                 tlb[minn]=page;
//                 lastUsed[page]=i+1;
//             }
//         }
//     }
    
//     return hits;
// }

// Optimized LRU Algorithm
// int LRU(int* memoryAccesses, int N, int pageSize, int tlbSize) {
//     int tlb[MAX_TLB_SIZE]; 
//     unordered_map<int, int> lastUsed; 
//     int hits = 0;
//     int idx = 0; 
//     memset(tlb, -1, sizeof(tlb));

//     for (int i = 0; i < N; ++i) {
//         int page = getPageNumber(memoryAccesses[i], pageSize);

//         if (lastUsed.find(page) != lastUsed.end()) {
//             hits++;
//             lastUsed[page] = i;  
//         } else {
//             if (idx < tlbSize) {
//                 tlb[idx] = page;
//                 lastUsed[page] = i;
//                 idx++;
//             } else {
//                 int lruIndex = 0;
//                 for (int j = 1; j < tlbSize; ++j) {
//                     if (lastUsed[tlb[j]] < lastUsed[tlb[lruIndex]]) {
//                         lruIndex = j;
//                     }
//                 }
//                 lastUsed.erase(tlb[lruIndex]);  
//                 tlb[lruIndex] = page;  
//                 lastUsed[page] = i; 
//             }
//         }
//     }

//     return hits; 
// }

struct Node {
    int page;
    Node* prev;
    Node* next;
    Node(int page) : page(page), prev(nullptr), next(nullptr) {}
};

void change_pos(Node* node,Node* head, Node* tail,unordered_map<int,Node*> tlb_lookup){
    if (node==head) return;
    else{
        if (node->prev) node->prev->next=node->next;
        if (node->next) node->next->prev=node->prev;
        if (node==tail) tail=node->prev;
        node->next=head;
        node->prev=nullptr;
        if (head) head->prev=node;
        head= node;
        if (tlb_lookup.size()==1) tail=head;
    }
}

void add_node(Node*node,Node* head, Node* tail,unordered_map<int,Node*> tlb_lookup){
    node->next=head;
    if (head) head->prev=node;
    head= node;
    if (tlb_lookup.size()==0) tail=head;
}

void remove_node(Node* head, Node* tail,unordered_map<int,Node*> tlb_lookup){
    tlb_lookup.erase(tail->page);  
    Node* remove_node=tail;
    tail=tail->prev;
    delete remove_node;
    if (tail!=NULL) tail->next=nullptr;
    if (tlb_lookup.size()==1) head=tail;
}


int LRU(int* memoryAccesses, int N, int pageSize, int tlbSize) {
    //int tlb[MAX_TLB_SIZE]; 
    unordered_map<int, Node*> tlb_lookup; 
    Node* head=nullptr;
    Node* tail=nullptr;
    int hits = 0;
    int idx = 0; 
    //memset(tlb, -1, sizeof(tlb));

    for (int i = 0; i < N; ++i) {
        int page = (unsigned int )memoryAccesses[i]/(1024* pageSize);

        if (tlb_lookup.find(page) != tlb_lookup.end()) {
            hits++;
            Node* node = tlb_lookup[page];
            if (node==head) continue;
            else{
                if (node->prev) node->prev->next=node->next;
                if (node->next) node->next->prev=node->prev;
                if (node==tail) tail=node->prev;
                node->next=head;
                node->prev=nullptr;
                if (head) head->prev=node;
                head= node;
                if (tlb_lookup.size()==1) tail=head;
            }
            tlb_lookup[page] = node; 

        } else {
            if (idx < tlbSize) {
                Node* node= new Node(page);
                node->next=head;
                if (head) head->prev=node;
                head= node;
                idx++;
                if (tlb_lookup.size()==0) tail=head;
                tlb_lookup[page]=node;
            } else {
                tlb_lookup.erase(tail->page);  
                Node* remove_node=tail;
                tail=tail->prev;
                delete remove_node;
                if (tail!=NULL) tail->next=nullptr;
                if (tlb_lookup.size()==1) head=tail;
                Node* node= new Node(page);
                node->next=head;
                if (head) head->prev=node;
                head= node;
                idx++;
                if (tlb_lookup.size()==0) tail=head;
                tlb_lookup[page]=node;
            }
        }
    }

    return hits; 
}

int* preprocess(int* memoryAccesses, int N, int pageSize) {
    unordered_map<int, int> nextPos; 
    int* next_occ = new int[N];  

    for (int i = 0; i < N; ++i) {
        next_occ[i] = N;
    }

    for (int i = N - 1; i >= 0; i--) {
        int page = (unsigned int )memoryAccesses[i]/(1024* pageSize);
        if (nextPos.find(page) != nextPos.end()) {
            next_occ[i] = nextPos[page]; 
        }
        nextPos[page] = i;
    }

    return next_occ;
}

int OPT(int* memoryAccesses, int N, int pageSize, int tlbSize) {
    int tlb[MAX_TLB_SIZE];  
    unordered_map<int, int> tlb_lookup;  
    int hits = 0;
    int idx = 0;

    int* next_occ = preprocess(memoryAccesses, N, pageSize);

    memset(tlb, -1, sizeof(tlb));

    for (int i = 0; i < N; ++i) {
        int page = (unsigned int )memoryAccesses[i]/(1024* pageSize);

        if (tlb_lookup.find(page) != tlb_lookup.end()) {
            tlb_lookup[page]=i;
            hits++; 
        } 
        else {
            if (idx < tlbSize) {
                tlb[idx] = page;
                tlb_lookup[page] = i;
                idx++;
            } 
            else {
                int replace_idx = -1, farthest = -1;
                for (int j = 0; j < tlbSize; ++j) {
                    int page_tlb = tlb[j];
                    int nextUse = next_occ[tlb_lookup[page_tlb]];

                    if (nextUse > farthest) {
                        farthest = nextUse;
                        replace_idx = j;
                    }
                }
                tlb_lookup.erase(tlb[replace_idx]);
                tlb[replace_idx] = page;
                tlb_lookup[page] = i;
            }
        }
    }

    delete[] next_occ;  

    return hits;
}


int main() {
    int tc;
    cin >> tc; 

    while (tc--) {
        int S, P, K, N;
        cin >> S >> P >> K >> N;

        int* memoryAccesses = new int[N];
        for (int i = 0; i < N; ++i) {
            unsigned int address;
            scanf("%x", &address);  
            memoryAccesses[i] = address;
        }

        int fifo = FIFO(memoryAccesses, N, P, K);
        int lifo = LIFO(memoryAccesses, N, P, K);
        int lru = LRU(memoryAccesses, N, P, K);
        int opt = OPT(memoryAccesses, N, P, K);
        // cout << opt<<' '<<flush;

        cout << fifo<< " " << lifo << " " << lru << " " << opt << endl;

        delete[] memoryAccesses;
    }

    return 0;
}