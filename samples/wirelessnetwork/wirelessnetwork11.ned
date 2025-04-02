network wn11
{
    types:
        channel canal extends ned.DelayChannel{ delay = 100ms; }
    submodules:
        box11[6]: box11;
    connections:
        box11[0].out++ --> canal --> box11[1].in++;
        box11[1].out++ --> canal --> box11[0].in++;
        box11[1].out++ --> canal --> box11[2].in++;
        box11[2].out++ --> canal --> box11[1].in++;
        box11[1].out++ --> canal --> box11[4].in++;
        box11[4].out++ --> canal --> box11[1].in++;
        box11[3].out++ --> canal --> box11[4].in++;
        box11[4].out++ --> canal --> box11[3].in++;
        box11[4].out++ --> canal --> box11[5].in++;
        box11[5].out++ --> canal --> box11[4].in++;
}

simple box11
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        input in[];
        output out[];
}



