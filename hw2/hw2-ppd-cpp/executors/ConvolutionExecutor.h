#ifndef HW2_PPD_CPP_CONVOLUTIONEXECUTOR_H
#define HW2_PPD_CPP_CONVOLUTIONEXECUTOR_H

#include <vector>
#include <thread>

using namespace std;

class ConvolutionExecutor {
public:
    virtual ~ConvolutionExecutor() = default;
    virtual void executeTask(int p) = 0;

    static void startAndJoin(vector<thread>& threads) {
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
};

#endif //HW2_PPD_CPP_CONVOLUTIONEXECUTOR_H
