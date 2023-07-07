#ifndef _JOINER_H_
#define _JOINER_H_

#include <vector>
#include <thread>

namespace util {

class Joiner
{   
    public:
        explicit Joiner(std::vector<std::thread> &thread_list_): thread_list(thread_list_)
        {}

        ~Joiner()
        {
            for (auto &&th : thread_list)
            {
                if (th.joinable())
                {
                    th.join();
                }
            }
        }

    private:
        std::vector<std::thread> &thread_list;
};

}

#endif