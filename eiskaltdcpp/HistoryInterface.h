#ifndef HISTORYINTERFACE_H
#define HISTORYINTERFACE_H

#include <QStack>

template <typename T>
class HistoryInterface{
public:
    HistoryInterface(unsigned size = 20)
    {
        this->size = (size == 0)? 20 : size;
    }

    void push(T t){
        if (stack.size() == size)
            stack.erase(stack.begin());

        stack.push(t);
    }

    T pop(){
        return stack.pop();
    }

    bool isEmty(){
        return stack.empty();
    }

    void clear(){
        stack.clear();
    }

    void setSize(unsigned sz){
        size = (sz == 0)? size : sz;
        
        while (size <= stack.size())
            stack.erase(stack.begin());
    }

private:
    QStack<T> stack;

    unsigned size;
};

#endif // HISTORYINTERFACE_H
