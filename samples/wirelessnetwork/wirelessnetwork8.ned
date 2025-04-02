network wn8
{
    @display("bgb=354,108");
    submodules:
        box8a: box8a{
        parameters:
            @display("i=,cyan");
            
        };
        box8b: box8b{
        parameters:
            @display("i=,red");
        };
    connections:
        box8a.out --> {  delay = 100ms; } --> box8b.in;
        box8b.out --> {  delay = 100ms; } --> box8a.in;
}

simple box8a
{
    parameters:
        @display("i=block/routing");
    
    gates:
        input in;
        output out;
}

simple box8b
{
    parameters:
        @display("i=block/routing");
    
    gates:
        input in;
        output out;
}

