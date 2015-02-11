#include "munit.h"
#include "asserts.h"
#include "../nodetypes.h"
#include "../matrix.private.h"
#include "../matrix.h"
#include "../definitions.h"

void testSum(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_SUM);
    aNode.params[0] = 1;
    aNode.params[1] = 2;
    aNode.params[2] = 4;
    aNode.paramsInUse = 3;
    aNode.paramIsConstant = 0b00000111;
    addNode(&aNode);
    
    runMatrix();
    
    assertEquals(7,aNode.result,"sum");
}

void testMultiply(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_MULTIPLY);
    aNode.params[0] = 2;
    aNode.params[1] = 4;
    aNode.params[2] = 8;
    aNode.paramsInUse = 3;
    aNode.paramIsConstant = 0b00000111;
    addNode(&aNode);
    
    runMatrix();

    assertEquals(64,aNode.result,"product");
}

void testInvertPositive(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_INVERT);
    aNode.params[0] = 10;
    aNode.paramIsConstant = 0b00000001;
    addNode(&aNode);

    runMatrix();

    assertEquals(-10,aNode.result,"invert positive");
}

void testInvertNegative(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_INVERT);
    aNode.params[0] = -10;
    aNode.paramIsConstant = 0b00000001;
    addNode(&aNode);

    runMatrix();

    assertEquals(10,aNode.result,"invert negative");
}

// special case as max positive is 1 less than max negative
void testInvertMaxNegative(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_INVERT);
    aNode.params[0] = MAX_NEGATIVE;
    aNode.paramIsConstant = 0b00000001;
    addNode(&aNode);

    runMatrix();

    assertEquals(MAX_POSITIVE,aNode.result,"invert max negative");
}

void testInvertZero(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_INVERT);
    aNode.params[0] = 0;
    aNode.paramIsConstant = 0b00000001;
    addNode(&aNode);

    runMatrix();

    assertEquals(0,aNode.result,"invert zero");
}

void testInvertEachSideZero(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_INVERT_EACH_SIDE);
    aNode.params[0] = 0;
    aNode.paramIsConstant = 0b00000001;
    addNode(&aNode);

    runMatrix();

    assertEquals(MAX_POSITIVE,aNode.result,"invert each side zero");
}

void testInvertEachSidePositive(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_INVERT_EACH_SIDE);
    aNode.params[0] = MAX_POSITIVE;
    aNode.paramIsConstant = 0b00000001;
    addNode(&aNode);

    runMatrix();

    assertEquals(0,aNode.result,"invert each side positive");
}

void testInvertEachSideNegative(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_INVERT_EACH_SIDE);
    aNode.params[0] = MAX_NEGATIVE;
    aNode.paramIsConstant = 0b00000001;
    addNode(&aNode);

    runMatrix();

    assertEquals(-1,aNode.result,"invert each side negative");
}

void testDelayLine(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_DELAY_LINE);
    aNode.params[0] = 10;
    aNode.paramIsConstant = 0b00000001;
    aNode.result = 0;
    addNode(&aNode);

    assertEquals(0,aNode.result,"delay line precondition");
    
    runMatrix();

    assertEquals(10,aNode.result,"delay line");
}

void testMemorySet(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_MEMORY);
    aNode.params[0] = 10;
    aNode.params[1] = 1; // should set
    aNode.params[2] = 0; // should not clear
    aNode.paramIsConstant = 0b00000111;
    addNode(&aNode);

    runMatrix();

    assertEquals(10,aNode.result,"memory set");
}

void testMemoryHold(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_MEMORY);
    aNode.result = 5; //value to hold - leave unchanged
    aNode.params[0] = 10;
    aNode.params[1] = 0; // should not set
    aNode.params[2] = 0; // should not clear
    aNode.paramIsConstant = 0b00000111;
    addNode(&aNode);

    runMatrix();

    assertEquals(5,aNode.result,"memory hold");
}

void testMemoryClear(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_MEMORY);
    aNode.result = 7; //initial value
    aNode.params[0] = 10;
    aNode.params[1] = 0; // should not set
    aNode.params[2] = 1; // should clear
    aNode.paramIsConstant = 0b00000111;
    addNode(&aNode);

    runMatrix();

    assertEquals(0,aNode.result,"memory hold");
}

void testLfoPulse(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_LFO_PULSE);
    aNode.params[0] = 3; // cycle width = 3
    aNode.params[1] = 2; // pulse width = 2
    aNode.params[2] = 1; // trigger pulse start (not necessary though)
    aNode.params[3] = 10; // max positive
    aNode.params[4] = -5; // max negative
    aNode.params[5] = 0b00000001; // start on top
    aNode.paramIsConstant = 0b00111111;
    aNode.result = 0;
    addNode(&aNode);

    runMatrix(); // on the first run, trigger should cause the output to go high and reset the iteration counter

    assertEquals(10,aNode.result,"LFO Pulse trigger");
    assertEquals(0,aNode.highResState,"LFO Pulse init iterator");
    
    aNode.params[2] = 0; // remove trigger

    runMatrix();

    // after one cycle, nothing should have changed except the counter
    assertEquals(10,aNode.result,"LFO Pulse step 2");
    assertEquals(1,aNode.highResState,"LFO Pulse iterator increment");

    runMatrix();
    
    // after two cycles we should have reached the pulse length and the lfo should drop to its minimum value
    assertEquals(-5,aNode.result,"LFO Pulse step 3");

    runMatrix();

    // after three cycles, we should be back to start:
    assertEquals(10,aNode.result,"LFO Pulse step 4");
    assertEquals(0,aNode.highResState,"LFO Pulse iterator reset");

}

void testLfoPulseRetrigger(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_LFO_PULSE);
    aNode.params[0] = 3; // cycle width = 3
    aNode.params[1] = 2; // pulse width = 2
    aNode.params[2] = 1; // trigger pulse start (not necessary though)
    aNode.params[3] = 10; // max positive
    aNode.params[4] = -5; // max negative
    aNode.params[5] = 0b00000001; // start on top
    aNode.paramIsConstant = 0b00111111;
    aNode.result = 0;
    addNode(&aNode);

    runMatrix();

    // on the first run, trigger should cause the output to go high and reset 
    // the iteration counter
    assertEquals(10,aNode.result,"LFO Pulse trigger");
    assertEquals(0,aNode.highResState,"LFO Pulse init iterator");

    runMatrix();

    // running again with the trigger high should reset state
    assertEquals(10,aNode.result,"LFO Pulse retrigger");
    assertEquals(0,aNode.highResState,"LFO Pulse reset iterator");
}

void testLfoPulseStartOnBottom(){
    Node aNode;
    aNode.func = getFunctionPointer(NODE_LFO_PULSE);
    aNode.params[0] = 3; // cycle width = 3
    aNode.params[1] = 2; // pulse width = 2
    aNode.params[2] = 1; // trigger pulse start (not necessary though)
    aNode.params[3] = 10; // max positive
    aNode.params[4] = -5; // max negative
    aNode.params[5] = 0b00000000; // start on bottom
    aNode.paramIsConstant = 0b00111111;
    aNode.result = 0;
    addNode(&aNode);

    runMatrix();

    // on the first run, trigger should cause the output to go low
    assertEquals(-5,aNode.result,"LFO Pulse start low");
}

// TODO: retrigger, negative start.

// setup and run test suite
void runMatrixTests(){
    reset();
    /*
    add(&testSum);
    add(&testMultiply);
    
    add(&testInvertPositive);
    add(&testInvertNegative);
    add(&testInvertMaxNegative);
    add(&testInvertZero);

    add(&testInvertEachSideZero);
    add(&testInvertEachSidePositive);
    add(&testInvertEachSideNegative);
    
    add(&testDelayLine);

    add(&testMemorySet);
    add(&testMemoryHold);
    add(&testMemoryClear);*/
    
    add(&testLfoPulse);
    add(&testLfoPulseRetrigger);
    add(&testLfoPulseStartOnBottom);
    
    run(resetMatrix);
}

// TODO void nodeFuncRamp(Node *aNode){
// TODO void nodeFuncLfoPulse(Node *aNode){