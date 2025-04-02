network wn9
{
    @display("bgb=354,108");
    submodules:
        box9a: box9a{
        parameters:
            @display("i=,cyan");
            
        };
        box9b: box9b{
        parameters:
            @display("i=,red");
        };
    connections:
        box9a.out --> {  delay = 100ms; } --> box9b.in;
        box9b.out --> {  delay = 100ms; } --> box9a.in;
}

simple box9a
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        input in;
        output out;
}

simple box9b
{
    parameters:
        @display("i=block/rxtx");
    
    gates:
        input in;
        output out;
}

