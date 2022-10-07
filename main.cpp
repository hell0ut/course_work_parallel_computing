#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <filesystem>
#include <mutex>

const int NUMBER_OF_THREADS = 8;
const int NUMBER_OF_FILES = 2000;

std::mutex console;


class ThreadStorage{
public:
    std::vector<std::thread> threads;
    explicit ThreadStorage(int number_of_threads){
        threads = std::vector<std::thread>(number_of_threads);
    }

};

class ParallelFileProcessor{
private:
    const std::filesystem::path p =std::filesystem::current_path();
    const std::vector<std::string> dir_paths{p.string()+R"(\data\test\neg\)",
                             p.string()+R"(\data\test\pos\)",
                             p.string()+R"(\data\train\neg\)",
                             p.string()+R"(\data\train\pos\)",
                             p.string()+R"(\data\train\unsup\)"
    };

    std::vector<std::string> files_paths;
    ThreadStorage &threadStorage;

    void generate_directories(){
        int i = 0;
        for (const auto& dir:dir_paths){
            for (const auto & entry : std::filesystem::directory_iterator(dir)){
                //std::cout<<entry.path().string()<<std::endl;
                files_paths[i]= entry.path().string();
                i++;
            }
        }
        std::cout<<"Directories have been read successfully"<<std::endl;
    }

public:
    explicit ParallelFileProcessor(ThreadStorage& threadStorage_):threadStorage(threadStorage_){
        files_paths=std::vector<std::string>(NUMBER_OF_FILES);
        generate_directories();
    };

    void print_directories(int number_of_rows){
        int i = 0;
        for (const std::string& path: files_paths)
        {
            std::cout << path << std::endl;
            i++;
            if (i==number_of_rows){
                break;
            }
        }
    }

    void apply_function_to_dir_files_parallel(void (*f)(std::vector<std::string>&,int,int)){

        std::vector<int> boundaries;
        int step = NUMBER_OF_FILES/NUMBER_OF_THREADS;
        for(int i = 0;i<NUMBER_OF_THREADS;i++){
            boundaries.push_back(step*i);
        }
        boundaries.push_back(NUMBER_OF_FILES);

        for(int i = 0;i<NUMBER_OF_THREADS;i++) {
            threadStorage.threads[i] = std::thread(f,std::ref(files_paths),boundaries[i],boundaries[i+1]);
        }

        for(int i = 0;i<NUMBER_OF_THREADS;i++) {
            threadStorage.threads[i].join();
        }

    }

    static void test_f(std::vector<std::string> &file_paths,int bd_low,int bd_high){
        console.lock();
        std::cout<<"First element " << file_paths[bd_low] << " Last element " << file_paths[bd_high-1]<< std::endl;
        console.unlock();
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
    BSTNode *root = nullptr;

    void inOrder(BSTNode* cur){
        if(cur == nullptr)
            return;
        inOrder(cur->left);
        std::cout<<cur->data<<" ";
        inOrder(cur->right);

    };

    BSTNode* insert(BSTNode* cur,int data){
        if(cur==nullptr){
            cur = new BSTNode(data, nullptr, nullptr);
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
    BSTSet(){

    };

    void InOrderTraversalPrint(){
        inOrder(root);
    }

    void Insert(int value){
        root =insert(root,value);
    }

};

struct LinkedListNode{
    int key;
    BSTSet* values;
    LinkedListNode* next;

    LinkedListNode(int _key,BSTSet* _values,LinkedListNode* _next){
    key=_key;
    values=_values;
    next=_next;
    };
};

class LinkedList{
private:
    LinkedListNode* head;

    LinkedListNode* insert(LinkedListNode* cur,int key,int value){
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

public:
    LinkedList():head(nullptr) {}

    void Insert(int key,int value){
        head = insert(head,key,value);
    }

    void Print(){
        print(head);
    }

};


int main(){
    //ThreadStorage threadStorage(NUMBER_OF_THREADS);
    //ParallelFileProcessor parallelFileProcessor(threadStorage);
    //parallelFileProcessor.apply_function_to_dir_files_parallel(test_f);

    LinkedList list;

    list.Insert(0,1);
    list.Insert(0,2);
    list.Insert(0,3);
    list.Insert(0,1);



    list.Insert(1,3);
    list.Insert(1,4);
    list.Insert(1,3);

    list.Insert(2,22);
    list.Insert(2,23);
    list.Print();
/*    0 : 1 2 3
    1 : 3 4
    2 : 22 23*/

    return 1;






}