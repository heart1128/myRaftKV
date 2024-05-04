#include <iostream>
#include <string>
#include "skipList.h"

int main()
{
    SkipList<std::string, std::string> skipList(5);

    skipList.insertElement("name", "heart");
    skipList.insertElement("age", "20");
    skipList.insertElement("gender", "男");
    skipList.displayList();
    return 0;
}
