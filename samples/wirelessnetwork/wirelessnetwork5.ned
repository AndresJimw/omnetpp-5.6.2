network wn5
{
    @display("bgb=354,108");
    submodules:
        box5a1: box5a{
        parameters:
            @display("i=,cyan");
            sendMsgOnInit = true;
        };
        box5b1: box5b{
        parameters:
            @display("i=,red");
            sendMsgOnInit = false;
            
        };
    connections:
        box5a1.out --> {  delay = 100ms; } --> box5b1.in;
        box5b1.out --> {  delay = 100ms; } --> box5a1.in;
}

simple box5
{
    parameters:
        bool sendMsgOnInit = default(false);
        int limit = default(2);
        @display("i=block/routing");
    
    gates:
        input in;
        output out;
}

simple box5a extends box5
{
    parameters:
        @display("i=,gold");
        sendMsgOnInit = default(true);
}

simple box5b extends box5
{
    parameters:
        @display("i=,cyan");
        sendMsgOnInit = default(false);
}
