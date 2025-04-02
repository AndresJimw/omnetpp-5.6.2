network wn10
{
    submodules:
        box10[6]: box10;
    connections:
        box10[0].out++ --> {  delay = 100ms; } --> box10[1].in++;
        box10[1].out++ --> {  delay = 100ms; } --> box10[0].in++;
        box10[1].out++ --> {  delay = 100ms; } --> box10[2].in++;
        box10[2].out++ --> {  delay = 100ms; } --> box10[1].in++;
        box10[1].out++ --> {  delay = 100ms; } --> box10[4].in++;
        box10[4].out++ --> {  delay = 100ms; } --> box10[1].in++;
        box10[3].out++ --> {  delay = 100ms; } --> box10[4].in++;
        box10[4].out++ --> {  delay = 100ms; } --> box10[3].in++;
        box10[4].out++ --> {  delay = 100ms; } --> box10[5].in++;
        box10[5].out++ --> {  delay = 100ms; } --> box10[4].in++;
}

simple box10
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        input in[];
        output out[];
}



