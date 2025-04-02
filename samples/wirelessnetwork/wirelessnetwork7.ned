network wn7
{
    @display("bgb=354,108");
    submodules:
        box7a: box7{
        parameters:
            @display("i=,cyan");
        };
        box7b: box7{
        parameters:
            @display("i=,red");
        };
    connections:
        box7a.out --> {  delay = 100ms; } --> box7b.in;
        box7b.out --> {  delay = 100ms; } --> box7a.in;
}

simple box7
{
    parameters:
        volatile double delayTime @unit(s);
        @display("i=block/routing");
    
    gates:
        input in;
        output out;
}

