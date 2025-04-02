network wn4
{
    @display("bgb=354,108");
    submodules:
        box4a: box4{
        parameters:
            @display("i=,cyan");
            sendMsgOnInit = true;
        };
        box4b: box4{
        parameters:
            @display("i=,red");
            sendMsgOnInit = false;
            
        };
    connections:
        box4a.out --> {  delay = 100ms; } --> box4b.in;
        box4b.out --> {  delay = 100ms; } --> box4a.in;
}

simple box4
{
    parameters:
        bool sendMsgOnInit = default(false);
        int limit = default(2);
        @display("i=block/routing");
    
    gates:
        input in;
        output out;
}

