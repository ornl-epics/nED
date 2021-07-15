#include <epicsUnitTest.h>
#include <epicsAlgorithm.h>
#include <testMain.h>
#include <ValueConvert.h>

#include <math.h>
#include <memory>
#include <string.h>

#define TEST_OK     1
#define TEST_FAIL   0


MAIN(mathTest)
{
    uint32_t bufSize, pushSize1, pushSize2, pushSize3, consumeSize1, consumeSize2;
    std::shared_ptr<BaseConvert> conv;

    testPlan(75);

    testDiag("UnsignConvert class");
    conv.reset(new UnsignConvert());
    testOk1(conv->fromRaw(0, 9) == 0);
    testOk1(conv->fromRaw(255, 9) == 255);
    testOk1(conv->fromRaw(256, 9) == 256);
    testOk1(conv->fromRaw(511, 9) == 511);
    testOk1(conv->fromRaw(512, 9) == 512);

    testOk1(conv->toRaw(0, 9) == 0);
    testOk1(conv->toRaw(255, 9) == 255);
    testOk1(conv->toRaw(256, 9) == 256);
    testOk1(conv->toRaw(511, 9) == 511);
    testOk1(conv->toRaw(512, 9) == 0);
    testOk1(conv->toRaw(-1, 9) == 0);
    testOk1(conv->toRaw(-255, 9) == 0);
    testOk1(conv->toRaw(-256, 9) == 0);
    testOk1(conv->toRaw(-511, 9) == 0);
    testOk1(conv->toRaw(-512, 9) == 0);

    testOk1(conv->checkBounds(0, 9) == true);
    testOk1(conv->checkBounds(255, 9) == true);
    testOk1(conv->checkBounds(256, 9) == true);
    testOk1(conv->checkBounds(511, 9) == true);
    testOk1(conv->checkBounds(512, 9) == false);
    testOk1(conv->checkBounds(-1, 9) == false);
    testOk1(conv->checkBounds(-255, 9) == false);
    testOk1(conv->checkBounds(-256, 9) == false);
    testOk1(conv->checkBounds(-511, 9) == false);
    testOk1(conv->checkBounds(-512, 9) == false);

    testDiag("Sign2sComplementConvert class");
    conv.reset(new Sign2sComplementConvert());
    testOk1(conv->fromRaw(0, 9) == 0);
    testOk1(conv->fromRaw(255, 9) == 255);
    testOk1(conv->fromRaw(256, 9) == -256);
    testOk1(conv->fromRaw(511, 9) == -1);
    testOk1(conv->fromRaw(512, 9) == 0);

    testOk1(conv->toRaw(0, 9) == 0);
    testOk1(conv->toRaw(255, 9) == 255);
    testOk1(conv->toRaw(256, 9) == 0);
    testOk1(conv->toRaw(511, 9) == 0);
    testOk1(conv->toRaw(512, 9) == 0);
    testOk1(conv->toRaw(-1, 9) == 511);
    testOk1(conv->toRaw(-255, 9) == 257);
    testOk1(conv->toRaw(-256, 9) == 256);
    testOk1(conv->toRaw(-511, 9) == 0);
    testOk1(conv->toRaw(-512, 9) == 0);

    testOk1(conv->checkBounds(0, 9) == true);
    testOk1(conv->checkBounds(255, 9) == true);
    testOk1(conv->checkBounds(256, 9) == false);
    testOk1(conv->checkBounds(511, 9) == false);
    testOk1(conv->checkBounds(512, 9) == false);
    testOk1(conv->checkBounds(-1, 9) == true);
    testOk1(conv->checkBounds(-255, 9) == true);
    testOk1(conv->checkBounds(-256, 9) == true);
    testOk1(conv->checkBounds(-511, 9) == false);
    testOk1(conv->checkBounds(-512, 9) == false);


    testDiag("SignMagnitudeConvert class");
    conv.reset(new SignMagnitudeConvert());
    testOk1(conv->fromRaw(0, 9) == 0);
    testOk1(conv->fromRaw(255, 9) == 255);
    testOk1(conv->fromRaw(256, 9) == 0);
    testOk1(conv->fromRaw(511, 9) == -255);
    testOk1(conv->fromRaw(512, 9) == 0);

    testOk1(conv->toRaw(0, 9) == 0);
    testOk1(conv->toRaw(255, 9) == 255);
    testOk1(conv->toRaw(256, 9) == 0);
    testOk1(conv->toRaw(511, 9) == 0);
    testOk1(conv->toRaw(512, 9) == 0);
    testOk1(conv->toRaw(-1, 9) == 257);
    testOk1(conv->toRaw(-255, 9) == 511);
    testOk1(conv->toRaw(-256, 9) == 0);
    testOk1(conv->toRaw(-511, 9) == 0);
    testOk1(conv->toRaw(-512, 9) == 0);

    testOk1(conv->checkBounds(0, 9) == true);
    testOk1(conv->checkBounds(255, 9) == true);
    testOk1(conv->checkBounds(256, 9) == false);
    testOk1(conv->checkBounds(511, 9) == false);
    testOk1(conv->checkBounds(512, 9) == false);
    testOk1(conv->checkBounds(-1, 9) == true);
    testOk1(conv->checkBounds(-255, 9) == true);
    testOk1(conv->checkBounds(-256, 9) == false);
    testOk1(conv->checkBounds(-511, 9) == false);
    testOk1(conv->checkBounds(-512, 9) == false);

    testDiag("Regression tests");
    conv.reset(new UnsignConvert());
    testOk1(conv->checkBounds(0, 32) == true);
    testOk1(conv->checkBounds(1792, 32) == true);

    return testDone();
}
