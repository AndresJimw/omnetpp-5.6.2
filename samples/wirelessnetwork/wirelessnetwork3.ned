network wn3
{
    @display("bgb=354,108");
    submodules:
        box3a: box3{
        parameters:
            @display("i=,cyan");
        };
        box3b: box3{
        parameters:
            @display("i=,red");
        };
    connections:
        box3a.out --> {  delay = 100ms; } --> box3b.in;
        box3b.out --> {  delay = 100ms; } --> box3a.in;
}









simple box3
{
    parameters:
        @display("i=block/routing");
    gates:
        input in;
        output out;
}

