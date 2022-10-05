#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <filesystem>

const int NUMBER_OF_THREADS = 8;


class ThreadStorage{
public:
    std::vector<std::thread> threads;
    explicit ThreadStorage(int number_of_threads){
    threads = std::vector<std::thread>(number_of_threads);
}

};

int main(){
    ThreadStorage threadStorage(NUMBER_OF_THREADS);



}