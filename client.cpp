
#include <iostream>
#include <mutex>

std::mutex console;

int main(){
    console.lock();
    console.unlock();
    std::cout<<sizeof(console);
    return 0;
}
