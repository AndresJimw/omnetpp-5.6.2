network wn6
{
    @display("bgb=354,108");
    submodules:
        box6a: box6{
        parameters:
            @display("i=,cyan");
            sendMsgOnInit = true;
        };
        box6b: box6{
        parameters:
            @display("i=,red");
            sendMsgOnInit = false;     
        };
    connections:
        box6a.out --> {  delay = 100ms; } --> box6b.in;
        box6b.out --> {  delay = 100ms; } --> box6a.in;
}

simple box6
{
    parameters:
        bool sendMsgOnInit = default(false);
        int limit = default(2);
        @display("i=block/routing");
    
    gates:
        input in;
        output out;
}

