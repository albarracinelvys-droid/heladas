#include <assert.h>
#include <stdint.h>
#include "risk.h"
int main() {
    assert(classify(8, Risk::SensorError) == Risk::Normal);
    assert(classify(4, Risk::Normal) == Risk::Watch);
    assert(classify(2, Risk::Normal) == Risk::Alert);
    assert(classify(0, Risk::Normal) == Risk::Frost);
    assert(classify(-5, Risk::Normal) == Risk::Frost);
    assert(classify(0.4f, Risk::Frost) == Risk::Frost);
    assert(classify(0.5f, Risk::Frost) == Risk::Alert);
    assert(classify(2.4f, Risk::Alert) == Risk::Alert);
    assert(classify(2.5f, Risk::Alert) == Risk::Watch);
    assert(classify(4.4f, Risk::Watch) == Risk::Watch);
    assert(classify(4.5f, Risk::Watch) == Risk::Normal);
    assert(classify(-127, Risk::Frost) == Risk::SensorError);
    assert(classify(NAN, Risk::Normal) == Risk::SensorError);
    assert(classify(INFINITY, Risk::Normal) == Risk::SensorError);
    assert(classify(85, Risk::Normal) == Risk::SensorError);
    assert(classify(126, Risk::Normal) == Risk::SensorError);
    assert(classify(-56, Risk::Normal) == Risk::SensorError);
    assert(classify(-1, Risk::SensorError) == Risk::Frost);
    assert(windSpeed(0, 2000) == 0);
    assert(windSpeed(5, 0) == 0);
    assert(fabs(windSpeed(4, 2000) - 2.0f) < 0.001f);
    assert(fabs(windSpeed(4, 4000) - 1.0f) < 0.001f);
}
