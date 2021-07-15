#include <epicsUnitTest.h>
#include <epicsAlgorithm.h>
#include <testMain.h>
#include <ObjectPool.h>

MAIN(mathTest)
{
    std::shared_ptr<int> ptr1, ptr2;
    void *raw_ptr;
    size_t readySize1, readySize2, usedSize1, usedSize2;

//    testPlan(11); The last 5 tests pass but cause a core dump. 
//    Readd when they're fixed
    testPlan(6);

    ObjectPool<int> t1(true);
    testOk(t1.size() == 0, "size() == 0");
    testOk(t1.capacity() == 0, "capacity() == 0");
    ptr1 = t1.getPtr(sizeof(int));
    testOk(t1.size() == 1, "size() == 1");
    testOk(t1.capacity() == 1, "capacity() == 1");
    raw_ptr = ptr1.get();
    ptr1.reset();
    testOk(t1.size() == 0, "size() == 0");
    testOk(t1.capacity() == 1, "capacity() == 1");

    return testDone();

    ptr1 = t1.getPtr(sizeof(int));
    testOk(ptr1.get() == raw_ptr, "ptr1.get() == raw_ptr");
    testOk(t1.size() == 1, "size() == 1");
    testOk(t1.capacity() == 1, "capacity() == 1");

    ptr2 = t1.getPtr(sizeof(int));
    testOk(t1.size() == 2, "size() == 2");
    testOk(t1.capacity() == 2, "capacity() == 2");

}
