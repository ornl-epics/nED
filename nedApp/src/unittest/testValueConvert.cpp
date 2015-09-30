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
    conv.reset(new UnsignConvert(9));
    testOk1(conv->fromRaw(0) == 0);
    testOk1(conv->fromRaw(255) == 255);
    testOk1(conv->fromRaw(256) == 256);
    testOk1(conv->fromRaw(511) == 511);
    testOk1(conv->fromRaw(512) == 512);

    testOk1(conv->toRaw(0) == 0);
    testOk1(conv->toRaw(255) == 255);
    testOk1(conv->toRaw(256) == 256);
    testOk1(conv->toRaw(511) == 511);
    testOk1(conv->toRaw(512) == 0);
    testOk1(conv->toRaw(-1) == 0);
    testOk1(conv->toRaw(-255) == 0);
    testOk1(conv->toRaw(-256) == 0);
    testOk1(conv->toRaw(-511) == 0);
    testOk1(conv->toRaw(-512) == 0);

    testOk1(conv->checkBounds(0) == true);
    testOk1(conv->checkBounds(255) == true);
    testOk1(conv->checkBounds(256) == true);
    testOk1(conv->checkBounds(511) == true);
    testOk1(conv->checkBounds(512) == false);
    testOk1(conv->checkBounds(-1) == false);
    testOk1(conv->checkBounds(-255) == false);
    testOk1(conv->checkBounds(-256) == false);
    testOk1(conv->checkBounds(-511) == false);
    testOk1(conv->checkBounds(-512) == false);

    testDiag("Sign2sComplementConvert class");
    conv.reset(new Sign2sComplementConvert(9));
    testOk1(conv->fromRaw(0) == 0);
    testOk1(conv->fromRaw(255) == 255);
    testOk1(conv->fromRaw(256) == -256);
    testOk1(conv->fromRaw(511) == -1);
    testOk1(conv->fromRaw(512) == 0);

    testOk1(conv->toRaw(0) == 0);
    testOk1(conv->toRaw(255) == 255);
    testOk1(conv->toRaw(256) == 0);
    testOk1(conv->toRaw(511) == 0);
    testOk1(conv->toRaw(512) == 0);
    testOk1(conv->toRaw(-1) == 511);
    testOk1(conv->toRaw(-255) == 257);
    testOk1(conv->toRaw(-256) == 256);
    testOk1(conv->toRaw(-511) == 0);
    testOk1(conv->toRaw(-512) == 0);

    testOk1(conv->checkBounds(0) == true);
    testOk1(conv->checkBounds(255) == true);
    testOk1(conv->checkBounds(256) == false);
    testOk1(conv->checkBounds(511) == false);
    testOk1(conv->checkBounds(512) == false);
    testOk1(conv->checkBounds(-1) == true);
    testOk1(conv->checkBounds(-255) == true);
    testOk1(conv->checkBounds(-256) == true);
    testOk1(conv->checkBounds(-511) == false);
    testOk1(conv->checkBounds(-512) == false);


    testDiag("SignMagnitudeConvert class");
    conv.reset(new SignMagnitudeConvert(9));
    testOk1(conv->fromRaw(0) == 0);
    testOk1(conv->fromRaw(255) == 255);
    testOk1(conv->fromRaw(256) == 0);
    testOk1(conv->fromRaw(511) == -255);
    testOk1(conv->fromRaw(512) == 0);

    testOk1(conv->toRaw(0) == 0);
    testOk1(conv->toRaw(255) == 255);
    testOk1(conv->toRaw(256) == 0);
    testOk1(conv->toRaw(511) == 0);
    testOk1(conv->toRaw(512) == 0);
    testOk1(conv->toRaw(-1) == 257);
    testOk1(conv->toRaw(-255) == 511);
    testOk1(conv->toRaw(-256) == 0);
    testOk1(conv->toRaw(-511) == 0);
    testOk1(conv->toRaw(-512) == 0);

    testOk1(conv->checkBounds(0) == true);
    testOk1(conv->checkBounds(255) == true);
    testOk1(conv->checkBounds(256) == false);
    testOk1(conv->checkBounds(511) == false);
    testOk1(conv->checkBounds(512) == false);
    testOk1(conv->checkBounds(-1) == true);
    testOk1(conv->checkBounds(-255) == true);
    testOk1(conv->checkBounds(-256) == false);
    testOk1(conv->checkBounds(-511) == false);
    testOk1(conv->checkBounds(-512) == false);

    testDiag("Regression tests");
    conv.reset(new UnsignConvert(32));
    testOk1(conv->checkBounds(0) == true);
    testOk1(conv->checkBounds(1792) == true);

    return testDone();
}
