#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <filesystem>
#include <mutex>
#include <fstream>
#include <atomic>
#include <string>
#include <cmath>
#include <chrono>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>


const int NUMBER_OF_FILES = 2000;
int NUMBER_OF_THREADS = 12;
#define prot IPPROTO_TCP;
std::mutex console;


std::vector<std::filesystem::path> files_paths;

class ThreadStorage{
public:
    std::vector<std::thread> threads;
    explicit ThreadStorage(int number_of_threads){
        threads = std::vector<std::thread>(number_of_threads);
    }

};


struct BSTNode{
    int data;
    BSTNode* left;
    BSTNode* right;

    BSTNode(int data_,BSTNode* left_,BSTNode* right_) :data(data_),left(left_),right(right_){}

};

class BSTSet{

private:
    void inOrder(BSTNode* cur){
        if(cur == nullptr)
            return;
        inOrder(cur->left);
        std::cout<<cur->data<<" ";
        inOrder(cur->right);

    };

    void inOrderString(BSTNode* cur,std::string &response,bool indexedView){
        if(cur == nullptr)
            return;
        inOrderString(cur->left,response,indexedView);
        int ind = cur->data;
        if (indexedView){
            response+=std::to_string(ind)+" ";
        }
        else{
            std::filesystem::path cur_path = files_paths[ind];
            response+= cur_path.parent_path().parent_path().filename().string()+
                    "/"+cur_path.parent_path().filename().string()+
                    "/"+cur_path.filename().string() +
                    "\n";
        }
        inOrderString(cur->right,response,indexedView);

    };

    BSTNode* insert(BSTNode* cur,int data){
        if(cur==nullptr){
            cur = new BSTNode(data, nullptr, nullptr);
            ++cardinality;
        }
        else if (data>cur->data){
            cur->right= insert(cur->right,data);
        }
        else if (data<cur->data) {
            cur->left= insert(cur->left,data);
        }
        return cur;

    };
public:
    BSTNode *root;
    int cardinality;

    BSTSet():root(nullptr),cardinality(0){};

    void InOrderTraversalPrint(){
        inOrder(root);
    }

    std::string InOrderTraversalString(bool IndexedView){
        std::string response;
        inOrderString(root,response,IndexedView);
        return response;

    }

    void Insert(int value){
        root =insert(root,value);
    }

};


struct LinkedListNode{
    std::string key;
    BSTSet* values;
    LinkedListNode* next;

    LinkedListNode(const std::string &_key,BSTSet* _values,LinkedListNode* _next){
    key=_key;
    values=_values;
    next=_next;
    };
};

class LinkedList{
private:
    LinkedListNode* head;

    LinkedListNode* insert(LinkedListNode* cur,const std::string& key,int value){
        if (cur == nullptr) {
            cur = new LinkedListNode(key, new BSTSet, nullptr);
            cur->values->Insert(value);
        }
        else if(key==cur->key){
            cur->values->Insert(value);
        }
        else{
            cur->next = insert(cur->next,key,value);
        }
        return cur;
    }

    void print(LinkedListNode* cur){
        if (cur!= nullptr){
            std::cout<<cur->key<<" : ";
            cur->values->InOrderTraversalPrint();
            std::cout<<std::endl;
            print(cur->next);
        }
    }

    BSTSet* find(LinkedListNode* cur,const std::string &key){
        if (cur!= nullptr){
            if (cur->key==key){
            return cur->values;
            }
        }
        return nullptr;
    }


public:
    LinkedList():head(nullptr) {}

    void Insert(const std::string &key,int value){
        head = insert(head,key,value);
    }

    void Print(){
        print(head);
    }

    BSTSet* Find(const std::string &key){
        return find(head,key);
    }

};


class ConcurrentHashTable{
private:
    const int PRIME_CONST = 3;
    int SIZE;
    float MAX_FILL_FACTOR;
    float FILL_FACTOR;
    std::vector<LinkedList> HashTable;
    std::vector<std::mutex> locks;
    BSTSet filled_indexes;
    std::mutex filled_indexes_lock;
    std::atomic<int> number_of_words = 0;

    int LOCK_STEP;

    int eval_hash(const std::string &word) {
        long hashVal = 0;
        for(int i = 0;i<word.size();i++) {
            hashVal += word[i] * 3*i+word[i]*3;
        }
        return hashVal%SIZE;
    }

    void inOrderPrint(BSTNode* cur) {
        if (cur == nullptr)
            return;
        inOrderPrint(cur->left);
        HashTable[cur->data].Print();
        inOrderPrint(cur->right);
    }


public:

    ConcurrentHashTable(int size,float max_fill_factor,int locks_size):
        HashTable(std::vector<LinkedList>(size)),
        SIZE(size),MAX_FILL_FACTOR(max_fill_factor)
    {
            if (locks_size>size/ 2){
                LOCK_STEP = 2;
                locks=std::vector<std::mutex>(size/2);
            }
            else{
                LOCK_STEP = size/locks_size;
                locks = std::vector<std::mutex>(locks_size);

            }
        };

    void Insert(const std::string  &key,int value){
        int index = abs(eval_hash(key));
        int lock_ind = index/LOCK_STEP;
        filled_indexes_lock.lock();
        filled_indexes.Insert(index);
        FILL_FACTOR = (float)filled_indexes.cardinality/(float)SIZE;
        number_of_words++;
        //if (number_of_words%10000==0){
            //std::cout<<"NOF " << number_of_words<<std::endl;
           // std::cout<<"Fill factor is " << FILL_FAC  TOR<<" for "<<number_of_words<<" words"<<std::endl;
        //}
        filled_indexes_lock.unlock();
        locks[lock_ind].lock();
        HashTable[index].Insert(key,value);
        locks[lock_ind].unlock();
    }

    std::string Find(const std::string& key, bool indexedView){
        std::string response;
        int index = eval_hash(key);
        auto set = HashTable[index].Find(key);
        if(set!= nullptr){
            response += "For word <<" + key+ ">> : \n";
            response+=set->InOrderTraversalString(indexedView);
            response += "\n";
        }
        else{
            response += "No word <<" + key +">> in the index table.\n";
        }
        return response;
    }

    void Print(){
        inOrderPrint(filled_indexes.root);
    }


};

ConcurrentHashTable hashTable(20000,0.75,1000);

class ParallelFileProcessor{
private:
    const std::filesystem::path p =std::filesystem::current_path()/std::filesystem::path("data");
    const std::filesystem::path train ="train";
    const std::filesystem::path test ="test";
    const std::filesystem::path neg = "neg";
    const std::filesystem::path pos = "pos";
    const std::filesystem::path unsup = "unsup";
    const std::vector<std::filesystem::path> dir_paths
    {
        p/test/neg,
        p/test/pos,
        p/train/neg,
        p/train/neg,
        p/train/unsup
    };


    ThreadStorage &threadStorage;

    void generate_directories(){
        int i = 0;
        for (const auto& dir:dir_paths){
            for (const auto & entry : std::filesystem::directory_iterator(dir)){
                //std::cout<<entry.path().string()<<std::endl;
                files_paths[i]= entry.path();
                i++;
            }
        }
        std::cout<<"Directories have been read successfully"<<std::endl;
    }

    static void add_files_to_hashtable(int bd_low,int bd_high){
        for(int i = bd_low;i<bd_high;i++){
            std::ifstream f(files_paths[i]);
            std::stringstream buffer;
            buffer << f.rdbuf();
            std::string word;
            while (buffer >> word)
            {
                hashTable.Insert(word,i);
            }
            //break;

        }
    }

    void apply_function_to_dir_files_parallel(void (*f)(int,int)){

        std::vector<int> boundaries;
        int step = NUMBER_OF_FILES/NUMBER_OF_THREADS;
        for(int i = 0;i<NUMBER_OF_THREADS;i++){
            boundaries.push_back(step*i);
        }
        boundaries.push_back(NUMBER_OF_FILES);

        for(int i = 0;i<NUMBER_OF_THREADS;i++) {
            threadStorage.threads[i] = std::thread(f,
                                                   boundaries[i],boundaries[i+1]);
        }

        for(int i = 0;i<NUMBER_OF_THREADS;i++) {
            threadStorage.threads[i].join();
        }

    }




public:
    explicit ParallelFileProcessor(ThreadStorage& threadStorage_):
    threadStorage(threadStorage_){
        files_paths=std::vector<std::filesystem::path>(NUMBER_OF_FILES);
        generate_directories();
    };

    void print_directories(int number_of_rows){
        int i = 0;
        for (const std::filesystem::path& path: files_paths)
        {
            std::cout << path << std::endl;
            i++;
            if (i==number_of_rows){
                break;
            }
        }
    }


    void CreateInvertedIndex(){
        apply_function_to_dir_files_parallel(add_files_to_hashtable);
        std::cout<<"Inverted Index was created successfully"<<std::endl;
    }

};


int shutdown_services(ADDRINFO* addrResult, SOCKET* ConnectSocket, std::string message, int result) {
    console.lock();
    std::cout << message << " " << result << std::endl;
    console.unlock();
    if (ConnectSocket != NULL) {
        closesocket(*ConnectSocket);
        *ConnectSocket = INVALID_SOCKET;
    }
    freeaddrinfo(addrResult);
    WSACleanup();
    return 1;
}

struct client {
    int id;
    SOCKET socket = INVALID_SOCKET;
    client( SOCKET sock, int id_) :id(id_), socket(sock) {};

};

class InvertedIndexServer{
private:
    SOCKET ListenSocket = INVALID_SOCKET;
    int result;
    ADDRINFO hints;
    ADDRINFO* addrResult = nullptr;



    int speak_client(int client_id) {
        char recvBuffer[512];
        bool close_connection = false;
        while (!close_connection) {
            ZeroMemory(recvBuffer, 512);
            int client_index = get_client_index_by_id(client_id);
            result = recv(clients[client_index].socket, recvBuffer, 512, 0);

            if (result > 0) {
                console.lock();
                std::cout << "Receieved data from client " << client_id << ":\n" << recvBuffer << std::endl;
                console.unlock();
                iasa_request cur_req = IASA_REQUEST_DECODER::char_to_request(recvBuffer);
                auto response = process_request(cur_req, client_index, close_connection);

                result = send(clients[client_index].socket, response.c_str(), (int) strlen(response.c_str()), 0);
                console.lock();
                std::cout << "Send data back to client" << client_id << ":\n" << response << std::endl;
                console.unlock();
                if (result == SOCKET_ERROR)
                    return shutdown_services(addrResult, &clients[client_index].socket,
                                             "Sending data back failed, result = ", result);
                if (close_connection) {
                    break;
                }

            }
        }
    }

    int listen_for_clients() {
        int i = 0;

        while (true) {
            console.lock();
            std::cout << "Server is waiting for connection" << std::endl;
            console.unlock();
            SOCKET ClientSocket = SOCKET_ERROR;
            while (ClientSocket == SOCKET_ERROR)
            {
                ClientSocket = accept(ListenSocket, nullptr, nullptr);
                if (ClientSocket == INVALID_SOCKET) return shutdown_services(addrResult, &ListenSocket, "Accepting socket failed, result = ", result);
            }
            std::thread th;
            client cur_client(ClientSocket,i);
            std::thread thread(&InvertedIndexServer::speak_client, this, i);
            thread.detach();
            console.lock();
            std::cout << "Client connected" << std::endl;
            console.unlock();
            i++;

        }
    }


    int start_server() {
        WSADATA wsaData;
        result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) return shutdown_services(addrResult, nullptr, "WSAStartup failed, result = ", result);

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = prot;
        hints.ai_flags = AI_PASSIVE;

        result = getaddrinfo(nullptr, "666", &hints, &addrResult);
        if (result != 0) return shutdown_services(addrResult, nullptr, "getaddrinfo failed, result = ", result);

        ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
        if (ListenSocket == INVALID_SOCKET) return shutdown_services(addrResult, &ListenSocket, "Socket creation failed, result = ", result);

        result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
        if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ListenSocket, "Binding socket failed, result = ", result);

        result = listen(ListenSocket, SOMAXCONN);
        if (result == SOCKET_ERROR) return shutdown_services(addrResult, &ListenSocket, "Listening socket failed, result = ", result);

        listen_for_clients();

    }

public:
    void Run(int number_of_threads){
        NUMBER_OF_THREADS = number_of_threads;
        ThreadStorage threadStorage(NUMBER_OF_THREADS);
        ParallelFileProcessor parallelFileProcessor(threadStorage);
        //count time
        auto start = std::chrono::high_resolution_clock::now();
        parallelFileProcessor.CreateInvertedIndex();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds >(stop - start);

        std::cout << "Time taken by function using " <<NUMBER_OF_THREADS <<" threads: "
                  << duration.count() << " milliseconds" << std::endl;

        start_server();


    }
};


int main(){
    InvertedIndexServer invertedIndexServer;
    invertedIndexServer.Run(6);


    return 1;

}